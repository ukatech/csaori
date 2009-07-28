#ifndef _CHTML2SS_H
#define _CHTML2SS_H

#include <string>

using namespace std;

class CHTML2SS {
public:
	static wstring translate(wstring& in, string& url);
private:
	static string _itoa(unsigned int num);
};

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

#endif