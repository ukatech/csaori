
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

			TinyXPath::expression_result r = xpath.er_compute_xpath();

			if ( r.e_type == TinyXPath::e_bool ) {
				out.result = r.o_get_bool() ? L"1" : L"0";
			}
			else if ( r.e_type == TinyXPath::e_string ) {
				TiXmlString str = r.S_get_string();
				out.result = SAORI_FUNC::MultiByteToUnicode(str.c_str(),(**it).cp);
			}
			else if ( r.e_type == TinyXPath::e_int ) {
				wchar_t s[32];
				swprintf(s,L"%d",r.i_get_int());
				out.result = s;
			}
			else if ( r.e_type == TinyXPath::e_double ) {
				wchar_t s[64];
				swprintf(s,L"%f",r.d_get_double());
				out.result = s;
			}
			else if ( r.e_type == TinyXPath::e_node_set ) {
				TinyXPath::node_set *np = r.nsp_get_node_set();
				if ( np ) {
					unsigned int n = np->u_get_nb_node_in_set();
					string_t wstr;

					if ( n == 1 && np->o_is_attrib(0) ) {
						const TiXmlAttribute *ap = np->XAp_get_attribute_in_set(0);
						if ( ap ) {
							wstr = SAORI_FUNC::MultiByteToUnicode(ap->Value(),(**it).cp);
							out.result = wstr;
							out.values.push_back(wstr);
						}
					}
					else {
						TiXmlString str;

						for ( unsigned int i = 0 ; i < n ; ++i ) {
							if ( ! np->o_is_attrib(i) ) {
								const TiXmlNode *xp = np->XNp_get_node_in_set(i);
								if ( xp ) {
									str.clear();
									const TiXmlText *tp = xp->ToText();
									if ( tp ) {
										str = tp->Value();
									}
									else {
										const TiXmlNode *cpl = xp->LastChild();
										const TiXmlNode *cp = xp->FirstChild();
										if ( cp && cpl && cp == cpl ) { //子がひとつだけ＋テキストノード
											tp = cp->ToText();
											if ( tp ) {
												str = tp->Value();
											}
										}
									}
									if ( str.length() ) {
										wstr = SAORI_FUNC::MultiByteToUnicode(str.c_str(),(**it).cp);
										out.result += wstr;
										out.values.push_back(wstr);
									}
								}
							}
						}
					}
				}
			}

		}
		catch(...) {
			out.result_code = SAORIRESULT_INTERNAL_SERVER_ERROR;
		}

		out.result_code = SAORIRESULT_OK;
	}
}

