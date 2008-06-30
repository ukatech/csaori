#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include "stdafx.h"
#include "csaori.h"

/*---------------------------------------------------------
	初期化
---------------------------------------------------------*/
bool CSAORI::load(){
	return true;
}

/*---------------------------------------------------------
	解放
---------------------------------------------------------*/
bool CSAORI::unload(){
	return true;
}

/*---------------------------------------------------------
	自動free
---------------------------------------------------------*/
class CAutoFree {
private:
	CAutoFree() { }

	void *m_pBuf;
public:
	CAutoFree(void* pBuf) : m_pBuf(pBuf) { }
	~CAutoFree() { if ( m_pBuf ) { free(m_pBuf); } }

	operator void*() { return m_pBuf; }
};

/*---------------------------------------------------------
	実行
---------------------------------------------------------*/
void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out){
	out.result_code = SAORIRESULT_BAD_REQUEST;
	if ( in.args.size() < 1 ) { return; }

	std::string filepath = SAORI_FUNC::UnicodeToMultiByte(in.args[0]);
	const char* pFile = filepath.c_str();

	out.result_code = SAORIRESULT_OK;

	//まずsize
	DWORD tmp;
	DWORD dwVersionSize = ::GetFileVersionInfoSize(pFile,&tmp);
	if ( ! dwVersionSize ) {
		out.result = L"ERROR_OPEN";
	}

	CAutoFree pBuf = malloc(dwVersionSize + 16); //念のため16バイトほどおまけしてみる

	//情報取得メイン
	if ( ! ::GetFileVersionInfo(pFile,NULL,dwVersionSize + 16,pBuf) ) {
		out.result = L"ERROR_NORESOURCE";
		return;
	}

	//この後の文字列取得に備えて言語識別子の取得
	struct LANGANDCODEPAGE {
	  WORD wLanguage;
	  WORD wCodePage;
	} *pTranslate;

	UINT size;
	::VerQueryValue(pBuf, "\\VarFileInfo\\Translation", (void**)&pTranslate, &size);

	char langText[32] = "000004b0";
	if ( size ) {
		sprintf(langText,"%04x%04x",pTranslate->wLanguage,pTranslate->wCodePage);
	}

	//基本的な固定サイズ情報
	UINT verInfoLen;
	VS_FIXEDFILEINFO *verInfo = NULL;
	::VerQueryValue(pBuf,"\\",(void**)&verInfo,&verInfoLen);

	if ( ! verInfoLen || ! verInfo ) {
		out.result = L"ERROR_NOINFO";
		return;
	}

	wchar_t version_str[70];
	swprintf(version_str,L"%u.%u.%u.%u",
		verInfo->dwFileVersionMS >> 16,verInfo->dwFileVersionMS & 0xFFFFU,
		verInfo->dwFileVersionLS >> 16,verInfo->dwFileVersionLS & 0xFFFFU);

	out.result = version_str;

	//文字列系情報
	std::string str_file_info_base = std::string("\\StringFileInfo\\") + langText + "\\";
	
	const char *pInfoList[] = {"LegalCopyright","FileDescription"};

	std::string str_file_info;
	
	for ( unsigned int i = 0 ; i < sizeof(pInfoList) / sizeof(pInfoList[0]) ; ++i ) {
		str_file_info = str_file_info_base + pInfoList[i];

		UINT verInfoLen;
		char *pText = NULL;
		::VerQueryValue(pBuf,const_cast<char*>(str_file_info.c_str()),(void**)&pText,&verInfoLen);

		if ( verInfoLen && pText ) {
			out.values.push_back(SAORI_FUNC::MultiByteToUnicode(pText));
		}
		else {
			out.values.push_back(L"");
		}
	}

}

