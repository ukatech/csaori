/*============================================================================
	Gainer制御クラス (h)
	以下のgainercxx (dandelionさん作) をたくさん参考にしました。
	http://www.atinfinity.info/wiki/index.php?gainercxx
============================================================================*/

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

class Gainer
{
public:
	//--------------------関数--------------------
	typedef void (*callback_t)(int state);
	
	Gainer() {
		m_inited = false;
	}
	bool Init(int portNum=1, int mode=0);
	
	void Exit();
	~Gainer() {
		Exit();
	}
	
	static void Search(std::vector<int> &v);
	
	std::string Version();
	
	bool SetLED(bool isOn);
	
	void ExecuteContinuousDigital();
	void ExecuteContinuousAnalog();
	void ExecuteExitContinuous();
	
	bool GetDigitalAll(WORD &result,size_t &bits);
	bool GetAnalogAll(std::vector<BYTE> &result);
	
	bool SetDigitalAll(int value);
	bool SetDigitalSingle(int port,bool high);
	bool SetAnalogSingle(int port,BYTE value);
	bool SetServoSingle(int port,BYTE value);
	
	void SetButtonCallbackFunc(callback_t funcp){m_button_func = funcp;}
	
	int GetCOMPort(void) { return m_port; }
	
	bool SetConfiguration(int mode);
	
private:
	//--------------------関数--------------------

	void reboot(bool nowait = false);
	
	std::string command(const std::string &cmd,bool nowait = false);
	std::string command_send(const std::string &cmd,bool nowait = false);
	
	void processEvent(std::string &event);
	
	void command_recv(void);
	std::string wait_recv(void);

	static unsigned __stdcall receiver(void *arg);

	//--------------------変数--------------------
	
	bool  m_led;
	BYTE  m_analogInputs[GAINER_MAX_INPUTS];
	DWORD m_digitalInputs;
	
	int m_config;
	
	int m_port;
	bool m_endFlag;
	bool m_inited;
	
	callback_t m_button_func;
	
	HANDLE m_thread_handle;
	
	CRITICAL_SECTION m_receive_queue_lock;
	
	HANDLE m_receive_queue_semaphore;
	std::queue<std::string> m_receive_queue;
	
	std::string m_version_string;
	
	enum pin_t {AIN = 0, DIN, AOUT, DOUT};
	static const int CONFIG[][4];
	static const int MATRIX_LED_CONFIGURATION;
};

#endif // __GAINER_H_
