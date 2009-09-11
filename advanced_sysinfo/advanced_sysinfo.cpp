#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include "csaori.h"
#include "DriveInfo.h"

//////////DEBUG/////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
////////////////////////////////////////

/*=========================================================
	SAORI CORE
=========================================================*/

/*---------------------------------------------------------
	初期化
---------------------------------------------------------*/
CDriveInfo *g_pDriveInfo = NULL;

bool CSAORI::load(){
	return true;
}

/*---------------------------------------------------------
	解放
---------------------------------------------------------*/
bool CSAORI::unload(){
	if ( g_pDriveInfo ) {
		delete g_pDriveInfo;
		g_pDriveInfo = NULL;
	}
	return true;
}

/*---------------------------------------------------------
	実行
---------------------------------------------------------*/
static bool GetSpecialFolderPath(const std::wstring &nFolder, std::wstring &path );
static bool GetDriveInfo(int driveID,std::wstring &result,std::vector<std::wstring> &values);
static bool GetDriveNames(std::wstring &result,std::vector<std::wstring> &values);

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out){
	out.result_code = SAORIRESULT_BAD_REQUEST;
	if ( in.args.size() < 1 ) { return; }
	if ( in.args[0].size() == 0 ) { return; }

	if ( wcsicmp(in.args[0].c_str(),L"get_special_folder_path") == 0 ) {
		std::wstring path;

		if ( GetSpecialFolderPath(in.args[1],path) ) {
			out.result = path;
			out.result_code = SAORIRESULT_OK;
		}
		else {
			out.result_code = SAORIRESULT_NO_CONTENT;
		}
	}
	else if ( wcsicmp(in.args[0].c_str(),L"get_drive_info") == 0 ) {
		if ( in.args.size() < 2 ) {
			if ( GetDriveNames(out.result,out.values) ) {
				out.result_code = SAORIRESULT_OK;
			}
			else {
				out.result_code = SAORIRESULT_NO_CONTENT;
			}
			return;
		}

		int id = _wtoi(in.args[1].c_str());
		if ( GetDriveInfo(id,out.result,out.values) ) {
			out.result_code = SAORIRESULT_OK;
		}
		else {
			out.result_code = SAORIRESULT_NO_CONTENT;
		}
	}
}

/*=========================================================
	処理関数群
=========================================================*/

/*---------------------------------------------------------
	get_special_folder_path
---------------------------------------------------------*/

