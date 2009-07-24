#include "csaori.h"
#include <wininet.h>
#include "mlang2unicode.h"

bool CSAORI::load()
{
	return true;
}

bool CSAORI::unload()
{
	return true;
}

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	if (in.args.size() > 0) {
		string oResult;
		string url = SAORI_FUNC::UnicodeToMultiByte(in.args[0], CP_UTF8);
		HINTERNET hInternet = InternetOpen(
			"chttpcSaori/1.0",
			INTERNET_OPEN_TYPE_PRECONFIG,
			NULL,
			NULL,
			0);
		if ( hInternet==NULL ) {
			out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
			return;
		}

		/* remote_folderのオープン */
		HINTERNET hFile = InternetOpenUrl(
			hInternet,
			url.c_str(),
			NULL,
			0,
			INTERNET_FLAG_RELOAD,
			0);
		if ( hFile==NULL ) {
			InternetCloseHandle(hInternet);
			out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
			return;
		}

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

		// 文字列終端。Unicodeの場合を考えて4byte。
		for (int i=0 ; i<4 ; ++i)
			theData.push_back(0);

		oResult.assign(&theData[0], theData.size());

		wstring nResult;
		mlangToUnicode(0, oResult, nResult);

#if 0
  FILE * pFile;
  pFile = fopen ( "chttpc.txt" , "wb" );
  fwrite (nResult.c_str() , 1 , wcslen(nResult.c_str())*2 , pFile );
  fclose (pFile);
#endif

		out.result_code = SAORIRESULT_OK;
		out.result = nResult;
	}

}
