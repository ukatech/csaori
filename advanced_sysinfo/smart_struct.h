
#ifndef STRUCT_SMARTIOCTL_INCLUDED
#define STRUCT_SMARTIOCTL_INCLUDED

#include <pshpack1.h>

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

#endif //STRUCT_SMARTIOCTL_INCLUDED

