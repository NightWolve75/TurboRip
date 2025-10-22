// Basic Registry Handling Functions

NAKED_EXTERN BOOL __fastcall DeleteKeyValue(HKEY KeyRoot, PSTR SubKey, PSTR lpValueName) { __asm {
;// ECX = KeyRoot
;// EDX = SubKey
;// HKEY hKey;
	SUB   ESP, 4

;// 1) Try to open root\subkey if it exists

;// RegOpenKeyExA(KeyRoot, SubKey, 0, KEY_READ | KEY_WRITE, &hKey)
	XOR   EAX, EAX
	PUSH  ESP                  ;// PUSH &hKey
	PUSH  KEY_READ | KEY_WRITE
	PUSH  EAX                  ;// PUSH 0
	PUSH  EDX                  ;// PUSH SubKey
	PUSH  ECX                  ;// PUSH KeyRoot
	CALL  DWORD PTR [RegOpenKeyExA]
	TEST  EAX, EAX
	JNZ   RETURN_FAILURE ;// ( EAX == ERROR_SUCCESS )?? no, exit; yes, continue

;// 2) Subkey exists, delete it!

	;// RegDeleteValue(hKey, ValueName)
		MOV   EAX, [ESP]       ;// Get hKey
		PUSH [ESP+4+4]         ;// PUSH ValueName
		PUSH  EAX              ;// PUSH hKey
		CALL  DWORD PTR [RegDeleteValueA] ;// If you wanna beat it, just delete it!
	;//	RegCloseKey(hKey)
		MOV   ECX, [ESP]       ;// Get hKey
		MOV  [ESP], EAX        ;// Save error/success code
		PUSH  ECX              ;// PUSH hKey
		CALL  DWORD PTR [RegCloseKey]
		MOV   EAX, [ESP]       ;// Restore error/success code

;// 3) Clean up stack, return true/false

		TEST  EAX, EAX         ;// Test return code
		JNZ   RETURN_FAILURE
		ADD   ESP, 4
		ADD   EAX, 1           ;// If zero, success, so we'll return 1 for TRUE
		RET   4

RETURN_FAILURE:
	ADD   ESP, 4
	XOR   EAX, EAX
	RET   4
}}

NAKED_EXTERN DWORD __stdcall GetKeyValueDWORD(PSTR RootCode_SubKey, PSTR lpValueName) { __asm {
;// ESP+4 = RootCode_SubKey
;// ESP+8 = lpValueName
	MOV   ECX, [ESP+4]  ;// Get RootCode_SubKey ptr
;// DWORD nSize+4, dwData+8, dwType+12;
;// HKEY  hKey+0;
	SUB   ESP, 16

;// 1) Try to open root\subkey if it exists

;// RegOpenKeyExA(KeyRoot, SubKey, 0, KEY_READ, &hKey);
	XOR   EAX, EAX
	PUSH  ESP                 ;// PUSH &hKey
	PUSH  KEY_READ            ;// PUSH KEY_READ Constant
	PUSH  EAX                 ;// PUSH 0
	MOVZX EAX, BYTE PTR [ECX] ;// Get Root Key Code
	ADD   ECX, 1              ;// ptr++ to SubKey
	ADD   EAX, 80000000h      ;// Compute RootKey
	PUSH  ECX                 ;// PUSH lpSubKey
	PUSH  EAX                 ;// PUSH RootKey
	CALL  DWORD PTR [RegOpenKeyExA]
	TEST  EAX, EAX
	JNZ   RETURN_FAILURE1     ;// ( EAX == ERROR_SUCCESS )? no, exit; yes, continue

;// 2) Get variable value from key

	;// RegQueryValueExA(hKey, ValueName, 0, &dwType, &dwData, &nSize)
		MOV   EDX, [ESP]      ;// Get hKey
		LEA   ECX, [ESP+4]    ;// Get &nSize
		MOV   [ECX], 4        ;// nSize = 4/REG_DWORD
		PUSH  ECX             ;// PUSH &nSize
		LEA   ECX, [ESP+8+4]  ;// Get &dwData
		MOV   [ECX], EAX      ;// dwData = 0
		PUSH  ECX             ;// PUSH &dwData
		LEA   ECX, [ESP+12+8] ;// Get &dwType
		MOV   [ECX], EAX      ;// dwType = 0
		PUSH  ECX             ;// PUSH &dwType
		PUSH  EAX             ;// PUSH 0
		PUSH  [ESP+8+16+16]   ;// PUSH lpValueName
		PUSH  EDX             ;// PUSH hKey
		CALL  DWORD PTR [RegQueryValueExA]
	;//	RegCloseKey(hKey)
		MOV   [ESP+4], EAX    ;// Save error/success code
		CALL  DWORD PTR [RegCloseKey] ;// hKey at top of stack, close it!
		MOV   EAX, [ESP]      ;// Restore error/success code

;// 3) Clean up stack, return value or 0

		TEST  EAX, EAX        ;// Test return code
		JNZ   RETURN_FAILURE2 ;// If zero, 50% success
		CMP   [ESP+8], 4      ;// dwType == REG_DWORD? yes, 100% success; no, wrong data type != DWORD!!
		JNE   RETURN_FAILURE2
		;// 100% success, return dwData!
			MOV   EAX, [ESP+4]
			ADD   ESP, 12
			RET   8
		RETURN_FAILURE2:
			ADD   ESP, 12
			XOR   EAX, EAX
			RET   8

RETURN_FAILURE1:
	ADD   ESP, 16
	XOR   EAX, EAX
	RET   8
}}