static bool GetSpecialFolderPath(const std::wstring &nFolder, std::wstring &path )
{
	IMalloc    *pMalloc;
	ITEMIDLIST *pidl;
	
	char path_buffer[MAX_PATH*2+1];

	static const struct { int id; wchar_t *nFolder; } csidl_list[] = {
		{CSIDL_APPDATA,L"ApplicationData"},
		{CSIDL_COMMON_APPDATA,L"CommonApplicationData"},
		{CSIDL_LOCAL_APPDATA,L"LocalApplicationData"},
		{CSIDL_COOKIES,L"Cookies"},
		{CSIDL_DESKTOPDIRECTORY,L"Desktop"}, //CSIDL_DESKTOPではない！
		{CSIDL_FAVORITES,L"Favorites"},
		{CSIDL_HISTORY,L"History"},
		{CSIDL_INTERNET_CACHE,L"InternetCache"},
		{CSIDL_PROGRAMS,L"Programs"},
		//{,L"MyComputer"},
		{CSIDL_MYMUSIC,L"MyMusic"},
		{CSIDL_MYPICTURES,L"MyPictures"},
		{CSIDL_RECENT,L"Recent"},
		{CSIDL_SENDTO,L"SendTo"},
		{CSIDL_STARTMENU,L"StartMenu"},
		{CSIDL_STARTUP,L"Startup"},
		{CSIDL_SYSTEM,L"System"},
		{CSIDL_WINDOWS,L"Windows"},
		{CSIDL_TEMPLATES,L"Templates"},
		{CSIDL_DESKTOPDIRECTORY,L"DesktopDirectory"},
		{CSIDL_PERSONAL,L"Personal"},
		{CSIDL_PERSONAL,L"MyDocuments"},
		{CSIDL_PROGRAM_FILES,L"ProgramFiles"},
		{CSIDL_PROGRAM_FILES_COMMON,L"CommonProgramFiles"},
	};

	int id = 0;

	for ( unsigned int i = 0 ; i < (sizeof(csidl_list)/sizeof(csidl_list[0])) ; ++i ) {
		if ( wcsicmp(nFolder.c_str(),csidl_list[i].nFolder) == 0 ) {
			id = csidl_list[i].id;
			break;
		}
	}

	if ( id == 0 ) {
		return false;
	}
	
	//先にいろいろフィルタ
	if ( id == CSIDL_WINDOWS ) {
		::GetWindowsDirectory(path_buffer,sizeof(path_buffer)-1);
		path = SAORI_FUNC::MultiByteToUnicode(path_buffer);
		return true;
	}
	if ( id == CSIDL_SYSTEM ) {
		::GetSystemDirectory(path_buffer,sizeof(path_buffer)-1);
		path = SAORI_FUNC::MultiByteToUnicode(path_buffer);
		return true;
	}

	//メイン
	if ( NOERROR == ::SHGetMalloc( &pMalloc ) ) {
		if ( SUCCEEDED(::SHGetSpecialFolderLocation(NULL, id, &pidl )) ) {
			if ( ::SHGetPathFromIDList( pidl, path_buffer ) ) {
				path = SAORI_FUNC::MultiByteToUnicode(path_buffer);
				pMalloc->Free( pidl );
				pMalloc->Release();
				return true;
			}
			pMalloc->Free( pidl );
		}
		pMalloc->Release();
	}

	//もひとつ
	{
		typedef HRESULT (WINAPI *P_SHGetFolderPathA)(HWND hwndOwner,int nFolder,HANDLE hToken,DWORD dwFlags,LPTSTR pszPath);
		
		HMODULE hmFolder = ::LoadLibrary("shfolder");
		if ( hmFolder ) {
			P_SHGetFolderPathA getFldrPath = (P_SHGetFolderPathA)::GetProcAddress(hmFolder,"SHGetFolderPathA");

			if ( getFldrPath ) {
				if ( SUCCEEDED(getFldrPath(NULL,id,NULL,0,path_buffer)) ) { //0=SHGFP_TYPE_CURRENT
					path = SAORI_FUNC::MultiByteToUnicode(path_buffer);
					return true;
				}
			}
			::FreeLibrary(hmFolder);
		}
	}

	return false;
}

/*---------------------------------------------------------
	get_drive_smart_info
---------------------------------------------------------*/
static std::wstring DriveStringToWString(const char *pBuffer,size_t n)
{
	size_t i;
	for ( i = 0 ; i < n ; ++i ) {
		if ( pBuffer[i] != L' ' ) { break; }
	}
	if ( i == n ) { return L""; }

	wchar_t buffer[128];
	size_t j = 0;
	for ( ; i < n ; ++i,++j ) {
		buffer[j] = pBuffer[i];
	}
	buffer[j] = 0; //ゼロ終端

	while ( j > 0 ) {
		if ( buffer[j-1] != L' ' ) { break; }
		buffer[j-1] = 0;
		--j;
	}
	return buffer;
}

