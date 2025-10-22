OSInfo  gOSInfo;
CPUInfo gCPUInfo;

/* CPUID - added to later Intel486 & Pentium CPUs in 1993
Function 0x01:  Returns the Processor Family, Model, and
Stepping info in EAX. EDX gets the Standard Feature Flags.

EDX register holds a bitmask of some features needed to
determine which SIMD extensions our CPU supports.

| Bit (EDX) | Feature |
|    23	    |  MMX    |
|    25	    |  SSE    |
|    26	    |  SSE2   |
*/

EXTERN NAKED void __fastcall GetCPUInfo(int ZERO_ECX, int ZERO_EDX) { __asm {
;// Call Win9X/NT GetSystemInfo() for some basic CPU info: 386/486 (Win95) or >=586 Pentium
	PUSH  OFFSET gCPUInfo.siSysInfo
	CALL  DWORD PTR [GetSystemInfo]
	MOV   EAX, gCPUInfo.siSysInfo.dwProcessorType
	CMP   EAX, 586   ;// ( CPU >= 586 )?? PROCESSOR_INTEL_PENTIUM++, 3/22/1993 CPU, CPUID is available, GO!
	JAE   CPUID_SAFE
;// Abort for Windows 9X/NT3 on 80386-80486 CPUs!
	CMP   EAX, 486   ;// PROCESSOR_INTEL_486
	JE    EXIT_ABORT
	CMP   EAX, 386   ;// PROCESSOR_INTEL_386
	JE    EXIT_ABORT
	CMP   gOSInfo.MajorVersion, 5 ;// Windows 2000 requires 586 Pentium++, so if lower, exit!
	JAE   CPUID_SAFE
EXIT_ABORT:
;// Abort, no need to risk CPUID on old CPU or on failure to detect >=586
;// MMX/SSE flags will be left FALSE, so it's assumed no support
		RET

CPUID_SAFE:
;// Function 0 (Highest Function Parameter and Manufacturer ID)
	XOR   EAX, EAX
	XOR   EBX, EBX
	XOR   EDX, EDX
	XOR   ECX, ECX
	CPUID
	LEA   EAX, gCPUInfo.CPUString
	MOV  [EAX], EBX
	MOV  [EAX+4], EDX
	MOV  [EAX+8], ECX
;// Function 1 (Processor Info and Feature Bits)
	MOV   EAX, 1
	XOR   ECX, ECX
	XOR   EDX, EDX
	CPUID
;// Test bit 23 = 1 if MMX is supported
;// Pentium/586 [MMX] released October 1996 (1997 to market)
	TEST  EDX, 0x800000
	JZ    L_MMX1
		MOV gCPUInfo.MMX, 1
	L_MMX1:
;// Test bit 0 = 1 if SSE3 is supported
	TEST  ECX, 1
	JZ    L_SSE3
	;// SSE3 flagged on as supported, but also test
	;// bits 26 = 1 (SSE2) & 25 = 1 (SSE1) (just in case)
		TEST EDX, 0x6000000
		JZ   L_SSE2
			MOV gCPUInfo.SSE3, 1
			MOV gCPUInfo.SSE2, 1
			MOV gCPUInfo.SSE1, 1
			JMP L_SSE1
	L_SSE3:
;// Test bit 26 = 1 if SSE2 is supported
	TEST  EDX, 0x4000000
	JZ    L_SSE2
		MOV gCPUInfo.SSE2, 1
		MOV gCPUInfo.SSE1, 1
		JMP L_SSE1
	L_SSE2:
;// Test bit 25 = 1 if SSE is supported
	TEST  EDX, 0x2000000
	JZ    L_SSE1
		MOV gCPUInfo.SSE1, 1
	L_SSE1:
;// Check if extended functions are supported for CPU Name/Brand string
	MOV   EAX, 0x80000000
	CPUID
	CMP   EAX, 0x80000004
	JAE   GET_CPUSTRING
		RET

GET_CPUSTRING:
;// 1) Get the 1st 16 bytes of the processor name
	MOV   EAX, 0x80000002
	LEA   EDI, gCPUInfo.CPUBrandString
	CPUID
	MOV  [EDI], EAX
	MOV  [EDI+4], EBX
	MOV  [EDI+8], ECX
	MOV  [EDI+12], EDX
	ADD   EDI, 16
;// 2) Get the 2nd 16 bytes of the processor name
	MOV EAX, 0x80000003
	CPUID
	MOV  [EDI], EAX
	MOV  [EDI+4], EBX
	MOV  [EDI+8], ECX
	MOV  [EDI+12], EDX
	ADD   EDI, 16
;// 3) Get the last 16 bytes of the processor name
	MOV EAX, 0x80000004
	CPUID
	XOR  EDX, EDX
	MOV  [EDI], EAX
	MOV  [EDI+4], EBX
	MOV  [EDI+8], ECX
	MOV  [EDI+12], EDX

	RET
}}