extern BOOL __fastcall SetKeyValueDWORD(HKEY KeyRoot, PSTR lpSubKey, PSTR lpValueName, DWORD Data) {
	LONG lRet;
	HKEY hKey;
	lRet = RegOpenKeyExA(KeyRoot, lpSubKey, 0, KEY_WRITE, &hKey);
	if ( lRet )
		lRet = RegCreateKeyExA(KeyRoot, lpSubKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
	if ( !lRet ) {
		lRet = RegSetValueExA(hKey, lpValueName, 0, REG_DWORD, (PBYTE)&Data, 4);
		RegCloseKey(hKey);
		return (!lRet);
	}
	return FALSE;
}

extern DWORD __fastcall GetKeyValueSTR(HKEY KeyRoot, PSTR lpSubKey, PSTR lpValueName, PSTR lpData, DWORD cbData) {
	DWORD MaxSize;
	HKEY hKey;
	if ( !RegOpenKeyExA(KeyRoot, lpSubKey, 0, KEY_READ, &hKey) ) {
		MaxSize = cbData;
	// Best way: Call RegQuery twice, 1st to determine string count, avoids the error-more-data caution error!
		if (
			( !RegQueryValueExA(hKey, lpValueName, NULL, NULL, NULL, &cbData) && cbData < MaxSize )
				&&
			( !RegQueryValueExA(hKey, lpValueName, NULL, NULL, (LPBYTE)lpData, &cbData) )
		)
			lpData[cbData-1] = 0;
		else
			cbData = 0;
		RegCloseKey(hKey);
		return cbData;
	}
	return 0;
}

// xGetKeyValueSTR x86
// Ensures values (REG_SZ, REG_MULTI_SZ, and REG_EXPAND_SZ) returned are null-terminated

EXTERN __declspec(naked) DWORD __stdcall xGetKeyValueSTR(PSTR RootCode_SubKey, PSTR lpValueName, PSTR lpData, DWORD cbData) { __asm {
;// ESP+4  = RootCode_SubKey
;// ESP+8  = lpValueName
;// ESP+12 = lpData
;// ESP+16 = cbData
	MOV   ECX,[ESP+4]  ;// Get RootCode_SubKey ptr
;// HKEY  hKey    = ESP+0;
;// DWORD MaxSize = ESP+4;
	SUB   ESP, 8

;// 1) Try to open HK Root\Subkey if it exists

;// RegOpenKeyExA(KeyRoot, SubKey, 0, KEY_READ, &hKey);
	XOR   EAX, EAX
	PUSH  ESP                 ;// PUSH &hKey
	PUSH  KEY_READ            ;// PUSH KEY_READ Constant
	PUSH  EAX                 ;// PUSH 0
	MOVZX EAX, BYTE PTR [ECX] ;// Get Root Key Code
	ADD   ECX, 1              ;// ptr++ to SubKey
	OR    EAX, 80000000h      ;// Compute RootKey
	PUSH  ECX                 ;// PUSH lpSubKey
	PUSH  EAX                 ;// PUSH RootKey
	CALL  DWORD PTR [RegOpenKeyExA]
	TEST  EAX, EAX
	JNZ   RETURN_FAILURE1     ;// ( EAX == ERROR_SUCCESS )? no, exit; yes, continue

;// 2) Get variable value from key, 2 step process!
;// Best way: Call RegQuery twice, 1st to get string count, avoids the error-more-data caution error!

	;// CALL #1 RegQueryValueExA(hKey, ValueName, 0, NULL, NULL, &MaxSize);
		LEA   ECX,[ESP+4]    ;// Get &MaxSize
		XOR   EAX, EAX       ;// EAX = 0
		MOV   EDX,[ESP]      ;// Get hKey
		PUSH  ECX            ;// PUSH &MaxSize
		MOV  [ECX], EAX      ;// MaxSize = 0;
		PUSH  EAX            ;// PUSH NULL
		PUSH  EAX            ;// PUSH NULL
		PUSH  EAX            ;// PUSH 0
		PUSH  [ESP+8+8+16]   ;// PUSH lpValueName
		PUSH  EDX            ;// PUSH hKey
		CALL  DWORD PTR [RegQueryValueExA]
		TEST  EAX, EAX
		JNZ   RETURN_FAILURE2
		MOV   EAX,[ESP+4]    ;// Get MaxSize
		LEA   ECX,[ESP+16+8] ;// Get cbData
		TEST  EAX, EAX
		JZ    RETURN_FAILURE2
		CMP   EAX,[ECX]      ;// ( MaxSize > *cbData )? Yes, close/exit failure; No, continue
		JA    RETURN_FAILURE2

	;// CALL #2 RegQueryValueExA(hKey, lpValueName, NULL, NULL, lpData, &cbData);
		MOV   EDX,[ESP]      ;// Get hKey
		PUSH  ECX		     ;// PUSH &cbData
		MOV   ECX,[ESP+12+12];// Get lpData
		XOR   EAX, EAX       ;// EAX = 0
		PUSH  ECX            ;// PUSH lpData
		PUSH  EAX            ;// PUSH NULL
		PUSH  EAX            ;// PUSH NULL
		PUSH [ESP+8+8+16]    ;// PUSH lpValueName
		PUSH  EDX            ;// PUSH hKey
		CALL  DWORD PTR [RegQueryValueExA]
		TEST  EAX, EAX
		JNZ   RETURN_FAILURE2
		MOV   ECX,[ESP+16+8]  ;// Get cbData
		TEST  ECX, ECX        ;// ( cbData > 0 )? Yes, continue; No, error/abort
		JZ    RETURN_FAILURE2
	;// lpData[cbData-1] = 0; Ensure NULL string termination as required
		MOV   EDX,[ESP+12+8]  ;// Get lpData
		MOV  [EDX+ECX-1], EAX ;// lpData[cbData-1] = NULL;
	;//	RegCloseKey(hKey)
		CALL  DWORD PTR [RegCloseKey] ;// hKey at top of stack, close it!

;// 3) Clean up stack, return >0 string count

		ADD   ESP, 4
		MOV   EAX,[ESP+16]    ;// Return cbData / Length (AKA Data/String Size in Bytes)
		RET   16

RETURN_FAILURE2:
;//	RegCloseKey(hKey)
	CALL  DWORD PTR [RegCloseKey] ;// hKey at top of stack, close it!
	ADD   ESP, 4
	XOR   EAX, EAX
	RET   16

RETURN_FAILURE1:
	ADD   ESP, 8
	XOR   EAX, EAX
	RET   16
}}

extern BOOL __fastcall SetKeyValueSTR(HKEY KeyRoot, PSTR lpSubKey, PSTR lpValueName, PSTR lpData, DWORD cbData) {
	LONG lRet;
	HKEY hKey;
// 1) Try to open root\subkey if it exists
	lRet = RegOpenKeyExA(KeyRoot, lpSubKey, 0, KEY_WRITE, &hKey);
// 2) Subkey doesn't exist, create it
	if ( lRet )
		lRet = RegCreateKeyExA(KeyRoot, lpSubKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
// 3) Write value/variable name's data and return
	if ( !lRet ) {
		lRet = RegSetValueExA(hKey, lpValueName, 0, REG_SZ, (PBYTE)lpData, cbData);
		RegCloseKey(hKey);
		return (!lRet);
	}
	return FALSE;
}

NAKED_EXTERN BOOL __stdcall SetKeyValueBIN(HKEY KeyRoot, PSTR lpSubKey, PSTR lpValueName, PBYTE lpData, DWORD cbData) { __asm {
;// HKEY hKey;
	SUB   ESP, 4

;// 1) Try to open root\subkey if it exists

;// RegOpenKeyExA(KeyRoot, SubKey, 0, KEY_WRITE, &hKey);
	XOR   EAX, EAX
	PUSH  ESP            ;// PUSH &hKey
	PUSH  KEY_WRITE
	PUSH  EAX            ;// PUSH 0
	PUSH  [ESP+4+12+8]   ;// PUSH SubKey
	PUSH  [ESP+4+16+4]   ;// PUSH sKeyRoot
	CALL  DWORD PTR [RegOpenKeyExA]
	TEST  EAX, EAX
	JZ    REG_KEY_EXISTS ;// ( EAX == ERROR_SUCCESS )??

;// 2) If Subkey doesn't exist, try to create it

	;// RegCreateKeyExA(KeyRoot, SubKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
		XOR   EAX, EAX
		MOV   ECX, ESP
		PUSH  EAX            ;// PUSH NULL
		PUSH  ECX            ;// PUSH &hKey
		PUSH  EAX            ;// PUSH NULL
		PUSH  KEY_WRITE
		PUSH  EAX            ;// PUSH 0
		PUSH  EAX            ;// PUSH NULL
		PUSH  EAX            ;// PUSH 0
		PUSH  [ESP+4+28+8]   ;// PUSH SubKey
		PUSH  [ESP+4+32+4]   ;// PUSH KeyRoot
		CALL  DWORD PTR [RegCreateKeyExA]
		TEST  EAX, EAX
		JZ    REG_KEY_EXISTS ;// ( EAX == ERROR_SUCCESS )??
			XOR   EAX, EAX   ;// ERROR: Cannot create key, return 0!
			RET   20

;// 3) Write value/variable name's data and return

REG_KEY_EXISTS:
;// RegSetValueExA(hKey, ValueName, 0, REG_BINARY, (LPBYTE)lpData, cbLen);
	MOV   ECX, [ESP]
	PUSH  [ESP+4+0+20]   ;// PUSH cbLen
	PUSH  [ESP+4+4+16]   ;// PUSH lpData
	PUSH  REG_BINARY
	PUSH  EAX            ;// PUSH 0
	PUSH  [ESP+4+16+12]  ;// PUSH ValueName
	PUSH  ECX            ;// PUSH &hKey
	CALL  DWORD PTR [RegSetValueExA]
;//	RegCloseKey(hKey);
	MOV   ECX, [ESP]     ;// Get hKey
	PUSH  EAX            ;// Save error/success code
	PUSH  ECX            ;// PUSH &hKey
	CALL  DWORD PTR [RegCloseKey]
	POP   EAX            ;// Restore error/success code

;// 4) Clean up stack, return true/false

	ADD   ESP, 4
	TEST  EAX, EAX       ;// Test return code
	JNZ   RETURN_FAILURE
		ADD   EAX, 1     ;// If zero, success, so we'll return 1 for TRUE
		RET   20

RETURN_FAILURE:
	XOR   EAX, EAX       ;// ERROR: We failed, return 0
	RET   20
}}
