#include <global.h>
#include <scsidefs.h>
#include <WNASPI32.H>
#include <my_ntddcdrm.h>

#pragma pack(1)

struct ASPI_HANDLE {
	BYTE HaId;
	BYTE Target;
	BYTE LUnit;
};

// Define no more than 10 elements (1-9)
struct CD_DRIVELIST {
	char szVendorProdID[29]; // SCSI VendorProdID string = 29 byte standard
	ASPI_HANDLE hCDrive;
	char cDriveLetter;
	BOOLEAN bDiscReady;  // Is CD loaded and drive ready ? False = No disc, not ready!
	BOOLEAN bAllAudioCD; // Are all tracks of type audio ? Set TRUE
	LPVOID lpReserved1;  // Pointer to custom data, like CDDB ID or CD Title info, etc.
};

struct TRACK_DESCRIPTOR_00 {
	BYTE Reserved1; // User reserved = set true if all tracks = audio
	BITS CONTROL : 4;
	BITS ADR     : 4;
	BYTE TrackNumber;
	BYTE Reserved2;
	union {
		BYTE  Address[4];
		DWORD LBAddress;
	};
};

// TOC FORMAT_00 SCSI SIZE = 804 bytes

struct TOC_FORMAT_00 {
	union {
		BYTE Length[2];
		WORD wLength;
	};
	BYTE FirstTrack;
	BYTE LastTrack;
	TRACK_DESCRIPTOR_00 TrackData[100];
};

typedef TOC_FORMAT_00* LPTOC;

// Full TOC Format
struct TRACK_DESCRIPTOR_0010 {
	BYTE SessionNumber;
	BITS CONTROL : 4;
	BITS ADR     : 4;
	BYTE TNO;
	BYTE POINT;
	BYTE Min;
	BYTE Sec;
	BYTE Frame;
	BYTE Zero;
	BYTE PMIN;
	BYTE PSEC;
	BYTE PFRAME;
};

struct TOC_FORMAT_0010 { 
	union {
		BYTE Length[2];
		WORD wLength;
	};
	BYTE FirstSession;
	BYTE LastSession;
	TRACK_DESCRIPTOR_0010 TrackData[100];
};

typedef TOC_FORMAT_0010* LPFULLTOC;

// Session Info Format
struct TOC_FORMAT_0001 {
	union {
		BYTE Length[2];
		WORD wLength;
	};
	BYTE FirstSession;
	BYTE LastSession;
// Single Track Descriptor
	BYTE Reserved1;
	BITS CONTROL : 4;
	BITS ADR     : 4;
	BYTE FirstTrackLastSession;
	BYTE Reserved2;
	BYTE Address[4];
};

// CD-TEXT Related Structures
// Reference: "SCSI-3 Multi-Media Commands (MMC-3) FROM Microsoft Link 2001 - mmc3r09.pdf"
// Search for: "CD-TEXT Format in the Lead-in" - page 428/444
// Implied Max 2,048 text packs possible * 18 = 36864 + 1 + 4

struct CDTEXT_PACK_DATA {
	BYTE PackType;
	BITS TrackOrPackNumber : 7;
	BITS ExtensionFlag     : 1;
	BYTE SequenceNumber;
	BITS CharacterPosition : 4;
	BITS BlockNumber       : 3;
	BITS DBCC              : 1;
	BYTE TextData[12];
	BYTE CRC16[2];
};

struct CDTEXT_FORMAT_05 {
	union {
		BYTE Length[2];
		WORD wLength;
	};
	BYTE Reserved[2];
	CDTEXT_PACK_DATA CDTextPacks[1];
};

typedef CDTEXT_PACK_DATA* LPCDTEXTPACK;
typedef CDTEXT_FORMAT_05* LPCDTEXT;

struct MASTER_TOC_CDTEXT_TRACKINFO {
	char Title[161];
	char Performer[161];
	char Songwriter[161];
	char Composer[161];
	char Arranger[161];
	char Message[161];
	char ISRC[13]; // The ISRC must be 12 chars long. The first 5 are alphanumeric, the last 7 are numeric-only.
};

