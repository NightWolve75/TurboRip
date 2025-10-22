// 1) _CRT_SECURE_NO_WARNINGS - Add to project settings Preprocessor option
// Hmm, sometimes exe can be smaller WITHOUT the line below... Try with and without!
// #pragma comment(linker, "/MERGE:.data=.text /MERGE:.rdata=.text /SECTION:.text,EWR")
// Or Copy & Paste into Linker CommandLine Option box for 'Release' Settings'
// /MERGE:.data=.text /MERGE:.rdata=.text /SECTION:.text,ERW /IGNORE:4254 /IGNORE:4210
// Disabling these linker warnings only works in Linker settings, not with #pragma
// Remember: ALIGN < 0x1000 (.e.g. 512) option breaks the EXE in Win95/98! Don't use!
#pragma once
#pragma warning(error:4002)
#pragma warning(error:4003)
#pragma warning(disable:4100 4254 4725)
// 4996: 'function': was declared deprecated
// 4311-4312: type cast conversion truncates or enlarges
//#pragma warning(disable: 4244 4254 4311 4312 4313 4996)
#ifndef _GLOBAL_H_LIB_
#define _GLOBAL_H_LIB_
#ifdef __cplusplus
	#define NAKED_EXTERN extern "C" __declspec(naked)
	#define EXTERN extern "C"
#else
	#define NAKED_EXTERN extern __declspec(naked)
	#define EXTERN extern
#endif
#undef _DLL
#undef  UNICODE // Make sure Unicode is not defined
#undef _UNICODE
#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers
#undef  WINVER
#undef _WIN32_WINNT
#ifndef _WIN64
#define WINVER         0x0400 // Both Windows 95/NT
#define _WIN32_WINNT   0x0400 // Windows NT 4.0 - Avoid new APIs to support Win9X platform
#define _WIN32_WINDOWS 0x0400 // Windows 95
#include <windows.h>
#else
#define _AMD64_
#define WINVER  0x0602 // Windows 8 Level
#define _WIN32_WINNT 0x0602
#include "C:\Program Files (x86)\Windows Kits\8.1\Include\um\windows.h"
#endif
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>
#pragma intrinsic(strcat, strlen, strcpy, strcmp, memset, memcpy, memcmp)

// ******** Structures ********
typedef struct _OSINFO {
	BOOLEAN IsWin9X;
	BOOLEAN IsWin95;
	BOOLEAN IsWin98;
	BOOLEAN IsWinME;
	BOOLEAN IsWinNT;
	BOOLEAN IsWin7;
	BOOLEAN IsServer;
	BOOLEAN Is64bit;
	BYTE    PlatformID;
	BYTE    MajorVersion;
	BYTE    MinorVersion;
	BYTE    ServicePackMajor;
	char    ExeDir[MAX_PATH];
} OSInfo;

typedef struct _CPUINFO {
	SYSTEM_INFO siSysInfo;
	char     CPUString[0x20];
	char     CPUBrandString[0x40];
	BOOLEAN  MMX;
	BOOLEAN  SSE1;
	BOOLEAN  SSE2;
	BOOLEAN  SSE3;
} CPUInfo;

typedef struct _MAPPEDFILE {
	PBYTE  lpMapAddress;
	DWORD  dwFileSize;
	HANDLE hMapFile;
	HANDLE hFile;
} MAPPEDFILE;

typedef struct _FILETIMES {
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
} FILETIMES, *LPFILETIMES;

#pragma pack(1)

// Assumes ONLY a CD PCM WAVE Header, they can be bigger for float/other formats!

typedef struct _WAVE_FILE_HEADER {
// RIFF
	DWORD	riffID;				// Contains identifier "RIFF"
	DWORD	riffSIZE;			// File size minus 8 bytes
	DWORD	riffFORMAT;			// Contains identifier "WAVE"
// FMT
	DWORD	fmtID;				// Contains identifier: "fmt " (with space)
	DWORD	fmtSIZE;			// Contains the size of this block (for WAVE PCM = 16)
// WAVEFORM
	WORD	wFormatTag;			// Format of digital sound
	WORD	nChannels;			// Number of channels (1 for mono and 2 for stereo)
	DWORD	nSamplesPerSec;		// Number of samples per second
	DWORD	nAvgBytesPerSec;	// Average number bytes of data per second
	WORD	nBlockAlign;		// Minimal data size for playing
	WORD	wBitsPerSample;		// Bits per sample (8 or 16)
// DATA
	DWORD	dataID;				// Contains identifier: "data"
	DWORD	dataSIZE;			// Data size (File size minus 44, unless trailing bytes or EX header)
} WAVE_FILE_HEADER;
/*
 *  Extended waveform format structure used for all non-PCM formats.
 *  This structure is common to all non-PCM formats.
 */
