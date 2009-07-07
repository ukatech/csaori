/** 
 * @file Gainer.cpp
 * @brief Gainer Class
 * @author dandelion
 * @date 2009-04-18
 * @version $Id: Gainer.cpp, v0.1$
 *
 * Copyright (C) 2009 dandelion. All rights reserved.
 * Modified by CSAORI Project
 */

// http://www.atinfinity.info/wiki/index.php?gainercxx

#pragma warning( disable : 4786 )

#include <iostream>
#include <string>
#include <sstream>

// Gainer class header
#include "gainercxx.h"

// ERSLIB(http://www.geocities.jp/in_subaru/erslib/)
#include "erslib.h"

const int Gainer::MATRIX_LED_CONFIGURATION = 7;

#define RECV_TIMEOUT_COMM     50000
#define RECV_TIMEOUT_INTERVAL 100
#define RECV_BUFFER	          1000

/**
 * Gainer class default constructor. COM port number is COM1.
 * @param portNum COM port number
 * @param mode configutation mode (1-8)
 */
bool Gainer::Init(int portNum, int mode)
{
	if ( inited ) {
		return true;
	}

	led_ = false;
	portNum_ = portNum;
	config_ = mode;
	endFlag = false;
	on_pressed = NULL;
	on_released = NULL;

	InitializeCriticalSection(&port_lock);
	
	receive_semaphore = ::CreateSemaphore(NULL,0,9999,NULL);

	// COM port open
	if ( ERS_Open(portNum_, RECV_BUFFER, RECV_BUFFER) != 0 ) {
		return false;
	}
	// setting of COM port
	if ( ERS_Config(portNum_, ERS_38400 | ERS_NO | ERS_1 | ERS_8) != 0 ) {
		ERS_Close(portNum_);
		return false;
	}

	// setting receiving timeout
	ERS_RecvTimeOut(portNum_, RECV_TIMEOUT_COMM, RECV_TIMEOUT_INTERVAL);

	// software reset
	reboot(true);
	handle = (HANDLE)_beginthreadex(NULL, 0, receiver, this, 0, NULL);
	wait_recv();

	// ver.
	ver = command("?");

	// set configulation mode
	if ( config_ ) {
		setConfiguration(config_);
	}

	inited = true;
	return true;
}

/**
 * Gainer class destructor.
 */
void Gainer::Exit()
{
	if ( ! inited ) {
		return;
	}

	// software reset
	endFlag = true;
	reboot(true);

	WaitForSingleObject(handle, INFINITE);
	CloseHandle(handle);

	// COM port close
	ERS_Close(portNum_);

	::CloseHandle(receive_semaphore);

	DeleteCriticalSection(&port_lock);
}

std::string Gainer::Version(void)
{
	return ver;
}

void Gainer::Search(std::vector<int> &v)
{
	for ( int i = 1 ; i <= 32 ; ++i ) {
		if ( ERS_Open(i, RECV_BUFFER, RECV_BUFFER) != 0 ) {
			continue;
		}
		if ( ERS_Config(i, ERS_38400 | ERS_NO | ERS_1 | ERS_8) != 0 ) {
			ERS_Close(i);
			continue;
		}
		ERS_RecvTimeOut(i, 100, 100);
		if ( ! ERS_Send(i, "?*",2) ) {
			ERS_Close(i);
			continue;
		}

		bool found = false;
		while ( true ) {
			int r = ERS_Getc(i);
			if ( r == EOF ) {
				break;
			}
			else if ( r == '*' ) {
				found = true;
				break;
			}
		}

		ERS_Close(i);
		if ( found ) {
			v.push_back(i);
		}
	}
}

bool Gainer::setConfiguration(int mode)
{
	// Configulation Mode check
	if (1 > mode || mode > 8) {
		return false;
	}

	config_ = mode;
	std::stringstream ss("");
	ss << "KONFIGURATION_" << mode;
	command(ss.str());

	::Sleep(100);
	return true;
}

/**
 * Turn on the LED on gainer. command "h*" 
 * @return
 */
bool Gainer::turnOnLED()
{
	return command("h").size() != 0;
}

/**
 * Turn off the LED on gainer. command "l*" 
 * @return
 */
bool Gainer::turnOffLED()
{
	return command("l").size() != 0;
}


bool Gainer::getDigitalInputAll(WORD &result,size_t &bits)
{
	std::string r = command("R");
	result = digitalInputs;
	bits = CONFIG[config_][DIN];
	return r.size() != 0;
}

bool Gainer::getAnalogInputAll(std::vector<BYTE> &result)
{
	std::string r = command("I");
	size_t n = CONFIG[config_][AIN];
	for ( size_t i = 0 ; i < n ; ++i ) {
		result.push_back(analogInputs[i]);
	}
	return r.size() != 0 && result.size() != 0;
}

void Gainer::continuous_digital_inputs()
{
	command("r");
}

void Gainer::continuous_analog_inputs()
{
	command("i");
}

void Gainer::exit_continuous()
{
	command("E");
}
//------------------------------------------

/**
 * It makes 0V or +5V on digital output port.
 * @param value values of the digital output ports true:1 false:0	
 * @return
 */
bool Gainer::setDigitalOutputAll(int value)
{
	TCHAR c[32];
	_stprintf(c,_T("D%04X"),value);
	return command(c).size() != 0;
}

/**
 * It makes +5V on digital output port.
 * @param port number of the digital output port
 * @return
 */
