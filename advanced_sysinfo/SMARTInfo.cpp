// SMARTInfo.cpp: CSMARTInfo クラスのインプリメンテーション

#include "SMARTInfo.h"

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

CSMARTInfo::CSMARTInfo()
{
	m_initTime = 0;
}

CSMARTInfo::~CSMARTInfo()
{
	Release();
}

/*-------------------------------------------------------
	取得
-------------------------------------------------------*/

CDriveSmartInfo *CSMARTInfo::GetInfo(int id)
{
	size_t n = m_driveInfo.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		if ( m_driveInfo[i].m_driveID == id ) {
			return &(m_driveInfo[i]);
		}
	}
	return NULL;
}

/*-------------------------------------------------------
	初期化ヘルパー関数群
-------------------------------------------------------*/
static BOOL DoReadSMARTCmd(HANDLE hSMARTIOCTL, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum, BYTE cmd, DWORD bufsize)
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
	scip.irDriveRegs.bCommandReg = IDE_EXECUTE_SMART_FUNCTION;
	scip.bDriveNumber = bDriveNum;

	return ( DeviceIoControl(hSMARTIOCTL, DFP_RECEIVE_DRIVE_DATA,
		(LPVOID)&scip, sizeof(SENDCMDINPARAMS) - 1,
		(LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + bufsize - 1,
		&cbBytesReturned, NULL) );
}

static BOOL DoSendSMARTCmd(HANDLE hSMARTIOCTL, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum, BYTE cmd, DWORD bufsize)
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
	scip.irDriveRegs.bCommandReg = IDE_EXECUTE_SMART_FUNCTION;
	scip.bDriveNumber = bDriveNum;

	return ( DeviceIoControl(hSMARTIOCTL, DFP_SEND_DRIVE_COMMAND,
		(LPVOID)&scip, sizeof(SENDCMDINPARAMS) - 1,
		(LPVOID)pSCOP, sizeof(SENDCMDOUTPARAMS) + bufsize - 1,
		&cbBytesReturned, NULL) );
}

static BOOL DoIDCmd(HANDLE hSMARTIOCTL, PSENDCMDOUTPARAMS pSCOP, BYTE bDriveNum,DWORD bufsize)
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
	scip.irDriveRegs.bCommandReg = IDE_ID_FUNCTION;
	scip.bDriveNumber = bDriveNum;

	return ( DeviceIoControl(hSMARTIOCTL, DFP_RECEIVE_DRIVE_DATA,
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
#define READ_BUFFER_SIZE 4096

bool CSMARTInfo::Init()
{
	DWORD current = ::timeGetTime();
	if ( (current - m_initTime) < 60000 ) { return true; } //1分間キャッシュ
	m_initTime = current;

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

	//バージョンと取り付けられているハードウェアの場所チェック
	GETVERSIONOUTPARAMS VersionParams;
	ZeroMemory(&VersionParams,sizeof(VersionParams));

	DWORD cbBytesReturned;

	if ( ! ::DeviceIoControl(hSMARTIOCTL, DFP_GET_VERSION,NULL, 0,
		&VersionParams,sizeof(VersionParams),&cbBytesReturned, NULL) ) {
		::CloseHandle(hSMARTIOCTL);
		return false;
	}

	SENDCMDOUTPARAMS	OutCmd;
	bool found = false;

	bool bDfpDriveMap[MAX_IDE_DRIVES];
	ZeroMemory(&bDfpDriveMap,sizeof(bDfpDriveMap));

	//ビット0～3 IDE Primary Master -> Secondary Slave  ビット4～7 ATAPIがついてる場合
	int i;
	for ( i = 0; i < MAX_IDE_DRIVES; ++i ) {
		if ( VersionParams.bIDEDeviceMap >> i & 1 ) {
			if ( !(VersionParams.bIDEDeviceMap >> i & 0x10) ) {
				ZeroMemory(&OutCmd, sizeof(OutCmd));

				if ( DoSendSMARTCmd(hSMARTIOCTL,&OutCmd,i,SMART_ENABLE_SMART_OPERATIONS,0) ) {
					bDfpDriveMap[i] = true;
					found = true;
				}
			}
		}
	}

	if ( ! found ) {
		::CloseHandle(hSMARTIOCTL);
		return false;
	}

	SENDCMDOUTPARAMS* OutCmdData = reinterpret_cast<SENDCMDOUTPARAMS*>(calloc(sizeof(SENDCMDOUTPARAMS) + READ_BUFFER_SIZE + 1,1));
	OutCmdData->cBufferSize = READ_BUFFER_SIZE;

	SENDCMDOUTPARAMS* OutCmdDataThreshold = reinterpret_cast<SENDCMDOUTPARAMS*>(calloc(sizeof(SENDCMDOUTPARAMS) + READ_BUFFER_SIZE + 1,1));
	OutCmdDataThreshold->cBufferSize = READ_BUFFER_SIZE;

	//全ドライブの詳細情報と
	for ( i = 0; i < MAX_IDE_DRIVES; ++i ) {
		if ( ! bDfpDriveMap[i] ) { continue; }

		CDriveSmartInfo inf;
		inf.m_driveID = i;

		//IDENTIFY
		if ( DoIDCmd(hSMARTIOCTL,OutCmdData,i,READ_BUFFER_SIZE) ) {
			inf.m_sector = *reinterpret_cast<IDSECTOR*>(OutCmdData->bBuffer);

			ChangeByteOrder(inf.m_sector.sModelNumber,sizeof(inf.m_sector.sModelNumber));
			ChangeByteOrder(inf.m_sector.sFirmwareRev,sizeof(inf.m_sector.sFirmwareRev));
			ChangeByteOrder(inf.m_sector.sSerialNumber,sizeof(inf.m_sector.sSerialNumber));

			//Attr+Threshold
			if ( DoReadSMARTCmd(hSMARTIOCTL,OutCmdData,
				i,SMART_READ_ATTRIBUTE_VALUES,READ_BUFFER_SIZE) ) {

				if ( DoReadSMARTCmd(hSMARTIOCTL,OutCmdDataThreshold,
					i,SMART_READ_ATTRIBUTE_THRESHOLDS,READ_BUFFER_SIZE) ) {

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

					m_driveInfo.push_back(inf);
				}
			}
		}
	}

	free(OutCmdData);
	free(OutCmdDataThreshold);

	return m_driveInfo.size() != 0;
}

void CSMARTInfo::Release()
{
	m_driveInfo.clear();
	m_initTime = 0;
}