typedef struct _WAVE_FORMAT_EX {
	WORD	wFormatTag;         /* format type */
	WORD	nChannels;          /* number of channels (i.e. mono, stereo...) */
	DWORD	nSamplesPerSec;     /* sample rate */
	DWORD	nAvgBytesPerSec;    /* for buffer estimation */
	WORD	nBlockAlign;        /* block size of data */
	WORD	wBitsPerSample;     /* number of bits per sample of mono data */
	WORD	cbSize;             /* the count in bytes of the size of extra info (after cbSize) - APE sets to 0! */
} WAVE_FORMAT_EX;

#pragma warning(disable:4201)
typedef struct _DEVMODEA95 {
    BYTE dmDeviceName[CCHDEVICENAME];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    union {
      /* printer only fields */
      struct {
        short dmOrientation;
        short dmPaperSize;
        short dmPaperLength;
        short dmPaperWidth;
        short dmScale;
        short dmCopies;
        short dmDefaultSource;
        short dmPrintQuality;
      };
      /* display only fields */
      struct {
        POINTL dmPosition;
        DWORD  dmDisplayOrientation;
        DWORD  dmDisplayFixedOutput; 
      };
    };
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    BYTE  dmFormName[CCHFORMNAME];
    WORD  dmLogPixels;
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
    union {
        DWORD  dmDisplayFlags;
        DWORD  dmNup;
    };
    DWORD  dmDisplayFrequency;
} DEVMODEA95;
#pragma warning(default:4201)

#pragma pack()

// Window/Console related variables
EXTERN HANDLE  ghConsoleInput, ghConsoleOutput;
EXTERN HWND    ghWndConsole;
EXTERN CPUInfo gCPUInfo;
EXTERN OSInfo  gOSInfo;
// Memory Alloc
EXTERN HANDLE  ghCRTHeap;
// General global use
EXTERN DWORD   gnNumberOfBytes, glpAddress, gCounter, gSemaphore;
EXTERN char    gRegistryDLL[], gKernel32DLL[], gUser32DLL[], gGDI32DLL[], gComdlg32DLL[];
EXTERN char    gExplorer[];
EXTERN char    gError[];
EXTERN char    gNull[];
EXTERN PSTR    MSG_YES_NO[];

// Math
EXTERN int _fltused;

// ******** Defines ********

#define BRK       "\r\n"
#define NEW_LINE  "\r\n"
#define KERNEL32     "kernel32"
#define KERNEL32_    "KERNEL32"
#define KERNEL32_DLL "kernel32.dll"
#define ERROR_FILE_OPEN      "Error: Can't open \"%s\""
#define ERROR_FILE_CREATE    "Error: Can't create new file for writing"
#define ERROR_FILE_READ      "Error: Read operation failed"
#define ERROR_FILE_READOP    "Error: Read operation on \"%s\" failed"
#define ERROR_FILE_WRITE     "Error: Write operation failed"
#define ERROR_FILE_WRITEOP   "Error: Write operation on \"%s\" failed"
#define ERROR_FILE_MAPOPEN   "Error: Can't open \"%s\" for mapping"
#define ERROR_FILE_MAPCREATE "Error: CreateFileMappingA() failed"
#define ERROR_FILE_MAPVIEW   "Error: MapViewOfFile() failed"
#define ERROR_MEM_ALLOC      "Error: Memory allocation failure"

// Win32 Sys/FileTimes
// File time is a 64-bit value that represents the number of 100-nanosecond intervals
// elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal Time (UTC). 
#define UTC_1HOUR ((ULONGLONG)10000000 * 60 * 60)      //  1 Hour in System Time
#define UTC_1DAY  ((ULONGLONG)10000000 * 60 * 60 * 24) // 24 Hours in System Time

//__asm {FLDPI; FSTP x;} - Load PI in x86 ASM
#define PI    3.1415927f
#define M_PI  3.1415927f
#define Q_PI  3.1415926535897931

// Registry
// Prepend HKEY_CLASSES_ROOT/HKEY_CURRENT_USER/HKEY_LOCAL_MACHINE codes
#define HKCR_ "\x00"
#define HKCU_ "\x01"
#define HKLM_ "\x02"

// ******** MACROS ********

// Add/Sub/MakePtr are macros that allows you to easily add to values
// (including pointers) together without dealing with C's pointer
// arithmetic. It treats the last two parameters as DWORDs.
#define AddPtr(cast, ptr, addValue) (cast)((ULONG_PTR)(ptr) + (ULONG_PTR)(addValue))
#define SubPtr(cast, ptr, subValue) (cast)((ULONG_PTR)(ptr) - (ULONG_PTR)(subValue))
// Help with floats and int-only user32.wsprintf proc
#define CDbl(value, multiplier) (long)((value - (long)value) * multiplier) 
#undef  IS_INTRESOURCE
#define IS_INTRESOURCE(_r) (((DWORD)(_r) >> 16) == 0)
#undef  MAKEINTRESOURCE
#define MAKEINTRESOURCE(i) (PSTR)((DWORD)((WORD)(i)))
#define FILE_EXISTS(szFileName) (GetFileAttributesA(szFileName) != (DWORD)-1)
#define sprintf wsprintfA
// Remember to use static variables with NAKED if need be or via ESP
#define NAKED        __declspec(naked)
#define ALIGN_16     __declspec(align(16))
#define DLLEXPORT(RETURNTYPE) EXTERN __declspec(dllexport) RETURNTYPE __stdcall 

