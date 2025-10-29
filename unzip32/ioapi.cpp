/* ioapi.c -- IO base function header for compress/uncompress .zip
   files using zlib + zip or unzip API

   Version 1.01, May 8th, 2004

   Copyright (C) 1998-2004 Gilles Vollant
*/

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zlib.h"
#include "ioapi.h"

voidpf ZCALLBACK fopen_file_func OF((
   voidpf opaque,
   const char* filename,
   int mode));

uLong ZCALLBACK fread_file_func OF((
   voidpf opaque,
   voidpf stream,
   void* buf,
   uLong size));

uLong ZCALLBACK fwrite_file_func OF((
   voidpf opaque,
   voidpf stream,
   const void* buf,
   uLong size));

long ZCALLBACK ftell_file_func OF((
   voidpf opaque,
   voidpf stream));

long ZCALLBACK fseek_file_func OF((
   voidpf opaque,
   voidpf stream,
   uLong offset,
   int origin));

int ZCALLBACK fclose_file_func OF((
   voidpf opaque,
   voidpf stream));

int ZCALLBACK ferror_file_func OF((
   voidpf opaque,
   voidpf stream));

voidpf ZCALLBACK fopen_file_func (
	voidpf opaque,
	const char* filename,
	int mode
)
{
    HANDLE file;
/*    const char* mode_fopen = NULL;
    if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER)==ZLIB_FILEFUNC_MODE_READ)
        mode_fopen = "rb";
    else if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
        mode_fopen = "r+b";
    else if (mode & ZLIB_FILEFUNC_MODE_CREATE)
        mode_fopen = "wb";*/
	file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( file == INVALID_HANDLE_VALUE )
		return NULL;
    /*if ((filename!=NULL) && (mode_fopen != NULL))
        file = fopen(filename, mode_fopen);*/
    return file;
}


uLong ZCALLBACK fread_file_func (
   voidpf opaque,
   voidpf stream,
   void* buf,
   uLong size
)
{
    uLong ret;
	return ReadFile(stream, buf, size, &ret, NULL)? ret : -1;
}


uLong ZCALLBACK fwrite_file_func (
	voidpf opaque,
	voidpf stream,
	const void* buf,
	uLong size
)
{
	uLong ret;
	return WriteFile(stream, buf, size, &ret, NULL)? ret : -1;
}

long ZCALLBACK ftell_file_func (
	voidpf opaque,
	voidpf stream
)
{
	return SetFilePointer(stream, 0, NULL, FILE_CURRENT);
}

long ZCALLBACK fseek_file_func (
   voidpf opaque,
   voidpf stream,
   uLong offset,
   int origin
)
{
    /*int fseek_origin=0;
    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR :
        fseek_origin = SEEK_CUR;
        break;
    case ZLIB_FILEFUNC_SEEK_END :
        fseek_origin = SEEK_END;
        break;
    case ZLIB_FILEFUNC_SEEK_SET :
        fseek_origin = SEEK_SET;
        break;
    default: return -1;
    }*/
    return SetFilePointer(stream, offset, NULL, origin) == INVALID_SET_FILE_POINTER;
}

int ZCALLBACK fclose_file_func (
	voidpf opaque,
	voidpf stream
)
{
	return CloseHandle(stream);
}

int ZCALLBACK ferror_file_func (
	voidpf opaque,
	voidpf stream
)
{
	return GetLastError();
}

void fill_fopen_filefunc (zlib_filefunc_def* pzlib_filefunc_def)
{
	pzlib_filefunc_def->zopen_file = fopen_file_func;
	pzlib_filefunc_def->zread_file = fread_file_func;
	pzlib_filefunc_def->zwrite_file = fwrite_file_func;
	pzlib_filefunc_def->ztell_file = ftell_file_func;
	pzlib_filefunc_def->zseek_file = fseek_file_func;
	pzlib_filefunc_def->zclose_file = fclose_file_func;
	pzlib_filefunc_def->zerror_file = ferror_file_func;
	pzlib_filefunc_def->opaque = NULL;
}
