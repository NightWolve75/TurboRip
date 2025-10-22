#include "\WORKSPACE\CodeBase\VisualC++\Global.h"

#if defined(_DEBUG) && defined(__cplusplus)
	long _DetectWinOS__ = DetectWinOS();
	long _SetTruncationMode_ = SetTruncationMode();
#else
	 #pragma message("You must initalize Heap! Call InitCRTHeap() or DetectWinOS() in your main!")
#endif

// * Global Strings/Variables *

DWORD gnNumberOfBytes, glpAddress, gCounter, gSemaphore;
PSTR MSG_YES_NO[] = {"No", "Yes"};
char gGDI32DLL[] = "GDI32.dll";
char gUser32DLL[] = "user32.dll";
char gKernel32DLL[] = "kernel32.dll";
char gRegistryDLL[] = "advapi32.dll";
char gComdlg32DLL[] = "comdlg32.dll";
char gExplorer[] = "explorer.exe";
char gError[] = "Error";
// Prevent crash in Win9X wsprintfA (Can't pass NULL/0, MUST pass address with null string!)
char gNull[] = "";

// * Dynamically-linked function prototypes *
DWORD (WINAPI *procUnZipFile)(PSTR IN szZipFile, PSTR IN szTargetFile, PSTR OUT szOutputPath);
DWORD (WINAPI *procUnZipGetLastError)(PSTR OUT lpBuffer, DWORD IN nSize);

// * Organized function includes *

// Memory/Allocation/Deallocation functions
#include <Malloc.cpp>
#include <Memory.cpp>
// Windows library functions
#include <Windows.cpp>
// Console library functions
#include <Console.cpp>
// File IO/Read/Write/Map library functions
#include <FileIO.cpp>
// String library functions
#include <Strings.cpp>
// Registry library functions
#include <Registry.cpp>
// Math library functions
#include <Math\Math.cpp>
// Process/Thread functions
#include <Process.cpp>

#include <Image.cpp>


extern LPVOID __fastcall HookAPI(PSTR thismodule, PSTR module, PSTR api, LPVOID hookproc) {
	PIMAGE_THUNK_DATA ptaddr, ptname;
	PIMAGE_IMPORT_DESCRIPTOR pidesc;
	PIMAGE_IMPORT_BY_NAME piname;
	PIMAGE_NT_HEADERS pnth;
	ULONG_PTR base, rva;
	DWORD dwOldProtect;
	PSTR impmodule;
	LPVOID org;

	base = (ULONG_PTR)GetModuleHandleA(thismodule);
	if ( !base )
		return 0;
	pnth = (PIMAGE_NT_HEADERS)(base + (ULONG_PTR)((PIMAGE_DOS_HEADER)base)->e_lfanew);
	if ( !pnth )
		return 0;
	rva = pnth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	if ( !rva )
		return 0;
	pidesc = (PIMAGE_IMPORT_DESCRIPTOR)(base + rva);
	for (;;) {
		if ( !pidesc->FirstThunk )
			return 0;
		impmodule = (PSTR)(base + pidesc->Name);
		if ( !StrCmpI(module, impmodule) )
			break;
		pidesc++;
	}
	ptaddr = (PIMAGE_THUNK_DATA)(base + pidesc->FirstThunk);
	ptname = (PIMAGE_THUNK_DATA)(base + pidesc->OriginalFirstThunk);
	for (;;) {
		if ( !ptaddr->u1.Function )
			return 0;
		if ( !IMAGE_SNAP_BY_ORDINAL(ptname->u1.Ordinal) ) {
			piname = (PIMAGE_IMPORT_BY_NAME)(base + ptname->u1.AddressOfData);
			if ( !StrCmpI(api, (LPSTR)piname->Name) )
				break;
		}
		ptaddr++;
		ptname++;
	}
	org = (LPVOID)((ULONG_PTR)ptaddr->u1.Function);
	if ( org == hookproc )
		return 0;
	if ( !VirtualProtect(&ptaddr->u1.Function, 4, PAGE_EXECUTE_READWRITE, &dwOldProtect) )
		return 0;
	#pragma warning(disable:4244)
	ptaddr->u1.Function = (ULONG_PTR)hookproc;
	#pragma warning(default:4244)
	VirtualProtect(&ptaddr->u1.Function, 4, dwOldProtect, &dwOldProtect);
	FlushInstructionCache(GetCurrentProcess(), NULL, 0);
	return org;
}

