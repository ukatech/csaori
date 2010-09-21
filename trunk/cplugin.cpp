//setlocale//
#ifdef _MSC_VER
#if _MSC_VER >= 1400
#pragma setlocale("japanese")
#endif
#endif
//setlocale end//

/*
 * csaori.cpp
 * 
 * written by Ukiya http://ukiya.sakura.ne.jp/
 * based by Mr.EBISAWA "gethwnd.dll"
 */

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#pragma warning( disable : 4996 )
#endif

#include "cplugin.h"

//////////WINDOWS DEFINE///////////////////////////
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

	language.erase();
	map_strpair::const_iterator itr = in.opts.find(L"Language");
	if ( itr != in.opts.end() ) {
		language = itr->second;
	}

	sender.erase();
	itr = in.opts.find(L"Sender");
	if ( itr != in.opts.end() ) {
		sender = itr->second;
	}
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

	if ( in.args.size() >= 5 ) {
		if ( wcsicmp(in.id.c_str(),L"OnGhostBoot") == 0 || wcsicmp(in.id.c_str(),L"OnGhostInfoUpdate") == 0 ) {
			string_t path = in.args[4];

			ghost_map_type::iterator itr = ghost_map.find(path);

			SAORI_FUNC::CCriticalSectionLock lock(sstp_thread_lock);

			CGhostInfo *pGI;
			if ( itr != ghost_map.end() ) {
				pGI = &(itr->second);
			}
			else {
				pGI = &(ghost_map[path]);
			}

			pGI->hwnd = reinterpret_cast<void*>(wcstoul(in.args[0].c_str(),NULL,10));
			pGI->name = in.args[1];
			pGI->shell_name = in.args[2];
			pGI->id = in.args[3];
			pGI->path = path;
			if ( in.args.size() >= 6 ) {
				pGI->shell_path = in.args[5];
			}
			else {
				pGI->shell_path.erase();
			}
		}
		else if ( wcsicmp(in.id.c_str(),L"OnGhostExit") == 0 ) {
			string_t path = in.args[4];

			SAORI_FUNC::CCriticalSectionLock lock(sstp_thread_lock);

			ghost_map_type::iterator itr = ghost_map.find(path);
			if ( itr != ghost_map.end() ) {
				ghost_map.erase(itr);
			}
		}
	}
}

void CPLUGIN::send_sstp(const std::string &str,void* hwnd)
{

	if ( ! hwnd ) {
		SAORI_FUNC::CCriticalSectionLock lock(sstp_thread_lock);
		
		DWORD result;

		COPYDATASTRUCT c;
		c.dwData = 9801;
		c.cbData = str.size();
		c.lpData = const_cast<char*>(str.c_str());

		for ( ghost_map_type::iterator itr = ghost_map.begin() ; 
			itr != ghost_map.end() ; ++itr ) {
			hwnd = itr->second.hwnd;

			::SendMessageTimeout(reinterpret_cast<HWND>(hwnd),
				WM_COPYDATA,
				reinterpret_cast<WPARAM>(hwnd),
				reinterpret_cast<LPARAM>(&c),
				SMTO_ABORTIFHUNG,1000,&result);
		}
	}
	else {
		DWORD result;
		
		COPYDATASTRUCT c;
		c.dwData = 9801;
		c.cbData = str.size();
		c.lpData = const_cast<char*>(str.c_str());

		::SendMessageTimeout(reinterpret_cast<HWND>(hwnd),
			WM_COPYDATA,
			reinterpret_cast<WPARAM>(hwnd),
			reinterpret_cast<LPARAM>(&c),
			SMTO_ABORTIFHUNG,1000,&result);
	}
}

bool CPLUGIN::load()
{
	return true; //NOOP
}

bool CPLUGIN::unload()
{
	return true; //NOOP
}

void CPLUGIN::exec(const CSAORIInput &in, CSAORIOutput &out)
{
	return; //NOOP
}

//CreateInstanceÇÕÇªÇÍÇºÇÍÇ™íËã`ÇµÇƒÇ≠ÇæÇ≥Ç¢ÅB
/*
#ifndef CUSTOM_CREATE_INSTANCE
CSAORIBase* CreateInstance(void)
{
	return new CPLUGIN();
}
#endif //CUSTOM_CREATE_INSTANCE
*/


