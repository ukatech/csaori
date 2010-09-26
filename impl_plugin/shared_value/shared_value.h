#include "cplugin.h"
#include <deque>

#pragma once

#ifndef SHARED_VALUE_INCLUDED
#define SHARED_VALUE_INCLUDED

class CSharedValue : public CPLUGIN
{
private:

public:
	CSharedValue(void);
	~CSharedValue();

	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};

#endif //SHARED_VALUE_INCLUDED


