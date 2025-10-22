extern __inline BOOL __fastcall GetFileTimes(PSTR lpFileName, LPFILETIMES lpFileTimes) {
	HANDLE hFile;
	BOOL bResult;
	mMemZeroBy4(lpFileTimes, SIZE FILETIMES/4);
	hFile = CreateFileA(lpFileName, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		bResult = GetFileTime(hFile, &lpFileTimes->ftCreationTime, &lpFileTimes->ftLastAccessTime, &lpFileTimes->ftLastWriteTime);
		CloseHandle(hFile);
		return bResult;
	}
	return FALSE;
}

extern __inline BOOL __fastcall SetFileWriteTimeToCurrentTime(PSTR lpFileName) {
	SYSTEMTIME st; FILETIME ft;
	HANDLE hFile;
	BOOL bResult;
	GetSystemTime(&st);              // Get current time
	SystemTimeToFileTime(&st, &ft);  // Converts to file time format
	hFile = CreateFileA(lpFileName, FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	bResult = SetFileTime(hFile, NULL, NULL, &ft);
	CloseHandle(hFile);
	return bResult;
}

extern PBYTE __fastcall ReadFileToMemory(PSTR lpFileName) {
	DWORD NumberOfBytes;
	LPBYTE lpBuffer;
	HANDLE hFile;
	hFile = _OpenFileRead(lpFileName);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		NumberOfBytes = GetFileSize(hFile, NULL);
		lpBuffer = (PBYTE)_Aligned_Malloc(NumberOfBytes+32, 16); // Add 32 bytes extra to avoid out-of-bounds overlap reads
		if ( !(lpBuffer && _ReadVerify(hFile, lpBuffer, NumberOfBytes)) )
			PrintLastSysError(ERROR_FILE_READOP, lpFileName);
		CloseHandle(hFile);
		return lpBuffer;
	} else {
		PrintLastSysError("Error: Can't open \"%s\" for reading", lpFileName);
		return NULL;
	}
}

extern BOOL __fastcall WriteNewFile(PSTR lpFileName, PVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
	BOOL bResult = FALSE;
	HANDLE hFile;
	hFile = CreateFileA(lpFileName, GENERIC_WRITE|GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH, NULL);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		if ( WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, &gnNumberOfBytes, NULL) && nNumberOfBytesToWrite == gnNumberOfBytes )
			bResult = TRUE;
		else
			PrintLastSysError(ERROR_FILE_WRITEOP, lpFileName);
		CloseHandle(hFile);
	} else
		PrintLastSysError("Error: Can't create \"%s\" for writing", lpFileName);
	return bResult;
}

extern BOOL __fastcall UpdateOldFile(PSTR lpFileName, DWORD dwStartOffset, PVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
	BOOL bResult = FALSE;
	HANDLE hFile;
	hFile = CreateFileA(lpFileName, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH, NULL);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		if ( dwStartOffset )
			SetFilePointer(hFile, dwStartOffset, NULL, FILE_BEGIN);
		if ( WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, &gnNumberOfBytes, NULL) && nNumberOfBytesToWrite == gnNumberOfBytes )
			bResult = TRUE;
		else
			PrintLastSysError(ERROR_FILE_WRITEOP, lpFileName);
		CloseHandle(hFile);
	} else
		PrintLastSysError("Error: Can't open \"%s\" for updating", lpFileName);
	return bResult;
}

extern BOOL __fastcall AppendOldFile(PSTR lpFileName, PVOID lpBuffer, DWORD nNumberOfBytesToWrite) {
	BOOL bResult = FALSE;
	HANDLE hFile;
	hFile = CreateFileA(lpFileName, GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH, NULL);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		SetFilePointer(hFile, 0, NULL, FILE_END);
		if ( WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, &gnNumberOfBytes, NULL) && nNumberOfBytesToWrite == gnNumberOfBytes )
			bResult = TRUE;
		else
			PrintLastSysError(ERROR_FILE_WRITEOP, lpFileName);
		CloseHandle(hFile);
	} else
		PrintLastSysError("Error: Can't open \"%s\" for appending", lpFileName);
	return bResult;
}

// For file sizes 1 - 4,294,967,295 / 0xFFFFFFFF (32-bit max)

