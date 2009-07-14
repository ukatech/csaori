/*============================================================================
	Gainer制御クラス (cpp)
	以下のgainercxx (dandelionさん作) をたくさん参考にしました。
	http://www.atinfinity.info/wiki/index.php?gainercxx
============================================================================*/

#pragma warning( disable : 4786 )

#include <iostream>
#include <string>

#include "gainercxx.h"
#include "erslib.h"

#include <mmsystem.h>

const int Gainer::MATRIX_LED_CONFIGURATION = 7;

static std::vector<int> g_gainer_open_ports;

#define RECV_TIMEOUT_COMM     5000
#define RECV_TIMEOUT_INTERVAL 100
#define SEND_TIMEOUT_COMM     1000
#define RECV_BUFFER	          64

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
	m_hwnd_sstp = NULL;

	m_analog_period = 0;
	m_analog_last = 0;
	m_digital_period = 0;
	m_digital_last = 0;

	ZeroMemory(m_LEDMatrixOutputs,sizeof(m_LEDMatrixOutputs));

	m_receive_buffer.assign(RECV_BUFFER,0);
	
	InitializeCriticalSection(&m_receive_queue_lock);
	
	m_receive_queue_semaphore = ::CreateSemaphore(NULL,0,9999,NULL);

	// COM port open
	if ( ERS_Open(m_port, RECV_BUFFER, RECV_BUFFER) != 0 ) {
		return false;
	}
	// setting of COM port
	if ( ERS_Config(m_port, ERS_38400|ERS_NO|ERS_1|ERS_8|ERS_NO_FLOW_CONTROL) != 0 ) {
		ERS_Close(m_port);
		return false;
	}

	// setting receiving timeout
	ERS_RecvTimeOut(m_port, RECV_TIMEOUT_COMM, RECV_TIMEOUT_INTERVAL);
	ERS_SendTimeOut(m_port, SEND_TIMEOUT_COMM);

	// software reset
	Reboot(true);
	m_thread_handle = (HANDLE)_beginthreadex(NULL, 0, receiver, this, 0, NULL);
	wait_recv();

	// ver.
	m_version_string = command_send("?*");
	m_version_string.erase(0,1);
	m_version_string.erase(m_version_string.size()-1,1);

	// set configulation mode
	if ( m_config ) {
		SetConfiguration(m_config);
	}

	m_inited = true;
	g_gainer_open_ports.push_back(m_port);
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

	g_gainer_open_ports.erase(
		std::remove(g_gainer_open_ports.begin(), g_gainer_open_ports.end(), m_port), g_gainer_open_ports.end());

	// software reset
	m_endFlag = true;
	Reboot(true);

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
		std::vector<int>::iterator it = std::find(g_gainer_open_ports.begin(),g_gainer_open_ports.end(),i);
		if ( it != g_gainer_open_ports.end() ) {
			v.push_back(i);
			continue;
		}

		if ( ERS_Open(i, RECV_BUFFER, RECV_BUFFER) != 0 ) {
			continue;
		}
		if ( ERS_Config(i, ERS_38400|ERS_NO|ERS_1|ERS_8|ERS_NO_FLOW_CONTROL) != 0 ) {
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
	if ( mode == m_config ) {
		return true;
	}

	if ( m_config ) {
		Reboot();
	}
	m_config = mode;

	char buf[64];
	sprintf(buf,"KONFIGURATION_%d*",mode);
	command_send(buf);

	::Sleep(100);
	return true;
}

/*****************************************************************************
	LED(h/l)
*****************************************************************************/
bool Gainer::SetLED(bool isOn)
{
	if ( isOn ) {
		return command_send("h*").size() != 0;
	}
	else {
		return command_send("l*").size() != 0;
	}
}

/*****************************************************************************
	全デジタル取得
*****************************************************************************/
bool Gainer::GetDigitalAll(WORD &result,size_t &bits)
{
	std::string r = command_send("R*");
	result = m_digitalInputs;
	bits = CONFIG[m_config][DIN];
	return r.size() != 0;
}

