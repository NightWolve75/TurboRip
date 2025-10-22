// Notes: Lib functions are dependent on InitConsole() to initialize I/O handles!

#ifdef _CONSOLE
// When in debug mode, our fast CRT is excluded, so init must be done pre-main/winmain somehow!
#ifdef _DEBUG
	int _InitConsole_ = InitConsole();
#endif
HANDLE ghConsoleInput, ghConsoleOutput; // These are initialized by InitConsole() in miniCRT startup!
HWND   ghWndConsole;

#endif
// Private Strings/Variables

#ifdef _CONSOLE

// Call ONCE at the start of a console-based program ( mainCRTStartup() )
// to initialize Input/Output handles and immediately center console window!

extern int __fastcall InitConsole() {
	COORD coordDest;
	MSG msg;
	__asm CLD // Clear direction flag
// 1) Get console handle for standard output
	ghConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	if ( ghConsoleOutput == INVALID_HANDLE_VALUE || ghConsoleOutput == 0 ) {
		ShowLastSysError("Allocating a Windows CMD Console failed");
		ExitProcess(0); // Give up!
	}
	FastPrintC(" \b \r");
	coordDest.X = coordDest.Y = 0; // Ensure cursor is back to 0,0
	SetConsoleCursorPosition(ghConsoleOutput, coordDest);
// 2) Try to center newly created or attached console window
	ghWndConsole = GetForegroundWindow(); // FIXED-Failed in debug env, gets MSVC++ IDE handle, heh!
	if ( ghWndConsole )
		CenterWindow(ghWndConsole);
// 3) Make sure CRT Heap is initialized.
	InitCRTHeap(); // Get our process heap for fast malloc/free memory functions
// 4) If we got standard output handle, we should be OK for input!
	ghConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
// 5) When app is GUI, Windows considers it "hung" for 5 seconds if Get/Peek-Message is
// not called in a standard message loop, so call it to avoid cursor in 'hourglass' mode
// In Win95, it seems to happen if app is run from existing Command window
	PeekMessageA(&msg, GetDesktopWindow(), 0, 0, PM_NOREMOVE);
	return 0;
}

#if !(defined(_DEBUG) || defined(_CRTVETO))

// Our faster (but limited) overloaded version of printf via user32.dll's wvsprintfA() WinAPI

extern "C" int __cdecl printf(LPCSTR format, ...) {
	char szBuffer[1024];
	va_list argptr;
	int nChars;
	va_start(argptr, format);
	nChars = wvsprintfA(szBuffer, format, argptr);
	WriteFile(ghConsoleOutput, szBuffer, nChars, &gnNumberOfBytes, 0);
	return nChars;
}

#endif

extern BOOL __fastcall YesNoPrompt(LPSTR lpText, DWORD nLength) {
	char c, bYes, szBuffer[4];
	DWORD dwMode;
// Get current cursor position
	GetConsoleMode(ghConsoleInput, &dwMode); // Save normal console mode
	SetConsoleMode(ghConsoleInput, ENABLE_PROCESSED_INPUT); // To allow only Ctrl+C
	FlushConsoleInputBuffer(ghConsoleInput); // Good to keep input buffer flushed
	FastNPrint(lpText, nLength);
// Attempt to read only digits from the prompt
	for (;;) {
		ReadConsoleA(ghConsoleInput, szBuffer, sizeof(szBuffer), &gnNumberOfBytes, NULL);
		c = *szBuffer;
		ToLowerCase(c);
		if ( (bYes = (c == 'y')) || c == 'n' ) {
			*(LPWORD)(szBuffer+1) = '\r\n';
			WriteConsoleA(ghConsoleOutput, szBuffer, 3, &gnNumberOfBytes, NULL);
			SetConsoleMode(ghConsoleInput, dwMode); // Restore previous console mode
			FlushConsoleInputBuffer(ghConsoleInput);
			return bYes;
		}
		Beep(750, 300);
		Sleep(0);
	}
}

