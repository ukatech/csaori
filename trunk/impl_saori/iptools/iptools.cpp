#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

//IPv6
#if _MSC_VER <= 1200
#ifndef _WSPIAPI_COUNTOF
#define _WSPIAPI_COUNTOF(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <iphlpapi.h>
#include <iptypes.h>

#include "csaori.h"

static HWND g_hwnd = NULL;
volatile HANDLE g_hThread = NULL;

string_t CountryCodeToName(const string_t& s);

/*---------------------------------------------------------
	初期化
---------------------------------------------------------*/
bool CSAORI::load(){
    WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	return true;
}

/*---------------------------------------------------------
	解放
---------------------------------------------------------*/
bool CSAORI::unload(){
	if ( g_hThread ) {
		::WaitForSingleObject(g_hThread,INFINITE);
	}

	WSACleanup();
	return true;
}

/*---------------------------------------------------------
	実行
---------------------------------------------------------*/

class ExecuteWhoisData {
public:
	string_t in_event;
	string_t in_host;
	string_t in_whois_to;

	string_t inetnum;
	string_t netname;
	string_t descr;
	string_t country;
	string_t raw;
};

static void _cdecl ExecuteWhoisThreadFunc(void *d);
static bool ExecuteWhois(ExecuteWhoisData &d,bool is_async = false);
static bool ExecuteWhoisSub(ExecuteWhoisData &d,bool is_async = false);
static const char_t* GetMediaTypeFromID(DWORD type);

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	//パラメータ必須 Arg0:コマンド
	if ( in.args.size() < 1 ) {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		return;
	}

	out.result_code = SAORIRESULT_BAD_REQUEST;
	const string_t &cmd = in.args[0];
	
	//***** hwndコマンド **************************************************
	if ( wcsnicmp(cmd.c_str(),L"hwnd",4) == 0 ) {
		//このコマンドのみ特殊：Arg1はhwnd
		if ( in.args.size() < 2 ) {
			out.result_code = SAORIRESULT_BAD_REQUEST;
			return;
		}
		g_hwnd = reinterpret_cast<HWND>(wcstoul(in.args[1].c_str(),NULL,10));
		return;
	}

	//***** whois(async) - こっちを必ず先に書く *************************
	if ( wcsnicmp(cmd.c_str(),L"ipwhois_async",13) == 0 || wcsnicmp(cmd.c_str(),L"whois_async",11) == 0 ) {
		if ( in.args.size() < 2 ) {
			out.result_code = SAORIRESULT_BAD_REQUEST;
			return;
		}
		if ( g_hwnd == NULL ) {
			out.result_code = SAORIRESULT_BAD_REQUEST;
			return;
		}
		if ( g_hThread != NULL ) {
			out.result_code = SAORIRESULT_BAD_REQUEST;
			return;
		}
		
		ExecuteWhoisData *pData = new ExecuteWhoisData;
		pData->in_host = in.args[1];
		if ( in.args.size() >= 3 ) {
			pData->in_whois_to = in.args[2];
		}
		if ( in.args.size() >= 4 ) {
			pData->in_event = in.args[3];
		}

		unsigned long result = _beginthread(ExecuteWhoisThreadFunc,0,pData);
		if ( result != static_cast<unsigned long>(-1) ) {
			g_hThread = reinterpret_cast<HANDLE>(result);
		}

		out.result_code = SAORIRESULT_OK;
		return;
	}

	//***** whois *******************************************************
	if ( wcsnicmp(cmd.c_str(),L"ipwhois",7) == 0 || wcsnicmp(cmd.c_str(),L"whois",5) == 0 ) {
		if ( in.args.size() < 2 ) {
			out.result_code = SAORIRESULT_BAD_REQUEST;
			return;
		}
		
		ExecuteWhoisData d;

		d.in_host = in.args[1];
		if ( in.args.size() >= 3 ) {
			d.in_whois_to = in.args[2];
		}

		if ( ! ExecuteWhois(d) ) {
			out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
			return;
		}

		out.result_code = SAORIRESULT_OK;
		out.result = CountryCodeToName(d.country);
		out.values.push_back(d.country);
		out.values.push_back(d.inetnum);
		out.values.push_back(d.netname);
		out.values.push_back(d.descr);

		return;
	}

	//***** flushdns *******************************************************
	if ( wcsnicmp(cmd.c_str(),L"flushdns",7) == 0 ) {
		BOOL (WINAPI *pDnsFlushResolverCache)();

		HMODULE hDNSAPI = ::LoadLibrary("dnsapi.dll");
		if ( hDNSAPI ) {
			pDnsFlushResolverCache = reinterpret_cast<BOOL (WINAPI*)()>(::GetProcAddress(hDNSAPI,"DnsFlushResolverCache"));

			if ( pDnsFlushResolverCache ) {
				out.result_code = SAORIRESULT_OK;
				if ( pDnsFlushResolverCache() ) {
					out.result = L"1";
				}
				else {
					out.result = L"0";
				}
			}
			else {
				out.result_code = SAORIRESULT_OK;
				out.result = L"0";
			}

			::FreeLibrary(hDNSAPI);
		}
		else {
			out.result_code = SAORIRESULT_OK;
			out.result = L"0";
		}
	}

	//***** adapter ************************************************************
	if ( wcsnicmp(cmd.c_str(),L"adapter",7) == 0 ) {
		ULONG buffer_info = sizeof(IP_ADAPTER_INFO)*30;
		IP_ADAPTER_INFO *ptr_info = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(buffer_info));

		DWORD result = ::GetAdaptersInfo(ptr_info,&buffer_info);
		if ( result == ERROR_BUFFER_OVERFLOW ) {
			free(ptr_info);
			ptr_info = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(buffer_info));

			result = ::GetAdaptersInfo(ptr_info,&buffer_info);
		}

		typedef ULONG (WINAPI *TypeGetAdaptersAddresses)(ULONG Family,ULONG Flags,PVOID Reserved,PIP_ADAPTER_ADDRESSES AdapterAddresses,PULONG SizePointer);

		TypeGetAdaptersAddresses pGetAdaptersAddresses = reinterpret_cast<TypeGetAdaptersAddresses>(::GetProcAddress(::GetModuleHandle("iphlpapi"),"GetAdaptersAddresses"));
		IP_ADAPTER_ADDRESSES *ptr_address = NULL;

		if ( pGetAdaptersAddresses ) {
			ULONG buffer_address = 0x2000U;
			ptr_address = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(malloc(buffer_address));

			result = pGetAdaptersAddresses(AF_UNSPEC,0x0080U/*GAA_FLAG_INCLUDE_GATEWAYS*/,0,ptr_address,&buffer_address);
			if ( result == ERROR_BUFFER_OVERFLOW ) {
				free(ptr_address);
				ptr_address = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(malloc(buffer_address));

				result = pGetAdaptersAddresses(AF_UNSPEC,0x0080U/*GAA_FLAG_INCLUDE_GATEWAYS*/,0,ptr_address,&buffer_address);
			}
		}

		string_t line;
		string_t name;
		char_t buf[64];
		DWORD count = 0;

		for ( IP_ADAPTER_INFO *pAdapterInfo = ptr_info ; pAdapterInfo ; pAdapterInfo = pAdapterInfo->Next ) {
			if ( pAdapterInfo->AddressLength == 0 ) { continue; }
			if ( pAdapterInfo->Type == MIB_IF_TYPE_LOOPBACK ) { continue; }

			count += 1;

			IP_ADAPTER_ADDRESSES *pAdapterAddress = ptr_address;
			while ( pAdapterAddress ) {
				if ( strcmp(pAdapterInfo->AdapterName,pAdapterAddress->AdapterName) == 0 ) {
					break;
				}
				pAdapterAddress = pAdapterAddress->Next;
			}

			MIB_IFROW ifrow;
			ZeroMemory(&ifrow,sizeof(ifrow));
			ifrow.dwIndex = pAdapterInfo->Index;
			::GetIfEntry(&ifrow);

			//-----Index-----
			swprintf(buf,L"%u",pAdapterInfo->Index);
			line = buf;
			line += L"\1";

			//-----Name(GUID)-----
			line += SAORI_FUNC::MultiByteToUnicode(std::string(pAdapterInfo->AdapterName));
			line += L"\1";

			if ( pAdapterAddress ) {
				//-----Desc-----
				line += pAdapterAddress->Description;
				line += L"\1";

				//-----Type-----
				line += GetMediaTypeFromID(pAdapterAddress->IfType);
				line += L"\1";

				//-----Status-----
				static const WORD* status_a[] = {L"unknown",L"up",L"down",L"testing",L"unknown",L"dormant",L"unknown",L"down"};
				if ( pAdapterAddress->OperStatus < (sizeof(status_a)/sizeof(status_a[0])) ) {
					line += status_a[pAdapterAddress->OperStatus];
				}
				else {
					line += L"unknown";
				}
				line += L"\1";

				//-----IP-----
				int address_count = 0;
				for ( IP_ADAPTER_UNICAST_ADDRESS *uni = pAdapterAddress->FirstUnicastAddress; uni; uni = uni->Next ) {
					if(~(uni->Flags) & IP_ADAPTER_ADDRESS_DNS_ELIGIBLE) { continue; }
					if(uni->Flags & IP_ADAPTER_ADDRESS_TRANSIENT) { continue; }

					char host[NI_MAXHOST + 1] = {'\0'};   // host に「0.0.0.0」形式の文字列を取得します。
					if ( getnameinfo(uni->Address.lpSockaddr, uni->Address.iSockaddrLength, host, sizeof(host), 0, 0, NI_NUMERICHOST/*NI_NAMEREQD*/) == 0 ) {
						line += SAORI_FUNC::MultiByteToUnicode(std::string(host));
						line += L",";
						address_count += 1;
					}
				}
				if ( address_count ) {
					line.erase(line.end()-1);
				}
				line += L"\1";
			
			}
			else {
				//-----Desc-----
				line += SAORI_FUNC::MultiByteToUnicode(std::string(pAdapterInfo->Description));
				line += L"\1";

				//-----Type-----
				line += GetMediaTypeFromID(pAdapterInfo->Type);
				line += L"\1";

				//-----Status-----
				static const WORD* status_i[] = {L"down",L"down",L"down",L"dormant",L"up",L"up"};
				if ( ifrow.dwOperStatus < (sizeof(status_i)/sizeof(status_i[0])) ) {
					line += status_i[ifrow.dwOperStatus];
				}
				else {
					line += L"unknown";
				}
				line += L"\1";

				//-----IP-----
				int address_count = 0;
				for ( IP_ADDR_STRING *uni = &(pAdapterInfo->IpAddressList) ; uni ; uni = uni->Next ) {
					line += SAORI_FUNC::MultiByteToUnicode(std::string(uni->IpAddress.String));
					line += L",";
					address_count += 1;
				}
				if ( address_count ) {
					line.erase(line.end()-1);
				}
				line += L"\1";
			}

			//-----PhysAddr-----
			if ( ifrow.dwPhysAddrLen ) {
				for ( DWORD i = 0 ; i < ifrow.dwPhysAddrLen ; ++i ) {
					swprintf(buf,L"%02X:",ifrow.bPhysAddr[i]);
					line += buf;
				}
				line.erase(line.end()-1);
			}
			line += L"\1";

			//-----DHCP-----
			line += pAdapterInfo->DhcpEnabled ? L"dynamic" : L"static";
			line += L"\1";

			//-----Speed-----
			swprintf(buf,L"%u,%u,%u",ifrow.dwSpeed,ifrow.dwInOctets,ifrow.dwOutOctets);
			line += buf;

			out.values.push_back(line);		
		}

		swprintf(buf,L"%u",count);
		out.result = buf;
		out.result_code = SAORIRESULT_OK;

		if ( ptr_address ) {
			free(ptr_address);
		}
		free(ptr_info);

		return;
	}
}