extern BOOL __fastcall FindImportDLL(LPSTR lpExeFileName, LPSTR lpDllName) {
	ULONG_PTR pImportsStartRVA, pMaxEOFAddr;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_NT_HEADERS pNTHeader;
	LPSTR lpDllImport;
	MAPPEDFILE mFile;
	BOOL bResult;
	if ( !MapFileFastRead(lpExeFileName, &mFile) )
		return FALSE;
	pNTHeader = (PIMAGE_NT_HEADERS)(mFile.lpMapAddress + (ULONG_PTR)((PIMAGE_DOS_HEADER)mFile.lpMapAddress)->e_lfanew);
	bResult = FALSE;
	if ( !pNTHeader )
		goto EXIT_RETURN;
	pImportsStartRVA = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	if ( !pImportsStartRVA )
		goto EXIT_RETURN;
	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)(mFile.lpMapAddress + pImportsStartRVA);
	pMaxEOFAddr = AddPtr(ULONG_PTR, mFile.lpMapAddress, mFile.dwFileSize) - 64;
	while ( (ULONG_PTR)pImportDesc < pMaxEOFAddr ) {
		if ( !pImportDesc->FirstThunk )
			goto EXIT_RETURN;
		lpDllImport = (PSTR)(mFile.lpMapAddress + pImportDesc->Name);
		if ( (ULONG_PTR)lpDllImport > pMaxEOFAddr )
			goto EXIT_RETURN;
		if ( !StrCmpI(lpDllImport, lpDllName) ) {
			bResult = TRUE;
			break;
		}
		pImportDesc++;
	}
EXIT_RETURN:
	CloseMap(mFile);
	return bResult;
}

//
// Given an RVA, look up the section header that encloses
// it and return a pointer to its IMAGE_SECTION_HEADER
//
extern PIMAGE_SECTION_HEADER __fastcall GetEnclosingSectionHeader(DWORD rva, PIMAGE_NT_HEADERS pNTHeader) { // 'T' == PIMAGE_NT_HEADERS 
	DWORD size;
	WORD i;
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
	for ( i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++ ) {
	// This 3 line idiocy is because Watcom's linker actually sets the
	// Misc.VirtualSize field to 0.  (!!! - Retards....!!!)
		size = section->Misc.VirtualSize;
		if ( size == 0 )
			size = section->SizeOfRawData;
	// Is the RVA within this section?
		if ( (rva >= section->VirtualAddress) && (rva < (section->VirtualAddress + size)))
			return section;
	}
	return 0;
}

extern PVOID __fastcall GetPtrFromRVA(DWORD rva, PIMAGE_NT_HEADERS pNTHeader, PBYTE imageBase) { // 'T' = PIMAGE_NT_HEADERS
	PIMAGE_SECTION_HEADER pSectionHdr;
	long delta;
	pSectionHdr = GetEnclosingSectionHeader(rva, pNTHeader);
	if ( !pSectionHdr )
		return 0;
	delta = (int)(pSectionHdr->VirtualAddress-pSectionHdr->PointerToRawData);
	return (PVOID) ( imageBase + rva - delta );
}

extern void __fastcall PatchKernelAPI(PBYTE lpBuffer) {
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	PIMAGE_NT_HEADERS pNTHeader;
	DWORD importsStartRVA;
	LPSTR lpImpFileName;
	pNTHeader = AddPtr(PIMAGE_NT_HEADERS, lpBuffer, ((PIMAGE_DOS_HEADER)lpBuffer)->e_lfanew);
// If we set the 4GB /LARGEADDRESSAWARE flag set, unset it to pass CRC check!
	if ( pNTHeader->FileHeader.Characteristics & 0x0020 )
		pNTHeader->FileHeader.Characteristics &= 0xFFDF;
	importsStartRVA = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR) GetPtrFromRVA(importsStartRVA, pNTHeader, lpBuffer);
	lpImpFileName = (char*)GetPtrFromRVA(pImportDesc->Name, pNTHeader, lpBuffer);
	MemCpy(sizeof(KERNEL32_)-1, lpImpFileName, KERNEL32_);
}

