// modified from o2on project, BSD license
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <vector>
#include <map>

#include <ole2.h>
#include <MLang.h>

using namespace std;

#if 0
# include <stdio.h>
#endif

bool mlangToUnicode(UINT codepage, string &in, wstring &out)
{
	INT inlen = in.size();
	CHAR *in_cstr = const_cast<char *>(in.c_str());
	//IMultiLanguage生成
	CoInitialize( NULL );
	IMultiLanguage2 *lang = NULL;
	HRESULT hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,
		CLSCTX_ALL, IID_IMultiLanguage2, (LPVOID*)&lang);

	if(!codepage) {
		//Charset情報取得
		int detectEncCount = 1;
		DetectEncodingInfo detectEnc;
		lang->DetectInputCodepage(
			MLDETECTCP_HTML, 0,
			in_cstr, &inlen,
			&detectEnc, &detectEncCount);
		codepage = detectEnc.nCodePage;
#if 0
		printf("Codepage: %d\n",detectEnc.nCodePage);
#endif
	}

	//変換後の文字長を取得
	DWORD pdwMode = 0;
	UINT outlen = 0;
	UINT uinlen = inlen;
	if (SUCCEEDED(hr)) {
		hr = lang->ConvertStringToUnicode(
			&pdwMode, codepage,
			in_cstr, &uinlen,
			NULL, &outlen);

		out.resize(outlen);
	}

	//convert to unicode
	if (SUCCEEDED(hr)) {
		hr = lang->ConvertStringToUnicode(
			&pdwMode, codepage,
			in_cstr, &uinlen,
			&out[0], &outlen);
	}

	if (lang)
		lang->Release();
	CoUninitialize();

	return (SUCCEEDED(hr) ? true : false);
}