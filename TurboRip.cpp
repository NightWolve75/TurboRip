#include "TurboRip.h"
#include "BladeMP3.h"
#include "APE.h"
#include "WAVE.h"
#include <CDDB.h>
#include <shellapi.h>

#define WWW_SPAM_OFF
//#define DEBUG_TRACE

char PERCENT_PROGRESS[] = "Progress : 100%, LBA: 123456 to 123456, (99.99 seconds)           \r";
char SECONDS_ELAPSED[]  = " seconds)    ";
DWORD gTickCountBegin, gTickCountEnd, gTickTotal, nTimeSecs, nTimeMSecs;
TurboRipParams ParamList;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
	if ( bGlobalLoopAlert >= 5 )
		if ( MessageBoxA(GetForegroundWindow(), "Do you wanna finish ripping the current track before exiting?", CONSOLE_TITLE, MB_YESNO | MB_TOPMOST) == IDYES ) {
			PrintColoredText(STR_SIZE(BRK"Abort requested. Shutting down upon completion of track extraction..."BRK), TEXT_COLOR_RED);
			bGlobalLoopAlert = 6;
			return TRUE;
		}
	Beep(750, 300);
	bGlobalAbort = TRUE;
	PrintColoredText(STR_SIZE(BRK"Abort requested. Shutting down as soon as possible..."BRK), TEXT_COLOR_RED);
	if ( bGlobalLoopAlert ) {
		Sleep(1000);
		return TRUE;
	}
	return FALSE;
}

DWORD __fastcall SetDetectCDDriveSpeed() {
	DWORD nMinSpeed, nTrueMinSpeed, nSpeed;
	LONG i;
	nTrueMinSpeed = DEF_MIN_SPEED;
	if ( ASPISetCDSpeed(0xFFFF) ) {
		if ( ParamList.Speed != 0xFFFF ) {
			FastPrint("Read Speed(s) Detected   :");
			__asm XOR EAX, EAX
			__asm MOV i, EAX
			__asm DEC EAX
			__asm MOV nMinSpeed, EAX ;// nMinSpeed = 0xFFFFFFFF
			__asm MOV WORD PTR i, AX ;// i = 0x0000FFFF
			for ( ; i >= 1; ASPISetCDSpeed((WORD)--i) ) {
				if ( bGlobalAbort )
					return 0;
				nSpeed = ASPIGetCDSpeed();
				if ( nMinSpeed > nSpeed ) {
					nMinSpeed = nSpeed;
					i = round(nSpeed/XX1);
					printf(" %lux", i);
					nTrueMinSpeed = i;
				} else if ( nSpeed == 0 )
					break;
			}
			FastPrint(NEW_LINE);
		}
		ASPISetCDSpeed(ParamList.Speed);
		nSpeed = round(ASPIGetCDSpeed()/XX1);
		if ( ParamList.Speed == 0xFFFF )
			ParamList.Speed = (WORD)nSpeed;
		printf("Read Speed Selected      : %lux\r\n", nSpeed);
		if ( (WORD)nSpeed != ParamList.Speed )
			printf("Note: Unable to set read speed at %hux (%lux will be used)\r\n", ParamList.Speed, nSpeed);
		ParamList.Speed = (WORD)nSpeed;
	} else {
		PrintLastSysError("Error: Unable to set drive speed : %s", ASPIGetLastError("Failure"));
		return 0;
	}
	return nTrueMinSpeed;
}

// The characters that are disallowed by Windows for
// filenames or folders are:  \/:*?"<>|

inline void __fastcall WINDOWSify(PSTR lpFileName) {
	char c;
	while ( c = *lpFileName ) {
		if ( c == '\\' || c == '/' || c == ':' || c == '|' )
			*lpFileName = '-';
		else if ( c == '*' || c == '?' || c == '"' || c == '<' || c == '>' )
			*lpFileName = '_';
		lpFileName++;
	}
}

// I don't know too much about XBOX's FAT-X character type limits but I do know
// the file name is limited to 42 characters in length, including the extension.
// As such, I've limited the character types to alpha, digits, and a few others.

void __fastcall XBOXify(PSTR lpFileName) {
	PSTR lpStart;
	char c;
	lpStart = lpFileName;
	for (;;) {
		c = *lpFileName;
		if ( !c )
			break;
		if ( isalpha(c) || isdigit(c) || c == '-' || c == '(' || c == ')' )
			lpFileName++;
		else if ( c == '&' )
			*lpFileName++ = '-';
		else {
			__asm MOV  ECX, lpFileName
			__asm LEA  EDX, [ECX+1]
			__asm CALL StrCpy
		}
	}
	if ( StrLen(lpStart) > 32 ) // 42 - 10, since you have the '.' + extension + 'HUGOX' (possibly)
		lpStart[32] = NULL;
}

int __fastcall LoadSessionsAndCDText(TOC_FORMAT_0010 *tocFull, BYTE FirstTrack) {
	CDTEXT_PACK_DATA *lpCDTextPack, *lpLastCDTextPack;
	TOC_FORMAT_0001 tocSessInfo;
	BOOL bHasCDTEXT;
	long TextPackSize;
	PSTR lpParam;
// Get the full TOC for rare Multi-Session considerations (CD-EXTRA Music CDs!)
	FastPrint("Notice: Reading the full TOC... ");
	if ( !ASPIGetTOCFull(tocFull) ) {


		//SetTextColorRed();


		FastPrint("Failure...\r\n");
	// ??????????????? Audio-only blocked full TOC test to eliminate DVDs ?? WHY?
	// Likely DVD media is being used
		if ( gSenseKey == 5 && gAddSenseCode == 0x24 && gAddSenQual == 0 ) {
			FastPrint(ERROR_NON_CDROM_MEDIA);
		} else {
		// Unknown error at this point
			lpParam = ASPIGetLastError(NULL);
			if ( lpParam )
				printf("Error: %s\r\n", lpParam);
		}
		//SetTextColorWhite();
		return -1; // Fatal error, old drive, can't read session count OR DVD media
	}
	FastPrint("OK...\r\n\n"); // Full TOC was successfully read
	MasterTOC.TotalSessions = 1;
	if ( tocFull->LastSession > 1 ) { // Possible multi-session CD!
	// Invalid Multi-Session TOC data possible (e.g. Virtual Drives)
	// Try minor validation before declaring it a multisession disc...
		if ( tocFull->FirstSession == 1 && tocFull->TrackData[0].SessionNumber == 1 ) {
			MasterTOC.TotalSessions = tocFull->LastSession;
			ASPIGetTOCSession(&tocSessInfo);
		// Worth a shot - One more minor validation to avoid a false read of a multi-session disc
			if ( tocSessInfo.FirstTrackLastSession == FirstTrack )
				MasterTOC.TotalSessions = 1; // It's a single session disc _most_ likely!
		}
	}
	if ( MasterTOC.TotalSessions > 1 )
		printf("Notice: A Multi-Session CD was detected with %hu sessions...\r\n", MasterTOC.TotalSessions);
// Load CD-TEXT data if present and supported
	FastPrint("Notice: Scanning for CD-TEXT packs...");
	lpCDTextPack = ASPIGetCDTEXT(&TextPackSize);
	if ( TextPackSize < 0 ) {
		PrintLastSysError(BRK"Error: %s", ASPIGetLastError("Failure"));
	} else if ( TextPackSize == 0 ) {
		FastPrint(" Not Found...\r\n\n");
	} else {
		printf(" Processing %lu bytes...\r\n\n", TextPackSize);
		lpLastCDTextPack = lpCDTextPack;
		bHasCDTEXT = FALSE;
		while ( lpCDTextPack->PackType && TextPackSize > 0 ) {
			//printf("PackType: %02X, TrackNumber: %02d\r\n", lpCDTextPack->PackType, lpCDTextPack->TrackOrPackNumber);
		// Handle only known CD-TEXT pack types
			if ( lpCDTextPack->PackType >= 0x80 && lpCDTextPack->PackType <= 0x87 || lpCDTextPack->PackType == 0x8E ) {
				lpCDTextPack = UnpackCDTextData(&MasterTOC, lpCDTextPack);
				if ( lpCDTextPack > lpLastCDTextPack )
					TextPackSize -= SubPtr(long, lpCDTextPack, lpLastCDTextPack);
				lpLastCDTextPack = lpCDTextPack;
				bHasCDTEXT = TRUE;
			} else {
				TextPackSize -= sizeof(CDTEXT_PACK_DATA);
				lpCDTextPack++;
			}
			if ( !lpCDTextPack ) // If null, something went wrong with CD-TEXT data, abort!
				return 0;
		}
		if ( bHasCDTEXT ) {
			MasterTOC.CDTextData.bHasCDText = TRUE;
			if ( *MasterTOC.CDTextData.AlbumTitle ) {
				printf("Album Title: \"%s\"\r\n\n", MasterTOC.CDTextData.AlbumTitle);
				gSetCD.CDTitle = MasterTOC.CDTextData.AlbumTitle;
			}
		}
	}
	return 1;
}

/*
BOOL HomeBrewBlock() {
	PSTR pHomeBrewURL;
	BOOLEAN bBlockIt;
	if ( !gSetCD.CDDBID )
		return FALSE;
	bBlockIt = FALSE;
// Block MindRec.com games: Meteor Blaster DX
	if ( gSetCD.CDDBID == 0x1E111315 ) {
		pHomeBrewURL = "http://www.mindrec.com";
		bBlockIt = TRUE;
	}
	if ( bBlockIt ) {
		PrintColoredText(STR_SIZE(
			BRK"Abort: Sorry, this is a homebrew game developed by dedicated PCE fans! Please"BRK
			"support their efforts by avoiding pirating it at least until it sells out."BRK
			), TEXT_COLOR_RED);
		ShellExecuteA(NULL, "open", pHomeBrewURL, NULL, NULL, SW_SHOWNOACTIVATE);
		Sleep(1300);
		SetWindowTopMost(ghWndConsole);
		return TRUE;
	}
	return FALSE;
}*/

// Dependency: Call InitConsole() first to init ghConsoleInput handle!

