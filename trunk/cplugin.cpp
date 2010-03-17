/*
 * csaori.cpp
 * 
 * written by Ukiya http://ukiya.sakura.ne.jp/
 * based by Ç¶Ç—Ç≥ÇÌól "gethwnd.dll"
 */

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#pragma warning( disable : 4996 )
#endif

#include "cplugin.h"

//////////WINDOWS DEFINE///////////////////////////
//includeÇÃÇ†Ç∆Ç…Ç®Ç¢ÇƒÇÀÅI
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

const string_t& CPLUGIN::s_saori_version(void) const
{
	static string_t s = L"PLUGIN/2.0";
	return s;
}
const string_t& CPLUGIN::s_saori_def(void) const
{
	static string_t s = L" PLUGIN/";
	return s;
}
const string_t& CPLUGIN::s_saori_argument(void) const
{
	static string_t s = L"Reference";
	return s;
}
const string_t& CPLUGIN::s_saori_value(void) const
{
	static string_t s = L"Reference";
	return s;
}
const string_t& CPLUGIN::s_saori_result(void) const
{
	static string_t s = L"Script";
	return s;
}

void CPLUGIN::exec_before(const CSAORIInput& in,CSAORIOutput& out)
{
	script.erase();
	script_option.erase();
	event.erase();
	event_option.erase();
	target.erase();
	marker.erase();
	security_level.erase();
}

void CPLUGIN::exec_after(const CSAORIInput& in,CSAORIOutput& out)
{
	if ( script.length() ) {
		out.result = script;
	}
	if ( script_option.length() ) {
		out.opts[L"ScriptOption"] = script_option;
	}
	if ( event.length() ) {
		out.opts[L"Event"] = event;
	}
	if ( event_option.length() ) {
		out.opts[L"EventOption"] = event_option;
	}
	if ( target.length() ) {
		out.opts[L"Target"] = target;
	}
	if ( marker.length() ) {
		out.opts[L"Marker"] = marker;
	}
	if ( security_level.length() ) {
		out.opts[L"SecurityLevel"] = security_level;
	}
}

CSAORIBase* CreateInstance(void)
{
	return new CPLUGIN();
}

