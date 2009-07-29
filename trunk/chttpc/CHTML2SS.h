#ifndef _CHTML2SS_H
#define _CHTML2SS_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <vector>
#include <map>

// htmlcxx
#include "html/ParserDom.h"
#include "html/utils.h"

// striphtml
#include "striphtml.h"

#define CHSS_HEAD 1
#define CHSS_TAIL 2

using namespace htmlcxx;
using namespace std;

class Curl {
public:
	string scheme;
	string domain;
	string fullpath;
	string path;
	string filename;
	string querystring;
	string hash;
	Curl(string& url) {
		int colonIdx, slashIdx, QSIdx, hashIdx, lastSlashIdx;
		colonIdx = url.find(":");
		scheme = url.substr(0,colonIdx);
		slashIdx = url.find("/",colonIdx + 3/* "://" */);
		domain = url.substr(colonIdx + 3, slashIdx - colonIdx - 3);
		QSIdx = url.find("?", slashIdx);
		hashIdx = url.find("#", slashIdx);
		hash = hashIdx == string::npos ? "" : url.substr(hashIdx);
		querystring = QSIdx == string::npos ? "" : url.substr(QSIdx, hashIdx - QSIdx);
		fullpath = url.substr(slashIdx, QSIdx != string::npos ? QSIdx - slashIdx: hashIdx - slashIdx);
		lastSlashIdx = fullpath.find_last_of("/");
		path = fullpath.substr(0,lastSlashIdx+1);
		filename = fullpath.substr(lastSlashIdx+1);
	}

	string toString() {
		return scheme + "://" + domain + path + filename + querystring + hash;
	}
};

class CHTML2SS {
public:
	static wstring translate(wstring& in, string& url);
private:
	static string translateSubTree(tree<HTML::Node>& top, tree<HTML::Node>::iterator it, Curl* cu, unsigned int& liCount);
	static string translateSingleTag(tree<HTML::Node>& top, tree<HTML::Node>::iterator it, Curl* cu, unsigned int& liCount, int translateType);
	static string _itoa(unsigned int num);
};

#endif