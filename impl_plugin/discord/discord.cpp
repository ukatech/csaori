//setlocale//
#ifdef _MSC_VER
#if _MSC_VER >= 1400
#pragma setlocale("japanese")
#endif
#endif
//setlocale end//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <new>
#include <iostream>
#include <fstream>
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
	初期化(DllMain縛り)
---------------------------------------------------------------*/
CDiscordPlugin::CDiscordPlugin(void)
{
}

CDiscordPlugin::~CDiscordPlugin()
{
}

/*---------------------------------------------------------------
	初期化(DllMainとは別)
---------------------------------------------------------------*/
void handleDiscordReady(const DiscordUser* request)
{
}

void handleDiscordDisconnected(int errorCode, const char* message)
{
}

void handleDiscordError(int errorCode, const char* message)
{
}

void handleDiscordJoinGame(const char* joinSecret)
{
}

void handleDiscordSpectateGame(const char* spectateSecret)
{
}

void handleDiscordJoinRequest(const DiscordUser* request)
{
}

bool CDiscordPlugin::load()
{
	std::string configPath = checkAndModifyPath("flags.txt");

	std::ifstream fin;
	fin.open(configPath);

	if (fin.is_open())
	{
		std::string line;

		while (std::getline(fin,line))
		{
			std::string flag, name;

			std::stringstream line_sr(line);

			if (line[line.size() - 1] == '\n') line.erase(line.size() - 1);
			if (line[line.size() - 1] == '\r') line.erase(line.size() - 1);

			if (std::getline(line_sr, flag, ',')) {
				if (std::getline(line_sr, name, ',')) {
					flag_array.push_back(CDiscordPluginGhostFlag(SAORI_FUNC::MultiByteToUnicode(name, CP_UTF8), std::stoul(flag)));
				}
			}
		}
	}

	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = handleDiscordReady;
	handlers.errored = handleDiscordError;
	handlers.disconnected = handleDiscordDisconnected;
	handlers.joinGame = handleDiscordJoinGame;
	handlers.spectateGame = handleDiscordSpectateGame;
	handlers.joinRequest = handleDiscordJoinRequest;

	// Discord_Initialize(const char* applicationId, DiscordEventHandlers* handlers, int autoRegister, const char* optionalSteamId)
    Discord_Initialize("514051485982785536", &handlers, 0, NULL);
	
	return true;
}

bool CDiscordPlugin::unload(void)
{
	Discord_Shutdown();

	std::string configPath = checkAndModifyPath("flags.txt");

	std::ofstream fout;
	fout.open(configPath, std::ofstream::out | std::ofstream::trunc);
	fout.seekp(0);

	for (auto itl = flag_array.begin(); itl != flag_array.end(); ++itl)
	{
		fout << itl->flags << "," << SAORI_FUNC::UnicodeToMultiByte(itl->name, CP_UTF8) << std::endl;
	}

	fout.close();

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

void CDiscordPlugin::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	out.result_code = SAORIRESULT_NO_CONTENT;

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnMenuExec") == 0 || wcsicmp(in.id.c_str(), L"OnDiscordPluginChoiceSelect") == 0) {

		if (wcsicmp(in.id.c_str(), L"OnDiscordPluginChoiceSelect") == 0) {
			if (in.args[0] == L"__CLOSE__")
			{
return;
			}

			auto itlList = std::find_if(ghost_map.begin(), ghost_map.end(), [&](auto &x) { return x.second.name == in.args[0]; });

			if (itlList != ghost_map.end())
			{
				auto itlFlag = std::find_if(flag_array.begin(), flag_array.end(), [&](auto &x) { return x.name == in.args[0]; });

				if (itlFlag == flag_array.end()) //見つからない
				{
					flag_array.push_back(CDiscordPluginGhostFlag(in.args[0], CDP_FLAG_DEV));
				}
				else
				{
					if ((itlFlag->flags & CDP_FLAG_DEV) == 0)
					{
						itlFlag->flags |= CDP_FLAG_DEV;
					}
					else
					{
						itlFlag->flags &= ~CDP_FLAG_DEV;
					}
				}

				Update(itlList->second.name);
			}
		}

		string_t menu_script;
		menu_script = L"\\t\\_q\\0開発中のゴーストにチェックを入れてください\\nPlease check ghosts in development.\\n\\n[half]";

		for (ghost_map_type::iterator it = ghost_map.begin(); it != ghost_map.end(); ++it)
		{
			auto itlFlag = std::find_if(flag_array.begin(), flag_array.end(), [&](auto &x) { return x.name == it->second.name; });

			menu_script += L"\\![*]";
			if (itlFlag != flag_array.end() && (itlFlag->flags & CDP_FLAG_DEV) != 0)
			{
				menu_script += L"\x2611";
			}
			else
			{
				menu_script += L"□";
			}

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
	else if (wcsicmp(in.id.c_str(), L"OnGhostBoot") == 0 || wcsicmp(in.id.c_str(), L"OnGhostInfoUpdate") == 0 || wcsicmp(in.id.c_str(), L"OnGhostExit") == 0) {
	CGhostInfo *pGI = NULL;
	if (ghost_map.empty())
	{
		return;
	}

	if (wcsicmp(in.id.c_str(), L"OnGhostExit") == 0)
	{
		pGI = &(ghost_map.begin()->second);
	}
	else
	{
		string_t path = in.args[4];
		pGI = &(ghost_map[path]);
	}

	Update(pGI->name);

	}


}

static const wchar_t* imageKeyTable[] =
{
	L"Emily/Phase4.5\1ghost_emily",
	L"とらふぃっく☆とれいん\1ghost_traffic",
};

void CDiscordPlugin::Update(string_t &ghostName)
{
	auto itlFlag = std::find_if(flag_array.begin(), flag_array.end(), [&](auto &x) { return x.name == ghostName; });

	string_t findKey = ghostName + L"\1";

	const wchar_t* foundPtr = NULL;

	for (int i = 0; i < _countof(imageKeyTable); i++)
	{
		if ( wcsstr(imageKeyTable[i], findKey.c_str()) != NULL ) {
			foundPtr = imageKeyTable[i] + findKey.length();
			break;
		}
	}

	std::string name = SAORI_FUNC::UnicodeToMultiByte(ghostName, CP_UTF8);

	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	time_t currentTime;
	discordPresence.startTimestamp = time(&currentTime);

	std::string foundIconName;
	if (foundPtr)
	{
		foundIconName = SAORI_FUNC::UnicodeToMultiByte(foundPtr, CP_UTF8);
		discordPresence.largeImageKey = foundIconName.c_str();
	}
	else
	{
		discordPresence.largeImageKey = "ssp_default";
	}

	discordPresence.details = name.c_str();
	discordPresence.largeImageText = name.c_str();

	if (itlFlag != flag_array.end() && (itlFlag->flags & CDP_FLAG_DEV) != 0)
	{
		discordPresence.smallImageKey = "development";
		discordPresence.smallImageText = "Developer";
	}

	Discord_UpdatePresence(&discordPresence);
}
