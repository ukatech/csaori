#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

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
	out.result_code = SAORIRESULT_OK;
	return;
}
