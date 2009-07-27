// modified from o2on project, BSD license
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>
#include <vector>
#include <map>

#include <ole2.h>
#include <MLang.h>

using namespace std;

#ifdef CHARSET_DEBUG
# include <stdio.h>
#endif

bool mlangToUnicode(const wchar_t *charset, string &in, wstring &out)
{
	INT inlen = in.size();
	UINT codepage = 0;
	CHAR *in_cstr = const_cast<char *>(in.c_str());
	//IMultiLanguage生成
	CoInitialize( NULL );
	IMultiLanguage2 *lang = NULL;
	HRESULT hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,
		CLSCTX_ALL, IID_IMultiLanguage2, (LPVOID*)&lang);

#ifdef CHARSET_DEBUG
	printf("mlangToUnicode : in =%s", in.c_str());
#endif

	//Charset情報取得
	if(charset != NULL) 
		codepage = _wtoi(charset);
	if(!codepage) {
		if(charset == NULL) {
			int detectEncCount = 1;
			DetectEncodingInfo detectEnc;
			lang->DetectInputCodepage(
				MLDETECTCP_HTML, 0,
				in_cstr, &inlen,
				&detectEnc, &detectEncCount);
			codepage = detectEnc.nCodePage;
		} else {
			MIMECSETINFO csetinfo;
			if (SUCCEEDED(hr))
				hr = lang->GetCharsetInfo(const_cast<wchar_t*>(charset), &csetinfo);
			codepage = csetinfo.uiInternetEncoding;
		}
	}

#ifdef CHARSET_DEBUG
	printf("Codepage: %d\n",codepage);
#endif

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

#ifdef CHARSET_DEBUG
	wprintf(L"mlangToUnicode : out =%s", out.c_str());
#endif

	return (SUCCEEDED(hr) ? true : false);
}