void __fastcall GetDiscTitle(LPSTR pDiscTitle) {
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	DWORD dwReadSize;
// Good to keep input buffer flushed
	FlushConsoleInputBuffer(ghConsoleInput);
// Request a base filename since there was no TOC match nor one given via parameter
	FastPrint(MSG_NO_FILENAME_PROMPT);
	GetConsoleScreenBufferInfo(ghConsoleOutput, &csbiInfo);
	do {
		ReadConsoleA(ghConsoleInput, pDiscTitle, MAX_PATH-1, &dwReadSize, NULL);
		if ( dwReadSize > 5 ) // Enforce 4 letter minimum for CD title!
			break;
		else {
			FillConsoleOutputCharacter(ghConsoleOutput, ' ', dwReadSize, csbiInfo.dwCursorPosition, &gnNumberOfBytes);
			SetConsoleCursorPosition(ghConsoleOutput, csbiInfo.dwCursorPosition);			
		}
	} while ( TRUE );
	pDiscTitle[dwReadSize-2] = NULL;
	WINDOWSify(pDiscTitle); // Enforce Windows' rules for character limits
	if ( ParamList.XBOX ) // Enforce the XBOX FAT-X file system character limits on the name
		XBOXify(pDiscTitle);
	FlushConsoleInputBuffer(ghConsoleInput);
}

long __fastcall BinarySearchGameCDDB(DWORD nKey) {
	LONG LowIndex, MidIndex, HighIndex;
	LowIndex = 0;
	HighIndex = CD_TITLE_COUNT;
	do {
		MidIndex = (LowIndex + HighIndex) >> 1;
		if ( TOC_INFO_LIST[MidIndex].CDDBID == nKey )
			return MidIndex;
		else if ( TOC_INFO_LIST[MidIndex].CDDBID > nKey )
			HighIndex = --MidIndex;
		else
			LowIndex = ++MidIndex;
	} while ( LowIndex <= HighIndex );
	__asm XOR EAX, EAX
	__asm SUB EAX, 1
}

// Attempt to identify a PCE/TG-16/PC-FX CD by its TOC signature

TOC_INFO* __fastcall SearchTOCDB(LPTOC lpTocMSF, BYTE nTotalTracks) {
	long IndexTOC;
	DWORD CDDBID;
// 1) Convert TOC MSF data to a CDDB ID
	CDDBID = cddb_discid(lpTocMSF->TrackData, nTotalTracks);
// 2) Search for that CDDB ID in our game database
	IndexTOC = BinarySearchGameCDDB(CDDBID);
	if ( IndexTOC >= 0 )
		return &TOC_INFO_LIST[IndexTOC];
	return NULL;
}

void __fastcall SetGameCDTitleInfo() {
	if ( !gSetCD.CDTitle )
		return;
// Copy title string from CD array to gTrackTitle	
	StrCpy(gTrackTitle, gSetCD.CDTitle);
	__asm MOV ECX, EAX
// 0=PC-FX, 1=PC Engine CD	
	if ( gSetCD.CDType < 2 )
		__asm MOV DWORD PTR [ECX], ')J( ' // " (J)"
// 2=TurboGrafx-16 CD
	else
		__asm MOV DWORD PTR [ECX], ')U( ' // " (U)"
	__asm MOV BYTE PTR [ECX+4], 0
	gSetCD.CDTitle = gTrackTitle;
}

// Dependencies:
// 1. Having successfully read the CD's TOC
// 2. Calling ASPIAnalyzeTrackModes() prior to this!

// XBOX's HDD FAT-X File system only supports 42 character long file names.

BOOL __fastcall WriteHUGOXCUEFile(LPSTR lpBaseFileName, LPTOC lpToc) {
	char szFileName[MAX_PATH], Line[1024], TrackMode;
	DWORD i, LastTrack, Size, MM, SS, FF;
	TRACK_DESCRIPTOR_00 *lpTrack;
	LPSTR szTrackMode, szExt;
	BOOL bResult = FALSE;
	HANDLE hFile;
// Copy title and append .CUE extension
	StrCpy(szFileName, lpBaseFileName);
	mStrNCpy32(EAX, OFFSET extHUGOX, SIZE extHUGOX);
// Open new CUE file for writing
	hFile = _CreateNewFile(szFileName);
	printf("Writing HUGO-X CUE file: %s\r\n\n", szFileName);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		lpTrack = lpToc->TrackData;
		LastTrack = lpToc->LastTrack;
		for ( i = lpToc->FirstTrack; i <= LastTrack; i = lpTrack->TrackNumber ) {
			TrackMode = lpTrack->Reserved1;
			if ( TrackMode == AUDIO_CDDA ) {
				szTrackMode = CDTRACK_MODES[AUDIO_CDDA];
				szExt = extMP3;
			} else if ( TrackMode == MODE1_2048 || TrackMode == MODE1_2352 ) {
				szTrackMode = CDTRACK_MODES[TrackMode];
				szExt = extISO;
			} else {
				PrintColoredText(STR_SIZE(ERROR_TRACK_MODE), TEXT_COLOR_RED);
				bResult = FALSE;
				break;
			}
	// Omission of 2:00/150 increment appears to be proper for HUGOX format...
	// Format has been matched to old XBOX cddissect MP3-only app!
			MM = *(LPDWORD)lpTrack->Address;
			FF = MM % 75;
			MM /= 75;
			SS = MM % 60;
			MM /= 60;
			Size = wsprintfA(
				Line,
				"FILE \"%02lu %s%s\" BINARY\r\n"
				"TRACK %u %s\r\n"
				"INDEX 1 %02lu:%02lu:%02lu\r\n",
				i, lpBaseFileName, szExt, i, szTrackMode, MM, SS, FF
			);
			bResult = _WriteVerify(hFile, Line, Size);
			if ( !bResult ) {
				PrintLastSysError(ERROR_FILE_WRITE);
				break;
			}
			lpTrack++;
		}
		CloseHandle(hFile);
	} else
		PrintLastSysError(ERROR_FILE_CREATE);
	return bResult;
}

DWORD __fastcall WriteCDTextTrackFields(HANDLE hFile, LPSTR lpTitle, LPSTR lpPerformer, LPSTR lpSongwriter, LPSTR lpComposer, LPSTR lpArranger, LPSTR lpMessage, LPSTR lpIRSC, LPSTR lpIndentSpacing) {
	CHAR szLine[1024];
	DWORD Size = 0;
// Add Title
	if ( *lpTitle ) {
		Size = wsprintfA(szLine, "%sTITLE \"%s\""BRK, lpIndentSpacing, lpTitle);
		if ( Size > 255 ) {
			*(LPDWORD)&szLine[251] = '\"\r\n\0';
			Size = 254;
		}
		WriteFile(hFile, &szLine, Size, &Size, NULL);
	}
// Add Performer
	if ( *lpPerformer ) {
		Size = wsprintfA(szLine, "%sPERFORMER \"%s\""BRK, lpIndentSpacing, lpPerformer);
		if ( Size > 255 ) {
			*(LPDWORD)&szLine[251] = '\"\r\n\0';
			Size = 254;
		}
		WriteFile(hFile, &szLine, Size, &Size, NULL);
	}
// Add Songwriter
	if ( *lpSongwriter ) {
		Size = wsprintfA(szLine, "%sSONGWRITER \"%s\""BRK, lpIndentSpacing, lpSongwriter);
		if ( Size > 255 ) {
			*(LPDWORD)&szLine[251] = '\"\r\n\0';
			Size = 254;
		}
		WriteFile(hFile, &szLine, Size, &Size, NULL);
	}
// Add Composer
	if ( *lpComposer ) {
		Size = wsprintfA(szLine, "%sREM COMPOSER \"%s\""BRK, lpIndentSpacing, lpComposer);
		if ( Size > 255 ) {
			*(LPDWORD)&szLine[251] = '\"\r\n\0';
			Size = 254;
		}
		WriteFile(hFile, &szLine, Size, &Size, NULL);
	}
// Add Arranger
	if ( *lpArranger ) {
		Size = wsprintfA(szLine, "%sREM ARRANGER \"%s\""BRK, lpIndentSpacing, lpArranger);
		if ( Size > 255 ) {
			*(LPDWORD)&szLine[251] = '\"\r\n\0';
			Size = 254;
		}
		WriteFile(hFile, &szLine, Size, &Size, NULL);
	}
// Add Message
	if ( *lpMessage ) {
		Size = wsprintfA(szLine, "%sREM MESSAGE \"%s\""BRK, lpIndentSpacing, lpMessage);
		if ( Size > 255 ) {
			*(LPDWORD)&szLine[251] = '\"\r\n\0';
			Size = 254;
		}
		WriteFile(hFile, &szLine, Size, &Size, NULL);
	}
// Add IRSC
	if ( *lpIRSC ) {
		Size = wsprintfA(szLine, "%sIRSC %s"BRK, lpIndentSpacing, lpIRSC);
		WriteFile(hFile, &szLine, Size, &Size, NULL);
	}
	return Size;
}

// Dependencies:
// 1. Having successfully read the CD's TOC and populated the MasterTOC object
// 2. Calling ASPIAnalyzeTrackModes() prior to this!

