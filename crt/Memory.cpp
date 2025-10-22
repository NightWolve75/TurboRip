/* Version: Ultimate
  Note: If EDI isn't preserved VB/VBA apps calling an exported
  function will return a "Bad DLL Calling Convention" error...
  Generally best to preserve most registers, tricky not to.
*/

NAKED_EXTERN long __fastcall FastFindDX(PVOID lpMem, DWORD nStartIndex, DWORD nStopIndex, PVOID lpFind, DWORD nFindSize) {
	static DWORD lpStackPointer; // To free up ESP as extra register!
	__asm {
;// ECX    = lpMem
;// EDX    = nStartIndex
;//[ESP+4 +PUSH*4] = nStopIndex
;//[ESP+8 +PUSH*4] = lpFind
;//[ESP+12 +PUSH*4]= nFindSize
	PUSH  EBX
	PUSH  ESI
	PUSH  EDI
	PUSH  EBP

;// 1) Load lpMem[nStartIndex]
	LEA   EDI, [ECX+EDX]      ;// EDI = lpMem[nStartIndex] for start of memory array
;// 2) Load lpFind bytes/chars to scan/find/search
	MOV   ESI, [ESP+16 +8]  ;// ESI = lpFind, buffer to find SUBST
;// 3) Load ByteCount/Size = nStopIndex - nStartIndex
	MOV   ECX, [ESP+16 +4]  ;// Compute number of bytes with nStopIndex
	SUB   ECX, EDX            ;// ECX = (nStopIndex-nStartIndex) Total bytes in search range
;// 4) Load nFindSize
	MOV   EBX, [ESP+16 +12]  ;// EBX = nFindSize
;// 5) Load 1st char/byte of lpFind
	MOVZX EAX, BYTE PTR [ESI] ;// AL = First byte to find/scan/search for
;// 6) Set all 4 bytes of EAX to 'AL' char/byte for faster 32-bit scanning!
	MOV   AH, AL    ;// EAX=0/0/AL/AL
	MOV   EDX, EAX  ;// EDX=0/0/AL/AL
	BSWAP EAX       ;// EAX=AL/AL/0/0
	OR    EAX, EDX  ;// EAX=AL/AL/AL/AL

;// 7) Choose advanced search method based on nFindSize

	CMP   EBX, 5
	JA    FIND_MAX_INIT
	JE    FIND_FIVE_BYTES_INIT
	CMP   BL, 4
	JE    FIND_FOUR_BYTES_INIT
	CMP   BL, 3
	JE    FIND_THREE_BYTES_INIT
	CMP   BL, 2
	JE    FIND_TWO_BYTES_INIT
	JMP   FIND_ONE_BYTE_INIT

;// *******************************************************************************
;// *** Find SUBST is 6 bytes or longer, scan here at maximum power! ***
;// *******************************************************************************
FIND_MAX_INIT:
	MOV   lpStackPointer, ESP ;// Save ESP to global variable to use it as extra register!!!
	MOV   EBP, [ESI+1] ;// EBP = Next 4 bytes of SUBST to scan for
	SUB   EBX, 5       ;// Size - 5
	ADD   ESI, 5
	MOV   ESP, EBX     ;// WARNING: ESP cannot be set to 0 if using as scratch! CAREFUL!

;// Begin searching for 1st char/byte

FIND_MAX_LOOP:
	MOV   EDX, [EDI]           ;// Load DWORD/32-bits/4-bytes of lpMemory
	XOR   EDX, EAX             ;// XOR DWORD with AL search bytes
	LEA   EBX, [EDX-01010101h] ;// Subtract 1 from each byte
	NOT   EDX                  ;// Invert all bytes
	AND   EBX, EDX             ;// AND these two
	TEST  EBX, 80808080h       ;// Test all sign bits 
	JNZ   FOUND_BYTE1          ;// Byte Match? yes, check for more; no, continue scan/search
	ADD   EDI, 4               ;// Increment pointer
	SUB   ECX, 4               ;// Decrement count
JA FIND_MAX_LOOP               ;// Loop until found or ECX <= 0

	MOV   ESP, lpStackPointer  ;// Restore stack pointer!!!
	XOR   EAX, EAX             ;// NOT FOUND - Return -1
	NOT   EAX
	POP   EBP
	POP   EDI
	POP   ESI
	POP   EBX
	RET   12

FOUND_BYTE1:
	REPNE SCASB                ;// Scan for exact byte position
	CMP   EBP, DWORD PTR [EDI] ;// Quick CMP/SCAN for next 4 bytes of SUBST!
	JNE   FIND_MAX_LOOP        ;// No match? Go back and scan for next 1st byte occurence
		ADD   EDI, 4
		XOR   EBX, EBX
		MOV   EDX, ESP
		SHR   EDX, 2           ;// Divide Length by 4
		JZ    BYTE_COMPARE_INIT1

		DWORD_COMPARE1:
			ADD   EBX, 4
			CMPSD
			JNE   RESTART_SEARCH1
			SUB   EDX, 1
		JNZ DWORD_COMPARE1

	BYTE_COMPARE_INIT1:
		MOV   EDX, ESP
		AND   EDX, 3
		JZ    FOUND_BYTES1     ;// SUBST found!!!!!!!!

		BYTE_COMPARE1:
			ADD   EBX, 1
			CMPSB
			JNE   RESTART_SEARCH1
			SUB   EDX, 1
		JNZ BYTE_COMPARE1

		JMP FOUND_BYTES1       ;// SUBST found!!!!!!!!

	RESTART_SEARCH1:
		SUB   ESI, EBX
		SUB   EDI, EBX
		SUB   EDI, 4

	JMP FIND_MAX_LOOP          ;// Search again! NEVER GIVE UP!


FOUND_BYTES1: ;// Index found, compute, return it
	MOV   ESP, lpStackPointer  ;// Restore our stack pointer!!!
	MOV   EAX, [ESP+4+16+ 0]   ;// Load StopIndex again 
	POP   EBP
	POP   EDI
	SUB   EAX, ECX             ;// (StopIndex - BytesRemaining) = Found Offset Index
	POP   ESI
	SUB   EAX, 1               ;// FoundIndex--
	POP   EBX
	RET   12

;// *******************************************************************************
;// Handle 5 Byte Find Buffer
;// *******************************************************************************

FIND_FIVE_BYTES_INIT:
	MOV   EBP, [ESI+1] ;// EBP = Next 4 bytes of SUBST to scan for

;// Begin searching for 1st char/byte

FIND_FIVE_BYTES_LOOP:
	MOV   EDX, [EDI]           ;// Load DWORD/32-bits/4-bytes of lpMemory
	XOR   EDX, EAX             ;// XOR DWORD with AL search bytes
	LEA   EBX, [EDX-01010101h] ;// Subtract 1 from each byte
	NOT   EDX                  ;// Invert all bytes
	AND   EBX, EDX             ;// AND these two
	TEST  EBX, 80808080h       ;// Test all sign bits 
	JNZ   FIND_ALL_FIVE_BYTES  ;// Byte Match? yes, check for more; no, continue scan/search
	ADD   EDI, 4               ;// Increment pointer
	SUB   ECX, 4               ;// Decrement count
JA FIND_FIVE_BYTES_LOOP        ;// Loop until found or ECX <= 0

	XOR   EAX, EAX             ;// NOT FOUND - Return -1
	NOT   EAX
	POP   EBP
	POP   EDI
	POP   ESI
	POP   EBX
	RET   12

FIND_ALL_FIVE_BYTES:
	REPNE SCASB                ;// Scan for exact byte position
	CMP   EBP, [EDI]           ;// Quick CMP/SCAN for next 4 bytes of SUBST!
	JE    FOUND_FIVE_BYTES     ;// SUBST found!!!!!!!!
JMP FIND_FIVE_BYTES_LOOP       ;// No match? Go back, scan for next 1st byte occurence

FOUND_FIVE_BYTES: ;// Index found, compute, return it
	MOV   EAX, [ESP+4+16+ 0]   ;// Load StopIndex again 
	POP   EBP
	POP   EDI
	SUB   EAX, ECX             ;// (StopIndex - BytesRemaining) = Found Offset Index
	POP   ESI
	SUB   EAX, 1               ;// FoundIndex--
	POP   EBX
	RET   12

;// *******************************************************************************
;// Handle 4 Byte Find Buffer
;// *******************************************************************************

FIND_FOUR_BYTES_INIT:
	MOV   EBP, [ESI]  ;// EBP = All 4 bytes of SUBST to find
;// Begin searching for 1st char/byte
FIND_FOUR_BYTES_LOOP:
	MOV   EDX, [EDI]           ;// Load DWORD/32-bits/4-bytes of lpMemory
	XOR   EDX, EAX             ;// XOR DWORD with AL search bytes
	LEA   EBX, [EDX-01010101h] ;// Subtract 1 from each byte
	NOT   EDX                  ;// Invert all bytes
	AND   EBX, EDX             ;// AND these two
	TEST  EBX, 80808080h       ;// Test all sign bits 
	JNZ   FIND_ALL_FOUR_BYTES  ;// Byte Match? yes, check for more; no, continue scan/search
	ADD   EDI, 4               ;// Increment pointer
	SUB   ECX, 4               ;// Decrement count
JA FIND_FOUR_BYTES_LOOP        ;// Loop until found or ECX <= 0

	XOR   EAX, EAX             ;// NOT FOUND - Return -1
	NOT   EAX
	POP   EBP
	POP   EDI
	POP   ESI
	POP   EBX
	RET   12

FIND_ALL_FOUR_BYTES:
	REPNE SCASB                ;// Scan for exact byte position
	CMP   EBP, [EDI-1]         ;// Quick CMP/SCAN of all 4 bytes of SUBST!
	JE    FOUND_FOUR_BYTES     ;// SUBST found!!!!!!!!
JMP FIND_FOUR_BYTES_LOOP       ;// No match? Go back, scan for next 1st byte occurence

FOUND_FOUR_BYTES: ;// Index found, compute, return it
	MOV   EAX, [ESP+4+16+ 0]   ;// Load StopIndex again 
	POP   EBP
	POP   EDI
	SUB   EAX, ECX             ;// (StopIndex - BytesRemaining) = Found Offset Index
	POP   ESI
	SUB   EAX, 1               ;// FoundIndex--
	POP   EBX
	RET   12

;// *******************************************************************************
;// Handle 3 Byte Find Buffer
;// *******************************************************************************

FIND_THREE_BYTES_INIT:
	MOVZX BP, WORD PTR [ESI+1]  ;// EBP = Remaining 2 bytes of SUBST to find
;// Begin searching for 1st char/byte
FIND_THREE_BYTES_LOOP:
	MOV   EDX, [EDI]           ;// Load DWORD/32-bits/4-bytes of lpMemory
	XOR   EDX, EAX             ;// XOR DWORD with AL search bytes
	LEA   EBX, [EDX-01010101h] ;// Subtract 1 from each byte
	NOT   EDX                  ;// Invert all bytes
	AND   EBX, EDX             ;// AND these two
	TEST  EBX, 80808080h       ;// Test all sign bits 
	JNZ   FIND_ALL_THREE_BYTES ;// Byte Match? yes, check for more; no, continue scan/search
	ADD   EDI, 4               ;// Increment pointer
	SUB   ECX, 4               ;// Decrement count
JA FIND_THREE_BYTES_LOOP       ;// Loop until found or ECX <= 0

	XOR   EAX, EAX             ;// NOT FOUND - Return -1
	NOT   EAX
	POP   EBP
	POP   EDI
	POP   ESI
	POP   EBX
	RET   12

FIND_ALL_THREE_BYTES:
	REPNE SCASB                ;// Scan for exact byte position
	CMP   BP, WORD PTR [EDI]   ;// Quick CMP/SCAN of all 4 bytes of SUBST!
	JE    FOUND_THREE_BYTES    ;// SUBST found!!!!!!!!
JMP FIND_THREE_BYTES_LOOP      ;// No match? Go back, scan for next 1st byte occurence

FOUND_THREE_BYTES: ;// Index found, compute, return it
	MOV   EAX, [ESP+4+16+ 0]   ;// Load StopIndex again 
	POP   EBP
	POP   EDI
	SUB   EAX, ECX             ;// (StopIndex - BytesRemaining) = Found Offset Index
	POP   ESI
	SUB   EAX, 1               ;// FoundIndex--
	POP   EBX
	RET   12

;// *******************************************************************************
;// Handle 2 Byte Find Buffer
;// *******************************************************************************

FIND_TWO_BYTES_INIT:
	MOVZX EDX, BYTE PTR [ESI+1]  ;// DL = Remaining 1 bytes of SUBST to find

;// Begin searching for 1st char/byte

FIND_TWO_BYTES_LOOP:
	MOV   ESI, [EDI]           ;// Load DWORD/32-bits/4-bytes of lpMemory
	XOR   ESI, EAX             ;// XOR DWORD with AL search bytes
	LEA   EBX, [ESI-01010101h] ;// Subtract 1 from each byte
	NOT   ESI                  ;// Invert all bytes
	AND   EBX, ESI             ;// AND these two
	TEST  EBX, 80808080h       ;// Test all sign bits 
	JNZ   FIND_ALL_TWO_BYTES   ;// Byte Match? yes, check for more; no, continue scan/search
	ADD   EDI, 4               ;// Increment pointer
	SUB   ECX, 4               ;// Decrement count
JA FIND_TWO_BYTES_LOOP         ;// Loop until found or ECX <= 0

	XOR   EAX, EAX             ;// NOT FOUND - Return -1
	NOT   EAX
	POP   EBP
	POP   EDI
	POP   ESI
	POP   EBX
	RET   12

FIND_ALL_TWO_BYTES:
	REPNE SCASB                ;// Scan for exact byte position
	CMP   DL, BYTE PTR [EDI]   ;// Quick CMP/SCAN of last 1 byte of SUBST!
	JE    FOUND_TWO_BYTES      ;// SUBST found!!!!!!!!
JMP FIND_TWO_BYTES_LOOP        ;// No match? Go back, scan for next 1st byte occurence

FOUND_TWO_BYTES: ;// Index found, compute, return it
	MOV   EAX, [ESP+4+16+ 0]   ;// Load StopIndex again 
	POP   EBP
	POP   EDI
	SUB   EAX, ECX             ;// (StopIndex - BytesRemaining) = Found Offset Index
	POP   ESI
	SUB   EAX, 1               ;// FoundIndex--
	POP   EBX
	RET   12

;// *******************************************************************************
;// Handle 1 Byte Find Buffer - Easiest (equivalent to strchr)
;// *******************************************************************************

FIND_ONE_BYTE_INIT:

;// Align by 4-byte boundary
;// Slight improvement of 4K cycles on 65K data if buffer is 4/16 aligned!
	TEST  EDI, 3
	JZ    FIND_ONE_BYTE_LOOP
	SUB   ECX, 1
	JB    NOTFOUND_ONE_BYTE
	SCASB
	JE    FOUND_ONE_BYTE
	
	TEST  EDI, 3
	JZ    FIND_ONE_BYTE_LOOP
	SUB   ECX, 1
	JB    NOTFOUND_ONE_BYTE
	SCASB
	JE    FOUND_ONE_BYTE
	
	TEST  EDI, 3
	JZ    FIND_ONE_BYTE_LOOP
	SUB   ECX, 1
	JB    NOTFOUND_ONE_BYTE
	SCASB
	JE    FOUND_ONE_BYTE

;// Begin searching for 1 char/byte

FIND_ONE_BYTE_LOOP:
	MOV   EDX, [EDI]           ;// Load DWORD/32-bits/4-bytes of lpMemory
	XOR   EDX, EAX             ;// XOR DWORD with AL search bytes
	LEA   EBX, [EDX-01010101h] ;// Subtract 1 from each byte
	NOT   EDX                  ;// Invert all bytes
	AND   EBX, EDX             ;// AND these two
	TEST  EBX, 80808080h       ;// Test all sign bits 
	JNZ   FOUND_BYTE           ;// Byte Match? yes, return; no, continue scan/search
	ADD   EDI, 4               ;// Increment pointer
	SUB   ECX, 4               ;// Decrement count
JA FIND_ONE_BYTE_LOOP          ;// Loop until found or ECX <= 0

NOTFOUND_ONE_BYTE:
	XOR   EAX, EAX             ;// NOT FOUND - Return -1
	NOT   EAX
	POP   EBP
	POP   EDI
	POP   ESI
	POP   EBX
	RET   12

FOUND_BYTE:
	REPNE SCASB                ;// Scan for exact byte position
FOUND_ONE_BYTE:                ;// Index found, compute, return it 
	MOV   EAX, [ESP+4+16+ 0]   ;// Load StopIndex again 
	POP   EBP
	POP   EDI
	SUB   EAX, ECX             ;// (StopIndex - BytesRemaining) = Found Offset Index
	POP   ESI
	SUB   EAX, 1               ;// FoundIndex--
	POP   EBX
	RET   12
}}

