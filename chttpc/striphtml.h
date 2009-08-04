#ifndef _STRIPHTML_H
#define _STRIPHTML_H

// modified from http://evotalk.net/blog/2007/02/strip-html-tag.html
#include <string>

using namespace std;

wstring replaceAll(wstring s, wstring f, wstring r);

wstring overReplaceAll(wstring s, wstring f, wstring r);

wstring stripHTMLTags(wstring s);

string replaceAll(string s, string f, string r);

string overReplaceAll(string s, string f, string r);

string stripHTMLTags(string s);

#endif