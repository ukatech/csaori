#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include "csaori.h"
#include "csaori_util.h"

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
	out.result_code = SAORIRESULT_OK;
	string_t a;
	for (auto& i : in.args) {
		a += L" " + i;
	}
	auto i = _wsystem(a.c_str());
	if(i!=-1)
		out.result = SAORI_FUNC::numToString(i);
	else
		out.result_code = SAORIRESULT_BAD_REQUEST;
	return;
}