NAKED_EXTERN PSTR __fastcall FindDWORD(DWORD dwSize, LPVOID lpMem, DWORD dwFind) { __asm {
;// ECX    = dwSize
;// EDX    = lpMemory
;//[ESP+4] = dwFind
	MOV   EAX, [ESP+4] ;// Load DWORD to scan/find
	PUSH  ESI
	PUSH  EDI
	PUSH  EBX
	MOV   ESI, EAX  ;// Copy DWORD scan/find
	MOV   EDI, EDX  ;// Use EDI for memory ptr of String/Byte array to search
;// Set all 4 bytes of EAX to 'AL' char/byte
	MOV   AH, AL    ;// EAX=0/0/AL/AL
	MOVZX EBX, AX   ;// EBX=0/0/AL/AL
	SHL   EAX, 16   ;// EAX=AL/AL/0/0
	OR    EAX, EBX  ;// EAX=AL/AL/AL/AL
;// Simple loop until memory is DWORD aligned
ALIGN_MEMORY:
	TEST  EDI, 3
	JZ    FIND_LOOP
REALIGN_MEMORY:
	SUB   ECX, 1
	JBE   NOT_FOUND
	SCASB
	JE    FIND_DWORD_TEST
	JMP   ALIGN_MEMORY

FIND_LOOP:
	MOV   EDX, [EDI]   ;// Load 4 bytes of lpMemory
	XOR   EDX, EAX     ;// XOR DWORD with AL search bytes
	LEA   EBX, [EDX-01010101h] ;// Subtract 1 from each byte
	NOT   EDX                  ;// Invert all bytes
	AND   EBX, EDX             ;// AND these two
	TEST  EBX, 80808080h       ;// Test all sign bits
	JNZ   FOUND_CHAR
	ADD   EDI, 4
	SUB   ECX, 4
	JA    FIND_LOOP    ;// Loop until found or ECX <= 0
	JMP   NOT_FOUND
FOUND_CHAR:
	REPNE SCASB        ;// Scan for 1st byte IN DWORD
	TEST  ECX, ECX
	JBE   NOT_FOUND
FIND_DWORD_TEST:
	CMP   ESI, [EDI-1] ;// Compare for DWORD
	JE    FOUND_DWORD
	TEST  EDI, 3
	JNZ   REALIGN_MEMORY
JMP FIND_LOOP

NOT_FOUND:
	XOR   EAX, EAX     ;// Return Null
	POP   EBX
	POP   EDI
	POP   ESI
	RET   4

FOUND_DWORD:
	LEA   EAX, [EDI-1]
	POP   EBX
	POP   EDI
	POP   ESI
	RET   4 ;// Must clean stack here under __fastcall and >= 3 parameters!
}}