/* Windows Version Chart
+--------------------------------------
|  Version  PlatformID  Major   Minor |
+--------------------------------------
| Win95     |    1    |   4   |   0   | 4.00.0950 - 4.03.1214
| Win98     |    1    |   4   |  10   | 4.10.1998 - 4.10.2222 A
| WinME     |    1    |   4   |  90   | 4.90.2476 - 4.90.3000

| WinNT31   |    2    |   3   |  10   | 3.10.0528
| WinNT35   |    2    |   3   |  50   | 3.50.0807
| WinNT351  |    2    |   3   |  51   | 3.51.1057
| WinNT4    |    2    |   4   |   0   | 4.00.1381
| Win2000   |    2    |   5   |   0   | 5.00.1515 - 5.00.2195
| WinXP     |    2    |   5   |   1   | 5.01.2505 - 5.01.2600
| WinSvr2003|    2    |   5   |   2   | 5.02.3718 - 5.02.3790
| WinVista  |    2    |   6   |   0   | 6.00.6000 - 6.00.6002
| Win7      |    2    |   6   |   1   | 6.01.7601
| Win8      |    2    |   6   |   2   | 6.02.9200 - 6.02.10211
| Win8.1    |    2    |   6   |   3   | 6.03.9200 - 6.3.9600
| WinSvr2012|    3    |   6   |   3   | 6.03.9600
| Win10     |    2    |   10  |   0   | 10.0.10240

| WinCE     |    3    |   ?   |   ?   |
+----------+------------+-------+-------+

Example, normal WinXP detection
	IsWinXPorUp = gOSInfo.MajorVersion > 5 || (gOSInfo.MajorVersion == 5 && gOSInfo.MinorVersion > 0);

"IsWow64Process" API was introduced with XP, other detection method (guaranteed if in compatibility mode)!
	IsWinXPorUp = GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");
*/
EXTERN int __fastcall DetectWinOS() {
	struct _OSVERSIONINFOEXA OSVerInfoEx;
	struct _OSVERSIONINFOA OSVerInfo;
	char szFolder[MAX_PATH];
	BOOL bSIMDReady;

	__asm CLD // Always clear direction flag
// Make sure CRT Heap is initialized.
	InitCRTHeap(); // Get our process heap for memory functions
// a) Do not display the critical-error-handler message box
// b) Do not make alignment faults visible to an application
// c) Do not display a message box when system fails to find a file
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT | SEM_NOOPENFILEERRORBOX);
// Detect Windows Operating System Families
	mMemZeroBy4_(OSVerInfo, SIZE OSVerInfo/4);
	__asm MOV bSIMDReady, EAX ;// bSIMDReady = FALSE/0;
	OSVerInfo.dwOSVersionInfoSize = sizeof(OSVerInfo);
	if ( !GetVersionExA(&OSVerInfo) ) return 0;
	gOSInfo.PlatformID = (BYTE)OSVerInfo.dwPlatformId;
	gOSInfo.MajorVersion = (BYTE)OSVerInfo.dwMajorVersion;
	gOSInfo.MinorVersion = (BYTE)OSVerInfo.dwMinorVersion; 
