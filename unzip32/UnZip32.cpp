#include <global.h>
#include "unzip.h"

#ifdef _USRDLL
	#define UnZipErrorHandler UnZipSetLastError
	char gszLastErrorMsg[MAX_PATH];
	DWORD dwLastErrorMsgSize;

	void __cdecl UnZipSetLastError(LPCSTR lpFormattedMsg, ...) {
		DWORD dwErrorCode;
		va_list argptr;
		dwErrorCode = GetLastError();
		va_start(argptr, lpFormattedMsg);
		dwLastErrorMsgSize = wvsprintfA(gszLastErrorMsg, lpFormattedMsg, argptr);
		if ( dwErrorCode && dwLastErrorMsgSize < MAX_PATH-80 ) {
			gszLastErrorMsg[dwLastErrorMsgSize++] = ' ';
			gszLastErrorMsg[dwLastErrorMsgSize++] = ':';
			gszLastErrorMsg[dwLastErrorMsgSize++] = ' ';
			dwLastErrorMsgSize += FormatMessage(
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dwErrorCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &gszLastErrorMsg[dwLastErrorMsgSize], MAX_PATH-dwLastErrorMsgSize, NULL
			);
			dwLastErrorMsgSize = FormatMessage(
				FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_IGNORE_INSERTS | 80,
				&gszLastErrorMsg[0],	NULL,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &gszLastErrorMsg[0], MAX_PATH, NULL
			);
		}
		gszLastErrorMsg[dwLastErrorMsgSize++] = NULL;
		SetLastError(0);
	}

	extern "C" DWORD WINAPI UnZipGetLastError(LPSTR lpBuffer, DWORD nSize) {
		DWORD retValue;
		retValue = dwLastErrorMsgSize;
		if ( nSize < dwLastErrorMsgSize )
			dwLastErrorMsgSize = nSize;
		if ( dwLastErrorMsgSize ) {
			fMemCpyBase(MOV, lpBuffer, LEA, gszLastErrorMsg, dwLastErrorMsgSize);
			dwLastErrorMsgSize = 0;
		}
		return retValue;
	}
#elif _CONSOLE
	#define UnZipErrorHandler PrintLastSysError
#endif

extern DWORD WINAPI UnZipFile32(PSTR lpZipFile, PSTR lpTargetFile) {
	char szCurrentFile[MAX_PATH], szTargetFile[MAX_PATH];
	DWORD nNumberOfBytes, nSize, bResult;
	unz_file_info pfile_info;
	unzFile lpZipArchive;
	LPVOID lpDataBuf;
	HANDLE hFile;
// Open zip file
	lpZipArchive = unzOpen2(lpZipFile, NULL);
	if ( !lpZipArchive ) {
		UnZipErrorHandler("Error: Unable to open \"%s\"", lpZipFile);
		return 0;
	}
	StrCpy(szTargetFile, lpTargetFile);
	StrLowerCase(szTargetFile);
	bResult = FALSE;
	do {
		if ( unzlocal_GetCurrentFileInfoInternal(lpZipArchive, &pfile_info, NULL, szCurrentFile, MAX_PATH, NULL, 0, NULL, 0) != UNZ_OK )
			break;
		StrLowerCase(szCurrentFile);
		if ( strstr(szCurrentFile, szTargetFile) ) {
			bResult = FALSE;
			if ( unzOpenCurrentFile3(lpZipArchive, NULL, NULL, 0, NULL) != UNZ_OK ) {
				UnZipErrorHandler("Error: Failed to open \"%s\" from ZIP file", lpTargetFile);
				break;
			}
			nSize = pfile_info.uncompressed_size;
			lpDataBuf = fmalloc(nSize);
			if ( !lpDataBuf ) {
				UnZipErrorHandler(ERROR_MEM_ALLOC);
				break;
			}
			nNumberOfBytes = unzReadCurrentFile(lpZipArchive, lpDataBuf, nSize);
			if ( nSize != nNumberOfBytes ) {
				ffree(lpDataBuf);
				UnZipErrorHandler("Error: Failed to read from ZIP file");
				break;
			}
			unzCloseCurrentFile(lpZipArchive);
			hFile = _CreateNewFile(lpTargetFile);
			if ( hFile != INVALID_HANDLE_VALUE ) {
				if ( _Write(hFile, (LPCVOID)lpDataBuf, nSize) )
					bResult = TRUE;
				else
					UnZipErrorHandler("Error: Failed to write to \"%s\"", lpTargetFile);
				_Close(hFile);
			} else
				UnZipErrorHandler("Error: Cannot create \"%s\" for writing", lpTargetFile);
			ffree(lpDataBuf);
			break;
		}
	} while ( unzGoToNextFile(lpZipArchive) == UNZ_OK );
	unzClose(lpZipArchive);
	return bResult;
}
