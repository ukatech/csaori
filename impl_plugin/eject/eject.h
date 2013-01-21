#include "cplugin.h"
#include <deque>

#include <time.h>

#pragma once

#ifndef SHARED_VALUE_INCLUDED
#define SHARED_VALUE_INCLUDED

class CEjectPluginGhost;
class CEjectPluginElement;

class CEjectPlugin : public CPLUGIN
{
private:
	std::vector<string_t> last_values;
	DWORD last_tick;

public:
	CEjectPlugin(void);
	~CEjectPlugin();

	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};

#endif //SHARED_VALUE_INCLUDED


