#include "ASPILib.h"
#include <ntspti.h>
#include <RegistryASM.h>

pfnGetASPI32SupportInfo GetASPI32SupportInfo;
pfnSendASPI32Command    SendASPI32Command;

// Catch a struct alignment missizing error!!! e.g. TOC_FORMAT_00 MUST equal 804 bytes!!!!
#ifdef _DEBUG
	STRUCT_PROTECT(TOC_FORMAT_00, 804, TOC_00);
	STRUCT_PROTECT(CDTEXT_PACK_DATA, 18, CDTEXT);
#endif

__declspec(align(4)) BYTE gSCSIBuffer[SCSI_BUFFER_SIZE];
__declspec(align(4)) BYTE gRawNullSector[RAW_SECTOR_SIZE];
// So we can align SCSI buffer address according to drive's align mask
LPBYTE gAlignedSCSIBuffer = gSCSIBuffer;
// 128-bit GUID to trap SCSI read failures when nothing is written to the SCSI buffer, the
// read opcode fails, yet status/flags return true/success preventing error detection...
DWORD gGUID[] = { 0xACE8DEFA, 0x3545F2FE, 0xAF1F158D, 0xADDEB9E5 };

SPTI_EXEC_SCSI_CMD gSPTI_Exec = {{sizeof(SCSI_PASS_THROUGH_DIRECT),0,0,0,0,0,0,0,0,DEF_TIMEOUT,0,sizeof(SCSI_PASS_THROUGH_DIRECT),0}, 0};
SRB_ExecSCSICmd    gASPI_Exec;
SRB_HAInquiry      gSRBHAInquiry;
PBYTE  gASPI_CDBByte, gASPI_CDBLen;

HANDLE ghCDRom, ghEventSRB;

DWORD       gLAST_IOCTL_ERROR;
char        gszLAST_ERROR[MAX_PATH];
BYTE        gASPIStatus, gSenseKey, gAddSenseCode, gAddSenQual;
BYTE        gNumAdapters = MAX_NUM_HA;
BOOLEAN     bUseSPTI;
BOOLEAN     bGlobalAbort;
HINSTANCE   ghinstWNASPI32;

HANDLE __fastcall SPTI_GetCDDriveHandle(char);
BOOL   __fastcall ASPI_ExecSCSICmd1();
BOOL   __fastcall SPTI_ExecSCSICmd1();
BOOL   __fastcall ASPI_ExecSCSICmd2(PBYTE lpBuffer, DWORD BufferSize);
BOOL   __fastcall SPTI_ExecSCSICmd2(PBYTE lpBuffer, DWORD BufferSize);

BOOL  (__fastcall *ExecuteSCSICmd1)() = &SPTI_ExecSCSICmd1;
BOOL  (__fastcall *ExecuteSCSICmd2)(PBYTE lpBuffer, DWORD BufferSize) = &SPTI_ExecSCSICmd2;

// How to represent each track in the CUE file given the extraction
// method? Well, that's what this table's for. I found that CDRWIN
// uses "CDI/" v. "MODE2/" to distinguish their MODE==2 condition. Also,
// I think they made a mistake: they have "CDI/2336" hardcoded near the
// other values and the documenation that's out there shows a user data
// size of 2336 is of a YellowBook MODE2 (Form0/Formless) Sector... I
// wonder if they know something others don't, oh well...

PSTR CDTRACK_MODES[] = {
// Redbook audio sector
	"AUDIO", // Raw
// Standard Yellow Book sectors
	"MODE1/2048", // Mode1 Form1
	"MODE1/2352", // Mode1 Raw
	"MODE2/2336", // Mode2 Form0
// XA (Extended) Architecture Sectors
	"MODE2/2048", // Mode2 Form1
	"MODE2/2324", // Mode2 Form2
// Yellow/XA
	"MODE2/2352", // Mode2 Raw

// RESEARCH CDI SECTORS, UNUSED NOW- WATCH INDEXES -BUG FIXED OF CDI USAGE
// Green Book CDi Sectors
	"CDI/2328", // CDI Mode2
	"CDI/2352"  // CDI Raw
};
/*
 * Genre categories from Enhanced CD Specification page 21
*/
PSTR GENRE_TYPES[] = {
	"",                          // 0 GENRE_UNUSED
	"Not defined",               // 1 GENRE_UNDEFINED
	"Adult Contemporary",        // 2
	"Alternative Rock",          // 3
	"Childrens' Music",          // 4
	"Classical",                 // 5
	"Contemporary Christian",    // 6
	"Country",                   // 7
	"Dance",                     // 8
	"Easy Listening",            // 9
	"Erotic",                    // 10
	"Folk",                      // 11
	"Gospel",                    // 12
	"Hip Hop",                   // 13
	"Jazz",                      // 14
	"Latin",                     // 15
	"Musical",                   // 16
	"New Age",                   // 17
	"Opera",                     // 18
	"Operetta",                  // 19
	"Pop Music",                 // 20
	"RAP",                       // 21
	"Reggae",                    // 22
	"Rock Music",                // 23
	"Rhythm & Blues",            // 24
	"Sound Effects",             // 25
	"Sound Track",               // 26
	"Spoken Word",               // 27
	"World Music"                // 28
	// 29..32767 Reserved
};

char WNASPI32_DLL[] = "WNASPI32.DLL";
PSTR WNASPI32_ERRORS[] = {
/*0x00 SS_PENDING*/   "SCSI command being processed - pending",
/*0x01 SS_OK */       "OK",
/*0x02 SS_ABORTED*/   "SCSI command aborted",

/*0x80 SS_INVALID_CMD*/   "Invalid ASPI command was passed",
/*0x81 SS_INVALID_HA*/    "An invalid host adapter ID was specified",
/*0x82 SS_NO_DEVICE*/     "No SCSI devices found/detected",

/*0xE0 SS_INVALID_SRB*/   "Invalid parameter set in SCSI command packet",
/*0xE1 SS_BUFFER_ALIGN*/  "SCSI buffer not aligned",
/*0xE2 SS_ILLEGAL_MODE*/  "Unsupported Windows mode",
/*0xE3 SS_NO_ASPI*/	      "No ASPI managers resident",
/*0xE4 SS_FAILED_INIT*/	  "ASPI for Windows failed initialization",
/*0xE5 SS_ASPI_IS_BUSY*/  "ASPI is busy - No resources available to execute command",
/*0xE6 SS_BUFFER_TO_BIG*/ "Buffer size too big to handle",
/*0xE7 SS_MISMATCHED_COMPONENTS*/  "The ASPI DLL/VXD components don't version check",
/*0xE8 SS_NO_ADAPTERS*/	           "No host adapters to manage",
/*0xE9 SS_INSUFFICIENT_RESOURCES*/ "Insufficient resources for initialization",
/*0xEA SS_ASPI_IS_SHUTDOWN*/ "Call came to ASPI after being shutdown",
/*0xEB SS_BAD_INSTALL*/	     "The ASPI DLL or other components are installed wrong"
};

// Universal function to get a handle of the CD device for SPTI cmds!
// Tries to avoid the need for Admin Rights! (Reference DiskID32 code)
// Multiple methods in flagging used in the hopes of one of them working!
// Key Point: False positives need to be addressed. NT4 fixed at least!
// CreateFile can return a handle that's no good, must test it some way.

// https://support.microsoft.com/en-us/kb/241374
// Starting with Windows NT 4.0 Server SP4 and beyond (including Windows 2K),
// there are new access requirements for SCSI (small computer system interface)
// pass through requests. For SCSI pass through requests, both GENERIC_READ and
// GENERIC_WRITE access must be specified in the dwDesiredAccess parameter of
// the CreateFile call.

