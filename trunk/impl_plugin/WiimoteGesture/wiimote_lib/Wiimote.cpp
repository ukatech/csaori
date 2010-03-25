/******************************************************************
*
*	Wiimote for C++
*
*	Copyright (C) Satoshi Konno 2007
*
*	File:	Wiimote.cpp
*
******************************************************************/

#include <Wiimote.h>

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

using namespace CyberGarage;

const int Wiimote::VENDER_ID = 0x057E;
const int Wiimote::PRODUCT_ID = 0x0306;

////////////////////////////////////////////////
//	Constructor
////////////////////////////////////////////////

Wiimote::Wiimote() 
{
}

////////////////////////////////////////////////
//	Destructor
////////////////////////////////////////////////

Wiimote::~Wiimote()
{
}

////////////////////////////////////////////////
//	open 
////////////////////////////////////////////////

bool Wiimote::open()
{
	bool openRes = UsbHID::open(VENDER_ID, PRODUCT_ID);
	if (!openRes)
		return false;

	setLEDs(false, false, false, false);

	unsigned char *outBuf = getOutputByteBuffer();

	clearOutputByteBuffer();
	outBuf[0] = 0x12;
	outBuf[1] = 0x00;
	outBuf[2] = 0x33;
	if (write(outBuf, 3) <= 0)
		return false;

	return true;
}

////////////////////////////////////////////////
//	setLEDs
////////////////////////////////////////////////

bool Wiimote::setLEDs(bool led1, bool led2, bool led3, bool led4, bool vibration)
{
	unsigned char *outBuf = getOutputByteBuffer();

	clearOutputByteBuffer();
	outBuf[0] = 0x11;
	outBuf[1] = 0x00;
	if (led1)
		outBuf[1]  |= 0x10;
	if (led2)
		outBuf[1]  |= 0x20;
	if (led3)
		outBuf[1]  |= 0x40;
	if (led4)
		outBuf[1]  |= 0x80;
	if (vibration) 
		outBuf[1]  |= 0x01;
	if (write(outBuf, 2) <= 0)
		return false;

	return true;
}