static bool GetDriveInfo(int driveID,std::wstring &result,std::vector<std::wstring> &values)
{
	if ( ! g_pDriveInfo ) {
		g_pDriveInfo = new CDriveInfo;
	}

	static const struct { unsigned int id; wchar_t *text; wchar_t *textj; } smart_description[] = {
		{0x01,L"Raw Read Error Rate",L"読み取りエラー状況"},
		{0x02,L"Throughput Performance",L"全体性能"},
		{0x03,L"Spin Up Time",L"回転開始時間"},
		{0x04,L"Start/Stop Count",L"起動回数"},
		{0x05,L"Reallocated Sectors Count",L"代替処理セクタ数"},
		{0x06,L"Read Channel Margin",L"リードチャネルマージン"},
		{0x07,L"Seek Error Rate",L"シークエラー状況"},
		{0x08,L"Seek Time Performance",L"シークタイム性能"},
		{0x09,L"Power-On Hours",L"電源投入時間"},
		{0x0A,L"Spin Retry Count",L"再回転試行回数"},
		{0x0B,L"Recalibration Retries",L"再調整再試行回数"},
		{0x0C,L"Device Power Cycle Count",L"電源投入回数"},
		{0x0D,L"Soft Read Error Rate",L"オフトラック状況"},
		{0xA0,L"Free-fall Sensor Self Test Result",L"自由落下検知センサ状態"}, //HGST Travelstar
		{0xBE,L"Airflow Temperature",L"エアフロー温度"}, //Seagate
		{0xBF,L"G-Sense Error Rate",L"衝撃感知エラー状況"}, //HGST Travelstar
		{0xC0,L"Power-Off Retract Count",L"電源切断緊急退避回数"}, //HGST Travelstar
		{0xC1,L"Load/Unload Cycle Count",L"ヘッド退避回数"}, //HGST Travelstar
		{0xC2,L"Device Temperature",L"温度"}, //HGST Travelstar
		{0xC3,L"Hardware ECC recovered",L"ECC修正回数"},
		{0xC4,L"Reallocation Event Count",L"代替処理回数"}, //HGST Travelstar
		{0xC5,L"Current Pending Sector Count",L"代替処理待ちセクタ数"}, //HGST Travelstar
		{0xC6,L"Off-Line Scan Uncorrectable Sector Count",L"オフラインスキャン発見修正不可セクタ数"}, //HGST Travelstar
		{0xC7,L"UltraDMA CRC Error Count",L"UltraDMA CRCエラー回数"}, //HGST Travelstar
		{0xC8,L"Write Error Rate",L"書き込みエラー数"},
		{0xC9,L"Soft Read Error Rate",L"ソフト読み取りエラー状況"},
		{0xCA,L"Data Address Mark Error",L"DAMエラー状況"},
		{0xCB,L"Run Out Cancel",L"ECCエラー状況"},
		{0xCC,L"Soft ECC Correction",L"ECC修正状況"},
		{0xCD,L"Thermal Asperity Rate",L"サーマル・アスペリティ状況"},
		{0xCE,L"Flying Height",L"ヘッド飛行高"},
		{0xCF,L"Spin High Current",L"回転開始時超過電流状況"},
		{0xD0,L"Spin Buzz",L"ヘッド跳ね上げ状況"},
		{0xD1,L"Offline Seek Performance",L"オフラインスキャンシーク性能"},
		{0xD2,L"Vibration During Write",L"書き込み時の振動状況"},
		{0xD3,L"Vibration During Read",L"読み取り時の振動状況"},
		{0xD4,L"Shock During Write",L"書き込み時のショック状況"},
		{0xDC,L"Drive Shift",L"プラッタずれ回数"}, //HGST MicroDrive
		{0xDD,L"G-Sense Error Rate",L"衝撃感知エラー状況"},
		{0xDE,L"Loaded Hours",L"ヘッド負荷状況"},
		{0xDF,L"Load/Unload Retry Count",L"ロード/アンロード再試行回数"}, //HGST Travelstar
		{0xE0,L"Load Friction",L"ヘッド摩擦負荷状況"},
		{0xE2,L"Load-in Time",L"ヘッド負荷状況"},
		{0xE3,L"Torque Amplification Count",L"回転負荷増大回数"},
		{0xE4,L"Power-Off Retract Count",L"電源切断緊急退避回数"},
		{0xE5,L"Load/Unload Cycle Count",L"ヘッド退避回数"}, //?
		{0xE6,L"GMR Head Amplitude",L"ヘッド振幅"},
		{0xF0,L"Head Flying Hours",L"ヘッド飛行時間"},
		{0xFA,L"Read Error Retry Rate",L"読み取り再試行状況"},
		{0xFE,L"Free-fall Sensor Work Count",L"自由落下検知センサ作動回数"}, //HGST Travelstar
	};


	if ( g_pDriveInfo->Init() ) {
		CDriveSmartInfo *pDriveInfo = g_pDriveInfo->GetInfo(driveID);
		if ( pDriveInfo ) {
			int status = 0;
			std::wstring tmpstr;
			wchar_t buffer[512];

			tmpstr = L"ModelNumber\1";
			tmpstr += DriveStringToWString(pDriveInfo->m_sector.sModelNumber,sizeof(pDriveInfo->m_sector.sModelNumber));
			values.push_back(tmpstr);
		
			tmpstr = L"SerialNumber\1";
			tmpstr += DriveStringToWString(pDriveInfo->m_sector.sSerialNumber,sizeof(pDriveInfo->m_sector.sSerialNumber));
			values.push_back(tmpstr);
		
			tmpstr = L"FirmwareRev\1";
			tmpstr += DriveStringToWString(pDriveInfo->m_sector.sFirmwareRev,sizeof(pDriveInfo->m_sector.sFirmwareRev));
			values.push_back(tmpstr);

			tmpstr = L"SectorCount\1";
			swprintf(buffer,L"%u",pDriveInfo->m_sector.ulTotalAddressableSectors);
			tmpstr += buffer;
			values.push_back(tmpstr);

			size_t n = pDriveInfo->m_smartParams.size();
			unsigned int raw;
			unsigned int attr;
			wchar_t *desc;
			wchar_t *descj;
			std::wstring flags;

			for ( size_t i = 0 ; i < n ; ++i ) {
				raw = pDriveInfo->m_smartParams[i].attr.bRawValue[3];
				raw = raw << 8;
				raw |= pDriveInfo->m_smartParams[i].attr.bRawValue[2];
				raw = raw << 8;
				raw |= pDriveInfo->m_smartParams[i].attr.bRawValue[1];
				raw = raw << 8;
				raw |= pDriveInfo->m_smartParams[i].attr.bRawValue[0];

				attr = pDriveInfo->m_smartParams[i].attr.bAttrID;
				desc = L"";
				descj = L"";
				for ( size_t j = 0 ; j < (sizeof(smart_description)/sizeof(smart_description[0])) ; ++j ) {
					if ( attr == smart_description[j].id ) {
						desc = smart_description[j].text;
						descj = smart_description[j].textj;
						break;
					}
				}

				flags = L"";
				if ( pDriveInfo->m_smartParams[i].attr.wStatusFlags & PRE_FAILURE_WARRANTY ) {
					flags += L"PW|";
				}
				else if ( pDriveInfo->m_smartParams[i].attr.wStatusFlags & ON_LINE_COLLECTION ) {
					flags += L"OC|";
				}
				else if ( pDriveInfo->m_smartParams[i].attr.wStatusFlags & PERFORMANCE_ATTRIBUTE ) {
					flags += L"PE|";
				}
				else if ( pDriveInfo->m_smartParams[i].attr.wStatusFlags & ERROR_RATE_ATTRIBUTE ) {
					flags += L"ER|";
				}
				else if ( pDriveInfo->m_smartParams[i].attr.wStatusFlags & EVENT_COUNT_ATTRIBUTE ) {
					flags += L"EC|";
				}
				else if ( pDriveInfo->m_smartParams[i].attr.wStatusFlags & SELF_PRESERVING_ATTRIBUTE ) {
					flags += L"SP|";
				}
				if ( flags.size() ) {
					flags.erase(flags.end()-1,flags.end());
				}

				swprintf(buffer,L"SMARTValue\1%u,%s,%s,%u,%u,%u,%u,%s",
					attr,desc,descj,
					pDriveInfo->m_smartParams[i].attr.bAttrValue,
					pDriveInfo->m_smartParams[i].attr.bWorstValue,
					pDriveInfo->m_smartParams[i].thresh.bWarrantyThreshold,
					raw,flags.c_str());
				values.push_back(buffer);

				if ( pDriveInfo->m_smartParams[i].attr.bAttrValue < pDriveInfo->m_smartParams[i].thresh.bWarrantyThreshold ) {
					if ( attr < 14 || attr >= 191 ) {
						if ( status <= 1 ) {
							status = 2;
						}
					}
				}
				else if ( pDriveInfo->m_smartParams[i].attr.bWorstValue < pDriveInfo->m_smartParams[i].thresh.bWarrantyThreshold ) {
					if ( status <= 0) {
						status = 1;
					}
				}
				else if ( attr == 5 || attr == 196 || attr == 197 || attr == 198 ) {
					if ( raw ) {
						if ( status <= 0) {
							status = 1;
						}
					}
				}
			}
			static const wchar_t* status_table[] = {L"Normal",L"Warning",L"Error"};
			result = status_table[status];
			return true;
		}
	}
	
	return false;
}

static bool GetDriveNames(std::wstring &result,std::vector<std::wstring> &values)
{
	if ( ! g_pDriveInfo ) {
		g_pDriveInfo = new CDriveInfo;
	}

	if ( g_pDriveInfo->Init() ) {
		int pDrives = g_pDriveInfo->GetDrives();
		result = SAORI_FUNC::intToString(pDrives);
		CDriveSmartInfo *pDriveInfo;
		if ( pDrives ) {
			for ( int i = 0 ; i < pDrives ; ++i ) {
				pDriveInfo = g_pDriveInfo->GetInfo(i);
				if ( pDriveInfo ) {
					values.push_back(DriveStringToWString(pDriveInfo->m_sector.sModelNumber,sizeof(pDriveInfo->m_sector.sModelNumber)));
				}
			}
		}
		return true;
	}
	
	return false;
}
