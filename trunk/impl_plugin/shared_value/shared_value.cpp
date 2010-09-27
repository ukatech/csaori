//setlocale//
#ifdef _MSC_VER
#if _MSC_VER >= 1400
#pragma setlocale("japanese")
#endif
#endif
//setlocale end//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <new>
#include <fstream>

#include "shared_value.h"

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

#define MAX_VALUE_LENGTH 32760
#define HEADER_LENGTH    6
#define FILE_NAME "values.dat"

void EncodeDecodeBuffer(char *buf)
{
	while ( *buf ) {
		*buf = static_cast<char>(static_cast<unsigned int>(*buf) ^ 0xFFU);
		++buf;
	}
}

/*===============================================================
	補助クラス定義
===============================================================*/

class CSharedValueGhost
{
private:
	std::vector<CSharedValueElement*> m_element;
	string_t m_ghost;

public:
	const int Find(const string_t& name) const;

	const string_t& Get(const string_t& name) const;
	bool Add(const string_t& name,const string_t& value);

	const string_t& GetGhost(void) const { return m_ghost; }
	void SetGhost(const string_t& n) { m_ghost = n; }

	void Save(std::ostream &f);

	void PushBack(std::vector<string_t> &vec);

	CSharedValueGhost(const string_t &n) : m_ghost(n) { }
	~CSharedValueGhost();
};

class CSharedValueElement
{
public:
	string_t m_name;
	string_t m_value;

	CSharedValueElement(const string_t &n,const string_t &v) : m_name(n) , m_value(v) { }
};

/*===============================================================
	補助クラス実装
===============================================================*/
const int CSharedValueGhost::Find(const string_t& name) const
{
	size_t n = m_element.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		if ( wcsicmp(m_element[i]->m_name.c_str(),name.c_str()) == 0 ) {
			return i;
		}
	}
	return -1;
}

const string_t& CSharedValueGhost::Get(const string_t& name) const
{
	static const string_t empty(L"");

	int idx = Find(name);
	if ( idx < 0 ) {
		return empty;
	}
	else {
		return m_element[idx]->m_value;
	}
}

bool CSharedValueGhost::Add(const string_t& name,const string_t& value)
{
	int idx = Find(name);
	if ( idx < 0 ) {
		if ( value.size() > 0 ) {
			m_element.push_back(new CSharedValueElement(name,value));
		}
		return true;
	}
	else {
		if ( value.size() > 0 ) {
			m_element[idx]->m_value = value;
		}
		else {
			delete m_element[idx];
			m_element.erase(m_element.begin()+idx);
		}
		return false;
	}
}

void CSharedValueGhost::Save(std::ostream &f)
{
	std::string save;

	char buf[MAX_VALUE_LENGTH+HEADER_LENGTH+1];
	buf[MAX_VALUE_LENGTH+HEADER_LENGTH+1] = 0;

	save =  "GHOST*";
	save += SAORI_FUNC::UnicodeToMultiByte(GetGhost(),CP_UTF8);

	strncpy(buf,save.c_str(),MAX_VALUE_LENGTH+HEADER_LENGTH);
	EncodeDecodeBuffer(buf);
	f << buf << std::endl;

	size_t n = m_element.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		save =  "NAME_*";
		save += SAORI_FUNC::UnicodeToMultiByte(m_element[i]->m_name,CP_UTF8);

		strncpy(buf,save.c_str(),MAX_VALUE_LENGTH+HEADER_LENGTH);
		EncodeDecodeBuffer(buf);
		f << buf << std::endl;

		save =  "VALUE*";
		save += SAORI_FUNC::UnicodeToMultiByte(m_element[i]->m_value,CP_UTF8);

		strncpy(buf,save.c_str(),MAX_VALUE_LENGTH+HEADER_LENGTH);
		EncodeDecodeBuffer(buf);
		f << buf << std::endl;
	}
}

void CSharedValueGhost::PushBack(std::vector<string_t> &vec)
{
	size_t n = m_element.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		vec.push_back(m_element[i]->m_name);
		vec.push_back(m_element[i]->m_value);
	}
}

CSharedValueGhost::~CSharedValueGhost()
{
	size_t n = m_element.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		delete m_element[i];
	}
	m_element.clear();
}

/*===============================================================
	制御クラス実装
===============================================================*/

/*---------------------------------------------------------------
	インスタンス作成（csaori_baseから呼ばれる）
---------------------------------------------------------------*/
CSAORIBase* CreateInstance(void)
{
	return new CSharedValue();
}