extern char __fastcall AbortRetryIgnorePrompt() {
	char c, szBuffer[4];
	DWORD dwMode;
// Get current cursor position
	GetConsoleMode(ghConsoleInput, &dwMode); // Save normal console mode
	SetConsoleMode(ghConsoleInput, ENABLE_PROCESSED_INPUT); // To allow only Ctrl+C
	FastPrint("Press (A)bort, (R)etry, (I)gnore:> ");
	FlushConsoleInputBuffer(ghConsoleInput); // Good to keep input buffer flushed
// Attempt to read only digits from the prompt
	for (;;) {
		ReadConsoleA(ghConsoleInput, szBuffer, sizeof(szBuffer), &gnNumberOfBytes, NULL);
		c = *szBuffer;
		ToLowerCase(c);
		if ( c == 'a' || c == 'r' || c == 'i' ) {
			*(LPWORD)(szBuffer+1) = '\r\n';
			WriteConsoleA(ghConsoleOutput, szBuffer, 3, &gnNumberOfBytes, NULL);
			SetConsoleMode(ghConsoleInput, dwMode); // Restore previous console mode
			FlushConsoleInputBuffer(ghConsoleInput);
			return c;
		}
		Sleep(0);
	}
}

// Dependency: Call InitConsole() first to init ghConsoleInput handle!

extern long __fastcall ScanForNumber(BYTE nMaxDigits, BOOL bSigned) {
	char c, szBuffer[16], szNumber[16], *lpNumPtr; // Max: "4,294,967,295" + \0
	BYTE nDigits = 0, bNegate = FALSE;
	DWORD dwMode;
// Get current cursor position
	GetConsoleMode(ghConsoleInput, &dwMode); // Save normal console mode
	SetConsoleMode(ghConsoleInput, ENABLE_PROCESSED_INPUT); // To allow only Ctrl+C
	FlushConsoleInputBuffer(ghConsoleInput); // Good to keep input buffer flushed
	lpNumPtr = szNumber;
// Attempt to read only digits from the prompt
	for (;;) {
		ReadConsoleA(ghConsoleInput, szBuffer, sizeof(szBuffer), &gnNumberOfBytes, NULL);
		c = *szBuffer;
		if ( c == '\b' && (nDigits || bNegate) ) { // User hit backspace, so delete any digits if present
			*(LPWORD)(szBuffer+1) = '\b ';
			WriteConsoleA(ghConsoleOutput, szBuffer, 3, &gnNumberOfBytes, NULL);
			if ( nDigits ) {
				lpNumPtr--;
				nDigits--;
			} else if ( bNegate )
				bNegate = FALSE;
		} else if ( bSigned && c == '-' && !nDigits && !bNegate ) { // If 1st char is '-' negative sign, note it
			WriteConsoleA(ghConsoleOutput, szBuffer, 1, &gnNumberOfBytes, NULL);
			bNegate = TRUE;
		} else if ( c == '\r' && nDigits ) { // User hit enter, and if we have digit(s), quit loop
			break;
		} else if ( c < '0' || c > '9' ) { // Any other non-digit, ignore and scan for input again
			continue;
		} else { // c is between 0-9 given previous test, so save and echo it back to console
			nDigits++;
			*lpNumPtr++ = c;
			WriteConsoleA(ghConsoleOutput, szBuffer, 1, &gnNumberOfBytes, NULL);
			if ( nDigits == nMaxDigits || nDigits == 10 )
				break;
		}
		Sleep(0);
	}
	*lpNumPtr = 0;
	FastPrint(BRK);
	SetConsoleMode(ghConsoleInput, dwMode); // Restore previous console mode
	FlushConsoleInputBuffer(ghConsoleInput);
	dwMode = atol(szNumber); // Use atol to convert an ANSI string to number
	if ( bNegate )
		return -(long)dwMode;
	return dwMode;
}
// FUNCTION: cls(HANDLE hConsole)
// PURPOSE : Clear the screen by filling it with blanks, then home cursor
// INPUT   : The Console Buffer to clear
// Dependency: Call InitConsole() first to init ghConsoleInput handle!
extern void __fastcall cls() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD ConsoleSize;
	COORD ResetXY00;
	ResetXY00.X = ResetXY00.Y = 0;
	GetConsoleScreenBufferInfo(ghConsoleOutput, &csbi);
	ConsoleSize = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacterA(ghConsoleOutput, ' ', ConsoleSize, ResetXY00, &gnNumberOfBytes);
	FillConsoleOutputAttribute(ghConsoleOutput, csbi.wAttributes, ConsoleSize, ResetXY00, &gnNumberOfBytes);
	SetConsoleCursorPosition(ghConsoleOutput, ResetXY00);
	FlushConsoleInputBuffer(ghConsoleInput);
}
// Dependency: Call InitConsole() first to init ghConsoleInput/ghConsoleInput handles!
// Bug Report: Win98SE: If nNumberOfCharsToRead = 1, ReadConsole/ReadFile didn't work!
// Must use FlushConsoleInputBuffer, solved everything - universal for 9X/NT!
// This was very tricky to get to work across 9X/NT... Be mindful. Now 99% sure!
// FIXED FOR WIN95/98/ME and NT cores such as Vista!!
extern void __fastcall pause(DWORD dwMilliseconds) {
	char MSG_PAUSE_SHOW[] = "--- Press enter to continue --- ", sInput[2];
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwStart, dwMode;
	SetWindowTopMost(ghWndConsole);
	FastPrint(NEW_LINE);
// Use FillConsoleOutputAttribute() for text coloring which works on Win9X cores!
	GetConsoleScreenBufferInfo(ghConsoleOutput, &csbi);
	if ( gOSInfo.IsWinNT ) // Bug fix for Vista when Y buffer exceeds limit, use NT-stable SetConsoleTextAttribute()
		SetConsoleTextAttribute(ghConsoleOutput, TEXT_COLOR_YELLOW);
	else // For Win9X core: Precoloring only works on Win9X core!
		FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_YELLOW, sizeof(MSG_PAUSE_SHOW)-1, csbi.dwCursorPosition, &gnNumberOfBytes);
