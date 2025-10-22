#include <global.h>
#include <scsidefs.h>
#include <winioctl.h>
#include <my_ntddcdrm.h>
#include <my_ntddscsi.h>

typedef BYTE BIT;
typedef BYTE BITS;
#define SPT_SENSE_LENGTH 19

#pragma pack(1)
struct DVD_DISC_STRUCTURE_00 {
	BITS PartVersion  : 4;
	BITS DiskCategory : 4;
	BITS MaximumRate  : 4;
	BITS DiscSize     : 4;
	BITS LayerType      : 4;
	BITS TrackPath      : 1;
	BITS NumberOfLayers : 2;
	BITS Reserved1      : 1;
	BITS TrackDensity   : 4;
	BITS LinearDensity  : 4;
	BYTE Null00;
	BYTE StartingPhysicalSector[3];
	BYTE Null01;
	BYTE EndPhysicalSector[3];
	BYTE Null02;
	BYTE EndPhysicalSectorLayer0[3];
	BITS Reserved2 : 7;
	BITS BCA       : 1;
};
#pragma pack()

// SPTI will NOT work if this structure is not DWORD aligned AND DeviceIoControl
// KNOWS if you don't use the pack(4) directive; manually aligning won't work!!!
#pragma pack(4)
struct SPTI_EXEC_SCSI_CMD {
	SCSI_PASS_THROUGH_DIRECT sptd;
	SENSE_DATA_FMT SenseArea;
};
#pragma pack()
//
// Command Descriptor Block constants.
//
#define CDB6GENERIC_LENGTH   6
#define CDB10GENERIC_LENGTH  10
#define CDB12GENERIC_LENGTH  12
#define SETBITOFF  0
#define SETBITON   1
//
// Mode Sense/Select page constants.
//
#define MODE_PAGE_ERROR_RECOVERY        0x01
#define MODE_PAGE_DISCONNECT            0x02
#define MODE_PAGE_FORMAT_DEVICE         0x03
#define MODE_PAGE_RIGID_GEOMETRY        0x04
#define MODE_PAGE_FLEXIBILE             0x05
#define MODE_PAGE_VERIFY_ERROR          0x07
#define MODE_PAGE_CACHING               0x08
#define MODE_PAGE_PERIPHERAL            0x09
#define MODE_PAGE_CONTROL               0x0A
#define MODE_PAGE_MEDIUM_TYPES          0x0B
#define MODE_PAGE_NOTCH_PARTITION       0x0C
#define MODE_SENSE_RETURN_ALL           0x3F
#define MODE_SENSE_CURRENT_VALUES       0x00
#define MODE_SENSE_CHANGEABLE_VALUES    0x40
#define MODE_SENSE_DEFAULT_VAULES       0x80
#define MODE_SENSE_SAVED_VALUES         0xC0
#define MODE_PAGE_DEVICE_CONFIG         0x10
#define MODE_PAGE_MEDIUM_PARTITION      0x11
#define MODE_PAGE_DATA_COMPRESS         0x0F

//
// SCSI CDB operation codes
//

#define SCSIOP_TEST_UNIT_READY     0x00
#define SCSIOP_REZERO_UNIT         0x01
#define SCSIOP_REWIND              0x01
#define SCSIOP_REQUEST_BLOCK_ADDR  0x02
#define SCSIOP_REQUEST_SENSE       0x03
#define SCSIOP_FORMAT_UNIT         0x04
#define SCSIOP_READ_BLOCK_LIMITS   0x05
#define SCSIOP_REASSIGN_BLOCKS     0x07
#define SCSIOP_READ6               0x08
#define SCSIOP_RECEIVE             0x08
#define SCSIOP_WRITE6              0x0A
#define SCSIOP_PRINT               0x0A
#define SCSIOP_SEND                0x0A
#define SCSIOP_SEEK6               0x0B
#define SCSIOP_TRACK_SELECT        0x0B
#define SCSIOP_SLEW_PRINT          0x0B
#define SCSIOP_SEEK_BLOCK          0x0C
#define SCSIOP_PARTITION           0x0D
#define SCSIOP_READ_REVERSE        0x0F
#define SCSIOP_WRITE_FILEMARKS     0x10
#define SCSIOP_FLUSH_BUFFER        0x10
#define SCSIOP_SPACE               0x11
#define SCSIOP_INQUIRY             0x12
#define SCSIOP_VERIFY6             0x13
#define SCSIOP_RECOVER_BUF_DATA    0x14
#define SCSIOP_MODE_SELECT         0x15
#define SCSIOP_RESERVE_UNIT        0x16
#define SCSIOP_RELEASE_UNIT        0x17
#define SCSIOP_COPY                0x18
#define SCSIOP_ERASE               0x19
#define SCSIOP_MODE_SENSE          0x1A
#define SCSIOP_START_STOP_UNIT     0x1B
#define SCSIOP_STOP_PRINT          0x1B
#define SCSIOP_LOAD_UNLOAD         0x1B
#define SCSIOP_RECEIVE_DIAGNOSTIC  0x1C
#define SCSIOP_SEND_DIAGNOSTIC     0x1D
#define SCSIOP_MEDIUM_REMOVAL      0x1E
#define SCSIOP_READ_CAPACITY       0x25
#define SCSIOP_READ                0x28
#define SCSIOP_WRITE               0x2A
#define SCSIOP_SEEK                0x2B
#define SCSIOP_LOCATE              0x2B
#define SCSIOP_WRITE_VERIFY        0x2E
#define SCSIOP_VERIFY              0x2F
#define SCSIOP_SEARCH_DATA_HIGH    0x30
#define SCSIOP_SEARCH_DATA_EQUAL   0x31
#define SCSIOP_SEARCH_DATA_LOW     0x32
#define SCSIOP_SET_LIMITS          0x33
#define SCSIOP_READ_POSITION       0x34
#define SCSIOP_SYNCHRONIZE_CACHE   0x35
#define SCSIOP_COMPARE             0x39
#define SCSIOP_COPY_COMPARE        0x3A
#define SCSIOP_WRITE_DATA_BUFF     0x3B
#define SCSIOP_READ_DATA_BUFF      0x3C
#define SCSIOP_CHANGE_DEFINITION   0x40
#define SCSIOP_READ_SUB_CHANNEL    0x42
#define SCSIOP_READ_TOC            0x43
#define SCSIOP_READ_HEADER         0x44
#define SCSIOP_PLAY_AUDIO          0x45
#define SCSIOP_PLAY_AUDIO_MSF      0x47
#define SCSIOP_PLAY_TRACK_INDEX    0x48
#define SCSIOP_PLAY_TRACK_RELATIVE 0x49
#define SCSIOP_PAUSE_RESUME        0x4B
#define SCSIOP_LOG_SELECT          0x4C
#define SCSIOP_LOG_SENSE           0x4D

// Function Prototypes

BOOL   ToggleDiscTray(LPSTR, BOOL);
BOOL   SPTIToggleDiscTray(HANDLE, BOOL);
HANDLE SPTIGetCdRomHandle(CHAR cDriveLetter);
BOOL   SPTIToggleMediaRemoval(HANDLE, BOOLEAN);
BOOL   SPTIStartStopDevice(HANDLE);
BOOL   SPTIIsMediaPresent(HANDLE);

DWORD  SPTIGetDiscType(HANDLE);
BOOL   SPTIGetTableOfContentsLBA(HANDLE, CDROM_TOC*);
BOOL   SPTIGetTableOfContentsMSF(HANDLE, CDROM_TOC*);
DWORD  SPTIGetDVDType(HANDLE, DVD_DISC_STRUCTURE_00*);