struct MASTER_TOC_CDTEXT_FORMAT {
	BOOLEAN bHasCDText;
	char AlbumTitle[161];
	char Performer[161];
	char Songwriter[161];
	char Composer[161];
	char Arranger[161];
	char Message[161];
	char Catalog[14]; // The catalog number must be 13 digits long and is encoded according to UPC/EAN rules.
    char UPC[14];
	PSTR Genre;
	char GenreDesc[11]; // Appears to be 10 character max according to CDRWIN CDTEXT editor
	MASTER_TOC_CDTEXT_TRACKINFO TrackData[100];
};

struct MASTER_TOC_TRACK_DESCRIPTOR {
	BYTE  TrackNumber;
	BYTE  SessionNumber;
	BYTE  TrackMode;
	char  TrackFileName[MAX_PATH];
	DWORD StartAddress;
	DWORD StopAddress;
	DWORD TotalSectors;
	char  szTotalSectors[14];
	DWORD TotalBytes;
	char  szTotalBytes[14];
};

struct TOC_MASTER_FORMAT {
	BYTE FirstTrack;
	BYTE LastTrack;
	BYTE TotalTracks;
	BYTE TotalSessions;
	MASTER_TOC_CDTEXT_FORMAT CDTextData;
	MASTER_TOC_TRACK_DESCRIPTOR TrackData[100];
};

typedef TOC_MASTER_FORMAT* LPMASTERTOC;

struct SUBCHANNEL_SUBQ_DESCRIPTOR {
	BITS ADR     : 4;
	BITS CONTROL : 4;
	BYTE TrackNumber; // Warning: CD Device inconsistency in returning Hex or BCD values
	BYTE IndexNumber; // Coding must do BCD/HEX checks before determining data is invalid
	BYTE Min;
	BYTE Sec;
	BYTE Frame;
	BYTE ZERO;
	BYTE AMIN;
	BYTE ASEC;
	BYTE AFRAME;
	BYTE CRC1;
	BYTE CRC2;
	BYTE Reserved1;
	BYTE Reserved2;
	BYTE Reserved3;
	BITS Reserved4 : 7;
	BITS PSubCode  : 1;
};

struct SCSIMODEHEADER {
	BYTE sense_data_len;
	BYTE medium_type;
	BITS res2		: 4;
	BITS cache		: 1;
	BITS res		: 2;
	BITS write_prot	: 1;
	BYTE nBlockLen;
};

#define	MP_P_CODE\
	BITS p_code	: 6;\
	BITS p_res	: 1;\
	BITS parsave: 1;

// Table 103 - CD Capabilities and Mechanical Status Page
// Reference: SCSI-3 MMC (1997), mmc-r10a.pdf, page 79

struct SCSICDMODEPAGE_2A {
	MP_P_CODE;		// parsave & pagecode			(0)
	BYTE p_len;		// 0x14 = 20 Bytes  			(1)

	BITS cd_r_read   	: 1;	// Reads CD-R  media			(2)
	BITS cd_rw_read  	: 1;	// Reads CD-RW media		     
	BITS method2 		: 1;	// Reads fixed packet method2 media  
	BITS dvd_rom_read	: 1;	// Reads DVD ROM media		     
	BITS dvd_r_read  	: 1;	// Reads DVD-R media		     
	BITS dvd_ram_read	: 1;	// Reads DVD-RAM media		     
	BITS res_2_67    	: 2;	// Reserved			     

	BITS cd_r_write  	: 1;	// Supports writing CD-R  media		(3)
	BITS cd_rw_write 	: 1;	// Supports writing CD-RW media	     
	BITS test_write  	: 1;	// Supports emulation write	     
	BITS res_3_3 		: 1;	// Reserved			     
	BITS dvd_r_write 	: 1;	// Supports writing DVD-R media	     
	BITS dvd_ram_write	: 1;	// Supports writing DVD-RAM media    
	BITS res_3_67    	: 2;	// Reserved			     

	BITS audio_play  	: 1;	// Supports Audio play operation	(4)    
	BITS composite   	: 1;	// Delivers composite A/V stream
	BITS digital_port_1	: 1;	// Supports digital output on port 1
	BITS digital_port_2	: 1;	// Supports digital output on port 2
	BITS mode_2_form_1	: 1;	// Reads Mode-2 form 1 media (XA)    
	BITS mode_2_form_2	: 1;	// Reads Mode-2 form 2 media	     
	BITS multi_session	: 1;	// Reads multi-session media	     
	BITS res_4   		: 1;	// Reserved			     

