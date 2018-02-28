#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "csaori.h"

/*---------------------------------------------------------
	èâä˙âª
---------------------------------------------------------*/
bool CSAORI::load()
{
	return true;
}

/*---------------------------------------------------------
	âï˙
---------------------------------------------------------*/
bool CSAORI::unload()
{
	return true;
}

/*---------------------------------------------------------
	é¿çs
---------------------------------------------------------*/
void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	out.result_code = SAORIRESULT_BAD_REQUEST;

	if ( in.args.size() ) {
		string_t str = in.args[0];
		for ( int i = 1 ; i < in.args.size() ; ++i ) {
			str += L"\r\n";
			str += in.args[i];
		}
		
		string_t::size_type pos(str.find(L"\\n"));
		while ( pos != string_t::npos ) {
			str.replace(pos,2,L"\r\n");
			pos = str.find(L"\\n");
		}

		std::string astr = SAORI_FUNC::UnicodeToMultiByte(str,CP_SJIS,0);

		HANDLE hMem = ::GlobalAlloc( GMEM_SHARE | GMEM_MOVEABLE , astr.length() + 1 );
		char *p = (char*)::GlobalLock(hMem);
		strcpy(p,astr.c_str());
		::GlobalUnlock(p);

		if ( ::OpenClipboard(NULL) ) {
			::EmptyClipboard();
			::SetClipboardData(CF_TEXT,hMem);
			::CloseClipboard();
			out.result_code = SAORIRESULT_OK;
		}
	}
}
