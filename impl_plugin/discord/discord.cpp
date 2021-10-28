#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <new>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <algorithm>

#include "discord.h"
#include "discord_rpc.h"

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#pragma warning( disable : 4996 )
#endif

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

/*===============================================================
	制御クラス実装
===============================================================*/

/*---------------------------------------------------------------
	インスタンス作成（csaori_baseから呼ばれる）
---------------------------------------------------------------*/
CSAORIBase* CreateInstance(void)
{
	return new CDiscordPlugin();
}

/*---------------------------------------------------------------
	初期化・開放(DllMain縛り)
---------------------------------------------------------------*/
CDiscordPlugin::CDiscordPlugin(void)
{
}

CDiscordPlugin::~CDiscordPlugin()
{
}

constexpr DiscordEventHandlers handlers{};
void CDiscordPlugin::Discord_ReSetAPPid(const char*appid) {
	Discord_Shutdown();
	Appid = appid;
	Discord_Initialize(appid, &handlers, 0, NULL);
}

/*---------------------------------------------------------------
	初期化(DllMainとは別)
---------------------------------------------------------------*/
bool CDiscordPlugin::load()
{
	return true;
}


/*---------------------------------------------------------------
	開放(DllMainとは別)
---------------------------------------------------------------*/
bool CDiscordPlugin::unload(void)
{
	Discord_Shutdown();
	return true;
}

static string_t replace_all(const string_t &ss,const string_t &target,const string_t &replacement)
{
	string_t s = ss;

	if (!target.empty()) {
		std::string::size_type pos = 0;
		while ((pos = s.find(target, pos)) != std::string::npos) {
			s.replace(pos, target.length(), replacement);
			pos += replacement.length();
		}
	}

	return s;
}

/*---------------------------------------------------------------
	イベント実行
---------------------------------------------------------------*/
void CDiscordPlugin::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	out.result_code = SAORIRESULT_NO_CONTENT;

	//-------------------------------------------------------------------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnMenuExec") == 0 || wcsicmp(in.id.c_str(), L"OnDiscordPluginChoiceSelect") == 0) {

		if (wcsicmp(in.id.c_str(), L"OnDiscordPluginChoiceSelect") == 0) {
			if (in.args[0] == L"__CLOSE__")
			{
				out.result = L"\\0\\b[-1]\\e";
				out.result_code = SAORIRESULT_OK;
				return;
			}

			auto itlList = std::find_if(ghost_map.begin(), ghost_map.end(), [&](auto &x) { return x.second.name == in.args[0]; });

			if (itlList != ghost_map.end())
			{
				if (GhostName == itlList->second.name){
					out.opts[L"Target"] = GhostName;
					out.opts[L"Event"] = L"OnDiscordPluginCustomEnd";
					out.opts[L"EventOption"] = L"notify";
					out.result_code = SAORIRESULT_OK;
					ClearAll();
					Discord_Shutdown();
				}
				else
					SetDefault(itlList->second.name);
			}
		}

		string_t menu_script;
		menu_script = L"\\t\\_q\\0\
						自分のDiscordプロファイルに表示したいゴーストをクリックしてください\\n\
						点击你想展示于Discord个人资料中的ghost\\n\
						Click on the ghost you want to display in your Discord profile\\n\
						\\n[half]\
						";

		for (ghost_map_type::iterator it = ghost_map.begin(); it != ghost_map.end(); ++it)
		{
			menu_script += L"\\![*]";
			if (GhostName == it->second.name)
				menu_script += L"☑";
			else
				menu_script += L"□";

			string_t q_name = replace_all(it->second.name, L"\"", L"\"\"");

			menu_script += L"\\q[\"";
			menu_script += q_name;
			menu_script += L"\",OnDiscordPluginChoiceSelect,\"";
			menu_script += q_name;
			menu_script += L"\"]\\n";
		}

		menu_script += L"\\n[half]\\![*]\\q[閉じる/Close,OnDiscordPluginChoiceSelect,__CLOSE__]";

		out.result = menu_script;
		out.result_code = SAORIRESULT_OK;

		return;
	}
	//-------------------------------------------------------------------------------------------------------------------
	else if (wcsicmp(in.id.c_str(), L"OnGhostBoot") == 0 || wcsicmp(in.id.c_str(), L"OnGhostExit") == 0)
	{
		CGhostInfo *pGI = NULL;
		if (ghost_map.empty())
			return;

		if (GhostName.empty() || (wcsicmp(in.id.c_str(), L"OnGhostExit") == 0 && GhostName == in.args[1]))
		{
			pGI = &(ghost_map.begin()->second);
			SetDefault(pGI->name);
		}

	}
	//-------------------------------------------------------------------------------------------------------------------
	else if (wcsicmp(in.id.c_str(), L"OnSecondChange") == 0)
	{
		Discord_RunCallbacks();
		if (NeedNotityGhost == DON_T_NEED);
		else if (NeedNotityGhost == NOTIFY_END) {
			if(GhostNameForNotityEnd.size()){
				out.opts[L"Target"] = GhostNameForNotityEnd;
				out.opts[L"Event"] = L"OnDiscordPluginCustomEnd";
				out.opts[L"EventOption"] = L"notify";
				out.result_code = SAORIRESULT_OK;
			}
			NeedNotityGhost = NOTIFY_BEGIN;
		}
		else if (NeedNotityGhost == NOTIFY_BEGIN) {
			if(GhostName.size()){
				out.opts[L"Target"] = GhostName;
				out.opts[L"Event"] = L"OnDiscordPluginCustom";
				out.opts[L"EventOption"] = L"notify";
				out.result_code = SAORIRESULT_OK;
			}
			NeedNotityGhost = DON_T_NEED;
		}
	}
	else if (wcsicmp(in.id.c_str(), L"OnDiscordPluginCustomAppid") == 0)
	{
		Discord_ReSetAPPid(SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8).c_str());
		BaseUpdate();
	}
	else if (wcsicmp(in.id.c_str(), L"OnDiscordPluginCustomState") == 0)
	{
		CustomState = SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8);
		BaseUpdate();
	}
	else if (wcsicmp(in.id.c_str(), L"OnDiscordPluginCustomDetail") == 0)
	{
		CustomDetail = SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8);
		BaseUpdate();
	}
	else if (wcsicmp(in.id.c_str(), L"OnDiscordPluginCustomLargeImageKey") == 0)
	{
		LargeImageKey = SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8);
		BaseUpdate();
	}
	else if (wcsicmp(in.id.c_str(), L"OnDiscordPluginCustomLargeImageText") == 0)
	{
		LargeImageText = SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8);
		BaseUpdate();
	}
	else if (wcsicmp(in.id.c_str(), L"OnDiscordPluginCustomSmallImageKey") == 0)
	{
		SmallImageKey = SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8);
		BaseUpdate();
	}
	else if (wcsicmp(in.id.c_str(), L"OnDiscordPluginCustomSmallImageText") == 0)
	{
		SmallImageText = SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8);
		BaseUpdate();
	}
	else if (wcsicmp(in.id.c_str(), L"OnDiscordPluginCustomALL") == 0)
	{
		auto args=in.args;
		args.resize(7);
		Discord_ReSetAPPid(SAORI_FUNC::UnicodeToMultiByte(args[0], CP_UTF8).c_str());
		CustomDetail = SAORI_FUNC::UnicodeToMultiByte(args[1], CP_UTF8);
		CustomState = SAORI_FUNC::UnicodeToMultiByte(args[2], CP_UTF8);
		LargeImageKey = SAORI_FUNC::UnicodeToMultiByte(args[3], CP_UTF8);
		LargeImageText = SAORI_FUNC::UnicodeToMultiByte(args[4], CP_UTF8);
		SmallImageKey = SAORI_FUNC::UnicodeToMultiByte(args[5], CP_UTF8);
		SmallImageText = SAORI_FUNC::UnicodeToMultiByte(args[6], CP_UTF8);
		BaseUpdate();
	}
}

