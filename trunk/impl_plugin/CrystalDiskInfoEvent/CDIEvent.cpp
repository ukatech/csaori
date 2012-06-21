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
#include <fstream>

#include "CDIEvent.h"

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

/*===============================================================
	制御クラス実装
===============================================================*/

/*---------------------------------------------------------------
	インスタンス作成（csaori_baseから呼ばれる）
---------------------------------------------------------------*/
CSAORIBase* CreateInstance(void)
{
	return new CSharedValue();
}

/*---------------------------------------------------------------
	初期化(DllMain縛り)
---------------------------------------------------------------*/
CSharedValue::CSharedValue(void)
{
}

CSharedValue::~CSharedValue()
{
}

/*---------------------------------------------------------------
	初期化(DllMainとは別)
---------------------------------------------------------------*/
bool CSharedValue::load()
{
	return true;
}

bool CSharedValue::unload(void)
{
	return true;
}

void CSharedValue::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	out.result_code = SAORIRESULT_BAD_REQUEST;

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSecondChange") == 0 ) {
		out.result_code = SAORIRESULT_NO_CONTENT;

		HKEY hKey = NULL;
		LONG result = ::RegOpenKeyEx(HKEY_CURRENT_USER,
			"Software\\Crystal Dew World\\CrystalDiskInfo",0,KEY_READ,&hKey);
		if ( result != ERROR_SUCCESS ) {
			return;
		}

		std::vector<std::string> names;

		DWORD type,dwData,size;
		char buffer[512];
		char name[64];

		size = 4;
		result = ::RegQueryValueEx(hKey,"DiskCount",0,&type,(BYTE*)&dwData,&size);
		if ( result == ERROR_SUCCESS ) {
			for ( unsigned int i = 0 ; i < dwData ; ++i ) {
				sprintf(name,"Disk%u",i);
				size = sizeof(buffer)-1;
				result = ::RegQueryValueEx(hKey,name,0,&type,(BYTE*)buffer,&size);
				if ( result == ERROR_SUCCESS ) {
					names.push_back(std::string(name));
				}
			}
		}

		result = ::RegQueryValueEx(hKey,"LastUpdate",0,&type,(BYTE*)&dwData,&size);
		DWORD lastUpdateTick = dwData;

		::RegCloseKey(hKey);

		if ( dwData != names.size() ) {
			return;
		}

		std::string sz,sz_line;

		DWORD status = 0;

		unsigned int n = names.size();
		for ( unsigned int i = 0 ; i < n ; ++i ) {
			sz = "Software\\Crystal Dew World\\CrystalDiskInfo\\";
			sz += names[i];

			result = ::RegOpenKeyEx(HKEY_CURRENT_USER,sz.c_str(),0,KEY_READ,&hKey);
			if ( result != ERROR_SUCCESS ) {
				continue;
			}

			sz_line = "";

			sz_line += names[i];
			sz_line += "\1";

			size = sizeof(buffer)-1;
			result = ::RegQueryValueEx(hKey,"DriveLetter",0,&type,(BYTE*)buffer,&size);
			sz_line += buffer;
			sz_line += "\1";

			size = 4;
			result = ::RegQueryValueEx(hKey,"DiskStatus",0,&type,(BYTE*)&dwData,&size);
			if ( dwData == 1 ) {
				sz_line += "Good";
			}
			else if ( dwData == 2 ) {
				sz_line += "Caution";
			}
			else if ( dwData == 3 ) {
				sz_line += "Bad";
			}
			else {
				sz_line += "Unknown";
			}
			sz_line += "\1";
			if ( dwData > status ) {
				status = dwData;
			}

			size = sizeof(buffer)-1;
			result = ::RegQueryValueEx(hKey,"DiskSize",0,&type,(BYTE*)buffer,&size);
			sz_line += buffer;
			sz_line += "\1";

			size = sizeof(buffer)-1;
			result = ::RegQueryValueEx(hKey,"Temperature",0,&type,(BYTE*)buffer,&size);
			sz_line += buffer;
			sz_line += "\1";


			size = sizeof(buffer)-1;
			result = ::RegQueryValueEx(hKey,"TemperatureClass",0,&type,(BYTE*)buffer,&size);
			sz_line += buffer;
			//sz_line += "\1";

			out.values.push_back(SAORI_FUNC::MultiByteToUnicode(sz_line));

			::RegCloseKey(hKey);
		}
		if ( status == 1 ) {
			out.result = L"Good";
		}
		else if ( status == 2 ) {
			out.result = L"Caution";
		}
		else if ( status == 3 ) {
			out.result = L"Bad";
		}
		else {
			out.result = L"Unknown";
		}
		out.result_code = SAORIRESULT_OK;
	}
}
