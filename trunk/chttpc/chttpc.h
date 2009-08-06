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
		wstring module_path;
		wstring charset;
		wstring saveOrginal;
		wstring saveUtf8;
		wstring saveParsed;
		wstring searchStart;
		wstring searchEnd;
		wstring id;
		HWND hwnd;
		bool isStripTag;
		bool isTranslateTag;
		bool isNoOutput;
		map<string,bool> stripTags;
		chttpc_conf() {
			url = "";
			codepage = 0;
			module_path = L"";
			charset = L"";
			saveOrginal = L"";
			saveUtf8 = L"";
			saveParsed = L"";
			searchStart = L"";
			searchEnd = L"";
			id = L"";
			hwnd = 0;
			isStripTag = false;
			isTranslateTag = false;
			isNoOutput = false;
		}
};

#endif