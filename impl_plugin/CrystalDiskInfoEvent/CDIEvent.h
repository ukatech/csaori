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
public:
	CSharedValue(void);
	~CSharedValue();

	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};

#endif //SHARED_VALUE_INCLUDED