// Print pause message
	FastPrint(MSG_PAUSE_SHOW);
// NT Core: Must print first, then color with FillConsoleOutputAttribute()!
	FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_YELLOW, sizeof(MSG_PAUSE_SHOW)-1, csbi.dwCursorPosition, &gnNumberOfBytes);
	GetConsoleMode(ghConsoleInput, &dwMode); // Save regular console mode
	SetConsoleMode(ghConsoleInput, 0); // Go raw mode
	FlushConsoleInputBuffer(ghConsoleInput);
	Sleep(0);
	if ( dwMilliseconds == 0 )
		dwMilliseconds = INFINITE;
	dwStart = GetTickCount();
	WaitForSingleObject(ghConsoleInput, dwMilliseconds); // Wait for input event, flaky on 9X core!
// Win9X bug: WaitForSingleObject might quickly return, try timer, use ReadConsoleA as failsafe!
	if ( (GetTickCount() - dwStart) < 250 )
		ReadConsoleA(ghConsoleInput, sInput, 1, &gnNumberOfBytes, NULL); // Normally works, IF, input handle is flushed!
	SetConsoleCursorPosition(ghConsoleOutput, csbi.dwCursorPosition);
// Blank out pause message
	FillConsoleOutputCharacterA(ghConsoleOutput, ' ', sizeof(MSG_PAUSE_SHOW)+4, csbi.dwCursorPosition, &gnNumberOfBytes);
