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

#define MAX_VALUE_LENGTH 32768

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
	void Load(std::istream &f);

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
		m_element.push_back(new CSharedValueElement(name,value));
		return true;
	}
	else {
		m_element[idx]->m_value = value;
		return false;
	}
}

void CSharedValueGhost::Save(std::ostream &f)
{
	size_t n = m_element.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		f << "-**" << SAORI_FUNC::UnicodeToMultiByte(m_element[i]->m_name,CP_UTF8) << std::endl;
		f << "--*" << SAORI_FUNC::UnicodeToMultiByte(m_element[i]->m_value,CP_UTF8) << std::endl;
	}
}

void CSharedValueGhost::Load(std::istream &f)
{
	char buf[MAX_VALUE_LENGTH+5];
	buf[MAX_VALUE_LENGTH+4] = 0;

	string_t name;
	string_t value;

	while ( true ) {
		f.getline(buf,MAX_VALUE_LENGTH+4);
		if ( f.fail() ) {
			break;
		}

		if ( strncmp(buf,"-**",3) == 0 ) {
			if ( name == L"" ) {
				name = SAORI_FUNC::MultiByteToUnicode(buf+3,CP_UTF8);
			}
			else {
				m_element.push_back(new CSharedValueElement(name,L""));
				name = L"";
			}
		}
		else if ( strncmp(buf,"--*",3) == 0 ) {
			value = SAORI_FUNC::MultiByteToUnicode(buf+3,CP_UTF8);

			m_element.push_back(new CSharedValueElement(name,value));

			name = L"";
			value = L"";
		}
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
	strm.open(checkAndModifyPath("values.txt").c_str());

	char buf[MAX_VALUE_LENGTH+5];
	buf[MAX_VALUE_LENGTH+4] = 0;

	CSharedValueGhost *pGhost = NULL;

	while ( true ) {
		strm.getline(buf,sizeof(buf));
		if ( strm.fail() ) {
			break;
		}

		if ( strncmp(buf,"***",3) == 0 ) {
			pGhost = new CSharedValueGhost(SAORI_FUNC::MultiByteToUnicode(buf+3,CP_UTF8));
		}
		else {
			if ( pGhost ) {
				pGhost->Load(strm);
			}
		}
	}

	strm.close();

	return true;
}

bool CSharedValue::unload()
{
	std::ofstream strm;
	strm.open(checkAndModifyPath("values.txt").c_str());

	size_t n = m_ghost_values.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		strm << "***" << SAORI_FUNC::UnicodeToMultiByte(m_ghost_values[i]->GetGhost(),CP_UTF8) << std::endl;
		m_ghost_values[i]->Save(strm);

		delete m_ghost_values[i];
	}
	m_ghost_values.clear();

	strm.close();

	return true;
}

void CSharedValue::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSharedValueWrite") == 0 ) {
		if ( in.args.size() >= 2 ) {
			CSharedValueGhost *pG = FindGhost(sender);
			if ( pG ) {
				pG->Add(in.args[0],in.args[1]);
			}
		}
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSharedValueRead") == 0 ) {
		if ( in.args.size() >= 3 ) {
			CSharedValueGhost *pG = FindGhost(in.args[0]);
			if ( pG ) {
			}
		}
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

