#include <Global.h>
#include <argcargv.h>
#include "ASPILib.h"
#include "GAMECDDB.h"

#define YEAR        "2025"
#define APP_VERSION "1.43"
#define VERDATE "Version "APP_VERSION" ("__DATE__")"
#define WWWURL  "http://www.ysutopia.net/link.php?id=update&data=TurboRip.exe&ver="APP_VERSION

char CONSOLE_TITLE[] = "TurboRip "VERDATE" - Copyright (c) 2006-"YEAR" by Nicolas Livaditis";
char CONSOLE_INTRO[] =
	"*** The PC Engine/TurboGrafx-16 CD-ROM ISO/WAV/CUE Ripper ***\r\n"\
	"PC Engine-FX.com ( www.pcengine-fx.com | www.ysutopia.net )\r\n"\
	"By Nicolas Livaditis (NightWolve) - Copyright (c) 2006-"YEAR"\r\n"VERDATE"\r\n"\
	"Thanks to: Squaresoft74, for TOC data + David Shadoff, for TECH Info\r\n\n";
char MSG_NO_FILENAME_PROMPT[] =
	"Unknown CD and no base title was entered at the prompt\r\n\n"\
	"Enter a CD title now (4 letter minimum):> ";
char MSG_NO_PARAMETERS[] =
	"No parameters were specified at the prompt\r\n"\
	"Enter them now (use /?, /h or /help for a list), or press enter to skip\r\n"\
	"Parameters:> ";
char MSG_NO_CDDA_STREAM_FORCERIP[] =
	"Note: You chose to forcibly use a drive that lacks the CD-DA Stream-Is-Accurate\r\n"\
	"feature! Try another drive or buy a new one; any new drive will have it! Jitter\r\n"\
	"Correction isn't implemented to compensate so audio-ripping quality may suffer!\r\n\n";
char MSG_NO_CDDA_STREAM_WARNING[] =
	"Warning: This drive may not support reading CD-DA accurately! You should\r\n"\
	"upgrade it for better ripping support of an audio/videogame CD, otherwise the\r\n"\
	"audio data when ripped to a wave file will be displaced or suffer from static\r\n"\
	"resulting in a less than perfect image backup... If you believe this detection\r\n"\
	"was in error and/or simply wanna force usage of the selected drive regardless,\r\n"\
	"run TurboRip again with the /FORCERIP parameter. But please, upgrade this\r\n"\
	"drive in the near future or use another one!\r\n";
char MSG_TOC_MATCH[] =
	"The CD's TOC was matched against a TOC database of "qCD_TITLE_COUNT" videogame titles!\r\n\n"\
	"Title: \"%s\"\r\n"\
	"Type : \"%s\"\r\n\n"\
	"Note: A TOC match is good, it verifies the CD will work properly with your\r\n"\
	"console, but if it's a burned CD-R or image, it does NOT guarantee that the\r\n"\
	"audio data hasn't been degraded by lossy encoders such as MP3/OGG...\r\n\n";
char MSG_TOC_NO_MATCH[] =
	"Note: This CD-ROM does not match any TOC in the internal videogame database.\r\n"\
	"If it IS a videogame in the database, damage due to MP3 decoding prevents\r\n"\
	"detection! Try TocFixer which can fix the CD image afterwards or it WON'T\r\n"\
	"function properly (e.g. crashes, audio/lip sync issues, etc.), so FYI!!\r\n\n";
//  "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
char ERROR_NON_CDROM_MEDIA[] =
	"Error: It appears you may be attempting to use TurboRip with non-CD media,\r\n"\
	"likely DVD - TurboRip is designed for use solely with CD-ROM media\r\n";
char ERROR_TRACK_MODE[] = "Error: Track Mode Analysis apparently failed\r\n";
char ERROR_EXT_PARAMS[] = "Error: Too many mutually exclusive parameters - remove one or more"BRK;
char ERROR_BAD_DATAMODE[] =
	"Error: A MODE2 track was detected that should be MODE1 for this game disc"BRK
	"You appear to be ripping a CD-R that was wrongly burned, likely by Ahead Nero"BRK;

char HELP_PART1[] =
	"Usage: TURBORIP [/1|/2.../9] [\"/NAME=xxx\"] [/SPEED=n|max]\r\n"
	"Example: TURBORIP /1 \"/name=Dracula X (J)\" /speed=max\r\n\n"
	"Parameters:\r\n\n"
	"/AUTO        - Autoscan all CD drives with TOC database, rip 1st game detected\r\n"
	"/1-9         - Manually select ID of CD-ROM drive to rip from\r\n"
	"/NAME=xxxx   - Alternative base filename for unknown CDs (Use quotes if spaced)\r\n"
	"/SPEED=n|max - Audio/raw sector reading speed (default is 8x)\r\n"
	"/TURBO       - Shorthand for /speed=max, sets max speed for audio/raw sectors\r\n"
	"/TRACK=n     - Only rip a specified data/audio track (1-99)\r\n"
	"/TOC         - Only save the TOC file\r\n"
	"/RAW         - Enable reading of raw (2352 byte) data sectors (default is 2048)\r\n"
	"/REDUMP      - Rip a BIN/BIN/CUE image conforming to the redump.org standard\r\n"
	"/DEVICES     - Only display a list of installed CD-ROM devices\r\n"
	"/NORMALIZE   - Apply basic normalize/amplifying algorithm to wave files\r\n"
	"/MP3         - Only rip audio tracks to MP3 (intended for music discs)\r\n"
	"/APE         - Rip an ISO/APE/CUE image for lossless 1:1 backup purposes\r\n"
	"/APELEVEL=xx - Adjust the APE compression level (default is normal)\r\n"
	"               Allowed: fast, normal, high, extrahigh, insane\r\n";
