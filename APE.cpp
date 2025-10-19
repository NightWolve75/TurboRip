#include <global.h>
#include "APE.h"

proc_APECompress_Create   g_APECompress_Create;
proc_APECompress_Destroy  g_APECompress_Destroy;
proc_APECompress_Start    g_APECompress_Start;
proc_APECompress_AddData  g_APECompress_AddData;
proc_APECompress_Finish   g_APECompress_Finish;
proc_GetVersionNumber     g_APEGetVersionNumber;

char APE_MAC_ZIP[] = "NewMACDll.zip";
char APE_MAC_DLL[] = "NewMACDll.dll";
char APE_MACD_ZIP[] = "MACD.zip";
char APE_MACD_EXE[] = "MACD.exe";
char APE_BATCH_FILENAME[] = "APE_TO_WAVE.bat";
char APE_BATCH_FILETEXT[] =
"@ECHO OFF"BRK
"CLS"BRK
"ECHO."BRK
"ECHO Decompression of all APE audio files will now begin! You'll be prompted"BRK
"ECHO to delete the APE files or not afterwards if you want to free space."BRK
"ECHO Once decoded to WAVE, the CUE file can be burned to CD-R or mounted!"BRK
"ECHO Official GUI APE app + Winamp plugin @ http://www.monkeysaudio.com/"BRK
"ECHO."BRK
"ECHO (This batch file was created by TurboRip for APE CD-ROM archival storage.)"BRK
"ECHO (Get the latest version here: http://www.ysutopia.net/get.php?id=TurboRip)"BRK
"ECHO."BRK
"ECHO -NightWolve"BRK
"ECHO."BRK
"PAUSE"BRK
"MACD"BRK;

// ** Compression Levels
// Fast   = 1000
// Normal = 2000
// High   = 3000
// ExHigh = 4000
// Insane = 5000
int APE_COMPRESSION_LEVEL = COMPRESSION_LEVEL_NORMAL; // Default
HINSTANCE ghlibAPE_ENC;

extern "C" {

BOOL __fastcall InitMACAPE() {
	BOOLEAN bResult = FALSE;
	DWORD dwVersion;
	FastPrint("Notice: Verifying APE Encoding Support...\r\n");
	bResult = FILE_EXISTS(APE_MAC_DLL);
	if ( bResult == FALSE ) {
		printf("Notice: The required MAC APE Library is not present\r\nExtract: %s\r\n", APE_MAC_DLL);
		if ( !GetResourceFile(NULL, APE_MAC_ZIP, "APE_MACDLL_ZIP", RT_RCDATA) )
			return FALSE;
		if ( UnZipFile32(APE_MAC_ZIP, APE_MAC_DLL) ) {
			DeleteFile(APE_MAC_ZIP);
			bResult = TRUE;
		}
	}
	if ( bResult ) {
		ghlibAPE_ENC = LoadLibraryA(APE_MAC_DLL); // Prefers system folder to current!
		if( !ghlibAPE_ENC ) {
			PrintLastSysError("Error: Cannot load MAC APE Library (%s)", APE_MAC_DLL);
			return FALSE;
		}
	// Get Interface functions from the DLL
		g_APECompress_Create = (proc_APECompress_Create) GetProcAddress(ghlibAPE_ENC, "c_APECompress_Create");
		g_APECompress_Destroy = (proc_APECompress_Destroy) GetProcAddress(ghlibAPE_ENC, "c_APECompress_Destroy");
		g_APECompress_Start = (proc_APECompress_Start) GetProcAddress(ghlibAPE_ENC, "c_APECompress_Start");
		g_APECompress_AddData = (proc_APECompress_AddData) GetProcAddress(ghlibAPE_ENC, "c_APECompress_AddData");
		g_APECompress_Finish = (proc_APECompress_Finish) GetProcAddress(ghlibAPE_ENC, "c_APECompress_Finish");
		g_APEGetVersionNumber = (proc_GetVersionNumber) GetProcAddress(ghlibAPE_ENC, "GetVersionNumber");
	// Check if all interfaces are present
		if ( !(g_APECompress_Create && g_APECompress_Destroy && g_APECompress_Start && g_APECompress_AddData && g_APECompress_Finish && g_APEGetVersionNumber) ) {
			PrintLastSysError("Error: Unable to obtain required MAC interfaces");
			return FALSE;
		}
		dwVersion = g_APEGetVersionNumber();
		printf("Notice: MAC APE Encoder version %4lu has been loaded...\r\n\n", dwVersion);
		bResult = TRUE;
	}
	return bResult;
}

BOOL __fastcall ExtractMACD_EXE() {
	if ( !GetResourceFile(NULL, APE_MACD_ZIP, "APE_MACEXE_ZIP", RT_RCDATA) )
		return FALSE;
	if ( UnZipFile32(APE_MACD_ZIP, APE_MACD_EXE) ) {
		DeleteFile(APE_MACD_ZIP);
		return WriteNewFile(APE_BATCH_FILENAME, APE_BATCH_FILETEXT, sizeof(APE_BATCH_FILETEXT)-1);
	}
	return FALSE;
}

void __fastcall FreeMACAPE() {
	if ( ghlibAPE_ENC ) {
		FreeLibrary(ghlibAPE_ENC);
		ghlibAPE_ENC = NULL;
	}
}

} // end extern "C" {