// dwPlatformId=1 Windows 95/98/ME          (VER_PLATFORM_WIN32_WINDOWS)
// dwPlatformId=2 Windows NT/2K/XP/V/7/8/10 (VER_PLATFORM_WIN32_NT).
// dwPlatformId=3 Windows CE, but unlikely we're executing on that!
	if ( OSVerInfo.dwPlatformId >= 2 /*VER_PLATFORM_WIN32_NT*/ ) {
		gOSInfo.IsWinNT = (BYTE)OSVerInfo.dwPlatformId;
		mMemZeroBy4_(OSVerInfoEx, SIZE OSVerInfoEx/4);
		OSVerInfoEx.dwOSVersionInfoSize = sizeof(OSVerInfoEx);
		if ( GetVersionExA((LPOSVERSIONINFOA)&OSVerInfoEx) ) {
			gOSInfo.IsServer = ( OSVerInfoEx.wProductType == VER_NT_SERVER );
			gOSInfo.ServicePackMajor = (BYTE)OSVerInfoEx.wServicePackMajor;
		}
		if ( gOSInfo.MajorVersion == 6 && gOSInfo.MinorVersion == 1 )
			gOSInfo.IsWin7 = TRUE;
		if ( gOSInfo.MajorVersion > 4 ) // Win2K++, safe to use SIMD SSE
			bSIMDReady = TRUE;
	// Detect 64-bit Windows (XP/Vista/2003 or greater. Win2000 is final 32-bit OS)
		GetProcAddress(GetModuleHandleA(gKernel32DLL), "GetSystemWow64DirectoryA");
		__asm {
			TEST  EAX, EAX      ;// EAX!=0 ? yes, Windows64; no, Windows32.
			JZ    L_WIN32
			;// GetSystemWow64DirectoryA() confirmed, but proceed for 100% 64-bit confirmation!
			;// On 32-bit Windows the function always fails, error ERROR_CALL_NOT_IMPLEMENTED.
				LEA   ECX, szFolder
				PUSH  MAX_PATH  ;// Push 260/MAX_PATH/sizeof(szFolder)
				PUSH  ECX       ;// Push &szFolder[0-260] address
				CALL  EAX       ;// CALL GetSystemWow64DirectoryA(lpBuffer, uSize);
				TEST  EAX, EAX  ;// EAX!=0 ? yes, Windows64; no, Windows32.
				JZ    L_WIN32
					MOV   gOSInfo.Is64bit, TRUE ;// Success, WOW64 dir, so CPU/Win 64-bit 100% confirmed!
			L_WIN32:
		}
	} else if ( OSVerInfo.dwPlatformId == 1 /*VER_PLATFORM_WIN32_WINDOWS*/ && gOSInfo.MajorVersion == 4 ) {
	// Windows 95/98/ME Detected
		gOSInfo.IsWin9X = TRUE;
		if ( OSVerInfo.dwMinorVersion < 10 ) // Win95
			gOSInfo.IsWin95 = TRUE;
		else if ( OSVerInfo.dwMinorVersion == 10 ) // Win98
			gOSInfo.IsWin98 = TRUE;
		else if ( OSVerInfo.dwMinorVersion == 90 ) { // WinME
			gOSInfo.IsWinME = TRUE;
			bSIMDReady = TRUE;
		}
	} else { // dwPlatformId=0 Failure, 0 results, or Win3.1 (should be impossible, so abort)
		ShowLastSysError("Error: Windows OS GetVersionEx() detection failure");
		return 0;
	}
	GetExeDirectory(MAX_PATH, gOSInfo.ExeDir);
	//i = GetCurrentDirectoryA(MAX_PATH, gOSInfo.CurrentDir);
	//if ( i && gOSInfo.ExeDir[i-1] != '\\' )
	//	*(LPDWORD)&gOSInfo.CurrentDir[i] = 0x0000005C;

// MMX/SSE detection accelerations!
	__asm CALL GetCPUInfo
// Disable SSE for old Windows 95/98/NT4!
	if ( !bSIMDReady )
		gCPUInfo.SSE3 = gCPUInfo.SSE2 = gCPUInfo.SSE1 = 0;
	return 0;
}
/*
 * CenterWindow(HWND hWindow) : Centers a window within its parent, owner or whole screen as last resort
 */
