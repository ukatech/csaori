#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include "csaori.h"


std::wstring GetSteamPath();

std::string GetSteamMostRecentUserPersonaName();


/*---------------------------------------------------------
	初期化
---------------------------------------------------------*/
bool CSAORI::load()
{
	return true;
}

/*---------------------------------------------------------
	解放
---------------------------------------------------------*/
bool CSAORI::unload()
{
	return true;
}

/*---------------------------------------------------------
	実行
---------------------------------------------------------*/
void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	if (in.args.size()) {
		if(in.args[0] == L"GetSteamPath") {
			out.result = GetSteamPath();
			out.result_code = SAORIRESULT_OK;
		}
		else if(in.args[0] == L"GetMostRecentUserPersonaName") {
			out.result = SAORI_FUNC::MultiByteToUnicode(GetSteamMostRecentUserPersonaName(),CP_UTF8);
			out.result_code = SAORIRESULT_OK;
		}
		else
			out.result_code = SAORIRESULT_BAD_REQUEST;
	}
	return;
}
