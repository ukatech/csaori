/******************************************************************
*
*	Wiimote for C++
*
*	Copyright (C) Satoshi Konno 2007
*
*	File:	UsbHID.cpp
*
******************************************************************/

#include <UsbHID.h>

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

using namespace CyberGarage;

////////////////////////////////////////////////
//	Constructor
////////////////////////////////////////////////

UsbHID::UsbHID() 
{
	mDevIfDetailData = NULL;
	mDevHandle = NULL;
	mInputBuf = NULL;
	mOutputBuf = NULL;

	mDevEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	memset(&mOverLapped, 0, sizeof(OVERLAPPED));
	mOverLapped.hEvent = mDevEvent;
	mOverLapped.Offset = 0;
	mOverLapped.OffsetHigh = 0;
}

////////////////////////////////////////////////
//	Destructor
////////////////////////////////////////////////

UsbHID::~UsbHID()
{
	close();
	CloseHandle(mDevEvent);
}

////////////////////////////////////////////////
//	open
////////////////////////////////////////////////

bool UsbHID::open(int venderID, int productID)
{
	close();

	GUID guid;
	HidD_GetHidGuid(&guid);

	HDEVINFO devInfoSet = SetupDiGetClassDevs(&guid, 0, 0, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE ));
	if (devInfoSet == INVALID_HANDLE_VALUE)
		return false;

	SP_DEVICE_INTERFACE_DATA devIf;
	devIf.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	for (int i=0; SetupDiEnumDeviceInterfaces(devInfoSet, NULL, &guid, i, &devIf); i++) {
		DWORD devIfDetailSize = 0;
		SetupDiGetDeviceInterfaceDetail(devInfoSet, &devIf, NULL, 0, &devIfDetailSize, 0);
		if (devIfDetailSize <=0)
			continue;
		SP_DEVICE_INTERFACE_DETAIL_DATA *devIfDetail = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(devIfDetailSize);
		devIfDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
		if (SetupDiGetDeviceInterfaceDetail(devInfoSet, &devIf, devIfDetail, devIfDetailSize, &devIfDetailSize, 0) != 0) {
			HANDLE devHandle = CreateFile(devIfDetail->DevicePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if(devHandle == INVALID_HANDLE_VALUE) {
				free(devIfDetail);
				continue;
			}
			HIDD_ATTRIBUTES devAttr;
			if (!HidD_GetAttributes(devHandle, &devAttr)) {
				free(devIfDetail);
				CloseHandle(devHandle);
				continue;
			}
			PHIDP_PREPARSED_DATA devPreData;
			if (!HidD_GetPreparsedData(devHandle, &devPreData )) {
				free(devIfDetail);
				CloseHandle(devHandle);
				continue;
			}
			HIDP_CAPS devCaps;
			if (HidP_GetCaps(devPreData, &devCaps ) != HIDP_STATUS_SUCCESS ) {
				free(devIfDetail);
				CloseHandle(devHandle);
				continue;
			}
			CloseHandle(devHandle);
			if (devAttr.VendorID == venderID && devAttr.ProductID == productID) {
				mDevHandle = CreateFile(devIfDetail->DevicePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
				SetupDiDestroyDeviceInfoList(devInfoSet);
				memcpy(&mDevAttr, &devAttr, sizeof(HIDD_ATTRIBUTES));
				memcpy(&mDevCaps, &devCaps, sizeof(HIDP_CAPS));
				mInputBuf = (unsigned char *)malloc(mDevCaps.InputReportByteLength);
				mOutputBuf = (unsigned char *)malloc(mDevCaps.OutputReportByteLength);
				free(devIfDetail);
				return true;
			}
		}
		free(devIfDetail);
	}

	SetupDiDestroyDeviceInfoList(devInfoSet);

	return false;
}

////////////////////////////////////////////////
//	close
////////////////////////////////////////////////

bool UsbHID::close()
{
	if (mDevIfDetailData) {
		free(mDevIfDetailData);
		mDevIfDetailData = NULL;
	}
	if (mDevHandle) {
		CloseHandle(mDevHandle);
		mDevHandle = NULL;
	}
	if (mInputBuf) {
		free(mInputBuf);
		mInputBuf = NULL;
	}
	if (mOutputBuf) {
		free(mOutputBuf);
		mOutputBuf = NULL;
	}
	return true;
}

////////////////////////////////////////////////
//	read
////////////////////////////////////////////////

int UsbHID::read()
{
	if (!isConnected()) 
		return 0;

	DWORD nRead;

	nRead = 0;
	clearInputByteBuffer();
	
	if (!ReadFile(mDevHandle, mInputBuf, mDevCaps.InputReportByteLength, &nRead, &mOverLapped)) {
		GetLastError();
		return 0;
	}

	DWORD waitRes = WaitForSingleObject(mDevEvent, 10);
	switch(waitRes) {
	case WAIT_OBJECT_0:
		GetOverlappedResult(mDevHandle, &mOverLapped, &nRead, TRUE);
		break;
	case WAIT_TIMEOUT:
		CancelIo(mDevHandle);
		break;
	default:
		CancelIo(mDevHandle);
		break;
	}

	return nRead;
}


////////////////////////////////////////////////
//	write
////////////////////////////////////////////////

int UsbHID::write(unsigned char *data, int dataLen)
{
	if (!isConnected()) 
		return 0;
	DWORD wroteByte;
	wroteByte = 0;
#if defined(CX3D_HID_USE_CTLREQ)
	wroteByte = HidD_SetOutputReport(mDevHa/ndle, data, dataLen);
#else
	if (WriteFile(mDevHandle, data, mDevCaps.OutputReportByteLength, &wroteByte, &mOverLapped))
		GetOverlappedResult(mDevHandle, &mOverLapped, &wroteByte, TRUE);
#endif
	return wroteByte;
}

////////////////////////////////////////////////
// GetLastError
////////////////////////////////////////////////

#if defined(WIN32)

void UsbHID::GetLastError()
{
	DWORD dw = ::GetLastError(); 
    char *errMsg;
    FormatMessage(
	    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
	    FORMAT_MESSAGE_FROM_SYSTEM,
	    NULL,
	    dw,
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	    (LPTSTR) &errMsg,
	    0, NULL );		
	LocalFree(errMsg);
}

#endif
