#ifndef _DEBUG

#define NAKED_EXTERN __declspec(naked) extern

#ifdef __cplusplus
	extern "C" {
#endif

NAKED_EXTERN void __cdecl _ftol2() {
	__asm {
		push	ebp
		mov	ebp, esp
		sub	esp, 20h
		and	esp, 0FFFFFFF0h
		fld	st
		fst	dword ptr [esp+18h]
		fistp	qword ptr [esp+10h]
		fild	qword ptr [esp+10h]
		mov	edx, [esp+18h]
		mov	eax, [esp+10h]
		test	eax, eax
		jz	short integer_QnaN_or_zero
arg_is_not_integer_QnaN:
		fsubp	st(1), st
		test	edx, edx
		jns	short positive
		fstp	dword ptr [esp]
		mov	ecx, [esp]
		xor	ecx, 80000000h
		add	ecx, 7FFFFFFFh
		adc	eax, 0
		mov	edx, [esp+14h]
		adc	edx, 0
		jmp	short localexit
positive:				
		fstp	dword ptr [esp]
		mov	ecx, [esp]
		add	ecx, 7FFFFFFFh
		sbb	eax, 0
		mov	edx, [esp+14h]
		sbb	edx, 0
		jmp	short localexit
integer_QnaN_or_zero:
		mov	edx, [esp+14h]
		test	edx, 7FFFFFFFh
		jnz	short arg_is_not_integer_QnaN
		fstp	dword ptr [esp+18h]
		fstp	dword ptr [esp+18h]
localexit:
		leave
		retn
	}
}

NAKED_EXTERN long __cdecl _ftol2_sse() { __asm {
	SUB	 ESP, 20h
	FLD	 ST
	FST	 DWORD PTR [ESP+18h]
	FISTP QWORD PTR [ESP+10h]
	FILD  QWORD PTR [ESP+10h]
	MOV  EDX, [ESP+18h]
	MOV	 EAX, [ESP+10h]
	TEST EAX, EAX
	JZ	 INTEGER_QNAN_OR_ZERO

ARG_IS_NOT_INTEGER_QNAN:
	FSUBP ST(1), ST
	TEST EDX, EDX
	JNS	 POSITIVE
	FSTP DWORD PTR [ESP]
	MOV  ECX, [ESP]
	XOR  ECX, 80000000h
	ADD  ECX, 7FFFFFFFh
	ADC  EAX, 0
	MOV  EDX, [ESP+14H]
	ADC  EDX, 0
	JMP  LOCALEXIT
POSITIVE:				
	FSTP DWORD PTR [ESP]
	MOV  ECX, [ESP]
	ADD  ECX, 7FFFFFFFh
	SBB  EAX, 0
	MOV  EDX, [ESP+14h]
	SBB  EDX, 0
	ADD  ESP, 20h
	RET
INTEGER_QNAN_OR_ZERO:
	MOV  EDX, [ESP+14h]
	TEST EDX, 7FFFFFFFh
JNZ	ARG_IS_NOT_INTEGER_QNAN

	FSTP DWORD PTR [ESP+18h]
	FSTP DWORD PTR [ESP+18h]
LOCALEXIT:
	ADD  ESP, 20h
	RET ;// Return to caller; result is in EDX:EAX
}}

/***
;_chkstk - check stack upon procedure entry
;
;Purpose:
;       Provide stack checking on procedure entry. Method is to simply probe
;       each page of memory required for the stack in descending order. This
;       causes the necessary pages of memory to be allocated via the guard
;       page scheme, if possible. In the event of failure, the OS raises the
;       _XCPT_UNABLE_TO_GROW_STACK exception.
;
;       NOTE:  Currently, the (EAX < _PAGESIZE_) code path falls through
;       to the "lastpage" label of the (EAX >= _PAGESIZE_) code path.  This
;       is small; a minor speed optimization would be to special case
;       this up top.  This would avoid the painful save/restore of
;       ecx and would shorten the code path by 4-6 instructions.
;
;Entry:
;       EAX = size of local frame
;
;Exit:
;       ESP = new stackframe, if successful
;
;Uses:
;       EAX
;
;Exceptions:
;       _XCPT_GUARD_PAGE_VIOLATION - May be raised on a page probe. NEVER TRAP
;                                    THIS!!!! It is used by the OS to grow the
;                                    stack on demand.
;       _XCPT_UNABLE_TO_GROW_STACK - The stack cannot be grown. More precisely,
;                                    the attempt by the OS memory manager to
;                                    allocate another guard page in response
;                                    to a _XCPT_GUARD_PAGE_VIOLATION has
;                                    failed.
;
;*******************************************************************************/
NAKED_EXTERN void __cdecl _chkstk() {
	#define _PAGESIZE_ 1000h
	_asm {
        push    ecx
;// Calculate new TOS.
        lea     ecx, [esp] + 8 - 4      ;// TOS before entering function + size for ret value
        sub     ecx, eax                ;// new TOS
;// Handle allocation size that results in wraparound.
;// Wraparound will result in StackOverflow exception.
        sbb     eax, eax                ;// 0 if CF==0, ~0 if CF==1
        not     eax                     ;// ~0 if TOS did not wrapped around, 0 otherwise
        and     ecx, eax                ;// set to 0 if wraparound
        mov     eax, esp                ;// current TOS
        and     eax, not ( _PAGESIZE_ - 1) ;// Round down to current page boundary
cs10:
        cmp     ecx, eax                ;// Is new TOS
        jb      short cs20              ;// in probed page?
        mov     eax, ecx                ;// yes.
        pop     ecx
        xchg    esp, eax                ;// update esp
        mov     eax, dword ptr [eax]    ;// get return address
        mov     dword ptr [esp], eax    ;// and put it at new TOS
        ret
;// Find next lower page and probe
cs20:
        sub     eax, _PAGESIZE_         ;// decrease by PAGESIZE
        test    dword ptr [eax],eax     ;// probe page.
        jmp     short cs10
	}
}
/***
;llmul - long multiply routine
;
;Purpose:
;       Does a long multiply (same for signed/unsigned)
;       Parameters are not changed.
;
;Entry:
;       Parameters are passed on the stack:
;               1st pushed: multiplier (QWORD)
;               2nd pushed: multiplicand (QWORD)
;
;Exit:
;       EDX:EAX - product of multiplier and multiplicand
;       NOTE: parameters are removed from the stack
;
;Uses:
;       ECX
;*******************************************************************************/
NAKED_EXTERN void __cdecl _allmul() { __asm {
	MOV     EAX, [ESP+8]
	MOV     ECX, [ESP+10H]
	OR      ECX, EAX
	MOV     ECX, [ESP+0CH]
	JNZ     LOC_419FA9
	MOV     EAX, [ESP+4]
	MUL     ECX
	RETN    10H
LOC_419FA9:
	PUSH    EBX
	MUL     ECX
	MOV     EBX, EAX
	MOV     EAX, [ESP+8]
	MUL     DWORD PTR [ESP+14H]
	ADD     EBX, EAX
	MOV     EAX, [ESP+8]
	MUL     ECX
	ADD     EDX, EBX
	POP     EBX
	RET    10H
}}
/***
;lldiv - signed long divide
;
;Purpose:
;       Does a signed long divide of the arguments.  Arguments are
;       not changed.
;
;Entry:
;       Arguments are passed on the stack:
;               1st pushed: divisor (QWORD)
;               2nd pushed: dividend (QWORD)
;
;Exit:
;       EDX:EAX contains the quotient (dividend/divisor)
;       NOTE: this routine removes the parameters from the stack.
;
;Uses:
;       ECX
;
;Exceptions:
;
;*******************************************************************************/
NAKED_EXTERN void __cdecl _alldiv() { __asm {
	PUSH    EDI
	PUSH    ESI
	PUSH    EBX
	XOR     EDI, EDI
	MOV     EAX, [ESP+14H]
	OR      EAX, EAX
	JGE     SHORT LOC_41A001
	INC     EDI
	MOV     EDX, [ESP+10H]
	NEG     EAX
	NEG     EDX
	SBB     EAX, 0
	MOV     [ESP+14H], EAX
	MOV     [ESP+10H], EDX
LOC_41A001:
	MOV     EAX, [ESP+1CH]
	OR      EAX, EAX
	JGE     SHORT LOC_41A01D
	INC     EDI
	MOV     EDX, [ESP+18H]
	NEG     EAX
	NEG     EDX
	SBB     EAX, 0
	MOV     [ESP+1CH], EAX
	MOV     [ESP+18H], EDX
LOC_41A01D:
	OR      EAX, EAX
	JNZ     SHORT LOC_41A039
	MOV     ECX, [ESP+18H]
	MOV     EAX, [ESP+14H]
	XOR     EDX, EDX
	DIV     ECX
	MOV     EBX, EAX
	MOV     EAX, [ESP+10]
	DIV     ECX
	MOV     EDX, EBX
	JMP     SHORT LOC_41A07A
LOC_41A039:
	MOV     EBX, EAX
	MOV     ECX, [ESP+18H]
	MOV     EDX, [ESP+14H]
	MOV     EAX, [ESP+10H]
LOC_41A047:
	SHR     EBX, 1
	RCR     ECX, 1
	SHR     EDX, 1
	RCR     EAX, 1
	OR      EBX, EBX
	JNZ     SHORT LOC_41A047
	DIV     ECX
	MOV     ESI, EAX
	MUL     DWORD PTR [ESP+1CH]
	MOV     ECX, EAX
	MOV     EAX, [ESP+18H]
	MUL     ESI
	ADD     EDX, ECX
	JB      SHORT LOC_41A075
	CMP     EDX, [ESP+14H]
	JA      SHORT LOC_41A075
	JB      SHORT LOC_41A076
	CMP     EAX, [ESP+10H]
	JBE     SHORT LOC_41A076
LOC_41A075:                             
	DEC     ESI
LOC_41A076:
	XOR     EDX, EDX
	MOV     EAX, ESI
LOC_41A07A:
	DEC     EDI
	JNZ     SHORT LOC_41A084
	NEG     EDX
	NEG     EAX
	SBB     EDX, 0
LOC_41A084:                             
	POP     EBX
	POP     ESI
	POP     EDI
	RET     10H
}}

NAKED_EXTERN void __cdecl _aulldiv() { __asm {
	PUSH  EBX
	PUSH  ESI
	MOV   EAX, [ESP+18H]
	OR    EAX, EAX
	JNZ   L1
	MOV   ECX, [ESP+14H]
	MOV   EAX, [ESP+10H]
	XOR   EDX, EDX
	DIV   ECX
	MOV   EBX, EAX
	MOV   EAX, [ESP+0CH]
	DIV   ECX
	MOV   EDX, EBX
	JMP   SHORT L2
L1:
	MOV   ECX, EAX
	MOV   EBX, [ESP+14H]
	MOV   EDX, [ESP+10H]
	MOV   EAX, [ESP+0CH]
L3:
	SHR   ECX, 1
	RCR   EBX, 1
	SHR   EDX, 1
	RCR   EAX, 1
	OR    ECX, ECX
	JNZ   SHORT L3
	DIV   EBX
	MOV   ESI, EAX
	MUL   DWORD PTR [ESP+18H]
	MOV   ECX, EAX
	MOV   EAX, [ESP+14H]
	MUL   ESI
	ADD   EDX, ECX
	JB    SHORT L4
	CMP   EDX, [ESP+10H]
	JA    SHORT L4
	JB	  SHORT L5
	CMP   EAX, [ESP+0CH]
	JBE   SHORT L5
L4:
	DEC	  ESI
L5:
	XOR   EDX, EDX
	MOV	  EAX, ESI
L2:
	POP   ESI
	POP   EBX
	RET   10H
}}

/*
;ullshr - long shift right
;
;Purpose:
;       Does a unsigned Long Shift Right (for 8-byte/64-bit ints)
;       Shifts a long right any number of bits.
;
;Entry:
;       EDX:EAX - long value to be shifted
;       CL    - number of bits to shift by
;
;Exit:
;       EDX:EAX - shifted value
;
;Uses:
;       CL is destroyed.
;
;Exceptions:
;
;*******************************************************************************/
NAKED_EXTERN void __cdecl _aullshr() { __asm {
//; Handle shifts of 64 bits or more (if shifting 64 bits or more, the result
//; depends only on the high order bit of edx).
	CMP   CL, 64
	JAE   SHORT RETZERO
// Handle shifts of between 0 and 31 bits
	CMP   CL, 32
	JAE   SHORT MORE32
	SHRD  EAX, EDX, CL
	SHR   EDX,CL
	RET
//; Handle shifts of between 32 and 63 bits
MORE32:
	MOV   EAX, EDX
	XOR   EDX, EDX
	AND   CL, 31
	SHR   EAX, CL
	RET
//; return 0 in edx:eax
RETZERO:
	XOR   EAX, EAX
	XOR   EDX, EDX
	RET
}}
/*
;llshl - long shift left
;
;Purpose:
;       Does a Long Shift Left (signed and unsigned are identical)
;       Shifts a long left any number of bits.
;
;Entry:
;       EDX:EAX - long value to be shifted
;       CL    - number of bits to shift by
;
;Exit:
;       EDX:EAX - shifted value
;
;Uses:
;       CL is destroyed.
;
;Exceptions:
;
;*******************************************************************************/
NAKED_EXTERN void __cdecl _allshl() { __asm {
// Handle shifts of 64 or more bits (all get 0)
	CMP   CL, 64
	JAE   RETZERO
// Handle shifts of between 0 and 31 bits
	CMP   CL, 32
	JAE   MORE32
	SHLD  EDX, EAX, CL
	SHL   EAX, CL
	RET
//; Handle shifts of between 32 and 63 bits
MORE32:
	MOV   EDX, EAX
	XOR   EAX, EAX
	AND   CL, 31
	SHL   EDX, CL
	RET
//; return 0 in edx:eax
RETZERO:
	XOR   EAX, EAX
	XOR   EDX, EDX
	RET
}}

#ifdef __cplusplus
	} // EXTERN "C" END
#endif

#endif
