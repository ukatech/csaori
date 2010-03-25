//setlocale//
#ifdef _MSC_VER
#if _MSC_VER >= 1400
#pragma setlocale("japanese")
#endif
#endif
//setlocale end//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>
#include <mmsystem.h>

#include "wiimotegesture.h"

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

/*===============================================================
	インスタンス作成（csaori_baseから呼ばれる）
===============================================================*/

CSAORIBase* CreateInstance(void)
{
	return new CWiimoteGesture();
}

/*===============================================================
	初期化(DllMain縛り)
===============================================================*/
CWiimoteGesture::CWiimoteGesture(void) : thread_active(true), thread_handle(NULL) , thread_running(false), timer_count(0xff)
{
	m_pWiimote = new CyberGarage::Wiimote();
}

CWiimoteGesture::~CWiimoteGesture()
{
	delete m_pWiimote;
}

/*===============================================================
	初期化(DllMainとは別)
===============================================================*/
static unsigned int __stdcall WMGThreadProc(void* pv)
{
	CWiimoteGesture *pb = reinterpret_cast<CWiimoteGesture*>(pv);
	pb->ThreadProc();
	return 0;
}

bool CWiimoteGesture::load()
{
	thread_active = true;

	ZeroMemory(current_button,sizeof(current_button));
	return true;
}

bool CWiimoteGesture::unload()
{
	thread_active = false;
	if ( ::WaitForSingleObject(thread_handle,5000) == WAIT_TIMEOUT ) {
		::TerminateThread(thread_handle,0);
	}

	::CloseHandle(thread_handle);
	thread_handle = NULL;

	return true;
}

/*===============================================================
	通信部
===============================================================*/
#define SLEEP_TIME 20
#define ACTION_THRESHOULD 180
#define ACTION_THRESHOULD_COUNT 15

#define MAXMINFUNC(c) static int get_max_min_diff_ ## c ## (POINT3D *legend,int scan_start_pos,int &last_pos) \
{ \
	int min = 65535; \
	int max = 0; \
	size_t min_pos = 0; \
	size_t max_pos = 0; \
	if ( scan_start_pos < 0 ) { scan_start_pos = 0; } \
 \
	for ( int i = scan_start_pos ; i < ACTION_THRESHOULD_COUNT ; ++i ) { \
		if ( legend[i]. ## c ##  < min ) { \
			min = legend[i]. ## c ## ; \
			min_pos = i; \
		} \
		if ( legend[i]. ## c ##  > max ) { \
			max = legend[i]. ## c ## ; \
			max_pos = i; \
		} \
	} \
 \
	if ( min_pos <= max_pos ) { \
		last_pos = max_pos; \
		return max-min; \
	} \
	else { \
		last_pos = min_pos; \
		return min-max; \
	} \
}

MAXMINFUNC(x)
MAXMINFUNC(y)
MAXMINFUNC(z)