BOOL __fastcall WriteCUEFile(LPSTR lpBaseFileName, LPMASTERTOC lpToc) {
	BYTE i, FirstTrack, TotalTracks, LastSessionNumber, LastTrackMode, TrackMode;
	CHAR szFileName[MAX_PATH], Line[1024], szPreGapLine[32];
	MASTER_TOC_CDTEXT_TRACKINFO *lpCDTextTrack = NULL;
	PSTR szFileType, szTrackMode, szPregap;
	MASTER_TOC_TRACK_DESCRIPTOR *lpTrack;
	DWORD Size, MM, SS, FF;
	BOOL bResult = FALSE;
	HANDLE hFile;
// Copy title and append .CUE extension
	StrCpy(szFileName, lpBaseFileName);
	__asm MOV DWORD PTR [EAX], EXTCUE
	__asm MOV BYTE PTR [EAX+4], 0
// Open new CUE file for writing
	hFile = _CreateNewFile(szFileName);
	printf("Writing CUE file: \"%s\"\r\n\n", szFileName);
	if ( hFile != INVALID_HANDLE_VALUE ) {
		lpTrack = lpToc->TrackData;
		FirstTrack = lpToc->FirstTrack;
		TotalTracks = lpToc->TotalTracks;
		TrackMode = LastTrackMode = lpTrack->TrackMode;
		LastSessionNumber = lpTrack->SessionNumber;
		if ( MasterTOC.CDTextData.bHasCDText ) {
			lpCDTextTrack = lpToc->CDTextData.TrackData;
			Size = 0;
		// Add Catalog
			if ( *lpToc->CDTextData.Catalog ) {
				Size = wsprintfA(Line, "CATALOG %s"BRK, lpToc->CDTextData.Catalog);
				WriteFile(hFile, &Line, Size, &gnNumberOfBytes, NULL);
			}
		// Add UPC
			if ( *lpToc->CDTextData.UPC ) {
				Size = wsprintfA(Line, "REM UPC %s"BRK, lpToc->CDTextData.UPC);
				WriteFile(hFile, &Line, Size, &gnNumberOfBytes, NULL);
			}
		// Add Genre+Description
			if ( lpToc->CDTextData.Genre ) {
				Size = wsprintfA(Line, "REM GENRE \"%s\""BRK, lpToc->CDTextData.Genre);
				WriteFile(hFile, &Line, Size, &gnNumberOfBytes, NULL);
				if ( *lpToc->CDTextData.GenreDesc ) {
					Size = wsprintfA(Line, "REM GENRE_DESCRIPTION \"%s\""BRK, lpToc->CDTextData.GenreDesc);
					WriteFile(hFile, &Line, Size, &gnNumberOfBytes, NULL);
				}
			}
		// Add the rest for the album
			Size += WriteCDTextTrackFields(hFile, lpToc->CDTextData.AlbumTitle, lpToc->CDTextData.Performer, lpToc->CDTextData.Songwriter, lpToc->CDTextData.Composer, lpToc->CDTextData.Arranger, lpToc->CDTextData.Message, lpCDTextTrack->ISRC, gNull);
			if ( Size > 0 )
				WriteFile(hFile, NEW_LINE, 2, &gnNumberOfBytes, NULL);
		}
		for ( i = FirstTrack; TotalTracks--; i = lpTrack->TrackNumber, TrackMode = lpTrack->TrackMode ) {
			szFileType = StrCpy(szFileName, lpTrack->TrackFileName) - 4;
			if ( TrackMode == AUDIO_CDDA ) {
				if ( *(PDWORD)szFileType == EXTAPE ) // Is APE extension ?
					*(PDWORD)szFileType = EXTWAV;
				if ( *(PDWORD)szFileType != EXTBIN ) // Is BIN extension ?
					szFileType = "WAVE";
				else
					szFileType = "BINARY";
				szTrackMode = CDTRACK_MODES[AUDIO_CDDA];
			} else if ( TrackMode == MODE1_2048 || TrackMode == MODE1_2352 || TrackMode == MODE2_2352 ) {
				szTrackMode = CDTRACK_MODES[TrackMode];
				szFileType = "BINARY";
			} else {
				PrintColoredText(STR_SIZE(ERROR_TRACK_MODE), TEXT_COLOR_RED);
				bResult = FALSE;
				break;
			}
			if ( i == FirstTrack && (MM = lpTrack->StartAddress) > 0 ) {
		// Error ?? Adding of 2:00/150 missing and what am I doing ?
				FF = MM % 75;
				MM /= 75;
				SS = MM % 60;
				MM /= 60;
				wsprintfA(szPreGapLine, "    PREGAP %02lu:%02lu:%02lu"BRK, MM, SS, FF);
				szPregap = szPreGapLine;
			} else if ( (TrackMode>0) == (LastTrackMode>0) || lpTrack->SessionNumber != LastSessionNumber ) // No pregap needed (No track type transition)
				szPregap = (PSTR)gNull; // Prevent crash in Win9X (pointer can't be null when using wsprintfA; must point to null string!)
			else if ( TrackMode == AUDIO_CDDA ) // Is Audio track?
				szPregap = "    PREGAP 00:02:00"BRK;
			else
				szPregap = "    PREGAP 00:03:00"BRK;
			bResult = TRUE;
			if ( lpToc->TotalSessions > 1 && (i == FirstTrack || lpTrack->SessionNumber != LastSessionNumber) ) {
				Size = wsprintfA(Line, "\r\nREM SESSION %02lu\r\n\r\n", lpTrack->SessionNumber);
				bResult = (WriteFile(hFile, &Line, Size, &gnNumberOfBytes, NULL) && Size == gnNumberOfBytes);
			}
			if ( bResult ) {
				Size = wsprintfA(Line, "FILE \"%s\" %s\r\n  TRACK %02lu %s"BRK, szFileName, szFileType, i, szTrackMode);
				bResult = (WriteFile(hFile, &Line, Size, &gnNumberOfBytes, NULL) && Size == gnNumberOfBytes);
				if ( lpCDTextTrack )
					WriteCDTextTrackFields(hFile, lpCDTextTrack->Title, lpCDTextTrack->Performer, lpCDTextTrack->Songwriter, lpCDTextTrack->Composer, lpCDTextTrack->Arranger, lpCDTextTrack->Message, lpCDTextTrack->ISRC, "    ");
                Size = wsprintfA(Line, "%s    INDEX 01 00:00:00"BRK, szPregap);
                bResult = (WriteFile(hFile, &Line, Size, &gnNumberOfBytes, NULL) && Size == gnNumberOfBytes);
			}
			if ( !bResult ) {
				PrintLastSysError(ERROR_FILE_WRITE);
				break;
			}
			LastSessionNumber = lpTrack->SessionNumber;
			LastTrackMode = TrackMode;
			lpTrack++;
			if ( lpCDTextTrack )
				lpCDTextTrack++;
		}
		CloseHandle(hFile);
	} else
		PrintLastSysError(ERROR_FILE_CREATE);
	return bResult;
}

BOOL __fastcall SetCDDrive(char DriveID, BOOLEAN bAuto, PBOOLEAN bAllAudioCD) {
	char ID, nCDDrives, *pStrPtr, szLine[160];
	CD_DRIVELIST CDDriveList[10], *lpCDDrive;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	CDTEXT_PACK_DATA *lpCDTextPack;
	BOOLEAN bResult, bNewLine;
	TOC_FORMAT_00 tocMSFTOC;
	TOC_INFO *lpTOC_INFO;
	BYTE nTotalTracks;
	long TextPackSize;
// 1) Get a list of the system's valid CD/DVD drives and build Menu
	FastPrint(MSG_SCANNING_BEGINS);
	MemZero(sizeof(CDDriveList), CDDriveList);
	*bAllAudioCD = FALSE;
	nCDDrives =	ASPIGetCDDrives(CDDriveList);
	if ( nCDDrives == 0 ) {
		PrintLastSysError(ERROR_CDS_NOT_FOUND);
		return FALSE;
	}
	bResult = FALSE;
	lpCDDrive = CDDriveList;
	for ( ID = 1; ID <= nCDDrives; ID++, lpCDDrive++, ASPICloseCDDrive() ) { // If at least 1 drive is present, proceed
		bNewLine = TRUE;
		if ( lpCDDrive->bDiscReady ) { // Is drive ready with CD loaded ? (Note Blank CD-Rs return 'ready' too!)
			printf(" %hu) [%c:\\] %s  (/%hu for this drive)"BRK, ID, lpCDDrive->cDriveLetter, lpCDDrive->szVendorProdID, ID);
			if ( !ASPISetCDDrive(lpCDDrive) ) {// Select CD drive
				PrintLastSysError("Error: Can't reopen %c:\\", lpCDDrive->cDriveLetter);
				continue;
			}
			nTotalTracks = ASPIGetTOCMSF(&tocMSFTOC); // Get quick version of TOC for CDDB matching, MSF-only!
			if ( !nTotalTracks ) { // 0 tracks returned ??
			// This can trap a false 'ready' drive with empty/blank CD-R/RW!!
				PrintLastSysError("   ( TOC Error: %s )", ASPIGetLastError("Failure..."));
				lpCDDrive->bDiscReady = FALSE;
				continue;
			} else if ( !tocMSFTOC.TrackData->Reserved1 ) { // Reserved1=user AllAudioTracks Flag - Is Music CD or not?
			// We have at least 1 data track, try looking up TOC in our CDDB
				lpTOC_INFO = SearchTOCDB(&tocMSFTOC, nTotalTracks); // Look up TOC in our CD TOC [games] database!
				if ( lpTOC_INFO ) { // Match found ??
					lpCDDrive->lpReserved1 = (LPVOID)lpTOC_INFO;
					gnNumberOfBytes = wsprintfA(szLine, "   ( CD Title: \"%s%s\" )"BRK, lpTOC_INFO->CDTitle, (lpTOC_INFO->CDType < 2)?JPCODE:USCODE);
					PrintColoredText(szLine, gnNumberOfBytes, TEXT_COLOR_YELLOW);
					bNewLine = FALSE; 
					if ( bAuto ) {
						bResult = TRUE;
						DriveID = ID;
						break;
					}
				}
			} else {
			// It's an all audio/music CD, test if CD-TEXT is present
				lpCDDrive->bAllAudioCD = TRUE;
				lpCDTextPack = ASPIGetCDTEXT(&TextPackSize);
				if ( TextPackSize > 0 ) { // Is CD-TEXT data present ??
				// Find only the Album title text pack if present (normally is 1st, but who knows)
					while ( TextPackSize > 0 && !(lpCDTextPack->PackType == 0x80 && lpCDTextPack->TrackOrPackNumber == 0) ) {
						TextPackSize -= sizeof(CDTEXT_PACK_DATA); //18 bytes
						lpCDTextPack++;
					}
					if ( TextPackSize > 0 && lpCDTextPack->PackType == 0x80 && lpCDTextPack->TrackOrPackNumber == 0 ) {
						mMemZeroBy4_(szLine, SIZE szLine/4);
						pStrPtr = szLine;
						do {
							MemCpy(12, pStrPtr, lpCDTextPack->TextData);
							pStrPtr += 12;
							lpCDTextPack++;
							if ( (pStrPtr - szLine) > 128 )
								break;
						} while ( lpCDTextPack->TrackOrPackNumber == 0 && lpCDTextPack->PackType == 0x80 );
						gnNumberOfBytes = wsprintfA((char*)gAlignedSCSIBuffer, "   ( CD Title: \"%s\" )"BRK, szLine);
						PrintColoredText((char*)gAlignedSCSIBuffer, gnNumberOfBytes, TEXT_COLOR_YELLOW);
						bNewLine = FALSE;
					}
				}
			}
			if ( ID == DriveID ) {
				bResult = TRUE;
				break;
			}
			if ( bNewLine )
				FastPrint(BRK);
		} else
			printf(" %hu) [%c:\\] %s  (No disc, not ready!)\r\n"BRK, ID, lpCDDrive->cDriveLetter, lpCDDrive->szVendorProdID);
	}
	if ( DriveID < 0 ) // Flag to not select drive, only print CD menu
		return FALSE;
	GetConsoleScreenBufferInfo(ghConsoleOutput, &csbi);
// 2) If no CD drive was specified, no auto, and CD drives exist, prompt user!
	while ( !bResult ) {
		if ( DriveID == 0 )
			FastPrint(MSG_NO_CD_SPECIFIED);
		else
			printf(ERROR_CD_NOT_SELECTED, DriveID);
		printf(MSG_SELECT_CDDRIVE, nCDDrives);
		DriveID = (char)ScanForNumber(1, FALSE);
		if ( DriveID > 0 && DriveID <= nCDDrives ) {
			lpCDDrive = &CDDriveList[DriveID-1];
			if ( lpCDDrive->bDiscReady && ASPISetCDDrive(lpCDDrive) ) {
				bResult = TRUE;
				break;
			}
		}
		FillConsoleOutputCharacter(ghConsoleOutput, ' ', csbi.dwSize.X*4, csbi.dwCursorPosition, &gnNumberOfBytes);
		SetConsoleCursorPosition(ghConsoleOutput, csbi.dwCursorPosition);
	}
	if ( bResult ) {
		printf(BRK"Notice: Drive (#%hu) %c:\\ was selected... Continuing..."BRK, DriveID, lpCDDrive->cDriveLetter);
		if ( lpCDDrive->bAllAudioCD )
			*bAllAudioCD = TRUE;
		if ( lpCDDrive->lpReserved1 ) { // Did we get a TOC match ? Save CD title/type info!
			gGameCDMatch = TRUE;
			gSetCD.CDDBID = ((TOC_INFO*)lpCDDrive->lpReserved1)->CDDBID;
			gSetCD.CDType = ((TOC_INFO*)lpCDDrive->lpReserved1)->CDType;
			if ( gSetCD.CDTitle ) // Did we get a custom name from the parameter list ?
				return TRUE; // /name= parameter override, don't use universal names if user requests
			gSetCD.CDTitle = ((TOC_INFO*)lpCDDrive->lpReserved1)->CDTitle;
			SetGameCDTitleInfo();
		}
		return TRUE;
	}
	return FALSE;
}