// Version 1.00 - 10:52 PM 3/16/2021

NAKED_EXTERN PBYTE __fastcall MemChr(DWORD dwSize, PVOID lpMem, BYTE Char) { __asm {
;// ECX    = dwSize
;// EDX    = lpMemory
;//[ESP+4] = Char
	MOVZX EAX, BYTE PTR [ESP+4] ;// Load Char to scan/find
	PUSH  ESI
	PUSH  EDI
	PUSH  EBX
	MOV   EDI, EDX  ;// Load EDI for memory search
;// Set all 4 bytes of EAX to 'AL' char/byte
	MOV   AH, AL    ;// EAX=0/0/AL/AL
	MOV   EDX, EAX  ;// EDX=0/0/AL/AL
	BSWAP EAX       ;// EAX=AL/AL/0/0
	OR    EAX, EDX  ;// EAX=AL/AL/AL/AL
;// Align by 4-byte boundary
;// Slight improvement of 4K cycles on 65K data if buffer is 4/16 aligned!
	TEST  EDI, 3
	JZ    FIND_INIT
	SCASB
	JE    RET_FOUND_CHAR
	SUB   ECX, 1
	JZ    NOT_FOUND
	TEST  EDI, 3
	JZ    FIND_INIT
	SCASB
	JE    RET_FOUND_CHAR
	SUB   ECX, 1
	JZ    NOT_FOUND
	TEST  EDI, 3
	JZ    FIND_INIT
	SCASB
	JE    RET_FOUND_CHAR
	SUB   ECX, 1
	JZ    NOT_FOUND

FIND_INIT:
	MOV   ESI, ECX     ;// Copy size for trail byte result
	AND   ESI, 3       ;// 1-3 Remainder trail bytes
	SHR   ECX, 2       ;// Divide Size by 4 to reflect 32-bit looping
	JZ    FIND_FINISH

FIND_LOOP:
	MOV   EDX, [EDI]   ;// Load 4 bytes of lpMemory
	XOR   EDX, EAX     ;// XOR DWORD with AL search bytes
	LEA   EBX, [EDX-01010101h] ;// Subtract 1 from each byte
	NOT   EDX                  ;// Invert all bytes
	AND   EBX, EDX             ;// AND these two
	TEST  EBX, 80808080h       ;// Test all sign bits 
	JNZ   FOUND_CHAR
	ADD   EDI, 4
	SUB   ECX, 1
JA FIND_LOOP    ;// Loop until found or ECX <= 0

FIND_FINISH: 
 ;// HANDLE 1-3 TRAIL BYTES!
	TEST  ESI, ESI
	JZ    NOT_FOUND
	MOV   ECX, ESI
	REPNE SCASB
	JE    RET_FOUND_CHAR
NOT_FOUND:
	XOR   EAX, EAX     ;// Return Null
	POP   EBX
	POP   EDI
	POP   ESI
	RET   4

FOUND_CHAR:
	MOV   ECX, 4       ;// Reset counter, char is between 1-4
	REPNE SCASB        ;// Scan for exact byte position
RET_FOUND_CHAR:
	LEA   EAX, [EDI-1] ;// Return address found
	POP   EBX
	POP   EDI
	POP   ESI
	RET   4            ;// MUST clean stack here with __fastcall + >= 3 parameters!
}}
/***
*void MemSet(Length, Destination, Fill) - sets "Length" bytes at "Destination" to "Fill"
*Purpose:
*       Sets the first "Length" bytes of the memory starting
*       at "Destination" to the byte value "Fill"
*Entry:
*       DWORD Length      - The size of the block of memory to fill, in bytes
*       PVOID Destination - A pointer to the starting address of the block of memory to fill
*       BYTE  Fill        - The byte value (0-0xFF) with which to fill the memory block
*Comment:
*       Optimized in x86 Assembly for speed
*******************************************************************************/
NAKED_EXTERN void __fastcall MemSet(DWORD Length, LPVOID Destination, BYTE Fill) { __asm {
;// ECX    = Length
;// EDX    = Destination
;//[ESP+4] = Fill
	MOVZX EAX, BYTE PTR [ESP+4] ;// Load 'Fill'
	PUSH  EDI
	TEST  EAX, EAX     ;// Test for NULL
	JZ    ZERO_MEM     ;// If 'Fill' is 0/NULL, skip, we're ready!
	;// Set all 4 bytes of EAX to 'Fill' if > 0
		MOV   AH, AL    ;// EAX=0/0/Fill/Fill; 01-FF, so duplicate in EAX
		MOV   EDI, EAX  ;// EDI=0/0/Fill/Fill; Copy to EDI
		BSWAP EAX       ;// EAX=Fill/Fill/0/0; Swap to HIGH WORD
		OR    EAX, EDI  ;// EAX=Fill/Fill/Fill/Fill; Finish!
ZERO_MEM:
	MOV   EDI, EDX     ;// Load 'Destination' addr
;// Align by 4-byte boundary
;// Slight improvement of 4K cycles on 65K data if 'Destination' is 4/16 aligned!
	TEST  EDX, 3       ;// Check if Destination is 4-byte aligned
	JZ    BEGIN_MEMSET ;// Yes, begin copy; No, do 4-byte align
	STOSB              ;// Memset 1-3 bytes over till aligned
	SUB   ECX, 1
	JZ    EXIT_RETURN
	TEST  EDI, 3       ;// Check if Destination is 4-byte aligned
	JZ    BEGIN_MEMSET ;// Yes, begin copy; No, do 4-byte align
	STOSB              ;// Memset 1-3 bytes over till aligned
	SUB   ECX, 1
	JZ    EXIT_RETURN
	TEST  EDI, 3       ;// Check if Destination is 4-byte aligned
	JZ    BEGIN_MEMSET ;// Yes, begin copy; No, do 4-byte align
	STOSB              ;// Memset 1-3 bytes over till aligned
	SUB   ECX, 1

BEGIN_MEMSET:
	MOVZX EDX, CL  ;// Save partial 1-3 Length
	SHR   ECX, 2   ;// Divide Length by 4
	AND   EDX, 3   ;// 1-3 Remainder byte Length
	REP   STOSD    ;// Copy 4 bytes of EAX to EDI till ECX=0
	MOV   ECX, EDX ;// Set ECX to 1-3 remaining bytes
	REP   STOSB    ;// Copy final 1-3 bytes if needed
EXIT_RETURN:
	POP   EDI
	RET   4        ;// Return, pop 3rd parameter from stack
}}