NAKED_EXTERN DWORD __fastcall GetFileSize32(PSTR lpFileName) { __asm {
;// *****  Parameters  *****
;// lpFileName = ECX
;// *****  Locals *****
;//	DWORD FileSizeLow  = [ESP];
;// DWORD FileSizeHigh = [ESP+4];
;// ****************************************************************
;// Open file Readonly mode and with no buffer flag/hint for speed
;// 1) hFile = CreateFileA(lpFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL);
;// ****************************************************************
	XOR   EAX, EAX               ;// EAX = 0;
	PUSH  EAX                    ;// PUSH NULL;
	PUSH  FILE_FLAG_NO_BUFFERING ;// PUSH dwFlagsAndAttributes;
	PUSH  OPEN_EXISTING          ;// PUSH dwCreationDisposition;
	PUSH  EAX                    ;// PUSH NULL;
	PUSH  FILE_SHARE_READ|FILE_SHARE_WRITE ;// PUSH dwShareMode;
	PUSH  GENERIC_READ           ;// PUSH dwDesiredAccess;
	PUSH  ECX                    ;// PUSH lpFileName;
	CALL  DWORD PTR [CreateFileA]
;// 2) ( hFile == INVALID_HANDLE_VALUE ) ? Yes, return 0; No, continue;
	CMP   EAX, -1
	JE    RET_FAILURE
	SUB   ESP, 8      ;//[ESP] = FileSizeLow; [ESP+4] = FileSizeHigh;
	XOR   ECX, ECX    ;// ECX = 0;
	LEA   EDX, [ESP+4];// Get &FileSizeHigh
	MOV  [EDX], ECX   ;// FileSizeHigh = 0;
;// For CloseHandle(); later
	PUSH  EAX         ;// PUSH hFile;
;// For GetFileSize(); later
	PUSH  EDX         ;// PUSH &FileSizeHigh;
	PUSH  EAX         ;// PUSH hFile;
;// 3) SetLastError(0);
	PUSH  ECX         ;// PUSH 0;
	CALL  DWORD PTR [SetLastError] ;// Clear last error to trap >4GB condition or not
;// 4) FileSizeLow = GetFileSize(hFile, &FileSizeHigh);
	CALL  DWORD PTR [GetFileSize]
;// Make sure we have an error and NOT a file greater than
;// 4GB in which case we return 0 anyway to force an error
;// 5) if ( FileSizeLow == INVALID_FILE_SIZE && GetLastError() || FileSizeHigh ) FileSizeLow = 0;
	CMP   EAX, -1
	JE    CHECK_FILE_SIZE ;// Might be INVALID_FILE_SIZE error OR 0xFFFFFFFF in size, must check!
VALID_FILE_SIZE:
	MOV   [ESP+4], EAX    ;// Save FileSizeLow!
;// 6) CloseHandle(hFile);
	CALL  DWORD PTR [CloseHandle]
	MOV   EAX,[ESP+4]   ;// Get *FileSizeHigh
	TEST  EAX, EAX      ;// FileSizeHigh == 0 ? Yes, return 32-bit size; No, return 0 as error, big too file!
	JNZ   LIMIT32_ABORT ;// File is >4GB, 32-bit limit exceeded, return 0;
	MOV   EAX, [ESP]    ;// return FileSizeLow;
	ADD   ESP, 8        ;// Clean stack
	RET

CHECK_FILE_SIZE:
	CALL  DWORD PTR [GetLastError]
	TEST  EAX, EAX      ;// GetLastError == 0 ? Yes, return valid 4GB file size; No, return 0 for error!
	JNZ   ERROR_DETECTED
	LEA   EAX, [EAX-1]  ;// EAX = 0xFFFFFFFF;
	JMP   VALID_FILE_SIZE
ERROR_DETECTED:
	CALL  DWORD PTR [CloseHandle] ;// 6) CloseHandle(hFile); GetLastError>0, close, failure, return 0;

LIMIT32_ABORT:
	ADD   ESP, 8   ;// Clean stack
RET_FAILURE:
	XOR   EAX, EAX ;// return 0;
	RET
}}