	BITS cd_da_supported: 1;	// CD-DA Commands Supported- Reads audio data with READ CD cmd	(5)
	BITS cd_da_accurate	: 1;	// CD-DA Stream is accurate - READ CD data stream is accurate   
	BITS rw_supported	: 1;	// Reads R-W sub channel information
	BITS rw_deint_cor	: 1;	// Reads de-interleaved & corrected R-W sub chan
	BITS c2_pointers 	: 1;	// Supports C2 error pointers
	BITS ISRC     		: 1;	// Reads ISRC information
	BITS UPC     		: 1;	// Reads media catalog number (UPC)
	BITS read_bar_code	: 1;	// Supports reading bar codes

	BITS lock    		: 1;	// PREVENT/ALLOW may lock media		(6)
	BITS lock_state  	: 1;	// Lock state 0=unlocked 1=locked    
	BITS prevent_jumper	: 1;	// State of prev/allow jumper 0=pres 
	BITS eject    		: 1;	// Ejects disc/cartr with STOP LoEj
	BITS res_6_4  		: 1;	// Reserved			     
	BITS loading_type	: 3;	// Loading mechanism type	     

	BITS sep_chan_vol	: 1;	// Vol controls each channel separat	(7)
	BITS sep_chan_mute	: 1;	// Mute controls each channel separat
	BITS disk_present_rep: 1;	// Changer supports disk present reporting 
	BITS sw_slot_sel  	: 1;	// Load empty slot in changer	     
	BITS res_7   		: 4;	// Reserved			     

	BYTE max_read_speed[2];	// Maximum Read Speed Supported (in kBps) (8)
	BYTE num_vol_levels[2];	// # of supported volume levels		(10)

	BYTE buffer_size[2];	// Buffer size supported by Lun for the data (in KBytes) (12)
	BYTE cur_read_speed[2];	// Current read speed selected in KB/s	(MMC-1)	(14)
	BYTE res_16; 			// Reserved				(16)
	BITS res_17_0	: 1;	// Reserved				(17)
	BITS BCK  		: 1;	// Data valid on falling edge of BCK 
	BITS RCK  		: 1;	// Set: HIGH high LRCK=left channel  
	BITS LSBF		: 1;	// Set: LSB first Clear: MSB first   
	BITS length		: 2;	// 0=32BCKs 1=16BCKs 2=24BCKs 3=24I2c
	BITS res_17		: 2;	// Reserved

	BYTE max_write_speed[2];	// Max. write speed supported in KB/s	(18)
	BYTE cur_write_speed[2];	// Current write speed in KB/s		(20)
};

// CD-DVD Read/Write Error Recovery Page

#define CPAGE_01_SIZE 20  // Looks like 20 bytes is the right value thus far

struct SCSICDMODEPAGE_01 {
	MP_P_CODE;		// parsave & pagecode			(0)
	BYTE p_len;		// 0x0A = 10 Bytes  			(1)

	BITS AWRE : 1;   // Automatic write reallocation enable (1)  
	BITS ARRE : 1;   // Automatic read reallocation enable (1) 
	BITS TB   : 1;   // Transfer block (0) 
	BITS RC   : 1;   // Read continuous (0) 
	BITS ERR  : 1;   // Enable early recovery
	BITS PER  : 1;   // Post error (0) 
	BITS DTE  : 1;   // Disable transfer on error (0) 
	BITS DCR  : 1;   // Disable correction (0) 

	BYTE ReadRetryCount;
	BYTE Reserved1;
	BYTE Reserved2;
	BYTE Reserved3;
	BYTE Reserved4;
	BYTE WriteRetryCount;
	BYTE Reserved5;
	BYTE RecoveryTimeLimit[2];
};