PSTR* __fastcall GetCmdParamsAtRuntime(PINT lpNumArgs) {
	char szCmdLine[512];
// Init buffer to zero since ReadConsole doesn't null terminate for you
	mMemZeroBy4_(szCmdLine, SIZE szCmdLine/4);
	FastPrint(MSG_NO_PARAMETERS); // Print no parameter message
	FlushConsoleInputBuffer(ghConsoleInput); // Good to keep input buffer flushed
// Read keyboard input from the user to collect parameters now
	ReadFile(ghConsoleInput, szCmdLine, sizeof(szCmdLine)-1, &gnNumberOfBytes, NULL);
	if ( *szCmdLine >= ' ' ) { // Process parameters to argv/argc style if we find any
		FlushConsoleInputBuffer(ghConsoleInput);
		*lpNumArgs = ConvertCmdLineToArgcArgv(szCmdLine);
		if ( *lpNumArgs ) // If nonzero, we have >=1 parameters
			return gszArgv;
	} else
		*lpNumArgs = 0; // Return 0 for number of arguments
	return NULL;
}

void __fastcall InitConsoleIntro() {
	#define BACKCOLOR BACK_COLOR_DARKGRAY
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	COORD pos;
// Set Console's title to what we want!
	SetConsoleTitleA(CONSOLE_TITLE);
	SetWindowTopMost(ghWndConsole);
	HIDE_CONSOLE_CURSOR();
// Clear screen in case TurboRip was run from an existing CMD Console
	cls();
	pos.X = pos.Y = 0;
	GetConsoleScreenBufferInfo(ghConsoleOutput, &csbi);
	FastNPrint(CONSOLE_INTRO, 62);
	for ( char i = 0; i < 2; i++ ) {
		FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_RED, 61, pos, &gnNumberOfBytes);
		Sleep(120);
		FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_GREEN, 61, pos, &gnNumberOfBytes);
		Sleep(120);
		FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_BLUE, 61, pos, &gnNumberOfBytes);
		Sleep(120);
		FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_YELLOW, 61, pos, &gnNumberOfBytes);
		Sleep(120);
	}
//** The PC Engine/TurboGrafx-16 CD-ROM ISO/WAV/CUE Ripper 
	FastPrint(CONSOLE_INTRO); // Print full TurboRip intro and color it
	FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_WHITE|BACKCOLOR, 80, pos, &gnNumberOfBytes);
// Color 'The'
	pos.X = 4;
	FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_RED|BACKCOLOR, 3, pos, &gnNumberOfBytes);
// Color 'Turbo'
	pos.X = 18;
	FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_RED|BACKCOLOR, 5, pos, &gnNumberOfBytes);
// Color 'Grafx'
	pos.X = 23;
	FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_YELLOW|BACKCOLOR, 5, pos, &gnNumberOfBytes);
// Color '16'
	pos.X = 29;
	FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_RED|BACKCOLOR, 2, pos, &gnNumberOfBytes);
// Color 'ISO'
	pos.X = 39;
	FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_YELLOW|BACKCOLOR, 3, pos, &gnNumberOfBytes);
// Color 'WAV'
	pos.X = 43;
	FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_RED|BACKCOLOR, 3, pos, &gnNumberOfBytes);
// Color 'CUE'
	pos.X = 47;
	FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_GREEN|BACKCOLOR, 3, pos, &gnNumberOfBytes);
// Color 'Ripper'
	pos.X = 51;
	FillConsoleOutputAttribute(ghConsoleOutput, TEXT_COLOR_RED|BACKCOLOR, 6, pos, &gnNumberOfBytes);
	csbi.dwCursorPosition.Y = 1;
	FillConsoleOutputAttribute(ghConsoleOutput, csbi.wAttributes, csbi.dwSize.X * 3, csbi.dwCursorPosition, &gnNumberOfBytes);
	RESTORE_CONSOLE_CURSOR();
}

