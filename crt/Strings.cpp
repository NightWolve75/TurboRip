#include <Global.h>

/***
*DWORD StrLen(LPCSTR lpText) x86 - returns length of null-terminated string
*
*Purpose:
*       Finds the length in bytes of the given string, not including
*       the final null character. Powered by Agner Fog, maximum speed
*       in x86 32-bit Assembly using 4-byte-at-a-time find null method!
*Entry:
*       const char * str - string whose length is to be computed
*       Note: ECX has parameter 1 in __fastcall mode (not stack!)
*Exit:
*       length of the string "str", exclusive of the final null byte
*       ECX returns address at null for string appending
*******************************************************************************/
NAKED_EXTERN DWORD __fastcall StrLen(PSTR lpText) { __asm {
;// ECX = lpText
	PUSH  EBX
	MOV   EDX, ECX            ;// Copy lpText ptr for end

LOOP_FIND_NULL:
;//REPEAT 1
	MOV   EBX,[ECX]           ;// Read 4 bytes of string
	ADD   ECX, 4              ;// Increment pointer
	LEA   EAX,[EBX-01010101h] ;// Subtract 1 from each byte
	NOT   EBX                 ;// Invert all bytes
	AND   EAX, EBX            ;// And these two
	TEST  EAX, 80808080h      ;// Test all sign bits
	JNZ   FOUND_NULL
;//REPEAT 2
	MOV   EBX,[ECX]           ;// Read 4 bytes of string
	ADD   ECX, 4              ;// Increment pointer
	LEA   EAX,[EBX-01010101h] ;// Subtract 1 from each byte
	NOT   EBX                 ;// Invert all bytes
	AND   EAX, EBX            ;// And these two
	TEST  EAX, 80808080h      ;// Test all sign bits
	JNZ   FOUND_NULL
;//REPEAT 3
	MOV   EBX,[ECX]           ;// Read 4 bytes of string 
	ADD   ECX, 4              ;// Increment pointer
	LEA   EAX,[EBX-01010101h] ;// Subtract 1 from each byte
	NOT   EBX                 ;// Invert all bytes
	AND   EAX, EBX            ;// And these two
	TEST  EAX, 80808080h      ;// Test all sign bits 
	JNZ   FOUND_NULL
;//REPEAT 4
	MOV   EBX,[ECX]           ;// Read 4 bytes of string 
	ADD   ECX, 4              ;// Increment pointer
	LEA   EAX,[EBX-01010101h] ;// Subtract 1 from each byte
	NOT   EBX                 ;// Invert all bytes
	AND   EAX, EBX            ;// And these two
	TEST  EAX, 80808080h      ;// Test all sign bits 
JZ  LOOP_FIND_NULL            ;// No zero bytes found, continue loop

FOUND_NULL:
	TEST  EAX, 00008080h      ;// Test first two bytes
	JNZ   FINISH_COUNT
	SHR   EAX, 16             ;// Not in the first 2 bytes
	ADD   ECX, 2

FINISH_COUNT:
	SHL   AL, 1               ;// Use carry flag to avoid branch
	LEA   EAX,[ECX-3]
	SBB   EAX, EDX            ;// Compute/return Length (Adds #2 + CF, substracts from #1)
	POP   EBX
	LEA   ECX,[EDX+EAX]       ;// Also return ptr to EOS via ECX for appending
	RET
}}
/***
*char* StrCpy(dest, source) x86 - copy all characters till NULL
*
*Purpose:
*       Fast ASM copy of all characters from the source string
*       to destination moving 4 bytes at-a-time via Agner Fog's
*       advanced null-byte detection technique in a 32-bit value.
*Entry:
*       char *lpDestination - pointer to destination
*       char *lpSource - source string for copy
*Exit:
*       Returns end of dest string, allows for repeat StrCpy() to append substr
*******************************************************************************/
NAKED_EXTERN PSTR __fastcall StrCpy(PSTR lpDestination, PSTR lpSource) { __asm {
// ECX = lpDestination
// EDX = lpSource
	PUSH  ESI                  ;// MUST save/restore
	PUSH  EDI                  ;// MUST save/restore
	MOV   ESI, EDX             ;// lpSource used at end of null search

LOOP_FIND_NULL:
;//REPEAT 1
	MOV   EDI, [EDX]           ;// Read 4 bytes of string 
	ADD   EDX, 4               ;// Increment pointer
	LEA   EAX, [EDI-01010101h] ;// Subtract 1 from each byte
	NOT   EDI                  ;// Invert all bytes
	AND   EAX, EDI             ;// And these two
	TEST  EAX, 80808080h       ;// Test all sign bits 
	JNZ   FOUND_NULL
;//REPEAT 2
	MOV   EDI, [EDX]           ;// Read 4 bytes of string 
	ADD   EDX, 4               ;// Increment pointer
	LEA   EAX, [EDI-01010101h] ;// Subtract 1 from each byte
	NOT   EDI                  ;// Invert all bytes
	AND   EAX, EDI             ;// And these two
	TEST  EAX, 80808080h       ;// Test all sign bits 
	JNZ   FOUND_NULL
;//REPEAT 3
	MOV   EDI, [EDX]           ;// Read 4 bytes of string 
	ADD   EDX, 4               ;// Increment pointer
	LEA   EAX, [EDI-01010101h] ;// Subtract 1 from each byte
	NOT   EDI                  ;// Invert all bytes
	AND   EAX, EDI             ;// And these two
	TEST  EAX, 80808080h       ;// Test all sign bits 
	JNZ   FOUND_NULL
;//REPEAT 4
	MOV   EDI, [EDX]           ;// Read 4 bytes of string 
	ADD   EDX, 4               ;// Increment pointer 
	LEA   EAX, [EDI-01010101h] ;// Subtract 1 from each byte
	NOT   EDI                  ;// Invert all bytes
	AND   EAX, EDI             ;// And these two
	TEST  EAX, 80808080h       ;// Test all sign bits 
JZ  LOOP_FIND_NULL             ;// No zero bytes found, continue loop

FOUND_NULL:
	TEST  EAX, 00008080h       ;// Test first two bytes
	JNZ   FINISH_COUNT
	SHR   EAX, 16              ;// Not in the first 2 bytes of DWORD
	ADD   EDX, 2

FINISH_COUNT:
	SHL   AL, 1        ;// Use carry flag to avoid branch
	SBB   EDX, ESI     ;// EDX - lpSource - Compute length/size + null (+3 extra)
;// [MemCpy] Source string count ready, now copy it to destination
	MOV   EDI, ECX     ;// Load the destination buffer address
	LEA   ECX,[EDX-2]  ;// Load the string size (sub -2 off)

;// Slight improvement of 4K cycles on 65K data if at least 'Destination' is 4/16 aligned!
;// EXCELLENT improvement ~= MOVDQU on 65K data if BOTH ESI/EDI are aligned via _aligned_malloc()
	ALIGN_MEMORY:
		TEST  EDI, 3       ;// Check if Destination is 4-byte aligned
		JZ    BEGIN_COPY   ;// Yes, begin copy; No, do 4-byte align
		MOVSB              ;// Memset 1-3 bytes over till aligned
		SUB   ECX, 1
	JNBE ALIGN_MEMORY

BEGIN_COPY:
	MOV   EAX, ECX     ;// Copy Length for 1-3 trail bytes
	SHR   ECX, 2       ;// Divide Length by 4
	REP   MOVSD        ;// Start DWORD copy ESI to EDI till ECX=0!
	AND   EAX, 3       ;// 1-3 Remainder byte Length
	JNZ   TRAIL_BYTES  ;// 1-3 Trail bytes ? Yes, handle them; No, continue.
;// Return destination buffer @EOS right at null to allow for easy str appending
	LEA   EAX,[EDI-1]  ;// Return ptr to EOS, better idea than returning size!
	POP   EDI          ;// MUST save/restore
	POP   ESI          ;// MUST save/restore
	RET

TRAIL_BYTES:
	MOV   ECX, EAX     ;// Set ECX to 1-3 remaining bytes
	REP   MOVSB        ;// Copy final 1-3 bytes
;// Return destination buffer @EOS right at null to allow for easy str appending
	LEA   EAX,[EDI-1]  ;// Return ptr to EOS, better idea than returning size!
	POP   EDI          ;// MUST save/restore
	POP   ESI          ;// MUST save/restore
	RET
}}
/***
*char *strchr(_Str, _Ch) - search a string for a character
*Purpose:
*       Searches a string for a given character, which may be the
*       null character '\0'.
*Entry:
*       char *_Str - string to search in
*       char _Ch - character to search for
*Exit:
*       returns pointer to the first occurence of c in string
*       returns NULL if c does not occur in string
*******************************************************************************/
#if !defined(_DEBUG) && defined(__cplusplus)

// Terrible! upgrade from strchr.asm !!!

