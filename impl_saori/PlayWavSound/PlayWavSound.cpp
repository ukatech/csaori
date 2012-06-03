#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "csaori.h"

/*---------------------------------------------------------
	èâä˙âª
---------------------------------------------------------*/
bool CSAORI::load(){
	return true;
}

/*---------------------------------------------------------
	âï˙
---------------------------------------------------------*/
bool CSAORI::unload(){
	return true;
}

/*---------------------------------------------------------
	é©ìÆfree
---------------------------------------------------------*/
class CAutoFree {
private:
	CAutoFree() { }

	void *m_pBuf;
public:
	CAutoFree(void* pBuf) : m_pBuf(pBuf) { }
	~CAutoFree() { if ( m_pBuf ) { free(m_pBuf); } }

	operator void*() { return m_pBuf; }
};

/*---------------------------------------------------------
	é¿çs
---------------------------------------------------------*/
void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out){
	out.result_code = SAORIRESULT_BAD_REQUEST;
	if ( in.args.size() < 1 ) { return; }

	if ( wcsicmp(in.args[0].c_str(),L"hwnd") == 0 ) {
		out.result_code = SAORIRESULT_NO_CONTENT;
		return;
	}

	DWORD flag = SND_ASYNC;
	std::string filepath;

	if ( wcsnicmp(in.args[0].c_str(),L"system:",7) == 0 ) {
		flag |= SND_NODEFAULT | SND_ALIAS;
		filepath = SAORI_FUNC::UnicodeToMultiByte(in.args[0].c_str()+7);
	}
	else {
		flag |= SND_NODEFAULT | SND_FILENAME;
		filepath = checkAndModifyPath(SAORI_FUNC::UnicodeToMultiByte(in.args[0]));
	}

	out.result_code = SAORIRESULT_OK;

	BOOL result = ::PlaySound(filepath.c_str(),NULL,flag);

	char_t buf[32];
	swprintf(buf,L"%d",result);

	out.result = buf;
}

