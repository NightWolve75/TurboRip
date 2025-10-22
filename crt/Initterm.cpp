#ifndef _DEBUG
//==========================================
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001
//==========================================
#include <Global.h>
#include <Initterm.h>

#pragma data_seg(".CRT$XCA")
_PVFV __xc_a[] = { NULL };

#pragma data_seg(".CRT$XCZ")
_PVFV __xc_z[] = { NULL };

#pragma data_seg()  /* reset */

#pragma comment(linker, "/merge:.CRT=.data")

#define ATEXIT_MAX 32
_PVFV pf_atexitlist[ATEXIT_MAX];
DWORD cur_atexitlist_entries = 0;
DWORD max_atexitlist_entries = 0;

NAKED_EXTERN void __fastcall _initterm(_PVFV * pfbegin, _PVFV * pfend) { __asm {
/* Walk the table of function pointers from the bottom up, until
 * the end is encountered.  Do not skip the first entry.  The initial
 * value of pfbegin points to the first valid entry.  Do not try to
 * execute what pfend points to.  Only entries before pfend are valid.
	while ( pfbegin < pfend ) {
		if ( *pfbegin )
			(**pfbegin)();
		pfbegin++;
	}
*/
	SUB   ESP, 8
	MOV   [ESP+4], EDX
	CMP   ECX, EDX     ;// ( pfbegin < pfend ) ? yes, loop; no, exit!
	JB    INIT_LOOP
	JMP   EXIT_RETURN

MAIN_LOOP:
	CMP   ECX, [ESP+4] ;// ( pfbegin < pfend ) ? yes, loop; no, exit!
	JAE   EXIT_RETURN
INIT_LOOP:
	MOV   EAX, [ECX]
	ADD   ECX, 4       ;// pfbegin++; (32-bit pointer!)
	TEST  EAX, EAX     ;// If table entry is non-NULL, call it!
	JZ    MAIN_LOOP
		MOV   [ESP], ECX
		CALL  EAX      ;// (**pfbegin)(); Call it!
		MOV   ECX, [ESP]
JMP MAIN_LOOP

EXIT_RETURN:
	ADD   ESP, 8
	RET
}}

NAKED_EXTERN void __fastcall _DoExit() {
	if ( cur_atexitlist_entries )
	// Use ptr math to find the end of the array
		_initterm(pf_atexitlist, pf_atexitlist + cur_atexitlist_entries);
	__asm RET
}

int __cdecl atexit(_PVFV func) { // Expected by MSVCompiler, keep __cdecl!
	if ( !pf_atexitlist )
		return -1;
	if ( cur_atexitlist_entries < ATEXIT_MAX ) {
		pf_atexitlist[cur_atexitlist_entries++] = func; 
		return 0;
	}
	return -1;
}

#endif