extern BOOL __fastcall GetResourceFile(PSTR szModuleFileName, PSTR szOutputFileName, PSTR szResName, PSTR szResType) {
	HMODULE hResModule;
	HGLOBAL hResLoad;
	HRSRC hResInfo;
	DWORD nSize;

	hResModule = GetModuleHandleA(szModuleFileName);
	if ( hResModule == NULL ) {
		PrintLastSysError("Error: Can't obtain module handle");
		return FALSE;
	}
	hResInfo = FindResourceA(hResModule, szResName, szResType);
	if ( hResInfo == NULL ) {
		PrintLastSysError("Error: Can't find resource");
		return FALSE;
	}
	hResLoad = LoadResource(hResModule, hResInfo);
	if ( hResLoad == NULL ) {
		PrintLastSysError("Error: Can't load resource");
		return FALSE;
	}
	nSize = SizeofResource(hResModule, hResInfo);
	if ( nSize == 0 ) {
		PrintLastSysError("Error: Can't return resource size");
		return FALSE;
	}
	return WriteNewFile(szOutputFileName, hResLoad, nSize);
}
/*
 * GetFileCount
 *
 * Can recursively find and count all >0 files relative from the
 * current folder based on a wildcard search string (e.g. "*").
 * 
*/
extern DWORD __fastcall GetFileCount(PSTR lpFileFilter) {
	WIN32_FIND_DATAA fd;
	DWORD FileCount;
	HANDLE hFind;
	FileCount = 0;
// Recursively find all subfolders
	hFind = FindFirstFileA("*", &fd);
	if ( hFind != INVALID_HANDLE_VALUE ) {
		do {
			if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && *fd.cFileName != '.' ) {
				SetCurrentDirectoryA(fd.cFileName);
				FileCount += GetFileCount(lpFileFilter);
				SetCurrentDirectoryA("..");
			}
		} while ( FindNextFileA(hFind, &fd) );
		FindClose(hFind);
	}
// Find all files in current folder and count
	hFind = FindFirstFileA(lpFileFilter, &fd);
	if ( hFind != INVALID_HANDLE_VALUE ) {
		do {
			if ( fd.nFileSizeLow > 0 )
				FileCount++;
		} while ( FindNextFileA(hFind, &fd) );
		FindClose(hFind);
	}
	return FileCount;
}
/*
 * GetFileCountFast
 *
 * Can recursively find and count all files relative from the
 * current folder based on a wildcard search string (e.g. "*").
 * Other than '*', the filter normally applies to the current folder.
*/
extern DWORD __fastcall GetFileCountFast(PSTR lpFindFilter) {
	WIN32_FIND_DATAA fd;
	DWORD FileCount = 0;
	HANDLE hFind;
// Recursively find all subfolders
	hFind = FindFirstFileA(lpFindFilter, &fd);
	if ( hFind != INVALID_HANDLE_VALUE ) {
		do {
			if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && *fd.cFileName != '.' ) {
				SetCurrentDirectoryA(fd.cFileName);
				FileCount += GetFileCount(lpFindFilter);
				SetCurrentDirectoryA("..");
			} else if ( fd.nFileSizeLow ) // Only count files with size > 0
				FileCount++;
		} while ( FindNextFileA(hFind, &fd) );
		FindClose(hFind);
	}
	return FileCount;
}

extern BOOL __fastcall MapFileFastRead(IN PSTR lpFileName, OUT MAPPEDFILE *mFile) {
	HANDLE hFile;
	mFile->dwFileSize = 0;
// Open file with sequential scan flag for start to end fast reads
	hFile = CreateFileA(
		lpFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, NULL
	);
	if ( hFile == INVALID_HANDLE_VALUE ) {
		PrintLastSysError(ERROR_FILE_MAPOPEN, lpFileName);
		return FALSE;
	}
// Create a file-mapping object for the file.
	mFile->hMapFile = CreateFileMappingA(hFile, // current file handle
                                NULL, // default security
                                PAGE_READONLY, // read/write permission
                                0,  // size of mapping object, high
                                0, // size of mapping object, low
                                NULL); // name of mapping object
	if ( mFile->hMapFile == NULL ) {
		PrintLastSysError(ERROR_FILE_MAPCREATE, lpFileName);
		CloseHandle(hFile);
		return FALSE;
	}
// Map the view and test the results.
	mFile->lpMapAddress = (LPBYTE)MapViewOfFile(mFile->hMapFile, // handle to mapping object
                               FILE_MAP_READ, // read/write permission 
                               0, // high-order 32 bits of file offset
                               0, // low-order 32 bits of file offset
                               0); // number of bytes to map
	if ( mFile->lpMapAddress == NULL ) {
		PrintLastSysError(ERROR_FILE_MAPVIEW);
		CloseHandle(mFile->hMapFile);
		CloseHandle(hFile);
		return FALSE;
	}
	mFile->hFile = hFile;
	mFile->dwFileSize = GetFileSize(hFile, NULL); // Assumes file is <= 4GB
	return TRUE;
}

