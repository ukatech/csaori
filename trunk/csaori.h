/*
 * csaori.h
 * 
 * written by Ukiya http://ukiya.sakura.ne.jp/
 * based by ‚¦‚Ñ‚³‚í—l "gethwnd.dll"
 */

#pragma once

#include "csaori_base.h"

class CSAORI : public CSAORIBase {
public:
	CSAORI(){}
	virtual ~CSAORI(){}

	//ˆÈ‰º‚ªÀ‘•‚·‚×‚«ŠÖ”
	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};
