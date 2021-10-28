#include "cplugin.h"
#include "discord_rpc.h"
#include <deque>

#include <time.h>

#pragma once


enum NotityGhostState {DON_T_NEED=0,NOTIFY_END=2,NOTIFY_BEGIN=1};
/*===============================================================
	主プラグイン
===============================================================*/
class CDiscordPlugin : public CPLUGIN
{
private:
	NotityGhostState NeedNotityGhost = DON_T_NEED;
	string_t GhostNameForNotityEnd;

	string_t GhostName;
	std::string Appid;
	std::string CustomState;
	std::string CustomDetail;
	std::string LargeImageKey;
	std::string LargeImageText;
	std::string SmallImageKey;
	std::string SmallImageText;

	DiscordRichPresence discordPresence{};


	void BaseUpdate();
	void ClearAll();
	void SetDefault(const string_t ghostName);

	void Discord_ReSetAPPid(const char* appid= "514051485982785536");
public:
	CDiscordPlugin(void);
	~CDiscordPlugin();

	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};