/*****************************************************************************
	全アナログ取得
*****************************************************************************/
bool Gainer::GetAnalogAll(std::vector<BYTE> &result)
{
	std::string r = command_send("I*");
	size_t n = CONFIG[m_config][AIN];
	for ( size_t i = 0 ; i < n ; ++i ) {
		result.push_back(m_analogInputs[i]);
	}
	return r.size() != 0 && result.size() != 0;
}

/*****************************************************************************
	Continuous系
*****************************************************************************/
void Gainer::ExecuteContinuousDigital(DWORD period)
{
	m_digital_period = period;
	command_send("r*",true);
}

void Gainer::ExecuteContinuousAnalog(DWORD period)
{
	m_analog_period = period;
	command_send("i*",true);
}

void Gainer::ExecuteExitContinuous()
{
	command_send("E*");
}

/*****************************************************************************
	全デジタル設定
*****************************************************************************/
bool Gainer::SetDigitalAll(int value)
{
	if ( ! CONFIG[m_config][DOUT] ) {
		return false;
	}

	char c[32];
	sprintf(c,"D%04X*",value);
	return command_send(c).size() != 0;
}

/*****************************************************************************
	全アナログ設定
*****************************************************************************/
bool Gainer::SetAnalogAll(const std::vector<WORD> &data)
{
	int limit = CONFIG[m_config][AOUT];
	if ( ! limit ) {
		return false;
	}

	std::string c("A");
	char buf[32];

	size_t n = data.size();
	if ( n > limit ) {
		n = limit;
	}

	for ( size_t i = 0 ; i < n ; ++i ) {
		sprintf(buf,"%02X",data[i]);
		c += buf;
	}

	int diff = limit - n;
	while ( diff > 0 ) {
		c += "00";
		--diff;
	}
	c += "*";

	return command_send(c).size() != 0;
}

/*****************************************************************************
	全サーボ設定
*****************************************************************************/
bool Gainer::SetServoAll(const std::vector<WORD> &data)
{
	int limit = 8;
	if ( m_config != 8 ) { return false; }

	std::string c("P");
	char buf[32];

	size_t n = data.size();
	if ( n > limit ) {
		n = limit;
	}

	for ( size_t i = 0 ; i < n ; ++i ) {
		sprintf(buf,"%02X",data[i]);
		c += buf;
	}

	int diff = limit - n;
	while ( diff > 0 ) {
		c += "00";
		--diff;
	}
	c += "*";

	return command_send(c).size() != 0;
}

/*****************************************************************************
	単独デジタル設定
*****************************************************************************/
bool Gainer::SetDigitalSingle(int port,bool high)
{
	char buf[32];
	if ( high ) {
		sprintf(buf,"H%d*",port);
	}
	else {
		sprintf(buf,"L%d*",port);
	}
	return command_send(buf).size() != 0;
}

/*****************************************************************************
	単独アナログ設定
*****************************************************************************/
bool Gainer::SetAnalogSingle(int port,BYTE value)
{
	char buf[64];
	sprintf(buf,"a%d%02X*",port,value);
	return command_send(buf).size() != 0;
}

/*****************************************************************************
	単独サーボ設定
*****************************************************************************/
bool Gainer::SetServoSingle(int port,BYTE value)
{
	int limit = 8;
	if ( m_config != 8 ) { return false; }

	char buf[64];
	sprintf(buf,"p%d%02X*",port,value);
	return command_send(buf).size() != 0;
}