int __cdecl main(int argc, char *argv[]) {
	DWORD nNumberOfBytes, nNumberOfSamples, nNumberOfSamplesEncoded;
	DWORD startLBA, stopLBA, nSectors, dwReadSize, TracksRipped;
	DWORD nTotalCDSectors, nTotalCDSectorsRead, Progress, Count;
	long i, nTotalTracksLeft, nTotalTracks, nSamplesToPerform;
	BYTE SessionNumber, SessionFirstTrack, SessionLastTrack;
	BOOLEAN bIsAudio, bResult, bMP3Mode;
	char szLine[120], szOutputFile[MAX_PATH];
	APE_COMPRESS_HANDLE hAPECompress;
	MASTER_TOC_TRACK_DESCRIPTOR *lpMasterTrack;
	TRACK_DESCRIPTOR_0010 *lpFullTrack;
	TRACK_DESCRIPTOR_00 *lpTrack;
	SCSICDMODEPAGE_01 *lpErrorRecPage;
	SCSICDMODEPAGE_2A *lpMMCPage;
	BYTE MMCPageBuffer[sizeof(SCSICDMODEPAGE_2A)+8];
	PBYTE lpOutBuffer;
	TOC_FORMAT_0010 tocFull;
	TOC_FORMAT_00 toc;
	HBE_STREAM hbeStream;
	PSHORT lpInputBuffer;
	NumericType ntParam, ntParam2;
	UINT nTrueMinSpeed;
	PSTR lpParam;
	HANDLE hFile;
// Initialize flags/variables
	__asm {
		XOR EAX, EAX
		FNINIT
		MOV bMP3Mode, AL
		MOV hAPECompress, EAX
		MOV TracksRipped, EAX
		MOV hbeStream, EAX
		SUB EAX, 1
		MOV hFile, EAX // hFile = -1
	}
	nTrueMinSpeed = DEF_MIN_SPEED;
// We need High priority in Windows XP when dealing with Firewire/USB connected devices.
// Otherwise, taskswitching to other applications while this is running causes read failures
	SetHighPriority();
// Init and Print fun colored title!!
	InitConsoleIntro();
// Console Interface behavior upgrade!!!!! It was about damn time!!!!
// If TurboRip was clicked from File Explorer, prompt for parameters!
// Don't just exit like all console apps do behaving like genuine DOS apps...
	if ( argc < 2 )
		argv = GetCmdParamsAtRuntime(&argc);
// Parameter processing loop
// Process parameters if present or post-entered!
	while ( argc ) {
		argc--; // Decrement paramter count
		lpParam = *argv++; // Get pointer to current paramter, move to the next one
		if ( *lpParam != '/' ) // Invalid parameter format or EXE so just skip to next parameter!
			continue;
		lpParam++;
		ntParam.dwVar = *(LPDWORD)lpParam; // Copy first 4 chars of parameter
		lNc(4, ntParam.szVar); // Lowercase only the first 4 chars due to /name= parameter!
		if ( ntParam.dwVar != 'eman' ) { // 'name'
			Count = SubPtr(DWORD, StrLowerCase(lpParam), lpParam); // It's definitely not the /name parameter, so lowercase all chars!
		} else { // It's the start of the /name parameter!
			lpParam += 4;
			if ( *lpParam == '=' ) // It's the /name= param, so process it (don't lowercase it)!
				gSetCD.CDTitle = ++lpParam; // User title is everything from array[5] and on!
			else
				PrintColoredText(STR_SIZE("Error: Problem with /name parameter usage"BRK), TEXT_COLOR_RED);
			continue;
		}
		if ( Count <= 4 ) {
		// Drive ID set /1 - /9
			if ( Count == 1 && isdigit(ntParam.bVar) ) {
				ParamList.DriveID = ntParam.bVar - '0';
		// Help request via /? or /h or /help
			} else if ( ntParam.bVar == '?' || ntParam.bVar == 'h' || ntParam.dwVar == 'pleh' ) { // /help
				cls();
				FastPrint(HELP_PART1);
				pause(0);
				FastPrint(HELP_PART2);
				MemZero(sizeof(ParamList), &ParamList); // Reset parameters to zero
				argv = GetCmdParamsAtRuntime(&argc); // Prompt user again!
				if ( argc )
					continue;
				break; // argc = 0, no params entered, so break out of loop, use defaults!
			} else if ( ntParam.dwVar == 'otua' ) { //auto
				ParamList.AUTO = TRUE;
			} else if ( ntParam.dwVar == 'cot' ) { //toc
				ParamList.TOC = TRUE;
			} else if ( ntParam.dwVar == 'war' ) { //raw
				ParamList.RAW = TRUE;
			} else if ( ntParam.dwVar == '3pm' ) { //mp3
				ParamList.MP3 = TRUE;
				bMP3Mode = TRUE;
			} else if ( ntParam.dwVar == 'epa' ) { //ape
				ParamList.APE = TRUE;
			} else if ( ntParam.dwVar == 'psp' ) { //psp
				beConfig.format.LHV1.nMode = BE_MP3_MODE_MONO;
				ParamList.PSP = TRUE;
				bMP3Mode = TRUE;
			} else if ( ntParam.dwVar == 'xobx' ) { //xbox
				ParamList.XBOX = TRUE;
				bMP3Mode = TRUE;
			}
			continue;
		}
		lpParam += 4;
		ntParam2.dwVar = *(PDWORD)lpParam; // Copy chars 5-8 of parameter
	// Speed parameters, either /turbo or speed=[00|max]
		if ( ntParam.dwVar == 'brut' && ntParam2.bVar == 'o' ) { //turbo
			ParamList.Speed = 0xFFFF;
	//speed=[00|max]
		} else if ( ntParam.dwVar == 'eeps' && ntParam2.wVar == '=d' ) {
			lpParam += 2;
			if ( *(PDWORD)lpParam == 'xam' ) //max
				ParamList.Speed = 0xFFFF;
			else
				ParamList.Speed = (WORD)atol(lpParam);
	//redump (all-bin format)
		} else if ( ntParam.dwVar == 'uder' && ntParam2.wVar == 'pm' ) { //redump - 'redu' 'mp'
			ParamList.RAW = TRUE;
			ParamList.REDUMP = TRUE;
			*(PDWORD)extISO = EXTBIN; // = ".bin"
			*(PDWORD)extWAV = EXTBIN; // = ".bin"
	//devices
		} else if ( ntParam.dwVar == 'ived' && ntParam2.dwVar == 'sec' ) {
			ParamList.Devices = TRUE;
	//forcerip
		} else if ( ntParam.dwVar == 'crof' && ntParam2.dwVar == 'pire' ) { 
			ParamList.ForceRip = TRUE;
	//useaspi
		} else if ( ntParam.dwVar == 'aesu' && ntParam2.dwVar == 'ips' ) {
			ParamList.USEASPI = TRUE;
	//track=1-99
		} else if ( ntParam.dwVar == 'cart' && ntParam2.wVar == '=k' ) {
			lpParam += 2;
			ParamList.TrackNum = (BYTE)atol(lpParam);
			if ( ParamList.TrackNum > 99 ) {
				PrintColoredText(STR_SIZE("Error: CD tracks only range from 1-99"), TEXT_COLOR_RED);
				pause(0);
				return 0;
			}
	//mbr=32, 48, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320
		} else if ( ntParam.dwVar == '=rbm' ) { // Regular, constant MP3 bitrate
			beConfig.format.LHV1.dwBitrate = atol(lpParam);
			if ( beConfig.format.LHV1.dwBitrate < 8 ) {
				PrintColoredText(STR_SIZE("Error: MP3 bitrate cannot be less than 8 kbps"), TEXT_COLOR_RED);
				pause(0);
				return 0;
			}
			bMP3Mode = TRUE;
	//mmbr=32, 48, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320
		} else if ( ntParam.dwVar == 'rbmm' && ntParam2.bVar == '=' ) { // Max MP3 Bitrate!
			beConfig.format.LHV1.dwMaxBitrate = atol(++lpParam); // This mode enables Variable Bitrate!
			if ( beConfig.format.LHV1.dwMaxBitrate < 16 ) {
				PrintColoredText(STR_SIZE("Error: Max MP3 bitrate shouldn't be less than 16 kbps"), TEXT_COLOR_RED);
				pause(0);
				return 0;
			}
			beConfig.format.LHV1.bWriteVBRHeader = TRUE;
            beConfig.format.LHV1.bEnableVBR = TRUE;
			bMP3Mode = TRUE;
	//mrs=8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
		} else if ( ntParam.dwVar == '=srm' ) {
			beConfig.format.LHV1.dwReSampleRate = atol(lpParam);
			if ( beConfig.format.LHV1.dwReSampleRate < 8000 ) {
				PrintColoredText(STR_SIZE("Error: MP3 resampling rate cannot be less than 8000 Hz"), TEXT_COLOR_RED);
				pause(0);
				return 0;
			}
			bMP3Mode = TRUE;
	//mvbr=0-9 (MP3 variable bit rate)
		} else if ( ntParam.dwVar == 'rbvm' && ntParam2.bVar == '=' ) {
			beConfig.format.LHV1.nVBRQuality = atol(++lpParam);
			if ( (UINT)beConfig.format.LHV1.nVBRQuality > 9 ) {
				PrintColoredText(STR_SIZE("Error: MP3 variable bit rate ranges from 0-9"), TEXT_COLOR_RED);
				pause(0);
				return 0;
			}
			beConfig.format.LHV1.bWriteVBRHeader = TRUE;
			beConfig.format.LHV1.bEnableVBR = TRUE;
			bMP3Mode = TRUE;
	//apelevel=fast, high, extrahigh, insane
		} else if ( ntParam.dwVar == 'lepa' && ntParam2.dwVar == 'leve' ) {
			lpParam += 5;
			ntParam.dwVar = *(LPDWORD)lpParam;
			ParamList.APE = TRUE;
		// apelevel=high
			if ( ntParam.dwVar == 'hgih' )
				APE_COMPRESSION_LEVEL = 3000;
		// apelevel=fast
			else if ( ntParam.dwVar == 'tsaf' )
				APE_COMPRESSION_LEVEL = 1000;
			else {
				lpParam += 4;
				ntParam2.dwVar = *(LPDWORD)lpParam;
			// apelevel=extrahigh
				if ( ntParam.dwVar == 'rtxe' && ntParam2.dwVar == 'giha' )
					APE_COMPRESSION_LEVEL = 4000;
			// apelevel=insane
				else if ( ntParam.dwVar == 'asni' && ntParam2.wVar == 'en' )
					APE_COMPRESSION_LEVEL = 5000;
			}
	// readretry=0-255
		} else if ( ntParam.dwVar == 'daer' && ntParam2.dwVar == 'rter' ) {
			lpParam += 6;
			ParamList.READRETRY = (BYTE)atol(lpParam);
	// normalize norm aliz e
		} else if ( ntParam.dwVar == 'mron' && ntParam2.dwVar == 'zila' ) {
			ParamList.Normalize = TRUE;
		}
	}
// Deal with mutually exclusive parameters
	if ( (ParamList.XBOX && ParamList.PSP) || (ParamList.MP3 && (ParamList.XBOX || ParamList.PSP)) ||
		(ParamList.APE && bMP3Mode) ) {
		FastPrint(ERROR_EXT_PARAMS);
		pause(0);
		return 0;
	}
	if ( !argv )
		FastPrint("Notice: Default settings will be used..."BRK);
// Initialize ASPI Interface if Win9X/ME; default is Native NT SPTI
	if ( !ASPIStart(ParamList.USEASPI) ) {
		pause(0);
		return 0;
	}
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler,  TRUE);
	if ( ParamList.Devices ) {
		ParamList.DriveID = -1;
		ParamList.AUTO = FALSE;
	}
// Present main CD device menu, select working drive, TOC title, data, etc.a
	if ( !SetCDDrive(ParamList.DriveID, ParamList.AUTO, &bIsAudio) )
		goto EXIT_RETURN;
	if ( ParamList.Devices ) // User only wanted a device list printed, so exit
		goto EXIT_RETURN;
// Load the MMC "Device Capabilities and Status" 0x2A page. A failure likely
// indicates we lack a generic SCSI-3 MMC supporting CD/DVD drive, so abort!
	lpMMCPage = (SCSICDMODEPAGE_2A*)ASPIModeSense10(0x2A, MMCPageBuffer, sizeof(MMCPageBuffer));
	if ( !lpMMCPage ) {
		PrintLastSysError("Error: This drive doesn't appear to be MMC/SCSI-3 compliant. Aborting...");
		goto EXIT_RETURN;
	}
// Initialize LAME MP3 Encoding Engine if needed
	if ( bMP3Mode ) {
		if ( !InitLAMEMP3() )
			goto EXIT_RETURN;
// Initialize Monkey's Audio APE Encoding Engine if needed
	} else if ( ParamList.APE ) {
		if ( !InitMACAPE() )
			goto EXIT_RETURN;
	// Create the APE encoder interface
		hAPECompress = g_APECompress_Create((int*)&i);
		if ( !hAPECompress ) {
			PrintLastSysError("Error: Failed to create the APE encoder (%li)", i);
			goto EXIT_RETURN;
		}
	}
	ASPISetCDSpeed(DEF_MIN_SPEED); // Since we'll be analyzing tracks first, we'll wanna go slow
// ************************************************************
// Procure the CD's Table-of-Contents & let's begin the magic!
// ************************************************************
	FastPrint("Notice: Reading the disc's TOC... ");
	MasterTOC.TotalTracks = ASPIGetTOCBasic(&toc, &nNumberOfBytes);
	if ( !MasterTOC.TotalTracks ) {
		PrintLastSysError(BRK"TOC Error: %s", ASPIGetLastError("Failure"));
		goto EXIT_RETURN;
	}
// 1st Phase of populating the MasterTOC object for track data ease of use
	MasterTOC.TotalSessions = 1;
	MasterTOC.FirstTrack = toc.FirstTrack;
	MasterTOC.LastTrack = toc.LastTrack;
// Print Table-of-Contents
	FastPrint("OK...\r\n\n---------Table of Contents--------\r\n\n");
	FastNPrint(gAlignedSCSIBuffer, nNumberOfBytes);
	FastPrint("----------------------------------"BRK);
// Save the TOC to disk always
	if ( !WriteNewFile("disc.toc", gAlignedSCSIBuffer, nNumberOfBytes) || ParamList.TOC )
		goto EXIT_RETURN;
// If it's an all audio/music CD, handle CD-EXTRA or CD-TEXT possibility
	if ( bIsAudio ) {
		if ( LoadSessionsAndCDText(&tocFull, toc.FirstTrack) < 0 )
			goto EXIT_RETURN;
	} else
	// If we have a base title, we found a Game TOC match OR /name= forced override
		if ( gGameCDMatch ) { // TOC Match found ?
			printf(MSG_TOC_MATCH, gSetCD.CDTitle, CDTYPE[gSetCD.CDType]);
		// Block ripping of homebrew games
			//if ( HomeBrewBlock() )
			//	goto EXIT_RETURN;
		} else if ( !gSetCD.CDTitle ) // If no forced /name= usage
			FastPrint(MSG_TOC_NO_MATCH);
