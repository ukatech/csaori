#ifndef _CHTTPC_H
#define _CHTTPC_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <vector>
#include <map>

#include <ole2.h>
#include <MLang.h>

using namespace std;

class chttpc_conf {
	public:
		unsigned int codepage;
		string url;
		wstring charset;
		wstring saveTo;
		wstring searchStart;
		wstring searchEnd;
		wstring id;
		HWND hwnd;
		bool isStripTag;
		bool isTranslateTag;
};

#endif