// Catch a struct alignment error!!! e.g. TOC_FORMAT_00 MUST equal 804 bytes!!!!
#define STRUCT_PROTECT(StructName, ProperSize, ID) char StructProtect##ID[1 - 2*(sizeof(StructName)!=ProperSize)]
#define SIZE_PROTECT(VariableName, ProperSize, ID) char SizeProtect##ID[1 - 2*(sizeof(VariableName)!=ProperSize)]
#define MAXSIZE_PROTECT(VariableName, MaxSize, ID) char MaxSizeProtect##ID[1 - 2*(sizeof(VariableName)>MaxSize)]

#define TIMER_INIT() DWORD rdtscBefore, rdtscAfter, rdtscTotal; SetHighPriority();
#define TIMER_START() { __asm LFENCE __asm XOR EAX, EAX __asm CPUID __asm XOR EAX, EAX __asm CPUID __asm XOR EAX, EAX __asm CPUID __asm XOR EAX, EAX __asm XOR EDX, EDX __asm RDTSC __asm MOV DWORD PTR [rdtscBefore], EAX };
#define TIMER_STOP()  {{__asm _emit 0x0F __asm _emit 0x01 __asm _emit 0xF9 __asm MOV DWORD PTR [rdtscAfter], EAX __asm LFENCE __asm CPUID}; rdtscTotal = rdtscAfter - rdtscBefore; printf("CPU Cycles: %lu\r\n", rdtscTotal); };

#define SET_FILE_CURRENT_TIME(hFile) { \
	SYSTEMTIME st; FILETIME ft; \
	GetSystemTime(&st); \
	SystemTimeToFileTime(&st, &ft); \
	SetFileTime(hFile, NULL, NULL, &ft);}

// Important NOTES: All fast ASM __inline defines need {} enclosing if only
// one used under branch condition, e.g. if (i==1) {fmemcpy(dest,src,count);}

#define mMemWChr(dwSize, lpMem, WChar)\
	__asm MOV   ECX, dwSize \
	__asm SHR   ECX, 1 \
	__asm JZ    $+38 \
	__asm XOR   EAX, EAX \
	__asm MOV   EDI, lpMem \
	__asm MOV   AX, WChar \
	__asm REPNE SCASW \
	__asm JE    $+14 \
		__asm MOV   lpMem, ECX \
		__asm JMP   $+11 \
	__asm SUB   EDI, 2 \
	__asm MOV   lpMem, EDI

// Quick fast version if buffer is evenly DWORD sized. You can set dwBytesBy4 equal
// to say "SIZE structobject.member/4" and the result will be hardcoded by the assembler
// DON'T FORGET TO DIVIDE BY 4!!! e.g. SIZE name/4
#define mMemZeroBy4(lpBuffer, LengthBy4)\
	__asm MOV   EDI, lpBuffer \
	__asm XOR   EAX, EAX \
	__asm MOV   ECX, LengthBy4 \
	__asm REP   STOSD
#define mMemZeroBy4_(aBuffer, LengthBy4)\
	__asm LEA   EDI, aBuffer \
	__asm XOR   EAX, EAX \
	__asm MOV   ECX, LengthBy4 \
	__asm REP   STOSD
// Use SIZE asm operand if need be for count: e.g. SIZE char_array
#define mStrNCpy32(pDestination, pSource, Count)\
	__asm MOV   EDI, pDestination \
	__asm MOV   ECX, Count \
	__asm MOV   ESI, pSource \
	__asm MOVZX EAX, CL \
	__asm SHR   ECX, 2 \
	__asm AND   EAX, 3 \
	__asm REP   MOVSD \
	__asm MOV   ECX, EAX \
	__asm REP   MOVSB

#define mMemCpy8(pDestination, pSource, Count)\
	__asm MOV  ECX, Count \
	__asm MOV  ESI, pSource \
	__asm MOV  EDI, pDestination \
	__asm REP  MOVSB

#define mMemCpy32(pDestination, pSource, CountBy4)\
	__asm MOV   ECX, CountBy4 \
	__asm MOV   ESI, pSource \
	__asm MOV   EDI, pDestination \
	__asm REP   MOVSD \
	
#define RETURN32EAX(Variable, FunctionCall) { FunctionCall; __asm MOV Variable, EAX }
#define ASM_FLOAT_TO_LONG(FloatVar, IntVar)   { __asm FLD DWORD PTR FloatVar  __asm FISTP DWORD PTR IntVar } // Only works if FPU set to truncation mode
#define ASM_DOUBLE_TO_LONG(DoubleVar, IntVar) { __asm FLD QWORD PTR DoubleVar __asm FISTP DWORD PTR IntVar } // Only works if FPU set to truncation mode
#define ASM_FABS(FloatVar, RetVar) { __asm FLD FloatVar __asm FABS __asm FSTP RetVar }
#define ASM_EXT_APPEND(szFileExt) { __asm MOV ECX, DWORD PTR [szFileExt] __asm MOV [EAX-4], ECX }

