#include "cplugin.h"
#include <deque>

#include <time.h>

#pragma once

#ifndef SHARED_VALUE_INCLUDED
#define SHARED_VALUE_INCLUDED

/*===============================================================
	ゴーストフラグ管理用
===============================================================*/
#define CDP_FLAG_DEV 0x01U

class CDiscordPluginGhostFlag
{
public:
	string_t name;
	unsigned int flags;

	CDiscordPluginGhostFlag() : flags(0) { }
	CDiscordPluginGhostFlag(const string_t &n,unsigned int f) : name(n), flags(f) { }
};

/*===============================================================
	主プラグイン
===============================================================*/
class CDiscordPlugin : public CPLUGIN
{
private:
	std::vector<CDiscordPluginGhostFlag> flag_array;

	void Update(string_t &ghostName);

public:
	CDiscordPlugin(void);
	~CDiscordPlugin();

	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};

#endif //SHARED_VALUE_INCLUDED


