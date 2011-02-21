#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <time.h>

#include "csaori.h"
#include "qreki.h"

/*---------------------------------------------------------
	èâä˙âª
---------------------------------------------------------*/
bool CSAORI::load(){
	return true;
}

/*---------------------------------------------------------
	âï˙
---------------------------------------------------------*/
bool CSAORI::unload(){
	return true;
}

/*---------------------------------------------------------
	é¿çs
---------------------------------------------------------*/
void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out){
	out.result_code = SAORIRESULT_BAD_REQUEST;
	if ( in.args.size() < 1 ) {
		return;
	}

	string_t cmd = in.args[0];
	size_t n = cmd.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		cmd[i] = tolower(cmd[i]);
	}

	//===================================================
	if ( cmd == L"time" ) {
		out.result = SAORI_FUNC::numToString(time(NULL));
		out.result_code = SAORIRESULT_OK;
		return;
	}

	//===================================================
	if ( cmd == L"system" ) {
		out.result = SAORI_FUNC::numToString(::GetTickCount()/1000);
		out.result_code = SAORIRESULT_OK;
		return;
	}

	//===================================================
	if ( cmd == L"fromdate" ) {
		if ( in.args.size() < 2 ) {
			return;
		}

		struct tm td;
		ZeroMemory(&td,sizeof(td));

		if ( in.args.size() >= 2 ) {
			td.tm_year = _wtoi(in.args[1].c_str()) - 1900;
		}
		if ( in.args.size() >= 3 ) {
			td.tm_mon = _wtoi(in.args[2].c_str()) - 1;
		}
		if ( in.args.size() >= 4 ) {
			td.tm_mday = _wtoi(in.args[3].c_str());
		}

		if ( in.args.size() >= 5 ) {
			td.tm_hour = _wtoi(in.args[4].c_str());
		}
		if ( in.args.size() >= 6 ) {
			td.tm_min = _wtoi(in.args[5].c_str());
		}
		if ( in.args.size() >= 7 ) {
			td.tm_sec = _wtoi(in.args[6].c_str());
		}

		time_t c = mktime(&td);

		wchar_t buf[32];
		swprintf(buf,L"%u",c);

		out.result = buf;
		out.result_code = SAORIRESULT_OK;
		return;
	}

	//===================================================
	if ( cmd == L"local" ) {
		time_t c;
		if ( in.args.size() < 2 ) {
			c = time(NULL);
		}
		else {
			c = wcstoul(in.args[1].c_str(),NULL,10);
		}

		const tm *ptd = localtime(&c);

		out.result = SAORI_FUNC::MultiByteToUnicode(asctime(ptd));
		out.result_code = SAORIRESULT_OK;

		out.values.push_back(SAORI_FUNC::numToString(ptd->tm_year+1900));
		out.values.push_back(SAORI_FUNC::numToString(ptd->tm_mon+1));
		out.values.push_back(SAORI_FUNC::numToString(ptd->tm_mday));

		out.values.push_back(SAORI_FUNC::numToString(ptd->tm_hour));
		out.values.push_back(SAORI_FUNC::numToString(ptd->tm_min));
		out.values.push_back(SAORI_FUNC::numToString(ptd->tm_sec));

		out.values.push_back(SAORI_FUNC::numToString(ptd->tm_wday));

		out.values.push_back(SAORI_FUNC::numToString(ptd->tm_yday));

		return;
	}

	//===================================================
	if ( cmd == L"tick" ) {
		out.result = SAORI_FUNC::numToString(::GetTickCount());
		out.result_code = SAORIRESULT_OK;
		return;
	}

	//===================================================
	if ( cmd == L"strftime" ) {
		if ( in.args.size() < 3 ) {
			return;
		}

		time_t c = wcstoul(in.args[1].c_str(),NULL,10);
		const tm *ptd = localtime(&c);

		wchar_t buf[2048];
		wcsftime(buf,sizeof(buf)/sizeof(buf[0]),in.args[2].c_str(),ptd);

		out.result = buf;
		out.result_code = SAORIRESULT_OK;
	}

	//===================================================
	if ( cmd == L"qreki" ) {
		time_t c;
		if ( in.args.size() < 2 ) {
			c = time(NULL);
		}
		else {
			c = wcstoul(in.args[1].c_str(),NULL,10);
		}

		const tm *ptd = localtime(&c);

		int yyyy = ptd->tm_year+1900;
		int mm = ptd->tm_mon+1;
		int dd = ptd->tm_mday;
		int uruu = 0;

		string_t rokuyo = QReki_RokuYo(yyyy,mm,dd,uruu);

		static const wchar_t* URUUTXT[] = {L"",L"â["};
		wchar_t buf[64];
		swprintf(buf,L"%d,%s%d,%d,",yyyy,URUUTXT[uruu],mm,dd);

		out.result = buf + rokuyo;

		out.values.push_back(SAORI_FUNC::numToString(yyyy));
		out.values.push_back(SAORI_FUNC::numToString(mm));
		out.values.push_back(SAORI_FUNC::numToString(dd));
		out.values.push_back(rokuyo);
		out.values.push_back(SAORI_FUNC::numToString(uruu));

		//out.result = buf;
		out.result_code = SAORIRESULT_OK;
	}
}