// Memory Allocation/Deallocation Routines
// Dependent/Complements "Malloc.cpp"
// Note: For huge memory needs (GB Range), VirtualAlloc is what's needed, NOT these!
#define InitCRTHeap() if ( !ghCRTHeap ) ghCRTHeap = GetProcessHeap()
#define fMalloc(lpMem, dwBytes) {HeapAlloc(ghCRTHeap, 0, (dwBytes)); __asm MOV lpMem, EAX}
#define fmalloc(dwBytes) (PBYTE)HeapAlloc(ghCRTHeap, 0, (DWORD)(dwBytes))
#define fCalloc(lpMem, dwBytes) {HeapAlloc(ghCRTHeap, HEAP_ZERO_MEMORY, (dwBytes)); __asm MOV lpMem, EAX}
#define fcalloc(dwBytes) HeapAlloc(ghCRTHeap, HEAP_ZERO_MEMORY, (dwBytes))
#define frealloc(lpMem, dwBytes) HeapReAlloc(ghCRTHeap, 0, (LPVOID)(lpMem), (dwBytes))
#define ffree(lpMem) HeapFree(ghCRTHeap, 0, (LPVOID)(lpMem))
#define _aligned_malloc _Aligned_Malloc
#define _aligned_free   _Aligned_Free

// Win9X Bug: SetThreadPriority(,THREAD_PRIORITY_HIGHEST) crashed when reading Console input from user!!
// NOTE: ONLY use SetHighPriority() for all Windows builds, it's good enough!!!!
#define SetHighPriority() SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)

// Win9X Bug: SetConsoleTextAttribute() is always buggy, use FillConsoleOutputAttribute()!

