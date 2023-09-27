#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include "../../csaori.h"
#include "bignum_impl.hpp"

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
void CSAORI::exec(const CSAORIInput& in, CSAORIOutput& out) {
	out.result_code = SAORIRESULT_BAD_REQUEST;
	if(in.args.size() < 3)
		return;
	if(in.args[2].size() == 0)
		return;
	out.result_code = SAORIRESULT_OK;
	out.result		= bignum_cacl(in.args[0],in.args[1],in.args[2]);
	return;
}