NAKED_EXTERN LPCSTR __cdecl strchr(LPCSTR _Str, int _Ch) { __asm {
	PUSH  EDI  ;// MUST save/restore
;// 1. Get source string size
	XOR   EAX, EAX    ;// AL has the target Null byte to scan for
	MOV   EDI, [ESP+8];// EDI has source string for byte scanning
	MOV   ECX, -1     ;// We're ready to search for Null up to 0xFFFFFFFF times... ;)
	REPNZ SCASB       ;// Scan for Null Byte (up to ECX times)
	NOT   ECX         ;// !ECX now has the string size counting Null
	SUB   EDI, ECX    ;// Go back to the start of the string
	DEC   ECX         ;// Minus Null terminator
	JZ    EXIT_RETURN ;// String size was 0, exit return
;// 2. Scan for char in string
		MOVZX EAX, BYTE PTR [ESP+12] ;// 'AL' has the char to scan/search for
		REPNE SCASB       ;// Begin scanning/searching for char up to ECX times
		JZ    RET_ADDR    ;// If ZF set, we found the char
			XOR   EAX, EAX    ;// ECX=0 and char not found, return null
			JMP   EXIT_RETURN
;// 3. Return results
RET_ADDR:
	LEA   EAX, [EDI-1] ;// Return address where char was found
EXIT_RETURN:
	POP   EDI  ;// MUST save/restore
	RET
}}

#endif
/***
*char *strrchr(string, ch) - find last occurrence of ch in string
*Purpose:
*       Finds the last occurrence of ch in string.  The terminating
*       null character is used as part of the search.
*Entry:
*       char *string - string to search in
*       char ch - character to search for
*Exit:
*       returns a pointer to the last occurrence of ch in the given
*       string
*       returns NULL if ch does not occurr in the string
*******************************************************************************/
#if !defined(_DEBUG) && defined(__cplusplus)

EXTERN LPCSTR __cdecl strrchr(LPCSTR string, int ch) {
	LPSTR start = (LPSTR)string;
	while ( *string++ ) /* find end of string */
		;
	/* search towards front */
	while ( --string != start && *string != (char)ch )
		;
	if ( *string == (char)ch ) /* char found ? */
		return string;
	return NULL;
}

#endif
/*
* InStrRev(string, ch)
* Purpose:
* Returns the position of an occurrence of one string within another, from the end of string. */

NAKED_EXTERN PSTR __fastcall InStrRev(PSTR lpText, int ch) { __asm {
	XOR EAX, EAX
}}



/***
*char *strstr(string1, string2) - search for string2 in string1
*Purpose:
*       finds the first occurrence of string2 in string1
*Entry:
*       char *string1 - string to search in
*       char *string2 - string to search for
*Exit:
*       returns a pointer to the first occurrence of string2 in
*       string1, or NULL if string2 does not occur in string1
*******************************************************************************/

#if !defined(_DEBUG) && defined(__cplusplus)

EXTERN LPCSTR __cdecl strstr(LPCSTR str1, LPCSTR str2) {
	LPSTR cp = (LPSTR) str1;
	LPSTR s1, s2;
	if ( !*str2 )
		return str1;
	while ( *cp ) {
		s1 = cp;
		s2 = (LPSTR)str2;
		while ( *s1 && *s2 && !(*s1-*s2) )
			s1++, s2++;
		if ( !*s2 )
			return cp;
		cp++;
	}
	return NULL;
}