// No base filename was matched nor passed as parameter; this is an unknown disc (audio, etc.)
	if ( !gSetCD.CDTitle ) {
		gSetCD.CDTitle = gTrackTitle;
		GetDiscTitle(gTrackTitle); // Get title from user now
	}
	HIDE_CONSOLE_CURSOR();
// Make a folder for all the file output
	if ( !SetCurrentDirectoryA(gSetCD.CDTitle) )
		if ( !CreateDirectoryA(gSetCD.CDTitle, NULL) ) {
			PrintLastSysError("Error: Unable to create CD image folder");
			goto EXIT_RETURN;
		} else
			SetCurrentDirectoryA(gSetCD.CDTitle);
	wsprintfA(szOutputFile, "%s.toc", gSetCD.CDTitle);
	printf("Wrote TOC File: \"%s\"\r\n\n", szOutputFile);
	lpParam = "..\\disc.toc";
	if ( !CopyFileA(lpParam, szOutputFile, FALSE) )
		PrintLastSysError("Error");
	DeleteFileA(lpParam);
// Perform Track Mode Analysis
	bGlobalLoopAlert = TRUE;
	FastPrint("Notice: Performing first track mode analysis... One moment..."BRK);
	if ( !ASPIAnalyzeTrackModes(&toc, ParamList.RAW) ) {
		if ( bGlobalAbort )
			goto EXIT_RETURN;
		PrintColoredText(STR_SIZE("Fatal Error: This analysis phase has failed. Aborting..."BRK), TEXT_COLOR_RED);
		goto EXIT_RETURN;
	}
	if ( bGlobalAbort )
		goto EXIT_RETURN;
// Set proper audio extension (default: wave)
	if ( bMP3Mode )
		lpAudioExt = extMP3;
	else if ( ParamList.APE )
		lpAudioExt = extAPE;
// ***** Finish populating the MasterTOC object *****
	lpTrack = toc.TrackData;
	lpFullTrack = tocFull.TrackData;
	nTotalTracksLeft = MasterTOC.TotalTracks;
	SessionNumber = lpFullTrack->SessionNumber;
// NEC Game CDs should all be MODE1 sectors, validate for CD-Rs burns!
	if ( gGameCDMatch ) { // Flag a PCE/TG16/PCFX CD, check for non-MODE1 data sector, error out? (Bonknuts idea)
		for ( BYTE nTracks = MasterTOC.TotalTracks; nTracks; lpTrack++, nTracks-- )
			if ( lpTrack->Reserved1 == MODE2_2352 ) {// Trap MODE2 improper burns caused by Nero bug!
				PrintColoredText(STR_SIZE(ERROR_BAD_DATAMODE), TEXT_COLOR_RED);
				goto EXIT_RETURN;
			}
		lpTrack = toc.TrackData;
	}
	if ( SessionNumber == 0 )
		SessionNumber = 1;
	while ( lpFullTrack->POINT != 0xA0 && nTotalTracksLeft > 0 ) { // Find first track of session
		lpFullTrack++;
		nTotalTracksLeft--;
	}
	SessionFirstTrack = lpFullTrack->PMIN;
	while ( lpFullTrack->POINT != 0xA1 && nTotalTracksLeft > 0 ) { // Find last track of session 
		lpFullTrack++;
        nTotalTracksLeft--;
	}
	SessionLastTrack = lpFullTrack->PMIN;
	MASTER_TOC_CDTEXT_TRACKINFO *lpCDTEXTTrack;
	lpCDTEXTTrack = ( MasterTOC.CDTextData.bHasCDText ) ? MasterTOC.CDTextData.TrackData : NULL;
	nTotalCDSectors = 0;
	for ( lpMasterTrack = MasterTOC.TrackData; lpTrack->TrackNumber <= MasterTOC.LastTrack; lpTrack++, lpMasterTrack++ ) {
		if ( bGlobalAbort )
			goto EXIT_RETURN;
		lpMasterTrack->TrackNumber = lpTrack->TrackNumber;
		lpMasterTrack->TrackMode = lpTrack->Reserved1;
		bIsAudio = (lpMasterTrack->TrackMode == AUDIO_CDDA);
		lpMasterTrack->StartAddress = *(LPDWORD)lpTrack->Address;
        lpMasterTrack->StopAddress = *(LPDWORD)(lpTrack+1)->Address;
		lpMasterTrack->SessionNumber = SessionNumber;
		if ( ParamList.PSP ) {
			wsprintfA(lpMasterTrack->TrackFileName, "%02d%s", lpMasterTrack->TrackNumber, bIsAudio ? lpAudioExt : extISO);
		} else {
			if ( lpCDTEXTTrack && *lpCDTEXTTrack->Title )
				lpParam = lpCDTEXTTrack->Title;
			else
				lpParam = gSetCD.CDTitle;
			wsprintfA(lpMasterTrack->TrackFileName, "%02d %s%s", lpMasterTrack->TrackNumber, lpParam, bIsAudio ? lpAudioExt : extISO);
		}
		WINDOWSify(lpMasterTrack->TrackFileName);
// TEMPORARY HARDCODED GAP RULES!!! REMOVE WHEN Q-SUBCODE STUFF WORKS!
	// Detect and enforce a track type transition condition to deal with PREGAPs!
		if ( lpMasterTrack->TrackNumber < toc.LastTrack && (lpMasterTrack->TrackMode > 0 != (lpTrack+1)->Reserved1 > 0) ) {
		// The next track is of type data, so a transition from audio to data will
		// take place, so _this_ audio track's stop LBA = LBA minus 03:00 (225 sectors).
			if ( bIsAudio )
				lpMasterTrack->StopAddress -= 225;
		// The next track is of type audio, so a transition from data to audio will
		// take place, so _this_ data track's stop LBA = LBA minus 02:00 (150 sectors).
			else
				lpMasterTrack->StopAddress -= 150;
		}
// END TEMPORARY HARDCODED GAP RULES
		if ( MasterTOC.TotalSessions > 1 ) {
			if ( lpTrack->TrackNumber == SessionLastTrack ) {
				while ( lpFullTrack->POINT != 0xA2 && nTotalTracksLeft > 0 ) { // Find Lead-out start
					lpFullTrack++;
					nTotalTracksLeft--;
				}
				if ( nTotalTracksLeft > 0 )
					lpMasterTrack->StopAddress = (((lpFullTrack->PMIN * 60) + lpFullTrack->PSEC) * 75 + lpFullTrack->PFRAME) - 150;
				while ( lpFullTrack->POINT != 0xA0 && nTotalTracksLeft > 0 ) { // Find first track of next session
					lpFullTrack++;
					nTotalTracksLeft--;
				}
				if ( nTotalTracksLeft > 0 )
					SessionFirstTrack = lpFullTrack->PMIN;
				while ( lpFullTrack->POINT != 0xA1 && nTotalTracksLeft > 0 ) { // Find last track of next session
					lpFullTrack++;
					nTotalTracksLeft--;
				}
				if ( nTotalTracksLeft > 0 ) {
					SessionLastTrack = lpFullTrack->PMIN;
					SessionNumber = lpFullTrack->SessionNumber;
				}
			}
		}
		lpMasterTrack->TotalSectors = lpMasterTrack->StopAddress - lpMasterTrack->StartAddress;
		nTotalCDSectors += lpMasterTrack->TotalSectors; // For total progress, add all CD sectors that will be read
		lpMasterTrack->StopAddress--; // The true stop LBA is one less than the total sectors the track occupies ( 0 to n-1 )
		lpMasterTrack->TotalBytes = lpMasterTrack->TotalSectors * (( lpMasterTrack->TrackMode == MODE1_2048 ) ? MODE1_SECTOR_SIZE : RAW_SECTOR_SIZE);
		FormatNumber(lpMasterTrack->TotalBytes, lpMasterTrack->szTotalBytes);
		FormatNumber(lpMasterTrack->TotalSectors, lpMasterTrack->szTotalSectors);
		if ( lpCDTEXTTrack ) lpCDTEXTTrack++;
	}
	bGlobalLoopAlert = FALSE;

// Print out MMC Page: CD/DVD Drive's features/specs/settings
	ASPIPrintMMCFeaturesPage(lpMMCPage);
	if ( !ParamList.Speed ) // Set our default speed if none was chosen
		ParamList.Speed = DEF_MIN_SPEED;
// Begin Drive Read Speed Detection and Selection
	nTrueMinSpeed = SetDetectCDDriveSpeed();
// If drive speed control fails, this won't work, we're dealing with emulation drive/windows or old Windows, so skip it!
	if ( nTrueMinSpeed ) {
	// Set the Read Retry Count to at least 5
		mMemZeroBy4(gAlignedSCSIBuffer, CPAGE_01_SIZE/4);
		lpErrorRecPage = (SCSICDMODEPAGE_01*)ASPIModeSense10(0x01, gAlignedSCSIBuffer, CPAGE_01_SIZE);
		if ( lpErrorRecPage && lpErrorRecPage->p_len > 0 ) {
			if ( ParamList.READRETRY == 0 && lpErrorRecPage->ReadRetryCount < 9 || ParamList.READRETRY > 0 && ParamList.READRETRY < 5 )
				ParamList.READRETRY = 5;
			if ( ParamList.READRETRY ) {
				lpErrorRecPage->ReadRetryCount = ParamList.READRETRY;
				if ( ASPIModeSelect10(gAlignedSCSIBuffer, CPAGE_01_SIZE) == SS_COMP ) {
					mMemZeroBy4(gAlignedSCSIBuffer, CPAGE_01_SIZE/4);
					lpErrorRecPage = (SCSICDMODEPAGE_01*)ASPIModeSense10(0x01, gAlignedSCSIBuffer, CPAGE_01_SIZE);
				}
			}
			if ( lpErrorRecPage )
				printf("Read Retry Count [DATA]  : %hu"BRK, lpErrorRecPage->ReadRetryCount);
		}
	}
	FastPrint(NEW_LINE);
