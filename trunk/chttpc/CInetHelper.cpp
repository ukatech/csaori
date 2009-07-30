#include "CInetHelper.h"
#include "mlang2unicode.h"

#ifdef INET_DEBUG
#include <stdio.h>
#endif

using namespace std;
// *** Public ***

int CInetHelper::getUrlContent(const char* url, const wchar_t* charset, wstring& out, const wchar_t* saveTo) {
	string oResult;

#ifdef INET_DEBUG
printf("CInetHelper::getUrlContent - getInternet\n");
#endif
	HINTERNET hInternet = CInetHelper::getInternet();
	if ( hInternet==NULL ) {
		out = L"InternetOpen failed";
		return CIH_FAIL;
	}

#ifdef INET_DEBUG
printf("CInetHelper::getUrlContent - InternetOpenUrl\n");
#endif
	/* remote_folderのオープン */
	HINTERNET hFile = InternetOpenUrl(
		hInternet,
		url,
		NULL,
		0,
		INTERNET_FLAG_RELOAD,
		0);
	if ( hFile==NULL ) {
		InternetCloseHandle(hInternet);
		out = L"InternetOpenUrl failed";
		return CIH_FAIL;
	}

#ifdef INET_DEBUG
printf("CInetHelper::getUrlContent - readdata\n");
#endif
		/* オープンしたremote_folderからデータを(8192バイトずつ)読み込む */
	vector<char>	theData;
	for(;;) {
		DWORD ReadSize=0;
		static char Buf[8192];
		BOOL bResult = InternetReadFile(
			hFile,
			Buf,
			8192,
			&ReadSize);

		/* 全て読み込んだらループを抜ける */
		if(bResult && (ReadSize == 0))
			break;

		int cur = theData.size();
		theData.resize( cur + ReadSize );
		memcpy( &theData[0] + cur, Buf, ReadSize );
	}

	/* 後処理 */
	InternetCloseHandle(hFile);
	InternetCloseHandle(hInternet);

	if(saveTo != NULL) {
		FILE *fp = _wfopen(saveTo, L"wb");
		int sz = fwrite(&theData[0], 1, theData.size(), fp);
		fclose(fp);
	}

	// 文字列終端。Unicodeの場合を考えて4byte。
	for (int i=0 ; i<4 ; ++i)
		theData.push_back(0);

	oResult.assign(&theData[0], theData.size());

#ifdef INET_DEBUG
printf("CInetHelper::getUrlContent - mlangToUnicode\n");
#endif
	wstring nResult;
	mlangToUnicode(charset, oResult, nResult);

#ifdef INET_DEBUG
printf("CInetHelper::getUrlContent - return\n");
#endif
	out = nResult;
	return CIH_OK;
}

bool CInetHelper::checkInternet() {
	HINTERNET hInternet = CInetHelper::getInternet();
	if ( hInternet==NULL ) {
		return false;
	} else {
		InternetCloseHandle(hInternet);
		return true;
	}
}

// *** Private ***

HINTERNET CInetHelper::getInternet() {
	HINTERNET hInternet = InternetOpen(
		"chttpcSaori/1.0",
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		0);
	return hInternet;
}