/*---------------------------------------------------------------
	初期化(DllMain縛り)
---------------------------------------------------------------*/
CSharedValue::CSharedValue(void)
{
}

CSharedValue::~CSharedValue()
{
}

/*---------------------------------------------------------------
	初期化(DllMainとは別)
---------------------------------------------------------------*/
bool CSharedValue::load()
{
	std::ifstream strm;
	strm.open(checkAndModifyPath(FILE_NAME).c_str());

	char buf[MAX_VALUE_LENGTH+HEADER_LENGTH+2];
	buf[MAX_VALUE_LENGTH+HEADER_LENGTH+1] = 0;

	strm.getline(buf,MAX_VALUE_LENGTH+HEADER_LENGTH);
	if ( strm.fail() ) {
		return false;
	}
	if ( strcmp(buf,"SVD1") != 0 ) {
		return false;
	}

	CSharedValueGhost *pGhost = NULL;
	string_t name;
	string_t value;

	while ( true ) {
		strm.getline(buf,sizeof(buf));
		if ( strm.fail() ) {
			break;
		}

		EncodeDecodeBuffer(buf);

		if ( strncmp(buf,"GHOST*",HEADER_LENGTH) == 0 ) {
			string_t gname = SAORI_FUNC::MultiByteToUnicode(buf+HEADER_LENGTH,CP_UTF8);
			pGhost = FindGhost(gname);
			if ( ! pGhost ) {
				pGhost = new CSharedValueGhost(gname);
				m_ghost_values.push_back(pGhost);
			}
		}
		else if ( strncmp(buf,"NAME_*",HEADER_LENGTH) == 0 ) {
			if ( pGhost && name.size() ) {
				pGhost->Add(name,L"");
				name = L"";
			}
			name = SAORI_FUNC::MultiByteToUnicode(buf+HEADER_LENGTH,CP_UTF8);
		}
		else if ( strncmp(buf,"VALUE*",HEADER_LENGTH) == 0 ) {
			if ( pGhost && name.size() ) {
				value = SAORI_FUNC::MultiByteToUnicode(buf+HEADER_LENGTH,CP_UTF8);
				pGhost->Add(name,value);
				name = L"";
				value = L"";
			}
		}
	}

	strm.close();

	return true;
}

bool CSharedValue::unload()
{
	std::ofstream strm;
	strm.open(checkAndModifyPath(FILE_NAME).c_str());

	strm << "SVD1" << std::endl;

	size_t n = m_ghost_values.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		m_ghost_values[i]->Save(strm);

		delete m_ghost_values[i];
	}
	m_ghost_values.clear();

	strm.close();

	return true;
}

void CSharedValue::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	out.result_code = SAORIRESULT_BAD_REQUEST;

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnGhostBoot") == 0 ) {
		if ( in.args.size() >= 2 ) {
			out.result_code = SAORIRESULT_NO_CONTENT;
			CSharedValueGhost *pG = FindGhost(in.args[1]);
			if ( pG ) {
				event = L"OnSharedValueReadNotify";
				event_option = L"notify";
				pG->PushBack(out.values);
			}
		}
		return;
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSharedValueWrite") == 0 ) {
		if ( in.args.size() >= 2 ) {
			CSharedValueGhost *pG = FindGhost(sender);
			if ( ! pG ) {
				pG = new CSharedValueGhost(sender);
				m_ghost_values.push_back(pG);
			}
			pG->Add(in.args[0],in.args[1]);
			out.result_code = SAORIRESULT_NO_CONTENT;
		}
		return;
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSharedValueRead") == 0 ) {
		if ( in.args.size() >= 2 ) {
			CSharedValueGhost *pG = FindGhost(in.args[0]);
			if ( pG ) {
				string_t v = pG->Get(in.args[1]);
				if ( v.size() ) {
					event = L"OnSharedValueReadComplete";
					out.values.push_back(in.args[0]);
					out.values.push_back(in.args[1]);
					out.values.push_back(v);
					out.result_code = SAORIRESULT_OK;
					return;
				}
			}

			event = L"OnSharedValueReadFailure";
			out.values.push_back(in.args[0]);
			out.values.push_back(in.args[1]);
			out.values.push_back(L"");
			out.result_code = SAORIRESULT_OK;
		}
		return;
	}

}


CSharedValueGhost *CSharedValue::FindGhost(const string_t &name)
{
	size_t n = m_ghost_values.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		if ( wcsicmp(m_ghost_values[i]->GetGhost().c_str(),name.c_str()) == 0 ) {
			return m_ghost_values[i];
		}
	}
	return NULL;
}