// Warn or abort with regard to CD-DA Command/StreamIsAccurate Support
	if ( !lpMMCPage->cd_da_accurate )
		if ( ParamList.ForceRip ) {
			PrintColoredText(STR_SIZE(MSG_NO_CDDA_STREAM_FORCERIP), TEXT_COLOR_RED);
		} else {
			PrintColoredText(STR_SIZE(MSG_NO_CDDA_STREAM_WARNING), TEXT_COLOR_RED);
			goto EXIT_RETURN;
		}
// FOR FUTURE Q SUBCODE ANALYSIS!!!
// Analyze Q subcodes to deal with pesky indexes
	//if ( nTrueMinSpeed )
	//ASPISetCDSpeed(nTrueMinSpeed);
	//if ( !ASPIAnalyzeQSubCodes(&MasterTOC) )
	//	goto EXIT_RETURN;

	if ( nTrueMinSpeed )
		ASPISetCDSpeed(ParamList.Speed);
// Write a CUE file if we're not in MP3 rip mode
	if ( !bMP3Mode ) {
		if ( !WriteCUEFile(gSetCD.CDTitle, &MasterTOC) )
			goto EXIT_RETURN;
	} else if ( ParamList.XBOX ) // Write the HUGO-X CUE file if we're in XBOX mode
		WriteHUGOXCUEFile(gSetCD.CDTitle, &toc);
// If APE mode, extract decode EXE+BAT in working folder
	if ( ParamList.APE && ExtractMACD_EXE() )
		FastPrint("Notice: APE decoding support files extracted"BRK);
	lpMasterTrack = MasterTOC.TrackData;
// Handle single track request for extraction
	if ( ParamList.TrackNum ) {
		nTotalTracksLeft = nTotalTracks = 1;
		Count = MasterTOC.TotalTracks;
		i = ParamList.TrackNum;
		printf("Notice: Track #%lu selected for ripping"BRK, i);
		bResult = ( (DWORD)i > Count || i == 0 );
		if ( !bResult )
			while ( (bResult = lpMasterTrack->TrackNumber != i) && Count-- > 0 )
				lpMasterTrack++;
		if ( bResult ) {
			PrintLastSysError("Error: Track #%02lu does not exist", i);
			goto EXIT_RETURN;
		}
		nTotalCDSectors = lpMasterTrack->TotalSectors;
	} else
		nTotalTracksLeft = nTotalTracks = MasterTOC.TotalTracks;
	FastPrint(NEW_LINE);
