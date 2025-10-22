typedef HANDLE (WINAPI *PROC_OPENTHREAD)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId);
typedef HANDLE (WINAPI *PROC_CREATETOOLHELP32SNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL (WINAPI *PROC_PROCESS32FIRST)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL (WINAPI *PROC_PROCESS32NEXT) (HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL (WINAPI *PROC_THREAD32FIRST)(HANDLE hSnapshot, LPTHREADENTRY32 lpte);
typedef BOOL (WINAPI *PROC_THREAD32NEXT) (HANDLE hSnapshot, LPTHREADENTRY32 lpte);

PROC_CREATETOOLHELP32SNAPSHOT glpfnCreateToolhelp32Snapshot;
PROC_PROCESS32FIRST glpfnProcess32First;
PROC_PROCESS32NEXT  glpfnProcess32Next;
PROC_THREAD32FIRST  glpfnThread32First;
PROC_THREAD32NEXT   glpfnThread32Next;
HMODULE ghKernel32Module;

EXTERN BOOL __fastcall WinExecute(PSTR lpExeName, PSTR lpExeDirectory, PSTR lpCommandLine, BOOL bWaitForExit, BOOL bHide) {
	char szPathFile[MAX_PATH], szCurDir[MAX_PATH];
	BOOL bResult, bCurDir;
	PROCESS_INFORMATION pi;
	STARTUPINFOA sinfo;
	PSTR lpString;
// Optimized for __fastcall, expect param2 in EDX, do NOT change calling convention!!
	__asm {
	// If lpCurrentDirectory isn't null, use it
		TEST  EDX, EDX
		JZ    LBL_SKIP
		MOVZX EAX,[EDX]
		TEST  EAX, EAX
		JZ    LBL_SKIP
		// Copy lpExeDirectory string to local variable
			LEA  ECX, szPathFile
			CALL StrCpy
		// Add final backslash if missing
			TEST BYTE PTR [EAX-1], '\\'
			JE   HAS_BACKSLASH
				MOV BYTE PTR [EAX], '\\'
				ADD EAX, 1
		HAS_BACKSLASH:
			MOV  lpString, EAX
		// Save the current dir to restore later
			LEA  ECX, szCurDir
			PUSH ECX
			PUSH MAX_PATH
			CALL DWORD PTR [GetCurrentDirectoryA]
		// Set dir to same where exe resides
			LEA  ECX, szPathFile
			PUSH ECX
			CALL DWORD PTR [SetCurrentDirectoryA]
		// Append exe to its dir, make full pathfile reference
			MOV  ECX, lpString
			MOV  EDX, lpExeName
			CALL StrCpy
			LEA  ECX, szPathFile
			MOV  [lpExeName], ECX
		LBL_SKIP:
	}
#ifndef _WIN64
	mMemZeroBy4_(sinfo, SIZE STARTUPINFOA/4);
#else
	ZeroMemory(&sinfo, sizeof(STARTUPINFOA));
#endif
	sinfo.cb = sizeof(STARTUPINFOA);
	if ( bHide ) {
		sinfo.dwFlags = STARTF_USESHOWWINDOW;
		sinfo.wShowWindow = SW_HIDE;
	}
	bResult = CreateProcessA(lpExeName, 0, 0, 0, FALSE, CREATE_DEFAULT_ERROR_MODE, 0, lpExeDirectory, &sinfo, &pi);
	if ( bCurDir )
		SetCurrentDirectoryA(szCurDir); // Restore previous directory to minimize disruptions
	if ( bResult ) {
		Sleep(50);
		if ( bWaitForExit )
			WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return bResult;
}

EXTERN BOOL __stdcall DDraw256WinExec(PSTR lpExeName) {
	PROCESS_INFORMATION pi;
	STARTUPINFOA sinfo;
	BOOL bResult;
	#ifndef _WIN64
		__asm {
			LEA   EDI, sinfo
			XOR   EAX, EAX
			MOV   ECX, SIZE STARTUPINFOA/4
			REP   STOSD
			LEA   EDI, pi
			MOV   ECX, SIZE PROCESS_INFORMATION/4
			REP   STOSD
		}
	#else
		ZeroMemory(&sinfo, sizeof(STARTUPINFOA));
		ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	#endif
	sinfo.cb = sizeof(STARTUPINFOA);
	bResult = CreateProcessA(0, lpExeName, 0, 0, FALSE, CREATE_DEFAULT_ERROR_MODE, 0, 0, &sinfo, &pi);
	if ( bResult ) {
		WaitForSingleObject(pi.hProcess, 250);
		SuspendResumeProcess(gExplorer, FALSE);
		WaitForInputIdle(pi.hProcess, 2500);
		WaitForSingleObject(pi.hProcess, INFINITE); // Wait until child process exits.
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		Sleep(25);
		SuspendResumeProcess(gExplorer, TRUE);
	}
	return bResult;
}

// KillProcess Compatibility: All except NT4
// Will simply return on NT4, no errors
// Compiles with NT4 API static linkage

EXTERN DWORD __stdcall KillProcess(PSTR lpExeName) {
	HANDLE hProcess, hSnapShot;
	PROCESSENTRY32 pe32;
	DWORD Count;
	if ( !glpfnCreateToolhelp32Snapshot ) {
		ghKernel32Module = GetModuleHandleA(KERNEL32_DLL);
	// Dynamic linking to pass WinNT4 static check
		glpfnCreateToolhelp32Snapshot = (PROC_CREATETOOLHELP32SNAPSHOT)GetProcAddress(ghKernel32Module, "CreateToolhelp32Snapshot");
		if ( !glpfnCreateToolhelp32Snapshot )
			return 0;
		glpfnProcess32First = (PROC_PROCESS32FIRST)GetProcAddress(ghKernel32Module, "Process32First");
		glpfnProcess32Next  = (PROC_PROCESS32NEXT) GetProcAddress(ghKernel32Module, "Process32Next");
	}
	Count = 0;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	hSnapShot = glpfnCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if ( glpfnProcess32First(hSnapShot, &pe32) )
		do {
			if ( !StrCmpI(pe32.szExeFile, lpExeName) ) {
				hProcess = OpenProcess(PROCESS_TERMINATE|PROCESS_QUERY_INFORMATION, 0, pe32.th32ProcessID);
				if ( hProcess ) {
				// Passing 1 instead of 0 ensured better forced termination. I disassembled
				// taskkill, saw 1 used by Microsoft, was effective to keep explorer.exe killed.
					TerminateProcess(hProcess, 1);
					CloseHandle(hProcess);
					Sleep(0);
				}
				Count++;
			}
		} while( glpfnProcess32Next(hSnapShot, &pe32) );
	CloseHandle(hSnapShot);
	return Count;
}

// SuspendResumeProcess Compatibility: >=WinMe/2000/XP/Vista/7+
// Will simply return on NT4, no effect/errors!
// Compiles on NT4 API without static linkage

EXTERN void __stdcall SuspendResumeProcess(PSTR lpExeName, BOOL bResume) {
	static PROC_OPENTHREAD lpfnOpenThread;
	HANDLE hThread, hSnapShot, hThreadSnap;
	PROCESSENTRY32 pe32;
	THREADENTRY32 te32;
	if ( !lpfnOpenThread ) {
		ghKernel32Module = GetModuleHandleA(KERNEL32_DLL);
		glpfnCreateToolhelp32Snapshot = (PROC_CREATETOOLHELP32SNAPSHOT)GetProcAddress(ghKernel32Module, "CreateToolhelp32Snapshot");
		if ( !glpfnCreateToolhelp32Snapshot )
			return;
	// OpenThread() is a WinMe or greater function 
		lpfnOpenThread = (PROC_OPENTHREAD)GetProcAddress(ghKernel32Module, "OpenThread");
		if ( !lpfnOpenThread )
			return;
		glpfnProcess32First = (PROC_PROCESS32FIRST)GetProcAddress(ghKernel32Module, "Process32First");
		glpfnProcess32Next = (PROC_PROCESS32NEXT)GetProcAddress(ghKernel32Module, "Process32Next");
		glpfnThread32First = (PROC_THREAD32FIRST)GetProcAddress(ghKernel32Module, "Thread32First");
		glpfnThread32Next = (PROC_THREAD32NEXT)GetProcAddress(ghKernel32Module, "Thread32Next");
	}
	hSnapShot = glpfnCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	hThreadSnap = glpfnCreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	te32.dwSize = sizeof(THREADENTRY32);
	if ( glpfnProcess32First(hSnapShot, &pe32) )
		do {
			if ( !StrCmpI(pe32.szExeFile, lpExeName) ) {
				if ( glpfnThread32First(hThreadSnap, &te32) )
					do {
						if ( te32.th32OwnerProcessID == pe32.th32ProcessID ) {
							hThread = lpfnOpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
							if ( !hThread ) // Fails on access denied (e.g. system process)
								break;
							if ( bResume ) // Call ResumeThread until Suspend Count = 0
								while ( (long)(ResumeThread(hThread)) > 0 ) ; // Make sure we resume 100%!
							else
								SuspendThread(hThread);
							CloseHandle(hThread);
						}
					} while ( glpfnThread32Next(hThreadSnap, &te32) );
			}
		} while( glpfnProcess32Next(hSnapShot, &pe32) );
	CloseHandle(hSnapShot);
	CloseHandle(hThreadSnap);
}

// Don't work for NT4, but compiles, no static linkage
// Return values: 0 for failure, 1 for process running, 2 for not running

EXTERN DWORD __stdcall IsProcessRunning(PSTR lpExeName) {
	HANDLE hSnapShot;
	PROCESSENTRY32 pe32;
	DWORD Count;
	if ( !glpfnCreateToolhelp32Snapshot ) {
		ghKernel32Module = GetModuleHandleA(KERNEL32_DLL);
	// Dynamic linking to pass WinNT4 static check
		glpfnCreateToolhelp32Snapshot = (PROC_CREATETOOLHELP32SNAPSHOT)GetProcAddress(ghKernel32Module, "CreateToolhelp32Snapshot");
		if ( !glpfnCreateToolhelp32Snapshot )
			return 0;
		glpfnProcess32First = (PROC_PROCESS32FIRST)GetProcAddress(ghKernel32Module, "Process32First");
		glpfnProcess32Next  = (PROC_PROCESS32NEXT) GetProcAddress(ghKernel32Module, "Process32Next");
	}
	Count = 0;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	hSnapShot = glpfnCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if ( glpfnProcess32First(hSnapShot, &pe32) )
		do {
			if ( !StrCmpI(pe32.szExeFile, lpExeName) )
				return 1;
		} while( glpfnProcess32Next(hSnapShot, &pe32) );
	CloseHandle(hSnapShot);
	return 2;
}