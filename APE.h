/*****************************************************************************************
Monkey's Audio MACDll.h (include for using MACDll.dll in your projects)
Copyright (C) 2000-2004 by Matthew T. Ashland   All Rights Reserved.
*****************************************************************************************/
#ifndef APE_MACDLL_H
#define APE_MACDLL_H
/*****************************************************************************************
Defines
*****************************************************************************************/
#undef _UNICODE
#undef UNICODE
/*****************************************************************************************
Global includes
****************************************************************************************/
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#define ENABLE_COMPRESSION_MODE_FAST
#define ENABLE_COMPRESSION_MODE_NORMAL
#define ENABLE_COMPRESSION_MODE_HIGH
#define ENABLE_COMPRESSION_MODE_EXTRA_HIGH

typedef unsigned __int32                            uint32;
typedef __int32                                     int32;
typedef unsigned __int16                            uint16;
typedef __int16                                     int16;
typedef unsigned __int8                             uint8;
typedef __int8                                      int8;
typedef char                                        str_ansi;
typedef unsigned char                               str_utf8;

/*****************************************************************************************
Global defines
*****************************************************************************************/
#define MAC_VERSION_NUMBER                              3990
/*****************************************************************************************
Error Codes
*****************************************************************************************/
// success
#ifndef ERROR_SUCCESS
	#define ERROR_SUCCESS                                   0
#endif

// file and i/o errors (1000's)
#define ERROR_IO_READ                                   1000
#define ERROR_IO_WRITE                                  1001
#define ERROR_INVALID_INPUT_FILE                        1002
#define ERROR_INVALID_OUTPUT_FILE                       1003
#define ERROR_INPUT_FILE_TOO_LARGE                      1004
#define ERROR_INPUT_FILE_UNSUPPORTED_BIT_DEPTH          1005
#define ERROR_INPUT_FILE_UNSUPPORTED_SAMPLE_RATE        1006
#define ERROR_INPUT_FILE_UNSUPPORTED_CHANNEL_COUNT      1007
#define ERROR_INPUT_FILE_TOO_SMALL                      1008
#define ERROR_INVALID_CHECKSUM                          1009
#define ERROR_DECOMPRESSING_FRAME                       1010
#define ERROR_INITIALIZING_UNMAC                        1011
#define ERROR_INVALID_FUNCTION_PARAMETER                1012
#define ERROR_UNSUPPORTED_FILE_TYPE                     1013
#define ERROR_UPSUPPORTED_FILE_VERSION                  1014

// memory errors (2000's)
#define ERROR_INSUFFICIENT_MEMORY                       2000

// dll errors (3000's)
#define ERROR_LOADINGAPE_DLL                            3000
#define ERROR_LOADINGAPE_INFO_DLL                       3001
#define ERROR_LOADING_UNMAC_DLL                         3002

// general and misc errors
#define ERROR_USER_STOPPED_PROCESSING                   4000
#define ERROR_SKIPPED                                   4001

// programmer errors
#define ERROR_BAD_PARAMETER                             5000

// IAPECompress errors
#define ERROR_APE_COMPRESS_TOO_MUCH_DATA                6000

// unknown error
#define ERROR_UNDEFINED                                -1

