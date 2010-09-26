//setlocale//
#ifdef _MSC_VER
#if _MSC_VER >= 1400
#pragma setlocale("japanese")
#endif
#endif
//setlocale end//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <new>

#include "shared_value.h"

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

/*===============================================================
	インスタンス作成（csaori_baseから呼ばれる）
===============================================================*/

CSAORIBase* CreateInstance(void)
{
	return new CSharedValue();
}

/*===============================================================
	初期化(DllMain縛り)
===============================================================*/

CSharedValue::CSharedValue(void)
{
}

CSharedValue::~CSharedValue()
{
}

/*===============================================================
	初期化(DllMainとは別)
===============================================================*/
bool CSharedValue::load()
{
	return true;
}

bool CSharedValue::unload()
{
	return true;
}

void CSharedValue::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSharedValueWrite") == 0 ) {
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSharedValueRead") == 0 ) {
	}

}

