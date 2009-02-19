
#ifndef SMARTIOCTL_INCLUDED
#define SMARTIOCTL_INCLUDED

#include <pshpack1.h>

// Miscellaneous

#define	MAX_IDE_DRIVES	4	// Max number of drives assuming primary/secondary, master/slave topology

//
// IOCTL commands
//
#define	DFP_GET_VERSION			0x00074080
#define	DFP_SEND_DRIVE_COMMAND	0x0007c084
#define DFP_RECEIVE_DRIVE_DATA	0x0007c088

//---------------------------------------------------------------------
// GETVERSIONOUTPARAMS contains the data returned from the 
// Get Driver Version function.
//---------------------------------------------------------------------
typedef struct _GETVERSIONOUTPARAMS {
	BYTE	bVersion;		// Binary driver version.
	BYTE	bRevision;		// Binary driver revision.
	BYTE	bReserved;		// Not used.
	BYTE	bIDEDeviceMap;	// Bit map of IDE devices.
	DWORD	fCapabilities;	// Bit mask of driver capabilities.
	DWORD	dwReserved[4];	// For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;

//
// Bits returned in the fCapabilities member of GETVERSIONOUTPARAMS 
//
#define	CAP_IDE_ID_FUNCTION				1	// ATA ID command supported
#define	CAP_IDE_ATAPI_ID				2	// ATAPI ID command supported
#define	CAP_IDE_EXECUTE_SMART_FUNCTION	4	// SMART commannds supported

//---------------------------------------------------------------------
// IDE registers
//---------------------------------------------------------------------

typedef struct _IDEREGS {
	BYTE	bFeaturesReg;		// Used for specifying SMART "commands".
	BYTE	bSectorCountReg;	// IDE sector count register
	BYTE	bSectorNumberReg;	// IDE sector number register
	BYTE	bCylLowReg;			// IDE low order cylinder value
	BYTE	bCylHighReg;		// IDE high order cylinder value
	BYTE	bDriveHeadReg;		// IDE drive/head register
	BYTE	bCommandReg;		// Actual IDE command.
	BYTE	bReserved;			// reserved for future use.  Must be zero.
} IDEREGS, *PIDEREGS, *LPIDEREGS;

//---------------------------------------------------------------------
// SENDCMDINPARAMS contains the input parameters for the 
// Send Command to Drive function.
//---------------------------------------------------------------------

typedef struct _SENDCMDINPARAMS {
	DWORD	cBufferSize;		// Buffer size in bytes
	IDEREGS	irDriveRegs;		// Structure with drive register values.
	BYTE	bDriveNumber;		// Physical drive number to send 
								// command to (0,1,2,3).
	BYTE	bReserved[3];		// Reserved for future expansion.
	DWORD	dwReserved[4];		// For future use.
	BYTE 	bBuffer[1];			// Input buffer.
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;


//
// Valid values for the bCommandReg member of IDEREGS.
//
#define	IDE_ATAPI_ID				0xA1	// Returns ID sector for ATAPI.
#define	IDE_ID_FUNCTION				0xEC	// Returns ID sector for ATA.
#define	IDE_EXECUTE_SMART_FUNCTION	0xB0	// Performs SMART cmd.
											// Requires valid bFeaturesReg,
											// bCylLowReg, and bCylHighReg
//
// Cylinder register values required when issuing SMART command
//
#define	SMART_CYL_LOW	0x4F
#define	SMART_CYL_HI	0xC2

//---------------------------------------------------------------------
// Status returned from driver
//---------------------------------------------------------------------

typedef struct _DRIVERSTATUS {
	BYTE	bDriverError;		// Error code from driver, 
								// or 0 if no error.
	BYTE	bIDEStatus;			// Contents of IDE Error register.
								// Only valid when bDriverError
								// is SMART_IDE_ERROR.
	BYTE	bReserved[2];		// Reserved for future expansion.
	DWORD	dwReserved[2];		// Reserved for future expansion.
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;

//
// bDriverError values
//
#define	SMART_NO_ERROR			0	// No error
#define	SMART_IDE_ERROR			1	// Error from IDE controller
#define	SMART_INVALID_FLAG		2	// Invalid command flag
#define	SMART_INVALID_COMMAND	3	// Invalid command byte
#define	SMART_INVALID_BUFFER	4	// Bad buffer (null, invalid addr..)
#define	SMART_INVALID_DRIVE		5	// Drive number not valid
#define	SMART_INVALID_IOCTL		6	// Invalid IOCTL
#define	SMART_ERROR_NO_MEM		7	// Could not lock user's buffer
#define	SMART_INVALID_REGISTER	8	// Some IDE Register not valid
#define	SMART_NOT_SUPPORTED		9	// Invalid cmd flag set
#define	SMART_NO_IDE_DEVICE		10	// Cmd issued to device not present
									// although drive number is valid
// 11-255 reserved

//---------------------------------------------------------------------
// Structure returned by SMART IOCTL for several commands
//---------------------------------------------------------------------

typedef struct _SENDCMDOUTPARAMS {
	DWORD 			cBufferSize;		// Size of bBuffer in bytes
	DRIVERSTATUS	DriverStatus;		// Driver status structure.
	BYTE			bBuffer[1];			// Buffer of arbitrary length in which to store the data read from the 											// drive.
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;


//---------------------------------------------------------------------
// Feature register defines for SMART "sub commands"
//---------------------------------------------------------------------
#define SMART_READ_ATTRIBUTE_VALUES				0xD0	// ATA4: Renamed 
														// SMART READ DATA
#define	SMART_READ_ATTRIBUTE_THRESHOLDS			0xD1	// Obsoleted in ATA4!
#define SMART_ENABLE_DISABLE_ATTRIBUTE_AUTOSAVE	0xD2
#define SMART_SAVE_ATTRIBUTE_VALUES				0xD3
#define	SMART_EXECUTE_OFFLINE_IMMEDIATE			0xD4	// ATA4
// Vendor specific commands:
#define	SMART_ENABLE_SMART_OPERATIONS			0xD8
#define	SMART_DISABLE_SMART_OPERATIONS			0xD9
#define	SMART_RETURN_SMART_STATUS				0xDA

#endif

//---------------------------------------------------------------------
// The following structure defines the structure of a Drive Attribute
//---------------------------------------------------------------------
typedef	struct	_DRIVEATTRIBUTE {
	BYTE	bAttrID;		// Identifies which attribute
	WORD	wStatusFlags;	// see bit definitions below
	BYTE	bAttrValue;		// Current normalized value
	BYTE	bWorstValue;	// How bad has it ever been?
	BYTE	bRawValue[6];	// Un-normalized value
	BYTE	bReserved;		// ...
} DRIVEATTRIBUTE, *PDRIVEATTRIBUTE, *LPDRIVEATTRIBUTE;

//---------------------------------------------------------------------
// The following structure defines the structure of a Warranty Threshold
// Obsoleted in ATA4!
//---------------------------------------------------------------------
typedef	struct	_ATTRTHRESHOLD {
	BYTE	bAttrID;			// Identifies which attribute
	BYTE	bWarrantyThreshold;	// Triggering value
	BYTE	bReserved[10];		// ...
} ATTRTHRESHOLD, *PATTRTHRESHOLD, *LPATTRTHRESHOLD;

//---------------------------------------------------------------------
// The following struct defines the interesting part of the IDENTIFY
// buffer:
//---------------------------------------------------------------------
typedef struct _IDSECTOR {
	USHORT	wGenConfig;
	USHORT	wNumCyls;
	USHORT	wReserved;
	USHORT	wNumHeads;
	USHORT	wBytesPerTrack;
	USHORT	wBytesPerSector;
	USHORT	wSectorsPerTrack;
	USHORT	wVendorUnique[3];
	CHAR	sSerialNumber[20];
	USHORT	wBufferType;
	USHORT	wBufferSize;
	USHORT	wECCSize;
	CHAR	sFirmwareRev[8];
	CHAR	sModelNumber[40];
	USHORT	wMoreVendorUnique;
	USHORT	wDoubleWordIO;
	USHORT	wCapabilities;
	USHORT	wReserved1;
	USHORT	wPIOTiming;
	USHORT	wDMATiming;
	USHORT	wBS;
	USHORT	wNumCurrentCyls;
	USHORT	wNumCurrentHeads;
	USHORT	wNumCurrentSectorsPerTrack;
	ULONG	ulCurrentSectorCapacity;
	USHORT	wMultSectorStuff;
	ULONG	ulTotalAddressableSectors;
	USHORT	wSingleWordDMA;
	USHORT	wMultiWordDMA;
	BYTE	bReserved[128];
} IDSECTOR, *PIDSECTOR;

//---------------------------------------------------------------------
// Valid Attribute IDs
//---------------------------------------------------------------------
#define	ATTR_INVALID				0
#define ATTR_READ_ERROR_RATE		1
#define ATTR_THROUGHPUT_PERF		2
#define ATTR_SPIN_UP_TIME			3
#define ATTR_START_STOP_COUNT		4
#define ATTR_REALLOC_SECTOR_COUNT	5
#define ATTR_READ_CHANNEL_MARGIN	6
#define ATTR_SEEK_ERROR_RATE		7
#define ATTR_SEEK_TIME_PERF			8
#define ATTR_POWER_ON_HRS_COUNT		9
#define ATTR_SPIN_RETRY_COUNT		10
#define ATTR_CALIBRATION_RETRY_COUNT 11
#define ATTR_POWER_CYCLE_COUNT		12

//---------------------------------------------------------------------
// Status Flags Values
//---------------------------------------------------------------------
#define	PRE_FAILURE_WARRANTY		0x1
#define	ON_LINE_COLLECTION			0x2
#define	PERFORMANCE_ATTRIBUTE		0x4
#define	ERROR_RATE_ATTRIBUTE		0x8
#define	EVENT_COUNT_ATTRIBUTE		0x10
#define	SELF_PRESERVING_ATTRIBUTE	0x20

#define	NUM_ATTRIBUTE_STRUCTS		 30

#include <poppack.h>