#define ERROR_EXPLANATION \
    { ERROR_IO_READ                               , "I/O read error" },                         \
    { ERROR_IO_WRITE                              , "I/O write error" },                        \
    { ERROR_INVALID_INPUT_FILE                    , "invalid input file" },                     \
    { ERROR_INVALID_OUTPUT_FILE                   , "invalid output file" },                    \
    { ERROR_INPUT_FILE_TOO_LARGE                  , "input file file too large" },              \
    { ERROR_INPUT_FILE_UNSUPPORTED_BIT_DEPTH      , "input file unsupported bit depth" },       \
    { ERROR_INPUT_FILE_UNSUPPORTED_SAMPLE_RATE    , "input file unsupported sample rate" },     \
    { ERROR_INPUT_FILE_UNSUPPORTED_CHANNEL_COUNT  , "input file unsupported channel count" },   \
    { ERROR_INPUT_FILE_TOO_SMALL                  , "input file too small" },                   \
    { ERROR_INVALID_CHECKSUM                      , "invalid checksum" },                       \
    { ERROR_DECOMPRESSING_FRAME                   , "decompressing frame" },                    \
    { ERROR_INITIALIZING_UNMAC                    , "initializing unmac" },                     \
    { ERROR_INVALID_FUNCTION_PARAMETER            , "invalid function parameter" },             \
    { ERROR_UNSUPPORTED_FILE_TYPE                 , "unsupported file type" },                  \
    { ERROR_INSUFFICIENT_MEMORY                   , "insufficient memory" },                    \
    { ERROR_LOADINGAPE_DLL                        , "loading MAC.dll" },                        \
    { ERROR_LOADINGAPE_INFO_DLL                   , "loading MACinfo.dll" },                    \
    { ERROR_LOADING_UNMAC_DLL                     , "loading UnMAC.dll" },                      \
    { ERROR_USER_STOPPED_PROCESSING               , "user stopped processing" },                \
    { ERROR_SKIPPED                               , "skipped" },                                \
    { ERROR_BAD_PARAMETER                         , "bad parameter" },                          \
    { ERROR_APE_COMPRESS_TOO_MUCH_DATA            , "APE compress too much data" },             \
    { ERROR_UNDEFINED                             , "undefined" }

#define COMPRESSION_LEVEL_FAST          1000
#define COMPRESSION_LEVEL_NORMAL        2000
#define COMPRESSION_LEVEL_HIGH          3000
#define COMPRESSION_LEVEL_EXTRA_HIGH    4000
#define COMPRESSION_LEVEL_INSANE        5000

#define CREATE_WAV_HEADER_ON_DECOMPRESSION    -1
#define MAX_AUDIO_BYTES_UNKNOWN -1
/*****************************************************************************************
IAPECompress wrapper(s)
*****************************************************************************************/
typedef void* APE_COMPRESS_HANDLE;
typedef int  (__stdcall * proc_GetVersionNumber)();
typedef void*(__stdcall * proc_APECompress_Create)(int *pErrorCode); 
typedef void (__stdcall * proc_APECompress_Destroy)(APE_COMPRESS_HANDLE hAPECompress); 
//int __stdcall c_APECompress_Start(APE_COMPRESS_HANDLE hAPECompress, const char * pOutputFilename, const WAVE_FORMAT_EX * pwfeInput, int nMaxAudioBytes = MAX_AUDIO_BYTES_UNKNOWN, int nCompressionLevel = COMPRESSION_LEVEL_NORMAL, const unsigned char * pHeaderData = NULL, int nHeaderBytes = CREATE_WAV_HEADER_ON_DECOMPRESSION);
typedef int  (__stdcall * proc_APECompress_Start)(APE_COMPRESS_HANDLE, const char *, const WAVE_FORMAT_EX *, int, int, const void *, int);
typedef int  (__stdcall * proc_APECompress_AddData)(APE_COMPRESS_HANDLE hAPECompress, unsigned char *pData, int nBytes);
typedef int  (__stdcall * proc_APECompress_Finish)(APE_COMPRESS_HANDLE hAPECompress, unsigned char * pTerminatingData, int nTerminatingBytes, int nWAVTerminatingBytes);

extern proc_APECompress_Create   g_APECompress_Create;
extern proc_APECompress_Destroy  g_APECompress_Destroy;
extern proc_APECompress_Start    g_APECompress_Start;
extern proc_APECompress_AddData  g_APECompress_AddData;
extern proc_APECompress_Finish   g_APECompress_Finish;

extern "C" {
	BOOL __fastcall InitMACAPE();
	BOOL __fastcall ExtractMACD_EXE();
	void __fastcall FreeMACAPE();
}
extern int APE_COMPRESSION_LEVEL;

#endif // #ifndef APE_MACDLL_H
