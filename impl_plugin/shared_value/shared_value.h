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
	std::vector<CSharedValueGhost*> m_ghost_values;
	time_t m_last_time;

public:
	CSharedValue(void);
	~CSharedValue();

	CSharedValueGhost *FindGhost(const string_t &name);

	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();

	void Save(void);
};

#endif //SHARED_VALUE_INCLUDED