char HELP_PART2[] =
	"/PSP         - Rip an ISO/MP3/TOC image for use with PCEP emulator on Sony PSP\r\n"
	"/XBOX        - Rip an ISO/MP3/CUE image for use with HUGO-X emulator on X-BOX\r\n"
	"/MRS=n       - Adjust the MP3 resampling rate (default is 44100 Hz)\r\n"
	"               Allowed: 8000,11025,12000,16000,22050,24000,32000,44100,48000\r\n"
	"/MBR=n       - Adjust the MP3 [minimum with VBR] bitrate (default is 128 kbps)\r\n"
	"               Allowed: 32,48,64,80,96,112,128,160,192,224,256,320\r\n"
	"/MMBR=n      - Adjust the maximum MP3 bitrate (default is encoder's choice)\r\n"
	"               Note: This will enable MP3 VBR mode with default quality rate\r\n"
	"               Allowed: 32,48,64,80,96,112,128,160,192,224,256,320\r\n"
	"/MVBR=n      - This will enable MP3 VBR mode and set its quality rate (0-9)\r\n"
	"/READRETRY=n - # of times a drive will try its read-recovery algorithm (5-255)\r\n"
	"/USEASPI     - Try your system ASPI (WNASPI32.DLL) if available. ASPI is NOT\r\n"
	"               needed on NT/2K/XP/Vista/7/8+ but the option exists just in case\r\n"
	"/FORCERIP    - Allow ripping despite lacking a drive that is capable of reading\r\n"
	"               CD-DA accurately. Using this option indicates your drive is very\r\n"
	"               old and ought to be replaced to ensure a higher quality rip\r\n\n";

char MSG_SCANNING_BEGINS[]  = "Notice: Scanning for installed CD-ROM drives...\r\n\n";
char MSG_NO_CD_SPECIFIED[]  = "\r\nNo CD-ROM drive ID was specified\r\n\n";
char MSG_SELECT_CDDRIVE[]   = "Select a CD drive from the list above (1-%hu):> ";
char ERROR_CD_NOT_SELECTED[] = "\r\nError: Drive #%hu can't be selected or doesn't exist\r\n\n";
char ERROR_CDS_NOT_FOUND[]  = "Error: No CD-ROM drives found";

#define DEF_MIN_SPEED 8

TOC_MASTER_FORMAT MasterTOC;

char    extISO[] = ".iso";
#define EXTISO 'osi.'
char    extWAV[] = ".wav";
#define EXTWAV 'vaw.'
char    extBIN[] = ".bin";
#define EXTBIN 'nib.'
char    extCUE[] = ".cue";
#define EXTCUE 'euc.'
char    extMP3[] = ".mp3";
#define EXTMP3 '3pm.'
char    extAPE[] = ".ape";
#define EXTAPE 'epa.'
char    extOGG[] = ".ogg";
#define EXTGOO 'ggo.'
char    extHUGOX[] = "-HUGOX.cue";

char    gTrackTitle[MAX_PATH];
PSTR    lpAudioExt = extWAV;
DWORD   SECTOR_SIZE = MODE1_SECTOR_SIZE;
BOOLEAN  gGameCDMatch;
BOOLEAN  bGlobalLoopAlert;
TOC_INFO gSetCD;

__declspec(align(16)) BYTE gEncodeBuffer[SCSI_BUFFER_SIZE]; // Static 64KB working buffer, better on 9X core

#pragma pack(1)

struct TurboRipParams {
	BYTE   AUTO;
	BYTE   DriveID;
	BYTE   Devices;
	WORD   Speed;
	BYTE   READRETRY;
	BYTE   TOC;
	BYTE   RAW;
	BYTE   MP3;
	BYTE   APE;
	BYTE   REDUMP;
	BYTE   PSP;
	BYTE   XBOX;
	BYTE   TrackNum;
	BYTE   USEASPI;
	BYTE   ForceRip;
	BYTE   Normalize;
};

union NumericType {
	char  bVar;
	WORD  wVar;
	DWORD dwVar;
	char  szVar[4];
};

// Initialize header to default Redbook CD-DA audio track (minus SIZEs)
WAVE_FILE_HEADER WaveHeader = {
	'FFIR', 0, 'EVAW', // 'RIFF' // 'WAVE'
	' tmf',	16, // 'FMT '
	1, 2, 44100, 176400, 4, 16, // Default redbook 16-bit stereo waveform data
	'atad', 0 // 'DATA'
};

WAVE_FORMAT_EX wfeAudioFormat = {
	1, 2, 44100, 176400, 4, 16, 0
};

#ifdef _DEBUG
	STRUCT_PROTECT(WAVE_FILE_HEADER, 44, WaveHeader);
#endif

#pragma pack()
