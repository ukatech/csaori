#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "csaori.h"
#include "gainercxx.h"

static std::vector<Gainer*> g_gainer;
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
		Gainer::Search(port_array);

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

	if ( wcsicmp(subcommand.c_str(),L"set.hwnd") == 0 ) {
		if ( in.args.size() < 2 ) {
			return;
		}
		out.result_code = SAORIRESULT_OK;
		out.values.push_back(L"OK");
		g_hwnd = reinterpret_cast<HWND>(wcstoul(in.args[1].c_str(),NULL,10));
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

	//Gainer初期化
	Gainer *pGainer = NULL;
	if ( port == 0 ) {
		if ( g_gainer.size() == 0 ) {
			std::vector<int> port_array;
			Gainer::Search(port_array);

			if ( port_array.size() == 0 ) {
				out.result_code = SAORIRESULT_NO_CONTENT;
				out.result = L"Gainer not found.";
				return;
			}

			port = port_array[0];
		}
		else {
			pGainer = g_gainer[0];
		}
	}

	if ( ! pGainer ) {
		size_t n = g_gainer.size();
		for ( size_t i = 0 ; i < n ; ++i ) {
			if ( g_gainer[i]->GetCOMPort() == port ) {
				pGainer = g_gainer[i];
			}
		}
	}

	if ( ! pGainer ) {
		pGainer = new Gainer;
		if ( ! pGainer->Init(port) ) {
			out.result_code = SAORIRESULT_NO_CONTENT;
			out.result = L"Gainer not found.";
			return;
		}

		g_gainer.push_back(pGainer);
	}

	//以降パラメータ2の場合のコマンド群
	if ( wcsicmp(subcommand.c_str(),L"get.version") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = SAORI_FUNC::MultiByteToUnicode(pGainer->Version());
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"in.analog.all") == 0 ) {
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

	if ( wcsnicmp(subcommand.c_str(),L"in.digital.all",14) == 0 ) {
		out.result_code = SAORIRESULT_OK;

		WORD result;
		size_t bits;
		if ( ! pGainer->GetDigitalAll(result,bits) ) {
			out.result = L"NG";
		}
		else {
			out.result = L"OK";

			char_t buf[32];
			if ( wcsnicmp(subcommand.c_str()+14,L".bit",4) == 0 ) {
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
			else {
				swprintf(buf,L"%u",result);
				out.values.push_back(buf);
			}
		}
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"in.digital.continuous") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = L"OK";
		pGainer->ExecuteContinuousDigital();
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"in.analog.continuous") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = L"OK";
		pGainer->ExecuteContinuousAnalog();
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"in.continuous.exit") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = L"OK";
		pGainer->ExecuteExitContinuous();
		return;
	}

	//******************** パラメータ3 ********************
	if ( in.args.size() < 3 ) {
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"set.config") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = pGainer->SetConfiguration(_wtoi(in.args[2].c_str())) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"set.pga.dgnd") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = pGainer->SetPGA(wcstod(in.args[2].c_str(),NULL),0) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"set.pga.agnd") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = pGainer->SetPGA(wcstod(in.args[2].c_str(),NULL),1) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"out.led") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		out.result = pGainer->SetLED(_wtoi(in.args[2].c_str()) != 0) ? L"OK" : L"NG";
		return;
	}

	if ( wcsnicmp(subcommand.c_str(),L"out.digital.all",15) == 0 ) {
		out.result_code = SAORIRESULT_OK;

		DWORD outd = 0;
		if ( wcsnicmp(subcommand.c_str()+15,L".bit",4) == 0 ) {
			size_t n = in.args.size();
			for ( size_t i = 2 ; i < n ; ++i ) {
				outd |= 1UL << (i-2);
			}
		}
		else {
			outd = _wtoi(in.args[2].c_str());
		}
		out.result = pGainer->SetDigitalAll(outd) ? L"OK" : L"NG";
		return;
	}

	//******************** パラメータ4 ********************
	if ( in.args.size() < 4 ) {
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"out.analog.single") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		int port = _wtoi(in.args[2].c_str());
		int data = _wtoi(in.args[3].c_str());

		out.result = pGainer->SetAnalogSingle(port,data) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"out.digital.single") == 0 ) {
		out.result_code = SAORIRESULT_OK;
		
		int port = _wtoi(in.args[2].c_str());
		int data = _wtoi(in.args[3].c_str());

		out.result = pGainer->SetDigitalSingle(port,data != 0) ? L"OK" : L"NG";
		return;
	}

	if ( wcsicmp(subcommand.c_str(),L"out.servo.single") == 0 ) {
		out.result_code = SAORIRESULT_OK;

		int port = _wtoi(in.args[2].c_str());
		int data = _wtoi(in.args[3].c_str());

		out.result = pGainer->SetServoSingle(port,data) ? L"OK" : L"NG";
		return;
	}


}