NAKED_EXTERN void __fastcall MemSet32(DWORD Length, LPVOID Destination, DWORD Fill) { __asm {
;// ECX    = Length
;// EDX    = Destination
;//[ESP+4] = Fill
	MOV   EAX, [ESP+4] ;// Load 'Fill'
	PUSH  EDI
	MOV   EDI, EDX     ;// Load 'Destination' addr
;// Align by 4-byte boundary
;// Slight improvement of 4K cycles on 65K data if 'Destination' is 4/16 aligned!
	TEST  EDX, 3       ;// Check if Destination is 4-byte aligned
	JZ    BEGIN_MEMSET ;// Yes, begin copy; No, do 4-byte align
	STOSB              ;// Memset 1-3 bytes over till aligned
	SUB   ECX, 1
	JZ    EXIT_RETURN
	TEST  EDI, 3       ;// Check if Destination is 4-byte aligned
	JZ    BEGIN_MEMSET ;// Yes, begin copy; No, do 4-byte align
	STOSB              ;// Memset 1-3 bytes over till aligned
	SUB   ECX, 1
	JZ    EXIT_RETURN
	TEST  EDI, 3       ;// Check if Destination is 4-byte aligned
	JZ    BEGIN_MEMSET ;// Yes, begin copy; No, do 4-byte align
	STOSB              ;// Memset 1-3 bytes over till aligned
	SUB   ECX, 1

BEGIN_MEMSET:
	MOVZX EDX, CL  ;// Save partial 1-3 Length
	SHR   ECX, 2   ;// Divide Length by 4
	AND   EDX, 3   ;// 1-3 Remainder byte Length
	REP   STOSD    ;// Copy 4 bytes of EAX to EDI till ECX=0
	MOV   ECX, EDX ;// Set ECX to 1-3 remaining bytes
	REP   STOSB    ;// Copy final 1-3 bytes if needed
EXIT_RETURN:
	POP   EDI
	RET   4        ;// Return, pop 3rd parameter from stack
}}

