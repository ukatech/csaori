/*============================================================================
	Gainer制御クラス (cpp)
	以下のgainercxx (dandelionさん作) をたくさん参考にしました。
	http://www.atinfinity.info/wiki/index.php?gainercxx
============================================================================*/

#pragma warning( disable : 4786 )

#include <iostream>
#include <string>
#include <sstream>

#include "gainercxx.h"
#include "erslib.h"

const int Gainer::MATRIX_LED_CONFIGURATION = 7;

#define RECV_TIMEOUT_COMM     50000
#define RECV_TIMEOUT_INTERVAL 100
#define RECV_BUFFER	          1000

/*****************************************************************************
	初期化
*****************************************************************************/
bool Gainer::Init(int portNum, int mode)
{
	if ( m_inited ) {
		return true;
	}

	m_led = false;
	m_config = mode;
	m_port = portNum;
	m_endFlag = false;
	m_button_func = NULL;

	InitializeCriticalSection(&m_receive_queue_lock);
	
	m_receive_queue_semaphore = ::CreateSemaphore(NULL,0,9999,NULL);

	// COM port open
	if ( ERS_Open(m_port, RECV_BUFFER, RECV_BUFFER) != 0 ) {
		return false;
	}
	// setting of COM port
	if ( ERS_Config(m_port, ERS_38400 | ERS_NO | ERS_1 | ERS_8) != 0 ) {
		ERS_Close(m_port);
		return false;
	}

	// setting receiving timeout
	ERS_RecvTimeOut(m_port, RECV_TIMEOUT_COMM, RECV_TIMEOUT_INTERVAL);

	// software reset
	reboot(true);
	m_thread_handle = (HANDLE)_beginthreadex(NULL, 0, receiver, this, 0, NULL);
	wait_recv();

	// ver.
	m_version_string = command("?");
	m_version_string.erase(0,1);
	m_version_string.erase(m_version_string.size()-1,1);

	// set configulation mode
	if ( m_config ) {
		SetConfiguration(m_config);
	}

	m_inited = true;
	return true;
}

/*****************************************************************************
	終了
*****************************************************************************/
void Gainer::Exit()
{
	if ( ! m_inited ) {
		return;
	}

	// software reset
	m_endFlag = true;
	reboot(true);

	WaitForSingleObject(m_thread_handle, INFINITE);
	CloseHandle(m_thread_handle);

	// COM port close
	ERS_Close(m_port);

	::CloseHandle(m_receive_queue_semaphore);

	DeleteCriticalSection(&m_receive_queue_lock);

	m_inited = false;
}

/*****************************************************************************
	バージョン
*****************************************************************************/
std::string Gainer::Version(void)
{
	return m_version_string;
}

/*****************************************************************************
	探索
*****************************************************************************/
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

/*****************************************************************************
	KONFIGURATION_N
*****************************************************************************/
bool Gainer::SetConfiguration(int mode)
{
	// Configulation Mode check
	if (1 > mode || mode > 8) {
		return false;
	}

	m_config = mode;
	std::stringstream ss("");
	ss << "KONFIGURATION_" << mode;
	command(ss.str());

	::Sleep(100);
	return true;
}

/*****************************************************************************
	LED(h/l)
*****************************************************************************/
bool Gainer::SetLED(bool isOn)
{
	if ( isOn ) {
		return command("h").size() != 0;
	}
	else {
		return command("l").size() != 0;
	}
}

/*****************************************************************************
	全デジタル取得
*****************************************************************************/
bool Gainer::GetDigitalAll(WORD &result,size_t &bits)
{
	std::string r = command("R");
	result = m_digitalInputs;
	bits = CONFIG[m_config][DIN];
	return r.size() != 0;
}

/*****************************************************************************
	全アナログ取得
*****************************************************************************/
bool Gainer::GetAnalogAll(std::vector<BYTE> &result)
{
	std::string r = command("I");
	size_t n = CONFIG[m_config][AIN];
	for ( size_t i = 0 ; i < n ; ++i ) {
		result.push_back(m_analogInputs[i]);
	}
	return r.size() != 0 && result.size() != 0;
}

/*****************************************************************************
	Continuous系(うまく動かない?)
*****************************************************************************/
void Gainer::ExecuteContinuousDigital()
{
	command("r");
}

void Gainer::ExecuteContinuousAnalog()
{
	command("i");
}

void Gainer::ExecuteExitContinuous()
{
	command("E");
}

/*****************************************************************************
	全デジタル設定
*****************************************************************************/
bool Gainer::SetDigitalAll(int value)
{
	TCHAR c[32];
	_stprintf(c,_T("D%04X"),value);
	return command(c).size() != 0;
}

/*****************************************************************************
	単独デジタル設定
*****************************************************************************/
bool Gainer::SetDigitalSingle(int port,bool high)
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