// Restore previous/default gray text color (usually)
	FillConsoleOutputAttribute(ghConsoleOutput, csbi.wAttributes, sizeof(MSG_PAUSE_SHOW)+4, csbi.dwCursorPosition, &gnNumberOfBytes);
	if ( gOSInfo.IsWinNT ) // More guaranteed attempt to restore previous color for NT core
		SetConsoleTextAttribute(ghConsoleOutput, csbi.wAttributes);
	SetConsoleMode(ghConsoleInput, dwMode); // Restore previous console mode
	UNDO_TOPMOST_CONSOLE();
}

extern void __fastcall DeleteLine() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(ghConsoleOutput, &csbi);
	csbi.dwCursorPosition.X = 0;
	FillConsoleOutputCharacterA(ghConsoleOutput, ' ', csbi.dwSize.X, csbi.dwCursorPosition, &gnNumberOfBytes);
	FastPrintC("\r");
}

extern void __fastcall RollConsoleBufferOver() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	SMALL_RECT MaxViewSize;
	CHAR_INFO *lpBuffer;
	COORD ResetXY;
	ResetXY.X = ResetXY.Y = 0;
	GetConsoleScreenBufferInfo(ghConsoleOutput, &csbi);
	MaxViewSize = csbi.srWindow;
	MaxViewSize.Bottom -= MaxViewSize.Top;
	MaxViewSize.Left = MaxViewSize.Top = 0;
	lpBuffer = (CHAR_INFO*)fmalloc(csbi.dwMaximumWindowSize.Y * csbi.dwMaximumWindowSize.X * sizeof(CHAR_INFO));
	if ( !lpBuffer )
		return;
	ReadConsoleOutputA(ghConsoleOutput, lpBuffer, csbi.dwMaximumWindowSize, ResetXY, &csbi.srWindow);
	cls();
	WriteConsoleOutputA(ghConsoleOutput, lpBuffer, csbi.dwMaximumWindowSize, ResetXY, &MaxViewSize);
	ffree(lpBuffer);
	ResetXY.Y = MaxViewSize.Bottom + 1;
	SetConsoleCursorPosition(ghConsoleOutput, ResetXY);
}

// Harder way to correctly print colored text bug-free on Windows 95/98/ME
// as well as WinNT/2K/XP/Vista/7/8/10!! Almost 99% guaranteed as of late!
// SetConsoleTextAttribute() is buggy in 9X core, rely on FillConsoleOutputAttribute()
// Conversely, SetConsoleTextAttribute() is more stable on NT core (Vista) so
// with gbIsWinNT (OS detection), it can be used to guarantee color changes!
// NT Core bugs: When screen buffer is pretty full, csbi.dwCursorPosition can no
// longer reposition cursor, let alone color text with FillConsoleOutputAttribute()
// Solution is to rely on SetConsoleTextAttribute() which is all buggy in Win9X.

// BUG: Win2000 bug, CTRL + BREAK & pause() miscolor each other!!! 

void __fastcall PrintColoredText(LPCSTR lpText, DWORD nLength, WORD wColor) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD nLineBlocks;
	WORD  wPrevColor;
	short Lines;
// Print out any starting CRs/LFs
	for ( ; *lpText == '\r' || *lpText == '\n'; lpText++, nLength-- )
		WriteFile(ghConsoleOutput, lpText, 1, &gnNumberOfBytes, 0);
// Get cursor position/console info
	if ( !GetConsoleScreenBufferInfo(ghConsoleOutput, &csbi) ) { // Abort and print regular if this fails
		WriteFile(ghConsoleOutput, lpText, nLength, &gnNumberOfBytes, 0); // If handle redirected to file for logging!
		return;
	}
// Count the lines to print, compute blocks (lines * typical text width of console[80])
	Lines = (short)chrCount((PSTR)lpText, '\n') + 1;
	if ( *(LPWORD)(lpText+nLength-1) == '\n\0' )
		Lines--;
	nLineBlocks = Lines * csbi.dwSize.X;
	wPrevColor = csbi.wAttributes; // Save current text color
	if ( gOSInfo.IsWinNT ) // Bug fix for Vista, use NT-stable SetConsoleTextAttribute()
		SetConsoleTextAttribute(ghConsoleOutput, wColor);
	else // For Win9X core: Precoloring only works on Win9X core without bugs using FillConsoleOutputAttribute()!
		FillConsoleOutputAttribute(ghConsoleOutput, wColor, nLineBlocks, csbi.dwCursorPosition, &gnNumberOfBytes);
