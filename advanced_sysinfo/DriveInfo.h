// DriveInfo.h: CDriveInfo クラスのインターフェイス

#if !defined(AFX_DriveINFO_H__9EED1D90_5944_4B4D_AE7D_1EA1A06D793B__INCLUDED_)
#define AFX_DriveINFO_H__9EED1D90_5944_4B4D_AE7D_1EA1A06D793B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>

#include "smart_struct.h"

typedef struct tagDRIVEATTRTHRESH {
	DRIVEATTRIBUTE attr;
	ATTRTHRESHOLD thresh;
} DRIVEATTRTHRESH;

class CDriveSmartInfo {
public:
	int m_driveID;
	IDSECTOR m_sector;
	DWORD m_structRev;
	std::vector<DRIVEATTRTHRESH> m_smartParams;
};

class CDriveInfo  
{
private:
	std::vector<CDriveSmartInfo> m_driveInfo;
	DWORD m_initTime;

	bool InitOneDrive(HANDLE hDriveIOCTL,int driveID,DWORD capability);

public:
	CDriveInfo();
	virtual ~CDriveInfo();

	void Release(void);
	bool Init(void);
	
	CDriveSmartInfo *GetInfo(int id);
};

#endif // !defined(AFX_DriveINFO_H__9EED1D90_5944_4B4D_AE7D_1EA1A06D793B__INCLUDED_)