extern "C" {
	BOOL  __fastcall ASPIStandardInquiry();
	BYTE  __fastcall ASPISerialInquiry();
	BOOL  __fastcall ASPIIsUnitReady();
	BOOL  __fastcall ASPIReZeroUnit();
	void  __fastcall ASPIResetUnit();
	BOOL  __fastcall ASPIStartStopUnit();
	BOOL  __fastcall ASPIEjectDiscTray();
	BOOL  __fastcall ASPILoadTrayReadyUnit();
	DWORD __fastcall ASPIReadCDROMCapacity();

	BOOL  __fastcall ASPIGetTOCLBA(LPTOC lpTocLBA);
	BYTE  __fastcall ASPIGetTOCMSF(LPTOC lpTocMSF);
	BYTE  __fastcall ASPIGetTOCBasic(LPTOC, PDWORD);
	BOOL  __fastcall ASPIGetTOCFull(TOC_FORMAT_0010*);
	BOOL  __fastcall ASPIGetTOCSession(TOC_FORMAT_0001*);
	LPCDTEXTPACK      __fastcall ASPIGetCDTEXT(PLONG OUT lpTextPackSize);
	CDTEXT_PACK_DATA* __fastcall UnpackCDTextData(LPMASTERTOC, CDTEXT_PACK_DATA*);

	BOOL  __fastcall ASPISetCDDrive(CD_DRIVELIST *lpCDDrive);
	void  __fastcall ASPICloseCDDrive();
	char  __fastcall ASPIGetCDDrives(CD_DRIVELIST *lpCDDriveList);
    DWORD __fastcall ASPIGetCDSpeed();
	BOOL  __fastcall ASPISetCDSpeed(WORD nSpeed);
	PBYTE __fastcall ASPIModeSense10(BYTE, PBYTE, WORD);
	BOOL  __fastcall ASPIModeSelect10(PBYTE, WORD);
	BOOL  __fastcall ASPIAnalyzeTrackModes(LPTOC, BYTE);
	BOOL  __fastcall ASPIAnalyzeQSubCodes(LPMASTERTOC);
	BOOL  __fastcall ASPIReadSectorHeader(DWORD LBA);
	BOOL  __fastcall ASPIReadQSubChannelData(DWORD LBA, DWORD nSectors);
	void  __fastcall ASPIPrintMMCFeaturesPage(SCSICDMODEPAGE_2A* lpMMCPage);
	LPSTR __fastcall ASPIGetLastError(LPSTR lpDefaultErrorMsg);
	BOOL  __fastcall ASPIGetDVDStruct();

	BOOL  __fastcall ASPIStart(BOOL bForceASPI);
	void  __fastcall ASPIStop();
}

__forceinline extern BOOL __fastcall ASPIReadCDDASectors(DWORD LBA, DWORD nSectors);
__forceinline extern BOOL __fastcall ASPIReadDATASectors(DWORD LBA, DWORD nSectors);
__forceinline extern BOOL __fastcall ASPIReadRAWSectors(DWORD LBA, DWORD nSectors);

// MMC READ CD, Sub-channel Data Selection Field definitions.
// 0 = Return no Sub data
#define RET_SUBCHANNEL_RAW_PW_DATA 0x01
#define RET_SUBCHANNEL_Q_DATA      0x02
#define RET_SUBCHANNEL_RW_DATA     0x04

// mmc-r01.pdf (1994) documentation for Control Field info
// Table 42: Sub-channel Q control bits
// Bit        Equals Zero            Equals One
// 0   Audio without premphasis   Audio with premphasis
// 1   Digital copy prohibited    Digital Copy permitted
// 2   Audio track                Data track 
// 3   Two channel audio          Four Channel Audio

#define TRACK_FLAG_DATA          0x04 /* data track */
#define TRACK_FLAG_4CHAN_AUDIO   0x08 // No discs were made with this... or one maybe.

#define IsDataTrack(Q_ControlBits) ((Q_ControlBits & TRACK_FLAG_DATA) == TRACK_FLAG_DATA)
#define IsAudioTrack(Q_ControlBits) ((Q_ControlBits & TRACK_FLAG_DATA) == 0)
#define Is4ChanAudioTrack(Q_ControlBits) ((Q_ControlBits & TRACK_FLAG_4CHAN_AUDIO) == TRACK_FLAG_4CHAN_AUDIO)

// ** INDEXing for CDTRACK_MODES[] - MAKE SURE ENTRIES VALID! **
// Type Audio
#define AUDIO_CDDA     0
// Type MODE1 Data
#define MODE1_2048     1
#define MODE1_2352     2 // Raw Read
// Type MODE2 Data
#define MODE2_F0_2336  3
#define MODE2_F1_2048  4
#define MODE2_F2_2324  5
#define MODE2_2352     6 // Raw Read

// CDMAGE support: Mode1/2048, Mode1/2352, Mode2/2336, Mode2/2352, Mode2/2056, CDI/2352, CDI/2336, CD+G/2448, Audio/2352
// SCSI-3 "mmc3r09.pdf" - Table 166 - Data Block Type Codes
// Raw R-W appended to sector (2448), no need for .sub format for CD-G discs ??? Solution ??

