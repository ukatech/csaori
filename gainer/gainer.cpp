#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "csaori.h"
#include "gainercxx.h"

//////////WINDOWS DEFINE///////////////////////////
//includeのあとにおいてね！
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

static std::vector<CGainer*> g_gainer;
static HWND g_hwnd;

/*---------------------------------------------------------
	初期化
---------------------------------------------------------*/
bool CSAORI::load()
{
	g_gainer.clear();
	return true;
}

/*---------------------------------------------------------
	解放
---------------------------------------------------------*/
bool CSAORI::unload()
{
	size_t n = g_gainer.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		g_gainer[i]->Exit();
		delete g_gainer[i];
	}
	g_gainer.clear();
	return true;
}

/*---------------------------------------------------------
	実行
---------------------------------------------------------*/
void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	out.result_code = SAORIRESULT_BAD_REQUEST;

	//******************** パラメータ1 = サブコマンド ********************
	if ( ! in.args.size() ) {
		return;
	}
	string_t subcommand = in.args[0];

	if ( wcsicmp(subcommand.c_str(),L"search") == 0 ) {
		std::vector<int> port_array;
		CGainer::Search(port_array);

		char_t buf[32];

		out.result_code = SAORIRESULT_OK;
		swprintf(buf,L"%u",port_array.size());
		out.result = buf;

		size_t n = port_array.size();
		for ( size_t i = 0 ; i < n ; ++i ) {
			swprintf(buf,L"%i",port_array[i]);
			out.values.push_back(buf);
		}
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"hwnd") == 0 ) {
		if ( in.args.size() < 2 ) {
			return;
		}
		out.result_code = SAORIRESULT_OK;
		out.values.push_back(L"OK");
		g_hwnd = reinterpret_cast<HWND>(wcstoul(in.args[1].c_str(),NULL,10));

		size_t n = g_gainer.size();
		for ( size_t i = 0 ; i < n ; ++i ) {
			g_gainer[i]->SetHWND(g_hwnd);
		}
		return;
	}

	//******************** パラメータ2 = ポート番号 ********************
	if ( in.args.size() < 2 ) {
		return;
	}
	int port = _wtoi(in.args[1].c_str());
	if ( port < 0 || port > 128 ) {
		return;
	}

	//CGainer初期化
	CGainer *pGainer = NULL;
	int p_gainer_pos = 0;

	if ( port == 0 ) {
		if ( g_gainer.size() == 0 ) {
			std::vector<int> port_array;
			CGainer::Search(port_array);

			if ( port_array.size() == 0 ) {
				out.result_code = SAORIRESULT_NO_CONTENT;
				out.result = L"NG:Gainer not found.";
				return;
			}

			port = port_array[0];
		}
		else {
			pGainer = g_gainer[0];
			p_gainer_pos = 0;
		}
	}

	if ( ! pGainer ) {
		size_t n = g_gainer.size();
		for ( size_t i = 0 ; i < n ; ++i ) {
			if ( g_gainer[i]->GetCOMPort() == port ) {
				pGainer = g_gainer[i];
				p_gainer_pos = i;
			}
		}
	}

	if ( ! pGainer ) {
		pGainer = new CGainer;
		if ( ! pGainer->Init(port) ) {
			out.result_code = SAORIRESULT_NO_CONTENT;
			out.result = L"NG:Gainer not found.";
			return;
		}
		pGainer->SetHWND(g_hwnd);

		g_gainer.push_back(pGainer);
		p_gainer_pos = g_gainer.size() - 1;
	}

	//以降パラメータ2の場合のコマンド群
	if ( wcsicmp(subcommand.c_str(),L"version") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = SAORI_FUNC::MultiByteToUnicode(pGainer->Version());
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"reboot") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		pGainer->Reboot();
		out.result = L"OK";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"disconnect") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		delete pGainer;
		g_gainer.erase(g_gainer.begin() + p_gainer_pos);
		out.result = L"OK";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"peekAnalogInput") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		std::vector<BYTE> result;
		if ( ! pGainer->GetAnalogAll(result) ) {
			out.result = L"NG";
		}
		else {
			out.result = L"OK";
			char_t buf[32];
			size_t n = result.size();
			for ( size_t i = 0 ; i < n ; ++i ) {
				swprintf(buf,L"%d",result[i]);
				out.values.push_back(buf);
			}
		}
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"peekDigitalInput") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		WORD result;
		size_t bits;
		if ( ! pGainer->GetDigitalAll(result,bits) ) {
			out.result = L"NG";
		}
		else {
			out.result = L"OK";

			DWORD r2 = result;
			for ( size_t i = 0 ; i < bits ; ++i ) {
				if ( (r2 & (0UL << i)) != 0 ) {
					out.values.push_back(L"1");
				}
				else {
					out.values.push_back(L"0");
				}
			}
		}
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"beginDigitalInput") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = L"OK";
		DWORD period = 0;
		if ( in.args.size() >= 3 ) {
			period = wcstoul(in.args[2].c_str(),NULL,10);
		}
		pGainer->ExecuteContinuousDigital(period);
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"beginAnalogInput") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = L"OK";
		DWORD period = 0;
		if ( in.args.size() >= 3 ) {
			period = wcstoul(in.args[2].c_str(),NULL,10);
		}
		pGainer->ExecuteContinuousAnalog(period);
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"endAnalogInput") == 0 ||
		wcsicmp(subcommand.c_str(),L"endDigitalInput") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = L"OK";
		pGainer->ExecuteExitContinuous();
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"turnOnLED") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = pGainer->SetLED(1) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"turnOffLED") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = pGainer->SetLED(0) ? L"OK" : L"NG";
		return;
	}

	//******************** パラメータ3 ********************
	if ( in.args.size() < 3 ) {
		return;
	}

	if ( wcsnicmp(subcommand.c_str(),L"config",6) == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = pGainer->SetConfiguration(_wtoi(in.args[2].c_str())) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"ampGainDGND") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = pGainer->SetPGA(wcstod(in.args[2].c_str(),NULL),0) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"ampGainAGND") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = pGainer->SetPGA(wcstod(in.args[2].c_str(),NULL),1) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"setHigh") == 0 || wcsicmp(subcommand.c_str(),L"setLow") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		
		int port = _wtoi(in.args[2].c_str());

		out.result = pGainer->SetDigitalSingle(port,wcsicmp(subcommand.c_str(),L"setLow") != 0) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"analogOutput") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		size_t n = in.args.size();
		std::vector<WORD> data;

		for ( size_t i = 2 ; i < n ; ++i ) {
			int d = _wtol(in.args[i].c_str());
			data.push_back(static_cast<WORD>(d));
		}
		out.result = pGainer->SetAnalogAll(data) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"digitalOutput") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		DWORD outd = 0;
		size_t n = in.args.size();

		for ( size_t i = 2 ; i < n ; ++i ) {
			if ( _wtol(in.args[i].c_str()) ) {
				outd |= 1UL << (i-2);
			}
		}
		out.result = pGainer->SetDigitalAll(outd) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"servoOutput") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		size_t n = in.args.size();
		std::vector<WORD> data;

		for ( size_t i = 2 ; i < n ; ++i ) {
			int d = _wtol(in.args[i].c_str());
			data.push_back(static_cast<WORD>(d));
		}
		out.result = pGainer->SetServoAll(data) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"scanMatrix") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		BYTE data[GAINER_LED_MATRIX][GAINER_LED_MATRIX];
		ZeroMemory(data,sizeof(data));

		size_t n = in.args.size() - 2;
		if ( n > GAINER_LED_MATRIX ) { n = GAINER_LED_MATRIX; }

		string_t buf;
		string_t str;
		int cutAt;
		size_t count = 0;
		for ( size_t i = 0 ; i < n ; ++i ) {
			str = in.args[i+2];
			count = 0;

			while ( (cutAt = str.find_first_of(L',')) != str.npos ) {
				if ( cutAt > 0 ) {
					buf = str.substr(0, cutAt);
				}
				str = str.substr(cutAt + 1);
				
				data[i][count] = _wtol(buf.c_str());
				++count;
				if ( count >= GAINER_LED_MATRIX-1 ) { break; } //とりあえず7要素目まで
			}
			if ( str.length() > 0 ) {
				data[i][count] = _wtol(str.c_str());
			}
		}

		out.result = pGainer->ScanMatrix(data) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"scan7segLED") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		char_t proctext[GAINER_LED_MATRIX];
		ZeroMemory(proctext,sizeof(proctext));

		bool procdot[GAINER_LED_MATRIX];
		ZeroMemory(procdot,sizeof(procdot));

		int i;
		int count = GAINER_LED_MATRIX;
		const string_t &s = in.args[2];

		//配列入れ替えとドット抽出
		for ( i = s.size() - 1; i >= 0 ; --i ) {
			if ( s[i] >= L'0' && s[i] <= L'9' ) {
				proctext[GAINER_LED_MATRIX-count] = s[i];
				--count;
			}
			else if ( s[i] == L'.' ) {
				procdot[GAINER_LED_MATRIX-count] = true;
			}
			if ( count <= 0 ) { break; }
		}
		//最後の0を消す
		for ( i = GAINER_LED_MATRIX-1 ; i >= 1 ; --i ) {
			if ( proctext[i] == 0 || proctext[i] == L'0' ) {
				proctext[i] = 0;
			}
			else {
				break;
			}
		}

		//7セグLED配列
		static const bool led_7seg_data[10][7] = {
			{1,1,1,1,1,1,0}, //0
			{0,1,1,0,0,0,0}, //1
			{1,1,0,1,1,0,1}, //2
			{1,1,1,1,0,0,1}, //3
			{0,1,1,0,0,1,1}, //4
			{1,0,1,1,0,1,1}, //5
			{1,0,1,1,1,1,1}, //6
			{1,1,1,0,0,0,0}, //7
			{1,1,1,1,1,1,1}, //8
			{1,1,1,1,0,1,1}  //9
		};

		BYTE data[GAINER_LED_MATRIX][GAINER_LED_MATRIX];
		ZeroMemory(data,sizeof(data));

		//出力
		for ( i = 0 ; i < GAINER_LED_MATRIX ; ++i ) {
			if ( proctext[i] == 0 ) { break; }

			const bool *segdata = led_7seg_data[proctext[i]-L'0'];
			for ( int j = 0 ; j < 7 ; ++j ) {
				if ( segdata[j] ) {
					data[j][GAINER_LED_MATRIX-i-1] = 0xF;
				}
			}
			if ( procdot[i] ) {
				data[7][GAINER_LED_MATRIX-i-1] = 0xF;
			}
		}

		out.result = pGainer->ScanMatrix(data) ? L"OK" : L"NG";
		return;
	}

	//******************** パラメータ4 ********************
	if ( in.args.size() < 4 ) {
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"analogOutputSingle") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		int port = _wtoi(in.args[2].c_str());
		int data = _wtoi(in.args[3].c_str());

		out.result = pGainer->SetAnalogSingle(port,data) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"digitalOutputSingle") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		
		int port = _wtoi(in.args[2].c_str());
		int data = _wtoi(in.args[3].c_str());

		out.result = pGainer->SetDigitalSingle(port,data != 0) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"servoOutputSingle") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		int port = _wtoi(in.args[2].c_str());
		int data = _wtoi(in.args[3].c_str());

		out.result = pGainer->SetServoSingle(port,data) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"scanLine") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		int raw = _wtoi(in.args[2].c_str());
		BYTE data[GAINER_LED_MATRIX];
		ZeroMemory(data,sizeof(data));

		int cutAt;
		string_t str = in.args[3];
		string_t buf;
		size_t count = 0;

		while ( (cutAt = str.find_first_of(L',')) != str.npos ) {
			if ( cutAt > 0 ) {
				buf = str.substr(0, cutAt);
			}
			str = str.substr(cutAt + 1);
			
			data[count] = _wtol(buf.c_str());
			++count;
			if ( count >= GAINER_LED_MATRIX-1 ) { break; } //とりあえず7要素目まで
		}
		if ( str.length() > 0 ) {
			data[count] = _wtol(str.c_str());
		}

		out.result = pGainer->ScanLine(raw,data) ? L"OK" : L"NG";
		return;
	}


}

