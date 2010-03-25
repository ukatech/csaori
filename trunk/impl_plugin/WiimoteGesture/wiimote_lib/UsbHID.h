/******************************************************************
*
*	Wiimote for C++
*
*	Copyright (C) Satoshi Konno 2007
*
*	File:	UsbHID.cpp
*
******************************************************************/

#ifndef _CG_USBHID_H_
#define _CG_USBHID_H_

#if defined(WIN32)
#include <windows.h>
#include <setupapi.h>
extern "C" {
#include <hidsdi.h>
}
#endif

namespace CyberGarage {

class UsbHID
{
private:

#if defined(WIN32)
	SP_DEVICE_INTERFACE_DETAIL_DATA *mDevIfDetailData;
	HANDLE mDevHandle;
	HIDD_ATTRIBUTES mDevAttr;
	HIDP_CAPS mDevCaps;
	HANDLE  mDevEvent;
	OVERLAPPED mOverLapped;
	unsigned char *mInputBuf;
	unsigned char *mOutputBuf;
#endif

public:

	UsbHID();
	virtual ~UsbHID();

	////////////////////////////////////////////////
	//	Functions
	////////////////////////////////////////////////

	bool open(int venderID, int productID);
	bool close();

	bool isConnected() 
	{
		return (mDevHandle ? true : false);
	}

	int read();
	int write(unsigned char *data, int dataLen);

	int getInputByteLength()
	{
		return mDevCaps.InputReportByteLength;
	}

	unsigned char *getInputByteBuffer()
	{
		return mInputBuf;
	}

	unsigned char getInputByte(int offset)
	{
		if (!mInputBuf || mDevCaps.InputReportByteLength <= offset)
			return 0;
		return mInputBuf[offset];
	}

	void clearInputByteBuffer()
	{
		if (mInputBuf)
			memset(mInputBuf, 0, mDevCaps.InputReportByteLength);
	}

	int getOutputByteLength()
	{
		return mDevCaps.OutputReportByteLength;
	}

	unsigned char *getOutputByteBuffer()
	{
		return mOutputBuf;
	}

	void clearOutputByteBuffer()
	{
		if (mOutputBuf)
			memset(mOutputBuf, 0, mDevCaps.OutputReportByteLength);
	}

	bool IsButtonPressed(int offset, unsigned char mask)
	{
		if (!mInputBuf)
			return false;
		return (mInputBuf[offset] & mask) ? true : false;
	}

#if defined(WIN32)
	void GetLastError();
#endif
};

}

#endif
