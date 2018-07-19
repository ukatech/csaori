#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "csaori.h"

class CSAORIProxy : public CSAORI
{
private:
	string_t delim;
	UINT cp;

public:
	CSAORIProxy() : delim(L" "), cp(CP_OEMCP) {}
	virtual ~CSAORIProxy() {}

	virtual bool load();
	virtual bool unload();
	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
};

CSAORIBase* CreateInstance(void)
{
	return new CSAORIProxy();
}

/*---------------------------------------------------------
	初期化
---------------------------------------------------------*/
bool CSAORIProxy::load()
{
	return true;
}

/*---------------------------------------------------------
	解放
---------------------------------------------------------*/
bool CSAORIProxy::unload()
{
	return true;
}

/*---------------------------------------------------------
	実行
---------------------------------------------------------*/
void CSAORIProxy::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	out.result_code = SAORIRESULT_BAD_REQUEST;

	if ( in.args.size() ) {
		string_t cmd = in.args[0];

		if ( cmd == L"hwnd") {
			out.result_code = SAORIRESULT_NO_CONTENT;
			//NOOP
		}
		else if ( cmd == L"/delim" ) {
			if ( in.args.size() >= 2 ) {
				delim = in.args[1];
				out.result_code = SAORIRESULT_NO_CONTENT;
			}
		}
		else if ( cmd == L"/charset" ) {
			if ( in.args.size() >= 2 ) {
				if ( in.args[1].length() == 0 ) {
					cp = CP_OEMCP;
				}
				else {
					cp = SAORI_FUNC::StringtoCodePage(SAORI_FUNC::UnicodeToMultiByte(in.args[1]).c_str());
				}
			}
			else {
				cp = CP_OEMCP;
			}
			out.result_code = SAORIRESULT_NO_CONTENT;
		}
		else {
			string_t exe = cmd;

			string_t param;
			for ( int i = 1 ; i < in.args.size() ; ++i ) {
				param += in.args[i];
				param += L" ";
			}

			std::string exe_a = SAORI_FUNC::UnicodeToMultiByte(exe);

			/*string_t exe_name = exe;
			char_t *pBS = wcsrchr(exe.c_str(),L'\\');
			if ( pBS ) {
				exe_name = pBS + 1;
			}

			std::string exe_name_a = SAORI_FUNC::UnicodeToMultiByte(exe_name);*/

			std::string exe_full = SAORI_FUNC::UnicodeToMultiByte(getModulePath());
			if ( exe_full[exe_full.length()-1] != '\\' ) {
				exe_full += "\\";
			}
			exe_full += exe_a;

			if ( ::GetFileAttributes(exe_full.c_str()) != 0xFFFFFFFFU ) {
				exe_a = exe_full;
			}

			std::string param_a = exe_a;
			if ( param.length() > 0 ) {
				param_a += " ";
				param_a += SAORI_FUNC::UnicodeToMultiByte(param);
			}

			//パイプをつないでプロセス起動
			//今後の機能拡張のため双方向パイプをつないでいるが片方向でとりあえず良い

			HANDLE hOutputRead = NULL,hInputWrite = NULL;
			HANDLE hProcess = NULL;

			bool bootSuccess = false;

			{
				HANDLE hOutputReadTmp,hOutputWrite;
				HANDLE hInputWriteTmp,hInputRead;
				//HANDLE hErrorWrite;
				SECURITY_ATTRIBUTES sa;
				
				sa.nLength= sizeof(SECURITY_ATTRIBUTES);
				sa.lpSecurityDescriptor = NULL;
				sa.bInheritHandle = TRUE;
				
				if ( ! ::CreatePipe(&hOutputReadTmp,&hOutputWrite,&sa,0) ) {
					return;
				}	
				/*if ( ! ::DuplicateHandle(GetCurrentProcess(),hOutputWrite,::GetCurrentProcess(),&hErrorWrite,0,TRUE,DUPLICATE_SAME_ACCESS) ) {
					return FALSE;
				}*/
				
				if ( ! ::CreatePipe(&hInputRead,&hInputWriteTmp,&sa,0) ) {
					return;
				}
				if ( ! ::DuplicateHandle(GetCurrentProcess(),hOutputReadTmp,::GetCurrentProcess(),&hOutputRead,0,FALSE,DUPLICATE_SAME_ACCESS) ) {
					return;
				}
				if ( ! ::DuplicateHandle(GetCurrentProcess(),hInputWriteTmp,::GetCurrentProcess(),&hInputWrite,0,FALSE,DUPLICATE_SAME_ACCESS) ) {
					return;
				}		
				
				::CloseHandle(hOutputReadTmp);
				::CloseHandle(hInputWriteTmp);

				PROCESS_INFORMATION pi;
				STARTUPINFO si;
				
				memset(&si,0,sizeof(si));
				si.cb = sizeof(STARTUPINFO);
				si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
				si.hStdOutput = hOutputWrite;
				si.hStdInput  = hInputRead;
				//si.hStdError  = hErrorWrite;
				si.wShowWindow = SW_HIDE;

				bootSuccess = (::CreateProcess(NULL/*exe_name_a.c_str()*/,(char*)param_a.c_str(),NULL,NULL,TRUE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi) != 0);

				if ( bootSuccess ) {
					::CloseHandle(pi.hThread);
					hProcess = pi.hProcess;
				}

				::CloseHandle(hOutputWrite);
				::CloseHandle(hInputRead);
			}

			if ( bootSuccess ) {
				DWORD outSize;
				char buf[1024];

				std::string out_a;

				while ( true ) {
					outSize = 0;
					
					if ( ::ReadFile(hOutputRead,buf,sizeof(buf)-1,&outSize,NULL) == 0 ) {
						break;
					}
					if ( outSize == 0 ) {
						break;
					}
					buf[outSize] = 0;
					out_a += buf;
				}

				string_t out_w = SAORI_FUNC::MultiByteToUnicode(out_a.c_str(),cp);
				
				for ( size_t i = 0 ; i < (out_w.length()-1) ; i++ ) {
					if ( out_w[i] == L'\r' && out_w[i+1] == L'\n' ) {
						out_w.replace(i,2,delim);
					}
				}
				
				::WaitForSingleObject(hProcess,INFINITE);

				::CloseHandle(hOutputRead);
				::CloseHandle(hInputWrite);
				::CloseHandle(hProcess);
		
				out.result_code = SAORIRESULT_OK;
				out.result = out_w;
			}
		}
	}
}
