#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "csaori.h"

extern std::string module_path;

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out){
	if(in.args.size()!=2){
		out.result=L"";
		out.result_code=SAORIRESULT_BAD_REQUEST;
		return;
	}

	int x=_wtoi(in.args[0].c_str());
	int y=_wtoi(in.args[1].c_str());
	bool result=::SetCursorPos(x,y);
	if(!result){
		out.result=L"Error:"+SAORI_FUNC::intToString(::GetLastError());
		out.result_code=SAORIRESULT_INTERNAL_SERVER_ERROR;
		return;
	}

	out.result=L"";
	out.result_code=SAORIRESULT_OK;
	return;
}

bool CSAORI::unload(){
	return true;
}

bool CSAORI::load(){
	return true;
}

//---------------------ƒnƒ“ƒhƒŠƒ“ƒO

