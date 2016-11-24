#include "csaori.h"

#include "mecab/sdk/mecab.h"

#define FORMAT_ARRAY_SIZE 5

/*---------------------------------------------------------
	定義
---------------------------------------------------------*/
class CSAORIKisaragi : public CSAORI
{
private:
	bool init_mecab(void);
	void tagger_error(CSAORIOutput &out);

	string_t m_dicDir;
	string_t m_outFormat;
	string_t m_userDic;
	HINSTANCE m_hMeCabDLL;
	
	string_t m_format[FORMAT_ARRAY_SIZE]; //node,unk,eos,bos,eon

	MeCab::Tagger *m_pTagger;

	void exec_type(const CSAORIInput& in,CSAORIOutput& out,string_t out_type);

public:
	CSAORIKisaragi() : m_dicDir(L"dic\\") , m_pTagger(NULL) , m_outFormat(L"chasen") , m_hMeCabDLL(NULL) { }
	virtual ~CSAORIKisaragi() { unload(); }

	virtual bool load(void);
	virtual bool unload(void);
	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
};

/*---------------------------------------------------------
	初期化
---------------------------------------------------------*/
bool CSAORIKisaragi::load()
{
	std::string dll_path = SAORI_FUNC::UnicodeToMultiByte(getModulePath() + L"libmecab.dll", CP_ACP);
	m_hMeCabDLL = ::LoadLibrary(dll_path.c_str());

	return true;
}

/*---------------------------------------------------------
	解放
---------------------------------------------------------*/
bool CSAORIKisaragi::unload()
{
	if ( m_pTagger ) {
		delete m_pTagger;
		m_pTagger = NULL;
	}

	if ( m_hMeCabDLL ) {
		::FreeLibrary(m_hMeCabDLL);
	}

	return true;
}

/*---------------------------------------------------------
	実行
---------------------------------------------------------*/
void CSAORIKisaragi::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	if ( in.args.size() ) {
		if ( in.args.size() >= 2 ) {
			if ( in.args[0] == L"parse" ) {
				exec_type(in,out,L"chasen");
				return;
			}
			if ( in.args[0] == L"parse-mecab" ) {
				exec_type(in,out,L"mecab");
				return;
			}
			if ( in.args[0] == L"wakati" ) {
				exec_type(in,out,L"wakati");
				return;
			}
			if ( in.args[0] == L"yomi" ) {
				exec_type(in,out,L"yomi");
				return;
			}
		}
	}

	//デフォルト動作
	out.result_code = SAORIRESULT_BAD_REQUEST;
}

void CSAORIKisaragi::exec_type(const CSAORIInput& in,CSAORIOutput& out,string_t out_type)
{
	if ( in.args.size() >= 3 ) {
		if ( m_format[0] != in.args[2] ) {
			m_format[0] = in.args[2];
			delete m_pTagger;
			m_pTagger = NULL;
		}
	}

	if ( m_outFormat != out_type ) {
		m_outFormat = out_type;
		delete m_pTagger;
		m_pTagger = NULL;
	}

	if ( ! init_mecab() ) {
		tagger_error(out);
		return;
	}

	std::string src = SAORI_FUNC::UnicodeToMultiByte(in.args[1],CP_SJIS);

	const char* pResult = m_pTagger->parse(src.c_str());
	if ( ! pResult ) {
		tagger_error(out);
		return;
	}

	string_t result = SAORI_FUNC::MultiByteToUnicode(pResult,CP_SJIS);
	string_t sl;
	string_t::size_type pos = 0;

	do {
		pos = SAORI_FUNC::getLine(sl, result, pos);
		out.values.push_back(sl);
	} while ( pos != string_t::npos );
	
	out.result = SAORI_FUNC::intToString(out.values.size());
	out.result_code = SAORIRESULT_OK;
	return;
}

bool CSAORIKisaragi::init_mecab(void)
{
	if ( m_pTagger ) { return true; }

	char* argv[32];
	int argc = 0;

	/////
	std::string arg_dir = "--dicdir=";
	string_t fullPath = checkAndModifyPathW(m_dicDir);
	arg_dir += SAORI_FUNC::UnicodeToMultiByte(fullPath.c_str(),CP_SJIS);

	argv[argc] = const_cast<char*>(arg_dir.c_str());
	argc += 1;

	/////
	const char* params[FORMAT_ARRAY_SIZE] = {
		"node","unk","eos","bos","eon"
	};

	bool is_format_found = false;

	std::string arg_format[FORMAT_ARRAY_SIZE];

	for ( size_t i = 0 ; i < FORMAT_ARRAY_SIZE ; ++i ) {
		if ( m_format[i].size() > 0 ) {
			arg_format[i] = "--";
			arg_format[i] += params[i];
			arg_format[i] += "-format=";
			arg_format[i] += SAORI_FUNC::UnicodeToMultiByte(m_format[i].c_str(),CP_SJIS);

			argv[argc] = const_cast<char*>(arg_format[i].c_str());
			argc += 1;

			is_format_found = true;
		}
	}

	/////
	std::string arg_out_format;

	if ( ! is_format_found ) {
		if ( m_outFormat.size() ) {
			if ( m_outFormat != L"mecab" ) {
				arg_out_format = "--output-format-type=";
				arg_out_format += SAORI_FUNC::UnicodeToMultiByte(m_outFormat.c_str(),CP_SJIS);

				argv[argc] = const_cast<char*>(arg_out_format.c_str());
				argc += 1;
			}
		}
	}

	/////
	std::string arg_userdic;
	
	if ( m_userDic.size() ) {
		arg_userdic = "--userdic=";
		fullPath = checkAndModifyPathW(m_userDic);
		arg_userdic += SAORI_FUNC::UnicodeToMultiByte(fullPath.c_str(),CP_SJIS);

		argv[argc] = const_cast<char*>(arg_userdic.c_str());
		argc += 1;
	}

	/////

	m_pTagger = MeCab::Tagger::create(argc,argv);
	if (! m_pTagger ) {
		return false;
	}
	return true;
}

void CSAORIKisaragi::tagger_error(CSAORIOutput &out)
{
	const char *e;
	if ( m_pTagger ) {
		e = m_pTagger->what();
	}
	else {
		e = MeCab::getTaggerError();
	}
	string_t err = SAORI_FUNC::MultiByteToUnicode(e,CP_SJIS);

	out.result = L"-1";
	out.result_code = SAORIRESULT_OK;
	out.values.push_back(err);
}

/*---------------------------------------------------------
	構築
---------------------------------------------------------*/
CSAORIBase* CreateInstance(void)
{
	return new CSAORIKisaragi();
}
