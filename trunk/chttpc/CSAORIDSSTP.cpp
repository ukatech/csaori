//------------------------------------------------------------------------------
//CSAORIDSSTP, modified from CSAORIOutput, need hwnd
//------------------------------------------------------------------------------

#include "csaori.h"
#include "CSAORIDSSTP.h"

string_t CSAORIDSSTP::toString()
{
	std::wstring dest;
	dest += L"NOTIFY SSTP/1.1\r\n";

	dest += L"Charset: " + SAORI_FUNC::CodePagetoString(codepage) + L"\r\n";
	dest += L"Sender: " + sender + L"\r\n";
	dest += L"Event: " + event + L"\r\n";

	if (!refs.empty()) {
		int i, n = (int)(refs.size());
		string_t tmp;
		for(i=0; i<n; i++) {
			swprintf(tmptxt,L"Reference%d",i);

			dest += std::wstring(tmptxt) + std::wstring(L": ");

			tmp = refs[i];
			std::string::size_type nPos = 0;
			while((nPos = tmp.find(L"\r\n", nPos)) != std::string::npos){
				tmp.replace(nPos, 2 , L"\1");
			}
			while((nPos = tmp.find(L"\r", nPos)) != std::string::npos){
				tmp.replace(nPos, 2 , L"\1");
			}
			while((nPos = tmp.find(L"\n", nPos)) != std::string::npos){
				tmp.replace(nPos, 2 , L"\1");
			}

			dest += refs[i] + L"\r\n";
		}
	}
	if (!opts.empty()) {
		std::map<string_t,string_t>::iterator i;
		for(i=opts.begin(); i != opts.end(); i++) {
			dest += i->first + L": " + i->second + L"\r\n";
		}
	}
	
	dest += L"\r\n";
	return dest;
}

void CSAORIDSSTP::send(string_t &sstp)
{
	DWORD result;
	
	COPYDATASTRUCT c;
	c.dwData = 9801;
	c.cbData = sstp.size();
	c.lpData = const_cast<char*>(sstp.c_str());

	SendMessageTimeout(hwnd,WM_COPYDATA,
		reinterpret_cast<WPARAM>(hwnd),
		reinterpret_cast<LPARAM>(&c),
		SMTO_ABORTIFHUNG,1000,&result);
}