// Print colored text (change to WriteFile() if HANDLE is redirected for logging!)
	WriteFile(ghConsoleOutput, lpText, nLength, &gnNumberOfBytes, 0);
// Trap NT bug when buffer is full and Y keeps returning max lines
	if ( gOSInfo.IsWinNT && csbi.dwCursorPosition.Y + 1 >= csbi.dwSize.Y )
		csbi.dwCursorPosition.Y -= Lines;
// NT Core: Must color AFTER printing text! "Usually" works on both 9X/NT cores though!
	FillConsoleOutputAttribute(ghConsoleOutput, wColor, nLineBlocks, csbi.dwCursorPosition, &gnNumberOfBytes);
// Get new cursor position and restore to default text color
	GetConsoleScreenBufferInfo(ghConsoleOutput, &csbi);
	FillConsoleOutputAttribute(ghConsoleOutput, wPrevColor, nLineBlocks, csbi.dwCursorPosition, &gnNumberOfBytes);
	if ( gOSInfo.IsWinNT ) // Use only on NT core - guaranteed miscoloring bugs if used on 9X!
		SetConsoleTextAttribute(ghConsoleOutput, wPrevColor);
}

#else

#if !(defined(_DEBUG) || defined(_CRTVETO))

EXTERN int __cdecl printf(const char *_Format, ...) {
	char szBuffer[1024];
	va_list argptr;
	int nChars;
	va_start(argptr, _Format);
	nChars = wvsprintfA(szBuffer, _Format, argptr);
	MessageBoxA(GetForegroundWindow(), szBuffer, "printf", MB_OK);
	return nChars;
}

#endif

#endif // End #ifdef _CONSOLE

DWORD __cdecl PrintLastSysError(const char *_Format, ...) {
	char szMsgBuf[1024], *lpMessage;
	DWORD dwErrorCode, nLength;
	va_list argptr;
// Get last error code if reported
	dwErrorCode = GetLastError();
	#ifdef _CONSOLE
		while ( *_Format == '\r' || *_Format == '\n' )
			WriteFile(ghConsoleOutput, _Format++, 1, &gnNumberOfBytes, 0);
	#endif
// Handle user-defined error message first
	va_start(argptr, _Format); 
	nLength = wvsprintfA(szMsgBuf, _Format, argptr);
	if ( dwErrorCode && nLength < 640 ) {
		lpMessage = (szMsgBuf+nLength);
		*(LPDWORD)lpMessage = '  : ';
	// Load Windows system error if reported 
		nLength += FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL, dwErrorCode, 0, // Default language
			lpMessage+3, (sizeof(szMsgBuf)-3)-nLength, NULL
		) + 4;
	// Wraps string for console proper (~74 char-per-line is good for Win98SE)
		#ifdef _CONSOLE
			nLength = FormatMessageA(
				FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_IGNORE_INSERTS | 74,
				(LPSTR)szMsgBuf, NULL, 0, // Default language
				szMsgBuf, sizeof(szMsgBuf), NULL
			);
		#endif
	}
	*(LPDWORD)(szMsgBuf+nLength) = '\r\n\0\0';
	#ifdef _CONSOLE
	// Print error in nice red color for user to get noticed! ;)
		PrintColoredText(szMsgBuf, nLength+2, TEXT_COLOR_RED);
	#else
		MessageBoxA(GetForegroundWindow(), szMsgBuf, "Error", MB_OK | MB_ICONERROR);
	#endif
	SetLastError(0);
	return dwErrorCode;
}