#pragma warning(disable:4480)
#ifdef __cplusplus
	enum eCONSOLE_COLORS : unsigned short {
#else
	enum eCONSOLE_COLORS {
#endif
	TEXT_COLOR_DARKBLUE    = FOREGROUND_BLUE,
	TEXT_COLOR_DARKGREEN   = FOREGROUND_GREEN,
	TEXT_COLOR_DARKCYAN    = FOREGROUND_GREEN | FOREGROUND_BLUE,
	TEXT_COLOR_DARKRED     = FOREGROUND_RED,
	TEXT_COLOR_DARKMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
	TEXT_COLOR_DARKYELLOW  = FOREGROUND_RED | FOREGROUND_GREEN,
	TEXT_COLOR_DARKGRAY    = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,

	TEXT_COLOR_GRAY        = FOREGROUND_INTENSITY,
	TEXT_COLOR_BLUE        = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
	TEXT_COLOR_GREEN       = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
	TEXT_COLOR_CYAN        = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
	TEXT_COLOR_RED         = FOREGROUND_INTENSITY | FOREGROUND_RED,
	TEXT_COLOR_MAGENTA     = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
	TEXT_COLOR_YELLOW      = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
	TEXT_COLOR_WHITE       = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,

	BACK_COLOR_DARKBLUE    = BACKGROUND_BLUE,
	BACK_COLOR_DARKGREEN   = BACKGROUND_GREEN,
	BACK_COLOR_DARKCYAN    = BACKGROUND_GREEN | BACKGROUND_BLUE,
	BACK_COLOR_DARKRED     = BACKGROUND_RED,
	BACK_COLOR_DARKMAGENTA = BACKGROUND_RED | BACKGROUND_BLUE,
	BACK_COLOR_DARKYELLOW  = BACKGROUND_RED | BACKGROUND_GREEN,
	BACK_COLOR_GRAY        = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,

	BACK_COLOR_DARKGRAY    = BACKGROUND_INTENSITY,
	BACK_COLOR_BLUE        = BACKGROUND_INTENSITY | BACKGROUND_BLUE,
	BACK_COLOR_GREEN       = BACKGROUND_INTENSITY | BACKGROUND_GREEN,
	BACK_COLOR_CYAN        = BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE,
	BACK_COLOR_RED         = BACKGROUND_INTENSITY | BACKGROUND_RED,
	BACK_COLOR_MAGENTA     = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE,
	BACK_COLOR_YELLOW      = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN,
	BACK_COLOR_WHITE       = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE
};
#pragma warning(default:4480)

#define RESIZE_CENTER_WINDOW(hWnd, nWidth, nHeight) {RECT rcOwner; GetWindowRect(GetDesktopWindow(), &rcOwner); MoveWindow(hWnd, (rcOwner.right-rcOwner.left-nWidth)/2, (rcOwner.bottom-rcOwner.top-nHeight)/2, nWidth, nHeight, true); Sleep(0);}
#define RESIZE_WINDOW(hWnd, nNewWidth, nNewHeight) {SetWindowPos(hWnd, HWND_TOP, 0, 0, nNewWidth, nNewHeight, SWP_NOMOVE); Sleep(0);}

#define HIDE_CONSOLE_CURSOR() CONSOLE_CURSOR_INFO cci; cci.bVisible = TRUE; { if ( GetConsoleCursorInfo(ghConsoleOutput, &cci) ) { cci.bVisible = FALSE; SetConsoleCursorInfo(ghConsoleOutput, &cci); } }
#define RESTORE_CONSOLE_CURSOR() if ( !cci.bVisible ) { cci.bVisible = TRUE; SetConsoleCursorInfo(ghConsoleOutput, &cci); }
#define UNDO_TOPMOST_CONSOLE() SetWindowPos(ghWndConsole, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSIZE | SWP_SHOWWINDOW)

// Use for progress output to keep from being redirected with WriteConsoleA!
// WriteConsoleA output doesn't show up if app is used in batch file, must use WriteFile!
#define STR_SIZE(cText) cText, (sizeof(cText)-1)
#define FastPrint(cText) WriteFile(ghConsoleOutput, cText, sizeof(cText)-1, &gnNumberOfBytes, 0)
#define FastPrintC(cText) WriteConsoleA(ghConsoleOutput, cText, sizeof(cText)-1, &gnNumberOfBytes, 0)
#define FastNPrint(szText, nCharCount) WriteFile(ghConsoleOutput, szText, nCharCount, &gnNumberOfBytes, 0)

#define _CreateNewFile(szFileName)   CreateFileA(szFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL)
#define _OpenAppendFile(szFileName)  CreateFileA(szFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL)
#define _OpenFileRead(szFileName)    CreateFileA(szFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL)
#define _OpenFileRndRead(szFileName) CreateFileA(szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL)
#define _OpenFileReadWrite(szFileName) CreateFileA(szFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL)
#define _Read(hFile, lpBuffer, dwBytes)  ReadFile(hFile, (LPVOID)(lpBuffer), (dwBytes), &gnNumberOfBytes, NULL)
#define _ReadVerify(hFile, lpBuffer, dwBytes)  ( ReadFile(hFile, (LPVOID)(lpBuffer), (dwBytes), &gnNumberOfBytes, NULL) && gnNumberOfBytes == (dwBytes) )
#define _Write(hFile, lpBuffer, dwBytes) WriteFile(hFile, (LPCVOID)(lpBuffer), (dwBytes), &gnNumberOfBytes, NULL)
#define _WriteText(hFile, cText) WriteFile(hFile, (LPCVOID)cText, sizeof(cText)-1, &gnNumberOfBytes, NULL)
#define _WriteVerify(hFile, lpBuffer, dwBytes) ( WriteFile(hFile, (LPCVOID)lpBuffer, dwBytes, &gnNumberOfBytes, NULL) && gnNumberOfBytes == (dwBytes) )
#define _Close(hFile) CloseHandle(hFile)
#define CloseMap(hMap) { UnmapViewOfFile((hMap).lpMapAddress); CloseHandle((hMap).hMapFile); CloseHandle((hMap).hFile); __asm PUSH EDI __asm LEA EDI,(hMap) __asm XOR EAX,EAX __asm MOV ECX,SIZE MAPPEDFILE/4 __asm REP STOSD __asm POP EDI }

#ifdef __cplusplus
	#define _tell(hFile) SetFilePointer(hFile, 0, NULL, FILE_CURRENT)
	#define _seek(handle, offset, origin) SetFilePointer(handle, offset, NULL, origin)
	#define _read(handle, buffer, count) (( ReadFile(handle, buffer, count, &gnNumberOfBytes, NULL) && count == gnNumberOfBytes ) ? gnNumberOfBytes : -1)
	#define _write(handle, buffer, count) (( WriteFile(handle, (LPCVOID) buffer, count, &gnNumberOfBytes, NULL) && count == gnNumberOfBytes ) ? gnNumberOfBytes : -1)
	#define _close(hFile) CloseHandle(hFile)
#endif

#define tolower(c) (( !( (BYTE)c < 'A' || (BYTE)c > 'Z' ) ) ? (c | 0x20) : (c))
#define toupper(c) (( !( (BYTE)c < 'a' || (BYTE)c > 'z' ) ) ? (c ^ 0x20) : (c))
#define toLower(c) (( !( (BYTE)c < 'A' || (BYTE)c > 'Z' ) ) ? (c | 0x20) : (c))
#define toUpper(c) (( !( (BYTE)c < 'a' || (BYTE)c > 'z' ) ) ? (c ^ 0x20) : (c))
#define ToLowerCase(c) if ( !( (BYTE)c < 'A' || (BYTE)c > 'Z' ) ) c |= 0x20
#define ToUpperCase(c) if ( !( (BYTE)c < 'a' || (BYTE)c > 'z' ) ) c ^= 0x20
#define IsUpperCase(c) !( (BYTE)c < 'A' || (BYTE)c > 'Z' )
#define IsLowerCase(c) !( (BYTE)c < 'a' || (BYTE)c > 'z' )
#define MakeLowerCase(c) c | 0x20
#define MakeUpperCase(c) c ^ 0x20
#define IsSJISLeadByte1(c) ( !((BYTE)c < 0x81 || (BYTE)c > 0x98) )
#define IsSJISTrailByte(c) ( !((BYTE)c < 0x40 || (BYTE)c > 0xFC) && (BYTE)c != 0x7F )


//#define ROUND(x) (floor(x+0.5))
#undef  isspace
#define isspace(c) ( (BYTE)c >= 0x09 && (BYTE)c <= 0x0D || c == ' ' )
#undef  isdigit
#define isdigit(c) !( (BYTE)c < '0' || (BYTE)c > '9' )
#define IsDigit(c) !( (BYTE)c < '0' || (BYTE)c > '9' )
#define IsNonDigit(c) ( (BYTE)c < '0' || (BYTE)c > '9' )
#define IsPunct(c) !( ( c < '!' || c > '/' ) && ( c < ':' || c > '@' ) && ( c < '[' || c > '`' ) && ( c < '{' || c > '~' ) )
#undef  isalpha
#define isalpha(c) !( ( (BYTE)c < 'a' || (BYTE)c > 'z' ) && ( (BYTE)c < 'A' || (BYTE)c > 'Z' ) )
#define IsHexDigit(c) !( ( (BYTE)c < '0' || (BYTE)c > '9' ) && ( (BYTE)c < 'A' || (BYTE)c > 'F' ) && ( (BYTE)c < 'a' || (BYTE)c > 'f' ) )
#define IsNonHexDigit(c) ( ( (BYTE)c < '0' || (BYTE)c > '9' ) && ( (BYTE)c < 'A' || (BYTE)c > 'F' ) && ( (BYTE)c < 'a' || (BYTE)c > 'f' ) )
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define atoi(str) (int)StrToLong(str)
#define atol StrToLong
#define LABS(lnumber) ( (long)(lnumber) >= 0L ? (lnumber) : -(lnumber) )
#define HasNulByte(x) ((x - 0x01010101) & ~x & 0x80808080) // Alan Mycroft

#define STOP(szSTOPMessage) { MessageBoxA(0, szSTOPMessage, "ATTACH DEBUGGER TO ME!!!!!!!!!!!", MB_OK|MB_TOPMOST); __debugbreak(); }
#define SHOW(szMessage) MessageBoxA(0, szMessage, "Message", MB_OK|MB_TOPMOST)

// "DebugBreakProcess" was introduced with XP, other XP detection method
#define IsWinXPOrAbove() (gOSInfo.MajorVersion > 5 || (gOSInfo.MajorVersion == 5 && gOSInfo.MinorVersion > 0))

typedef BOOL (WINAPI *PROC_OPENPROCESSTOKEN)(HANDLE, DWORD, PHANDLE);
typedef BOOL (WINAPI *PROC_ATTACHCONSOLE)(DWORD);
typedef BOOL (WINAPI *FARPROC1)(void);
typedef unsigned char BITS;

typedef signed char        int8_t;
typedef short int          int16_t;
typedef int                int32_t;
typedef long int           int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short int  uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long int   uint64_t;

// ******** Function Prototypes ********

// Math library functions
EXTERN long SetTruncationMode();
EXTERN long   __stdcall round(float x);
EXTERN long   __stdcall Round(double x);

EXTERN float  __fastcall fs_ceil(float x);
EXTERN double __fastcall fs_Ceil(double x);
EXTERN long   __fastcall fs_ceilInt(float x);
EXTERN long   __fastcall fs_CeilInt(double x);

EXTERN float  __fastcall fs_floor(float x);
EXTERN double __fastcall fs_Floor(double x);
EXTERN long   __fastcall fs_floorInt(float x);
EXTERN long   __fastcall fs_FloorInt(double x);

EXTERN __inline float   __stdcall fs_fabs(float x);
EXTERN __inline double  __stdcall fs_Fabs(double x);
EXTERN __inline int    __fastcall fs_absdiff(short a, short b);
EXTERN __inline float  __fastcall fs_sin(float x);
EXTERN __inline double __fastcall fs_Sin(double x);
EXTERN __inline float  __fastcall fs_cos(float x);
EXTERN __inline double __fastcall fs_Cos(double x);
EXTERN __inline float  __fastcall fs_atan(float x);
EXTERN __inline double __fastcall fs_Atan(double x);
EXTERN           float  __stdcall fs_acos(float x);
EXTERN          double  __stdcall fs_Acos(double x);

EXTERN __inline float  __fastcall fs_sqrt(float x);
EXTERN __inline double __fastcall fs_Sqrt(double x);
EXTERN          float  __fastcall fs_ldexp(float x, long exp);
EXTERN          double __fastcall fs_ldExp(double x, long exp);

EXTERN __inline float  __fastcall fs_log(float x);
EXTERN __inline double __fastcall fs_Log(double x);
EXTERN          float  __stdcall fs_exp(float x);
EXTERN          double __stdcall fs_Exp(double x);
EXTERN          double __stdcall fs_ExpFPU();

EXTERN float    __stdcall fs_fmod(float x, float y);
EXTERN double   __stdcall fs_Fmod(double x, double y);

EXTERN double   __fastcall fs_power(double x, long n);
EXTERN double   __stdcall  fs_Power(double x, double y);
// String library functions
EXTERN void     __fastcall strshr(PSTR);
EXTERN DWORD    __fastcall StrLen(PSTR lpText);
EXTERN PSTR     __fastcall StrCpy(PSTR lpDestination, PSTR lpSource);
EXTERN long     __fastcall StrCmp(PSTR lpString1, PSTR lpString2);
EXTERN long     __fastcall StrCmpI(PSTR lpString1, PSTR lpString2);
EXTERN long     __fastcall StrNCmpI(PSTR lpString1, PSTR lpString2, DWORD Count);
EXTERN void     __fastcall lNc(DWORD Count, PSTR lpText);
EXTERN PSTR     __fastcall StrLowerCase(PSTR lpText);
EXTERN PSTR     __fastcall StrUpperCase(PSTR lpText);
EXTERN DWORD    __fastcall chrCount(PSTR, char);
EXTERN void     __fastcall chrFindSet(PSTR lpText, char cFindChar, char cSetChar);
EXTERN void     __fastcall chrFindSetW(PSTR lpText, WORD wFindChar, WORD wSetChar);
EXTERN DWORD    __fastcall HexToDec(PSTR lpText);
EXTERN DWORD    __fastcall HexToDecN(DWORD Count, PSTR lpText);
EXTERN long     __fastcall StrToLong(PSTR lpText);
EXTERN DWORD    __fastcall StrToDWORD(PSTR lpText);
EXTERN DWORD    __fastcall StringToNumber(PSTR lpText);
EXTERN DWORD    __fastcall LongToStr(IN long Number, OUT PSTR lpTextBuffer, BYTE nPadWidth, char cPadChar);
EXTERN DWORD    __fastcall DWORDToStr(IN DWORD Number, OUT PSTR lpTextBuffer, BYTE nPadWidth, char cPadChar);
EXTERN void     __fastcall FormatNumber(DWORD Number, OUT PSTR lpFormattedNumber);
// Memory library functions
EXTERN void     __fastcall MemZero(DWORD Length, PVOID Destination);
EXTERN void     __fastcall MemSet(DWORD Length, PVOID Destination, BYTE val);
EXTERN void     __fastcall MemSet32(DWORD Length, PVOID Destination, DWORD val);
EXTERN void *   __fastcall MemCpy(DWORD Length, PVOID Destination, PVOID Source);
EXTERN void     __fastcall MemMove(DWORD Length, PVOID Destination, PVOID Source);
EXTERN long     __fastcall MemCmp(DWORD dwSize, PVOID lpMemory1, PVOID lpMemory2);
EXTERN void *   __fastcall _Aligned_Malloc(size_t size, size_t alignment);
EXTERN void *   __fastcall _Aligned_Calloc(size_t size, size_t alignment);
EXTERN void *   __fastcall _Aligned_ReAlloc(void *memblock, size_t size, size_t alignment);
EXTERN void     __fastcall _Aligned_Free(void *memblock);
// Windows library functions
EXTERN int      __fastcall DetectWinOS(); // Should be called in init for version-detection features
EXTERN BOOL     __fastcall SetVGAMode(DWORD nWidth, DWORD nHeight, DWORD nBitsPerPixel, DWORD nFrequency);
EXTERN void     __fastcall SetWindowTopMost(HWND hWindow);
EXTERN void     __fastcall CenterWindow(HWND hWindow);
EXTERN void     __fastcall RestoreWindow(HWND hWindow);
EXTERN void     __fastcall ResizeWindow(HWND hWindow, int nNewWidth, int nNewHeight);
EXTERN BOOL     __fastcall PasteToTextBox(HWND hTextBox);
EXTERN DWORD    __fastcall GetExeFile(PSTR lpBuffer, DWORD uSize);
EXTERN DWORD    __fastcall GetExeDirectory(DWORD uSize, PSTR lpBuffer);
EXTERN DWORD    __cdecl    GetLastSysError(PSTR lpBuffer, UINT uSize, PCSTR format, ...);
EXTERN DWORD    __cdecl    ShowLastSysError(PCSTR format, ...);
// Console library functions
EXTERN int      __fastcall InitConsole(); // Must be called in init to have working console
EXTERN void     __fastcall cls();
EXTERN void     __fastcall DeleteLine();
EXTERN void     __fastcall pause(DWORD dwMilliseconds); // Pause till key press or with Timeout also
EXTERN long     __fastcall ScanForNumber(BYTE nMaxDigits, BOOL bSigned);
EXTERN char     __fastcall AbortRetryIgnorePrompt();
EXTERN BOOL     __fastcall YesNoPrompt(PSTR lpText, DWORD nLength);
EXTERN DWORD    __cdecl    PrintLastSysError(PCSTR format, ...);
EXTERN void     __fastcall PrintColoredText(PCSTR lpText, DWORD nLength, WORD wColor);
// File I/O (Read/Write/Map) library functions
EXTERN DWORD    __fastcall MapFile32(PSTR lpFileName, BOOL ReadOnly, OUT MAPPEDFILE *mFile);
EXTERN BOOL     __fastcall MapFileFastRead(PSTR lpFileName, OUT MAPPEDFILE *mFile);
EXTERN PBYTE    __fastcall ReadFileToMemory(PSTR lpFileName);
EXTERN BOOL     __fastcall WriteNewFile(PSTR lpFileName, PVOID lpBuffer, DWORD nNumberOfBytesToWrite);
EXTERN BOOL     __fastcall UpdateOldFile(PSTR lpFileName, DWORD dwStartOffset, PVOID lpBuffer, DWORD nNumberOfBytesToWrite);
EXTERN BOOL     __fastcall AppendOldFile(PSTR lpFileName, PVOID lpBuffer, DWORD nNumberOfBytesToWrite);
EXTERN void        __cdecl FastLog(PSTR lpLogFileName, char *format, ...);
EXTERN DWORD    __fastcall GetFileSize32(PSTR);
EXTERN BOOL     __fastcall SetFileWriteTimeToCurrentTime(PSTR lpFileName);
EXTERN BOOL     __fastcall GetFileTimes(PSTR lpFileName, LPFILETIMES lpFileTimes);
EXTERN DWORD    __fastcall GetFileCount(PSTR lpFileFilter);
EXTERN DWORD    __fastcall GetFileCountFast(PSTR lpFindFilter);
EXTERN BOOL     __fastcall GetResourceFile(PSTR szModuleFileName, PSTR szOutputFileName, PSTR szResName, PSTR szResType);
// Basic Registry library functions
EXTERN DWORD    __stdcall  GetKeyValueDWORD(PSTR RootCode_SubKey, PSTR lpValueName);
EXTERN BOOL     __fastcall SetKeyValueDWORD(HKEY KeyRoot, PSTR lpSubKey, PSTR lpValueName, DWORD Data);
EXTERN DWORD    __fastcall GetKeyValueSTR(HKEY KeyRoot, PSTR lpSubKey, PSTR lpValueName, PSTR lpData, DWORD cbData);
EXTERN DWORD    __stdcall  xGetKeyValueSTR(PSTR RootCode_SubKey, PSTR lpValueName, PSTR lpData, DWORD cbData);
EXTERN BOOL     __fastcall SetKeyValueSTR(HKEY KeyRoot, PSTR lpSubKey, PSTR lpValueName, PSTR lpData, DWORD cbData);
EXTERN BOOL     __stdcall  SetKeyValueBIN(HKEY KeyRoot, PSTR lpSubKey, PSTR lpValueName, PBYTE lpData, DWORD cbData);
EXTERN BOOL     __fastcall DeleteKeyValue(HKEY KeyRoot, PSTR lpSubKey, PSTR lpValueName);
// Image functions
EXTERN HBITMAP  __fastcall ConvertMenuIconToBitmap(HICON hIcon);
// Zip util
EXTERN DWORD    __stdcall  UnZipFile32(PSTR szZipFile, PSTR szTargetFile);
// General Windows
EXTERN BOOL     __fastcall WinExecute(PSTR lpExeName, PSTR lpExeDirectory, PSTR lpCommandLine, BOOL bWaitForExit, BOOL bHide);
EXTERN BOOL     __stdcall  DDraw256WinExec(PSTR lpExeName);
EXTERN DWORD    __stdcall  IsProcessRunning(PSTR lpExeName);
EXTERN DWORD    __stdcall  KillProcess(PSTR lpExeName);
EXTERN void     __stdcall  SuspendResumeProcess(LPSTR lpExeName, BOOL bResume);
EXTERN BOOL     __fastcall IsUserAdmin();
EXTERN void     __fastcall XORDecrypt(PSTR);
EXTERN PVOID    __fastcall HookAPI(PSTR thismodule, PSTR module, PSTR api, LPVOID hookproc);
EXTERN BOOL     __fastcall FindImportDLL(PSTR lpExeFileName, PSTR lpDllName);
EXTERN void     __fastcall PatchKernelAPI(PBYTE lpBuffer);

EXTERN long     __fastcall FastFindDX(PVOID lpMem, DWORD nStartIndex, DWORD nStopIndex, PVOID lpFind, DWORD nFindSize);
EXTERN PSTR     __stdcall  FindFast(PVOID lpMem, DWORD dwMemSize, PVOID lpFind, DWORD dwFindSize);
EXTERN PSTR     __fastcall FindDWORD(DWORD dwSize, PVOID lpMem, DWORD dwFind);
EXTERN PBYTE    __fastcall MemChr(DWORD dwSize, PVOID lpMem, BYTE Char);

EXTERN  void    __fastcall DoBenchMark(ULONGLONG rdtscBefore, ULONGLONG rdtscAfter);

#endif  // _GLOBAL_H_LIB_ END
