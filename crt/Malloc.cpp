// Memory Allocation/Deallocation/Move Routines

// Must be initalized for our overloaded new/delete, fmalloc/ffree functions
#if defined(_DEBUG) && defined(__cplusplus)
	HANDLE ghCRTHeap = GetProcessHeap();
#else
	HANDLE ghCRTHeap;
#endif
// Notes: Must call InitCRTHeap() macro once to use these fast functions!
// #define InitCRTHeap() ghCRTHeap = GetProcessHeap()
// InitConsole() for console apps currently handles this!

#ifdef __cplusplus
	#ifdef _WIN64
		void * __cdecl operator new(SIZE_T nBytes) {return HeapAlloc(ghCRTHeap, 0, (SIZE_T)nBytes);}
	#else
		void * __cdecl operator new(size_t nBytes) {return HeapAlloc(ghCRTHeap, 0, nBytes);}
		NAKED void  __cdecl operator delete(void *lpMemory) { __asm {
			XOR  EAX, EAX
			PUSH [ESP+4]
			PUSH EAX
			MOV  EAX, [ghCRTHeap]
			PUSH EAX
			CALL DWORD PTR [HeapFree]
			RET
		}}
	#endif

	void* __cdecl operator new[](size_t nBytes) {return HeapAlloc(ghCRTHeap, 0, nBytes);}
	//HeapFree(ghCRTHeap, 0, lpMemory);
	void  __cdecl operator delete[](void* lpMemory) {HeapFree(ghCRTHeap, 0, lpMemory);}
#endif

#if !(defined(_DEBUG) || defined(_CRTVETO))

__declspec(noalias) void * __cdecl malloc(size_t size) {
	return HeapAlloc(ghCRTHeap, 0, size);
}

__declspec(noalias) void __cdecl free(void *lpMemory) {
	HeapFree(ghCRTHeap, 0, lpMemory);
}

__declspec(noalias) void * __cdecl realloc(void *lpMemory, size_t size) {
	if ( lpMemory )
		return HeapReAlloc(ghCRTHeap, 0, lpMemory, size);
	else    // 'memblock' is 0, and HeapReAlloc doesn't act like realloc() here
		return HeapAlloc(ghCRTHeap, 0, size);
}

__declspec(noalias) void * __cdecl calloc(size_t num, size_t size) {
	return HeapAlloc(ghCRTHeap, HEAP_ZERO_MEMORY, num * size);
}

#endif
/***
* void *_aligned_malloc(size_t size, size_t alignment)
*       - Get a block of aligned memory from the heap.
* Purpose:
*       Allocate of block of aligned memory aligned on the alignment of at least
*       size bytes from the heap and return a pointer to it.
* Entry:
*       size_t size - size of block requested
*       size_t alignment - alignment of memory (needs to be a power of 2)
* Exit:
*       Success: Pointer to memory block
*       Failure: Null, errno is set
*******************************************************************************/
#define PTR_SZ sizeof(DWORD)
#ifdef _DEBUG
	STRUCT_PROTECT(LPVOID, sizeof(ULONG_PTR), PTR_MATCH);
#endif

EXTERN LPVOID __fastcall _Aligned_Malloc(size_t size, size_t alignment) {
	ULONG_PTR lpMemory, retptr;
	alignment--;
	lpMemory = (ULONG_PTR)fmalloc(PTR_SZ + alignment + size);
	if ( lpMemory ) {
		retptr = (lpMemory + PTR_SZ + alignment) & ~alignment;
		((PULONG_PTR)(retptr))[-1] = lpMemory;
		return (PVOID)retptr;
	}
	return NULL;
}

EXTERN LPVOID __fastcall _Aligned_Calloc(size_t size, size_t alignment) {
	ULONG_PTR lpMemory, retptr;
	alignment--;
	lpMemory = (ULONG_PTR)fcalloc(PTR_SZ + alignment + size);
	if ( lpMemory ) {
		retptr = (lpMemory + PTR_SZ + alignment) & ~alignment;
		((PULONG_PTR)(retptr))[-1] = lpMemory;
		return (PVOID)retptr;
	}
	return NULL;
}

// _Aligned_ReAlloc() is trickiest, but I think it's correct now.

EXTERN LPVOID __fastcall _Aligned_ReAlloc(void *memblock, size_t size, size_t alignment) {
	ULONG_PTR ptr, lpNewMemory, retptr;
	SIZE_T oldsize;
// 1. Try to allocate new memory
	lpNewMemory = (ULONG_PTR)fmalloc(PTR_SZ + alignment + size);
	if ( !lpNewMemory )
		return NULL;
// 2. Get real mem ptr and size without alignment mod
	ptr = (ULONG_PTR)memblock;
// ptr points to the pointer to starting of the memory block
	ptr = (ptr & ~(PTR_SZ -1)) - PTR_SZ;
	ptr = *((PDWORD)ptr); // ptr = ACTUAL start of memory block
// 3. Get old size of actual data
	oldsize = HeapSize(ghCRTHeap, 0, (LPCVOID)ptr);
	if ( oldsize == -1 ) // Shouldn't fail/error normally
		oldsize = size; // This may lead to read violation, but Hail Mary
	oldsize -= SubPtr(SIZE_T, memblock, ptr);
// 4. Apply alignment mod on our new memory
	alignment--;
	retptr = (lpNewMemory + PTR_SZ + alignment) & ~alignment;
	((PULONG_PTR)(retptr))[-1] = lpNewMemory;
// 5. Manually copy over data
	MemCpy((DWORD)oldsize, (PVOID)retptr, (PVOID)memblock);
// 6. Free old memory
	ffree(ptr);
// 7. Return new memory ptr with all previous data
	return (LPVOID)retptr;
}
/***
*
* void *_aligned_free(void *memblock)
*       - Free the memory which was allocated using _aligned_malloc
* Purpose:
*       Frees the algned memory block which was allocated using _aligned_malloc
* Entry:
*       void * memblock - pointer to the block of memory
*******************************************************************************/
EXTERN void  __fastcall _Aligned_Free(void *memblock) {
	ULONG_PTR ptr;
	ptr = (ULONG_PTR)memblock;
/* ptr points to the pointer to starting of the memory block */
	ptr = (ptr & ~(PTR_SZ -1)) - PTR_SZ;
/* ptr is the pointer to the start of memory block*/
	ffree(*((PULONG_PTR)ptr));
}