#if !( defined(_DEBUG) || defined(_WIN64) )
#pragma function(memset)

NAKED_EXTERN void * __cdecl memset(void *dst, int val, size_t count) { __asm {
;//[ESP+4] = dst
;//[ESP+8] = val
;//[ESP+12] = count
	PUSH  EDI          ;// MUST save/restore
	MOV   EDI, [ESP+4+4]          ;// Load 'dst' addr
	MOVZX EAX, BYTE PTR [ESP+8+4] ;// Load 'val'
	MOV   ECX, [ESP+12+4]         ;// Load 'count'
	TEST  EAX, EAX     ;// Test for NULL
	JZ    ZERO_MEM     ;// If 'val' is 0/NULL, skip, we're ready!
	;// Set all 4 bytes of EAX to 'val' if > 0
		MOV    AH, AL  ;// EAX=0/0/val/val; 01-FF, so duplicate in EAX
		SHL   EAX, 8   ;// EAX=0/val/val/0
		MOV    AL, AH  ;// EAX=0/val/val/val
		SHL   EAX, 8   ;// EAX=val/val/val/0
		MOV    AL, AH  ;// EAX=val/val/val/val
ZERO_MEM:
	CMP   ECX, 16
	JBE   BEGIN_MEMSET
;// Align by 4-byte boundary if Length > 16
;// Slight improvement of 4K cycles on 65K data if 'Destination' is 4/16 aligned!
	ALIGN_MEMORY:
		TEST  EDI, 3       ;// Check if Destination is 4-byte aligned
		JZ    BEGIN_MEMSET ;// Yes, begin copy; No, do 4-byte align
		STOSB              ;// Memset 1-3 bytes over till aligned
		SUB   ECX, 1
		JMP   ALIGN_MEMORY
BEGIN_MEMSET:
	MOVZX EDX, CL  ;// Save partial 1-3 Length
	SHR   ECX, 2   ;// Divide Length by 4
	AND   EDX, 3   ;// 1-3 Remainder byte Length
	REP   STOSD    ;// Copy 4 bytes of EAX to EDI till ECX=0
	MOV   ECX, EDX ;// Set ECX to 1-3 remaining bytes
	REP   STOSB    ;// Copy final 1-3 bytes if needed
	POP   EDI      ;// MUST save/restore
	RET
}}
#endif
/***
* void MemZero(Length, Destination) - Sets "Length" bytes at "Destination" to 0/NULL
*Purpose:
*       Sets the first "Length" bytes of the memory starting
*       at "Destination" to 0/NULL
*Entry:
*       DWORD Length      - The size in bytes of the block of memory to zero
*       PVOID Destination - A pointer to the starting address of the memory block
*Comment:
*       Optimized in x86 Assembly for speed
*******************************************************************************/
NAKED_EXTERN void __fastcall MemZero(DWORD Length, PVOID Destination) { __asm {
;// ECX = Length
;// EDX = Destination
	MOV  [ESP-4], EDI ;// Preserve EDI
	XOR   EAX, EAX    ;// Set 'Fill' to 0/Zero/NULL for STOSD op

;// Less cycles/more improvement if 'Destination' is 4/16 aligned!
	ALIGN_MEMORY:
		TEST  DL, 3         ;// Check if Destination is 4-byte aligned
		JZ    BEGIN_MEMZERO ;// Yes, begin memzero; No, do 4-byte align
		MOV  [EDX], AL      ;// Zero 1-3 bytes over till aligned
		ADD   EDX, 1
		SUB   ECX, 1
	JNBE ALIGN_MEMORY

BEGIN_MEMZERO:
	MOV   EDI, EDX    ;// Load 'Destination' addr
	MOV   EDX, ECX    ;// Copy Length for 1-3 trail bytes
	SHR   ECX, 2      ;// Divide Length by 4
	REP   STOSD       ;// Zero 4 bytes at [EDI], EDI+=4, REP until ECX=0
	AND   EDX, 3      ;// 1-3 Remainder byte Length
	JNZ   TRAIL_BYTES ;// 1-3 Trail bytes ? Yes, handle them! No, return now!
	MOV   EDI,[ESP-4] ;// Restore EDI
	RET

TRAIL_BYTES:
	MOV   ECX, EDX    ;// Set ECX to 1-3 remaining bytes
	REP   STOSB       ;// Zero final 1-3 bytes
	MOV   EDI,[ESP-4] ;// Restore EDI
	RET
}}
/***
*MemCpy - Copy source buffer to destination buffer
*Purpose:
*       MemCpy() copies a source memory buffer to a destination memory buffer.
*       This routine does NOT recognize overlapping buffers, and thus can lead
*       to propogation. Destination will be aligned, EXCELLENT performance if BOTH
*       Source/Destination are 4/16-byte aligned with _aligned_malloc()!!!!!!
*       For cases where propogation must be avoided, memmove() must be used.
*Notes: This is AS fast as SSE3 128-bit copying with aligned memory!
*Entry:
*       DWORD Length      - The size of the block of memory to fill, in bytes
*       PVOID Destination - A pointer to the starting address of the copied block's destination
*       PVOID Source      - A pointer to the starting address of the block of memory to copy
*******************************************************************************/
NAKED_EXTERN void * __fastcall MemCpy(DWORD Length, PVOID Destination, PVOID Source) { __asm {
;// ECX    = Length
;// EDX    = Destination
;//[ESP+4] = Source
	MOV   EAX, EDI     ;// Save old EDI
	MOV   EDI, EDX     ;// Load Destination address in EDI
	MOV   EDX, EAX     ;// Save old EDI in EDX (Avoids use of slow "XCHG EDI, EDX")
	MOV   EAX, ESI     ;// Save old ESI in EAX
	MOV   ESI,[ESP+4]  ;// Load Source address in ESI

;// Slight improvement of 4K cycles on 65K data if at least 'Destination' is 4/16 aligned!
;// EXCELLENT improvement ~= MOVDQU on 65K data if BOTH ESI/EDI are aligned via _aligned_malloc()
	ALIGN_MEMORY:
		TEST  EDI, 3      ;// Check if Destination is 4-byte aligned
		JZ    BEGIN_COPY  ;// Yes, begin copy; No, do 4-byte align
		MOVSB             ;// Copy 1-3 bytes over till aligned
		SUB   ECX, 1
	JNBE ALIGN_MEMORY

BEGIN_COPY:
	TEST  CL, 3
	JNZ   TRAIL_BYTES_COPY
	SHR   ECX, 2      ;// Divide Length by 4
	REP   MOVSD       ;// Start DWORD copy ESI to EDI till ECX=0!
	MOV   ESI, EAX    ;// Restore ESI
	MOV   EAX, EDI    ;// Return Destination address+1 for strcat/append op
	MOV   EDI, EDX    ;// Restore EDI
	RET   4           ;// Return, pop 3rd parameter from stack

TRAIL_BYTES_COPY:
	MOV  [ESP+4], EAX  ;// To restore ESI later, we need EAX as temp!
	MOV   EAX, ECX     ;// Copy Length
	SHR   ECX, 2       ;// Divide Length by 4
	AND   EAX, 3       ;// 1-3 Remainder byte Length
	REP   MOVSD        ;// Start DWORD copy ESI to EDI till ECX=0!
	MOV   ECX, EAX     ;// Set ECX to 1-3 remaining bytes
	REP   MOVSB        ;// Copy final 1-3 bytes

	MOV   EAX, EDI     ;// Return Destination address+1 for strcat/append op
	MOV   ESI,[ESP+4]  ;// Restore ESI
	MOV   EDI, EDX     ;// Restore EDI
	RET   4            ;// Return/Pop 3rd parameter from stack
}}