#endif
/***
*int StrCmpI(lpString1, lpString2) - Compare English strings, ignore case
*Version: 1.00
*Purpose:
*       Perform a case-insensitive string comparision via fast 32-bit moves
*       For differences, uppercase letters are lowercased.
*       Thus, "abc_" < "ABCD" since "_" < "d".
*Entry:
*       char *string1, *string2 - strings to compare
*Notes: Uses "(x-0x01010101)&~x&0x80808080" Alan Mycroft fast Null DWORD scan
*Return:
*       <0 if string1 < string2
*        0 if string1 = string2
*       >0 if string1 > string2
* Original C Algorithm/Code (slow 8-bit moves):

int __fastcall StrCmpI(PSTR lpString1, PSTR lpString2) {
	register BYTE A, B;
	for (;;) {
		A = *lpString1;
		B = *lpString2;
		ToLowerCase(A);
		ToLowerCase(B);
		if ( A && B && A == B ) {
			lpString1++;
			lpString2++;
			continue;
		}
		return A - B;
	}
}
*******************************************************************************/
NAKED_EXTERN long __fastcall StrCmpI(PSTR lpString1, PSTR lpString2) { __asm {
// ECX = lpString1
// EDX = lpString2
	PUSH  ESI
	PUSH  EDI
	PUSH  EBX

LOOP_STRCMPI:
;// REPEAT 1

	MOV   EAX, [ECX]     ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX]     ;// EBX = BBBB; Load 4 chars from lpString2
;// Find NULL first in AAAA 
	MOV   ESI, EAX             ;// Copy AAAA for NULL test
	LEA   EDI, [EAX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL1          ;// Handle NULL exit when found
;// Find NULL first in BBBB
	MOV   ESI, EBX             ;// Copy BBBB for NULL test
	LEA   EDI, [EBX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL2          ;// Handle NULL exit when found
;// Force lowercase for case-neutral comparison. Avoid check if byte
;// is A-Z for speed; OR'ing a bit on non-alpha won't matter for CMP!
	OR    EAX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
	OR    EBX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-insensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
	ADD   ECX, 4         ;// lpString1 += 4;
	ADD   EDX, 4         ;// lpString2 += 4;	

;// REPEAT 2

	MOV   EAX, [ECX]     ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX]     ;// EBX = BBBB; Load 4 chars from lpString2
;// Find NULL first in AAAA 
	MOV   ESI, EAX             ;// Copy AAAA for NULL test
	LEA   EDI, [EAX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL1          ;// Handle NULL exit when found
;// Find NULL first in BBBB
	MOV   ESI, EBX             ;// Copy BBBB for NULL test
	LEA   EDI, [EBX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL2          ;// Handle NULL exit when found
;// Force lowercase for case-neutral comparison. Avoid check if byte
;// is A-Z for speed; OR'ing a bit on non-alpha won't matter for CMP!
	OR    EAX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
	OR    EBX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-insensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
	ADD   ECX, 4         ;// lpString1 += 4;
	ADD   EDX, 4         ;// lpString2 += 4;

;// REPEAT 3

	MOV   EAX, [ECX]     ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX]     ;// EBX = BBBB; Load 4 chars from lpString2
;// Find NULL first in AAAA 
	MOV   ESI, EAX             ;// Copy AAAA for NULL test
	LEA   EDI, [EAX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL1          ;// Handle NULL exit when found
;// Find NULL first in BBBB
	MOV   ESI, EBX             ;// Copy BBBB for NULL test
	LEA   EDI, [EBX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL2          ;// Handle NULL exit when found
;// Force lowercase for case-neutral comparison. Avoid check if byte
;// is A-Z for speed; OR'ing a bit on non-alpha won't matter for CMP!
	OR    EAX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
	OR    EBX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-insensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
	ADD   ECX, 4         ;// lpString1 += 4;
	ADD   EDX, 4         ;// lpString2 += 4;

;// REPEAT 4

	MOV   EAX, [ECX]     ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX]     ;// EBX = BBBB; Load 4 chars from lpString2
;// Find NULL first in AAAA 
	MOV   ESI, EAX             ;// Copy AAAA for NULL test
	LEA   EDI, [EAX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL1          ;// Handle NULL exit when found
;// Find NULL first in BBBB
	MOV   ESI, EBX             ;// Copy BBBB for NULL test
	LEA   EDI, [EBX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL2          ;// Handle NULL exit when found
;// Force lowercase for case-neutral comparison. Avoid check if byte
;// is A-Z for speed; OR'ing a bit on non-alpha won't matter for CMP!
	OR    EAX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
	OR    EBX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-insensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
	ADD   ECX, 4         ;// lpString1 += 4;
	ADD   EDX, 4         ;// lpString2 += 4;

JMP LOOP_STRCMPI         ;// continue;

FOUND_NULL1:
	CMP   EAX, EBX
	JE    EXIT_EQUAL
;// Search NULL in low bytes
	TEST  AL, AL
	JZ    EXIT_RETURN1
	OR    AL, 20h
	TEST  BL, BL
	JZ    EXIT_RETURN1
	OR    BL, 20h
	CMP   AL, BL ;// if ( AL != BL ) EXIT_RETURN1
	JNE   EXIT_RETURN1
;// Search NULL in high bytes
	TEST  AH, AH
	JZ    EXIT_RETURN2
	OR    AH, 20h
	TEST  BH, BH
	JZ    EXIT_RETURN2
	OR    BH, 20h
	CMP   AH, BH ;// if ( AH != BH ) EXIT_RETURN2
	JNE   EXIT_RETURN2
	BSWAP EAX    ;// May be faster than "SHR EAX,16" on most CPUs
	BSWAP EBX
;// Search NULL in high bytes
	TEST  AH, AH
	JZ    EXIT_RETURN2
	OR    AH, 20h
	TEST  BH, BH
	JZ    EXIT_RETURN2
	OR    BH, 20h
	CMP   AH, BH ;// if ( AH != BH ) EXIT_RETURN2
	JNE   EXIT_RETURN2
;// Final BYTE, AL must be NULL (or both AL/BL = NULL)
	TEST  BL, BL
	JZ    EXIT_RETURN1
	OR    BL, 20h ;// if not NULL, lowercase it for consistency
	JMP   EXIT_RETURN1

FOUND_NULL2:
	CMP   EAX, EBX
	JE    EXIT_EQUAL
	OR    EAX, 20202020h ;// We know NULL isn't in EAX here, only in EBX
;// Search NULL in low byte
	TEST  BL, BL
	JZ    EXIT_RETURN1
	OR    BL, 20h
	CMP   AL, BL ;// if ( AL != BL )
	JNE   EXIT_RETURN1
;// Search NULL in high byte
	TEST  BH, BH
	JZ    EXIT_RETURN2
	OR    BH, 20h
	CMP   AH, BH ;// if ( AH != BH )
	JNE   EXIT_RETURN2
	BSWAP EAX    ;// May be faster than "SHR EAX,16" on most CPUs
	BSWAP EBX
;// Search NULL in high byte
	TEST  BH, BH
	JZ    EXIT_RETURN2
	OR    BH, 20h
	CMP   AH, BH ;// if ( AH != BH )
	JNE   EXIT_RETURN2
;// Final BYTE, BL must be NULL
	JMP   EXIT_RETURN1

EXIT_EQUAL:
	XOR   EAX, EAX ;// return 0; (A == B)
	POP   EBX
	POP   EDI
	POP   ESI
	RET

FOUND_DIFF:
;// Is different byte in HIWORD or LOWORD ?
	CMP   AX, BX
	JNE   DIFF_LOWORD
;// Shift HIWORD 16-bits down to LOWORD 16-bits for access
	BSWAP EAX    ;// May be faster than "SHR EAX,16" on most CPUs
	BSWAP EBX
	CMP   AH, BH ;// if ( AH != BH )
	JNE   EXIT_RETURN2
	JMP   EXIT_RETURN1 // AL != BL for sure, so exit1
DIFF_LOWORD:
	CMP   AL, BL ;// if ( AL != BL )
	JNE   EXIT_RETURN1

;// EAX<0 if A<B,  EAX>0 if A>B, or EAX=0 if A==B
EXIT_RETURN2:
		MOVZX EAX, AH
		MOVZX EDX, BH
		SUB   EAX, EDX    ;// Char A[1] - Char B[1];
		POP   EBX
		POP   EDI
		POP   ESI
		RET   ;// return (A[1] - B[1]);
;// EAX<0 if A<B,  EAX>0 if A>B, or EAX=0 if A==B
EXIT_RETURN1:
	MOVZX EAX, AL
	MOVZX EDX, BL
	SUB   EAX, EDX    ;// Char A - Char B;
	POP   EBX
	POP   EDI
	POP   ESI
	RET   ;// return (A - B);
}}

/***
*int StrNCmpI(lpString1, lpString2, Count) - Compare English strings, ignore case
*Version: 1.00
*Purpose:
*       Perform a case-insensitive string comparision via fast 32-bit moves N times
*       For differences, uppercase letters are lowercased.
*       Thus, "abc_" < "ABCD" since "_" < "d".
*Entry:
*       char *string1, *string2 - strings to compare, count - length in bytes
*Return:
*       <0 if string1 < string2
*        0 if string1 = string2
*       >0 if string1 > string2
* Original C Algorithm/Code (slow 8-bit moves):

extern long __fastcall StrNCmpI(PSTR lpString1, PSTR lpString2, DWORD Count) {
	BYTE A, B;
	for (;;) {
		A = *lpString1;
		B = *lpString2;
		ToLowerCase(A);
		ToLowerCase(B);
		if ( Count && A && B && A == B ) {
			Count--;
			if ( Count ) {
				lpString1++;
				lpString2++;
				continue;
			}
		}
		return A - B;
	}
}
*******************************************************************************/

NAKED_EXTERN long __fastcall StrNCmpI(PSTR lpString1, PSTR lpString2, DWORD Count) { __asm {
;// lpString1 = ECX 
;// lpString2 = EDX
;// Count     =[ESP+4 + PUSH32*n]

	PUSH  ESI
	PUSH  EBX
	MOV   ESI, [ESP+4+4*2]

LOOP_STRNCMPI:
;// REPEAT 1

	MOV   EAX, [ECX]     ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX]     ;// EBX = BBBB; Load 4 chars from lpString2
;// Force lowercase for case-neutral comparison. Avoid check if byte
;// is A-Z for speed; OR'ing a bit on non-alpha won't matter for CMP!
	OR    EAX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
	OR    EBX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-insensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
;// Counter -= 4;
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL

;// REPEAT 2

	MOV   EAX, [ECX+4]   ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX+4]   ;// EBX = BBBB; Load 4 chars from lpString2
;// Force lowercase for case-neutral comparison. Avoid check if byte
;// is A-Z for speed; OR'ing a bit on non-alpha won't matter for CMP!
	OR    EAX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
	OR    EBX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-insensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
;// Counter -= 4;
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL

;// REPEAT 3

	MOV   EAX, [ECX+8]   ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX+8]   ;// EBX = BBBB; Load 4 chars from lpString2
;// Force lowercase for case-neutral comparison. Avoid check if byte
;// is A-Z for speed; OR'ing a bit on non-alpha won't matter for CMP!
	OR    EAX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
	OR    EBX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-insensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
;// Counter -= 4;
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL

;// REPEAT 4

	MOV   EAX, [ECX+12]  ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX+12]  ;// EBX = BBBB; Load 4 chars from lpString2
;// Force lowercase for case-neutral comparison. Avoid check if byte
;// is A-Z for speed; OR'ing a bit on non-alpha won't matter for CMP!
	OR    EAX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
	OR    EBX, 20202020h ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-insensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
;// Counter -= 4;
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	ADD   ECX, 16         ;// lpString1 += 16;
	ADD   EDX, 16         ;// lpString2 += 16;	

JMP LOOP_STRNCMPI         ;// continue;

EXIT_EQUAL:
	XOR   EAX, EAX ;// return 0; (A == B)
	POP   EBX
	POP   ESI
	RET   4

FOUND_DIFF:
	CMP   AL, BL ;// if ( AL != BL )
	JE    NEXT_CHAR2
	JMP   EXIT_RETURN1
	
NEXT_CHAR2:
	SUB   ESI, 1
	JZ    EXIT_EQUAL
	CMP   AH, BH ;// if ( AH != BH )
	JE    NEXT_CHAR3
	JMP   EXIT_RETURN2

NEXT_CHAR3:
	SUB   ESI, 1
	JZ    EXIT_EQUAL

;// Shift HIWORD 16-bits down to LOWORD 16-bits for access
	BSWAP EAX    ;// May be faster than "SHR EAX,16" on most CPUs
	BSWAP EBX

	CMP   AH, BH ;// if ( AH != BH )
	JE    NEXT_CHAR4
	JMP   EXIT_RETURN2

NEXT_CHAR4:
	SUB   ESI, 1
	JZ    EXIT_EQUAL
;// AL != BL - Final char of 4, difference is here, no need for CMP
	JMP   EXIT_RETURN1

;// EAX<0 if A<B,  EAX>0 if A>B, or EAX=0 if A==B
EXIT_RETURN2:
	MOVZX ECX, AH
	MOVZX EDX, BH
	SUB   ECX, EDX    ;// Char A[1] - Char B[1];
	POP   EBX
	POP   ESI
	RET   4 ;// return (A[1] - B[1]);

;// EAX<0 if A<B,  EAX>0 if A>B, or EAX=0 if A==B
EXIT_RETURN1:
	MOVZX ECX, AL
	MOVZX EDX, BL
	SUB   ECX, EDX    ;// Char A - Char B;
	POP   EBX
	POP   ESI
	RET   4 ;// return (A - B);
}}