void _cdecl ExecuteWhoisThreadFunc(void *ptr)
{
	ExecuteWhoisData *pData = reinterpret_cast<ExecuteWhoisData*>(ptr);

	ExecuteWhoisData &d = *pData;

	bool result = ExecuteWhois(d,true);

	string_t sstp(L"NOTIFY SSTP/1.1\r\nCharset: UTF-8\r\nSender: tsvnexec SAORI\r\nHWnd: 0\r\nEvent: ");
	if ( d.in_event.size() ) {
		sstp += d.in_event;
		if ( ! result ) {
			sstp += L"Failure";
		}
	}
	else {
		if ( result ) {
			sstp += L"OnIPToolsWhoisComplete";
		}
		else {
			sstp += L"OnIPToolsWhoisFailure";
		}
	}
	sstp += L"\r\n";

	sstp += L"Reference0: ";
	sstp += d.in_host;
	sstp += L"\r\n";

	if ( result ) {
		sstp += L"Reference1: ";
		sstp += d.raw;
		sstp += L"\r\n";

		sstp += L"Reference2: ";
		sstp += d.country;
		sstp += L",";
		sstp += CountryCodeToName(d.country);
		sstp += L"\r\n";

		sstp += L"Reference3: ";
		sstp += d.inetnum;
		sstp += L"\r\n";

		sstp += L"Reference4: ";
		sstp += d.netname;
		sstp += L"\r\n";

		sstp += L"Reference5: ";
		sstp += d.descr;
		sstp += L"\r\n";
	}

	sstp += L"\r\n";

	std::string sstp_a = SAORI_FUNC::UnicodeToMultiByte(sstp.c_str(),CP_UTF8);

	COPYDATASTRUCT c;
	c.dwData = 9801;
	c.cbData = sstp.size();
	c.lpData = const_cast<char*>(sstp_a.c_str());

	DWORD sstpresult;
	::SendMessageTimeout(reinterpret_cast<HWND>(g_hwnd),
		WM_COPYDATA,
		reinterpret_cast<WPARAM>(g_hwnd),
		reinterpret_cast<LPARAM>(&c),
		SMTO_ABORTIFHUNG,5000,&sstpresult);

	delete pData;
	g_hThread = NULL;
}