#if !( defined(_DEBUG) || defined(_WIN64) )
#pragma function(memcpy)

NAKED_EXTERN void * __cdecl memcpy(void *dest, const void *src, size_t count) { __asm {
	PUSH  ESI  ;// MUST save/restore
	PUSH  EDI  ;// MUST save/restore
	MOV   ECX, [ESP+12+8]  ;// Load count
	MOV   EDI, [ESP+4 +8]  ;// Load dest address
	MOV   ESI, [ESP+8 +8]  ;// Load src address
	CMP   ECX, 16
	JBE   BEGIN_COPY
;// Slight improvement of 4K cycles on 65K data if at least 'Destination' is 4/16 aligned!
;// EXCELLENT improvement ~= MOVDQU on 65K data if BOTH ESI/EDI are aligned via _aligned_malloc()
	ALIGN_MEMORY:
		TEST  EDI, 3      ;// Check if Destination is 4-byte aligned
		JZ    BEGIN_COPY  ;// Yes, begin copy; No, do 4-byte align
		MOVSB             ;// Copy 1-3 bytes over till aligned
		SUB   ECX, 1
		JMP   ALIGN_MEMORY
BEGIN_COPY:
	MOVZX EAX, CL      ;// Copy partial Length
	SHR   ECX, 2       ;// Divide Length by 4
	AND   EAX, 3       ;// 1-3 Remainder byte Length
	REP   MOVSD        ;// Start DWORD copy ESI to EDI till ECX=0!
	MOV   ECX, EAX     ;// Set ECX to 1-3 remaining bytes
	REP   MOVSB        ;// Copy final 1-3 bytes if needed
	POP   EDI  ;// MUST save/restore
	POP   ESI  ;// MUST save/restore
	RET
}}
#endif
/***
*memmove - Copy source buffer to destination buffer
*Purpose:
*       MemMove() copies a source memory buffer to a destination memory buffer.
*       This routine recognizes overlapping buffers to avoid propogation.
*       For cases where propogation is not a problem, memcpy() can be used.
*Entry:
*       DWORD Length      - The size of the block of memory to move, in bytes
*       PVOID Destination - A pointer to the starting address of the move destination
*       PVOID Source      - A pointer to the starting address of the block of memory to be moved
*******************************************************************************/
NAKED_EXTERN void __fastcall MemMove(DWORD Length, LPVOID Destination, LPVOID Source) { __asm {
;// ECX    = Length
;// EDX    = Destination
;//[ESP+4] = Source
	MOV   EBX, ESI      ;// Save old ESI
	MOV   ESI, EDI      ;// Copy old EDI to save in EDX
	MOV   EDI, EDX      ;// Load Destination address
	MOV   EDX, ESI      ;// Save old EDI in EDX
	MOV   ESI,[ESP+4]   ;// Load Source address
;// Check for overlapping buffers:
;// If (Destination <= Length) Or (Destination >= Source + Length) Then
;//		Do Normal MemCopy (Upwards)
;//	Else
;//		Do Downwards Copy to avoid propagation
	CMP   EDI, ESI      ;// ( Destination <= Source ) ?
	JBE   MEMCPY_NORMAL ;// Yes? Do normal memcopy toward higher addresses
		LEA   EAX, [ESI+ECX] ;// EAX = Source address + Length
		CMP   EDI, EAX       ;// ( Destination < (Source + Length) ) ?
		JB    COPY_OVERLAP   ;// Yes, copy down toward lower addresses (OVERLAP); No, normal
MEMCPY_NORMAL:
;// Slight improvement of 4K cycles on 65K data if at least 'Destination' is 4/16 aligned!
;// EXCELLENT improvement ~= MOVDQU on 65K data if BOTH ESI/EDI are aligned via _aligned_malloc()
	ALIGN_MEMORY:
		TEST  EDI, 3      ;// Check if Destination is 4-byte aligned
		JZ    BEGIN_COPY  ;// Yes, begin copy; No, do 4-byte align
		MOVSB             ;// Copy 1-3 bytes over till aligned
		SUB   ECX, 1
		JMP   ALIGN_MEMORY
	BEGIN_COPY:
		MOVZX EAX, CL   ;// Copy partial count 
		SHR   ECX, 2    ;// Divide Length by 4
		AND   EAX, 3    ;// Trap 1-3 remainder byte Length
		REP   MOVSD     ;// Start DWORD copy ESI to EDI till ECX=0!
		MOV   ECX, EAX  ;// Set ECX to 1-3 remaining bytes
		REP   MOVSB     ;// Copy final 1-3 bytes if needed
		MOV   EDI, EDX
		MOV   ESI, EBX
		RET   4
;// Overlapping Buffers
COPY_OVERLAP:
	STD   ;// Right to Left ESI/EDI copy processing
	CMP   ECX, 8     ;// ( Length < 8 ) ?
	JB    SHORT_COPY ;// Yes, use quick 1-7 byte by byte copy; No, full copy below
		LEA   ESI, [ESI+ECX-4] ;// Point to 4 bytes before src buffer end
		LEA   EDI, [EDI+ECX-4] ;// Point to 4 bytes before dest buffer end
		MOVZX EAX, CL          ;// Copy partial Length
		SHR   ECX, 2           ;// Divide Length by 4
		AND   EAX, 3           ;// 1-3 Remainder byte Length
		REP   MOVSD            ;// Start DWORD copy ESI to EDI till ECX=0!
		TEST  AL, AL           ;// ( Remainder == 0 ) ?
		JZ    EXIT_RETURN      ;// Yes, exit, we're finished!
			MOV   ECX, EAX     ;// No, set ECX to 1-3 remaining bytes, then copy
			ADD   ESI, 3
			ADD   EDI, 3
			REP   MOVSB        ;// Copy final 1-3 bytes if needed
		EXIT_RETURN:
			CLD   ;// RESTORE to compiler default!!
			MOV   EDI, EDX
			MOV   ESI, EBX
			RET   4
;// Copy 7 bytes or less
SHORT_COPY:
	LEA   ESI, [ESI+ECX-1]
	LEA   EDI, [EDI+ECX-1]
	REP   MOVSB
	CLD   ;// RESTORE to compiler default!!
	MOV   EDI, EDX
	MOV   ESI, EBX
	RET   4
}}

