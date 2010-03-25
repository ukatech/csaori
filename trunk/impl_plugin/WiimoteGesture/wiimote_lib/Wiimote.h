/******************************************************************
*
*	Wiimote for C++
*
*	Copyright (C) Satoshi Konno 2007
*
*	File:	Wiimote.cpp
*
******************************************************************/

#ifndef _CG_WIIREMOTE_H_
#define _CG_WIIREMOTE_H_

#include <UsbHID.h>

namespace CyberGarage {

class Wiimote : public UsbHID
{

	static const int VENDER_ID;
	static const int PRODUCT_ID;

public:

	Wiimote();
	virtual ~Wiimote();

	////////////////////////////////////////////////
	//	Functions
	////////////////////////////////////////////////

	bool open();

	////////////////////////////////////////////////
	//	LED
	////////////////////////////////////////////////

	bool setLEDs(bool led1, bool led2, bool led3, bool led4, bool vibration = false);

	////////////////////////////////////////////////
	//	Button
	////////////////////////////////////////////////

	bool IsAPressed()
	{
		return IsButtonPressed(2, 0x08);
	}

	bool IsBPressed()
	{
		return IsButtonPressed(2, 0x04);
	}

	bool IsOnePressed()
	{
		return IsButtonPressed(2, 0x02);
	}

	bool IsTwoPressed()
	{
		return IsButtonPressed(2, 0x01);
	}

	bool IsLeftPressed()
	{
		return IsButtonPressed(1, 0x01);
	}

	bool IsRightPressed()
	{
		return IsButtonPressed(1, 0x02);
	}

	bool IsUpPressed()
	{
		return IsButtonPressed(1, 0x08);
	}

	bool IsDownPressed()
	{
		return IsButtonPressed(1, 0x04);
	}

	bool IsMinusPressed()
	{
		return IsButtonPressed(2, 0x10);
	}

	bool IsPlusPressed()
	{
		return IsButtonPressed(1, 0x10);
	}

	bool IsHomePressed()
	{
		return IsButtonPressed(2, 0x80);
	}

	////////////////////////////////////////////////
	//	Motion
	////////////////////////////////////////////////

	unsigned char getXMotion()
	{
		return getInputByte(3);
	}

	unsigned char getYMotion()
	{
		return getInputByte(4);
	}

	unsigned char getZMotion()
	{
		return getInputByte(5);
	}
};

}

#endif