HANDLE __fastcall SPTI_GetCDDriveHandle(char cDriveLetter) {
	char szDevicePath[] = "\\\\.\\A:";
	DWORD dwDesiredAccess;
	HANDLE hDevice;
// SPTI calls assume PlatformID test >= 2 took place for >=NT
	szDevicePath[4] = cDriveLetter;
//** Microsoft's modern way for WinNT4svr/2K/XP/V/7/8/10, requires GENERIC_READ|GENERIC_WRITE!
	dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
	if ( gOSInfo.MajorVersion < 5 && gOSInfo.IsWinNT == VER_PLATFORM_WIN32_NT )
	//** Old Windows NT3-4 Workstation method if needed...
	// MajorVersion = 3 is Windows NT 3.1-3.51, 4 is Windows NT 4.0
		if ( !(gOSInfo.MajorVersion == 4 && gOSInfo.IsServer && gOSInfo.ServicePackMajor == 4) ) // Exit on NT4 Server
		// Tested on NT4 Workstation (SP6) - Fully works with user/admin rights!!!
			dwDesiredAccess = GENERIC_READ; // Must ONLY ask for GENERIC_READ permission for NT3/4 Client, fails otherwise!
	hDevice = CreateFileA(
		szDevicePath,
		dwDesiredAccess,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if ( hDevice != INVALID_HANDLE_VALUE )
		return hDevice;
//** Final: Try Windows 2K/XP ZERO Rights method (AdminRights not required). Not sure of usefulness...??
	return CreateFileA(szDevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

// IOCTL_SCSI_PASS_THROUGH_DIRECT is much, much faster according to Chris Covell, than
// using the SPTI prescribed IO codes for raw or cooked SCSI reads in WinXP (and others?).

BOOL __fastcall SPTI_ExecSCSICmd1() { // Full SCSI buffer version
	BOOLEAN bResult;
// Set up for most common SCSI commands using our full 64KB buffer
	gSPTI_Exec.sptd.DataIn = SCSI_IOCTL_GETTING_DATA_FROM_DEVICE;
	gSPTI_Exec.sptd.DataBuffer = gAlignedSCSIBuffer;
	gSPTI_Exec.sptd.DataTransferLength = SCSI_BUFFER_LIMIT;
	gSPTI_Exec.sptd.SenseInfoLength = sizeof(SENSE_DATA_FMT); // This gets reset to 0 per call!
// We're ready, send SCSI command to the CD/DVD drive!
	bResult = DeviceIoControl(
		ghCDRom,
		IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&gSPTI_Exec, sizeof(SPTI_EXEC_SCSI_CMD),
		&gSPTI_Exec, sizeof(SPTI_EXEC_SCSI_CMD),
		&gnNumberOfBytes, NULL
	);
// 1st test for failure, the SCSI Status flag!
	if ( gSPTI_Exec.sptd.ScsiStatus ) { // != STATUS_GOOD
	// Failure: Set global SENSE variables for ASPIGetLastError reporting use
		gSenseKey = gSPTI_Exec.SenseArea.SenseKey;
		gAddSenQual =  gSPTI_Exec.SenseArea.AddSenQual;
		gAddSenseCode = gSPTI_Exec.SenseArea.AddSenseCode;
		return FALSE;
	}
// 2nd test for failure, true or false from DeviceIoControl's return!
	if ( bResult )
		return TRUE; // We're OK, return true!
// Failure: Set global Win ErrorCode, return false!
// Use ASPIGetLastError to map code to error string
	gLAST_IOCTL_ERROR = GetLastError();
	return FALSE;
}

BOOL __fastcall SPTI_ExecSCSICmd2(PBYTE lpBuffer, DWORD BufferSize) {
	BOOLEAN bResult;
// If buffer is present, we're USUALLY *GETTING* data FROM the SCSI device!
	if ( BufferSize ) {
		if ( gSPTI_Exec.sptd.CDBByte[0] != SCSI_MODE_SEL10 ) // The _ONE_ exception so far is MODE SELECT!
			gSPTI_Exec.sptd.DataIn = SCSI_IOCTL_GETTING_DATA_FROM_DEVICE;
	// PROPER DATA_IN FLAG FOR MODE SELECT = 0 - THE DRIVE WILL LOCK UP IF THIS FLAG IS SET WRONGLY! !
	// We're SENDING a databuffer to change the drive's behavior, NOT to receive to data
		else
			gSPTI_Exec.sptd.DataIn = 0; //or SCSI_IOCTL_SENDING_DATA_TO_DEVICE; 
		gSPTI_Exec.sptd.DataBuffer = lpBuffer;
		gSPTI_Exec.sptd.DataTransferLength = BufferSize;
	} else {
	// We're not expecting data, we're doing testUnitReady, or StartStopLoad, etc. type commands	
		gSPTI_Exec.sptd.DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
		gSPTI_Exec.sptd.DataBuffer = 0;
		gSPTI_Exec.sptd.DataTransferLength = 0;
	}
	gSPTI_Exec.sptd.SenseInfoLength = sizeof(SENSE_DATA_FMT); // This gets reset to 0 per call!
// We're ready, send the SCSI command to the CD/DVD drive!
	bResult = DeviceIoControl(
		ghCDRom,
		IOCTL_SCSI_PASS_THROUGH_DIRECT,
		&gSPTI_Exec, sizeof(SPTI_EXEC_SCSI_CMD),
		&gSPTI_Exec, sizeof(SPTI_EXEC_SCSI_CMD),
		&gnNumberOfBytes, NULL
	);
// 1st test for failure, the SCSI Status flag!
	if ( gSPTI_Exec.sptd.ScsiStatus ) { // != STATUS_GOOD
	// Failure: Set global SENSE variables for ASPIGetLastError reporting use
		gSenseKey = gSPTI_Exec.SenseArea.SenseKey;
		gAddSenQual =  gSPTI_Exec.SenseArea.AddSenQual;
		gAddSenseCode = gSPTI_Exec.SenseArea.AddSenseCode;
		return FALSE;
	}
// 2nd test for failure, true or false from DeviceIoControl's return!
	if ( bResult )
		return TRUE; // We're OK, return true!
// Failure: Set global Win ErrorCode, return false!
// Use ASPIGetLastError to map code to error string
	gLAST_IOCTL_ERROR = GetLastError();
	return FALSE;
}

BOOL __fastcall ASPI_ExecSCSICmd1() { // Full SCSI buffer version
	gASPI_Exec.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
	gASPI_Exec.SRB_SenseLen   = sizeof(SENSE_DATA_FMT);
	gASPI_Exec.SRB_BufPointer = gAlignedSCSIBuffer;
	gASPI_Exec.SRB_BufLen     = SCSI_BUFFER_LIMIT;
	gASPI_Exec.SRB_Status = 0;
	ResetEvent(ghEventSRB);
	if ( SendASPI32Command((LPSRB)&gASPI_Exec) == SS_PENDING )
		WaitForSingleObject(ghEventSRB, DEF_WAITLEN_MS);
	if ( gASPI_Exec.SRB_Status != SS_COMP ) {
		if ( gASPI_Exec.SRB_TargStat == STATUS_CHKCOND ) {
			gSenseKey = gASPI_Exec.SenseArea.SenseKey;
			gAddSenQual = gASPI_Exec.SenseArea.AddSenQual;
			gAddSenseCode = gASPI_Exec.SenseArea.AddSenseCode;
			return FALSE;
		}
		gASPIStatus = gASPI_Exec.SRB_Status;
		return FALSE;
	}
	return TRUE;
}

BOOL __fastcall ASPI_ExecSCSICmd2(PBYTE lpBuffer, DWORD BufferSize) {
	if ( BufferSize )
		if ( gASPI_CDBByte[0] != SCSI_MODE_SEL10 )
			gASPI_Exec.SRB_Flags = SRB_DIR_IN | SRB_EVENT_NOTIFY;
		else
			gASPI_Exec.SRB_Flags = SRB_DIR_OUT | SRB_EVENT_NOTIFY;
	else
		gASPI_Exec.SRB_Flags = SRB_DIR_SCSI | SRB_EVENT_NOTIFY;
	gASPI_Exec.SRB_SenseLen   = sizeof(SENSE_DATA_FMT);
	gASPI_Exec.SRB_BufPointer = lpBuffer;
	gASPI_Exec.SRB_BufLen     = BufferSize;
	gASPI_Exec.SRB_Status = 0;
	ResetEvent(ghEventSRB);
	if ( SendASPI32Command((LPSRB)&gASPI_Exec) == SS_PENDING )
		WaitForSingleObject(ghEventSRB, DEF_WAITLEN_MS);
	if ( gASPI_Exec.SRB_Status != SS_COMP ) {
		if ( gASPI_Exec.SRB_TargStat == STATUS_CHKCOND ) {
			gSenseKey = gASPI_Exec.SenseArea.SenseKey;
			gAddSenQual = gASPI_Exec.SenseArea.AddSenQual;
			gAddSenseCode = gASPI_Exec.SenseArea.AddSenseCode;
			return FALSE;
		}
		gASPIStatus = gASPI_Exec.SRB_Status;
		return FALSE;
	}
	return TRUE;
}

extern "C" {

// CD-TEXT Unpacking
// Unresolved for possible future considerations:
// 1) Handling of 0x09 tab character to indicate repetition of previous string
// 2) Crunching the included CRC16 against text pack to make sure it's not corrupted

CDTEXT_PACK_DATA* __fastcall UnpackCDTextData(LPMASTERTOC lpMasterTOC, CDTEXT_PACK_DATA* lpCDTextPack) {
	MASTER_TOC_CDTEXT_TRACKINFO *lpTrackData;
	UINT i, j, k, TrackIndex;
	BYTE LastPackType;
	LPSTR AlbumText;
// Don't deal with DBCS encoded CD-TEXT
	if ( lpCDTextPack->DBCC == 1 )
		return NULL;
	LastPackType = lpCDTextPack->PackType;
	switch (LastPackType) {
		case 0x80: // Title of Album name(ID2=00h) or Track Titles (ID2=01h...63h)
			AlbumText = lpMasterTOC->CDTextData.AlbumTitle;
			break;
		case 0x81: // Name(s) of the performer(s) (in ASCII)
			AlbumText = lpMasterTOC->CDTextData.Performer;
			break;
		case 0x82: // Name(s) of the songwriter(s) (in ASCII)
			AlbumText = lpMasterTOC->CDTextData.Songwriter;
			break;
		case 0x83: // Name(s) of the composer(s) (in ASCII)
			AlbumText = lpMasterTOC->CDTextData.Composer;
			break;
		case 0x84: // Name(s) of the arranger(s) (in ASCII)
			AlbumText = lpMasterTOC->CDTextData.Arranger;
			break;
		case 0x85: // Message(s) from content provider and/or artist (in ASCII)
			AlbumText = lpMasterTOC->CDTextData.Message;
			break;
		case 0x86: // Media Catalogue Number/Disc Identification information
			AlbumText = lpMasterTOC->CDTextData.Catalog;
			break;
		case 0x87: // Genre Identification and Genre information
			AlbumText = lpMasterTOC->CDTextData.GenreDesc;
			break;
		case 0x8E: // UPC/EAN code of the album and ISRC code of each track
			AlbumText = lpMasterTOC->CDTextData.UPC;
			break;
		default:
			return ++lpCDTextPack; // We don't know how to deal with any other or undefined types
	}
	lpTrackData = lpMasterTOC->CDTextData.TrackData;
	TrackIndex = lpCDTextPack->TrackOrPackNumber;
	i = j = 0;
	do {
		if ( TrackIndex == 0 ) {
			if ( LastPackType == 0x87 && i == 0 ) { // Handle special case for Genre
				i = 2;
				k = lpCDTextPack->TextData[1];
				if ( k >= 1 && k <= 29 )
					lpMasterTOC->CDTextData.Genre = GENRE_TYPES[k];
			}
			while ( i < 12 && lpCDTextPack->TextData[i] )
				AlbumText[j++] = lpCDTextPack->TextData[i++];
		} else {
			if ( TrackIndex > lpMasterTOC->LastTrack )
				return NULL;
			for ( k = TrackIndex - 1; i < 12 && lpCDTextPack->TextData[i]; j++, i++ ) {
				switch (LastPackType) {
					case 0x80: // Title of Album name(ID2=00h) or Track Titles (ID2=01h...63h)
						lpTrackData[k].Title[j] = lpCDTextPack->TextData[i];
						break;
					case 0x81: // Name(s) of the performer(s) (in ASCII)
						lpTrackData[k].Performer[j] = lpCDTextPack->TextData[i];
						break;
					case 0x82: // Name(s) of the songwriter(s) (in ASCII)
						lpTrackData[k].Songwriter[j] = lpCDTextPack->TextData[i];
						break;
					case 0x83: // Name(s) of the composer(s) (in ASCII)
						lpTrackData[k].Composer[j] = lpCDTextPack->TextData[i];
						break;
					case 0x84: // Name(s) of the arranger(s) (in ASCII)
						lpTrackData[k].Arranger[j] = lpCDTextPack->TextData[i];
						break;
					case 0x85: // Message(s) from content provider and/or artist (in ASCII)
						lpTrackData[k].Message[j] = lpCDTextPack->TextData[i];
						break;
					case 0x8E: // UPC/EAN code of the album and ISRC code of each track
						lpTrackData[k].ISRC[j] = lpCDTextPack->TextData[i];
				}
			}
		}
		if ( i < 12 ) // We finished a string from a text pack
			j = 0; // Reset target string index to get next possible new string
		while ( i < 12 && lpCDTextPack->TextData[i] == 0 ) { // Moved passed all nulls
			TrackIndex++; // Every null passed indicates skipping of track; we could still get to valid data before the pack ends!
			i++;
		}
		if ( i == 12 ) { // We've reached the end of a pack; goto the next one
			i = 0;
			lpCDTextPack++;
		}
	} while ( lpCDTextPack->PackType == LastPackType );
	return lpCDTextPack;
}

BOOL __fastcall ASPIGetDVDStruct() {
// 0. Zero Initialize SCSI buffer/CDB
	__asm {
		CLD
		XOR   EAX, EAX
	// Zero out SCSI buffer
		MOV   EDI, gAlignedSCSIBuffer
		MOV   ECX, 2052/4
		REP   STOSD
	// Zero 16 bytes of CDB
		MOV   EDI, gASPI_CDBByte
		MOV   ECX, 4
		REP   STOSD
	}
	*gASPI_CDBLen = 12; // Set command size
	gASPI_CDBByte[0] = 0xAD;  // READ DVD Struct command (SCSIOP_READ_DVD_STRUCTURE)
// 0x0804 = 2052 bytes
    gASPI_CDBByte[8] = 0x08;  // MSB of max length of bytes to receive
    gASPI_CDBByte[9] = 0x04;  // LSB of max length of bytes to receive  
	return ExecuteSCSICmd2(gAlignedSCSIBuffer, 2052);
}

LPBYTE __fastcall ASPIModeSense10(BYTE nPageCode, LPBYTE pbtBuffer, WORD nBufSize) {
	mMemZeroBy4(gASPI_CDBByte, 4); // Zero 16 bytes of CDB
	*gASPI_CDBLen = 12; // Set command size
	gASPI_CDBByte[0] = SCSI_MODE_SEN10;	// MODESENSE Operation
	gASPI_CDBByte[2] = nPageCode;		// Set to desired page
	gASPI_CDBByte[7] = (BYTE)(nBufSize >> 8);
// You cannot recklessly overspecify a buffer size! If it's greater than the true
// page size + ~8, it WILL fail! Use Default Page Size + 8 for best results!!
	gASPI_CDBByte[8] = (BYTE)nBufSize;
	if ( ExecuteSCSICmd2(pbtBuffer, nBufSize) ) {
	// First 8 bytes returned of code page have size/len info
		WORD modeDataLen = (pbtBuffer[0] << 8) | pbtBuffer[1];
		WORD blockDescLen = (pbtBuffer[6] << 8) | pbtBuffer[7];
		if ( modeDataLen > blockDescLen + 6 ) {
			blockDescLen += 8;
			pbtBuffer += blockDescLen;
			if ( (*pbtBuffer & 0x3F) == nPageCode && pbtBuffer[1] > 0 ) // Verification of Code Page data OK!
				return pbtBuffer;
		}
	}
	return NULL;
}

// 1:0:0 - MODE SELECT 10
// CDB Contents: 55h 10h 00h 00h 00h 00h 00h 00h 10h 00h 
// ASPI Status : 04h 00h 02h 05h 1Ah 00h
// Requesting SCSI-2 Compliant PAGE

BOOL __fastcall ASPIModeSelect10(BYTE btBuffer[], WORD BufferSize) {
	mMemZeroBy4(gASPI_CDBByte, 4); // Zero 16 bytes of CDB
	*gASPI_CDBLen = 10; // Set command size
	gASPI_CDBByte[0] = SCSI_MODE_SEL10;
	gASPI_CDBByte[1] = 0x10;
	gASPI_CDBByte[7] = (BYTE)(BufferSize >> 8);
	gASPI_CDBByte[8] = (BYTE)BufferSize;
	return ExecuteSCSICmd2(btBuffer, BufferSize);
}

// Print out some of the CD/DVD Drive's features/specs/settings if MMC

void __fastcall ASPIPrintMMCFeaturesPage(SCSICDMODEPAGE_2A* lpMMCPage) {
	char szReadBufferSize[16], szMaxReadSpeed[16];
	DWORD dwValue;
	dwValue = (lpMMCPage->buffer_size[0]<<8) | lpMMCPage->buffer_size[1];
	FormatNumber(dwValue, szReadBufferSize);
	dwValue = (lpMMCPage->max_read_speed[0]<<8) | lpMMCPage->max_read_speed[1];
	dwValue = round(dwValue/XX1);
	FormatNumber(dwValue, szMaxReadSpeed);
	printf("\r\n"
		"*** Drive Capabilities & Features ***\r\n"
		"Reads DVD-ROM Media      : %s\r\n"
		"Reads Multi-Session Media: %s\r\n"
		"Reads MODE2/FORM 1&2 [XA]: %s,%s\r\n"
		"Reads R-W SubCodes Joined: %s\r\n"
		"Reads R-W De-Interleaved : %s\r\n"
		"Reads C2 Error Pointers  : %s\r\n"
		"Reads ISRC Information   : %s\r\n"
		"Reads Media Catalog#[UPC]: %s\r\n"
		"Reads Media Bar Codes    : %s\r\n"
		"CD-DA Command Support    : %s\r\n"
		"CD-DA Stream-Is-Accurate : %s\r\n"
		"Read Buffer Size         : %s KB\r\n"
		"Read Speed Max Reported  : %lux (%s kBps)\r\n",
		MSG_YES_NO[lpMMCPage->dvd_rom_read],
		MSG_YES_NO[lpMMCPage->multi_session],
		MSG_YES_NO[lpMMCPage->mode_2_form_1],
		MSG_YES_NO[lpMMCPage->mode_2_form_2],
		MSG_YES_NO[lpMMCPage->rw_supported],
		MSG_YES_NO[lpMMCPage->rw_deint_cor],
		MSG_YES_NO[lpMMCPage->c2_pointers],
		MSG_YES_NO[lpMMCPage->ISRC],
		MSG_YES_NO[lpMMCPage->UPC],
		MSG_YES_NO[lpMMCPage->read_bar_code],
		MSG_YES_NO[lpMMCPage->cd_da_supported],
		MSG_YES_NO[lpMMCPage->cd_da_accurate],
		szReadBufferSize, dwValue, szMaxReadSpeed
	);
}

// I think, for the most part, this is only needed for NT family
// and can assume, with luck, the drive letter ordering in 95/98/ME
// will match the SCSI ordering. Not true for NT, and no solution to
// confirm letters on 9X exists, so I give up, will accept just NT support
// If Win9X user has only 1 CDROM, accuracy is 100%, so good enough

void __fastcall ASPIValidateDriveIDs(CD_DRIVELIST *lpCDDriveList, char nCDDrives) {
	char i, *lpszVendorID;
	CD_DRIVELIST *lpDrive;
// If there is only 1 detected CD drive, we're OK, no letter assignment guesses!
	if ( nCDDrives == 1 )
		return;
// If we're on the NT family, we can cheat and use SPTI to confirm drive letters!
	if ( gOSInfo.IsWinNT ) {
	// Temporarily switch to SPTI mode since we can on NT
		gASPI_CDBByte = gSPTI_Exec.sptd.CDBByte;
		gASPI_CDBLen = &gSPTI_Exec.sptd.CdbLength;
		ExecuteSCSICmd2 = &SPTI_ExecSCSICmd2;
	// Loop thru drives 'A' to 'Z' for CDROMs
		for ( char szDrive[] = "A:\\"; *szDrive <= 'Z'; (*szDrive)++ )
			if ( GetDriveTypeA(szDrive) == DRIVE_CDROM ) {
				ghCDRom = SPTI_GetCDDriveHandle(*szDrive);
				if ( ghCDRom == INVALID_HANDLE_VALUE )
					continue;
			// Get unique CD drive's Vendor/ProductID
				if ( ASPIStandardInquiry() ) {
					lpszVendorID = (PSTR)(gAlignedSCSIBuffer+8);
					for ( i = 0, lpDrive = lpCDDriveList; i < nCDDrives; i++, lpDrive++ )
						if ( !memcmp(lpszVendorID, lpDrive->szVendorProdID, 28) ) {
							lpDrive->cDriveLetter = *szDrive;
							break;
						}
				}
				ASPICloseCDDrive();
			}
	// Switch back to ASPI32 mode, return control to ASPI Manager running on NT
		if ( !bUseSPTI ) {
			ExecuteSCSICmd2 = &ASPI_ExecSCSICmd2;
			gASPI_CDBLen = &gASPI_Exec.SRB_CDBLen;
			gASPI_CDBByte = gASPI_Exec.CDBByte;
		}
	}
}

char __fastcall ASPIGetCDDrives(CD_DRIVELIST *lpCDDriveList) {
	char szDrive[] = "A:\\";
	SRB_GDEVBlock DevType;
	CD_DRIVELIST *lpDrive;
	char nCDDrives = 0;
// *********************************************************************************
// Use easy SPTI way for NT family
// *********************************************************************************
	if ( bUseSPTI ) {
		for ( ; *szDrive <= 'Z'; (*szDrive)++ ) // Loop thru drives 'A' to 'Z' for CDROMs
			if ( GetDriveTypeA(szDrive) == DRIVE_CDROM ) {
				ghCDRom = SPTI_GetCDDriveHandle(*szDrive);
				if ( ghCDRom != INVALID_HANDLE_VALUE ) {
					lpCDDriveList->cDriveLetter = *szDrive; // Save drive letter
				// Get CD Vendor Product ID string
					if ( ASPIStandardInquiry() )
						MemCpy(29, lpCDDriveList->szVendorProdID, gAlignedSCSIBuffer+8);
					else
						*lpCDDriveList->szVendorProdID = 0;
				// Is drive ready with CD loaded ?
					lpCDDriveList->bDiscReady = ASPILoadTrayReadyUnit();
					CloseHandle(ghCDRom); // We're done
					lpCDDriveList++;
					nCDDrives++; // Count total drives found
					if ( nCDDrives == 9 ) // Enforce 9 drive maximum support!
						break;
				} else
					PrintLastSysError("Error: Drive %s", *szDrive);
			}
		ghCDRom = NULL;
		return nCDDrives;
	}
// *********************************************************************************
// Must use harder ASPI method if requested or Win95/98/ME core
// *********************************************************************************
	nCDDrives = 0;
	lpDrive = lpCDDriveList;
// Get all CD drive letters (May or may not be accurate when matched against ASPI ID)
	for ( ; *szDrive <= 'Z'; (*szDrive)++ ) // Loop thru drives 'A' to 'Z' for CDROMs
		if ( GetDriveTypeA(szDrive) == DRIVE_CDROM ) {
			lpDrive->cDriveLetter = *szDrive;
			lpDrive++;
			nCDDrives++; // Count total drives found
			if ( nCDDrives == 9 ) // Enforce 9 drive maximum support!
				break;
		}
	nCDDrives = 0;
	lpDrive = lpCDDriveList;
// Loop thru all CD drives ASPI way, populate needed data
	for ( BYTE byHaId = 0; byHaId < gNumAdapters; byHaId++ )
		for ( BYTE byTarget = 0; byTarget < MAXTARG; byTarget++ )
			for ( BYTE byLun = 0; byLun < MAXLUN; byLun++ ) {
				mMemZeroBy4(DevType, SIZE SRB_GDEVBlock/4);
				DevType.SRB_Cmd = SC_GET_DEV_TYPE;
				DevType.SRB_HaId = byHaId;
				DevType.SRB_Target = byTarget;
				DevType.SRB_Lun = byLun;
				SendASPI32Command((LPSRB)&DevType);
				if ( DevType.SRB_DeviceType == DTYPE_CDROM ) {
					nCDDrives++; // Count CD Device ID at #1
					lpDrive->hCDrive.HaId = byHaId;
					lpDrive->hCDrive.Target = byTarget;
					lpDrive->hCDrive.LUnit = byLun;
					gASPI_Exec.SRB_HaId = byHaId;
					gASPI_Exec.SRB_Target = byTarget;
					gASPI_Exec.SRB_Lun = byLun;
					if ( ASPIStandardInquiry() ) {
					// Get CD Vendor Product ID string
						MemCpy(29, lpDrive->szVendorProdID, gAlignedSCSIBuffer+8);
					// Is drive ready with CD loaded?
						lpDrive->bDiscReady = ASPILoadTrayReadyUnit();
					} else {
						lpDrive->bDiscReady = *lpDrive->szVendorProdID = 0;
						PrintLastSysError("Error: Drive #%hu/%c:\\ (%hu:%hu:%hu)", nCDDrives, lpDrive->cDriveLetter, byHaId, byTarget, byLun);
					}
					lpDrive++;
					if ( nCDDrives == 9 ) {// Enforce 9 drive maximum support!
						ASPIValidateDriveIDs(lpCDDriveList, 9);
						return 9;
					}
				}
			}
	ASPIValidateDriveIDs(lpCDDriveList, nCDDrives);
	return nCDDrives;
}

void __fastcall SPTIAlignBuffer() {
	char szSCSIDevicePath[] = "\\\\.\\Scsi0:";
	IO_SCSI_CAPABILITIES SCSIDeviceCaps;
	SCSI_ADDRESS SCSIAddress;
	BOOLEAN bResult;
	HANDLE hDevice;
// Only with SPTI if we're in NT/2K/XP/VISTA/7/8/10
	if ( !ghCDRom )
		return;
	MemZero(sizeof(SCSIDeviceCaps), &SCSIDeviceCaps);
// Try #1: With normal DriveLetter Device Handle (My tests: Works on Win2000, fails on Vista)
	if (
		!(DeviceIoControl(ghCDRom, IOCTL_SCSI_GET_CAPABILITIES, NULL, 0, &SCSIDeviceCaps, sizeof(IO_SCSI_CAPABILITIES), &gnNumberOfBytes, NULL)
			&&
		gnNumberOfBytes == sizeof(IO_SCSI_CAPABILITIES))
		) {
	// Try #2: With SCSI Backdoor handle to device via port #, i.e. "\\.\Scsi[1-9]:" (Works on Vista!)
		SCSIAddress.PortNumber = 0;
		if ( !(DeviceIoControl(ghCDRom, IOCTL_SCSI_GET_ADDRESS, NULL, 0, &SCSIAddress, sizeof(SCSI_ADDRESS), &gnNumberOfBytes, NULL) && gnNumberOfBytes == sizeof(SCSI_ADDRESS)) )
			return;
		szSCSIDevicePath[8] = SCSIAddress.PortNumber + '0';
		hDevice = CreateFileA(
			szSCSIDevicePath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if ( hDevice == INVALID_HANDLE_VALUE )
			return;
		bResult = (
			DeviceIoControl(hDevice, IOCTL_SCSI_GET_CAPABILITIES, NULL, 0, &SCSIDeviceCaps, sizeof(IO_SCSI_CAPABILITIES), &gnNumberOfBytes, NULL)
				&&
			gnNumberOfBytes == sizeof(IO_SCSI_CAPABILITIES)
		);
		CloseHandle(hDevice);
		if ( !bResult )
			return;
	}
// 64,528 (27*2352+1024) - Our ~63KB SCSI buffer has 1K unused, so we have space to spare for alignment
	if ( SCSIDeviceCaps.AlignmentMask && SCSIDeviceCaps.AlignmentMask < 512 ) // I doubt mask could be >64, but I'll leave it at 512
		gAlignedSCSIBuffer = (LPBYTE)(AddPtr(ULONG_PTR, gAlignedSCSIBuffer, SCSIDeviceCaps.AlignmentMask) & (ULONG_PTR)~SCSIDeviceCaps.AlignmentMask);
}

DWORD __fastcall ASPIHostAdapterInquiry() { // Needs SPTI update
	DWORD dwMaxTransferBytes;
// An Adaptec ASPI32 feature only
	if ( !SendASPI32Command )
		return 0;
	gSRBHAInquiry.SRB_Cmd  = SC_HA_INQUIRY;
	gSRBHAInquiry.SRB_HaId = gASPI_Exec.SRB_HaId;
	SendASPI32Command((LPSRB)&gSRBHAInquiry);
	if ( gSRBHAInquiry.SRB_Status != SS_COMP )
		return FALSE;
	dwMaxTransferBytes = *(LPDWORD)(gSRBHAInquiry.HA_Unique + 4);
	return dwMaxTransferBytes;
}

BOOL __fastcall ASPISetCDDrive(CD_DRIVELIST *lpCDDrive) {
// Use easy SPTI method
	if ( bUseSPTI ) {
	// Close previous CD handle just in case (avoid CloseCDDrive calls)
		if ( ghCDRom && ghCDRom != INVALID_HANDLE_VALUE )
			CloseHandle(ghCDRom);
		ghCDRom = SPTI_GetCDDriveHandle(lpCDDrive->cDriveLetter);
		if ( ghCDRom != INVALID_HANDLE_VALUE ) {
			SPTIAlignBuffer(); // Is helpful if it succeeds, no big deal if not
			return TRUE;
		}
		ghCDRom = NULL;
		return FALSE;
// Use Adaptec ASPI selection method
	} else {
		gASPI_Exec.SRB_HaId = lpCDDrive->hCDrive.HaId;
		gASPI_Exec.SRB_Target = lpCDDrive->hCDrive.Target;
		gASPI_Exec.SRB_Lun = lpCDDrive->hCDrive.LUnit;
		return TRUE;
	}
}

void __fastcall ASPICloseCDDrive() {
// Only needed for SPTI usage
	if ( ghCDRom ) {
		if ( ghCDRom != INVALID_HANDLE_VALUE )
			CloseHandle(ghCDRom);
		ghCDRom = NULL;
	}
}

BOOL __fastcall ASPIGetTOCLBA(LPTOC lpTocLBA) {
	TRACK_DESCRIPTOR_00 *lpLBATrack;
	BYTE TotalTracks, bAllAudio;
	PDWORD pLBA;
// Get TOC in default LBA format
// 0. Zero Initialize TOC data/CDB
	__asm {
		CLD
		XOR   EAX, EAX
	// Zero out LBA TOC buffer
		MOV   EDI, lpTocLBA
		MOV   ECX, SIZE TOC_FORMAT_00/4
		REP   STOSD
	// Zero 16 bytes of CDB
		MOV   EDI, gASPI_CDBByte
		MOV   ECX, 4
		REP   STOSD
	}
	*gASPI_CDBLen = 10; // Set command size
	gASPI_CDBByte[0] = SCSI_READ_TOC; // READ TOC Command
	//gASPI_CDBByte[1] = 0x00;   // Default 00 TOC Format: Address in LBA form (MSF bit zero)
	gASPI_CDBByte[7] = (BYTE)(sizeof(TOC_FORMAT_00) >> 8); // Should always be 804 bytes for default TOC format!!
	gASPI_CDBByte[8] = (BYTE) sizeof(TOC_FORMAT_00);      // Low word of size
	if ( ExecuteSCSICmd2((PBYTE)lpTocLBA, sizeof(TOC_FORMAT_00)) ) {
		TotalTracks = (lpTocLBA->LastTrack - lpTocLBA->FirstTrack) + 2; // Count Leadout track too!
		lpLBATrack = lpTocLBA->TrackData;
		bAllAudio = TRUE;
		while ( TotalTracks ) {
			pLBA = (LPDWORD)&lpLBATrack->LBAddress; // Fix byte order of LBA
			MSB2DWORD(pLBA); // Fix byte order of LBA
			if ( lpLBATrack->TrackNumber != 0xAA && !IsAudioTrack(lpLBATrack->CONTROL) )
				bAllAudio = FALSE;
			TotalTracks--;
			lpLBATrack++;
		}
		lpTocLBA->TrackData[0].Reserved1 = bAllAudio;
		return TRUE;
	}
	return FALSE;
}

BYTE __fastcall ASPIGetTOCMSF(LPTOC lpTocMSF) {
	BYTE retTotalTracks, TotalTracks, bAllAudio;
	TRACK_DESCRIPTOR_00 *lpMSFTrack;
// Get TOC in MSF format
// 0. Zero Initialize TOC data/CDB
	__asm {
		CLD
		XOR   EAX, EAX
	// Zero out MSF TOC buffer
		MOV   EDI, lpTocMSF
		MOV   ECX, SIZE TOC_FORMAT_00/4
		REP   STOSD
	// Zero 16 bytes of CDB
		MOV   EDI, gASPI_CDBByte
		MOV   ECX, 4
		REP   STOSD
	}
	*gASPI_CDBLen = 10; // Set command size
	gASPI_CDBByte[0] = SCSI_READ_TOC; // READ TOC Command
	gASPI_CDBByte[1] = 0x02;   // Set MSF bit : Address in MSF form (MSF bit zero)
	gASPI_CDBByte[7] = (BYTE)(sizeof(TOC_FORMAT_00) >> 8); // Should always be 804 bytes for default TOC format!!
	gASPI_CDBByte[8] = (BYTE)sizeof(TOC_FORMAT_00);      // Low word of size
	if ( ExecuteSCSICmd2((PBYTE)lpTocMSF, sizeof(TOC_FORMAT_00)) ) {
		TotalTracks = lpTocMSF->LastTrack;
		if ( !TotalTracks ) // Minor TOC validation, if last track #=0, TOC's bad!!
			return 0;
		TotalTracks = (TotalTracks - lpTocMSF->FirstTrack) + 1; // Compute total tracks (last - first track#)
		if ( TotalTracks > 99 ) // Minor TOC validation, if >99, TOC's bad!!
			return 0;
		lpMSFTrack = lpTocMSF->TrackData;
		retTotalTracks = TotalTracks;
		bAllAudio = TRUE;
		while ( TotalTracks ) {
			if ( !IsAudioTrack(lpMSFTrack->CONTROL) )
				bAllAudio = FALSE;
			TotalTracks--;
			lpMSFTrack++;
		}
		if ( lpMSFTrack->TrackNumber != 0xAA ) // Minor TOC validation, leadout track should = 0xAA
			return 0;
		lpTocMSF->TrackData[0].Reserved1 = bAllAudio;
		return retTotalTracks;
	}
	return 0;
}

// 1:1:0 - READ TOC
// CDB Contents: 43h 00h 00h 00h 00h 00h 00h 03h 24h 00h 
// ASPI Status : 01h 00h 00h 00h 00h 00h

// 1:1:0 - READ TOC ???? CDRWIN Mystery here with this other CDB/Command packet...
// CDB Contents: 43h 00h 00h 00h 00h 00h 00h 00h 0Ch 40h 
// ASPI Status : 01h 00h 00h 00h 00h 00h

BYTE __fastcall ASPIGetTOCBasic(LPTOC lpTocLBA, PDWORD lpBytesWritten) {
	TRACK_DESCRIPTOR_00 *lpLBATrack, *lpMSFTrack;
	BYTE retTotalTracks, TotalTracks;
	TOC_FORMAT_00 tocMSF;
	LPSTR lpBuffer;
// 0. Zero Initialize TOC data/CDB
	__asm {
		CLD
		XOR   EAX, EAX
	// Zero out LBA TOC buffer
		MOV   EDI, lpTocLBA
		MOV   ECX, SIZE TOC_FORMAT_00/4
		REP   STOSD
	// Zero out TOC MSF buffer
		LEA   EDI, tocMSF
		MOV   ECX, SIZE TOC_FORMAT_00/4
		REP   STOSD
	// Zero 16 bytes of CDB
		MOV   EDI, gASPI_CDBByte
		MOV   ECX, 4
		REP   STOSD
	}
// 1. Get TOC in default LBA format
	*gASPI_CDBLen = 10; // Set command size
	gASPI_CDBByte[0] = SCSI_READ_TOC; // READ TOC Command
	//gASPI_CDBByte[1] = 0x00;   // Default 00 TOC Format: Address in LBA form (MSF bit zero)
	gASPI_CDBByte[7] = (BYTE)(sizeof(TOC_FORMAT_00) >> 8); // Should always be 804 bytes for default TOC format!!
	gASPI_CDBByte[8] = (BYTE)sizeof(TOC_FORMAT_00);      // Low word of size
	if ( !ExecuteSCSICmd2((PBYTE)lpTocLBA, sizeof(TOC_FORMAT_00)) )
		return 0;
// 2. Get TOC in MSF format
	gASPI_CDBByte[1] = 0x02; // Set MSF bit
	if ( !ExecuteSCSICmd2((LPBYTE)&tocMSF, sizeof(TOC_FORMAT_00)) )
		return 0;
	lpBuffer = (LPSTR)gAlignedSCSIBuffer;
	*lpBuffer = NULL;
	lpLBATrack = lpTocLBA->TrackData;
	lpMSFTrack = tocMSF.TrackData;
	retTotalTracks = TotalTracks = (lpTocLBA->LastTrack - lpTocLBA->FirstTrack) + 1;
// 3. Store TOC track line to buffer but reorder LBA bytes to proper Intel order (annoying) first...
	while ( TotalTracks ) { 
		MSB2DWORD2(lpLBATrack); // Fix byte order of LBA
		wsprintfA(
			lpBuffer,
			"Track %02lu %-5s %02lu:%02lu:%02lu LBA=%06lu"BRK,
			lpLBATrack->TrackNumber, IsAudioTrack(lpLBATrack->CONTROL)? "Audio" : "Data",
			(DWORD)lpMSFTrack->Address[1], (DWORD)lpMSFTrack->Address[2], (DWORD)lpMSFTrack->Address[3], lpLBATrack->LBAddress
		);
		__asm ADD lpBuffer, EAX
		lpMSFTrack++;
		lpLBATrack++;
		TotalTracks--;
	}
// 4. Verify the ending track is really the Leadout Track
	if ( lpLBATrack->TrackNumber == 0xAA ) { // Leadout Track is numbered 0xAA by MMC standards
		MSB2DWORD2(lpLBATrack); // Fix byte order of LBA
		wsprintfA(
			lpBuffer,
			"\r\nLeadout: %02lu:%02lu:%02lu\tLBA %06lu\r\n\r\n",
			(DWORD)lpMSFTrack->Address[1], (DWORD)lpMSFTrack->Address[2], (DWORD)lpMSFTrack->Address[3], lpLBATrack->LBAddress
		);
		__asm ADD lpBuffer, EAX
	} else {
		PrintLastSysError("TOC Error: The Lead-out Track is 0x%02X, not 0xAA as is proper", lpLBATrack->TrackNumber);
		return 0;
	}
	*lpBytesWritten = SubPtr(DWORD, lpBuffer, gAlignedSCSIBuffer);
	*lpBuffer = NULL;
	return retTotalTracks;
}

// This is needed to detect a multi-session CD (and thus process it appropriately)!
// (e.g. Audio CDs that carry the EnhancedCD logo and have an ISO data track
// on the 2nd session. This allows the CD to play normally in regular CD players
// that can't detect the 2nd session while giving the publisher the ability to
// include content such as images, videos, etc. that becomes accessible when the
// CD is loaded in your average Windows PC.)

BOOL __fastcall ASPIGetTOCFull(TOC_FORMAT_0010* lpFullTOC) {
// Zero Initialize TOC data/CDB
	__asm {
		CLD
		XOR   EAX, EAX
	// Zero out LBA TOC buffer
		MOV   EDI, lpFullTOC
		MOV   ECX, SIZE TOC_FORMAT_0010/4
		REP   STOSD
	// Zero 16 bytes of CDB
		MOV   EDI, gASPI_CDBByte
		MOV   ECX, 4
		REP   STOSD
	}
	*gASPI_CDBLen = 10; // Set command size
	gASPI_CDBByte[0] = SCSI_READ_TOC;  // READ TOC command
	gASPI_CDBByte[2] = 0x02; // Set Format bit to 0010 for Session Info TOC
	gASPI_CDBByte[7] = (BYTE)(sizeof(TOC_FORMAT_0010) >> 8); // High word of size
	gASPI_CDBByte[8] = (BYTE)sizeof(TOC_FORMAT_0010);      // Low word of size
	return ExecuteSCSICmd2((LPBYTE)lpFullTOC, sizeof(TOC_FORMAT_0010));
	//if ( SRB_Status == SS_COMP ) // Where did I get the math for this below length computation??? *shrugs* Urrmmm. No clue.
	//	*(LPWORD)lpFullTOC->Length = (((lpFullTOC->Length[0] << 8) | lpFullTOC->Length[1]) / sizeof(TRACK_DESCRIPTOR_0010)) & 0x007F;
	//return SRB_Status;
}

BOOL __fastcall ASPIGetTOCSession(TOC_FORMAT_0001* lpSessionTOC) {
// Zero Initialize TOC data/CDB
	__asm {
		CLD
		XOR   EAX, EAX
	// Zero out LBA TOC buffer
		MOV   EDI, lpSessionTOC
		MOV   ECX, SIZE TOC_FORMAT_0001/4
		REP   STOSD
	// Zero 16 bytes of CDB
		MOV   EDI, gASPI_CDBByte
		MOV   ECX, 4
		REP   STOSD
	}
	*gASPI_CDBLen = 10; // Set command size
	gASPI_CDBByte[0] = SCSI_READ_TOC;  // READ TOC command
	gASPI_CDBByte[2] = 0x01; // Set Format bit to 0001 for Session Info TOC
	gASPI_CDBByte[7] = (BYTE)(sizeof(TOC_FORMAT_0001) >> 8); // High word of size
	gASPI_CDBByte[8] = (BYTE)(sizeof(TOC_FORMAT_0001));      // Low word of size
	return ExecuteSCSICmd2((LPBYTE)lpSessionTOC, sizeof(TOC_FORMAT_0001));
	//if ( SRB_Status == SS_COMP ) // Where did I get the math for this below length computation??? *shrugs* Urrmmm. No clue.
	//	*(LPWORD)lpSessionTOC->Length = (((lpSessionTOC->Length[0] << 8) | lpSessionTOC->Length[1]));
	//return SRB_Status;
}

LPCDTEXTPACK __fastcall ASPIGetCDTEXT(PLONG OUT lpTextPackSize) {
	WORD retSize;
// Zero Initialize CDTEXT buffer/CDB
	__asm {
		CLD
		XOR   EAX, EAX
	// Zero out start of CDTEXT buffer
		MOV   EDI, gAlignedSCSIBuffer
		MOV   ECX, 24/4
		REP   STOSD
	// Zero 16 bytes of CDB
		MOV   EDI, gASPI_CDBByte
		MOV   ECX, 4
		REP   STOSD
	}
	*gASPI_CDBLen = 10; // Set command size
	gASPI_CDBByte[0] = SCSI_READ_TOC; // READ TOC command
// This format returns CD-TEXT information that is recorded in the
// Lead-in track as R-W Sub-Channel Data. Older drives will fail!
	gASPI_CDBByte[2] = 0x05; // Set Format bit for CD-TEXT
	gASPI_CDBByte[7] = (BYTE)(CDTEXT_BUFFER_SIZE >> 8); // High word of size
	gASPI_CDBByte[8] = (BYTE)(CDTEXT_BUFFER_SIZE);      // Low word of size
	if ( !ExecuteSCSICmd2(gAlignedSCSIBuffer, CDTEXT_BUFFER_SIZE) ) {
		*lpTextPackSize = -1;
		return NULL;
	}
	retSize = (gAlignedSCSIBuffer[0] << 8) | gAlignedSCSIBuffer[1];
	if ( retSize < 38 ) { // At least 2 CD-Text packs test [(2*18)+2] ?
		*lpTextPackSize = 0;
		return NULL;
	} else {
		retSize -= 2; // Subtract off 2 bytes of length WORD variable which gets counted
		*lpTextPackSize = (DWORD)retSize; // Text Pack bytes evenly divisible by 18
	}
	return ((LPCDTEXT)gAlignedSCSIBuffer)->CDTextPacks;
}

BOOL __fastcall ASPIAnalyzeTrackModes(LPTOC lpTocLBA, BYTE bForceRAWMode) {
	TRACK_DESCRIPTOR_00* lpTrack;
	BYTE DataTrackMode;
	DWORD LBA;
	lpTrack = lpTocLBA->TrackData;
	while ( lpTrack->TrackNumber <= lpTocLBA->LastTrack ) {
		if ( bGlobalAbort )
			return FALSE;
		if ( IsAudioTrack(lpTrack->CONTROL) ) { // Is track of type audio?
			lpTrack->Reserved1 = AUDIO_CDDA;
		} else {
			if ( !IsDataTrack(lpTrack->CONTROL) ) { // Is track of type data?
				PrintLastSysError("TOC Error: Track %02u: Type audio or data undetectable", lpTrack->TrackNumber);
				return FALSE;
			}
		// We know it's a data track for sure, but now we must detect what mode (01 or 02)
			LBA = lpTrack->LBAddress;
			gAlignedSCSIBuffer[0] = 0;

			//fpmemzero(gAlignedSCSIBuffer, 2352)
			//ASPIReadRAWSectors(16, 1);
			//fpmemzero(gAlignedSCSIBuffer, 2352)
			//ASPIReadDATASectors(780, 1);

		// Try #1: Get data mode of 1st sector of track
			ASPIReadSectorHeader(LBA);
			DataTrackMode = gAlignedSCSIBuffer[0]; // Byte 00 has data mode if ReadSectorHeader succeeds!
			if ( DataTrackMode == 0 ) { // Did ReadSectorHeader fail ?
				gAlignedSCSIBuffer[15] = 0;
			// Try #2: Read 1 sector raw to get data mode 2nd way
				ASPIReadRAWSectors(LBA, 1);
			// Byte 15 in header has data mode (Mode 01, Mode 02) if ReadRAW succeeds!
				DataTrackMode = gAlignedSCSIBuffer[15];
			}
			if ( DataTrackMode ) { // Should be 1 or 2 under normal data tracks!
				if ( DataTrackMode == 1 ) { // MODE1 Sector
					if ( !bForceRAWMode )
						lpTrack->Reserved1 = MODE1_2048;
					else
						lpTrack->Reserved1 = MODE1_2352;
				} else if ( DataTrackMode == 2 ) { // MODE2 Sector
					if ( bForceRAWMode )
						lpTrack->Reserved1 = MODE2_2352;
					else {

						lpTrack->Reserved1 = MODE2_2352; // MODE2_F0_2336; /// TEMPORARY

					}
				} else { // Unknown or illegal mode; Abort
					PrintLastSysError("Error: Data Track %02u: Undetectable or illegal mode (Mode: 0x%02X)", (UINT)lpTrack->TrackNumber, DataTrackMode);
					return FALSE;
				}
			} else if ( ASPIReadDATASectors(LBA, 1) ) {
				lpTrack->Reserved1 = MODE1_2048;
			} else {
				PrintLastSysError("Error: Data Track %02u: %s", lpTrack->TrackNumber, ASPIGetLastError("Unknown read failure"));
				return FALSE;
			}
		}
		lpTrack++;
	} 
	return TRUE;
}

BOOL __fastcall ASPIAnalyzeQSubCodes(LPMASTERTOC lpMasterTOC) {
	MASTER_TOC_TRACK_DESCRIPTOR *lpMasterTrack;
	SUBCHANNEL_SUBQ_DESCRIPTOR *lpSubQData;
	DWORD TrackCount;
	lpMasterTrack = lpMasterTOC->TrackData;
	lpSubQData = (SUBCHANNEL_SUBQ_DESCRIPTOR*)gAlignedSCSIBuffer;
	FastPrint("Notice: Performing 'Q' Sub-Channel Analysis... This may take a while..."BRK);
	TrackCount = lpMasterTOC->TotalTracks;
	while ( TrackCount > 0 ) {
		if ( bGlobalAbort )
			return FALSE;
		if ( ASPIReadQSubChannelData(lpMasterTrack->StartAddress, 1) != SS_COMP )
			return FALSE;
		//lpSubQData->ADR == ADR_ENCODES_MEDIA_CATALOG
		//lpSubQData->ADR == ADR_ENCODES_ISRC
	// ADR=1 (0001b) – Mode-1 Q
		if ( lpSubQData->ADR == ADR_ENCODES_CURRENT_POSITION && lpSubQData->TrackNumber != lpMasterTrack->TrackNumber ) {// Check if Q Subcode reported track# is equal to current track#
			printf("Invalid Q subcode value with track #%hu"BRK, lpMasterTrack->TrackNumber);
			//return FALSE;
		}
		printf("Track #: %.2x, ADR: %hu, START INDEX: %hu"BRK, lpSubQData->TrackNumber, lpSubQData->ADR, lpSubQData->IndexNumber);
		//printf("Track #: %hu, START INDEX: %hu\r\n", lpSubQData->TrackNumber, lpSubQData->IndexNumber);
		if ( ASPIReadQSubChannelData(lpMasterTrack->StopAddress-1, 1) == SS_COMP )
			printf("Track #: %.2x, ADR: %hu,  STOP INDEX: %hu"BRK, lpSubQData->TrackNumber, lpSubQData->ADR, lpSubQData->IndexNumber);
		/*
		if ( lpSubQData->IndexNumber != 1 )
			return FALSE;
		if ( ASPIReadQSubChannelData(lpMasterTrack->StopAddress-10, 10) == SS_COMP ) {
			MM = (lpMasterTrack->StopAddress-10) + 150;
			FF = MM % 75;
			MM /= 75;
			SS = MM % 60;
			MM /= 60;
			for ( i = 0; i < 10; i++ ) {
				if ( lpSubQData[i].TrackNumber == lpMasterTrack->TrackNumber )
					break;
			// Pregap detected! This sector belongs to the next track!
				else if ( lpSubQData[i].TrackNumber == lpMasterTrack->TrackNumber+1 && lpSubQData[i].IndexNumber == 0 ) 
					break;
			}
		}*/
		lpMasterTrack++;
		TrackCount--;
	}
	//return FALSE;
	return TRUE;
/*
	// Detect and enforce a track type transition condition to deal with PREGAPs!
	if ( lpMasterTrack->TrackNumber < MasterTOC.LastTrack && (lpMasterTrack->TrackMode > 0 != (lpTrack+1)->Reserved1 > 0) ) {
	// The next track is of type data, so a transition from audio to data will
	// take place, so _this_ audio track's stop LBA = LBA minus 03:00 (225 sectors).
		if ( bIsAudio )
			lpMasterTrack->StopAddress -= 225;
	// The next track is of type audio, so a transition from data to audio will
	// take place, so _this_ data track's stop LBA = LBA minus 02:00 (150 sectors).
		else
			lpMasterTrack->StopAddress -= 150;
	}*/
}

// 14.2.8 READ CD-ROM CAPACITY command
// Provides a means for the initiator to request  
// info regarding the capacity of the logical unit.
// 8 bytes of READ CD-ROM CAPACITY data shall be 
// sent during the DATA IN phase of the command

DWORD __fastcall ASPIReadCDROMCapacity() {
// Zero Initialize buffer/CDB
	__asm {
		CLD
		XOR   EAX, EAX
	// Zero out buffer
		MOV   EDI, gAlignedSCSIBuffer
		MOV   ECX, 2
		REP   STOSD
	// Zero 16 bytes of CDB
		MOV   EDI, gASPI_CDBByte
		MOV   ECX, 4
		REP   STOSD
	}
	*gASPI_CDBLen = 10; // Set command size
	gASPI_CDBByte[0] = SCSI_READCDCAP;
	if ( ExecuteSCSICmd2(gAlignedSCSIBuffer, 8) ) {
		MSB2DWORD(gAlignedSCSIBuffer);
		return *(LPDWORD)gAlignedSCSIBuffer;
	}
	return 0;
}

// File: "SCSI-2 - 1993.pdf", page 104 (140/502 PDF page)
// 8.2.5 INQUIRY 6-byte command
// page 159, Table 107 - Unit serial number page (195/502 PDF page)

BYTE __fastcall ASPISerialInquiry() {
// Zero Initialize Inquiry buffer/CDB
	__asm {
		CLD
		XOR   EAX, EAX
	// Zero out Inquiry buffer
		MOV   EDI, gAlignedSCSIBuffer
		MOV   ECX, 96/4
		REP   STOSD
	// Zero 16 bytes of CDB
		MOV   EDI, gASPI_CDBByte
		MOV   ECX, 4
		REP   STOSD
	}
	*gASPI_CDBLen = 6; // Set command size
	gASPI_CDBByte[0] = SCSI_INQUIRY;
	gASPI_CDBByte[1] = 0x01; // Set EVPD flag - enable vital product data
	gASPI_CDBByte[2] = 0x80; // Set to UnitSerialNumber Page Code
// Serial inquiry = 96 bytes
	gASPI_CDBByte[4] = 96; // Set allocation length
	if ( ExecuteSCSICmd2(gAlignedSCSIBuffer, 96) )
		return gAlignedSCSIBuffer[3];
	return 0;
}

// File: "SCSI-2 - 1993.pdf", page 104 (140/502 PDF page)
// 8.2.5 INQUIRY 6-byte command
// The standard INQUIRY data (table 45) contains 36 required bytes, followed by a
// variable number of vendor-specific parameters. Bytes 56 through 95, if returned,
// are reserved for future standardization.

BOOL __fastcall ASPIStandardInquiry() {
	BOOLEAN bResult;
// Zero Initialize Inquiry buffer/CDB
	__asm {
		CLD
		XOR   EAX, EAX
	// Zero out Inquiry buffer
		MOV   EDI, gAlignedSCSIBuffer
		MOV   ECX, 36/4
		REP   STOSD
	// Zero 16 bytes of CDB
		MOV   EDI, gASPI_CDBByte
		MOV   ECX, 4
		REP   STOSD
	}
	*gASPI_CDBLen = 6; // Set command size
	gASPI_CDBByte[0] = SCSI_INQUIRY;
	//gASPI_CDBByte[2] // CodePage = 0x00 (Supported vital product data page)
// Standard inquiry Allocation Length must be 36 bytes! Must be 36 EXACT or
// CDROM drives can lockup!!! This also allows a Daemon Tools drive to be
// properly queried with other ASPI layers, otherwise you get a time out error!
	gASPI_CDBByte[4] = 36;
	bResult = ExecuteSCSICmd2(gAlignedSCSIBuffer, 36);
	gAlignedSCSIBuffer[36] = NULL;
	return bResult;
}

BOOL __fastcall ASPIIsUnitReady() {
	mMemZeroBy4(gASPI_CDBByte, 4); // Zero 16 bytes of CDB
	*gASPI_CDBLen = 6; // Set command size
	return ExecuteSCSICmd2(NULL, 0);
}

BOOL __fastcall ASPILoadTrayReadyUnit() {
	#define SLEEP_TIME 500
	BYTE ReTries = 30; // Retries = 30 * 500ms = 15,000ms = 15seconds
// Test if drive's ready/loaded
	if ( ASPIIsUnitReady() )
		return TRUE;
// Status/error: "Medium not present - tray closed"
	if ( gSenseKey == KEY_NOTREADY && gAddSenseCode == 0x3A && gAddSenQual == 0x01 )
		return FALSE; // No disc, nothing to do, return!
// Likely status/error: "Medium not present - tray open"
	ASPIStartStopUnit(); // Try to load/close the tray, spin up disc IF present
	Sleep(1500);
// Wait ReTries * SLEEP_TIME = # Seconds for the drive to become ready!
	while ( !ASPIIsUnitReady() && ReTries-- > 0 ) {
	// ** Sleep for Sense Status/Errors **
	// (1) "Device is in the process of becoming ready"
	// (2) "Not ready to ready change, medium may have changed"
		if ( (gSenseKey == KEY_NOTREADY && gAddSenseCode == 4 && gAddSenQual == 1) || (gSenseKey == KEY_UNITATT && gAddSenseCode == 0x28 && gAddSenQual == 0) )
			Sleep(SLEEP_TIME);  // Retries * SLEEP_TIME = Max Seconds
	// "Medium not present - tray closed OR Open" (USB drives CAN'T be closed with LoadTray Cmd!)
		else if ( gSenseKey == KEY_NOTREADY && gAddSenseCode == 0x3A )
			return FALSE;
		if ( bGlobalAbort )
			return FALSE;
	}
	return ASPIIsUnitReady();
}

// 1:0:0 - REZERO UNIT
// CDB Contents: 01h 00h 00h 00h 00h 00h 
// ASPI Status : 01h 00h 00h 00h 00h 00h

BOOL __fastcall ASPIReZeroUnit() {
	BYTE bResult;
	mMemZeroBy4(gASPI_CDBByte, 4); // Zero 16 bytes of CDB
	*gASPI_CDBLen = 6; // Set command size
	gASPI_CDBByte[0] = SCSI_REZERO;
	bResult = ExecuteSCSICmd2(NULL, 0);
	Sleep(100);
	return bResult;
}

void __fastcall ASPIResetUnit() {
	#define SLEEP_TIME 500
	BYTE ReTries = 30; // Retries = 30 * 500ms = 15,000ms = 15seconds
// Zero 16 bytes of CDB
	mMemZeroBy4(gASPI_CDBByte, 4); 
// Execute STOP unit and then START unit
	*gASPI_CDBLen = 6; // Set command size
	gASPI_CDBByte[0] = SCSI_START_STP;
	ExecuteSCSICmd2(NULL, 0); // Set to "stop" by default
// Bit: |        7-4       |   3-2    |  1   |   0   |
// [1]  | Power Conditions | Reserved | LOEJ | START |
	gASPI_CDBByte[4] = 0x01; // "Start" set to 1.
	ExecuteSCSICmd2(NULL, 0);
	ASPIReZeroUnit();
// Wait ReTries * SLEEP_TIME = # Seconds for the drive to become ready!
	do {
		Sleep(SLEEP_TIME); // Retries * SLEEP_TIME = Max Seconds
		if ( bGlobalAbort )
			return;
	} while ( !ASPIIsUnitReady() && ReTries-- > 0 );
}

// 1:1:0 - START STOP UNIT
// CDB Contents: 1Bh 00h 00h 00h 03h 00h 
// ASPI Status : 01h 00h 00h 00h 00h 00h
// This also closes the CD tray if it's open!

BOOL __fastcall ASPIStartStopUnit() {
	mMemZeroBy4(gASPI_CDBByte, 4); // Zero 16 bytes of CDB
	*gASPI_CDBLen = 6; // Set command size
	gASPI_CDBByte[0] = SCSI_START_STP;
// Bit: |        7-4       |   3-2    |  1   |   0   |
// [1]  | Power Conditions | Reserved | LOEJ | START |
	gASPI_CDBByte[4] = 0x03; // "Load Eject" and "Start" set to 1. (Load tray, spin up disc, turn on laser and servo system.)
	return ExecuteSCSICmd2(NULL, 0);
}

// File: "SCSI2 - 1993.pdf", page 198
// 9.2.17 START STOP UNIT command

BOOL __fastcall ASPIEjectDiscTray() {
	mMemZeroBy4(gASPI_CDBByte, 4); // Zero 16 bytes of CDB
	*gASPI_CDBLen = 6; // Set command size
	gASPI_CDBByte[0] = SCSI_START_STP;
// Bit: |        7-4       |   3-2    |  1   |   0   |
// [1]  | Power Conditions | Reserved | LOEJ | START |
	gASPI_CDBByte[4] = 0x02; // Set to eject tray
	return ExecuteSCSICmd2(NULL, 0);
}

DWORD __fastcall ASPIGetCDSpeed() {
	BYTE btBuffer[sizeof(SCSICDMODEPAGE_2A)+8];
	SCSICDMODEPAGE_2A *pPage;
	MemZero(sizeof(btBuffer), &btBuffer);
	pPage = (SCSICDMODEPAGE_2A*)ASPIModeSense10(0x2A, btBuffer, sizeof(btBuffer));
	if ( pPage )
		return (pPage->cur_read_speed[0] << 8) | pPage->cur_read_speed[1];
	return 0;
}

BOOL __fastcall ASPISetCDSpeed(WORD nSpeed) {
	mMemZeroBy4(gASPI_CDBByte, 4); // Zero 16 bytes of CDB
	*gASPI_CDBLen = 12; // Set command size
	if ( nSpeed != 0xFFFF )
		nSpeed = (WORD)round(nSpeed * XX1);
	gASPI_CDBByte[0] = MMC1_SET_SPEED; // SETSPEED Operation Code
	gASPI_CDBByte[2] = (BYTE)(nSpeed >> 8); // High word of speed
	gASPI_CDBByte[3] = (BYTE)(nSpeed);      // Low word of speed
	gASPI_CDBByte[4] = 0xFF; // Set Write speed to Max as apparently is required...
	gASPI_CDBByte[5] = 0xFF;
	return ExecuteSCSICmd2(NULL, 0);
}

} // end extern "C"

// 1:1:0 - READ CD
// Example: Reading audio sectors 1024 to 1024 (1 blocks) with CDRWIN
// CDB Contents: BEh 04h 00h 00h 00h 01h 00h 00h 01h 10h 00h 00h 
// ASPI Status : 01h 00h 00h 00h 00h 00h
// Reference: Page 45 of mmc-r01.pdf (SCSI-3 Multi-Media Commands (MMC) 1994)

RAW_READ_INFO rawCDDARead = {0, 0, 0, CDDA};
BOOLEAN bTryASPI = TRUE;
BOOLEAN bASPIGood;

__forceinline BOOL __fastcall ASPIReadCDDASectors(DWORD LBA, DWORD nSectors) {
// *********************************************************************************
// Try ASPI first for speed (given XP) or if Win95/98/ME or if forced by /useaspi parameter
// WILL fail on some Windows 7 (works on latest SP1) given security blocks!
// *********************************************************************************
	if ( bTryASPI ) {
		*gASPI_CDBLen = 12; // Set command size
		__asm MOV EBX, gASPI_CDBByte
		mMemZeroBy4(EBX, 4); // Zero 16 bytes of CDB
		__asm {
		;// gASPI_CDBByte[0] = MMC1_READCD;
			MOV   BYTE PTR [EBX], MMC1_READCD ;// READ CD MMC Operation Code
		;// Bit: |   7-5    |     4-2     |    1     |   0    |
		;// [1]  | Reserved | Sector Type | Reserved | RELADR |
			MOV   BYTE PTR [EBX+1], 0x04 ;// Filter for Red Book (CD-DA) sectors only
		;// Load Starting Logical Block Address
		;// gASPI_CDBByte[2] = (BYTE)(LBA >> 24); // MSB	
		;// gASPI_CDBByte[3] = (BYTE)(LBA >> 16);
		;// gASPI_CDBByte[4] = (BYTE)(LBA >>  8);
		;// gASPI_CDBByte[5] = (BYTE)(LBA);       // LSB
			MOV   EAX, LBA
			BSWAP EAX          ;// Convert from little-endian to big-endian!!! Very useful!
			MOV   [EBX+2], EAX ;// Reduced to ONE 32-bit MOV thanks to BSWAP!!!			
		;// Number of sectors to read (gASPI_CDBByte[6-8]) - we'll only ever care about gASPI_CDBByte[8]
		;// gASPI_CDBByte[8] = nSectors;
			MOVZX EAX, BYTE PTR [nSectors] ;// 1,2,3...27, NEVER above so 2KB to 64KB max!
			MOV   BYTE PTR [EBX+8], AL ;// (LSB) 1-27 to keep at or below 64KB for max SCSI compatibility!
		;// Bit: |      7      |    6    5    |     4     |     3     |   2    1    |    0     |
		;// [9]  | Synch Field | Header Codes | User Data | EDC & ECC | Error Flags | Reserved |
		;// Return ALL and ONLY 2352 bytes - Must be 0xF8 or won't work on a Goldstar drive! Other
		;// coders are wrong! 0xF8 is THE right value!!! 0xFF to return error flags would be >2352++ bytes
			MOV   BYTE PTR [EBX+9], 0xF8
		}
		if ( ExecuteSCSICmd1() ) {
			bASPIGood = TRUE; // Raw read ASPI worked at least once, so note it! (Not Windows 7)
			return TRUE;
		}
		if ( !bASPIGood ) // We're likely in Windows 7 & direct SCSI command packets for reading are illegal!
			bTryASPI = FALSE; // Don't use this ASPI method again!!!
	}
// *********************************************************************************
// Use SPTI if we're in NT/2K/XP/VISTA/7/8/10 if ASPI method above failed!
// Note: The only way to read raw sectors on Windows7 given security changes (which didn't last Win8-10)
// Note: This method might be much slower than ASPI/SCSI pass-through on WinXP/etc.
// *********************************************************************************
	if ( ghCDRom ) {
	// Must use general CD-ROM data sector size of 2048, and NOT 2352 as one would expect
	// while buffer must be able to hold the raw size, 2352 * nSectors, as you *WOULD* expect!
		rawCDDARead.DiskOffset.LowPart = (LBA * CDROM_SECTOR_SIZE);
		*(LPBYTE)&rawCDDARead.SectorCount = (BYTE)nSectors;
	// Call DeviceIoControl and trap BOTH possible errors: a return value of
	// FALSE or the NumberOfBytes count returned not matching expectations!
		if ( DeviceIoControl(ghCDRom, IOCTL_CDROM_RAW_READ, &rawCDDARead, sizeof(RAW_READ_INFO), gAlignedSCSIBuffer, SCSI_BUFFER_LIMIT, &gnNumberOfBytes, NULL) )
			if (
				gnNumberOfBytes == (SECTOR_READ_CHUNKS * RAW_SECTOR_SIZE) // 99% this is true, use hardcoded value, no multiplying!
						||
				gnNumberOfBytes == (nSectors * RAW_SECTOR_SIZE) // Save the runtime multiplying when reading final sectors!
			)
				return TRUE;
	}
	return FALSE;
}

// 1:1:0 - READ 10
// CDB Contents: 28h 00h 00h 00h 0Fh 32h 00h 00h 01h 00h 
// ASPI Status : 01h 00h 00h 00h 00h 00h
// Example: Reading cooked (2048) data sectors 3890 to 3890 (1 block) with CDRWIN

BOOLEAN bTryREAD10 = TRUE; // Try the normal ASPI READ10 method before trying SPTI
BOOLEAN bREAD10Good;

BOOLEAN bTryMMCREAD = TRUE;
BOOLEAN bMMCREADGood;

BOOLEAN bTryReadFile = TRUE;
BOOLEAN bReadFileGood;

// Initialize GUID test. We write 128-bit GUID to buffer, if CD device doesn't overwrite
// ANY bit, we trap the read failure! Varying devices and Windows 7 will fail depending
// on SCSI read commands between SCSI-1 0x28 (READ10), SCSI-3/MMC 0xBE (READCD/RAW) or
// the pure SPTI IOCTL method with ReadFile() where Windows chooses what opcode to use.
void __fastcall ASPIInitGUIDTest() {
	__asm {
		MOV ECX, SIZE gGUID/4
		LEA ESI, gGUID
		MOV EDI, gAlignedSCSIBuffer
REPEAT_LOOP:
		MOV EAX, DWORD PTR [ESI]
		MOV DWORD PTR [EDI], EAX
		ADD ESI, 4
		ADD EDI, 4
		DEC ECX
		JNZ REPEAT_LOOP
	}
}
// Run this AFTER a read attempt was made to the first sector of a CD device
// assuming you executed the ASPIInitGUIDTest() function to prepare the test.
// If any bit changed, we return true to indicate the test succeeded, the CD
// device wrote new data to the buffer! If no bits changed of the 128-bit GUID,
// the read failed, the SCSI opcode was rejected or Windows 7 security, etc...
BOOLEAN __fastcall ASPIRunGUIDTest() {
	__asm {
		MOV ECX, SIZE gGUID/4
		LEA ESI, gGUID
		MOV EDI, gAlignedSCSIBuffer
REPEAT_LOOP:
		MOV EAX, DWORD PTR [ESI]
		CMP EAX, DWORD PTR [EDI]
		JNE RETURN_TRUE
		ADD ESI, 4
		ADD EDI, 4
		DEC ECX
		JNZ REPEAT_LOOP
	}
	return FALSE;
RETURN_TRUE:
	return TRUE;
}

__forceinline BOOL __fastcall ASPIReadDATASectors(DWORD LBA, DWORD nSectors) {
	DWORD dwAbsBytesToRead;
// *********************************************************************************
// 1) Try ASPI READ10 first on all Windows: Win95/98/ME/NT/2K/XP/VISTA/7/8/10++
// *********************************************************************************
	if ( bTryREAD10 ) {
		*gASPI_CDBLen = 10; // Set command size
		__asm MOV EBX, gASPI_CDBByte
		mMemZeroBy4(EBX, 4); // Zero 16 bytes of CDB
		__asm {
		;// gASPI_CDBByte[0] = SCSI_READ10;
			MOV BYTE PTR [EBX], SCSI_READ10 ;// Set READ10 SCSI command
		;// Load Starting Logical Block Address
		;// gASPI_CDBByte[2] = (BYTE)(LBA >> 24); // MSB	
		;// gASPI_CDBByte[3] = (BYTE)(LBA >> 16);
		;// gASPI_CDBByte[4] = (BYTE)(LBA >>  8);
		;// gASPI_CDBByte[5] = (BYTE)(LBA);       // LSB
			MOV   EAX, LBA
			BSWAP EAX          ;// Convert from little-endian to big-endian!!! Very useful!
			MOV   [EBX+2], EAX ;// Reduced to ONE 32-bit MOV thanks to BSWAP!!!			
		;// Number of sectors to read (gASPI_CDBByte[6-8]) - we'll only ever care about gASPI_CDBByte[8]
		;// gASPI_CDBByte[8] = nSectors;
			MOVZX EAX, BYTE PTR [nSectors]
			MOV   BYTE PTR [EBX+8], AL ;// (LSB) - Should be <= 27 to keep at or below 64KB for maximum compatibility
		}
		if ( !bREAD10Good ) // 1 test with 1st data sector, will the buffer get overwritten?
			ASPIInitGUIDTest();
		if ( ExecuteSCSICmd1() ) {
			if ( !bREAD10Good ) {
				if ( ASPIRunGUIDTest() ) {
					bREAD10Good = TRUE; // This read worked at least once, so note it!
					return TRUE;
				} else     // We're likely in Windows 7 & direct SCSI command packets for reading are illegal!
					bTryREAD10 = FALSE; // Don't use this ASPI method again!!!
			} else
				return TRUE;
		}
	}
// *********************************************************************************
// 2) Try the MMC READCD Method next, this appears to work for certain Windows 7
// PCs when READ10 fails...
// *********************************************************************************
	if ( bTryMMCREAD ) {
		*gASPI_CDBLen = 12; // Set command size
		__asm MOV EBX, gASPI_CDBByte
		mMemZeroBy4(EBX, 4); // Zero 16 bytes of CDB
		__asm {
		;// gASPI_CDBByte[0] = MMC1_READCD;
			MOV   BYTE PTR [EBX], MMC1_READCD ;// READ CD MMC Operation Code
		;// Load Starting Logical Block Address
		;// gASPI_CDBByte[2] = (BYTE)(LBA >> 24); // MSB	
		;// gASPI_CDBByte[3] = (BYTE)(LBA >> 16);
		;// gASPI_CDBByte[4] = (BYTE)(LBA >>  8);
		;// gASPI_CDBByte[5] = (BYTE)(LBA);       // LSB
			MOV   EAX, LBA
			BSWAP EAX          ;// Convert from little-endian to big-endian!!! Very useful!
			MOV   [EBX+2], EAX ;// Reduced to ONE 32-bit MOV thanks to BSWAP!!!
		;// Number of sectors to read (gASPI_CDBByte[6-8]) - we'll only ever care about gASPI_CDBByte[8]
		;// gASPI_CDBByte[8] = (BYTE)nSectors;
			MOVZX EAX, BYTE PTR [nSectors] ;// 1,2,3...27, NEVER above so 2KB to 64KB max!
			MOV   BYTE PTR [EBX+8], AL ;// (LSB) 1-27 to keep at or below 64KB for max SCSI compatibility!
		;// Bit: |      7      |    6    5    |     4     |     3     |   2    1    |    0     |
		;// [9]  | Synch Field | Header Codes | User Data | EDC & ECC | Error Flags | Reserved |
		;// Return only userdata, just like READ10, this might work on Windows 7
		;// gASPI_CDBByte[9] = 0x10;
			MOV   BYTE PTR [EBX+9], 0x10 ;// Return: Userdata
		}
		if ( !bMMCREADGood ) // 1 test with 1st data sector, will the buffer get overwritten?
			ASPIInitGUIDTest();
		if ( ExecuteSCSICmd1() ) {
			if ( !bMMCREADGood ) {
				if ( ASPIRunGUIDTest() ) {
					bMMCREADGood = TRUE; // This read worked at least once, so note it!
					return TRUE;
				} else     // We're likely in Windows 7 & direct SCSI command packets for reading are illegal!
					bTryMMCREAD = FALSE; // Don't use this ASPI method again!!!
			} else
				return TRUE;
		}
	}
// *********************************************************************************
// 3) Use SPTI if we're in NT/2K/XP/VISTA/7/8/10 if ASPI methods above failed!
// ReadFile() fails on NT/2K/XP/VISTA [7] with data sectors not using a ISO9660 file system...
// BUT it succeeds on SOME Windows 7 builds and forward (8/10+) - CDROM driver was updated!!
// (ReadFile() fails on my new Windows 7 SP1 Pro build, so new info... It's not trustworthy.)
// *********************************************************************************
	if ( bTryReadFile && ghCDRom ) {
		if ( (BYTE)nSectors == SECTOR_READ_CHUNKS )
			dwAbsBytesToRead = SECTOR_READ_CHUNKS * CDROM_SECTOR_SIZE;
		else
			dwAbsBytesToRead = nSectors * CDROM_SECTOR_SIZE;
	// Must use standard CDROM data sector size of 2048, and not 2352 as one would expect
	// while buffer must be able to hold the raw size, 2352 * nSectors, as you *would* expect!
		SetFilePointer(ghCDRom, LBA * CDROM_SECTOR_SIZE, NULL, FILE_BEGIN);
	// The ReadFile() cooked data read method ONLY works on CDs with ISO9660 file systems
	// on NT/2K/XP/VISTA - it will succeed on Windows ~7/8/10 and forward as my testing found!!!!
	// This FAILS on PC Engine MODE1 CD sectors that have NO file system below Windows 7/8/10...
		if ( !bReadFileGood ) // 1 test with 1st data sector, will the buffer get overwritten?
			ASPIInitGUIDTest();
		if (
			ReadFile(ghCDRom, gAlignedSCSIBuffer, dwAbsBytesToRead, &gnNumberOfBytes, NULL)
				&&
			gnNumberOfBytes == dwAbsBytesToRead
		) {
			if ( !bReadFileGood ) // Test if buffer was overwritten!!!
				if ( ASPIRunGUIDTest() )
					bReadFileGood = TRUE;
				else {
					bTryReadFile = FALSE;
					return FALSE;
				}
			return TRUE;
		}
	}
	return FALSE;
}

// 1:1:0 - READ CD
// CDB Contents: BEh 00h 00h 00h 0Fh 32h 00h 00h 01h F8h 00h 00h 
// ASPI Status : 01h 00h 00h 00h 00h 00h
// Example: Reading raw data sectors 3890 to 3890 (1 block) with CDRWIN

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// NEEDS WIN7 FIX!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

__forceinline BOOL __fastcall ASPIReadRAWSectors(DWORD LBA, DWORD nSectors) {
	*gASPI_CDBLen = 12; // Set command size
	__asm MOV EBX, gASPI_CDBByte
	mMemZeroBy4(EBX, 4); // Zero 16 bytes of CDB
	__asm {
	;// gASPI_CDBByte[0] = MMC1_READCD;
		MOV   BYTE PTR [EBX], MMC1_READCD ;// READ CD MMC Operation Code
	;// Load Starting Logical Block Address
	;// gASPI_CDBByte[2] = (BYTE)(LBA >> 24); // MSB	
	;// gASPI_CDBByte[3] = (BYTE)(LBA >> 16);
	;// gASPI_CDBByte[4] = (BYTE)(LBA >>  8);
	;// gASPI_CDBByte[5] = (BYTE)(LBA);       // LSB
		MOV   EAX, LBA
		BSWAP EAX          ;// Convert from little-endian to big-endian!!! Very useful!
		MOV   [EBX+2], EAX ;// Reduced to ONE 32-bit MOV thanks to BSWAP!!!
	;// Number of sectors to read (gASPI_CDBByte[6-8]) - we'll only ever care about gASPI_CDBByte[8]
	;// gASPI_CDBByte[8] = (BYTE)nSectors;
		MOVZX EAX, BYTE PTR [nSectors] ;// 1,2,3...27, NEVER above so 2KB to 64KB max!
		MOV   BYTE PTR [EBX+8], AL ;// (LSB) 1-27 to keep at or below 64KB for max SCSI compatibility!
	;// Bit: |      7      |    6    5    |     4     |     3     |   2    1    |    0     |
	;// [9]  | Synch Field | Header Codes | User Data | EDC & ECC | Error Flags | Reserved |
	;// Return ALL and ONLY 2352 bytes - Must be 0xF8 or won't work on a Goldstar drive! Other
	;// coders are wrong! 0xF8 is THE right value!!! 0xFF to return error flags would be >2352++ bytes
	;// gASPI_CDBByte[9] = 0xF8;
		MOV   BYTE PTR [EBX+9], 0xF8 ;// Return: Synch, Header, Userdata, EDC & ECC
	}
	return ExecuteSCSICmd1();
}

extern "C" {

/// WORK IN PROGRESS Q SUB CHAN PROB!

BOOL __fastcall ASPIReadQSubChannelData(DWORD LBA, DWORD nSectors) {
	*gASPI_CDBLen = 12; // Set command size
	__asm MOV EBX, gASPI_CDBByte
	mMemZeroBy4(EBX, 4); // Zero 16 bytes of CDB
	__asm MOVZX ECX, BYTE PTR nSectors
	__asm SHL   ECX, 2              // Zero nSectors * 16 = nSectors * 4
	mMemZeroBy4(gAlignedSCSIBuffer, ECX); // EQUIV = nSectors * 16 (sizeof(SUBCHANNEL_SUBQ_DESCRIPTOR))
	__asm {
	;//	gASPI_CDBByte[0] = MMC1_READCD; // READCD MMC Operation Code
		MOV   BYTE PTR [EBX], MMC1_READCD ;// READ CD MMC Operation Code
	;// Load Starting Logical Block Address
	;// gASPI_CDBByte[2] = (BYTE)(LBA >> 24); // MSB	
	;// gASPI_CDBByte[3] = (BYTE)(LBA >> 16);
	;// gASPI_CDBByte[4] = (BYTE)(LBA >>  8);
	;// gASPI_CDBByte[5] = (BYTE)(LBA);       // LSB
		MOV   EAX, LBA
		BSWAP EAX          ;// Convert from little-endian to big-endian!!! Very useful!
		MOV   [EBX+2], EAX ;// Reduced to ONE 32-bit MOV thanks to BSWAP!!!
	;// Transfer Length in blocks (CDBByte[6-8])
	;//	gASPI_CDBByte[8] = nSectors; 
		MOVZX EAX, BYTE PTR [nSectors]
		MOV   BYTE PTR [EBX+8], AL     ;// Number of sectors to read (LSB)
	;//	gASPI_CDBByte[10] = RET_SUBCHANNEL_Q_DATA;
		MOV   BYTE PTR [EBX+10], RET_SUBCHANNEL_Q_DATA ;// Set to return Q subchannel data
	}
	return ExecuteSCSICmd1();
}

BOOL __fastcall ASPIReadSectorHeader(DWORD LBA) {
// Zero Initialize buffer/CDB
	__asm {
		CLD
		XOR   EAX, EAX
	// Zero out buffer
		MOV   EDI, gAlignedSCSIBuffer
		MOV   ECX, 2
		REP   STOSD
	// Zero 16 bytes of CDB
		MOV   EDI, gASPI_CDBByte
		MOV   ECX, 4
		REP   STOSD
	}
	*gASPI_CDBLen = 10; // Set command size
	gASPI_CDBByte[0] = SCSI_READHEADER;
	gASPI_CDBByte[2] = (BYTE)(LBA >> 24);
	gASPI_CDBByte[3] = (BYTE)(LBA >> 16);
	gASPI_CDBByte[4] = (BYTE)(LBA >>  8);
	gASPI_CDBByte[5] = (BYTE)(LBA);
	gASPI_CDBByte[8] = 8;
	return ExecuteSCSICmd2(gAlignedSCSIBuffer, 8);
}

BOOL __fastcall ASPIStart(BOOL bForceASPI) {
	WORD wASPIStatus;
// We appear to need HIGH priority in Windows XP when dealing with Firewire/USB
// connected CD/DVD devices. Otherwise, taskswitching to other applications while
// this is running will cause read failures...
	SetHighPriority(); // WinXP Bugfix, so might as well *always* go HIGH priority!
// TRUE = Windows NT/2K/XP/Vista/7/8/10, FALSE = Windows 95/98/Me/Win32
	bUseSPTI = gOSInfo.IsWinNT;
	if ( bUseSPTI ) {
		//if ( gOSInfo.Is64bit )
		//	Use64RegView();
		//SetKeyValueSTR(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", "AllocateCDRoms", "1", 2);
	}
	if ( bUseSPTI && !bForceASPI ) {
		FastPrint("Notice: Using Native Microsoft NT SCSI Library..."BRK);
		//ExecuteSCSICmd1 = &SPTI_ExecSCSICmd1; // Initialized to this by default
		//ExecuteSCSICmd2 = &SPTI_ExecSCSICmd2; // Initialized to this by default
		//*(PBYTE)&gSPTI_Exec.sptd.TimeOutValue = DEF_TIMEOUT;
		//*(PBYTE)&gSPTI_Exec.sptd.Length = sizeof(gSPTI_Exec.sptd);
		//*(PBYTE)&gSPTI_Exec.sptd.SenseInfoOffset = sizeof(gSPTI_Exec.sptd);
		gASPI_CDBByte = gSPTI_Exec.sptd.CDBByte;
		gASPI_CDBLen = &gSPTI_Exec.sptd.CdbLength;
	} else {
		bUseSPTI = FALSE;
	// 1) Load ASPI WNASPI32.DLL
		FastPrint("Notice: Initializing System ASPI32 Manager..."BRK);
		ghinstWNASPI32 = LoadLibrary(WNASPI32_DLL);
		if( !ghinstWNASPI32 ) {
			PrintLastSysError("Error: Loading ASPI32 Manager (%s) failed", WNASPI32_DLL);
			return FALSE;
		}
	// 2) Load the address of SendASPI32Command
		SendASPI32Command = (pfnSendASPI32Command) GetProcAddress(ghinstWNASPI32, "SendASPI32Command");
	// 3) Load the address of GetASPI32SupportInfo
		GetASPI32SupportInfo = (pfnGetASPI32SupportInfo) GetProcAddress(ghinstWNASPI32, "GetASPI32SupportInfo");
		if ( !(SendASPI32Command && GetASPI32SupportInfo) ) {
			PrintLastSysError("Error: Cannot load ASPI32 interface");
			ASPIStop();
			return FALSE;
		}
	// 4) Check ASPI Status = SS_OK && HostAdapters > 1
		wASPIStatus = (WORD)GetASPI32SupportInfo();
		gNumAdapters = LOBYTE(wASPIStatus);
		wASPIStatus = ( gNumAdapters )? HIBYTE(wASPIStatus) : SS_NO_ADAPTERS;
		if ( wASPIStatus == SS_OK ) {
		// ASPI support OK!
			if ( gNumAdapters > MAX_NUM_HA )
				gNumAdapters = MAX_NUM_HA;
		// Create the event object for waiting
			ghEventSRB = CreateEventA(NULL, TRUE, FALSE, NULL);
			if ( !ghEventSRB ) {
				PrintLastSysError("Error: Cannot create event object");
				ASPIStop();
				return FALSE;
			}
		// 5) Full ASPI success with at least 1 HostAdapter!!
		// Universal ASPI functions point to ASPI methods, not SPTI
			ExecuteSCSICmd1 = &ASPI_ExecSCSICmd1;
			ExecuteSCSICmd2 = &ASPI_ExecSCSICmd2;
			gASPI_Exec.SRB_PostProc = ghEventSRB;
			gASPI_Exec.SRB_Cmd = SC_EXEC_SCSI_CMD;
			gASPI_CDBLen = &gASPI_Exec.SRB_CDBLen;
			gASPI_CDBByte = gASPI_Exec.CDBByte;
			ASPIHostAdapterInquiry();
			printf("Notice: ASPI32 Manager (%s) loaded (#SCSI Hosts: %hu)..."BRK, WNASPI32_DLL, gNumAdapters);
			if ( *gSRBHAInquiry.HA_ManagerId )
				printf("Notice: Host ID: \"%s\", Manager ID: \"%s\""BRK, gSRBHAInquiry.HA_Identifier, gSRBHAInquiry.HA_ManagerId);
		} else {
			gASPIStatus = (BYTE)wASPIStatus;
			PrintLastSysError("Error: %s", ASPIGetLastError("ASPI32 for Windows unavailable"));
			ASPIStop();
			return FALSE;
		}
	}
	SetLastError(0);
	return TRUE;
}

void __fastcall ASPIStop() {
	if ( ghCDRom ) {
		if ( ghCDRom != INVALID_HANDLE_VALUE )
			CloseHandle(ghCDRom);
		ghCDRom = NULL;
	}
	if ( ghEventSRB ) {
		ResetEvent(ghEventSRB);
		CloseHandle(ghEventSRB);
		ghEventSRB = NULL;
	}
	if ( ghinstWNASPI32 ) {
		FreeLibrary(ghinstWNASPI32);
		ghinstWNASPI32 = NULL;
	}
}

DWORD __cdecl ASPIPrintLastError(LPCSTR lpFormattedMessage, ...) {
	return 0;
}

LPSTR __fastcall ASPIGetLastError(LPSTR lpDefaultErrorMsg) {
	BYTE SenseKey, ASPIStatus;
// Handle SCSI errors reported directly from drive (most common!)
	if ( gSenseKey ) {
		SenseKey = gSenseKey;
		gSenseKey = 0;
	} else if ( gLAST_IOCTL_ERROR ) { 
	// Handle errors reported by SPTI
		SetLastError(0);
		if ( FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, gLAST_IOCTL_ERROR, 0, gszLAST_ERROR, sizeof(gszLAST_ERROR), NULL) ) {
			gLAST_IOCTL_ERROR = 0;
			return gszLAST_ERROR;
		}
		gLAST_IOCTL_ERROR = 0;
		return lpDefaultErrorMsg;
	} else if ( gASPIStatus ) {
	// Handle errors reported by the ASPI32 Manager DLL (Win95/98/ME)
		ASPIStatus = gASPIStatus;
		gASPIStatus = 0;
		if ( ASPIStatus < 3 )
			return WNASPI32_ERRORS[ASPIStatus];
		if ( ASPIStatus >= 0x80 && ASPIStatus <= 0x82 ) {
			ASPIStatus = (ASPIStatus - 0x80) + 3;
			return WNASPI32_ERRORS[ASPIStatus];
		} else if ( ASPIStatus >= 0xE0 && ASPIStatus <= 0xEB ) {
			ASPIStatus = (ASPIStatus - 0xE0) + 6;
			return WNASPI32_ERRORS[ASPIStatus];
		}
		return lpDefaultErrorMsg;
	} else
		return lpDefaultErrorMsg;
	switch (SenseKey) {
// RECOVERED ERROR: Indicates that the command completed successfully, with some recovery
// action performed by the device server. Details may be determined by examining the additional
// sense bytes and the INFORMATION field. When multiple recovered errors occur during one command,
// the choice of which error to report (e.g., first, last, most severe) is vendor specific.
		case KEY_RECERROR:
			switch (gAddSenseCode) {
				case 0x17:
					switch (gAddSenQual) {
						case 0x00:
							return "Recovered data with no error correction applied";
						case 0x01:
							return "Recovered data with retries";
						case 0x02:
							return "Recovered data with positive head offset";
						case 0x03:
							return "Recovered data with negative head offset";
						case 0x04:
							return "Recovered data with retries and/or CIRC applied";
						case 0x05:
							return "Recovered data using previous sector id";
					}
					break;
				case 0x18:
					switch (gAddSenQual) {
						case 0x00:
							return "Recovered data with error correction applied";
						case 0x01:
							return "Recovered data with error corr. & retries applied";
						case 0x02:
							return "Recovered data - data auto-reallocated";
						case 0x03:
							return "Recovered data with CIRC";
						case 0x04:
							return "Recovered data with Layered-Error Correction (L-EC)";
					}
					break;
			}
			break;
// NOT READY: Indicates that the device is not accessible.
// Operator intervention may be required to correct this condition.
		case KEY_NOTREADY:
			switch (gAddSenseCode) {
				case 0x04:
					switch (gAddSenQual) {
						case 0x00:
							return "Device not ready, cause not reportable";
						case 0x01:
							return "Device is in the process of becoming ready";
						case 0x02:
							return "Device not ready, initializing cmd. required";
						case 0x03:
							return "Device not ready, manual intervention required";
						case 0x07:
							return "Device not ready, operation in progress";
						case 0x08:
							return "Device not ready, long write in progress";
					}
					break;
				case 0x30:
					switch (gAddSenQual) {
						case 0x00:
							return "Incompatible medium installed";
						case 0x01:
							return "Cannot read medium - unknown format";
						case 0x02:
							return "Cannot read medium - incompatible format";
						case 0x07:
							return "Cleaning failure";
					}
					break;
				case 0x3A:
					switch (gAddSenQual) {
						case 0x00:
							return "Medium not present";
						case 0x01:
							return "Medium not present - tray closed";
						case 0x02:
							return "Medium not present - tray open";
					}
					break;
				case 0x3E:
					switch (gAddSenQual) {
						case 0x00:
							return "Device has not self-configured yet";
					}
					break;
			}
			return "Device not ready, cause not reportable";
// MEDIUM ERROR: Indicates that the command terminated with a non-recovered error condition
// that may have been caused by a flaw in the medium or an error in the recorded data.
// This sense key may also be returned if the device server is unable to distinguish
// between a flaw in the medium and a specific hardware failure (i.e., sense key 4h).
		case KEY_MEDIUMERR:
			switch (gAddSenseCode) {
				case 0x02:
					switch (gAddSenQual) {
						case 0x00:
							return "No seek complete (Unreadable sector: unburned/gap/bad disc or lens)";
					}
					break;
				case 0x06:
					switch (gAddSenQual) {
						case 0x00:
							return "No reference position found";
					}
					break;
				case 0x11:
					switch (gAddSenQual) {
						case 0x00:
							return "Unrecovered read error";
						case 0x01:
							return "Read retries exhausted";
						case 0x05:
							return "Layered-Error Correction uncorrectable error";
						case 0x06:
							return "CIRC unrecovered error";
						case 0x0F:
							return "Error reading UPC/EAN number";
						case 0x10:
							return "Error reading ISRC number";
					}
					break;
				case 0x15:
					switch (gAddSenQual) {
						case 0x00:
							return "Random positioning error";
						case 0x01:
							return "Mechanical positioning error";
						case 0x02:
							return "Positioning error detected by read of medium";
					}
					break;
				case 0x31:
					switch (gAddSenQual) {
						case 0x00:
							return "Medium format corrupted";
					}
					break;
				case 0x57:
					switch (gAddSenQual) {
						case 0x00:
							return "Unable to recover table-of-contents";
					}
					break;
				case 0x73:
					switch (gAddSenQual) {
						case 0x00:
							return "CD control error";
					}
					break;
			}
			return "Unknown medium error";
// HARDWARE ERROR: Indicates that the device server detected a non-recoverable
// hardware failure (e.g., controller failure, device failure, or parity error)
// while performing the command or during a self test.
		case KEY_HARDERROR:
			switch (gAddSenseCode) {
				case 0x05:
					switch (gAddSenQual) {
						case 0x00:
							return "Device does not respond to selection";
					}
					break;
				case 0x08:
					switch (gAddSenQual) {
						case 0x00:
							return "Device communication failure";
						case 0x01:
							return "Device communication time-out";
						case 0x02:
							return "Device communication parity error";
					}
					break;
				case 0x09:
					switch (gAddSenQual) {
						case 0x00:
							return "Track following error";
						case 0x01:
							return "Tracking servo failure";
						case 0x02:
							return "Focus servo failure";
						case 0x03:
							return "Spindle servo failure";
						case 0x04:
							return "Head select fault";
					}
					break;
				case 0x1B:
					switch (gAddSenQual) {
						case 0x00:
							return "Synchronous data transfer error";
					}
					break;
				case 0x3E:
					switch (gAddSenQual) {
						case 0x01:
							return "Device failure";
						case 0x02:
							return "Timeout on Device";
					}
					break;
				case 0x44:
					switch (gAddSenQual) {
						case 0x00:
							return "Internal device failure";
					}
					break;
				case 0x46:
					switch (gAddSenQual) {
						case 0x00:
							return "Unsuccessful soft reset";
					}
					break;
				case 0x47:
					switch (gAddSenQual) {
						case 0x00:
							return "SCSI parity error";
					}
					break;
				case 0x4A:
					switch (gAddSenQual) {
						case 0x00:
							return "Command phase error";
					}
					break;
				case 0x4B:
					switch (gAddSenQual) {
						case 0x00:
							return "Data phase error";
					}
					break;
				case 0x4C:
					switch (gAddSenQual) {
						case 0x00:
							return "Device failed self-configuration";
					}
					break;
				case 0x65:
					switch (gAddSenQual) {
						case 0x00:
							return "Voltage fault";
					}
					break;
			}
			return "Unknown hardware failure";
/*	ILLEGAL REQUEST: Indicates that:
	a) The command was addressed to an incorrect logical unit number (see SAM-3);
	b) The command had an invalid task attribute (see SAM-3);
	c) The command was addressed to a logical unit whose current configuration prohibits
	   processing the command;
	d) There was an illegal parameter in the CDB; or
	e) There was an illegal parameter in the additional parameters supplied as data for some
	   commands (e.g., PERSISTENT RESERVE OUT).
If the device server detects an invalid parameter in the CDB, it shall terminate the command without
altering the medium. If the device server detects an invalid parameter in the additional parameters
supplied as data, the device server may have already altered the medium. */
		case KEY_ILLGLREQ:
			switch (gAddSenseCode) {
				case 0x07:
					switch (gAddSenQual) {
						case 0x00:
							return "Multiple peripheral devices selected";
					}
					break;
				case 0x1A:
					switch (gAddSenQual) {
						case 0x00:
							return "Parameter list length error";
					}
					break;
				case 0x20:
					switch (gAddSenQual) {
						case 0x00:
							return "Invalid Command operation code";
					}
					break;
				case 0x21:
					switch (gAddSenQual) {
						case 0x00:
							return "Logical block address out of range";
					}
					break;
				case 0x24:
					switch (gAddSenQual) {
						case 0x00:
							return "Invalid field in Command Descriptor Block";
					}
					break;
				case 0x25:
					switch (gAddSenQual) {
						case 0x00:
							return "Device not supported";
					}
					break;
				case 0x26:
					switch (gAddSenQual) {
						case 0x00:
							return "Invalid field in parameter list";
						case 0x01:
							return "Parameter not supported";
						case 0x02:
							return "Parameter value invalid";
					}
					break;
				case 0x3D:
					switch (gAddSenQual) {
						case 0x00:
							return "Invalid bits in identify message";
					}
					break;
				case 0x43:
					switch (gAddSenQual) {
						case 0x00:
							return "Message error";
					}
					break;
				case 0x63:
					switch (gAddSenQual) {
						case 0x00:
							return "End of user area encountered on this track";
					}
					break;
				case 0x64:
					switch (gAddSenQual) {
						case 0x00:
							return "Illegal mode for this track";
					}
					break;
			}
			return "Illegal request";
// UNIT ATTENTION: Indicates that a unit attention condition has been established
// (e.g., the removable medium may have been changed, a logical unit reset occurred).
		case KEY_UNITATT:
			switch (gAddSenseCode) {
				case 0x0A:
					switch (gAddSenQual) {
						case 0x00:
							return "Error log overflow";
					}
					break;
				case 0x28:
					switch (gAddSenQual) {
						case 0x00:
							return "Not ready to ready change, medium may have changed";
					}
					break;
				case 0x29:
					switch (gAddSenQual) {
						case 0x00:
							return "Power on, reset, or bus device reset occurred";
						case 0x01:
							return "Power on occurred";
						case 0x02:
							return "SCSI bus reset occurred";
						case 0x03:
							return "Bus device reset function occurred";
						case 0x04:
							return "Device internal reset";
					}
					break;
				case 0x2A:
					switch (gAddSenQual) {
						case 0x00:
							return "Parameters changed";
						case 0x01:
							return "Mode parameters changed";
						case 0x02:
							return "Log parameters changed";
					}
					break;
				case 0x2F:
					switch (gAddSenQual) {
						case 0x00:
							return "Commands cleared by another initiator";
					}
					break;
				case 0x3F:
					switch (gAddSenQual) {
						case 0x00:
							return "Device operating conditions have changed";
						case 0x01:
							return "Microcode has been changed";
						case 0x02:
							return "Changed operating definition";
						case 0x03:
							return "Inquiry data has changed";
					}
					break;
				case 0x5A:
					switch (gAddSenQual) {
						case 0x00:
							return "Operator request or state change input";
						case 0x01:
							return "Operator medium removal request";
					}
					break;
				case 0x5B:
					switch (gAddSenQual) {
						case 0x00:
							return "Log exception";
						case 0x01:
							return "Threshold condition met";
						case 0x02:
							return "Log counter at maximum";
						case 0x03:
							return "Log list codes exhausted";
					}
					break;
				case 0x5D:
					switch (gAddSenQual) {
						case 0x00:
							return "Failure prediction threshold exceeded";
						case 0xFF:
							return "Failure prediction threshold exceeded (false)";
					}
					break;
				case 0x5E:
					switch (gAddSenQual) {
						case 0x00:
							return "Low power condition on";
						case 0x01:
							return "Idle condition activated by timer";
						case 0x02:
							return "Standby condition activated by timer";
						case 0x03:
							return "Idle condition activated by command";
						case 0x04:
							return "Standby condition activated by command";
					}
					break;
			}
			return "An issue with the device requires your attention";
// ABORTED COMMAND: Indicates that the device server aborted the command.
// The application client may be able to recover by trying the command again.
		case KEY_CMDABORT:
			switch (gAddSenseCode) {
				case 0x00:
                    if ( gAddSenQual == 6 )
						return "I/O process terminated";
				case 0x45:
                    if ( gAddSenQual == 00 )
						return "Select or reselect failure";
				case 0x48:
					if ( gAddSenQual == 00 )
						return "Host detected error message received";
				case 0x49:
					if ( gAddSenQual == 00 )
						return "Invalid message error";
				case 0x4D:
					return "Tagged overlapped commands";
				case 0x4E:
					if ( gAddSenQual == 00 )
						return "Overlapped commands attempted";
			}
			return "Command aborted for unknown reason - it may be retried";
	}
	return lpDefaultErrorMsg;
}

} // End extern "C" {