/*****************************************************************************
	単独アナログ設定
*****************************************************************************/
bool Gainer::SetAnalogSingle(int port,BYTE value)
{
	char buf[64];
	sprintf(buf,"a%d%02X",port,value);
	return command(buf).size() != 0;
}

/*****************************************************************************
	単独サーボ設定
*****************************************************************************/
bool Gainer::SetServoSingle(int port,BYTE value)
{
	char buf[64];
	sprintf(buf,"p%d%02X",port,value);
	return command(buf).size() != 0;
}

/*****************************************************************************
	再起動
*****************************************************************************/
void Gainer::reboot(bool nowait)
{
	command("Q",nowait);
	if ( ! nowait ) {
		Sleep(100);
	}
}

/*****************************************************************************
	コマンド送信
*****************************************************************************/
std::string Gainer::command(const std::string &cmd,bool nowait)
{
	return command_send(cmd + "*",nowait);
}

std::string Gainer::command_send(const std::string &cmd,bool nowait)
{
#ifdef _DEBUG
	std::cout << "command_send : " << cmd.c_str() << std::endl << std::flush;
#endif

	ERS_Send(m_port, const_cast<char *>(cmd.c_str()),cmd.length());

	//ERS_WaitSend(m_port);

	if ( nowait ) {
		return _T("");
	}
	else {
		return wait_recv();
	}
}

/*****************************************************************************
	コマンド受信
*****************************************************************************/
void Gainer::command_recv(void)
{
	char buf[RECV_BUFFER+1] = "";
	size_t i = 0;

	while (strchr(buf,'*') == NULL && i < RECV_BUFFER) {
		if ( m_endFlag ) {
			break;
		}

		int r = ERS_CheckRecv(m_port);
		if ( r == 0 ) {
			r = 1; //1バイト読んで待機
		}

		i += ERS_Recv(m_port,buf+i,r);
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

/*****************************************************************************
	受信待機
*****************************************************************************/
std::string Gainer::wait_recv(void)
{
	::WaitForSingleObject(m_receive_queue_semaphore,INFINITE);

	EnterCriticalSection(&m_receive_queue_lock);
	std::string result = m_receive_queue.front();
	m_receive_queue.pop();
	LeaveCriticalSection(&m_receive_queue_lock);
	
#ifdef _DEBUG
	std::cout << "command_recv : " << result << std::endl << std::flush;
#endif

	if ( strstr("!*",result.c_str()) ) {
		return "";
	}
	return result;
}

/*****************************************************************************
	受信結果処理
*****************************************************************************/
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
		m_led = true;
		break;
	case 'l': // led off
		m_led = false;
		break;
	case 'N': // button pressed
		if(m_button_func){
			m_button_func(true);
		}
		break;
	case 'F': // button released
		if(m_button_func){
			m_button_func(false);
		}
		break;
	case 'i':
	case 'I': { // analog_input
		std::string::size_type ast(event.find('*'));
		std::string s(event.substr(1, ast-1));

		int ai[GAINER_MAX_INPUTS];

		if ( CONFIG[m_config][AIN] == 4 ) {
			sscanf(s.c_str(), "%02X%02X%02X%02X",
				&ai[0], &ai[1],&ai[2], &ai[3]);

			for ( int i = 0 ; i < 4 ; ++i ) {
				m_analogInputs[i] = static_cast<BYTE>(ai[i]);
			}
		}
		else if ( CONFIG[m_config][AIN] == 8 ) {
			sscanf(s.c_str(), "%02X%02X%02X%02X%02X%02X%02X%02X",
				&ai[0], &ai[1],&ai[2], &ai[3], 
				&ai[4], &ai[5],&ai[6], &ai[7] );

			for ( int i = 0 ; i < 8 ; ++i ) {
				m_analogInputs[i] = static_cast<BYTE>(ai[i]);
			}
		}
		break;
			  }
	case 'r':
	case 'R': { // digital input
		std::string::size_type ast(event.find('*'));		std::string s(event.substr(1, ast-1));
		sscanf(s.c_str(),"%04X",&m_digitalInputs);
		break;
			  }
	default:
		break;
	}

	char c = event[0];
	if ( c != 'r' && c != 'i' && c != 'N' && c != 'F' ) {
		EnterCriticalSection(&m_receive_queue_lock);
		m_receive_queue.push(event);
		ReleaseSemaphore(m_receive_queue_semaphore,1,NULL);
		LeaveCriticalSection(&m_receive_queue_lock);
	}
}

/*****************************************************************************
	受信スレッド
*****************************************************************************/
unsigned __stdcall Gainer::receiver(void *arg)
{
	Gainer *self = reinterpret_cast<Gainer *>(arg);
	while (!self->m_endFlag) {
		self->command_recv();
	}
	_endthreadex(0);
	return NULL;
}

/*****************************************************************************
	設定構造体
*****************************************************************************/
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
	{0, 0, 8, 8}, // 7 for matrix LED
	{4, 0, 0, 4}  // 8 for Servo
};