EXTERN void __fastcall CenterWindow(HWND hWindow) {
	RECT rcOwner, rcRect;
	int nWidth, nHeight;
	HWND hWndOwner;
// Get current window width/height
	if ( !GetWindowRect(hWindow, &rcRect) )
		return;
	nWidth = rcRect.right - rcRect.left;
	nHeight = rcRect.bottom - rcRect.top;
// Get Parent/owner window first if exists, if not, center over whole desktop/screen
	hWndOwner = GetParent(hWindow);
	if ( !hWndOwner )
		hWndOwner = GetWindow(hWindow, GW_OWNER); // This can work if GetParent() fails!
	if ( hWndOwner )
		GetWindowRect(hWndOwner, &rcOwner); // Get owner window's rect
// If no Parent/Owner, get the Desktop Shell rect (AKA current Screen resolution)
	else if ( !GetWindowRect(GetDesktopWindow(), &rcOwner) )
		return; // Shell crashed/not running?
// Center window over owner or desktop
	MoveWindow(hWindow, rcOwner.left + (rcOwner.right-rcOwner.left-nWidth)/2, rcOwner.top + (rcOwner.bottom-rcOwner.top-nHeight)/2, nWidth, nHeight, TRUE);
	Sleep(0); // Give CPU time to other threads/windows/for repainting
}

void __fastcall ResizeWindow(HWND hWindow, int nNewWidth, int nNewHeight) {
	RESIZE_WINDOW(hWindow, nNewWidth, nNewHeight);
}

void __fastcall RestoreWindow(HWND hWindow) {
	ShowWindow(hWindow, SW_RESTORE);
	SetForegroundWindow(hWindow);
}

void __fastcall SetWindowTopMost(HWND hWindow) {
	SetWindowPos(hWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE | SWP_SHOWWINDOW);
	ShowWindow(hWindow, SW_RESTORE);
	SetForegroundWindow(hWindow);
}

// Manual CTRL+V Detection:
// if ( GetKeyState(VK_CONTROL) & 0x8000 && wParam == 'V' ) PasteToTextBox(hWnd);

EXTERN BOOL __fastcall PasteToTextBox(HWND hTextBox) {
	HGLOBAL hglb;
	BOOL bResult;
    if ( !IsClipboardFormatAvailable(CF_TEXT) )
        return FALSE;
    if ( !OpenClipboard(NULL) )
        return FALSE;
	hglb = GetClipboardData(CF_TEXT);
	if ( hglb ) {
		bResult = (BOOL)SendMessageA(hTextBox, WM_SETTEXT, 0, (LPARAM)GlobalLock(hglb));
		GlobalUnlock(hglb);
	} else
		bResult = FALSE;
	CloseClipboard();
	return bResult;
}

EXTERN DWORD __fastcall GetExeFile(PSTR lpBuffer, DWORD uSize) {
	LPSTR lpDir;
	DWORD i;
	i = GetModuleFileNameA(NULL, lpBuffer, uSize);
	if ( i > 0 ) {
		lpDir = AddPtr(LPSTR, lpBuffer, i);
		for ( i = 0; *lpDir != '\\'; ) {
			i++;
			lpDir--;
			if ( lpDir == lpBuffer )
				break;
		}
		lpDir++;
		while ( *lpDir )
			*lpBuffer++ = *lpDir++;
		*lpBuffer = 0;
		return i;
	}
	return 0;
}

EXTERN DWORD __fastcall GetExeDirectory(DWORD uSize, PSTR lpBuffer) {
	PSTR lpDir;
	DWORD i;
	i = GetModuleFileNameA(NULL, lpBuffer, uSize);
	if ( i > 0 ) {
		lpDir = AddPtr(LPSTR, lpBuffer, i);
		while ( *lpDir != '\\' ) {
			lpDir--;
			if ( lpDir == lpBuffer )
				break;
		}
		*(++lpDir) = 0;
		return SubPtr(DWORD, lpDir, lpBuffer);
	}
	return 0;
}

