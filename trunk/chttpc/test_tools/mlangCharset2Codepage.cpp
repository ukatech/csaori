#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <vector>
#include <map>

#include <ole2.h>
#include <MLang.h>

using namespace std;

#include <stdio.h>

	std::wstring MultiByteToUnicode(const char* Source, unsigned int CodePage, DWORD Flags)
	{
	  if (int Len = ::MultiByteToWideChar(CodePage, Flags, Source, strlen(Source), NULL, 0)) {
		std::vector<wchar_t> Dest(Len);
		if (Len = ::MultiByteToWideChar(CodePage, 0, Source, strlen(Source), &Dest[0], static_cast<int>(Dest.size()))) {
		  return std::wstring(Dest.begin(), Dest.begin() + Len);
		}
	  }
	  return L"";
	}


int main( int argc, char * argv[] ){
	
	if(argc>1) {
		wstring Charset = MultiByteToUnicode(argv[1],CP_ACP,0);

	//IMultiLanguage生成
	CoInitialize( NULL );
	IMultiLanguage2 *lang = NULL;
	HRESULT hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,
		CLSCTX_ALL, IID_IMultiLanguage2, (LPVOID*)&lang);

		MIMECSETINFO csetinfo;
		if (SUCCEEDED(hr))
			hr = lang->GetCharsetInfo(const_cast<wchar_t*>(Charset.c_str()), &csetinfo);

	printf("Codepage: %d\n",csetinfo.uiInternetEncoding);
	}
}