void CWiimoteGesture::ThreadProc()
{
	::Sleep(100);
	::SetThreadPriority(::GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);

	last_button[0] = 0;

	POINT3D last;
	last.x = m_pWiimote->getXMotion();
	last.y = m_pWiimote->getYMotion();
	last.z = m_pWiimote->getZMotion();

	POINT3D current;
	POINT3D legend[ACTION_THRESHOULD_COUNT];
	for ( size_t i = 0 ; i < ACTION_THRESHOULD_COUNT ; ++i ) {
		legend[i] = last;
	}

	m_pWiimote->setLEDs(false,false,false,true,false);

	char* last_action = NULL;
	size_t count = ACTION_THRESHOULD_COUNT;
	size_t last_action_count = 0;
	size_t last_led_update_count = 0;
	bool led = true;
	bool vibration = false;
	int scan_start_pos = 0;
	int last_max_value = 0;

	::timeBeginPeriod(5);

	while ( thread_active ) {
		if ( ! m_pWiimote->read() ) {
			break;
		}
		{ //ロック開放ブレース
			SAORI_FUNC::CCriticalSectionLock lock(thread_lock);

			if ( scan_start_pos > 0 ) {
				--scan_start_pos;
			}
			else {
				last_max_value = 0;
			}

			memmove(legend,legend+1,sizeof(legend[0])*(ACTION_THRESHOULD_COUNT-1));
			current.x = m_pWiimote->getXMotion();
			current.y = m_pWiimote->getYMotion();
			current.z = m_pWiimote->getZMotion();

			legend[ACTION_THRESHOULD_COUNT-1] = current;

			int x_pos,y_pos,z_pos;

			int x_r = get_max_min_diff_x(legend,scan_start_pos,x_pos);
			int y_r = get_max_min_diff_y(legend,scan_start_pos,y_pos);
			int z_r = get_max_min_diff_z(legend,scan_start_pos,z_pos);

			int maxax = 0;
			int max_abs = abs(x_r);

			int cur_abs = abs(y_r);
			if ( max_abs < cur_abs ) {
				maxax = 1;
				max_abs = cur_abs;
			}
			
			cur_abs = abs(z_r);
			if ( max_abs < cur_abs ) {
				maxax = 2;
				max_abs = cur_abs;
			}

			if ( (max_abs >= ACTION_THRESHOULD) && (max_abs >= last_max_value) ) {
				char* action;
				if ( maxax == 0 ) {
					if ( x_r < 0 ) {
						action = "left";
					}
					else {
						action = "right";
					}
					m_pWiimote->setLEDs(true,false,false,false,vibration);
					scan_start_pos = x_pos;
					last_max_value = abs(x_r);
				}
				else if ( maxax == 1 ) {
					if ( y_r < 0 ) {
						action = "push";
					}
					else {
						action = "pull";
					}
					m_pWiimote->setLEDs(false,true,false,false,vibration);
					scan_start_pos = y_pos;
					last_max_value = abs(y_r);
				}
				else /*if ( maxax == 2 )*/ {
					if ( z_r < 0 ) {
						action = "up";
					}
					else {
						action = "down";
					}
					m_pWiimote->setLEDs(false,false,true,false,vibration);
					scan_start_pos = z_pos;
					last_max_value = abs(z_r);
				}

				bool sstp_send = true;
				if ( last_action ) {
					if ( strcmp(last_action,action) == 0 ) {
						sstp_send = false;
					}
				}
				last_action = action;
				last_action_count = count;

				if ( sstp_send )  {
					std::string sstp = ""
						"NOTIFY SSTP/1.1\r\n"
						"Charset: Shift_JIS\r\n"
						"Sender: WiimoteGesture Plugin\r\n"
						"HWnd: 0\r\n"
						"Event: OnWiimoteGesture\r\n"
						"SecurityLevel: local\r\n"
						"Reference0: ";

					sstp += action;
					sstp += "\r\n\r\n";

					send_sstp(sstp);
				}
			}
			else {
				if ( last_action ) {
					if ( (count - last_action_count) >= ACTION_THRESHOULD_COUNT ) {
						m_pWiimote->setLEDs(false,false,false,true,false);
						last_action = NULL;
					}
				}
				else {
					if ( (count - last_led_update_count) >= ((1000/(SLEEP_TIME*ACTION_THRESHOULD_COUNT))*ACTION_THRESHOULD_COUNT) ) {
						led = ! led;
						m_pWiimote->setLEDs(false,false,false,led,false);
						last_led_update_count = count;
					}
				}
			}

			last = current;

			current_button[0] = 0;
			if ( m_pWiimote->IsAPressed() ) {
				strcat(current_button,"A");
			}
			if ( m_pWiimote->IsBPressed() ) {
				strcat(current_button,"B");
			}
			if ( m_pWiimote->IsOnePressed() ) {
				strcat(current_button,"1");
			}
			if ( m_pWiimote->IsTwoPressed() ) {
				strcat(current_button,"2");
			}
			if ( m_pWiimote->IsUpPressed() ) {
				strcat(current_button,"U");
			}
			if ( m_pWiimote->IsDownPressed() ) {
				strcat(current_button,"D");
			}
			if ( m_pWiimote->IsLeftPressed() ) {
				strcat(current_button,"L");
			}
			if ( m_pWiimote->IsRightPressed() ) {
				strcat(current_button,"R");
			}
			if ( m_pWiimote->IsMinusPressed() ) {
				strcat(current_button,"-");
			}
			if ( m_pWiimote->IsPlusPressed() ) {
				strcat(current_button,"+");
			}
			if ( m_pWiimote->IsHomePressed() ) {
				strcat(current_button,"H");
			}

			if ( strcmp(current_button,last_button) != 0 ) {
				std::string sstp = ""
					"NOTIFY SSTP/1.1\r\n"
					"Charset: Shift_JIS\r\n"
					"Sender: WiimoteGesture Plugin\r\n"
					"HWnd: 0\r\n"
					"Event: OnWiimoteButtonStatus\r\n"
					"SecurityLevel: local\r\n"
					"Reference0: ";

				sstp += current_button;
				sstp += "\r\n\r\n";

				send_sstp(sstp);

				strcpy(last_button,current_button);
			}
		}

		::Sleep(SLEEP_TIME);
		++count;
	}

	::timeEndPeriod(5);

	m_pWiimote->setLEDs(false,false,false,false,false);
	m_pWiimote->close();

	thread_running = false;
}

/*===============================================================
	実行部
===============================================================*/
void CWiimoteGesture::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnMenuExec") == 0 ) {
		SAORI_FUNC::AsyncMessageBox(NULL,L"WiimoteGesture\r\n"
			L"Wiiリモコンからゴーストに対してジェスチャー動作をします。"
			,L"WiimoteGesture",MB_OK | MB_ICONINFORMATION);

		out.result_code = SAORIRESULT_NO_CONTENT;
		return;
	}
	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSecondChange") == 0 ) {
		out.result_code = SAORIRESULT_NO_CONTENT;
		if ( ! thread_running ) {
			if ( thread_handle ) {
				if ( ::WaitForSingleObject(thread_handle,5000) == WAIT_TIMEOUT ) {
					::TerminateThread(thread_handle,0);
				}
				::CloseHandle(thread_handle);
				thread_handle = NULL;
			}
			++timer_count;
			if ( timer_count >= 3 ) {
				timer_count = 0;
				if ( m_pWiimote->open() ) {
					thread_running = true;
					thread_handle = reinterpret_cast<HANDLE>(_beginthreadex(NULL,0,WMGThreadProc,this,0,NULL));
				}
			}
		}
		return;
	}
	//--------------------------------------------------------
}

