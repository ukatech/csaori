// shiolink_adapter.cpp : コンソール アプリケーション用のエントリ ポイントの定義
//

//setlocale//
#ifdef _MSC_VER
#if _MSC_VER >= 1400
#pragma setlocale("japanese")
#endif
#endif
//setlocale end//

/*
 * csaori_base.cpp
 */

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#pragma warning( disable : 4996 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
 
#include "csaori_util.h"

//#define DEBUG_LOG 1

typedef BOOL (__cdecl *SHIORI_load)(HGLOBAL h, long len);
typedef BOOL (__cdecl *SHIORI_unload)();

typedef HGLOBAL (__cdecl *SHIORI_request)(HGLOBAL h, long* len);


static void readline_stdin(std::basic_string<char> &out)
{
	char c;

	while ( true ) {
		c = getchar();
		out += c;

		if ( out.length() >= 2 ) {
			if ( out[out.length()-2] == '\r' && out[out.length()-1] == '\n' ) {
				break;
			}
		}
	}

#ifdef DEBUG_LOG
	std::ofstream ofs;
	ofs.open("log.txt",std::ios::app | std::ios::binary);
	ofs << out;
	ofs.close();
#endif
}

static void writeline_stdout(const char *txt)
{
	char c;
	const char *out = txt;

	while ( true ) {
		c = *out;
		if ( c == 0 ) { break; }
		putchar(c);
		out += 1;
	}

#ifdef DEBUG_LOG
	std::ofstream ofs;
	ofs.open("log.txt",std::ios::app | std::ios::binary);
	ofs << txt;
	ofs.close();
#endif
}

int main(int argc, char *argv[])
{
	if ( argc <= 1 ) {
		return -1;
	}

	_setmode( _fileno( stdin ), _O_BINARY );
	_setmode( _fileno( stdout ), _O_BINARY );

	//DLL読み込み
	//先頭のアンダースコアは互換用

	std::basic_string<char> dll_path;
	dll_path = argv[1];

	HMODULE hDLL = ::LoadLibraryA(dll_path.c_str());
	if ( ! hDLL ) {
		return -1;
	}

	SHIORI_load pfLoad = (SHIORI_load)::GetProcAddress(hDLL,"load");
	if ( ! pfLoad ) {
		pfLoad = (SHIORI_load)::GetProcAddress(hDLL,"_load");
	}

	SHIORI_unload pfUnload = (SHIORI_unload)::GetProcAddress(hDLL,"unload");
	if ( ! pfUnload ) {
		pfUnload = (SHIORI_unload)::GetProcAddress(hDLL,"_unload");
	}

	SHIORI_request pfRequest = (SHIORI_request)::GetProcAddress(hDLL,"request");
	if ( ! pfRequest ) {
		pfRequest = (SHIORI_request)::GetProcAddress(hDLL,"_request");
	}

	if ( pfLoad == NULL || pfUnload == NULL || pfRequest == NULL ) {
		return -1;
	}

	//カレントディレクトリ設定
	const char *s = dll_path.c_str();
	const char *p = strrchr(dll_path.c_str(),'\\');

	std::basic_string<char> work_dir(s,p-s+1);
	::SetCurrentDirectory(work_dir.c_str());

	//メインループ
	bool loopFlag = true;

	while ( loopFlag ) {
		std::basic_string<char> line;
		
		readline_stdin(line);
		if ( line.size() == 0 ) { continue; }
		
		if ( line[line.size()-1] == '\n' ) { line.erase(line.size()-1,1); }
		if ( line[line.size()-1] == '\r' ) { line.erase(line.size()-1,1); }
		if ( line.size() < 3 ) { continue; }

		if ( line[0] != '*' || line[2] != ':' ) { continue; }

		switch ( line[1] ) {
		case 'L': //load
			{
				std::basic_string<char> path(line.c_str()+3);
				if ( path[path.length()-1] != '\\' || path[path.length()-1] != '/' ) {
					path += "\\";
				}

				long size = path.length();

				HGLOBAL h = ::GlobalAlloc(GMEM_FIXED,size+1);
				memcpy(h,path.c_str(),size+1);

				int result = pfLoad(h,size);
			}
			break;

		case 'U': //unload
			{
				pfUnload();

				loopFlag = false;
			}
			break;

		case 'S':
			{
				writeline_stdout(line.c_str());
				writeline_stdout("\r\n");
				fflush(stdout);

				std::basic_string<char> rq;
				std::basic_string<char> rline;

				//Request
				while ( true ) {
					rline.erase();
					readline_stdin(rline);
					if ( rline.size() == 0 ) { continue; }

					rq += rline;

					if ( (rline.size() == 2) && (rline[0] == '\r') && (rline[1] == '\n') ) {
						break;
					}
				}

				LONG rsize = rq.size();

				HGLOBAL rh = ::GlobalAlloc(GMEM_FIXED,rsize+1);
				memcpy(rh,rq.c_str(),rq.size());
				((char*)rh)[rsize] = 0;

				HGLOBAL sh = pfRequest(rh,&rsize);

				//Response
				std::basic_string<char> rs((const char*)sh,rsize);

				if ( rs.find_first_of("\r\n\r\n") < 0 ) {
					rs += "\r\n";
				}

				writeline_stdout(rs.c_str());
				fflush(stdout);

				::GlobalFree(sh);

			}
			break;
		}
	}

	//おそうじ
	::FreeLibrary(hDLL);
	return 0;
}
