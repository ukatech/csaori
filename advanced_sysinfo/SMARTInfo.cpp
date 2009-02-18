// SMARTInfo.cpp: CSMARTInfo クラスのインプリメンテーション

#include "SMARTInfo.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "smart_struct.h"

/*-------------------------------------------------------
	構築/消滅
-------------------------------------------------------*/

CSMARTInfo::CSMARTInfo()
{
}

CSMARTInfo::~CSMARTInfo()
{
	Release();
}

/*-------------------------------------------------------
	初期化ヘルパー関数群
-------------------------------------------------------*/
static BOOL DoIDENTIFY(HANDLE hSMARTIOCTL, PSENDCMDINPARAMS pSCIP,
	PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum, PDWORD lpcbBytesReturned)
{
	//
	// Set up data structures for IDENTIFY command.
	//

	pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;

	pSCIP->irDriveRegs.bFeaturesReg = 0;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = 0;
	pSCIP->irDriveRegs.bCylHighReg = 0;

	//
	// Compute the drive number.
	//
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 

	//
	// The command can either be IDE identify or ATAPI identify.
	//
	pSCIP->irDriveRegs.bCommandReg = bIDCmd;
	pSCIP->bDriveNumber = bDriveNum;
	pSCIP->cBufferSize = IDENTIFY_BUFFER_SIZE;

    return (DeviceIoControl(hSMARTIOCTL, DFP_RECEIVE_DRIVE_DATA,
             	(LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1,
               	(LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
               	lpcbBytesReturned, NULL) );
}

static BOOL DoEnableSMART(HANDLE hSMARTIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum, PDWORD lpcbBytesReturned)
{
	//
	// Set up data structures for Enable SMART Command.
	//
	pSCIP->cBufferSize = 0;

	pSCIP->irDriveRegs.bFeaturesReg = SMART_ENABLE_SMART_OPERATIONS;
	pSCIP->irDriveRegs.bSectorCountReg = 1;
	pSCIP->irDriveRegs.bSectorNumberReg = 1;
	pSCIP->irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	pSCIP->irDriveRegs.bCylHighReg = SMART_CYL_HI;

	//
	// Compute the drive number.
	//
	pSCIP->irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 
	pSCIP->irDriveRegs.bCommandReg = IDE_EXECUTE_SMART_FUNCTION;
	pSCIP->bDriveNumber = bDriveNum;

        return ( DeviceIoControl(hSMARTIOCTL, DFP_SEND_DRIVE_COMMAND,
                (LPVOID)pSCIP, sizeof(SENDCMDINPARAMS) - 1,
                (LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) - 1,
                lpcbBytesReturned, NULL) );
}

/*-------------------------------------------------------
	初期化
-------------------------------------------------------*/
bool CSMARTInfo::Init()
{
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&os);


	HANDLE hSMARTIOCTL;
	if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
		//9x系：SMARTVSD.VXD
		hSMARTIOCTL = CreateFile("\\\\.\\SMARTVSD", 0,0,0,CREATE_NEW, 0, 0);
	}
	else {
		//NT系：ドライブオープン
		hSMARTIOCTL = CreateFile("\\\\.\\PhysicalDrive0",GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	}
	if ( hSMARTIOCTL == INVALID_HANDLE_VALUE ) {
		return false;
	}

	DWORD cbBytesReturned;

	//VERSION
	GETVERSIONOUTPARAMS VersionParams;
	ZeroMemory(&VersionParams,sizeof(VersionParams));

	if ( ! ::DeviceIoControl(hSMARTIOCTL, DFP_GET_VERSION,NULL, 0,
		&VersionParams,sizeof(VersionParams),&cbBytesReturned, NULL) ) {
		::CloseHandle(hSMARTIOCTL);
		return false;
	}

	SENDCMDINPARAMS 	scip;
	SENDCMDOUTPARAMS	OutCmd;

	bool bDfpDriveMap[MAX_IDE_DRIVES];
	ZeroMemory(&bDfpDriveMap,sizeof(bDfpDriveMap));

	for ( int i = 0; i < MAX_IDE_DRIVES; ++i ) {
		if ( VersionParams.bIDEDeviceMap >> i & 1 ) {
			if ( !(VersionParams.bIDEDeviceMap >> i & 0x10) ) {
				ZeroMemory(&scip, sizeof(scip));
				ZeroMemory(&OutCmd, sizeof(OutCmd));

				if ( DoEnableSMART(hSMARTIOCTL,&scip,&OutCmd,i,&cbBytesReturned) ) {
					bDfpDriveMap[i] = true;
				}
			}
		}
	}
}

void CSMARTInfo::Release()
{

}