bool ExecuteWhois(ExecuteWhoisData &d,bool is_async)
{
	if ( d.in_whois_to.size() == 0 ) {
		const char_t* const table[] = {L"whois.iana.org",L"whois.apnic.net",L"whois.arin.net",L"whois.ripe.net",L"whois.lacnic.net",L"whois.afrinic.net"};

		bool result = false;
		for ( int i = 0 ; i < (sizeof(table)/sizeof(table[0])) ; ++i ) {
			d.in_whois_to = table[i];
			result = ExecuteWhoisSub(d,is_async);
			/*if ( ! result ) {
				return result;
			}*/
			if ( d.country.size() > 0 ) {
				return result;
			}
		}
		return result;
	}
	else {
		return ExecuteWhoisSub(d,is_async);
	}
}

bool ExecuteWhoisSub(ExecuteWhoisData &d,bool is_async)
{
	std::string whois_to = SAORI_FUNC::UnicodeToMultiByte(d.in_whois_to.c_str());

	int Ports = 43; //whois

	ADDRINFO Hints,*AddrInfo;
	memset(&Hints, 0, sizeof (Hints));
	Hints.ai_family = PF_UNSPEC;
	Hints.ai_socktype = SOCK_STREAM;
	
	int RetVal = getaddrinfo(whois_to.c_str(), "43", &Hints, &AddrInfo);
	if ( RetVal != 0 ) {
		return false;
	}

	SOCKET ConnSocket = INVALID_SOCKET;
    for (ADDRINFO *AI = AddrInfo; AI != NULL; AI = AI->ai_next) {
        ConnSocket = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol);
        if (ConnSocket == INVALID_SOCKET) {
            continue;
        }
        if (connect(ConnSocket, AI->ai_addr, AI->ai_addrlen) == SOCKET_ERROR) {
			closesocket(ConnSocket);
			ConnSocket = INVALID_SOCKET;
            continue;
		}
	}
	
	freeaddrinfo(AddrInfo);

	std::string send_msg = SAORI_FUNC::UnicodeToMultiByte(d.in_host);
	send_msg += "\r\n";
	
	RetVal = send(ConnSocket, send_msg.c_str(), send_msg.size(), 0);
	if (RetVal == SOCKET_ERROR) {
		return false;
	}

    shutdown(ConnSocket, SD_SEND);

	char buffer[1024];
	buffer[1023] = 0;

	string_t data;
	{
		std::string read_string;

		while ( true ) {
			RetVal = recv(ConnSocket, buffer, sizeof(buffer)-1, 0);
			if (RetVal == SOCKET_ERROR) {
				break;
			}
			if (RetVal == 0) {
				break;
			}
			buffer[RetVal] = 0;
			read_string += buffer;
		}

		closesocket(ConnSocket);

		if (read_string.size() == 0) {
			return false;
		}

		data = SAORI_FUNC::MultiByteToUnicode(read_string);
	}

	size_t pos = 0, nextpos, ts;
	string_t sl,k,v;

	bool suppress = false;
	bool last_descr = false;

	while ( true ) {
		nextpos = SAORI_FUNC::getLine(sl, data, pos);

		if ( ! suppress ) {
			if ( sl[0] != 0 && sl[0] != L'%' && sl[0] != L'[' && sl[0] != L'#' ) {
				d.raw += sl;
				d.raw += L"\1";

				ts = sl.find(L":");

				if (ts != string_t::npos) {
					k = sl.substr(0, ts);

					size_t vs = ts + 1;
					while ( sl[vs] == L' ' ) { ++vs; }
					v = sl.substr(vs);

					if ( k.find(L"inetnum") != string_t::npos || k.find(L"NetRange") != string_t::npos ) {
						d.inetnum = v;
						last_descr = false;
					}
					else if ( k.find(L"netname") != string_t::npos || k.find(L"OrgID") != string_t::npos ) {
						d.netname = v;
						last_descr = false;
					}
					else if ( k.find(L"descr") != string_t::npos || k.find(L"OrgName") != string_t::npos ) {
						if ( ! last_descr ) {
							d.descr = v;
						}
						last_descr = true;
					}
					else if ( k.find(L"country") != string_t::npos || k.find(L"Country") != string_t::npos ) {
						d.country = v;
						last_descr = false;
					}
					else if ( k.find(L"route") != string_t::npos ) {
						suppress = true;
						last_descr = false;
					}
					else {
						last_descr = false;
					}
				}
			}
		}

		if (nextpos == string_t::npos) break;
		pos = nextpos;
	}

	return true;
}