enum CDTRACK_MODE : BYTE {
// Type Audio
	eAUDIO_CDDA = 0,
// Type MODE1 Data
	eMODE1_2048,
	eMODE1_2352,    // Raw Read
// Type MODE2 Data
	eMODE2_F0_2336,
	eMODE2_F1_2048,
	eMODE2_F2_2324,
	eMODE2_2352     // Raw Read
};

//     ****** Some Corresponding Facts ****
// CD-Audio         ICE 908                 Red book*             
// CD-ROM           ISO/IEC 10149           Yellow book*          
// CD-I                                     Green book*           
// Video CD                                 White book*           

#define CDROM_SECTOR_SIZE     2048 // General data sector size - NEVER CHANGE THIS!
#define MODE1_SECTOR_SIZE     2048 // (Yellow Book) Mode1 Form1
#define MODE2_F0_SECTOR_SIZE  2336 // (Yellow Book) Mode2 Form0 (Formless)

// ERROR NOT GREENBOOK, IT'S XA - RESEARCH FIXXXXXXXXXXXXX!!!!
#define MODE2_F1_SECTOR_SIZE  2048 // (XA) Mode2 Form1
#define MODE2_F2_SECTOR_SIZE  2328 // (XA) Mode2 Form2
#define RAW_SECTOR_SIZE       2352

// 27 sectors/read (63,504 Bytes) I found was the max on all my PCs, so leave alone!
#define SECTOR_READ_CHUNKS  27     // For ~63KB 2352 Chunks (Same as CDRWIN's chunk size!)
#define SCSI_BUFFER_SIZE    65536  // 4096 * 16 = 64KB, Make 4K Aligned (RAW_SECTOR_SIZE * READ_CHUNKS), allow ~1024 PADDING
#define SCSI_BUFFER_LIMIT   63504  // 27 * 2352 = Size - possible alignment adjustment
#define CDTEXT_BUFFER_SIZE  38912  // 2048x(18+1)=38912 bytes for max via Sony doc - drive wants even value or FAILS!
#define DEF_WAITLEN_MS      15000  // In milliseconds for WaitForSingleObject proc
#define DEF_TIMEOUT         15     // In seconds for SPTI/DeviceIoControl method
// 44100 KHz * 4 Bytes (2 bytes per channel/16-bit stereo) = 176400
// (176400 / 2352) = 75 sectors = 1 second of 16-bit stereo audio
// Meaning, basic CD-ROM hardware had to be able to burst-read 75 sectors to achieve speed of 1x
// CD Cracking Uncovered Book says use 177, but I think exact 176.4 + rounding up works fine!
#define X1  (float)176400.0f // 1x = 176400 B/s - That's the right value, CDRWIN (176.5) was off a bit
#define XX1 (float)176.4f    // 1x = 176.4 kB/s - Mystery solved: 44100 * 4 = 1x

#define MSB2DWORD(lpMSB_DWORD)\
	__asm MOV ESI, DWORD PTR lpMSB_DWORD \
	__asm MOV EBX, DWORD PTR [ESI] \
	__asm MOV  CL, 16 \
	__asm MOV  AH, BL \
	__asm MOV  AL, BH \
	__asm SHL EAX, CL \
	__asm SHR EBX, CL \
	__asm MOV  AH, BL \
	__asm MOV  AL, BH \
	__asm MOV DWORD PTR [ESI], EAX

#define MSB2DWORD2(lpLBATrack)\
	__asm MOV EBX, lpLBATrack \
	__asm LEA ESI, [EBX].LBAddress \
	__asm MOV EBX, DWORD PTR [ESI] \
	__asm MOV  CL, 16 \
	__asm MOV  AH, BL \
	__asm MOV  AL, BH \
	__asm SHL EAX, CL \
	__asm SHR EBX, CL \
	__asm MOV  AH, BL \
	__asm MOV  AL, BH \
	__asm MOV DWORD PTR [ESI], EAX

extern BYTE    gSenseKey, gAddSenseCode, gAddSenQual;
extern PSTR    CDTRACK_MODES[], GENRE_TYPES[];
extern LPBYTE  gAlignedSCSIBuffer;
extern BYTE    gRawNullSector[];
extern BOOLEAN bGlobalAbort;

#pragma pack()
