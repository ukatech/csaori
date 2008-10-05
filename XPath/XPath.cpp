
#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#include "csaori.h"
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <time.h>

#include "tinyxml.h"
#include "xpath_processor.h"

//////////WINDOWS DEFINE///////////////////////////
//includeのあとにおいてね！
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

class XMLDocInfo
{
private:
	XMLDocInfo() { }
public:
	TiXmlDocument *xml;
	unsigned int id;
	string_t filepath;
	unsigned int cp;

	XMLDocInfo(TiXmlDocument *x,unsigned int i,string_t f,unsigned int c) :
		xml(x) , id(i) , filepath(f) , cp(c) { }
	~XMLDocInfo() {
		delete xml;
	}
};

typedef std::vector<XMLDocInfo *> XMLPtrVector;

static XMLPtrVector g_xml;

/*---------------------------------------------------------
	初期化
---------------------------------------------------------*/
bool CSAORI::load()
{
	srand(time(NULL));
	return true;
}

/*---------------------------------------------------------
	解放
---------------------------------------------------------*/
bool CSAORI::unload()
{
	for ( XMLPtrVector::iterator it = g_xml.begin() ; it != g_xml.end() ; ++it ) {
		delete *it;
	}
	g_xml.clear();
	return true;
}

/*---------------------------------------------------------
	実行
---------------------------------------------------------*/
void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	//標準：バッドリクエスト
	//正常にいけることがわかった時点で200/204に書き換える
	out.result_code = SAORIRESULT_BAD_REQUEST;
	if ( ! in.args.size() ) {
		return;
	}

	if ( wcsicmp(in.args[0].c_str(),L"open") == 0 ) {
		//入力：open,ファイル名
		//出力：ファイルID

		if ( in.args.size() < 2 ) {
			return;
		}

		//同名のファイルが開いてたら再利用
		for ( XMLPtrVector::iterator it = g_xml.begin() ; it != g_xml.end() ; ++it ) {
			if ( (**it).filepath == in.args[1] ) {
				std::wstringstream outstr(out.result);
				outstr << (**it).id;

				out.result_code = SAORIRESULT_OK;
				return;
			}
		}

		//ふぁいるおーぽん！
		std::string fname = SAORI_FUNC::UnicodeToMultiByte(in.args[1]);

		TiXmlDocument *doc = new TiXmlDocument;
		if ( ! doc->LoadFile(fname.c_str()) ) {
			delete doc;
			return;
		}

		//最初の子があるか確認
		TiXmlNode *pNodeDec = doc->FirstChild();
		if ( ! pNodeDec ) {
			delete doc;
			return;
		}

		//定義
		unsigned int cp;
		TiXmlDeclaration *pDec = pNodeDec->ToDeclaration();
		if ( pDec && strlen(pDec->Encoding()) ) {
			cp = SAORI_FUNC::StringtoCodePage(pDec->Encoding());
		}
		else {
			cp = CP_UTF8;
		}

		//管理構造体につっこんで200リターン
		unsigned int id = rand();

		XMLDocInfo *pInfo = new XMLDocInfo(doc,id,in.args[1],cp);
		g_xml.push_back(pInfo);

		std::wstringstream outstr;
		outstr << id;

		out.result = outstr.str();
		out.result_code = SAORIRESULT_OK;
	}
	else if ( wcsicmp(in.args[0].c_str(),L"close") == 0 ) {
		//入力：close,ファイルID

		if ( in.args.size() < 2 ) {
			return;
		}

		//単純に探して見つかったらボッシュート
		//何かひどいことが起こってIDがだぶる場合に備えて全部なめる
		unsigned int id = wcstoul(in.args[1].c_str(),NULL,10);
		bool found = false;

		for ( XMLPtrVector::iterator it = g_xml.begin() ; it != g_xml.end() ; ++it ) {
			if ( (**it).id == id ) {
				delete *it;
				it = g_xml.erase(it);
				found = true;

				if ( it == g_xml.end() ) { break; }
			}
		}

		if ( found ) {
			out.result_code = SAORIRESULT_NO_CONTENT;
		}
	}
	else if ( wcsicmp(in.args[0].c_str(),L"get") == 0 ) {
		//入力：get,ファイルID,XPath

		if ( in.args.size() < 3 ) {
			return;
		}

		//単純に探して見つけるだけ
		unsigned int id = wcstoul(in.args[1].c_str(),NULL,10);
		bool found = false;
		XMLPtrVector::iterator it = g_xml.begin();

		for ( ; it != g_xml.end() ; ++it ) {
			if ( (**it).id == id ) {
				break;
			}
		}

		if ( it == g_xml.end() ) {
			return;
		}

		std::string xpath = SAORI_FUNC::UnicodeToMultiByte(in.args[2],(**it).cp);
		try {
			TinyXPath::xpath_processor xpath((**it).xml->FirstChild(),xpath.c_str());
			TiXmlString str = xpath.S_compute_xpath();
			out.result = SAORI_FUNC::MultiByteToUnicode(str.c_str(),(**it).cp);
		}
		catch(...) {
			out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
		}

		out.result_code = SAORIRESULT_OK;
	}
}