/*****************************************************************************
	PGA / AGND(2.5v) . VSS(0v)
*****************************************************************************/
bool Gainer::SetPGA(double gain,bool isAGNDRef)
{
	char buf[64];
	int gi;

	if ( gain <= 1.00 ) {
		gi = 0;
	}
	else if ( gain <= 1.14 ) {
		gi = 1;
	}
	else if ( gain <= 1.33 ) {
		gi = 2;
	}
	else if ( gain <= 1.46 ) {
		gi = 3;
	}
	else if ( gain <= 1.60 ) {
		gi = 4;
	}
	else if ( gain <= 1.78 ) {
		gi = 5;
	}
	else if ( gain <= 2.00 ) {
		gi = 6;
	}
	else if ( gain <= 2.27 ) {
		gi = 7;
	}
	else if ( gain <= 2.67 ) {
		gi = 8;
	}
	else if ( gain <= 3.20 ) {
		gi = 9;
	}
	else if ( gain <= 4.00 ) {
		gi = 10;
	}
	else if ( gain <= 5.33 ) {
		gi = 11;
	}
	else if ( gain <= 8.00 ) {
		gi = 12;
	}
	else if ( gain <= 16.0 ) {
		gi = 13;
	}
	else if ( gain <= 24.0 ) {
		gi = 14;
	}
	else /*if ( gain > 24.0 )*/ {
		gi = 15;
	}

	if ( isAGNDRef ) {
		sprintf(buf,"G%01X1*",gi);
	}
	else {
		sprintf(buf,"G%01X0*",gi);
	}
	return command_send(buf).size() != 0;
}

/*****************************************************************************
	再起動
*****************************************************************************/
void Gainer::Reboot(bool nowait)
{
	command_send("Q*",nowait);
	m_config = 0;
	if ( ! nowait ) {
		Sleep(100);
	}
}

/*****************************************************************************
	LEDアレイ用
*****************************************************************************/
static bool ScanLineCompare(BYTE *d1,BYTE *d2)
{
	for ( size_t i = 0 ; i < GAINER_LED_MATRIX ; ++i ) {
		if ( d1[i] != d2[i] ) {
			return false;
		}
	}
	return true;
}

bool Gainer::ScanLine(size_t row,BYTE data[GAINER_LED_MATRIX])
{
	if ( m_config != 7 ) { return false; }
	if ( row > GAINER_LED_MATRIX-1 ) { return false; }

	//最適化：前と同じなら実行しない
	if ( ScanLineCompare(data,m_LEDMatrixOutputs[row]) ) {
		return true;
	}

	std::string c = "a";
	size_t i;
	char buf[32];

	sprintf(buf,"%u",row);
	c += buf;

	for ( i = 0 ; i < GAINER_LED_MATRIX ; ++i ) {
		if ( data[i] > 15 ) {
			data[i] = 15;
		}
		sprintf(buf,"%01X",data[i]);
		c += buf;
	}
	c += "*";

	command_send(c,true);

	memcpy(m_LEDMatrixOutputs[row],data,sizeof(m_LEDMatrixOutputs[row]));

	return true;
}

bool Gainer::ScanMatrix(BYTE data[GAINER_LED_MATRIX][GAINER_LED_MATRIX])
{
	if ( m_config != 7 ) { return false; }

	for ( size_t row = 0 ; row < GAINER_LED_MATRIX ; ++row ) {
		ScanLine(row,data[row]);
	}

	return true;
}

