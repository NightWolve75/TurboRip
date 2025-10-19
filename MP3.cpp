#include <global.h>
#include "BladeMP3.h"

// Dependency: Add line below to resource file to include the LAME MP3 DLL
// MP3_LAMENC_ZIP RCDATA "BIN/LAME_ENC.zip"

BE_ERR (__cdecl *beInitStream)			(PBE_CONFIG, PDWORD, PDWORD, PHBE_STREAM);
BE_ERR (__cdecl *beEncodeChunk)			(HBE_STREAM, DWORD, PSHORT, PBYTE, PDWORD);
BE_ERR (__cdecl *beDeinitStream)		(HBE_STREAM, PBYTE, PDWORD);
BE_ERR (__cdecl *beCloseStream)			(HBE_STREAM);
VOID   (__cdecl *beVersion)				(PBE_VERSION);
BE_ERR (__cdecl *beWriteVBRHeader)		(LPCSTR);
INT    (__cdecl *lame_set_num_channels)	(HBE_STREAM, INT);

BE_CONFIG beConfig;

char MP3_LAME_ZIP[] = "LAME_ENC.zip";
char MP3_LAME_W95[] = "LAME_W95.zip";
char MP3_LAME_DLL[] = "LAME_ENC.dll";
HINSTANCE ghlibLAME_ENC;

extern "C" {

BOOL __fastcall InitLAMEMP3() {
	BE_VERSION Version;
	BOOLEAN bResult;
	LPSTR lpZipFile;
	MemZero(sizeof(Version), &Version);
	FastPrint("Notice: Verifying LAME MP3 Encoding Support...\r\n");
	bResult = FILE_EXISTS(MP3_LAME_DLL);
	if ( !bResult ) {
		printf("Notice: The required LAME MP3 Library is not present\r\nExtract: %s\r\n", MP3_LAME_DLL);
		if ( gOSInfo.IsWinNT && gOSInfo.MajorVersion > 4 ) { // Use latest LAME DLL for >=Win2K/XP/V/7/8/10
			if ( !GetResourceFile(NULL, MP3_LAME_ZIP, "MP3_LAMENC_ZIP", RT_RCDATA) )
				return FALSE;
			lpZipFile = MP3_LAME_ZIP;
		} else { // Use old 9x LAME dll for Win95/98/ME/NT4
			if ( !GetResourceFile(NULL, MP3_LAME_W95, "MP3_LAME95_ZIP", RT_RCDATA) )
				return FALSE;
			lpZipFile = MP3_LAME_W95;
		}
		if ( UnZipFile32(lpZipFile, MP3_LAME_DLL) ) {
			DeleteFile(lpZipFile);
			bResult = TRUE;
		}
	}
	if ( bResult ) {
		ghlibLAME_ENC = LoadLibrary(MP3_LAME_DLL);
		if( !ghlibLAME_ENC ) {
			PrintLastSysError("Error: Cannot load LAME MP3 Library (%s)", MP3_LAME_DLL);
			return FALSE;
		}
	// Get Interface functions from the DLL
		beVersion = (BEVERSION) GetProcAddress(ghlibLAME_ENC, TEXT_BEVERSION);
		beInitStream = (BEINITSTREAM) GetProcAddress(ghlibLAME_ENC, TEXT_BEINITSTREAM);
		beEncodeChunk = (BEENCODECHUNK) GetProcAddress(ghlibLAME_ENC, TEXT_BEENCODECHUNK);
		beDeinitStream = (BEDEINITSTREAM) GetProcAddress(ghlibLAME_ENC, TEXT_BEDEINITSTREAM);
		beCloseStream = (BECLOSESTREAM) GetProcAddress(ghlibLAME_ENC, TEXT_BECLOSESTREAM);
		beWriteVBRHeader = (BEWRITEVBRHEADER) GetProcAddress(ghlibLAME_ENC, TEXT_BEWRITEVBRHEADER);
		lame_set_num_channels = (LAME_SET_NUM_CHANNELS) GetProcAddress(ghlibLAME_ENC, TEXT_LAME_SET_NUM_CHANNELS);
	// Check if all interfaces are present
		if ( !(beInitStream && beEncodeChunk && beDeinitStream && beCloseStream && beVersion && beWriteVBRHeader && lame_set_num_channels) ) {
			PrintLastSysError("Error: Unable to obtain required LAME MP3 interfaces");
			return FALSE;
		}
	// Load default settings
		beConfig.dwConfig = BE_CONFIG_LAME; // Use the LAME config structure
		beConfig.format.LHV1.dwSampleRate = 44100; // INPUT FREQUENCY
		beConfig.format.LHV1.dwStructSize = sizeof(BE_CONFIG);
		beConfig.format.LHV1.dwStructVersion = 1;
		if ( !beConfig.format.LHV1.dwReSampleRate )
            beConfig.format.LHV1.dwReSampleRate = 44100; // OUTPUT FREQUENCY
		if ( !beConfig.format.LHV1.dwBitrate )
			beConfig.format.LHV1.dwBitrate = 128;        // MINIMUM BIT RATE
		if ( beConfig.format.LHV1.dwBitrate < 32 || beConfig.format.LHV1.dwMaxBitrate < 32 || beConfig.format.LHV1.dwBitrate == 144 )
			beConfig.format.LHV1.dwMpegVersion = MPEG2;  // MPEG VERSION (II)
		else
			beConfig.format.LHV1.dwMpegVersion = MPEG1;  // MPEG VERSION (I)
		beConfig.format.LHV1.bOriginal = TRUE;
		beVersion(&Version); // Get the version info
		printf(
			"Notice: LAME MP3 Encoder version %u.%02u [Engine %u.%02u] (%u/%u/%u) was loaded...\r\n\n",
			Version.byDLLMajorVersion,
			Version.byDLLMinorVersion,
			Version.byMajorVersion,
			Version.byMinorVersion,
			Version.byMonth,
			Version.byDay,
			Version.wYear
		);
		bResult = TRUE;
	}
	return bResult;
}

void __fastcall FreeLAMEMP3() {
	if ( ghlibLAME_ENC ) {
		FreeLibrary(ghlibLAME_ENC);
		ghlibLAME_ENC = NULL;
	}
}

} // end extern "C" {
