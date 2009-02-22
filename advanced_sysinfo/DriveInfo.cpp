// DriveInfo.cpp: CDriveInfo クラスのインプリメンテーション

#include "DriveInfo.h"
#include <winioctl.h>

#include <stdlib.h>
#include <mmsystem.h>

//////////DEBUG/////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
////////////////////////////////////////

/*-------------------------------------------------------
	構築/消滅
-------------------------------------------------------*/

CDriveInfo::CDriveInfo()
{
	m_initTime = 0;
}

CDriveInfo::~CDriveInfo()
{
	Release();
}

/*-------------------------------------------------------
	取得
-------------------------------------------------------*/

CDriveSmartInfo *CDriveInfo::GetInfo(int id)
{
	int n = m_driveInfo.size();
	if ( id < n ) {
		return &(m_driveInfo[id]);
	}
	return NULL;
}

/*-------------------------------------------------------
	初期化ヘルパー関数群
-------------------------------------------------------*/
static BOOL DoReadDriveCmd(HANDLE hDriveIOCTL, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum, BYTE cmd, DWORD bufsize)
{
	DWORD	cbBytesReturned;

	SENDCMDINPARAMS scip;
	ZeroMemory(&scip,sizeof(scip));

	scip.cBufferSize = bufsize;

	scip.irDriveRegs.bFeaturesReg = cmd;
	scip.irDriveRegs.bSectorCountReg = 1;
	scip.irDriveRegs.bSectorNumberReg = 1;
	scip.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	scip.irDriveRegs.bCylHighReg = SMART_CYL_HI;
	scip.irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 
	scip.irDriveRegs.bCommandReg = SMART_CMD; //ATAPI_ID_CMD,ID_CMD,Drive_CMD
	scip.bDriveNumber = bDriveNum;

	return ( DeviceIoControl(hDriveIOCTL, SMART_RCV_DRIVE_DATA,
		(LPVOID)&scip, sizeof(SENDCMDINPARAMS) - 1,
		(LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + bufsize - 1,
		&cbBytesReturned, NULL) );
}

static BOOL DoSendDriveCmd(HANDLE hDriveIOCTL, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum, BYTE cmd, DWORD bufsize)
{
	DWORD	cbBytesReturned;

	SENDCMDINPARAMS 	scip;
	ZeroMemory(&scip,sizeof(scip));

	scip.cBufferSize = bufsize;

	scip.irDriveRegs.bFeaturesReg = cmd;
	scip.irDriveRegs.bSectorCountReg = 1;
	scip.irDriveRegs.bSectorNumberReg = 1;
	scip.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	scip.irDriveRegs.bCylHighReg = SMART_CYL_HI;
	scip.irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 
	scip.irDriveRegs.bCommandReg = SMART_CMD; //ATAPI_ID_CMD,ID_CMD,Drive_CMD
	scip.bDriveNumber = bDriveNum;

	return ( DeviceIoControl(hDriveIOCTL, SMART_SEND_DRIVE_COMMAND,
		(LPVOID)&scip, sizeof(SENDCMDINPARAMS) - 1,
		(LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + bufsize - 1,
		&cbBytesReturned, NULL) );
}

static BOOL DoIDCmd(HANDLE hDriveIOCTL, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum, bool isATAPI, DWORD bufsize)
{
	DWORD	cbBytesReturned;

	SENDCMDINPARAMS 	scip;
	ZeroMemory(&scip,sizeof(scip));

	scip.cBufferSize = bufsize;

	scip.irDriveRegs.bFeaturesReg = 0;
	scip.irDriveRegs.bSectorCountReg = 1;
	scip.irDriveRegs.bSectorNumberReg = 1;
	//scip.irDriveRegs.bCylLowReg = 0;
	//scip.irDriveRegs.bCylHighReg = 0;
	scip.irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4); 
	scip.irDriveRegs.bCommandReg = isATAPI ? ATAPI_ID_CMD : ID_CMD; //ATAPI_ID_CMD,ID_CMD,Drive_CMD
	scip.bDriveNumber = bDriveNum;

	return ( DeviceIoControl(hDriveIOCTL, SMART_RCV_DRIVE_DATA,
		(LPVOID)&scip, sizeof(SENDCMDINPARAMS) - 1,
		(LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + bufsize - 1,
		&cbBytesReturned, NULL) );
}

static void ChangeByteOrder(PCHAR szString, USHORT uscStrSize)
{
	USHORT	i;
	CHAR	temp;

	for (i = 0; i < uscStrSize; i+=2) {
		temp = szString[i];
		szString[i] = szString[i+1];
		szString[i+1] = temp;
	}
}

/*-------------------------------------------------------
	初期化
-------------------------------------------------------*/
#define READ_BUFFER_SIZE      4096
#define	MAX_IDE_DRIVES_9X     4
#define	MAX_IDE_DRIVES_NT     16

bool CDriveInfo::Init()
{
	DWORD current = ::timeGetTime();
	if ( (current - m_initTime) < 300*1000 ) { return true; } //5分間キャッシュ

	m_driveInfo.clear();
	m_initTime = current;

	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&os);
	int found = 0;

	if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) { //W9x
		//9x系：DriveVSD.VXD
		HANDLE hDriveIOCTL = CreateFile("\\\\.\\DriveVSD", 0,0,0,CREATE_NEW, 0, 0);
		if ( hDriveIOCTL == INVALID_HANDLE_VALUE ) {
			return false;
		}

		//バージョンと取り付けられているハードウェアの場所チェック
		GETVERSIONINPARAMS VersionParams;
		ZeroMemory(&VersionParams,sizeof(VersionParams));

		DWORD cbBytesReturned;

		if ( ! ::DeviceIoControl(hDriveIOCTL, SMART_GET_VERSION,NULL, 0,
			&VersionParams,sizeof(VersionParams),&cbBytesReturned, NULL) ) {
			::CloseHandle(hDriveIOCTL);
			return false;
		}

		//ビット0～15 IDE Primary Master -> Secondary Slave  ビット16～3 ATAPIがついてる場合
		int i;
		for ( i = 0; i < MAX_IDE_DRIVES_9X; ++i ) {
			if ( VersionParams.bIDEDeviceMap >> i & 1 ) {
				if ( !(VersionParams.bIDEDeviceMap >> i & 0x10) ) {
					if ( InitOneDrive(hDriveIOCTL,i,CAP_ATA_ID_CMD | CAP_SMART_CMD) ) {
						++found;
					}
				}
				else {
					if ( InitOneDrive(hDriveIOCTL,i,CAP_ATA_ID_CMD | CAP_ATAPI_ID_CMD) ) {
						++found;
					}
				}
			}
		}

		::CloseHandle(hDriveIOCTL);
	}
	else { //WNT
		char physicalDriveName[32];
		GETVERSIONINPARAMS VersionParams;
		DWORD cbBytesReturned;

		for ( size_t i = 0 ; i < MAX_IDE_DRIVES_NT ; ++i ) {

			sprintf(physicalDriveName,"\\\\.\\PhysicalDrive%u",i);

			HANDLE hDriveIOCTL = CreateFile(physicalDriveName,GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

			if ( hDriveIOCTL == INVALID_HANDLE_VALUE ) { continue; }

			ZeroMemory(&VersionParams,sizeof(VersionParams));
			if ( ! ::DeviceIoControl(hDriveIOCTL, SMART_GET_VERSION,NULL, 0,
				&VersionParams,sizeof(VersionParams),&cbBytesReturned, NULL) ) {
				::CloseHandle(hDriveIOCTL);
				continue;
			}

			if ( InitOneDrive(hDriveIOCTL,i,VersionParams.fCapabilities) ) {
				++found;
			}

			::CloseHandle(hDriveIOCTL);
		}
	}
	return true;
}

bool CDriveInfo::InitOneDrive(HANDLE hDriveIOCTL,int driveID,DWORD capability)
{
	if ( capability & CAP_SMART_CMD ) {
		SENDCMDOUTPARAMS	OutCmd;
		ZeroMemory(&OutCmd, sizeof(OutCmd));

		if ( ! DoSendDriveCmd(hDriveIOCTL,&OutCmd,driveID,ENABLE_SMART,0) ) {
			return false;
		}
	}

	SENDCMDOUTPARAMS* OutCmdData = reinterpret_cast<SENDCMDOUTPARAMS*>(calloc(sizeof(SENDCMDOUTPARAMS) + READ_BUFFER_SIZE + 1,1));
	OutCmdData->cBufferSize = READ_BUFFER_SIZE;

	SENDCMDOUTPARAMS* OutCmdDataThreshold = reinterpret_cast<SENDCMDOUTPARAMS*>(calloc(sizeof(SENDCMDOUTPARAMS) + READ_BUFFER_SIZE + 1,1));
	OutCmdDataThreshold->cBufferSize = READ_BUFFER_SIZE;

	//ドライブの詳細情報
	CDriveSmartInfo inf;
	inf.m_driveID = driveID;
	bool found = false;

	//IDENTIFY
	if ( DoIDCmd(hDriveIOCTL,OutCmdData,driveID,false,READ_BUFFER_SIZE) ) {
		inf.m_sector = *reinterpret_cast<IDSECTOR*>(OutCmdData->bBuffer);

		ChangeByteOrder(inf.m_sector.sModelNumber,sizeof(inf.m_sector.sModelNumber));
		ChangeByteOrder(inf.m_sector.sFirmwareRev,sizeof(inf.m_sector.sFirmwareRev));
		ChangeByteOrder(inf.m_sector.sSerialNumber,sizeof(inf.m_sector.sSerialNumber));

		//Attr+Threshold
		if ( capability & CAP_SMART_CMD ) {
			if ( DoReadDriveCmd(hDriveIOCTL,OutCmdData,driveID,READ_ATTRIBUTES,READ_BUFFER_SIZE) ) {

				if ( DoReadDriveCmd(hDriveIOCTL,OutCmdDataThreshold,driveID,READ_THRESHOLDS,READ_BUFFER_SIZE) ) {

					inf.m_structRev = *reinterpret_cast<WORD*>(OutCmdData->bBuffer);

					size_t n = (READ_BUFFER_SIZE-2) / sizeof(DRIVEATTRIBUTE);

					DRIVEATTRIBUTE* pAttr = reinterpret_cast<DRIVEATTRIBUTE*>(OutCmdData->bBuffer + 2);
					ATTRTHRESHOLD* pThresh = reinterpret_cast<ATTRTHRESHOLD*>(OutCmdDataThreshold->bBuffer + 2);

					for ( size_t i = 0 ; i < n ; ++i ) {
						if ( ! pAttr->bAttrID ) { break; }

						DRIVEATTRTHRESH t;
						t.attr = *pAttr;
						t.thresh = *pThresh;

						inf.m_smartParams.push_back(t);

						++pAttr;
						++pThresh;
					}

				}
			}
		}

		found = true;
		m_driveInfo.push_back(inf);
	}

	free(OutCmdData);
	free(OutCmdDataThreshold);

	return found;
}

void CDriveInfo::Release()
{
	m_driveInfo.clear();
	m_initTime = 0;
}
