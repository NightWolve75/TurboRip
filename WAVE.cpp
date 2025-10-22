#include <Global.h>
#include "WAVE.h"

const DWORD INTMAX = 32767;

// Special Thanks: piuskerala, VB6 generic amplify algorithm I learned from

BOOL __fastcall WAVE_Normalize(PSTR lpFileName) {
	DWORD Max, MaxPeakValue;
	MAPPEDFILE mFile;
	short *lpStart;
	BOOL bResult;
// 1) Open wave file memory mapping style
	if ( !MapFile32(lpFileName, FALSE, &mFile) )
		return FALSE;
// 2) Verify Microsoft 16-bit stereo/2-channel RIFF WAVE file
	__asm {
		XOR   EAX, EAX                ;// EAX = 0
		MOV   EBX, mFile.lpMapAddress ;// EBX = Pointer to WaveHeader/WAVE file
		MOV   bResult, EAX            ;// bResult = 0/FALSE;
	;// ( WAVE_FILE_HEADER.riffID == 'FFIR' ) ? RIFF Format
		CMP   DWORD PTR [EBX], 'FFIR'
		JNE   EXIT_RETURN
	;// ( WAVE_FILE_HEADER.riffFORMAT == 'EVAW' ) ?
		CMP   DWORD PTR [EBX+8], 'EVAW'
		JNE   EXIT_RETURN
	;// ( WAVE_FILE_HEADER.fmtID == ' tmf' ) ?
		CMP   DWORD PTR [EBX+12], ' tmf'
		JNE   EXIT_RETURN
	;// ( WAVE_FILE_HEADER.fmtSIZE == 16 ) ?
		CMP   DWORD PTR [EBX+16], 16
		JNE   EXIT_RETURN
	;// ( WAVE_FILE_HEADER.dataID == 'atad' ) ?
		CMP   DWORD PTR [EBX+36], 'atad'
		JNE   EXIT_RETURN
	;// ( WAVE_FILE_HEADER.wFormatTag == 1 ) ?
		CMP   WORD PTR [EBX+20], 1  ;// WAVE_FORMAT_PCM = 0x0001
		JNE   EXIT_RETURN
	;// ( WAVE_FILE_HEADER.nChannels == 2 ) ?
		CMP   WORD PTR [EBX+22], 2  ;// Stereo = 2, no mono for now
		JNE   EXIT_RETURN
	;// ( WAVE_FILE_HEADER.wBitsPerSample == 16 ) ?
		CMP   WORD PTR [EBX+34], 16 ;// 16-bit samples, not 8!
		JNE   EXIT_RETURN

		FNINIT ;// Init/Reset FPU, default mode of rounding (change from compiler's default)!

;// 3) Find max/highest abs(value/peak) in WAVE data
		LEA   EDX, [EBX+44] ;// Get data start of 1st lp16BitSample (WaveHeader + 44)
		MOV   EAX, [EBX+40] ;// Get wave data size (EAX = WAVE_FILE_HEADER.dataSIZE)
		MOV   lpStart, EDX  ;// Save 1st lp16BitSample Address
		SHR   EAX, 1        ;// Divide wave data size by 2 (Max = WaveHeader->dataSIZE / 2) (SHR sets Zero Flag!)
		JZ    EXIT_RETURN   ;// if ( Counter == 0 ) return 0; Wave data/count is 0, error data, so exit!
		MOV   ECX, EAX      ;// Counter for 1st sample loop ready
		SHR   EAX, 1        ;// Divide Counter by 2 again (Size/4)
		JZ    EXIT_RETURN   ;// if ( Counter == 0 ) return 0; Wave data/count is <4, error data, exit!
		MOV   Max, EAX      ;// Save Counter in Max
		XOR   EBX, EBX      ;// EBX = 0; Working register to find ABS(MaxPeakValue)

	SAMPLE_LOOP1:
		MOVSX EAX, WORD PTR [EDX] ;// SampleValue = *lp16BitSample;
		TEST  EAX, EAX            ;// if ( SampleValue == 0 )
		JZ    NEXT_SAMPLE1        ;//    goto NEXT_SAMPLE1;
		JNS   SKIP_ABS
			NEG   EAX      ;// SampleValue = ABS(SampleValue)
	SKIP_ABS:
		CMP   EAX, EBX     ;// if ( SampleValue < MaxPeakValue )
		JB    NEXT_SAMPLE1 ;//    goto NEXT_SAMPLE1;
			MOV   EBX, EAX ;// else MaxPeakValue = SampleValue;
	NEXT_SAMPLE1:
		ADD   EDX, 2       ;// lp16BitSample++;
		SUB   ECX, 1       ;// Counter--;
	JNZ SAMPLE_LOOP1

		MOV   ECX, Max          ;// Restore Counter in Max for next sample loop
		MOV   MaxPeakValue, EBX ;// We found our max, save to memory for FPU loading

;// 4) Compute Volume Level Multiplier
;// VolumeLevel = 1.0f / (abs(MaxPeakValue) / 32767.0f);
		FLD1                    ;// 1.0f
		FILD   MaxPeakValue     ;// Load Max(peak)
		FIDIV  DWORD PTR INTMAX ;// Max(peak) / 32767.0f
		MOV    EDX, lpStart     ;// EDX = lp16BitSample
		FDIV                    ;// Final result: 1.0f / Max(abs(peak)) / 32767.0f
;// 5) GO! Normalize WAVE a 16-bit sample at a time!
;// Volume multiplier ready in ST(0) as full 80-bit float!
	SAMPLE_LOOP2:
		MOV   EAX, DWORD PTR [EDX] ;// Load two 16-bit SampleValues in EAX
		TEST  EAX, EAX             ;// if ( EAX != 0 ) goto FOUND_SAMPLE
		JNZ   FOUND_SAMPLE         ;// Skip zero/silence audio samples if EAX == 0
		ADD   EDX, 4               ;// NULL lp16BitSample++; NULL lp16BitSample++;
		SUB   ECX, 1               ;// Counter--;
	JNZ SAMPLE_LOOP2               ;// ( Counter != 0 ) ? Loop again until non-zero sample!
		JMP   EXIT_SAMPLE_LOOP2    ;// Counter finished here, exit!
	FOUND_SAMPLE:
		TEST  AX, AX
		JZ    SKIP_SAMPLE1
			FILD  WORD PTR [EDX]  ;// Load 16-bit SampleValue in FPU (*lp16BitSample)
			FMUL  ST(0), ST(1)    ;// SampleValue * VolumeLevel 
			FISTP WORD PTR [EDX]  ;// Store result back to memory<->file [key: memory-mapped file!]!
	SKIP_SAMPLE1:
		ADD   EDX, 2              ;// lp16BitSample++;
		TEST  EAX, 0FFFF0000h
		JZ    SKIP_SAMPLE2
			FILD  WORD PTR [EDX]  ;// Load 16-bit SampleValue in FPU (*lp16BitSample)
			FMUL  ST(0), ST(1)    ;// SampleValue * VolumeLevel
			FISTP WORD PTR [EDX]  ;// Store result back to memory<->file [key: memory-mapped file!]!
	SKIP_SAMPLE2:
		ADD   EDX, 2          ;// lp16BitSample++;
		SUB   ECX, 1          ;// Counter--;
	JNZ SAMPLE_LOOP2

	EXIT_SAMPLE_LOOP2:
		FSTP  ST(0)           ;// Restore FPU stack balance! We're done, yay!
		SUB   ECX, 1
		MOV   bResult, ECX    ;// return TRUE;
	EXIT_RETURN:
	}
// 6) Close WAVE file, we're done!
	CloseMap(mFile);
	return bResult;
}
