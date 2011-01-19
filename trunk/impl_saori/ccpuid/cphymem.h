#ifndef CPHYMEM_H
#define CPHYMEM_H

#include <windows.h>
#include "native.h"

class CPhyMem
{
private:
	PBYTE	type[4];
	PBYTE	ii;
	char	buf[1024*64*3];
	HANDLE	physmem;
	DWORD	vaddress, paddress, length;
	PBYTE MemSearch(PBYTE ToSearch,long size,const PBYTE StartAddr,const PBYTE endAddr);
	bool CopyDmiMemory(PBYTE buf,DWORD phyAddr,DWORD size);
public:
	PBYTE GetType(int num);
	CPhyMem();
	~CPhyMem();
	static const int bsize;
};

BOOL GetDMIInfo(LPSTR dmiBIOSVendor, LPSTR dmiBIOSVersion, LPSTR dmiBIOSReleaseDate,
				LPSTR dmiSysManufacturer, LPSTR dmiSysProductName, LPSTR dmiSysVersion,
				LPSTR dmiSysSerialNumber, LPSTR dmiMBManufacturer, LPSTR dmiMBProduct,
				LPSTR dmiMBVersion, LPSTR dmiMBSerialNumber, LPSTR dmiChassisManufacturer,
				LPSTR dmiChassisVersion, LPSTR dmiChassisSerialNumber, LPSTR dmiChassisAssetTagNumber);

#endif