// **********************************************************************************************
//			Finally, begin main loop for every track to extract!
// **********************************************************************************************
	nTotalCDSectorsRead = 0;
	gTickCountBegin = GetTickCount();
	for ( ASPIResetUnit(); nTotalTracksLeft > 0; nTotalTracksLeft--, lpMasterTrack++ ) {
		if ( bGlobalAbort || bGlobalLoopAlert == 6 ) // If abort was requested
			goto EXIT_RETURN; 
		bGlobalLoopAlert = TRUE;
		bIsAudio = (lpMasterTrack->TrackMode == AUDIO_CDDA);
		if ( ParamList.MP3 && !bIsAudio ) { // Skip data tracks for MP3 audio disc extraction mode
			printf("Skipping : Data Track %02hu (%s Bytes)", lpMasterTrack->TrackNumber, lpMasterTrack->szTotalBytes);
			goto SKIP_DATA_TRACK;
		}
		nSectors = lpMasterTrack->TotalSectors;
		startLBA = lpMasterTrack->StartAddress;
		stopLBA = lpMasterTrack->StopAddress;
	// Open our track file for writing, unless APE mode which encoder will do for us
		if ( !(bIsAudio && ParamList.APE) ) {
			hFile = _CreateNewFile(lpMasterTrack->TrackFileName);
			if ( hFile == INVALID_HANDLE_VALUE ) {
				PrintLastSysError("Error: Cannot create file ('%s') for writing", lpMasterTrack->TrackFileName);
				goto EXIT_RETURN;
			}
		} else // APE = True - encoder will do it for us
			hFile = INVALID_HANDLE_VALUE;
	// Initialization for audio track
		if ( bIsAudio ) {
		// Initialize MP3 output mode
			if ( bMP3Mode ) {
				if ( beInitStream(&beConfig, &nNumberOfSamples, &gnNumberOfBytes, &hbeStream) != BE_ERR_SUCCESSFUL ) {
					PrintLastSysError("Error: Failed to open MP3 encoding stream\r\nError: Check your encoding parameters");
					goto EXIT_RETURN;
				}
				if ( beConfig.format.LHV1.nMode == BE_MP3_MODE_MONO ) // Deal with Blade stereo-mono downmix bug
					lame_set_num_channels(hbeStream, 2);
		// Initialize APE output mode
			} else if ( ParamList.APE ) {
				i = g_APECompress_Start(
					hAPECompress, lpMasterTrack->TrackFileName, &wfeAudioFormat, lpMasterTrack->TotalBytes,
					APE_COMPRESSION_LEVEL, NULL, CREATE_WAV_HEADER_ON_DECOMPRESSION
				);
				if ( i != 0 ) {
					PrintLastSysError("Error: Failed to open APE encoding stream (%lu)", i);
					goto EXIT_RETURN;
				}
		// Initialize WAVE output mode
			} else if ( !ParamList.REDUMP ) {
				WaveHeader.riffSIZE = lpMasterTrack->TotalBytes + (44 - 8);
				WaveHeader.dataSIZE = lpMasterTrack->TotalBytes;
				if ( !(WriteFile(hFile, &WaveHeader, 44, &gnNumberOfBytes, NULL) && gnNumberOfBytes == 44) ) {
					PrintLastSysError(ERROR_FILE_WRITE);
					goto EXIT_RETURN;
				}
			}
			lpParam = "Audio";
			if ( nTrueMinSpeed )
				ASPISetCDSpeed(ParamList.Speed);
		// Initialization for data track
		} else {
			if ( lpMasterTrack->TrackMode == MODE1_2048 ) {
				SECTOR_SIZE = MODE1_SECTOR_SIZE;
				if ( nTrueMinSpeed )
					ASPISetCDSpeed(0xFFFF); // max speed for cooked/EDC data reads
			} else
				SECTOR_SIZE = RAW_SECTOR_SIZE;
			lpParam = "Data";
		}
		printf(
			"Ripping  : %s Track %02hu (%s Bytes)"BRK
			"LBA Range: %06lu to %06lu (%s Sectors)"BRK
			"File Name: \"%s\""BRK,
			lpParam, lpMasterTrack->TrackNumber, lpMasterTrack->szTotalBytes,
			startLBA, stopLBA, lpMasterTrack->szTotalSectors, lpMasterTrack->TrackFileName
		);
		bGlobalLoopAlert = 5; // Set this flag # to allow "finish track before quitting" option
	//
	// Begin ripping the current track for all its sectors
	//
		gTickCountBegin = GetTickCount();
		for ( dwReadSize = SECTOR_READ_CHUNKS; nSectors; ) {
			if ( bGlobalLoopAlert == 6 )
				nTotalTracksLeft = 0; // Abort was requested AFTER the current track is fully extracted
			if ( nSectors < SECTOR_READ_CHUNKS )
				dwReadSize = nSectors;
		// Read Audio Sectors
			if ( bIsAudio ) {
				nNumberOfBytes = dwReadSize * RAW_SECTOR_SIZE;
				bResult = ASPIReadCDDASectors(startLBA, dwReadSize);
		// Read Data Sectors
			} else {
				nNumberOfBytes = dwReadSize * SECTOR_SIZE;
				if ( SECTOR_SIZE == MODE1_SECTOR_SIZE )
					bResult = ASPIReadDATASectors(startLBA, dwReadSize);
				else
					bResult = ASPIReadRAWSectors(startLBA, dwReadSize);
			}
		// Trap and handle read errors!
			if ( !bResult ) {
				Count = wsprintfA(szLine, BRK"Read Error: %s"BRK, ASPIGetLastError("Failure"));
				PrintColoredText(szLine, Count, TEXT_COLOR_RED);
				ASPILoadTrayReadyUnit();
			// Trap and ignore data read errors near end of track due to nearness of unreadable gap sectors
				if ( nSectors <= SECTOR_READ_CHUNKS+14 && !bIsAudio ) {
				// SCSI Read Error Code depends on drive model, I've seen (3) so far
				// (1) "Illegal mode for this track"
				// (2) "Layered-Error Correction uncorrectable error"
				// (3) "Unrecovered read error"
				// Strategy: Read final sectors 1-sector-at-a-time for these [pre]gap/leadout transition errors!
					PrintColoredText(STR_SIZE("Entering Read Error Recovery Mode for gap/lead-out area (expect slowdown)..."BRK), TEXT_COLOR_YELLOW);
					for ( i = Count = 0; nSectors; nSectors--, startLBA++ ) {
						if ( SECTOR_SIZE == MODE1_SECTOR_SIZE )
							bResult = ASPIReadDATASectors(startLBA, 1);
						else
							bResult = ASPIReadRAWSectors(startLBA, 1);
						if ( bResult ) {
							lpOutBuffer = gAlignedSCSIBuffer;
							Count++; // Count good sector read
						} else {
							lpOutBuffer = gRawNullSector;
							i++; // Count bad unreadable sector skilled/nulled
						}
						if ( !_WriteVerify(hFile, lpOutBuffer, SECTOR_SIZE) ) {
							PrintLastSysError(ERROR_FILE_WRITE);
							goto EXIT_RETURN;
						}
						printf(
							"Progress : LBA: %06lu/%06lu (%lu), Sectors Recovered / Failed: %lu / %lu      \r",
							startLBA, stopLBA, (stopLBA-startLBA), Count, i 
						); 
						if ( bGlobalAbort )
							break;
					}
					dwReadSize = nSectors = 1; // We finished reading all sectors, mark it to exit FOR loop!!
					startLBA = stopLBA;
					CloseHandle(hFile); // Close out track file now, we're done!
					hFile = INVALID_HANDLE_VALUE;
					if ( i > 0 ) {
						Count = wsprintfA(szLine, BRK"Note: Nulled/Zeroed %lu unreadable data sector(s) near gap/lead-out area"BRK, i);
						PrintColoredText(szLine, Count, TEXT_COLOR_YELLOW);
					} else
						PrintColoredText(STR_SIZE(BRK"Note: Recovered all data sectors(s) near gap/lead-out area!"BRK), TEXT_COLOR_YELLOW);
				} else {
				// OK, GIVE UP/RETRY/IGNORE... :(
					RESTORE_CONSOLE_CURSOR();
					char c = AbortRetryIgnorePrompt();
					if ( c == 'a' ) {
						if ( YesNoPrompt(STR_SIZE("Delete unfinished file[Y,N]?")) == 'y' ) {
							if ( hFile != INVALID_HANDLE_VALUE ) {
								CloseHandle(hFile);
								hFile = INVALID_HANDLE_VALUE;
							} else if ( hAPECompress )
								g_APECompress_Destroy(hAPECompress);
							DeleteFileA(lpMasterTrack->TrackFileName);
						}
						goto EXIT_RETURN;
					} else if ( c == 'r' ) {
						continue;
					} else { //( c == 'i' )
						MemZero(nNumberOfBytes, gAlignedSCSIBuffer);
						PrintColoredText(STR_SIZE(BRK"Note: Nulled/Zeroed sectors and ignored read errors"BRK), TEXT_COLOR_YELLOW);
					}
				//* Testing Error trapping area - Chris C. Road Spirit failure
				//	Progress 100% LBA: 013229 to 013255 (data track 2)
				//	Read Error: Unknown read failure after track 2

				//	Track 01	Audio	00:02:00	LBA=000000
				//	Track 02	Data	00:49:65	LBA=003590
				//	013229 + 27 = 013255
				//	013256 + 23 = 013279 
				//	True Stop LBA = 13279
				//	...pregap/index 00...
				//	Track 03	Audio	03:01:05	LBA=013430 
				// Temp testing session on Sub Q
				//	SUBCHANNEL_SUBQ_DESCRIPTOR *lpSubData;
				//	lpSubData = (SUBCHANNEL_SUBQ_DESCRIPTOR*)gAlignedSCSIBuffer;
				//	ASPIReadQSubChannelData(i, 1);

				// ** Fighting Street (U) transition layout **
				// Track 1, Index 01 = 0000 to 3364 (Actual Audio)
				// Track 2, Index 00 = 3365 to 3444 (Marked as Audio)
				// Track 2, Index 00 = 3445 to 3594 (Marked as Data)
				// Track 2, Index 01 = 3595 to 11330 (Actual Data)
				// Track 3, Index 00 = 11331 to     (Marked as Audio)
					// errors at 11336
					// 11337 continues
				// Track 3, Index 01 = 11481 (Actual Audio)

					//for ( i = startLBA; (DWORD)i <= stopLBA; i++ ) {
						//if ( bIsAudio ) // 3439 - 3440
							//bResult = ASPIReadCDDASectors(i, 1); // TEMP Delete!!1
						// When 2 sectors of different type are read, then an error is thrown;
						// Not always true if reading 1 sector at a time! Gotta be 2!
						//	 bResult = ASPIReadCDDASectors(i, 2); 
						//ASPIReadQSubChannelData(i, 1);
						//if ( !bResult )
						//	break;
						//bResult = ASPIReadDATASectors(i, 1);
					//}
					//ASPIReadQSubChannelData(i, 1);
				}
				ASPIResetUnit(); // Read Errors switch drive to low speed, this should restore to full
			}
			lpOutBuffer = gAlignedSCSIBuffer;
		// Encode Audio sectors with codec if requested
			if ( bIsAudio ) {
			// Encode with MP3 (MPEG Layer III)
				if ( bMP3Mode ) {
					nSamplesToPerform = nNumberOfBytes >> 1;
					lpInputBuffer = (PSHORT)gAlignedSCSIBuffer;
					lpOutBuffer = gEncodeBuffer;
					Count = nNumberOfSamples;
					nNumberOfBytes = 0;
					do {
						if ( beEncodeChunk(hbeStream, Count, lpInputBuffer, lpOutBuffer, &nNumberOfSamplesEncoded) != BE_ERR_SUCCESSFUL ) {
							PrintLastSysError(BRK"Error: MP3 encoding failure");
							beCloseStream(hbeStream);
							goto EXIT_RETURN;
						}
						nSamplesToPerform -= Count;
						lpInputBuffer += Count;
						lpOutBuffer += nNumberOfSamplesEncoded;
						nNumberOfBytes += nNumberOfSamplesEncoded;
						if ( nSamplesToPerform < (LONG)Count )
							Count = nSamplesToPerform;
					} while ( nSamplesToPerform > 0 ); // Encode samples
					lpOutBuffer = gEncodeBuffer;
			// Encode with APE (Monkey's Audio)
				} else if ( ParamList.APE ) {
					i = g_APECompress_AddData(hAPECompress, gAlignedSCSIBuffer, nNumberOfBytes);
					if ( i != ERROR_SUCCESS ) {
						PrintLastSysError(BRK"Error: APE encoding failure (%li)", i);
						goto EXIT_RETURN;
					}
				}
			}
			if ( hFile != INVALID_HANDLE_VALUE )
				if ( !(WriteFile(hFile, lpOutBuffer, nNumberOfBytes, &Count, NULL) && nNumberOfBytes == Count) ) {
					PrintLastSysError("\r\n"ERROR_FILE_WRITE);
					goto EXIT_RETURN;
				}
			gTickCountEnd = GetTickCount() - gTickCountBegin;
			if ( bGlobalAbort )
				goto EXIT_RETURN;
			nSectors -= dwReadSize;
			Progress = 100 - ((nSectors*100) / lpMasterTrack->TotalSectors);
			nTimeSecs  = gTickCountEnd / 1000; // seconds
			nTimeMSecs = (gTickCountEnd % 1000) / 10; // .XX milliseconds
		// Optimize progress reporting line since this loop occurs the most
			lpParam = &PERCENT_PROGRESS[11]; // Move to '100%' for % output
			lpParam += 8 + LongToStr(Progress, lpParam, 3, ' '); // Convert number to ANSI text
			PERCENT_PROGRESS[14] = '%';
			lpParam += 4 + LongToStr(startLBA, lpParam, 6, '0'); // Convert number to ANSI text
			lpParam += 3 + LongToStr((startLBA+dwReadSize-1), lpParam, 6, '0');
			lpParam += LongToStr(nTimeSecs, lpParam, 2, '0');
			*lpParam++ = '.';
			lpParam += LongToStr(nTimeMSecs, lpParam, 2, '0');
			mStrNCpy32(lpParam, OFFSET SECONDS_ELAPSED, (SIZE SECONDS_ELAPSED)-1);
			FastPrintC(PERCENT_PROGRESS);
			startLBA += dwReadSize;
		}
	// Finish off encoded audio file as necessary
		if ( bIsAudio )
			if ( bMP3Mode ) {
			// Deinit the stream
				if ( beDeinitStream(hbeStream, lpOutBuffer, &nNumberOfBytes) != BE_ERR_SUCCESSFUL ) {
					PrintLastSysError(BRK"Error: MP3 encoding finalization failed");
					beCloseStream(hbeStream);
					goto EXIT_RETURN;
				}
				beCloseStream(hbeStream); // Close the MP3 Stream
			// Are there any bytes returned from the DeInit call? If so, write them to disk
				if ( nNumberOfBytes )
					if ( !(WriteFile(hFile, lpOutBuffer, nNumberOfBytes, &Count, NULL) && nNumberOfBytes == Count) ) {
						PrintLastSysError(BRK"Error: MP3 write operation failed");
						goto EXIT_RETURN;
					}
				if ( beConfig.format.LHV1.bWriteVBRHeader ) {
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;
					beWriteVBRHeader(lpMasterTrack->TrackFileName); // Write VBR header
				}
			} else if ( ParamList.APE ) {
				i = g_APECompress_Finish(hAPECompress, NULL, 0, 0);
				if ( i != ERROR_SUCCESS ) {
					PrintLastSysError(BRK"Error: APE encoding finalization failed (%li)", i);
					goto EXIT_RETURN;
				}
			}
		if ( hFile != INVALID_HANDLE_VALUE ) {
			CloseHandle(hFile);
			hFile = INVALID_HANDLE_VALUE;
			if ( bIsAudio && ParamList.Normalize ) {// Normalize wave file if requested
				FastPrint(BRK"Notice   : Normalizing/Amplifying...");
				WAVE_Normalize(lpMasterTrack->TrackFileName);
			}
		}
SKIP_DATA_TRACK:
		gTickTotal += GetTickCount() - gTickCountBegin;
		nTimeSecs  = gTickTotal / 1000;	// seconds
		nTimeMSecs = (gTickTotal % 1000) / 10;	// .XX milliseconds
		nTotalCDSectorsRead += lpMasterTrack->TotalSectors;
		TracksRipped++;
		Count = wsprintfA(
			szLine,
			BRK"Overall %%: %3lu%% (%lu/%lu tracks), %lu:%02lu.%02lu elapsed\r\n\n",
			(nTotalCDSectorsRead*100)/nTotalCDSectors, TracksRipped, nTotalTracks, nTimeSecs / 60, nTimeSecs % 60, nTimeMSecs
		);
	// Print total progress report for tracks so far
		PrintColoredText(szLine, Count, TEXT_COLOR_YELLOW);
		Sleep(0); // Relinquish the remainder of time slice to other threads
	}
// If we're completely finished, spam website via update linkage
	#ifndef WWW_SPAM_OFF
		if ( nTotalTracksLeft == 0 ) {
			FILETIMES ftFileTimes;
			SYSTEMTIME st, ft;
			GetModuleFileNameA(NULL, szOutputFile, MAX_PATH);
			GetFileTimes(szOutputFile, &ftFileTimes);
			FileTimeToSystemTime(&ftFileTimes.ftLastWriteTime, &ft);
			GetSystemTime(&st);
		// If systime is greater than a day/month/year versus our exe, THEN spam; 24-hour minimum
			if ( st.wYear > ft.wYear || st.wMonth > ft.wMonth || st.wDay > ft.wDay ) {
				ShellExecuteA(NULL, "open", WWWURL, NULL, NULL, SW_SHOWNOACTIVATE);
				Sleep(1300);
				SetWindowTopMost(ghWndConsole);
				SetFileWriteTimeToCurrentTime(szOutputFile);
			}
		}
	#endif
EXIT_RETURN:
// Cleanup for exiting
	RESTORE_CONSOLE_CURSOR();
	bGlobalAbort = TRUE;
	if ( hFile != INVALID_HANDLE_VALUE )
		CloseHandle(hFile);
	ASPIStop();
	if ( bMP3Mode )
		FreeLAMEMP3();
	if ( hAPECompress ) {
		g_APECompress_Destroy(hAPECompress);
		FreeMACAPE();
	}
	pause(0);
	return 0;
}