/*---------------------------------------------------------------
	Discordの状態更新
---------------------------------------------------------------*/
void CDiscordPlugin::BaseUpdate()
{
	discordPresence.state = CustomState.c_str();
	discordPresence.details = CustomDetail.c_str();
	discordPresence.largeImageKey = LargeImageKey.c_str();
	discordPresence.largeImageText = LargeImageText.c_str();
	discordPresence.smallImageKey = SmallImageKey.c_str();
	discordPresence.smallImageText = SmallImageText.c_str();

	Discord_UpdatePresence(&discordPresence);
}
void CDiscordPlugin::ClearAll()
{
	discordPresence = {};
	GhostName.clear();
	CustomState.clear();
	CustomDetail.clear();
	LargeImageKey.clear();
	LargeImageText.clear();
	SmallImageKey.clear();
	SmallImageText.clear();

	Discord_UpdatePresence(&discordPresence);
}
void CDiscordPlugin::SetDefault(const string_t ghostName)
{
	static std::unordered_map<string_t,std::string> CompatibilityImageKeyTable =
	{
		{L"Emily/Phase4.5","ghost_emily"},
		{L"とらふぃっく☆とれいん","ghost_traffic"},
		{L"Us' Lovex2 Dev","ghost_uslovex2dev"},
		{L"雪葵V2","ghost_susugiv2"},
		{L"Francis (is not to work)","ghost_francis"},
		{L"ナナとくろねこ","ghost_nanakuro"},
	};
	GhostNameForNotityEnd = GhostName;
	NeedNotityGhost = NOTIFY_END;

	ClearAll();
	Discord_ReSetAPPid();

	GhostName = ghostName;

	std::string GhostName = SAORI_FUNC::UnicodeToMultiByte(ghostName, CP_UTF8);

	discordPresence.startTimestamp = time(NULL);

	LargeImageKey = CompatibilityImageKeyTable[ghostName];
	if(LargeImageKey.empty())
		LargeImageKey = "ssp_default";
	LargeImageText = GhostName;
	CustomDetail = GhostName;

	BaseUpdate();
}
