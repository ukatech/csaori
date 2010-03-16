#ifndef _MLANG2UNICODE_H
#define _MLANG2UNICODE_H

#include <ole2.h>
#include <MLang.h>

using namespace std;

bool mlangToUnicode(const wchar_t *charset, string &in, wstring &out);

#endif