/*---------------------------------------------------------
	国ID->国名
---------------------------------------------------------*/
string_t CountryCodeToName(const string_t& s)
{
	static const struct {const char_t * k;const char_t * v;} values[] = {
		{L"AF",L"AFGHANISTAN"},
		{L"AX",L"ALAND ISLANDS"},
		{L"AL",L"ALBANIA"},
		{L"DZ",L"ALGERIA"},
		{L"AS",L"AMERICAN SAMOA"},
		{L"AD",L"ANDORRA"},
		{L"AO",L"ANGOLA"},
		{L"AI",L"ANGUILLA"},
		{L"AQ",L"ANTARCTICA"},
		{L"AG",L"ANTIGUA AND BARBUDA"},
		{L"AR",L"ARGENTINA"},
		{L"AM",L"ARMENIA"},
		{L"AW",L"ARUBA"},
		{L"AU",L"AUSTRALIA"},
		{L"AT",L"AUSTRIA"},
		{L"AZ",L"AZERBAIJAN"},

		{L"BS",L"BAHAMAS"},
		{L"BH",L"BAHRAIN"},
		{L"BD",L"BANGLADESH"},
		{L"BB",L"BARBADOS"},
		{L"BY",L"BELARUS"},
		{L"BE",L"BELGIUM"},
		{L"BZ",L"BELIZE"},
		{L"BJ",L"BENIN"},
		{L"BM",L"BERMUDA"},
		{L"BT",L"BHUTAN"},
		{L"BO",L"BOLIVIA, PLURINATIONAL STATE OF"},
		{L"BA",L"BOSNIA AND HERZEGOVINA"},
		{L"BW",L"BOTSWANA"},
		{L"BV",L"BOUVET ISLAND"},
		{L"BR",L"BRAZIL"},
		{L"IO",L"BRITISH INDIAN OCEAN TERRITORY"},
		{L"BN",L"BRUNEI DARUSSALAM"},
		{L"BG",L"BULGARIA"},
		{L"BF",L"BURKINA FASO"},
		{L"BI",L"BURUNDI"},

		{L"KH",L"CAMBODIA"},
		{L"CM",L"CAMEROON"},
		{L"CA",L"CANADA"},
		{L"CV",L"CAPE VERDE"},
		{L"KY",L"CAYMAN ISLANDS"},
		{L"CF",L"CENTRAL AFRICAN REPUBLIC"},
		{L"TD",L"CHAD"},
		{L"CL",L"CHILE"},
		{L"CN",L"CHINA"},
		{L"CX",L"CHRISTMAS ISLAND"},
		{L"CC",L"COCOS (KEELING) ISLANDS"},
		{L"CO",L"COLOMBIA"},
		{L"KM",L"COMOROS"},
		{L"CG",L"CONGO"},
		{L"CD",L"CONGO, THE DEMOCRATIC REPUBLIC OF THE"},
		{L"CK",L"COOK ISLANDS"},
		{L"CR",L"COSTA RICA"},
		{L"CI",L"COTE D'IVOIRE"},
		{L"HR",L"CROATIA"},
		{L"CU",L"CUBA"},
		{L"CY",L"CYPRUS"},
		{L"CZ",L"CZECH REPUBLIC"},

		{L"DK",L"DENMARK"},
		{L"DJ",L"DJIBOUTI"},
		{L"DM",L"DOMINICA"},
		{L"DO",L"DOMINICAN REPUBLIC"},

		{L"EC",L"ECUADOR"},
		{L"EG",L"EGYPT"},
		{L"SV",L"EL SALVADOR"},
		{L"GQ",L"EQUATORIAL GUINEA"},
		{L"ER",L"ERITREA"},
		{L"EE",L"ESTONIA"},
		{L"ET",L"ETHIOPIA"},

		{L"FK",L"FALKLAND ISLANDS (MALVINAS)"},
		{L"FO",L"FAROE ISLANDS"},
		{L"FJ",L"FIJI"},
		{L"FI",L"FINLAND"},
		{L"FR",L"FRANCE"},
		{L"GF",L"FRENCH GUIANA"},
		{L"PF",L"FRENCH POLYNESIA"},
		{L"TF",L"FRENCH SOUTHERN TERRITORIES"},

		{L"GA",L"GABON"},
		{L"GM",L"GAMBIA"},
		{L"GE",L"GEORGIA"},
		{L"DE",L"GERMANY"},
		{L"GH",L"GHANA"},
		{L"GI",L"GIBRALTAR"},
		{L"GR",L"GREECE"},
		{L"GL",L"GREENLAND"},
		{L"GD",L"GRENADA"},
		{L"GP",L"GUADELOUPE"},
		{L"GU",L"GUAM"},
		{L"GT",L"GUATEMALA"},
		{L"GG",L"GUERNSEY"},
		{L"GN",L"GUINEA"},
		{L"GW",L"GUINEA-BISSAU"},
		{L"GY",L"GUYANA"},

		{L"HT",L"HAITI"},
		{L"HM",L"HEARD ISLAND AND MCDONALD ISLANDS"},
		{L"VA",L"HOLY SEE (VATICAN CITY STATE)"},
		{L"HN",L"HONDURAS"},
		{L"HK",L"HONG KONG"},
		{L"HU",L"HUNGARY"},

		{L"IS",L"ICELAND"},
		{L"IN",L"INDIA"},
		{L"ID",L"INDONESIA"},
		{L"IR",L"IRAN, ISLAMIC REPUBLIC OF"},
		{L"IQ",L"IRAQ"},
		{L"IE",L"IRELAND"},
		{L"IM",L"ISLE OF MAN"},
		{L"IL",L"ISRAEL"},
		{L"IT",L"ITALY"},

		{L"JM",L"JAMAICA"},
		{L"JP",L"JAPAN"},
		{L"JE",L"JERSEY"},
		{L"JO",L"JORDAN"},

		{L"KZ",L"KAZAKHSTAN"},
		{L"KE",L"KENYA"},
		{L"KI",L"KIRIBATI"},
		{L"KP",L"KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF"},
		{L"KR",L"KOREA, REPUBLIC OF"},
		{L"KW",L"KUWAIT"},
		{L"KG",L"KYRGYZSTAN"},

		{L"LA",L"LAO PEOPLE'S DEMOCRATIC REPUBLIC"},
		{L"LV",L"LATVIA"},
		{L"LB",L"LEBANON"},
		{L"LS",L"LESOTHO"},
		{L"LR",L"LIBERIA"},
		{L"LY",L"LIBYAN ARAB JAMAHIRIYA"},
		{L"LI",L"LIECHTENSTEIN"},
		{L"LT",L"LITHUANIA"},
		{L"LU",L"LUXEMBOURG"},

		{L"MO",L"MACAO"},
		{L"MK",L"MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF"},
		{L"MG",L"MADAGASCAR"},
		{L"MW",L"MALAWI"},
		{L"MY",L"MALAYSIA"},
		{L"MV",L"MALDIVES"},
		{L"ML",L"MALI"},
		{L"MT",L"MALTA"},
		{L"MH",L"MARSHALL ISLANDS"},
		{L"MQ",L"MARTINIQUE"},
		{L"MR",L"MAURITANIA"},
		{L"MU",L"MAURITIUS"},
		{L"YT",L"MAYOTTE"},
		{L"MX",L"MEXICO"},
		{L"FM",L"MICRONESIA, FEDERATED STATES OF"},
		{L"MD",L"MOLDOVA, REPUBLIC OF"},
		{L"MC",L"MONACO"},
		{L"MN",L"MONGOLIA"},
		{L"ME",L"MONTENEGRO"},
		{L"MS",L"MONTSERRAT"},
		{L"MA",L"MOROCCO"},
		{L"MZ",L"MOZAMBIQUE"},
		{L"MM",L"MYANMAR"},

		{L"NA",L"NAMIBIA"},
		{L"NR",L"NAURU"},
		{L"NP",L"NEPAL"},
		{L"NL",L"NETHERLANDS"},
		{L"AN",L"NETHERLANDS ANTILLES"},
		{L"NC",L"NEW CALEDONIA"},
		{L"NZ",L"NEW ZEALAND"},
		{L"NI",L"NICARAGUA"},
		{L"NE",L"NIGER"},
		{L"NG",L"NIGERIA"},
		{L"NU",L"NIUE"},
		{L"NF",L"NORFOLK ISLAND"},
		{L"MP",L"NORTHERN MARIANA ISLANDS"},
		{L"NO",L"NORWAY"},

		{L"OM",L"OMAN"},

		{L"PK",L"PAKISTAN"},
		{L"PW",L"PALAU"},
		{L"PS",L"PALESTINIAN TERRITORY, OCCUPIED"},
		{L"PA",L"PANAMA"},
		{L"PG",L"PAPUA NEW GUINEA"},
		{L"PY",L"PARAGUAY"},
		{L"PE",L"PERU"},
		{L"PH",L"PHILIPPINES"},
		{L"PN",L"PITCAIRN"},
		{L"PL",L"POLAND"},
		{L"PT",L"PORTUGAL"},
		{L"PR",L"PUERTO RICO"},

		{L"QA",L"QATAR"},

		{L"RE",L"REUNION"},
		{L"RO",L"ROMANIA"},
		{L"RU",L"RUSSIAN FEDERATION"},
		{L"RW",L"RWANDA"},

		{L"BL",L"SAINT BARTHELEMY"},
		{L"SH",L"SAINT HELENA, ASCENSION AND TRISTAN DA CUNHA"},
		{L"KN",L"SAINT KITTS AND NEVIS"},
		{L"LC",L"SAINT LUCIA"},
		{L"MF",L"SAINT MARTIN"},
		{L"PM",L"SAINT PIERRE AND MIQUELON"},
		{L"VC",L"SAINT VINCENT AND THE GRENADINES"},
		{L"WS",L"SAMOA"},
		{L"SM",L"SAN MARINO"},
		{L"ST",L"SAO TOME AND PRINCIPE"},
		{L"SA",L"SAUDI ARABIA"},
		{L"SN",L"SENEGAL"},
		{L"RS",L"SERBIA"},
		{L"SC",L"SEYCHELLES"},
		{L"SL",L"SIERRA LEONE"},
		{L"SG",L"SINGAPORE"},
		{L"SK",L"SLOVAKIA"},
		{L"SI",L"SLOVENIA"},
		{L"SB",L"SOLOMON ISLANDS"},
		{L"SO",L"SOMALIA"},
		{L"ZA",L"SOUTH AFRICA"},
		{L"GS",L"SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS"},
		{L"ES",L"SPAIN"},
		{L"LK",L"SRI LANKA"},
		{L"SD",L"SUDAN"},
		{L"SR",L"SURINAME"},
		{L"SJ",L"SVALBARD AND JAN MAYEN"},
		{L"SZ",L"SWAZILAND"},
		{L"SE",L"SWEDEN"},
		{L"CH",L"SWITZERLAND"},
		{L"SY",L"SYRIAN ARAB REPUBLIC"},

		{L"TW",L"TAIWAN, PROVINCE OF CHINA"},
		{L"TJ",L"TAJIKISTAN"},
		{L"TZ",L"TANZANIA, UNITED REPUBLIC OF"},
		{L"TH",L"THAILAND"},
		{L"TL",L"TIMOR-LESTE"},
		{L"TG",L"TOGO"},
		{L"TK",L"TOKELAU"},
		{L"TO",L"TONGA"},
		{L"TT",L"TRINIDAD AND TOBAGO"},
		{L"TN",L"TUNISIA"},
		{L"TR",L"TURKEY"},
		{L"TM",L"TURKMENISTAN"},
		{L"TC",L"TURKS AND CAICOS ISLANDS"},
		{L"TV",L"TUVALU"},

		{L"UG",L"UGANDA"},
		{L"UA",L"UKRAINE"},
		{L"AE",L"UNITED ARAB EMIRATES"},
		{L"GB",L"UNITED KINGDOM"},
		{L"US",L"UNITED STATES"},
		{L"UM",L"UNITED STATES MINOR OUTLYING ISLANDS"},
		{L"UY",L"URUGUAY"},
		{L"UZ",L"UZBEKISTAN"},

		{L"VU",L"VANUATU"},
		{L"VE",L"VENEZUELA, BOLIVARIAN REPUBLIC OF"},
		{L"VN",L"VIET NAM"},
		{L"VG",L"VIRGIN ISLANDS, BRITISH"},
		{L"VI",L"VIRGIN ISLANDS, U.S."},

		{L"WF",L"WALLIS AND FUTUNA"},
		{L"EH",L"WESTERN SAHARA"},

		{L"YE",L"YEMEN"},

		{L"ZM",L"ZAMBIA"},
		{L"ZW",L"ZIMBABWE"},
	};

	static const size_t n = sizeof(values) / sizeof(values[0]);

	for ( size_t i = 0 ; i < n ; ++i ) {
		if ( values[i].k == s ) {
			return values[i].v;
		}
	}
	return s;
}


static const char_t* GetMediaTypeFromID(DWORD type)
{
	switch ( type ) {
	case IF_TYPE_ETHERNET_CSMACD:
		return L"Ethernet";
	case IF_TYPE_ISO88025_TOKENRING:
		return L"TokenRing";
	case IF_TYPE_PPP:
		return L"PPP";
	case IF_TYPE_SOFTWARE_LOOPBACK:
		return L"Loopback";
	case IF_TYPE_ATM:
		return L"ATM";
	case IF_TYPE_IEEE80211:
		return L"IEEE802.11";
	case IF_TYPE_TUNNEL:
		return L"Tunnel";
	case IF_TYPE_IEEE1394:
		return L"IEEE1394";
	default :
		return L"Other";
	}
}