#if ( _WIN32_WINNT > 0x400 )
/* These are new NT-using functions, need more dynamic loading, comment for now!
extern BOOL IsUserAdmin() {
	SID_IDENTIFIER_AUTHORITY siaNTAuthority = SECURITY_NT_AUTHORITY;
	const long INFOBUFFERSIZE = 1024;
	UCHAR InfoBuffer[INFOBUFFERSIZE];
	PTOKEN_GROUPS ptgGroups = (PTOKEN_GROUPS)InfoBuffer;
	HANDLE hAccessToken;
	DWORD dwInfoBufferSize;
	PSID psidAdministrators;
	BOOL bIsAdmin = FALSE;
	UINT uCount;
	if( !OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hAccessToken) )
		return FALSE;
	bIsAdmin = GetTokenInformation(hAccessToken, TokenGroups, InfoBuffer, INFOBUFFERSIZE, &dwInfoBufferSize);
	CloseHandle(hAccessToken);
	if ( !bIsAdmin )
		return FALSE;
	if ( !AllocateAndInitializeSid(&siaNTAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 
		0, 0, 0, 0, 0, 0, &psidAdministrators) )
		return FALSE;
	bIsAdmin = FALSE;
	for ( uCount = 0; uCount < ptgGroups->GroupCount; uCount++ )
		if ( EqualSid(psidAdministrators, ptgGroups->Groups[uCount].Sid) ) {
			bIsAdmin = TRUE;
			break;
		}
	FreeSid(psidAdministrators);
	return bIsAdmin;
}*/
#endif

extern void __fastcall XORDecrypt(LPSTR lpEncryptedText) {
	__asm {
		MOV  EBX, lpEncryptedText
	LOOP_PER_CHAR:
		MOV  AL, BYTE PTR [EBX]	
		TEST AL, AL
		JZ   EXIT_RETURN
		XOR  AL, 7Fh
		MOV  BYTE PTR [EBX], AL
		INC  EBX
		JMP  LOOP_PER_CHAR
	EXIT_RETURN:
	}
}

#ifndef _DEBUG // C-RUNTIME FUNCTIONS BEGIN

/***
*__int64 _filelengthi64(filedes) - find length of a file
*
*Purpose:
*       Returns the length in bytes of the specified file.
*Entry:
*       long filedes - handle referring to file to find length of
*Exit:
*       returns length of file in bytes
*       returns -1i64 if fails
*******************************************************************************/
extern ULONGLONG __cdecl filelengthi64(HANDLE hFile) {
	ULARGE_INTEGER qwFileSize;
	qwFileSize.LowPart = GetFileSize(hFile, &qwFileSize.HighPart);
	return qwFileSize.QuadPart;
}
/***
*int abs/labs(number) - find absolute value of number
*
*Purpose:
*       Returns the absolute value of number (if number >= 0, returns number,
*       else returns -number).
*Entry:
*       long number - number to find absolute value of
*Exit:
*       returns the aboslute value of number
*******************************************************************************/
#pragma function(abs)
extern int __cdecl abs(int number) { return( number>=0 ? number : -number ); }

#pragma function(labs)
long __cdecl labs(long lnumber) { return( lnumber>=0L ? lnumber : -lnumber ); }
/* _purecall is a placeholder function that lives in the CRT so that a Class with
a pure virtual function can have its vtable entry pointed to something instead
of nothing. It's a function that doesn’t do anything, and it's an error for it
to ever be called, so all you need to do is create your own do-nothing _purecall.
*******************************************************************************/
extern int __cdecl _purecall() { return 0; }
/***
*qsort(base, num, wid, comp) - quicksort function for sorting arrays
*
*Purpose:
*       quicksort the array of elements
*       side effects:  sorts in place
*       maximum array size is number of elements times size of elements,
*       but is limited by the virtual address space of the processor
*
*Entry:
*       char *base = pointer to base of array
*       size_t num  = number of elements in the array
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*******************************************************************************/
/* prototypes for local routines */
void __fastcall shortsort(char *lo, char *hi, size_t width,
                int (__cdecl *comp)(const void *, const void *));