EXTERN BOOL __fastcall SetVGAMode(DWORD nWidth, DWORD nHeight, DWORD nBitsPerPixel, DWORD nFrequency) {
	DEVMODEA95 DevMode;
	LONG lResult;
	#ifndef _WIN64
		mMemZeroBy4_(DevMode, SIZE DEVMODEA95/4);
	#else
		ZeroMemory(&DevMode, sizeof(DEVMODEA95));
	#endif
	DevMode.dmSize = sizeof(DEVMODEA95);
	if ( EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, (LPDEVMODEA)&DevMode) ) {
		if ( nWidth ) {
			DevMode.dmPelsWidth = nWidth;
			DevMode.dmFields |= DM_PELSWIDTH;
		}
		if ( nHeight ) {
			DevMode.dmPelsHeight = nHeight;
			DevMode.dmFields |= DM_PELSHEIGHT;
		}
		if ( nBitsPerPixel ) {
			DevMode.dmBitsPerPel = nBitsPerPixel;
			DevMode.dmFields |= DM_BITSPERPEL;
		}
		if ( nFrequency ) {
			DevMode.dmDisplayFrequency = nFrequency;
			DevMode.dmFields |= DM_DISPLAYFREQUENCY;
		}
	// Win95-tested/approved - only CDS_UPDATEREGISTRY flag valid (CDS_GLOBAL=invalid param/fail)
		lResult = ChangeDisplaySettingsA((LPDEVMODEA)&DevMode, CDS_UPDATEREGISTRY);
		switch ( lResult ) {
			case DISP_CHANGE_SUCCESSFUL:
				return TRUE;
			case DISP_CHANGE_BADMODE:
				ShowLastSysError("Error: The graphics mode is not supported");
				return FALSE;
			case DISP_CHANGE_FAILED:
				ShowLastSysError("Error: The display driver failed the specified graphics mode");
				return FALSE;
			case DISP_CHANGE_RESTART:
				ShowLastSysError("Error: The computer must be restarted in order for the graphics mode to work");
		}
	}
	return FALSE;
}

EXTERN DWORD __cdecl ShowLastSysError(LPCSTR format, ...) {
	char szMsgBuf[1024];
	va_list argptr;
	DWORD dwErrorCode;
	dwErrorCode = GetLastError();
	va_start(argptr, format);
	wvsprintfA(szMsgBuf, format, argptr);
	SetLastError(dwErrorCode);
	GetLastSysError(szMsgBuf, sizeof(szMsgBuf), szMsgBuf);
	MessageBoxA(GetForegroundWindow(), szMsgBuf, gError, MB_OK | MB_ICONERROR);
	return dwErrorCode;
}

EXTERN DWORD __cdecl GetLastSysError(PSTR lpBuffer, UINT uSize, LPCSTR format, ...) {
	DWORD dwErrorCode, nLength;
	char *lpMessage;
	va_list argptr;
// Get last error code if reported
	dwErrorCode = GetLastError();
	SetLastError(0);
// Handle user-defined error message first
	va_start(argptr, format);
	nLength = wvsprintfA(lpBuffer, format, argptr);
	if ( dwErrorCode && nLength+4 < uSize ) {
		lpMessage = AddPtr(char*, lpBuffer, nLength);
		*(LPDWORD)lpMessage = '  : ';
	// Load Windows system error if reported 
		FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL, dwErrorCode, 0, // Default language
			lpMessage+3, (uSize-3)-nLength, NULL
		);
	// Wraps string at about 80 characters per line in standard Windows messagebox
		nLength = FormatMessageA(
			FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_IGNORE_INSERTS | 80,
			(LPSTR)lpBuffer, 0, 0, // Default language
			lpBuffer, uSize, NULL
		);
		if ( lpBuffer[nLength-1] == ' ' )
			nLength--;
	}
	lpBuffer[nLength] = 0;
	return dwErrorCode;
}

EXTERN BOOL __fastcall SystemShutdown() {
	PROC_OPENPROCESSTOKEN lpfnOpenProcessTokenProc;
	TOKEN_PRIVILEGES tkp;
	HMODULE hModule;
	HANDLE hToken;
	hModule = LoadLibraryA(gRegistryDLL);
	lpfnOpenProcessTokenProc = (PROC_OPENPROCESSTOKEN)GetProcAddress(hModule, "OpenProcessToken");
	if ( lpfnOpenProcessTokenProc ) { // WinNT/2K/XP Only!
	// Get the current process token handle so we can get shutdown privilege.
		if ( !lpfnOpenProcessTokenProc(GetCurrentProcess(), 
			TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) ) {
				FreeLibrary(hModule);
				return FALSE;
		}
	// Get the LUID for shutdown privilege. 
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
		tkp.PrivilegeCount = 1;  // one privilege to set
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	// Get shutdown privilege for this process.
		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);
	// Cannot test the return value of AdjustTokenPrivileges. 
		if ( GetLastError() != ERROR_SUCCESS ) {
			FreeLibrary(hModule);
			return FALSE;
		}
	}
	if ( hModule )
		FreeLibrary(hModule);
	return ExitWindowsEx(EWX_POWEROFF|EWX_FORCE, 0);
}