/*****************************************************************************
	コマンド送信
*****************************************************************************/
std::string Gainer::command_send(const std::string &cmd,bool nowait)
{
#ifdef _DEBUG
	std::cout << "command_send : " << cmd.c_str() << std::endl << std::flush;
#endif

	ERS_Send(m_port, const_cast<char *>(cmd.c_str()),cmd.length());
	ERS_WaitSend(m_port);

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
	size_t i = 0;
	m_receive_buffer[0] = 0;

	while ( strchr(&m_receive_buffer[0],'*') == NULL ) {
		if ( m_endFlag ) {
			break;
		}

		int r = ERS_CheckRecv(m_port);
		if ( r == 0 ) {
			r = 2; //2バイト読んで待機
		}

		if ( m_receive_buffer.size() < r+1 ) {
			m_receive_buffer.assign(r+1,0);
		}

		i += ERS_Recv(m_port,&m_receive_buffer[i],r);
		m_receive_buffer[i] = 0;

		if ( i == 0 ) {
			::Sleep(0);
		}

#ifdef _DEBUG
		std::cout << "*";
#endif
	}

	size_t len = i;
	size_t start = 0;
	std::string s;

	std::string data_i;
	std::string data_r;

	for ( i = 0 ; i < len ; ++i ) {
		if ( m_receive_buffer[i] == '*' ) {
			s.assign(&m_receive_buffer[start],i-start+1);
			start = i+1;

			if ( s[0] != 'r' && s[0] != 'i' ) {
				processEvent(s);
			}
			else {
				if ( s[0] == 'r' ) {
					data_r = s;
				}
				else {
					data_i = s;
				}
			}
		}
	}

	if ( data_r.size() ) {
		processEvent(data_r);
	}
	if ( data_i.size() ) {
		processEvent(data_i);
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
void Gainer::processEvent(const std::string &event)
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
		execute_sstp_button(true);
		break;
	case 'F': // button released
		execute_sstp_button(false);
		break;

	case 'i':
	case 'I':
		{ // analog_input
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

			if ( event[0] == 'i' ) {
				execute_sstp_analog();
			}
		}
		break;

	case 'r':
	case 'R':
		{ // digital input
			std::string::size_type ast(event.find('*'));
			std::string s(event.substr(1, ast-1));
			sscanf(s.c_str(),"%04X",&m_digitalInputs);
			
			if ( event[0] == 'r' ) {
				execute_sstp_digital();
			}
		}
		break;

	default:
		break;
	}

	char c = event[0];

	if ( c != 'r' && c != 'i' && c != 'N' && c != 'F' ) {
		if ( ! (m_config == 7 && c == 'a') ) {
			EnterCriticalSection(&m_receive_queue_lock);
			m_receive_queue.push(event);
			ReleaseSemaphore(m_receive_queue_semaphore,1,NULL);
			LeaveCriticalSection(&m_receive_queue_lock);
		}
	}
}

/*****************************************************************************
	受信スレッド
*****************************************************************************/
unsigned __stdcall Gainer::receiver(void *arg)
{
	Gainer *self = reinterpret_cast<Gainer *>(arg);
	return self->receive();
}

unsigned Gainer::receive(void)
{
	while ( ! m_endFlag ) {
		//DWORD wr = ERS_WaitEvent(m_port);
		//if ( wr & EV_RXCHAR ) {
		command_recv();
		//}
	}
	_endthreadex(0);
	return NULL;
}

/*****************************************************************************
	SSTP
*****************************************************************************/
void Gainer::execute_sstp_button(bool isPressed)
{
	if ( ! m_hwnd_sstp ) {
		return;
	}

	std::string sstp = "NOTIFY SSTP/1.1\r\nCharset: Shift_JIS\r\nSender: GainerSAORI\r\n";
	sstp += "HWnd: 0\r\nEvent: ";

	if ( isPressed ) {
		sstp += "OnGainerButtonPressed";
	}
	else {
		sstp += "OnGainerButtonReleased";
	}
	
	sstp += "\r\nReference0: ";
	char buf[32];
	sprintf(buf,"%d",m_port);
	sstp += buf;

	sstp += "\r\n\r\n";

	send_sstp(sstp);
}

