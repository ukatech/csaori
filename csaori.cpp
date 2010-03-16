/*
 * csaori.cpp
 * 
 * written by Ukiya http://ukiya.sakura.ne.jp/
 * based by Ç¶Ç—Ç≥ÇÌól "gethwnd.dll"
 */

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#pragma warning( disable : 4996 )
#endif

#include "csaori.h"

//////////WINDOWS DEFINE///////////////////////////
//includeÇÃÇ†Ç∆Ç…Ç®Ç¢ÇƒÇÀÅI
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

#define SAORI_DEF   L"SAORI/"
#define SAORI_DEF_A "SAORI/"

#define SAORI_VERSIONSTRING_10		SAORI_DEF L"1.0"
#define SAORI_VERSIONSTRING_10_A	SAORI_DEF_A "1.0"

#define SAORI_VERSIONSTRING			SAORI_VERSIONSTRING_10
#define SAORI_VERSIONSTRING_A		SAORI_VERSIONSTRING_10_A

#define SAORI_ARGUMENT L"Argument"
#define SAORI_VALUE    L"Value"

#ifndef CSAORI_IMPL
#define CSAORI_IMPL
#include "csaori_base.cpp"
#endif //CSAORI_IMPL


CSAORIBase* CreateInstance(void)
{
	return new CSAORI();
}

