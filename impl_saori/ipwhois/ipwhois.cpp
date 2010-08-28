#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "csaori.h"

string_t CountryCodeToName(const string_t& s);

/*---------------------------------------------------------
	èâä˙âª
---------------------------------------------------------*/
bool CSAORI::load(){
    WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	return true;
}

/*---------------------------------------------------------
	âï˙
---------------------------------------------------------*/
bool CSAORI::unload(){
	WSACleanup();
	return true;
}

/*---------------------------------------------------------
	é¿çs
---------------------------------------------------------*/

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	if ( in.args.size() < 2 ) {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		return;
	}
	
	if ( in.args[0] != L"ipwhois" ) {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		return;
	}

	int Ports = 43; //whois

	ADDRINFO Hints,*AddrInfo;
	memset(&Hints, 0, sizeof (Hints));
	Hints.ai_family = PF_UNSPEC;
	Hints.ai_socktype = SOCK_STREAM;
	
	int RetVal = getaddrinfo("whois.lacnic.net", "43", &Hints, &AddrInfo);
	if ( RetVal != 0 ) {
		out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
		return;
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

	std::string send_msg = SAORI_FUNC::UnicodeToMultiByte(in.args[1]);
	send_msg += "\r\n";
	
	RetVal = send(ConnSocket, send_msg.c_str(), send_msg.size(), 0);
	if (RetVal == SOCKET_ERROR) {
		out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
		return;
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
			out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
			return;
		}

		data = SAORI_FUNC::MultiByteToUnicode(read_string);
	}

	size_t pos = 0, nextpos, ts;
	string_t sl,k,v;

	string_t inetnum,netname,descr,country;
	bool suppress = false;
	bool last_descr = false;

	while ( true ) {
		nextpos = SAORI_FUNC::getLine(sl, data, pos);

		if ( ! suppress ) {
			if ( sl[0] != L'%' && sl[0] != L'[' && sl[0] != L'#' ) {
				ts = sl.find(L":");

				if (ts != string_t::npos) {
					k = sl.substr(0, ts);

					size_t vs = ts + 1;
					while ( sl[vs] == L' ' ) { ++vs; }
					v = sl.substr(vs);

					if ( k.find(L"inetnum") != string_t::npos || k.find(L"NetRange") != string_t::npos ) {
						inetnum = v;
						last_descr = false;
					}
					else if ( k.find(L"netname") != string_t::npos || k.find(L"OrgID") != string_t::npos ) {
						netname = v;
						last_descr = false;
					}
					else if ( k.find(L"descr") != string_t::npos || k.find(L"OrgName") != string_t::npos ) {
						if ( ! last_descr ) {
							descr = v;
						}
						last_descr = true;
					}
					else if ( k.find(L"country") != string_t::npos || k.find(L"Country") != string_t::npos ) {
						country = v;
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

	out.result_code = SAORIRESULT_OK;
	out.result = CountryCodeToName(country);
	out.values.push_back(country);
	out.values.push_back(inetnum);
	out.values.push_back(netname);
	out.values.push_back(descr);
}

/*---------------------------------------------------------
	çëID->çëñº
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


