#include "cplugin.h"
#include <deque>

#include <time.h>

#pragma once

#ifndef SHARED_VALUE_INCLUDED
#define SHARED_VALUE_INCLUDED

class CSharedValueGhost;
class CSharedValueElement;

class CSharedValue : public CPLUGIN
{
private:
	std::vector<string_t> last_values;
	DWORD last_tick;

public:
	CSharedValue(void);
	~CSharedValue();

	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};

#endif //SHARED_VALUE_INCLUDED