NAKED_EXTERN DWORD __fastcall MapFile32(PSTR lpFileName, BOOL ReadOnly, OUT MAPPEDFILE *mFile) {
	static char gERROR_FILE_MAPOPEN[]   = ERROR_FILE_MAPOPEN;
	static char gERROR_FILE_MAPCREATE[] = ERROR_FILE_MAPCREATE;
	static char gERROR_FILE_MAPVIEW[]   = ERROR_FILE_MAPVIEW;
	__asm {
;// *****  Parameters  *****
;// lpFileName = ECX
;// ReadOnly   = EDX
;// mFile      =[ESP+4 + PUSH32*n]
	PUSH  EBX
	XOR   EAX, EAX ;// EAX = 0;
	PUSH  ESI
	XOR   EBX, EBX ;// EBX = 0;
;// ****************************************************************
;// 1) hFile = CreateFileA(lpFileName, dwDesiredAccess, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING|FILE_FLAG_RANDOM_ACCESS, NULL);
;// ****************************************************************
	PUSH  EAX                     ;// PUSH NULL;
	PUSH  FILE_FLAG_NO_BUFFERING  ;// PUSH dwFlagsAndAttributes;
	PUSH  OPEN_EXISTING           ;// PUSH dwCreationDisposition;
	PUSH  EAX                     ;// PUSH NULL;
	PUSH  FILE_SHARE_READ|FILE_SHARE_WRITE ;// PUSH dwShareMode;
	TEST  EDX, EDX      ;// ( ReadOnly )?? No, PUSH GENERIC_READ | GENERIC_WRITE; Yes, PUSH GENERIC_READ
	JNZ   L1
		MOV   BL, PAGE_READWRITE           ;// Mapping/flProtect = PAGE_READWRITE
		MOV   BH, SECTION_MAP_WRITE        ;// MapViewOfFileAccess = FILE_MAP_READ | FILE_MAP_WRITE
		PUSH  GENERIC_READ | GENERIC_WRITE ;// PUSH dwDesiredAccess;
		JMP   L2
	L1:
		MOV   BL, PAGE_READONLY    ;// Mapping/flProtect = PAGE_READONLY
		MOV   BH, SECTION_MAP_READ ;// MapViewOfFileAccess = FILE_MAP_READ
		PUSH  GENERIC_READ         ;// PUSH dwDesiredAccess;
	L2:
	PUSH  ECX           ;// PUSH lpFileName;
	MOV   ESI, ECX      ;// Copy lpFileName in case of failure
	CALL  DWORD PTR [CreateFileA]
	;// ( hFile == INVALID_HANDLE_VALUE ) ? No, continue; Yes, set FileSize/Addr to 0, print error, return 0;
		CMP   EAX, -1
		JNE   FILE_READY
			PUSH  ESI                         ;// PUSH lpFileName
			PUSH  OFFSET gERROR_FILE_MAPOPEN  ;// PUSH gERROR_FILE_MAPOPEN
			CALL  PrintLastSysError           ;// PrintLastSysError(ERROR_FILE_MAPOPEN, lpFileName);
			ADD   ESP, 8        ;// Clean stack parameters of __cdecl CALL
			MOV   ESI,[ESP+4+8] ;// Get mFile in ESI
			XOR   EAX, EAX      ;// EAX=0; return 0; (failure)
		;// Zero out mFile struct
			MOV  [ESI]MAPPEDFILE.lpMapAddress, EAX
			MOV  [ESI]MAPPEDFILE.dwFileSize, EAX
			MOV  [ESI]MAPPEDFILE.hMapFile, EAX
			MOV  [ESI]MAPPEDFILE.hFile, EAX
			POP   EBX
			POP   ESI
			RET   4
		FILE_READY:
	MOV   ESI,[ESP+4+8] ;// Get mFile in ESI
;// ****************************************************************
;// 2) Create a file-mapping object for the file. 
;// hMapFile = CreateFileMappingA(hFile, NULL, flProtect/MappingAccess, 0, 0, NULL);
;// ****************************************************************
	XOR   ECX, ECX   ;// ECX = 0/NULL;
	MOV  [ESI]MAPPEDFILE.hFile, EAX ;// mFile->hFile = hFile;
	MOVZX EDX, BL    ;// Get MappingAccess
	PUSH  ECX        ;// PUSH lpName;
	PUSH  ECX        ;// PUSH dwMaximumSizeLow;
	PUSH  ECX        ;// PUSH dwMaximumSizeHigh;
	PUSH  EDX        ;// PUSH flProtect (MappingAccess); Read/write permission
	PUSH  ECX        ;// PUSH lpAttributes;
	PUSH  EAX        ;// PUSH hFile;
	CALL  DWORD PTR [CreateFileMappingA]
	;// ( hFileMap == NULL ) ? No, continue; Yes, set mFile 0, print error, return 0;
		TEST  EAX, EAX
		JNZ   MAP_READY
			PUSH  DWORD PTR [ESI]MAPPEDFILE.hFile  ;// PUSH hFile;
		;// Zero out mFile struct
			MOV  [ESI]MAPPEDFILE.lpMapAddress, EAX
			MOV  [ESI]MAPPEDFILE.dwFileSize, EAX
			MOV  [ESI]MAPPEDFILE.hMapFile, EAX
			MOV  [ESI]MAPPEDFILE.hFile, EAX
			CALL  DWORD PTR [CloseHandle]          ;// CloseHandle(hFile);
			PUSH  OFFSET gERROR_FILE_MAPCREATE     ;// PrintLastSysError(ERROR_FILE_MAPCREATE);
			CALL  PrintLastSysError
			ADD   ESP, 4   ;// Clean stack parameters of __cdecl CALL
			XOR   EAX, EAX ;// return 0; (failure)
			POP   EBX
			POP   ESI
			RET   4
		MAP_READY:
;// ****************************************************************
;// 3) Map the view with the file-mapping object
;// lpMapAddress = MapViewOfFile(hFileMappingObject, dwDesiredAccess, 0, 0, 0);
;// ****************************************************************
	XOR   ECX, ECX     ;// ECX = 0;
	MOV  [ESI]MAPPEDFILE.hMapFile, EAX ;// mFile->hMapFile = hMapFile;
	MOVZX EDX, BH      ;// Get MapViewOfFileAccess as dwDesiredAccess
	PUSH  ECX          ;// PUSH 0; (SIZE_T dwNumberOfBytesToMap)
	PUSH  ECX          ;// PUSH 0; (DWORD dwFileOffsetLow)
	PUSH  ECX          ;// PUSH 0; (DWORD dwFileOffsetHigh)
	PUSH  EDX          ;// PUSH dwDesiredAccess; Read/Write permission
	PUSH  EAX          ;// PUSH hFileMappingObject;
	CALL  DWORD PTR [MapViewOfFile]
	;// ( lpMapAddress == NULL ) ? No, continue; Yes, set mFile 0, print error, return 0;
		TEST  EAX, EAX
		JNZ   MAPVIEW_READY
			PUSH  DWORD PTR [ESI]MAPPEDFILE.hMapFile
			CALL  DWORD PTR [CloseHandle]    ;// CloseHandle(hMapFile);
			PUSH  DWORD PTR [ESI]MAPPEDFILE.hFile
			CALL  DWORD PTR [CloseHandle]    ;// CloseHandle(hFile);
			PUSH  OFFSET gERROR_FILE_MAPVIEW ;// PrintLastSysError(ERROR_FILE_MAPVIEW);
			CALL  PrintLastSysError
			XOR   EAX, EAX ;// EAX=0; return 0; (failure)
			ADD   ESP, 4   ;// Clean stack parameters of __cdecl CALL
		;// Zero out mFile struct
			MOV  [ESI]MAPPEDFILE.lpMapAddress, EAX
			MOV  [ESI]MAPPEDFILE.dwFileSize, EAX
			MOV  [ESI]MAPPEDFILE.hMapFile, EAX
			MOV  [ESI]MAPPEDFILE.hFile, EAX
			POP   EBX
			POP   ESI
			RET   4
		MAPVIEW_READY:
	XOR   ECX, ECX
	MOV  [ESI]MAPPEDFILE.lpMapAddress, EAX
;//	dwFileSize = GetFileSize(mFile->hFile, NULL); // Assumes file is < 4GB. Mapping fails past ~2GB anyway!
	PUSH  ECX                  ;// PUSH NULL;
	PUSH [ESI]MAPPEDFILE.hFile ;// PUSH hFile;
	CALL  DWORD PTR [GetFileSize]
	POP   EBX
;// The map fails if filesize = 0, so we know it's > 0 without a test
	MOV  [ESI]MAPPEDFILE.dwFileSize, EAX ;// mFile->dwFileSize = EAX; saves & returns nonzero size of file/mapaddress
	POP   ESI
	RET   4
}}

EXTERN void __cdecl FastLog(PSTR lpLogFileName, char *format, ...) {
	char buffer[1024];
	va_list argptr;
	HANDLE hFile;
	DWORD Size;
	va_start(argptr, format);
	Size = wvsprintf(buffer, format, argptr);
	va_end(argptr);
	hFile = _OpenAppendFile(lpLogFileName);
	if ( hFile == INVALID_HANDLE_VALUE )
		hFile = _CreateNewFile(lpLogFileName);
	else
		SetFilePointer(hFile, 0, 0, FILE_END);
	WriteFile(hFile, buffer, Size, &gnNumberOfBytes, NULL);
	_Close(hFile);
}