/*
 * Credits Block *** PLEASE DO NOT REMOVE ***
 * 
 * UDS (UpDownSpeed) SAORI is modified from KSM's SAORI Sample.
 * 
 * IP helper code snippets are taken from:
 * Retrieve the number of bytes sent/received (and other useful infos) for any network adapter
 * http://www.codeproject.com/internet/BytesSentReceived.asp
 * 
 * Copyright(C) Roy 2007. Some Rights Reserved.
 * Licence: CC-SA 3.0 ( http://creativecommons.org/licenses/by-sa/3.0/deed.zh_TW )
 *
 * Modified for CSaori Library by C.Ponapalt.
 */

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>

#include <iphlpapi.h>

#include "csaori.h"

//////////DEBUG/////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
////////////////////////////////////////

typedef DWORD (_stdcall *TGetIfTable) (
  MIB_IFTABLE *pIfTable,  // buffer for interface table 
  ULONG *pdwSize,         // size of buffer
  BOOL bOrder             // sort the table by index?
);

typedef DWORD (_stdcall *TGetNumberOfInterfaces) (
  PDWORD pdwNumIf  // pointer to number of interfaces
);

TGetIfTable pGetIfTable;
TGetNumberOfInterfaces pGetNumberOfInterfaces;
HMODULE hModuleIpHlp;

bool CSAORI::load()
{
	hModuleIpHlp = LoadLibrary("Iphlpapi.dll");

	pGetIfTable=(TGetIfTable)GetProcAddress(hModuleIpHlp,"GetIfTable");
	pGetNumberOfInterfaces=(TGetNumberOfInterfaces)GetProcAddress(hModuleIpHlp,"GetNumberOfInterfaces");

	return true;
}

bool CSAORI::unload()
{
	if ( hModuleIpHlp ) {
		FreeLibrary(hModuleIpHlp);
	}
	return true;
}

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	//iphlpapi loading check
	if ( !pGetIfTable || !pGetNumberOfInterfaces) {
		out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
		out.result = L"NG";
		out.values.push_back(string_t(L"Error loading Iphlpapi.dll"));
		return;
	}

	//parameter check
	if ( in.args.size() == 0 ) {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		out.result = L"NG";
		out.values.push_back(L"Invalid arguments");
		return;
	}

	DWORD dwCountOfInterfaces = 0;
	ULONG uRetCode = pGetNumberOfInterfaces(&dwCountOfInterfaces);
	if ( uRetCode != NO_ERROR ) {
		out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
		out.result = L"NG";
		out.values.push_back(string_t(L"Error. RetCode=" + SAORI_FUNC::intToString(uRetCode) ));
		return;
	}

	DWORD dwSize = sizeof(DWORD) + (sizeof(MIB_IFROW) * dwCountOfInterfaces);
	MIB_IFTABLE *pTable = reinterpret_cast<MIB_IFTABLE*>(malloc(dwSize));

	uRetCode=pGetIfTable(pTable,&dwSize,TRUE);
	if (uRetCode == 122) {
		// now we know how much memory allocate
		free(pTable);
		pTable = reinterpret_cast<MIB_IFTABLE*>(malloc(dwSize));
		uRetCode=pGetIfTable(pTable,&dwSize,TRUE);
	}

	if ( uRetCode != 0 ) {
		out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
		out.result = L"NG";
		out.values.push_back(string_t(L"Error. RetCode=" + SAORI_FUNC::intToString(uRetCode) ));
		free(pTable);
		return;
	}

	//////////////////////////////////////////////////////////////
	if ( in.args[0] == L"ListInt") {
		out.result_code=SAORIRESULT_OK;

		if(in.args.size() < 2 || !wcstoul(in.args[1].c_str(),NULL,10) ) {
			//values mode
			for ( UINT i=0 ; i<pTable->dwNumEntries ; ++i ) {
				MIB_IFROW Row=pTable->table[i];

				char szDescr[MAXLEN_IFDESCR+1];
				memcpy(szDescr,Row.bDescr,Row.dwDescrLen);
				szDescr[Row.dwDescrLen]=0;

				out.result = L"OK";
				
				out.values.push_back(SAORI_FUNC::MultiByteToUnicode(std::string(szDescr)));
			}
		}
		else {
			//result mode
			for ( UINT i=0 ; i<pTable->dwNumEntries ; ++i ) {
				MIB_IFROW Row=pTable->table[i];

				char szDescr[MAXLEN_IFDESCR+1];
				memcpy(szDescr,Row.bDescr,Row.dwDescrLen);
				szDescr[Row.dwDescrLen]=0;

				out.result += SAORI_FUNC::intToString(i) + L": " + SAORI_FUNC::MultiByteToUnicode(std::string(szDescr)) + L"\\n";
			}
		}
	}
	//////////////////////////////////////////////////////////////
	else if ( in.args[0] == L"UpDownSpeed") {
		int nIndex = 0;
		UINT nFactor = 1;

		if (in.args.size() >= 2) {
			nIndex = wcstoul(in.args[1].c_str(),NULL,10);
		}
		if (in.args.size() >= 3) {
			nFactor = wcstoul(in.args[2].c_str(),NULL,10);
		}

		if (1 > nFactor)
			nFactor = 1;
		else if (20 < nFactor)
			nFactor = 20;

		if (nIndex >= (long)pTable->dwNumEntries)
			nIndex = pTable->dwNumEntries-1;

		DWORD odwInOctets = 0, odwOutOctets = 0;
		if (nIndex < 0) {
			for (UINT i=0;i<pTable->dwNumEntries;i++) {
				MIB_IFROW &Row=pTable->table[i];
				odwInOctets += Row.dwInOctets;
				odwOutOctets += Row.dwOutOctets;
			}
		} else {
			MIB_IFROW &Row=pTable->table[nIndex];
			odwInOctets = Row.dwInOctets;
			odwOutOctets = Row.dwOutOctets;
		}

		Sleep((1000 / nFactor));
		pGetIfTable(pTable,&dwSize,TRUE);

		if (nIndex < 0) {
			DWORD ndwInOctets = 0, ndwOutOctets = 0;
			for (UINT i=0;i<pTable->dwNumEntries;i++) {
				MIB_IFROW &Row=pTable->table[i];
				ndwInOctets += Row.dwInOctets;
				ndwOutOctets += Row.dwOutOctets;
			}
			odwInOctets = (ndwInOctets - odwInOctets) * nFactor;
			odwOutOctets = (ndwOutOctets - odwOutOctets) * nFactor;
		} else {
			MIB_IFROW &Row=pTable->table[nIndex];
			odwInOctets = (Row.dwInOctets - odwInOctets) * nFactor;
			odwOutOctets = (Row.dwOutOctets - odwOutOctets) * nFactor;
		}

		out.result_code=SAORIRESULT_OK;

		if(in.args.size() < 4 || !wcstoul(in.args[3].c_str(),NULL,10) ) {
			out.result = L"OK";
			out.values.push_back(SAORI_FUNC::intToString(odwInOctets) );
			out.values.push_back(SAORI_FUNC::intToString(odwOutOctets) );
		} else {
			out.result = L"Down: " + SAORI_FUNC::intToString(odwInOctets) + L" Bps, Up: " + SAORI_FUNC::intToString(odwOutOctets) + L" Bps";
		}
	}
	else {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		out.result = L"NG";
		out.values.push_back(L"Invalid arguments");
	}

	free(pTable);


}
