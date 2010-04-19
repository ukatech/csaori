#include "CInetHelper.h"
#include "mlang2unicode.h"
#include "csaori.h"

using namespace std;
// *** Public ***

int CInetHelper::getUrlContent(const char* url, const wchar_t* charset, wstring& out, const wchar_t* saveTo, bool isLocalFile) {
	string oResult;
	vector<char> theData;

	if(isLocalFile) {
		wstring	fileW = SAORI_FUNC::MultiByteToUnicode(url,CP_UTF8);
		FILE *	pFile;
		size_t	result;
		long	lSize;

		pFile = _wfopen(fileW.substr(5).c_str(), L"rb");
		if (pFile==NULL) {
			out = L"fopen failed";
			return CIH_FAIL;
		}

		// obtain file size:
		fseek(pFile, 0, SEEK_END);
		lSize = ftell(pFile);
		rewind(pFile);

		// allocate memory to contain the whole file:
		theData.resize(lSize);

		// copy the file into the buffer:
		result = fread(&theData[0],1,lSize,pFile);
		if (result != lSize) {
			out = L"fread failed";
			return CIH_FAIL;
		}

		/* the whole file is now loaded in the memory buffer. */
		fclose(pFile);	
	}
	else {
		HINTERNET hInternet = CInetHelper::getInternet();
		if ( hInternet==NULL ) {
			out = L"InternetOpen failed";
			return CIH_FAIL;
		}

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

		/* オープンしたremote_folderからデータを(8192バイトずつ)読み込む */
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
	}

	if(saveTo != NULL) {
		FILE *fp = _wfopen(saveTo, L"wb");
		int sz = fwrite(&theData[0], 1, theData.size(), fp);
		fclose(fp);
	}

	// 文字列終端。Unicodeの場合を考えて4byte。
	for (int i=0 ; i<4 ; ++i)
		theData.push_back(0);

	oResult.assign(&theData[0], theData.size());

	wstring nResult;
	mlangToUnicode(charset, oResult, nResult);

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
