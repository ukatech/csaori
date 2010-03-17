/*
 * csaori.h
 * 
 * written by Ukiya http://ukiya.sakura.ne.jp/
 * based by えびさわ様 "gethwnd.dll"
 */

#pragma once

#include "csaori_base.h"

class CPLUGIN : public CSAORIBase {
private:
	string_t script;
	string_t script_option;
	string_t event;
	string_t event_option;
	string_t target;
	string_t marker;
	string_t security_level;

public:
	CPLUGIN(){}
	virtual ~CPLUGIN(){}

	//インターフェース固有文字列群
	virtual const string_t& s_saori_version(void) const;
	virtual const string_t& s_saori_def(void) const;
	virtual const string_t& s_saori_argument(void) const;
	virtual const string_t& s_saori_value(void) const;
	virtual const string_t& s_saori_result(void) const;

	//Prologue / Epilogue
	virtual void exec_before(const CSAORIInput& in,CSAORIOutput& out);
	virtual void exec_after(const CSAORIInput& in,CSAORIOutput& out);

	//以下が実装すべき関数
	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};