#if !( defined(_DEBUG) )
extern void * __cdecl memmove(void * dst, const void * src, size_t count) {
	void * ret = dst;
// Non-Overlapping Buffers
// copy from lower addresses to higher addresses
	if ( dst <= src || (char *)dst >= ((char *)src + count) ) {
		while (count--) {
			*(char *)dst = *(char *)src;
			dst = (char *)dst + 1;
			src = (char *)src + 1;
		}
	} else {
	// Overlapping Buffers
	// copy from higher addresses to lower addresses
		dst = (char *)dst + count - 1;
		src = (char *)src + count - 1;
		while (count--) {
			*(char *)dst = *(char *)src;
			dst = (char *)dst - 1;
			src = (char *)src - 1;
		}
	}
	return ret;
}
#endif
/***
*int MemCmp(dwSize, lpMemory1, lpMemory2s) - compare buffers
*Purpose:
*       MemCmp() performs fast buffer comparision.
*Entry:
*       char *string1, *string2 - strings to compare
*Return:
*       <0 if lpMemory1 < lpMemory2
*        0 if lpMemory1 = lpMemory2
*       >0 if lpMemory1 > lpMemory2
*******************************************************************************/
NAKED_EXTERN long __fastcall MemCmp(DWORD dwSize, LPVOID lpMemory1, LPVOID lpMemory2) { __asm {
;// ECX    = dwSize
;// EDX    = lpMemory1
;//[ESP+4] = lpMemory2
	PUSH  ESI           ;// Must save ESI first, so lpMemory2 = ESP+8
	MOV   ESI,[ESP+4+4] ;// Load lpMemory2
	PUSH  EDI
	MOV   EDI, ECX     ;// Copy Size
	PUSH  EBX
	SHR   ECX, 2       ;// Divide Size by 4 to reflect 32-bit looping
	JZ    TRAIL_BYTES  ;// if ( Size/4 == 0 ) try 1-3 trail bytes

LOOP_MEMCMP:
;// REPEAT 1

	MOV   EAX,[EDX]   ;// EAX = AAAA; Load 4 bytes from lpMemory1
	MOV   EBX,[ESI]   ;// EBX = BBBB; Load 4 bytes from lpMemory2
;// if ( AAAA != BBBB ) goto FOUND_DIFF
	CMP   EAX, EBX    ;// Compare 4 bytes at a time!
	JNE   FOUND_DIFF  ;// Handle difference if found!
	ADD   EDX, 4      ;// lpMemory1 += 4;
	ADD   ESI, 4      ;// lpMemory2 += 4;
	SUB   ECX, 1

JA  LOOP_MEMCMP       ;// Loop until diff or ECX = 0

;// Memory equal so far in 4-byte intervals, check 1-3 trail bytes and finish!
TRAIL_BYTES:
	AND   EDI, 3       ;// 1-3 Remainder trail bytes
	JNZ   LOOP_MEMCMP2 ;// If 1-3 trail bytes, finish off testing

;// FINISHED, memory/buffer bytes equal!

	XOR   EAX, EAX ;// return 0; (A == B)
	POP   EBX
	POP   EDI
	POP   ESI
	RET   4

LOOP_MEMCMP2:

	MOVZX EAX, BYTE PTR [EDX] ;// EAX = 000A;
	MOVZX EBX, BYTE PTR [ESI] ;// EBX = 000B;
	CMP   EAX, EBX            ;// Compare!
	JNE   EXIT_RETURN1        ;// Handle difference if found!
	ADD   EDX, 1              ;// lpMemory1++
	ADD   ESI, 1              ;// lpMemory2++
	SUB   EDI, 1

JA  LOOP_MEMCMP2

;// FINISHED, memory/buffer bytes equal!

	XOR   EAX, EAX ;// return 0; (A == B)
	POP   EBX
	POP   EDI
	POP   ESI
	RET   4

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
		RET   4 ;// return (A[1] - B[1]);
;// EAX<0 if A<B,  EAX>0 if A>B, or EAX=0 if A==B
EXIT_RETURN1:
	MOVZX EAX, AL
	MOVZX EDX, BL
	SUB   EAX, EDX    ;// Char A - Char B;
	POP   EBX
	POP   EDI
	POP   ESI
	RET   4 ;// return (A - B); ;// Return/Pop 3rd parameter from stack
}}