bool Gainer::setDigitalOutput(int port,bool high)
{
	char buf[32];
	if ( high ) {
		sprintf(buf,"H%d",port);
	}
	else {
		sprintf(buf,"L%d",port);
	}
	return command(buf).size() != 0;
}

bool Gainer::setAnalogOutput(int port,BYTE value)
{
	char buf[64];
	sprintf(buf,"a%d%02X",port,value);
	return command(buf).size() != 0;
}

bool Gainer::setServoOutput(int port,BYTE value)
{
	char buf[64];
	sprintf(buf,"p%d%02X",port,value);
	return command(buf).size() != 0;
}

//------------------------------------------

/**
 * Software reset. User should not use this.
 * @return
 */
void Gainer::reboot(bool nowait)
{
	command("Q",nowait);
	if ( ! nowait ) {
		Sleep(100);
	}
}

/**
 * 
 * @param cmd command string
 * @param wait wait time(ms)
 * @return
 */
std::string Gainer::command(const std::string &cmd,bool nowait)
{
	return command_send(cmd + "*",nowait);
}

std::string Gainer::command_send(const std::string &cmd,bool nowait)
{
#ifdef _DEBUG
	std::cout << "command_send : " << cmd.c_str() << std::endl << std::flush;
#endif

	ERS_Send(portNum_, const_cast<char *>(cmd.c_str()),cmd.length());

	//ERS_WaitSend(portNum_);

	if ( nowait ) {
		return _T("");
	}
	else {
		return wait_recv();
	}
}

void Gainer::command_recv(void)
{
	char buf[RECV_BUFFER+1] = "";
	size_t i = 0;

	while (strchr(buf,'*') == NULL && i < RECV_BUFFER) {
		if ( endFlag ) {
			break;
		}

		int r = ERS_CheckRecv(portNum_);
		if ( r == 0 ) {
			r = 1; //1バイト読んで待機
		}

		i += ERS_Recv(portNum_,buf+i,r);
		buf[i] = 0;

#ifdef _DEBUG
		std::cout << "*";
#endif
	}

	size_t len = i;
	size_t start = 0;
	std::string s;
	for ( i = 0 ; i < len ; ++i ) {
		if ( buf[i] == '*' ) {
			s.assign(buf+start,i-start+1);
			start = i+1;
			processEvent(s);
		}
	}
}

std::string Gainer::wait_recv(void)
{
	::WaitForSingleObject(receive_semaphore,INFINITE);

	EnterCriticalSection(&port_lock);
	std::string result = receive_queue.front();
	receive_queue.pop();
	LeaveCriticalSection(&port_lock);
	
#ifdef _DEBUG
	std::cout << "command_recv : " << result << std::endl << std::flush;
#endif

	if ( strstr("!*",result.c_str()) ) {
		return "";
	}
	return result;
}

void Gainer::processEvent(std::string &event)
{
	if ( event.length() == 0 ) {
		return;
	}

	switch(event[0])
	{
	case '!': // something wrong
		break;
	case 'h': // led on
		led_ = true;
		break;
	case 'l': // led off
		led_ = false;
		break;
	case 'N': // button pressed
		if(on_pressed)
		{
			on_pressed();
		}
		break;
	case 'F': // button released
		if(on_released)
		{
			on_released();
		}
		break;
	case 'i':
	case 'I': { // analog_input
		std::string::size_type ast(event.find('*'));
		std::string s(event.substr(1, ast-1));

		int ai[GAINER_MAX_INPUTS];

		if ( CONFIG[config_][AIN] == 4 ) {
			sscanf(s.c_str(), "%02X%02X%02X%02X",
				&ai[0], &ai[1],&ai[2], &ai[3]);

			for ( int i = 0 ; i < 4 ; ++i ) {
				analogInputs[i] = static_cast<BYTE>(ai[i]);
			}
		}
		else if ( CONFIG[config_][AIN] == 8 ) {
			sscanf(s.c_str(), "%02X%02X%02X%02X%02X%02X%02X%02X",
				&ai[0], &ai[1],&ai[2], &ai[3], 
				&ai[4], &ai[5],&ai[6], &ai[7] );

			for ( int i = 0 ; i < 8 ; ++i ) {
				analogInputs[i] = static_cast<BYTE>(ai[i]);
			}
		}
		break;
			  }
	case 'r':
	case 'R': { // digital input
		std::string::size_type ast(event.find('*'));		std::string s(event.substr(1, ast-1));
		sscanf(s.c_str(),"%04X",&digitalInputs);
		break;
			  }
	default:
		break;
	}

	char c = event[0];
	if ( c != 'r' && c != 'i' && c != 'N' && c != 'F' ) {
		EnterCriticalSection(&port_lock);
		receive_queue.push(event);
		ReleaseSemaphore(receive_semaphore,1,NULL);
		LeaveCriticalSection(&port_lock);
	}
}

unsigned __stdcall Gainer::receiver(void *arg)
{
	Gainer *self = reinterpret_cast<Gainer *>(arg);
	while (!self->endFlag) {
		self->command_recv();
	}
	_endthreadex(0);
	return NULL;
}

const int Gainer::CONFIG[][4] = 
{
	// N_AIN, N_DIN, N_AOUT, N_DOUT
	{0, 0, 0, 0}, // 0
	{4, 4, 4, 4}, // 1
	{8, 0, 4, 4}, // 2
	{4, 4, 8, 0}, // 3
	{8, 0, 8, 0}, // 4
	{0,16, 0, 0}, // 5
	{0, 0, 0,16}, // 6
	{0, 0, 8, 8},  // 7 for matrix LED
	{4, 0, 0, 4}  // 8 for Servo
};

