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
CSharedValue::CSharedValue(void) : last_tick(0)
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
	out.result_code = SAORIRESULT_NO_CONTENT;

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnGhostBoot") == 0 ) {
		if ( last_values.size() > 0 ) {
			event = L"OnCrystalDiskInfoEvent";
			out.result_code = SAORIRESULT_OK;
			out.values = last_values;
		}

		return;
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSecondChange") == 0 ) {
		HKEY hKey = NULL;
		LONG result = ::RegOpenKeyEx(HKEY_CURRENT_USER,
			"Software\\Crystal Dew World\\CrystalDiskInfo",0,KEY_READ,&hKey);
		if ( result != ERROR_SUCCESS ) {
			if ( last_tick || (last_values.size() > 0) ) {
				out.result_code = SAORIRESULT_OK;
				event = L"OnCrystalDiskInfoClear";

				last_tick = 0;
				last_values.clear();
			}
			return;
		}

		//last update
		DWORD type,dwData,size;

		size = 4;
		result = ::RegQueryValueEx(hKey,"LastUpdate",0,&type,(BYTE*)&dwData,&size);
		DWORD update_tick = dwData;
		time_t lastUpdate = time(NULL) - ((::GetTickCount() - update_tick) / 1000);

		//更新されてない
		if ( (last_values.size() > 0) && (update_tick == last_tick) ) {
			::RegCloseKey(hKey);
			return;
		}

		//diskcountを取って名前リスト構築
		std::vector<std::string> names;

		char buffer[512];
		char name[64];

		size = 4;
		result = ::RegQueryValueEx(hKey,"DiskCount",0,&type,(BYTE*)&dwData,&size);
		DWORD diskCount = dwData;

		if ( result == ERROR_SUCCESS ) {
			for ( unsigned int i = 0 ; i < diskCount ; ++i ) {
				sprintf(name,"Disk%u",i);
				size = sizeof(buffer)-1;
				result = ::RegQueryValueEx(hKey,name,0,&type,(BYTE*)buffer,&size);
				if ( result == ERROR_SUCCESS ) {
					names.push_back(std::string(name));
				}
			}
		}

		::RegCloseKey(hKey);

		//ぜんぶとれてないっぽ？→ボッシュートになります！
		if ( diskCount != names.size() ) {
			return;
		}

		//values(Reference)構築
		std::string sz,sz_line;

		DWORD status = 0;

		//更新日付
		struct tm *tm_update = localtime(&lastUpdate);
		sprintf(buffer,"%d,%d,%d,%d,%d,%d",tm_update->tm_year+1900,tm_update->tm_mon+1,tm_update->tm_mday,tm_update->tm_hour,tm_update->tm_min,tm_update->tm_sec);
		out.values.push_back(SAORI_FUNC::MultiByteToUnicode(buffer));

		unsigned int n = names.size();
		for ( unsigned int i = 0 ; i < n ; ++i ) {
			//情報キーをひとつずつまわる
			sz = "Software\\Crystal Dew World\\CrystalDiskInfo\\";
			sz += names[i];

			result = ::RegOpenKeyEx(HKEY_CURRENT_USER,sz.c_str(),0,KEY_READ,&hKey);
			if ( result != ERROR_SUCCESS ) {
				continue;
			}

			sz_line = "";

			sz_line += names[i];
			sz_line += ",";

			size = sizeof(buffer)-1;
			result = ::RegQueryValueEx(hKey,"DriveLetter",0,&type,(BYTE*)buffer,&size);
			sz_line += buffer;
			sz_line += ",";

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
			sz_line += ",";
			if ( dwData > status ) {
				status = dwData;
			}

			size = sizeof(buffer)-1;
			result = ::RegQueryValueEx(hKey,"DiskSize",0,&type,(BYTE*)buffer,&size);
			sz_line += buffer;
			sz_line += ",";

			size = sizeof(buffer)-1;
			result = ::RegQueryValueEx(hKey,"Temperature",0,&type,(BYTE*)buffer,&size);
			sz_line += buffer;
			sz_line += ",";


			size = sizeof(buffer)-1;
			result = ::RegQueryValueEx(hKey,"TemperatureClass",0,&type,(BYTE*)buffer,&size);
			sz_line += buffer;
			//sz_line += ",";

			out.values.push_back(SAORI_FUNC::MultiByteToUnicode(sz_line));

			::RegCloseKey(hKey);
		}

		//Reference0にステータスを足す
		if ( status == 1 ) {
			out.values[0] = L"Good," + out.values[0];
		}
		else if ( status == 2 ) {
			out.values[0] = L"Caution," + out.values[0];
		}
		else if ( status == 3 ) {
			out.values[0] = L"Bad," + out.values[0];
		}
		else {
			out.values[0] = L"Unknown," + out.values[0];
		}

		last_tick = update_tick;
		last_values = out.values;
		
		event = L"OnCrystalDiskInfoEvent";
		out.result_code = SAORIRESULT_OK;
		return;
	}
}