/***
*int StrCmp(lpString1, lpString2) - compare strings, case-sensitive
*Purpose:
*       strcmp perform fast string comparision.
*Entry:
*       char *string1, *string2 - strings to compare
*Return:
*       <0 if string1 < string2
*        0 if string1 = string2
*       >0 if string1 > string2
* C Algorithm/Code:

long __fastcall StrCmp(PSTR lpString1, PSTR lpString2) {
	BYTE A, B;
	for (;;) {
		A = *lpString1;
		B = *lpString2;
		if ( A && B && A == B ) {
			lpString1++;
			lpString2++;
			continue;
		}
		return A - B;
	}
}
*******************************************************************************/
NAKED_EXTERN long __fastcall StrCmp(PSTR lpString1, PSTR lpString2) { __asm {
// ECX = lpString1
// EDX = lpString2
	PUSH  ESI
	PUSH  EDI
	PUSH  EBX

LOOP_STRCMPI:
;// REPEAT 1

	MOV   EAX, [ECX]     ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX]     ;// EBX = BBBB; Load 4 chars from lpString2
;// Find NULL first in AAAA 
	MOV   ESI, EAX             ;// Copy AAAA for NULL test
	LEA   EDI, [EAX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL1          ;// Handle NULL exit when found
;// Find NULL first in BBBB
	MOV   ESI, EBX             ;// Copy BBBB for NULL test
	LEA   EDI, [EBX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL2          ;// Handle NULL exit when found
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-sensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
	ADD   ECX, 4         ;// lpString1 += 4;
	ADD   EDX, 4         ;// lpString2 += 4;	

;// REPEAT 2

	MOV   EAX, [ECX]     ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX]     ;// EBX = BBBB; Load 4 chars from lpString2
;// Find NULL first in AAAA 
	MOV   ESI, EAX             ;// Copy AAAA for NULL test
	LEA   EDI, [EAX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL1          ;// Handle NULL exit when found
;// Find NULL first in BBBB
	MOV   ESI, EBX             ;// Copy BBBB for NULL test
	LEA   EDI, [EBX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL2          ;// Handle NULL exit when found
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-sensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
	ADD   ECX, 4         ;// lpString1 += 4;
	ADD   EDX, 4         ;// lpString2 += 4;

;// REPEAT 3

	MOV   EAX, [ECX]     ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX]     ;// EBX = BBBB; Load 4 chars from lpString2
;// Find NULL first in AAAA 
	MOV   ESI, EAX             ;// Copy AAAA for NULL test
	LEA   EDI, [EAX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL1          ;// Handle NULL exit when found
;// Find NULL first in BBBB
	MOV   ESI, EBX             ;// Copy BBBB for NULL test
	LEA   EDI, [EBX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL2          ;// Handle NULL exit when found
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-sensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
	ADD   ECX, 4         ;// lpString1 += 4;
	ADD   EDX, 4         ;// lpString2 += 4;

;// REPEAT 4

	MOV   EAX, [ECX]     ;// EAX = AAAA; Load 4 chars from lpString1
	MOV   EBX, [EDX]     ;// EBX = BBBB; Load 4 chars from lpString2
;// Find NULL first in AAAA 
	MOV   ESI, EAX             ;// Copy AAAA for NULL test
	LEA   EDI, [EAX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL1          ;// Handle NULL exit when found
;// Find NULL first in BBBB
	MOV   ESI, EBX             ;// Copy BBBB for NULL test
	LEA   EDI, [EBX-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EDI, ESI             ;// And these two
	TEST  EDI, 80808080h       ;// Test all sign bits
	JNZ   FOUND_NULL2          ;// Handle NULL exit when found
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX       ;// Case-sensitive compare 4 chars at a time!
	JNE   FOUND_DIFF     ;// Handle difference if found!
	ADD   ECX, 4         ;// lpString1 += 4;
	ADD   EDX, 4         ;// lpString2 += 4;

JMP LOOP_STRCMPI         ;// continue;

FOUND_NULL1:
	CMP   EAX, EBX
	JE    EXIT_EQUAL
;// Search NULL in low bytes
	TEST  AL, AL
	JZ    EXIT_RETURN1
	TEST  BL, BL
	JZ    EXIT_RETURN1
	CMP   AL, BL ;// if ( AL != BL ) EXIT_RETURN1
	JNE   EXIT_RETURN1
;// Search NULL in high bytes
	TEST  AH, AH
	JZ    EXIT_RETURN2
	TEST  BH, BH
	JZ    EXIT_RETURN2
	CMP   AH, BH ;// if ( AH != BH ) EXIT_RETURN2
	JNE   EXIT_RETURN2
	BSWAP EAX    ;// Faster than "SHR EAX,16" on most CPUs
	BSWAP EBX
;// Search NULL in high bytes
	TEST  AH, AH
	JZ    EXIT_RETURN2
	TEST  BH, BH
	JZ    EXIT_RETURN2
	CMP   AH, BH ;// if ( AH != BH ) EXIT_RETURN2
	JNE   EXIT_RETURN2
;// Final BYTE, AL must be NULL (or both AL/BL = NULL)
	JMP   EXIT_RETURN1

FOUND_NULL2:
	CMP   EAX, EBX
	JE    EXIT_EQUAL
;// Search NULL in low byte
	TEST  BL, BL
	JZ    EXIT_RETURN1
	CMP   AL, BL ;// if ( AL != BL )
	JNE   EXIT_RETURN1
;// Search NULL in high byte
	TEST  BH, BH
	JZ    EXIT_RETURN2
	CMP   AH, BH ;// if ( AH != BH )
	JNE   EXIT_RETURN2
	BSWAP EAX    ;// May be faster than "SHR EAX,16" on most CPUs
	BSWAP EBX
;// Search NULL in high byte
	TEST  BH, BH
	JZ    EXIT_RETURN2
	CMP   AH, BH ;// if ( AH != BH )
	JNE   EXIT_RETURN2
;// Final BYTE, BL must be NULL (or both AL/BL = NULL)
	JMP   EXIT_RETURN1

EXIT_EQUAL:
	XOR   EAX, EAX ;// return 0; (A == B)
	POP   EBX
	POP   EDI
	POP   ESI
	RET

FOUND_DIFF:
;// Is different byte in HIWORD or LOWORD ?
	CMP   AX, BX
	JNE   DIFF_LOWORD
;// Shift HIWORD 16-bits down to LOWORD 16-bits for access
	BSWAP EAX    ;// May be faster than "SHR EAX,16" on most CPUs
	BSWAP EBX
	CMP   AH, BH ;// if ( AH != BH )
	JNE   EXIT_RETURN2
	JMP   EXIT_RETURN1 // AL != BL for sure, so exit1
DIFF_LOWORD:
	CMP   AL, BL ;// if ( AL != BL )
	JNE   EXIT_RETURN1

;// EAX<0 if A<B,  EAX>0 if A>B, or EAX=0 if A==B
EXIT_RETURN2:
		MOVZX EAX, AH
		MOVZX EDX, BH
		SUB   EAX, EDX    ;// Char A[1] - Char B[1];
		POP   EBX
		POP   EDI
		POP   ESI
		RET   ;// return (A[1] - B[1]);
;// EAX<0 if A<B,  EAX>0 if A>B, or EAX=0 if A==B
EXIT_RETURN1:
	MOVZX EAX, AL
	MOVZX EDX, BL
	SUB   EAX, EDX    ;// Char A - Char B;
	POP   EBX
	POP   EDI
	POP   ESI
	RET   ;// return (A - B);
}}
/*
* Todo: Rewrite in ASM
* lpFormattedNumber should point to an allocated char array of at least 16 bytes
*/
extern void __fastcall FormatNumber(DWORD Number, PSTR lpFormattedNumber) {
	char szNumberBuffer[16]; // Maximum: "4,294,967,295" + \0x00
	BYTE CommaTrigger;
	PSTR lpNumber;
	DWORD nSize;
	BYTE Commas;
	nSize = LongToStr(Number, szNumberBuffer, 0, 0);
	if ( nSize == 10 )
		Commas = 3; // Need 3 commas+null
	else if ( nSize > 6 )
		Commas = 2; // Need 2 commas+null
	else if ( nSize > 3 )
		Commas = 1; // Need 1 comma+null
	else { // No commas needed (0 to 999), so copy to buffer and return!
		*(LPDWORD)lpFormattedNumber = *(LPDWORD)szNumberBuffer;
		return;
	}
	lpNumber = szNumberBuffer + nSize - 1;
	lpFormattedNumber += (nSize + Commas);
	*lpFormattedNumber-- = '\0';
	for ( CommaTrigger = 0; nSize; nSize-- ) {
		if ( CommaTrigger == 3 ) {
			*lpFormattedNumber-- = ',';
			CommaTrigger = 0;
		}
		*lpFormattedNumber-- = *lpNumber--;
		CommaTrigger++;
	}
}
/*
 * [Hex or Digit] StringToNumber by David Shadoff
 */
extern DWORD __fastcall StringToNumber(PSTR lpText) {
	DWORD ret, radix, currval, tempint;
	PSTR pChar;
	BYTE c;
	radix = 10;
	ret = tempint = currval = 0;
	if ( *(PWORD)lpText == 'x0' ) {
		radix = 16;
		lpText += 2;
	} else
		for ( pChar = lpText; (c = *pChar) != 0; pChar++ )
			if ( !(( c < 'A' || c > 'F' ) && ( c < 'a' || c > 'f' )) ) {
				radix = 16;
				break;
			}
	while ( (c = *lpText) != 0 ) {
		if ( IsDigit(c) ) {
			tempint = (DWORD) (c - '0');
		} else if ( c == ',' ) {
			lpText++;
			continue;
		} else if ( radix == 16 ) {
			if ( !( c < 'A' || c > 'F' ) )
				tempint = (DWORD) (c - 'A') + 10;
			else if ( !( c < 'a' || c > 'f' ) )
				tempint = (DWORD) (c - 'a') + 10;
		} else {
			return currval;
		}
		currval *= radix;
		currval += tempint;
		lpText++;
	}
	return currval;
}

NAKED_EXTERN DWORD __fastcall HexToDec(PSTR lpText) { __asm { // 100%
// ECX = lpText
	PUSH  EBX
	XOR   EAX, EAX      ;// Sum = 0;
;// for ( Sum = 0; (c = *lpText) != 0; lpText++ ) {
HEXTODEC_LOOP:
	MOVZX EBX, BYTE PTR [ECX] ;// c = *lpText;
	TEST  BL, BL
	JZ    EXIT_RETURN   ;// ( c == 0 ) ? yes, exit/return; no, loop for more hex chars
	ADD   ECX, 1        ;// lpText++
	CMP   BL, ' '       ;// Skip all whitespace chars (\r\n\t\b ), Range: 0x01-0x20
	JBE   HEXTODEC_LOOP
;// IsNonDigit(c) - if ( c < '0' || c > '9' ) then skip it!
	CMP   BL, '0'       ;// ( c < '0' ) ?
	JB    NON_DIGIT     ;// if yes, skip it, it's nondigit
	CMP   BL, '9'       ;// ( c > '9' ) ?
	JA    NON_DIGIT     ;// if yes, skip it, it's notdigit
	;// It's a 0-9 digit, add it to Sum!
		SHL   EAX, 4    ;// Sum = Sum << 4; Or (Sum *= 16);
		LEA   EAX, [EAX+EBX-'0'] ;// Sum += (c - '0');
		JMP   HEXTODEC_LOOP      ;// Loop again
NON_DIGIT:
	OR    BL, 20h      ;// c = c | 0x20; MakeLowerCase(c)
;// IsHexDigit(c) -  if ( c < 'a' || c > 'f' ) then exit/return!
	CMP   BL, 'a'      ;// c < 'a' ?
	JB    EXIT_RETURN  ;// if yes, it's non-hex, exit/return
	CMP   BL, 'f'      ;// c > 'f' ?
	JA    EXIT_RETURN  ;// if yes, it's non-hex, exit/return
	;// It's A-F hex, add it to Sum!
		SHL   EAX, 4   ;// Sum = Sum << 4; Or (Sum *= 16);
		LEA   EAX, [EAX+EBX-'a'+10] ;// Sum += (c - 'a') + 10;
JMP HEXTODEC_LOOP      ;// Loop again

EXIT_RETURN:
	POP  EBX
	RET
}}

NAKED_EXTERN DWORD __fastcall HexToDecN(DWORD Count, PSTR lpText) { __asm { // 100%
// ECX = Count
// EDX = lpText
	PUSH  EBX
	XOR   EAX, EAX      ;// Sum = 0;
;// for ( Sum = 0; (c = *lpText) != 0; lpText++ ) {
HEXTODEC_LOOP:
	MOVZX EBX, BYTE PTR [EDX] ;// c = *lpText;
	TEST  BL, BL
	JZ    EXIT_RETURN   ;// ( c == 0 ) ? yes, exit/return; no, loop for more hex chars
	ADD   EDX, 1        ;// lpText++
	CMP   BL, ' '       ;// Skip all whitespace chars (\r\n\t\b ), Range: 0x01-0x20
	JBE   HEXTODEC_LOOP
;// IsNonDigit(c) - if ( c < '0' || c > '9' ) then skip it!
	CMP   BL, '0'       ;// ( c < '0' ) ?
	JB    NON_DIGIT     ;// if yes, skip it, it's nondigit
	CMP   BL, '9'       ;// ( c > '9' ) ?
	JA    NON_DIGIT     ;// if yes, skip it, it's nondigit
	;// It's 0-9 digit, add it to Sum!
		SHL   EAX, 4    ;// Sum = Sum << 4; Or (Sum *= 16);
		LEA   EAX, [EAX+EBX-'0'] ;// Sum += (c - '0');
		SUB   ECX, 1
		JZ    EXIT_RETURN
		JMP   HEXTODEC_LOOP ;// Loop again
NON_DIGIT:
	OR    BL, 20h      ;// c = c | 0x20; MakeLowerCase(c)
;// IsHexDigit(DL) -  if ( c < 'a' || c > 'f' ) then exit/return!
	CMP   BL, 'a'      ;// c < 'a' ?
	JB    EXIT_RETURN  ;// if yes, it's non-hex, exit/return
	CMP   BL, 'f'      ;// c > 'f' ?
	JA    EXIT_RETURN  ;// if yes, it's non-hex, exit/return
	;// It's A-F hex, add it to Sum!
		SHL   EAX, 4   ;// Sum = Sum << 4; Or (Sum *= 16);
		LEA   EAX, [EAX+EBX-'a'+10] ;// Sum += (c - 'a') + 10;
		SUB   ECX, 1
		JZ    EXIT_RETURN
JMP HEXTODEC_LOOP ;// Loop again

EXIT_RETURN:
	POP  EBX
	RET
}}
/***
*DWORD LongToStr, NumberToStr, NumberToString(val, lpTextBuffer) - 
*
*Purpose:
*       Converts a number to an ASCII character string.
*Entry:
*       Number - number to be converted (int, long or unsigned long)
*       lpTextBuffer - ptr to buffer to place result
*       nPadWidth - Non-zero to pad result with '0' or ' ' typically
*       cPadChar - The character to pad with (e.g. '0' or ' ')
*Remarks:
*       Text buffer should be at least 15 (16 for align!) bytes in size
*       for long data range: "–2,147,483,648" to "2,147,483,647" + "\0"
*******************************************************************************/
extern DWORD __fastcall LongToStr(IN long Number, OUT PSTR lpTextBuffer, BYTE nPadWidth, char cPadChar) {
	DWORD nChars;  // Count total characters returned
	PSTR putDigit; // pointer to traverse string
	char cDigit;   // Value of digit/temp
// If negative requested, output '-' and negate
	if ( Number < 0 ) {
		nChars = 1;
		Number = -Number;
		*lpTextBuffer++ = '-';
	} else
		nChars = 0;
	putDigit = lpTextBuffer;
	do {
		cDigit = (char)(Number % 10); // Get digit
		*putDigit++ = (cDigit + '0'); // Convert to ASCII code and store
		Number /= 10;   // Move to next digit
		nChars++;       // Count every digit
	} while ( Number );
// Pad if requested either ' ' or '0'
	while ( (BYTE)nChars < nPadWidth ) {
		*putDigit++ = cPadChar; // general pad either with ' ' or '0', 1% --> 001%
		nChars++;
	}
	*putDigit = '\0';
/* We now have the digits of the number in the buffer,
   but in reverse order.  Thus we reverse them now. */
	putDigit--;            /* ; p points to last digit */
	do {
		cDigit = *putDigit;
		*putDigit = *lpTextBuffer;
		*lpTextBuffer = cDigit;  /* swap *p and *firstdig */
		lpTextBuffer++;          /* advance to next two digits */
		putDigit--;
	} while ( lpTextBuffer < putDigit ); /* repeat until halfway */
	return nChars;
}
/***
* DWORD DWORDToStr(IN DWORD Number, OUT PSTR lpTextBuffer, BYTE nPadWidth, char cPadChar)
* Version: 0.99 - Finish return value as digit string count!
*
*Purpose:
*       Converts a DWORD Number to ASCII character string.
*Entry:
*       Number - Number to be converted (DWORD/unsigned long)
*       lpTextBuffer - Pointer to buffer to return result
*       nPadWidth - Pad size (1-15 max), guarantees # chars returned. e.g. 10 returns "0000000001" for 1 if cPadChar = '0'
*       cPadChar  - Either ' ' or '0' typically. Pass 0 for both nPadWidth/cPadChar to avoid use
*Remarks:
*       Text buffer MUST be 12 bytes minimum (12/16 for align!) in size
*       for DWORD data range (10 digits + NULL + NULL): 0 to "4294967295" + "\0"
*******************************************************************************/
NAKED_EXTERN DWORD __fastcall DWORDToStr(IN DWORD Number, OUT PSTR lpTextBuffer, BYTE nPadWidth, char cPadChar) { __asm {
;// Number       = ECX
;// lpTextBuffer = EDX
;// nPadWidth    = [ESP+4 + PUSH32*n]
;// cPadChar     = [ESP+8 + PUSH32*n]
	PUSH  ESI
	PUSH  EDI
	MOV   ESI, EDX     ;// (putDigit = lpTextBuffer;) // Pointer to traverse string
	XOR   EAX, EAX

;// 1) Get Digit Count in advance

	CMP   ECX, 10         ;// if (n < 10) Digits = 1;
	JB    L1
	CMP   ECX, 100        ;// if (n < 100) Digits = 2;
	JB    L2
	CMP   ECX, 1000       ;// if (n < 1000) Digits = 3;
	JB    L3
	CMP   ECX, 10000      ;// if (n < 10000) Digits = 4;
	JB    L4
	CMP   ECX, 100000     ;// if (n < 100000) Digits = 5;
	JB    L5
	CMP   ECX, 1000000    ;// if (n < 1000000) Digits = 6;
	JB    L6
	CMP   ECX, 10000000   ;// if (n < 10000000) Digits = 7;
	JB    L7
	CMP   ECX, 100000000  ;// if (n < 100000000) Digits = 8;
	JB    L8
	CMP   ECX, 1000000000 ;// if (n < 1000000000) Digits = 9;
	JB    L9
	MOV   AL, 10          ;// else Digits = 10;
	JMP   DIGIT_COUNT
L1:	MOV   AL, 1          ;// Digits = 1;
	JMP   DIGIT_COUNT
L2: MOV   AL, 2          ;// Digits = 2;
	JMP   DIGIT_COUNT
L3: MOV   AL, 3          ;// Digits = 3;
	JMP   DIGIT_COUNT
L4: MOV   AL, 4          ;// Digits = 4;
	JMP   DIGIT_COUNT
L5: MOV   AL, 5          ;// Digits = 5;
	JMP   DIGIT_COUNT
L6: MOV   AL, 6          ;// Digits = 6;
	JMP   DIGIT_COUNT
L7: MOV   AL, 7          ;// Digits = 7;
	JMP   DIGIT_COUNT
L8: MOV   AL, 8          ;// Digits = 8;
	JMP   DIGIT_COUNT
L9: MOV   AL, 9          ;// Digits = 9;
DIGIT_COUNT:

;// 2) Pad with spaces ' ' or '0' if requested

	MOVZX EDI, BYTE PTR [ESP+4 + 4*2] ;// EDI = nPadWidth
	CMP   EAX, EDI         ;// ( DigitCount >= nPadWidth ) ??
	JAE   NO_PADDING_INDEX ;// Skip; no need for padding
		AND   DI, 0Fh      ;// (1-15 max) Prevent buffer overrun in case wild argument sent
		MOVZX EAX, BYTE PTR [ESP+8 + 4*2] ;// AL = cPadChar
		ADD   ESI, EDI     ;// Pad Chars > Digits, set index to where digits should be written right to left
		MOV   AH, AL       ;// AX=Fill/Fill | cPadChar/cPadChar
		SHR   EDI, 1       ;// Divide Counter/2
		JZ    PAD_BUFFER2
		PAD_BUFFER1:
			MOV   WORD PTR [EDX], AX    ;// Pre-pad 16-bits per iteration, good enough
			ADD   EDX, 2
			SUB   EDI, 1
		JNZ PAD_BUFFER1
	PAD_BUFFER2:
		MOV   WORD PTR [ESI], DI ;// NULL terminate at exact final byte x 2
		JMP   LOOP1

NO_PADDING_INDEX:
	ADD   ESI, EAX           ;// Position index to where digits should be written right to left
	MOV   BYTE PTR [ESI], AH ;// NULL terminate

;// 3) FINAL: Compute number/10 & number%10, encode to ASCII string

LOOP1:
;// Multiplicative Inverse method to achieve both n/10 & n%10 - Agner Fog protip!
;// DIV is slow, idea is to replace say (n/10) with (n * 1/10) instead!
	MOV   EAX, 0CCCCCCCDh      ;// Fixed-point approximation magic, multiply n by (2^35/10 + 1)
	MUL   ECX                  ;// n * (2^35 / 10 + 1); results in EDX & EAX
	SHR   EDX, 3               ;// EDX = n / 10;
;// Achieve (n % 10) with [n - (n/10 * 10)]
	LEA   EAX, [EDX*4+EDX]     ;// EAX = (n * 4 + n) = (n * 5)
	SUB   ESI, 1               ;// lpPutDigit--;
	NEG   EAX                  ;// -(n * 5); so we can subtract remainder with LEA add-only limits!
	LEA   EAX, [ECX+EAX*2+'0'] ;// Convert to ASCII code! [n + -([n*5] * 2)] + '0'
	MOV   BYTE PTR [ESI], AL   ;// *lpPutDigit = ch;
	MOV   ECX, EDX
	TEST  EDX, EDX
JNZ LOOP1

;// .99 - Finish EAX as returning string count somehow!

	POP   EDI
	POP   ESI
	RET   8
}}
/***
*long atol/StrToLong(char *str) - Convert string to long
*
*Purpose:
*       Converts ASCII string pointed to by lpText to binary.
*       Overflow is not detected.
*Entry:
*       lpText = ptr to string to convert
*Exit:
*       return long decimal int value of the string
*******************************************************************************/
NAKED_EXTERN long __fastcall StrToLong(PSTR lpText) { __asm {
;// lpText = ECX
	SUB   ESP, 4       ;// BOOL bIsNeg;
	XOR   EAX, EAX     ;// Total = 0
	MOV  [ESP], EAX    ;// bIsNeg/[ESP+4] = True if negative '-' sign was used

;// Slide to 1st digit or EOS & check for '-' to negate result!
;// (Slides past any \s\t\r\n\c chars)

LOOP1:

	MOVZX EDX, BYTE PTR [ECX] ;// c = *lpText;
	TEST  EDX, EDX            ;// c == NULL ? yes, exit loop; no, continue
	JZ    EXIT_RETURN2
	ADD   ECX, 1         ;// lpText++
	CMP   DL, '-'        ;// c == '-' ? yes, mark negative; no, continue
	JNE   L1
		MOV   [ESP], EDX ;// Mark as negative
		JMP   LOOP1      ;// Loop again
	L1:
	CMP   DL, '0'
	JB    LOOP1          ;// Not digit, get next char
	CMP   DL, '9'

JA LOOP1                 ;// Not digit, get next char	

;// Found a digit, start adding!

MAIN_LOOP:

;// Add digit to the total
	LEA   EAX, [EAX*4+EAX]     ;// #1 Multiply Total * 5
	LEA   EAX, [EAX*2+EDX-'0'] ;// #2 Total = ([Total*5] * 2) + (c -'0')
;// Get next digit & slide past commas!
NEXT_DIGIT:
	MOVZX EDX, BYTE PTR [ECX]  ;// c = *lpText;
	CMP   DL, ','              ;// Skip commas!
	JNE   L2
		ADD   ECX, 1
		JMP   NEXT_DIGIT
	L2:
	CMP   DL, '0'
	JB    EXIT_RETURN          ;// Not digit, exit
	CMP   DL, '9'
	JA    EXIT_RETURN          ;// Not digit, exit
	ADD   ECX, 1               ;// lpText++

JMP MAIN_LOOP           ;// It's a digit, loop again (0x30-0x39 / '0'-'9')!

EXIT_RETURN:

	CMP  [ESP], 0       ;// Non-zero flag. If we had negative sign, return negated value
	JZ    EXIT_RETURN2  
		NEG  EAX        ;// return -Total
EXIT_RETURN2:
	ADD   ESP, 4
	RET
}}
/***
* long StrToDWORD(char *str) - Convert string to DWORD number
*
*Purpose:
*       Converts ASCII string pointed to by lpText to 32-bit DWORD.
*Entry:
*       lpText = ptr to string to convert
*Exit:
*       return decimal DWORD number in string
*******************************************************************************/
NAKED_EXTERN DWORD __fastcall StrToDWORD(PSTR lpText) { __asm {
;// lpText = ECX
	XOR   EAX, EAX      ;// Total = 0

;// Slide to 1st digit or EOS (Slides past any \s\t\r\n\c chars)

LOOP1:

	MOVZX EDX, BYTE PTR [ECX] ;// c = *lpText;
	TEST  DL, DL        ;// c == NULL ? yes, exit loop; no, continue
	JZ    EXIT_RETURN
	ADD   ECX, 1        ;// lpText++
	CMP   DL, '0'
	JB    LOOP1         ;// Not digit, get next char
	CMP   DL, '9'

JA LOOP1                ;// Not digit, get next char	

;// Found a digit, start adding!

MAIN_LOOP:

;// Add digit to the total
	LEA   EAX, [EAX*4+EAX]     ;// #1 Multiply Total * 5
	LEA   EAX, [EAX*2+EDX-'0'] ;// #2 Total = ([Total*5] * 2) + (c -'0')
;// Get next digit & slide past commas!
NEXT_DIGIT:
	MOVZX EDX, BYTE PTR [ECX]  ;// c = *lpText;
	CMP   DL, ','              ;// Skip commas!
	JNE   L2
		ADD   ECX, 1
		JMP   NEXT_DIGIT
	L2:
	CMP   DL, '0'
	JB    EXIT_RETURN          ;// Not digit, exit
	CMP   DL, '9'
	JA    EXIT_RETURN          ;// Not digit, exit
	ADD   ECX, 1               ;// lpText++

JMP MAIN_LOOP           ;// It's a digit, loop again (0x30-0x39 / '0'-'9')!

EXIT_RETURN:

	RET
}}
/*
 * chrCount
 */
NAKED_EXTERN DWORD __fastcall chrCount(PSTR lpText, char cFindChar) { __asm {
;// ECX = lpText
;// EDX = DL = cFindChar
	PUSH  EBX
	PUSH  ESI
	PUSH  EDI

	XOR   EDI, EDI  ;// Counter = 0
;// Set all 4 bytes of EDX to 'DL' char/byte
	MOV   DH, DL    ;// EDX=0/0/DL/DL
	MOVZX EAX, DX   ;// EAX=0/0/DL/DL
	BSWAP EDX       ;// EDX=DL/DL/0/0
	OR    EDX, EAX  ;// EDX=DL/DL/DL/DL

LOOP_CHRCOUNT:

	MOV   ESI, [ECX]   ;// *lpText = 'ABCD' - Read 4 chars/32-bits per iteration!
	MOV   EAX, ESI     ;// Copy it
;// Search for char byte
	XOR   ESI, EDX     ;// XOR 'ABCD' with DL search bytes
	LEA   EBX, [ESI-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EBX, ESI             ;// AND these two
	TEST  EBX, 80808080h       ;// Test all sign bits
	JNZ   FOUND_CHAR
;// Search for NULL byte
	LEA   EBX, [EAX-01010101h] ;// Subtract 1 from each byte
	NOT   EAX                  ;// Invert all bytes
	AND   EBX, EAX             ;// And these two
	TEST  EBX, 80808080h       ;// Test all sign bits
	JNZ   EXIT_RETURN

	ADD   ECX, 4               ;// Increment pointer
	JMP   LOOP_CHRCOUNT

FOUND_CHAR:

;// Process Char #A
	CMP   AL, DL     ;// Char found ? yes, count it; no, skip
	JNE   SKIP_CHAR1
		ADD   EDI, 1 ;// Char found, count it!
SKIP_CHAR1:
	TEST  AL, AL
	JZ    EXIT_RETURN
;// Process Char #B
	CMP   AH, DL     ;// Char found ? yes, count it; no, skip
	JNE   SKIP_CHAR2
		ADD   EDI, 1 ;// Char found, count it!
SKIP_CHAR2:
	TEST  AH, AH
	JZ    EXIT_RETURN

	BSWAP EAX        ;// May be faster than "SHR EAX,16" on most CPUs

;// Process Char #C
	CMP   AH, DL     ;// Char found ? yes, count it; no, skip
	JNE   SKIP_CHAR3
		ADD   EDI, 1 ;// Char found, count it!
SKIP_CHAR3:
	TEST  AH, AH
	JZ    EXIT_RETURN
;// Process Char #D
	CMP   AL, DL     ;// Char found ? yes, count it; no, skip
	JNE   SKIP_CHAR4
		ADD   EDI, 1 ;// Char found, count it!
SKIP_CHAR4:
	TEST  AL, AL
	JZ    EXIT_RETURN

	ADD   ECX, 4     ;// Increment pointer

JMP LOOP_CHRCOUNT

EXIT_RETURN:
	MOV  EAX, EDI
	POP  EDI
	POP  ESI
	POP  EBX
	RET
}}
/*
 * chrFindSet 
 */
NAKED_EXTERN void __fastcall chrFindSet(PSTR lpText, char cFindChar, char cSetChar) { __asm {
;// ECX = lpText
;// EDX = DL = cFindChar
;//[ESP+4] = cSetChar
	XOR   EAX, EAX
	MOV   DH, BYTE PTR [ESP+4]

MAIN_LOOP:

	MOV   AL, BYTE PTR [ECX]
	CMP   AL, DL
	JE    FIND_SET
	ADD   ECX, 1
	TEST  EAX, EAX
	JNZ   MAIN_LOOP
	RET   4

FIND_SET:
;// Char found ? Replace it!
	MOV   BYTE PTR [ECX], DH		;// *lpText = cSetChar;
	ADD   ECX, 1

JMP MAIN_LOOP

}}
/*
 * chrFindSetW
 */
extern void __fastcall chrFindSetW(PSTR lpText, WORD wFindChar, WORD wSetChar) {
	char c;
	for (;;) {
		c = *lpText;
		if ( !c )
			return;
		if ( *(LPWORD)lpText == wFindChar ) // Char found ? Replace it!
			*(LPWORD)lpText = wSetChar;
		lpText++;
	}
}
/*
 * Fast 32-bit ASM ASCII StrLowerCase()
 */
NAKED_EXTERN PSTR __fastcall StrLowerCase(PSTR lpText) { __asm { // 100%
// ECX = lpText
LOOP_LOWERCASE:
	MOV   EAX, [ECX]  ;// *lpText = 'ABCD' - Read 4 chars/32-bits per iteration!

;// Process Char #A

;//if ( char < 'A' || char > 'Z' ) then skip it!
	CMP   AL, 'A'     ;// ( char < 'A' ) ?
	JB    TEST_NULL1  ;// Yes, skip it, it's lower or other char
	CMP   AL, 'Z'     ;// ( char > 'Z' ) ?
	JA    SKIP_CHAR1  ;// Yes, skip it, it's lower or other char
;//else: It's uppercase, so lowercase it!
		OR    AL, 20h    ;// OR'ing 20h to 'A-Z' converts to 'a-z'
		MOV   [ECX], AL  ;// Write lowercase value back to string memory
		JMP   SKIP_CHAR1
TEST_NULL1:
	TEST  AL, AL      ;// ( AL == NULL ) ?
	JZ    EXIT_RETURN ;// Yes, exit on NULL; No, continue
SKIP_CHAR1:

;// Process Char #B

	ADD   ECX, 1      ;// Increment string pointer (Avoid AGI stall, use ADD, not INC)
;//if ( char < 'A' || char > 'Z' ) then skip it!
	CMP   AH, 'A'     ;// ( char < 'A' ) ?
	JB    TEST_NULL2  ;// Yes, skip it, it's lower or other char
	CMP   AH, 'Z'     ;// ( char > 'Z' ) ?
	JA    SKIP_CHAR2  ;// Yes, skip it, it's lower or other char
;//else: It's uppercase, lowercase so it!
		OR    AH, 20h    ;// OR'ing 20h to 'A-Z' converts to 'a-z'
		MOV   [ECX], AH  ;// Write lowercase value back to string memory
		JMP   SKIP_CHAR2
TEST_NULL2:
	TEST  AH, AH      ;// ( AH == NULL ) ?
	JZ    EXIT_RETURN ;// Yes, exit on NULL; No, continue
SKIP_CHAR2:

;// Process Char #C

	BSWAP EAX         ;// "SHR EAX,16" is very slow on most CPUs, BSWAP alternative
	ADD   ECX, 1      ;// Increment string pointer (Avoid AGI stall, use ADD, not INC)
;//if ( char < 'A' || char > 'Z' ) then skip it!
	CMP   AH, 'A'     ;// ( char < 'A' ) ?
	JB    TEST_NULL3  ;// Yes, skip it, it's lower or other char
	CMP   AH, 'Z'     ;// ( char > 'Z' ) ?
	JA    SKIP_CHAR3  ;// Yes, skip it, it's lower or other char
;//else: It's uppercase, so lowercase it!
		OR    AH, 20h    ;// OR'ing 20h to 'A-Z' converts to 'a-z'
		MOV   [ECX], AH  ;// Write lowercase value back to string memory
		JMP   SKIP_CHAR3
TEST_NULL3:
	TEST  AH, AH      ;// ( AH == NULL ) ?
	JZ    EXIT_RETURN ;// Yes, exit on NULL; No, continue
SKIP_CHAR3:

;// Process Char #D

	ADD   ECX, 1      ;// Increment string pointer (Avoid AGI stall, use ADD, not INC)
;//if ( char < 'A' || char > 'Z' ) then skip it!
	CMP   AL, 'A'     ;// ( char < 'A' ) ?
	JB    TEST_NULL4  ;// Yes, skip it, it's lower or other char
	CMP   AL, 'Z'     ;// ( char > 'Z' ) ?
	JA    SKIP_CHAR4  ;// Yes, skip it, it's lower or other char
;//else: It's uppercase, so lowercase it!
		OR    AL, 20h     ;// OR'ing 20h to 'A-Z' converts to 'a-z'
		MOV   [ECX], AL   ;// Write lowercase value back to string memory
		ADD   ECX, 1      ;// Increment string pointer (Avoid AGI stall, use ADD, not INC)
		JMP   LOOP_LOWERCASE ;// Loop until NULL is found
TEST_NULL4:
	TEST  AL, AL      ;// ( AL == NULL ) ?
	JZ    EXIT_RETURN ;// Yes, exit on NULL; No, continue
SKIP_CHAR4:
	ADD   ECX, 1      ;// Increment string pointer (Avoid AGI stall, use ADD, not INC)
JMP LOOP_LOWERCASE    ;// Loop until NULL is found

EXIT_RETURN:
	MOV   EAX, ECX    ;// Return string pointer right at NULL for easy strcat/append() op
	RET
}}
/*
 * Fast 32-bit ASM ASCII StrUpperCase
 */
NAKED_EXTERN PSTR __fastcall StrUpperCase(PSTR lpText) { __asm { // 100%
// ECX = lpText
LOOP_UPPERCASE:
	MOV   EAX, [ECX]  ;// *lpText = 'ABCD' - Read 4 chars/32-bits per iteration!

;// Process Char #A

;//if ( char < 'a' || char > 'z' ) then skip it!
	CMP   AL, 'a'     ;// ( char < 'a' ) ?
	JB    TEST_NULL1  ;// Yes, skip it, it's upper or other char
	CMP   AL, 'z'     ;// ( char > 'z' ) ?
	JA    SKIP_CHAR1  ;// Yes, skip it, it's upper or other char
;//else: It's lowercase, so uppercase it!
		XOR   AL, 20h    ;// XOR'ing 20h to 'a-z' converts to 'A-Z'
		MOV   [ECX], AL  ;// Write uppercase value back to string memory
		JMP   SKIP_CHAR1
TEST_NULL1:
	TEST  AL, AL      ;// ( AL == NULL ) ?
	JZ    EXIT_RETURN ;// Yes, exit on NULL; No, continue
SKIP_CHAR1:

;// Process Char #B

	ADD   ECX, 1      ;// Increment string pointer (Avoid AGI stall, use ADD, not INC)
;//if ( char < 'a' || char > 'z' ) then skip it!
	CMP   AH, 'a'     ;// ( char < 'a' ) ?
	JB    TEST_NULL2  ;// Yes, skip it, it's upper or other char
	CMP   AH, 'z'     ;// ( char > 'z' ) ?
	JA    SKIP_CHAR2  ;// Yes, skip it, it's upper or other char
;//else: It's lowercase, so uppercase it!
		XOR   AH, 20h    ;// XOR'ing 20h to 'A-Z' converts to 'a-z'
		MOV   [ECX], AH  ;// Write uppercase value back to string memory
		JMP   SKIP_CHAR2
TEST_NULL2:
	TEST  AH, AH      ;// ( AH == NULL ) ?
	JZ    EXIT_RETURN ;// Yes, exit on NULL; No, continue
SKIP_CHAR2:

;// Process Char #C

	BSWAP EAX         ;// "SHR EAX,16" is very slow on most CPUs, BSWAP alternative
	ADD   ECX, 1      ;// Increment string pointer (Avoid AGI stall, use ADD, not INC)
;//if ( char < 'a' || char > 'z' ) then skip it!
	CMP   AH, 'a'     ;// ( char < 'a' ) ?
	JB    TEST_NULL3  ;// Yes, skip it, it's upper or other char
	CMP   AH, 'z'     ;// ( char > 'z' ) ?
	JA    SKIP_CHAR3  ;// Yes, skip it, it's upper or other char
;//else: It's lowercase, so uppercase it!
		XOR   AH, 20h    ;// XOR'ing 20h to 'a-z' converts to 'A-Z'
		MOV   [ECX], AH  ;// Write uppercase value back to string memory
		JMP   SKIP_CHAR3
TEST_NULL3:
	TEST  AH, AH      ;// ( AH == NULL ) ?
	JZ    EXIT_RETURN ;// Yes, exit on NULL; No, continue
SKIP_CHAR3:

;// Process Char #D

	ADD   ECX, 1      ;// Increment string pointer (Avoid AGI stall, use ADD, not INC)
;//if ( char < 'a' || char > 'z' ) then skip it!
	CMP   AL, 'a'     ;// ( char < 'a' ) ?
	JB    TEST_NULL4  ;// Yes, skip it, it's upper or other char
	CMP   AL, 'z'     ;// ( char > 'z' ) ?
	JA    SKIP_CHAR4  ;// Yes, skip it, it's upper or other char
;//else: It's lowercase, so uppercase it!
		XOR   AL, 20h     ;// XOR'ing 20h to 'a-z' converts to 'A-Z'
		MOV   [ECX], AL   ;// Write uppercase value back to string memory
		ADD   ECX, 1      ;// Increment string pointer (Avoid AGI stall, use ADD, not INC)
		JMP   LOOP_UPPERCASE ;// Loop until NULL is found
TEST_NULL4:
	TEST  AL, AL      ;// ( AL == NULL ) ?
	JZ    EXIT_RETURN ;// Yes, exit on NULL; No, continue
SKIP_CHAR4:
	ADD   ECX, 1      ;// Increment string pointer (Avoid AGI stall, use ADD, not INC)
JMP LOOP_UPPERCASE    ;// Loop until NULL is found

EXIT_RETURN:
	MOV   EAX, ECX    ;// Return string pointer right at NULL for easy strcat/append() op
	RET
}}
/*
 * lNc
 */
NAKED_EXTERN void __fastcall lNc(DWORD Count, PSTR lpText) { __asm {
// ECX = Count
// EDX = lpText
LOOP_LOWERCASE:
	MOVZX EAX, BYTE PTR [EDX] ;// Read next char
	TEST  AL, AL              ;// ( AL == NULL ) ?
	JZ    EXIT_RETURN         ;// Yes, exit on NULL; No, continue
;//if ( char < 'A' || char > 'Z' ) then skip it!
	CMP   AL, 'A'     ;// ( char < 'A' ) ?
	JB    NEXT_CHAR   ;// Yes, skip it, it's lower or other char
	CMP   AL, 'Z'     ;// ( char > 'Z' ) ?
	JA    NEXT_CHAR   ;// Yes, skip it, it's lower or other char
;//else: It's uppercase, so lowercase it!
		OR    AL, 20h    ;// OR'ing 20h to 'A-Z' converts it to 'a-z'
		MOV   [EDX], AL  ;// Write lowercase value back to string memory
NEXT_CHAR:
	ADD   EDX, 1      ;// Increment string pointer (Avoid AGI stall, use ADD, not INC)
	SUB   ECX, 1      ;// Count--
JNZ LOOP_LOWERCASE    ;// Loop until Count = 0

EXIT_RETURN:
	RET
}}

// Shift a string to the right

void __fastcall strshr(PSTR lpText) {
	PSTR cp = lpText;
	while ( *cp )
		cp++; // Get the end of the string
	while ( lpText != cp ) {
        *(cp+1) = *cp;
		cp--;
	}
}
/***
*char *_strdup(string) - duplicate string into malloc'd memory
*Purpose:
*       Allocates enough storage via malloc() for a copy of the
*       string, copies the string into the new memory, and returns
*       a pointer to it.
*Entry:
*       char *string - string to copy into new memory
*Exit:
*       returns a pointer to the newly allocated storage with the
*       string in it or NULL if low memory or NULL string passed.
*******************************************************************************/
 NAKED_EXTERN PSTR __cdecl strdup(PCSTR _Src) { __asm {
	MOV   ECX, [ESP+4] ;// Load _Src
	TEST  ECX, ECX     ;// ( ECX != NULL ) ?
	JNZ   NOT_NULL     ;// Yes, get string length
		MOV   EAX, ECX ;// No, return null/exit
		RET

NOT_NULL:
	CALL  StrLen       ;// Get string length
	TEST  EAX, EAX     ;// ( EAX != NULL ) ?
	JNZ   ALLOC_MEMORY ;// Yes, has length, allocate memory
		RET            ;// No, memory alloc failure, return null/exit

ALLOC_MEMORY:
	PUSH  EBX          ;// Save old EBX
	LEA   EBX, [EAX+1] ;// Save Length+1 in EBX
	ADD   EAX, 5       ;// Add extra 5 bytes to string length bytes
	PUSH  EAX          ;// Push dwBytes
	XOR   EAX, EAX
	PUSH  EAX          ;// Push dwFlags 0x00
	PUSH  ghCRTHeap    ;// Push global hHeap
	CALL  DWORD PTR [HeapAlloc]
	TEST  EAX, EAX
	JZ    EXIT_RETURN
		PUSH  ESI
		PUSH  EDI
		MOV   ECX, EBX
		MOV   EDX, EAX
		MOV   EBX, EAX ;// Save a copy of new string address
		PUSH  [ESP+4 + 12]
		CALL  MemCpy
		POP   EDI
		POP   ESI
		MOV   EAX, EBX ;// Return address of newly copied string
EXIT_RETURN:
	POP   EBX          ;// Restore EBX
	RET
}}
/***
*char *strncpy(dest, source, count) - copy at most n characters
*Purpose:
*       Copies count characters from the source string to the
*       destination.  If count is less than the length of source,
*       NO NULL CHARACTER is put onto the end of the copied string.
*       If count is greater than the length of sources, dest is padded
*       with null characters to length count.
*Entry:
*       char *dest - pointer to destination
*       char *source - source string for copy
*       unsigned count - max number of characters to copy
*Exit:
*       returns dest
*******************************************************************************/
#pragma function(strcpy)
extern PSTR __cdecl strncpy(PSTR dest, const char * source, size_t count) {
	PSTR start = dest;
	DWORD dwSourceVal;
	BYTE bCount;

	bCount = (BYTE)count & 3;
	count >>= 2; // Divide by 4
	while ( count ) { // copy string
		dwSourceVal = *(LPDWORD)source;
		if ( HasNulByte(dwSourceVal) ) {
			bCount += (BYTE)(count*4);
			break;
		}
		*(LPDWORD)dest = dwSourceVal;
		source += 4;
		dest += 4;
		count--;
	}
// Copy remaining 1-3 bytes
	while ( bCount ) {
		*dest++ = *source++;
		bCount--;
	}
	return start;
}
/***
*int strncmp(first, last, count) - compare first count chars of strings
*Purpose:
*       Compares two strings for lexical order.  The comparison stops
*       after: (1) a difference between the strings is found, (2) the end
*       of the strings is reached, or (3) count characters have been
*       compared.
*Entry:
*       char *first, *last - strings to compare
*       unsigned count - maximum number of characters to compare
*Exit:
*       returns <0 if first < last
*       returns  0 if first == last
*       returns >0 if first > last
*******************************************************************************/
#ifndef _DEBUG // C-RUNTIME FUNCTION BLOCK
extern int __cdecl strncmp (
        const char * first,
        const char * last,
        size_t count
        )
{
	if ( !count )
		return(0);
	while ( --count && *first && *first == *last ) {
		first++;
		last++;
	}
	return( *(unsigned char *)first - *(unsigned char *)last );
}

#pragma function(strcmp)
extern int __cdecl strcmp(const char * first, const char * last) {
	while ( *first && *first == *last ) {
		first++;
		last++;
	}
	return (BYTE)*first - (BYTE)*last;
}
#endif