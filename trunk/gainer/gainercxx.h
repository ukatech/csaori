/** 
 * @file Gainer.h
 * @brief Gainer Class Header
 * @author dandelion
 * @date 2009-04-18
 * @version $Id: Gainer.h, v0.1$
 * 
 * Copyright (C) 2009 dandelion. All rights reserved.
 * Modified by CSAORI Project
 */


#ifndef _GAINER_H_
#define _GAINER_H_

#include <string>
#include <vector>
#include <queue>
#include <process.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Gainer;

#define GAINER_MAX_INPUTS 8

/**
 * Gainer class. 
 * @author dandelion
 * @version 0.1, 2009/04/18
 */
class Gainer
{
public:
   /**
    * Event handler calback
    */
	typedef void (*callback_t)(void);

   /**
    * Gainer class default constructor.
    */
	Gainer() {
		inited = false;
	}

	bool Init(int portNum=1, int mode=0);

	static void Search(std::vector<int> &v);

   /**
    * Gainer class destructor.
    */
	void Exit();

	~Gainer() {
		Exit();
	}

   /**
    * Return version. command "?*" 
    */
	std::string Version();

   /**
    * Turn on the LED on gainer. command "h*" 
    * @return
    */
	bool turnOnLED();
   
   /**
    * Turn off the LED on gainer. command "l*" 
    * @return
    */
	bool turnOffLED();

// Not implemented yet. (2009/04/18) ------
	void continuous_digital_inputs();
	void continuous_analog_inputs();
	void exit_continuous();

	bool getDigitalInputAll(WORD &result,size_t &bits);
	bool getAnalogInputAll(std::vector<BYTE> &result);

	bool setDigitalOutputAll(int value);
	bool setDigitalOutput(int port,bool high);
	bool setAnalogOutput(int port,BYTE value);
	bool setServoOutput(int port,BYTE value);

   /**
    * set button pressed event Callback function
    * @param funcp button pressed event Callback function pointer
    * @return
    */
	void setOnPressedCallback(callback_t funcp){on_pressed = funcp;}

   /**
    * set button released event Callback function
    * @param funcp button released event Callback function pointer
    * @return
    */
	void setOnReleasedCallback(callback_t funcp){on_released = funcp;}

	int port(void) { return portNum_; }

   /**
    * set configuration mode.
    * @param mode configutation mode (1-7)
    * @return
    */
	bool setConfiguration(int mode);

private:
   /**
    * Software reset. User should not use this.
    * @return
    */
	void reboot(bool nowait = false);

   /**
    * 
    * @param cmd command string
    * @param wait wait time(ms)
    * @return
    */
	std::string command(const std::string &cmd,bool nowait = false);
	std::string command_send(const std::string &cmd,bool nowait = false);

	void processEvent(std::string &event);

	void command_recv(void);
	std::string wait_recv(void);

	// properties
	bool led_;
	BYTE analogInputs[GAINER_MAX_INPUTS];
	WORD digitalInputs;

   /**
    * configuration
    */
	int config_;

   /**
    * COM port number
    */
	int portNum_;
	bool endFlag;
	bool inited;

   /**
    * callback function pointer
    */
	callback_t on_pressed, on_released;

   /**
    * handle for receiving thread
    */
	HANDLE handle;

	CRITICAL_SECTION port_lock;

	HANDLE receive_semaphore;
	std::queue<std::string> receive_queue;

	std::string ver;

	enum pin_t {AIN = 0, DIN, AOUT, DOUT};
	static const int CONFIG[][4];
	static const int MATRIX_LED_CONFIGURATION;
	static unsigned __stdcall receiver(void *arg);
};

#endif // __GAINER_H_