void Gainer::execute_sstp_digital(void)
{
	if ( ! m_hwnd_sstp ) {
		return;
	}
	if ( m_digital_period ) {
		DWORD current = ::timeGetTime();
		if ( (current - m_digital_last) < m_digital_period ) {
			return;
		}
		m_digital_last = current;
	}

	std::string sstp = "NOTIFY SSTP/1.1\r\nCharset: Shift_JIS\r\nSender: GainerSAORI\r\n";
	sstp += "HWnd: 0\r\nEvent: OnGainerDigitalInput\r\nReference0: ";

	char buf[512];
	sprintf(buf,"%d\r\n",m_port);
	sstp += buf;

	sstp += "Reference1: ";
	if ( CONFIG[m_config][DIN] == 4 ) {
		sprintf(buf,"%d,%d,%d,%d\r\n",
			(m_digitalInputs & 1) != 0, (m_digitalInputs & 2) != 0, (m_digitalInputs & 4) != 0, (m_digitalInputs & 8) != 0 );
	}
	else if ( CONFIG[m_config][DIN] == 8 ) {
		sprintf(buf,"%d,%d,%d,%d,%d,%d,%d,%d\r\n",
			(m_digitalInputs & 1) != 0, (m_digitalInputs & 2) != 0, (m_digitalInputs & 4) != 0, (m_digitalInputs & 8) != 0,
			(m_digitalInputs & 16) != 0, (m_digitalInputs & 32) != 0, (m_digitalInputs & 64) != 0, (m_digitalInputs & 128) != 0 );
	}
	else /*if ( CONFIG[m_config][DIN] == 16 )*/ {
		sprintf(buf,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
			(m_digitalInputs & 1) != 0, (m_digitalInputs & 2) != 0, (m_digitalInputs & 4) != 0, (m_digitalInputs & 8) != 0,
			(m_digitalInputs & 16) != 0, (m_digitalInputs & 32) != 0, (m_digitalInputs & 64) != 0, (m_digitalInputs & 128) != 0,
			(m_digitalInputs & 256) != 0, (m_digitalInputs & 512) != 0, (m_digitalInputs & 1024) != 0, (m_digitalInputs & 2048) != 0,
			(m_digitalInputs & 4096) != 0, (m_digitalInputs & 8192) != 0, (m_digitalInputs & 16384) != 0, (m_digitalInputs & 32768) != 0 );
	}
	sstp += buf;

	send_sstp(sstp);
}

void Gainer::execute_sstp_analog(void)
{
	if ( ! m_hwnd_sstp ) {
		return;
	}
	if ( m_analog_period ) {
		DWORD current = ::timeGetTime();
		if ( (current - m_analog_last) < m_analog_period ) {
			return;
		}
		m_analog_last = current;
	}

	std::string sstp = "NOTIFY SSTP/1.1\r\nCharset: Shift_JIS\r\nSender: GainerSAORI\r\n";
	sstp += "HWnd: 0\r\nEvent: OnGainerAnalogInput\r\nReference0: ";

	char buf[512];
	sprintf(buf,"%d\r\n",m_port);
	sstp += buf;

	sstp += "Reference1: ";
	if ( CONFIG[m_config][AIN] == 4 ) {
		sprintf(buf,"%d,%d,%d,%d\r\n",
			m_analogInputs[0], m_analogInputs[1], m_analogInputs[2], m_analogInputs[3] );
	}
	else /*if ( CONFIG[m_config][AIN] == 8 )*/ {
		sprintf(buf,"%d,%d,%d,%d,%d,%d,%d,%d\r\n",
			m_analogInputs[0], m_analogInputs[1], m_analogInputs[2], m_analogInputs[3],
			m_analogInputs[4], m_analogInputs[5], m_analogInputs[6], m_analogInputs[7] );
	}
	sstp += buf;

	send_sstp(sstp);
}

void Gainer::send_sstp(std::string &sstp)
{
	DWORD result;
	
	COPYDATASTRUCT c;
	c.dwData = 9801;
	c.cbData = sstp.size();
	c.lpData = const_cast<char*>(sstp.c_str());

	SendMessageTimeout(m_hwnd_sstp,WM_COPYDATA,
		reinterpret_cast<WPARAM>(m_hwnd_sstp),
		reinterpret_cast<LPARAM>(&c),
		SMTO_ABORTIFHUNG,1000,&result);
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

