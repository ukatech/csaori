#include "cplugin.h"
#include "discord_rpc.h"
#include <deque>

#include <time.h>

#pragma once

#ifndef SHARED_VALUE_INCLUDED
#define SHARED_VALUE_INCLUDED

/*===============================================================
	主プラグイン
===============================================================*/
class CDiscordPlugin : public CPLUGIN
{
private:
	bool NeedNotityGhost=0;
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

#endif //SHARED_VALUE_INCLUDED