void __fastcall swap(char *p, char *q, size_t width);

#define CUTOFF 8 /* testing shows that this is good value */
/* sort the array between lo and hi (inclusive) */
#define STKSIZ (8*sizeof(void*) - 2)

extern void __cdecl qsort (
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
    )
{
    /* Note: the number of stack entries required is no more than
       1 + log2(num), so 30 is sufficient for any array */
    char *lo, *hi;              /* ends of sub-array currently sorting */
    char *mid;                  /* points to middle of subarray */
    char *loguy, *higuy;        /* traveling pointers for partition step */
    size_t size;                /* size of the sub-array */
    char *lostk[STKSIZ], *histk[STKSIZ];
    int stkptr;                 /* stack for saving sub-array to be processed */

    if (num < 2 || width == 0)
        return;                 /* nothing to do */

    stkptr = 0;                 /* initialize stack */

    lo = (char *)base;
    hi = (char *)base + width * (num-1);        /* initialize limits */

    /* this entry point is for pseudo-recursion calling: setting
       lo and hi and jumping to here is like recursion, but stkptr is
       preserved, locals aren't, so we preserve stuff on the stack */
recurse:

    size = (hi - lo) / width + 1;        /* number of el's to sort */

    /* below a certain size, it is faster to use a O(n^2) sorting method */
    if (size <= CUTOFF) {
        shortsort(lo, hi, width, comp);
    }
    else {
        /* First we pick a partitioning element.  The efficiency of the
           algorithm demands that we find one that is approximately the median
           of the values, but also that we select one fast.  We choose the
           median of the first, middle, and last elements, to avoid bad
           performance in the face of already sorted data, or data that is made
           up of multiple sorted runs appended together.  Testing shows that a
           median-of-three algorithm provides better performance than simply
           picking the middle element for the latter case. */

        mid = lo + (size / 2) * width;      /* find middle element */

        /* Sort the first, middle, last elements into order */
        if (comp(lo, mid) > 0) {
            swap(lo, mid, width);
        }
        if (comp(lo, hi) > 0) {
            swap(lo, hi, width);
        }
        if (comp(mid, hi) > 0) {
            swap(mid, hi, width);
        }

        /* We now wish to partition the array into three pieces, one consisting
           of elements <= partition element, one of elements equal to the
           partition element, and one of elements > than it.  This is done
           below; comments indicate conditions established at every step. */

        loguy = lo;
        higuy = hi;

        /* Note that higuy decreases and loguy increases on every iteration,
           so loop must terminate. */
        for (;;) {
            /* lo <= loguy < hi, lo < higuy <= hi,
               A[i] <= A[mid] for lo <= i <= loguy,
               A[i] > A[mid] for higuy <= i < hi,
               A[hi] >= A[mid] */

            /* The doubled loop is to avoid calling comp(mid,mid), since some
               existing comparison funcs don't work when passed the same
               value for both pointers. */

            if (mid > loguy) {
                do  {
                    loguy += width;
                } while (loguy < mid && comp(loguy, mid) <= 0);
            }
            if (mid <= loguy) {
                do  {
                    loguy += width;
                } while (loguy <= hi && comp(loguy, mid) <= 0);
            }

            /* lo < loguy <= hi+1, A[i] <= A[mid] for lo <= i < loguy,
               either loguy > hi or A[loguy] > A[mid] */

            do  {
                higuy -= width;
            } while (higuy > mid && comp(higuy, mid) > 0);

            /* lo <= higuy < hi, A[i] > A[mid] for higuy < i < hi,
               either higuy == lo or A[higuy] <= A[mid] */

            if (higuy < loguy)
                break;

            /* if loguy > hi or higuy == lo, then we would have exited, so
               A[loguy] > A[mid], A[higuy] <= A[mid],
               loguy <= hi, higuy > lo */

            swap(loguy, higuy, width);

            /* If the partition element was moved, follow it.  Only need
               to check for mid == higuy, since before the swap,
               A[loguy] > A[mid] implies loguy != mid. */

            if (mid == higuy)
                mid = loguy;

            /* A[loguy] <= A[mid], A[higuy] > A[mid]; so condition at top
               of loop is re-established */
        }

        /*     A[i] <= A[mid] for lo <= i < loguy,
               A[i] > A[mid] for higuy < i < hi,
               A[hi] >= A[mid]
               higuy < loguy
           implying:
               higuy == loguy-1
               or higuy == hi - 1, loguy == hi + 1, A[hi] == A[mid] */

        /* Find adjacent elements equal to the partition element.  The
           doubled loop is to avoid calling comp(mid,mid), since some
           existing comparison funcs don't work when passed the same value
           for both pointers. */

        higuy += width;
        if (mid < higuy) {
            do  {
                higuy -= width;
            } while (higuy > mid && comp(higuy, mid) == 0);
        }
        if (mid >= higuy) {
            do  {
                higuy -= width;
            } while (higuy > lo && comp(higuy, mid) == 0);
        }

        /* OK, now we have the following:
              higuy < loguy
              lo <= higuy <= hi
              A[i]  <= A[mid] for lo <= i <= higuy
              A[i]  == A[mid] for higuy < i < loguy
              A[i]  >  A[mid] for loguy <= i < hi
              A[hi] >= A[mid] */

        /* We've finished the partition, now we want to sort the subarrays
           [lo, higuy] and [loguy, hi].
           We do the smaller one first to minimize stack usage.
           We only sort arrays of length 2 or more.*/

        if ( higuy - lo >= hi - loguy ) {
            if (lo < higuy) {
                lostk[stkptr] = lo;
                histk[stkptr] = higuy;
                ++stkptr;
            }                           /* save big recursion for later */

            if (loguy < hi) {
                lo = loguy;
                goto recurse;           /* do small recursion */
            }
        }
        else {
            if (loguy < hi) {
                lostk[stkptr] = loguy;
                histk[stkptr] = hi;
                ++stkptr;               /* save big recursion for later */
            }

            if (lo < higuy) {
                hi = higuy;
                goto recurse;           /* do small recursion */
            }
        }
    }

    /* We have sorted the array, except for any pending sorts on the stack.
       Check if there are any, and do them. */

    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;           /* pop subarray from stack */
    }
    else
        return;                 /* all subarrays done */
}
/***
*shortsort(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*       sorts the sub-array of elements between lo and hi (inclusive)
*       side effects:  sorts in place
*       assumes that lo < hi
*
*Entry:
*       char *lo = pointer to low element to sort
*       char *hi = pointer to high element to sort
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*******************************************************************************/
void __fastcall shortsort (
    char *lo,
    char *hi,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
    )
{
    char *p, *max;
    /* Note: in assertions below, i and j are alway inside original bound of
       array to sort. */
    while (hi > lo) {
        /* A[i] <= A[j] for i <= j, j > hi */
        max = lo;
        for (p = lo+width; p <= hi; p += width) {
            /* A[i] <= A[max] for lo <= i < p */
            if (comp(p, max) > 0) {
                max = p;
            }
            /* A[i] <= A[max] for lo <= i <= p */
        }
        /* A[i] <= A[max] for lo <= i <= hi */
        swap(max, hi, width);
        /* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */
        hi -= width;
        /* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
    }
    /* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
       so array is sorted */
}
/***
*swap(a, b, width) - swap two elements
*
*Purpose:
*       swaps the two array elements of size width
*
*Entry:
*       char *a, *b = pointer to two elements to swap
*       size_t width = width in bytes of each array element
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/
void __fastcall swap (
    char *a,
    char *b,
    size_t width
    )
{
    char tmp;
    if ( a != b )
        /* Do the swap one character at a time to avoid potential alignment
           problems. */
        while ( width-- ) {
            tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
}


#ifndef _CRTVETO
	extern int _fltused = 0x9875; // Special MSVC Compiler need of this variable
#endif

#endif
