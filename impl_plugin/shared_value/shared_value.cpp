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
#include <stdio.h>

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
#define BUFFER_ENCODE ((MAX_VALUE_LENGTH+HEADER_LENGTH)*2)
#define BUFFER_DECODE (MAX_VALUE_LENGTH+HEADER_LENGTH)
#define FILE_NAME L"values.dat"

void EncodeBuffer2(const char *buf_from,char *buf_to)
{
	const unsigned char *buf_from2 = reinterpret_cast<const unsigned char*>(buf_from);

	unsigned int c;
	char asc[3];

	while ( *buf_from2 ) {
		c = *buf_from2;
		c = c ^ 0xFFU;

		sprintf(asc,"%02x",c);

		buf_to[0] = asc[0];
		buf_to[1] = asc[1];

		buf_from2 += 1;
		buf_to += 2;
	}

	*buf_to = 0;
}

void DecodeBuffer1(const char *buf_from,char *buf_to)
{
	const unsigned char *buf_from2 = reinterpret_cast<const unsigned char*>(buf_from);
	unsigned char *buf_to2 = reinterpret_cast<unsigned char*>(buf_to);

	unsigned int c;
	while ( *buf_from2 ) {
		c = *buf_from2;

		if ( c == 0xEE ) { //0x11 XOR 0xFF
			c = 0xF5; //0x0a XOR 0xFF
		}
		else if ( c == 0xED ) { //0x12 XOR 0xFF
			c = 0xF2; //0x0d XOR 0xFF
		}
		else {
			c = c ^ 0xFFU;
		}

		*buf_to2 = static_cast<unsigned char>(c);
		
		buf_from2 += 1;
		buf_to2 += 1;
	}

	*buf_to2 = 0;
}

void DecodeBuffer2(const char *buf_from,char *buf_to)
{
	unsigned char *buf_to2 = reinterpret_cast<unsigned char*>(buf_to);

	unsigned int c;
	char asc[3];
	asc[2] = 0;

	while ( buf_from[0] && buf_from[1] ) {
		asc[0] = buf_from[0];
		asc[1] = buf_from[1];

		c = strtoul(asc,NULL,16);
		c = c ^ 0xFFU;

		*buf_to2 = static_cast<unsigned char>(c);

		buf_from += 2;
		buf_to2 += 1;
	}

	*buf_to2 = 0;
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

	void Save(FILE *f);

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

void CSharedValueGhost::Save(FILE *f)
{
	size_t n = m_element.size();
	if ( n == 0 ) {
		return;
	}

	std::string save;

	char buf_decode[BUFFER_DECODE+1];
	buf_decode[BUFFER_DECODE] = 0;

	char buf_encode[BUFFER_ENCODE+1];
	buf_encode[BUFFER_ENCODE] = 0;

	save =  "GHOST*";
	save += SAORI_FUNC::UnicodeToMultiByte(GetGhost(),CP_UTF8);

	strncpy(buf_decode,save.c_str(),BUFFER_DECODE);
	EncodeBuffer2(buf_decode,buf_encode);
	fprintf(f,"%s\n",buf_encode);

	for ( size_t i = 0 ; i < n ; ++i ) {
		save =  "NAME_*";
		save += SAORI_FUNC::UnicodeToMultiByte(m_element[i]->m_name,CP_UTF8);

		strncpy(buf_decode,save.c_str(),BUFFER_DECODE);
		EncodeBuffer2(buf_decode,buf_encode);
		fprintf(f,"%s\n",buf_encode);

		save =  "VALUE*";
		save += SAORI_FUNC::UnicodeToMultiByte(m_element[i]->m_value,CP_UTF8);

		strncpy(buf_decode,save.c_str(),BUFFER_DECODE);
		EncodeBuffer2(buf_decode,buf_encode);
		fprintf(f,"%s\n",buf_encode);
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
	m_last_time = 0;
}

CSharedValue::~CSharedValue()
{
}

/*---------------------------------------------------------------
	初期化(DllMainとは別)
---------------------------------------------------------------*/
bool CSharedValue::load()
{
	m_last_time = time(NULL);

	FILE *fp = _wfopen(checkAndModifyPathW(FILE_NAME).c_str(),L"r");
	if ( ! fp ) {
		return false;
	}

	char buf_decode[BUFFER_DECODE+1];
	buf_decode[BUFFER_DECODE] = 0;
	char buf_encode[BUFFER_ENCODE+1];
	buf_encode[BUFFER_ENCODE] = 0;

	if ( ! fgets(buf_encode,BUFFER_ENCODE,fp) ) {
		fclose(fp);
		return false;
	}

	//fgetsの末尾改行を除去
	{
		size_t len = strlen(buf_encode);
		while ( len > 0 && (buf_encode[len-1] == '\n' || buf_encode[len-1] == '\r') ) {
			buf_encode[--len] = 0;
		}
	}

	int version = 0;

	if ( strcmp(buf_encode,"SVD2") == 0 ) {
		version = 2;
	}
	else if ( strcmp(buf_encode,"SVD1") == 0 ) {
		version = 1;
	}
	else {
		fclose(fp);
		return false;
	}

	CSharedValueGhost *pGhost = NULL;
	string_t name;
	string_t value;

	while ( fgets(buf_encode,sizeof(buf_encode),fp) ) {
		//fgetsの末尾改行を除去
		{
			size_t len = strlen(buf_encode);
			while ( len > 0 && (buf_encode[len-1] == '\n' || buf_encode[len-1] == '\r') ) {
				buf_encode[--len] = 0;
			}
		}

		if ( version == 1 ) {
			DecodeBuffer1(buf_encode,buf_decode);
		}
		else {
			DecodeBuffer2(buf_encode,buf_decode);
		}

		if ( strncmp(buf_decode,"GHOST*",HEADER_LENGTH) == 0 ) {
			string_t gname = SAORI_FUNC::MultiByteToUnicode(buf_decode+HEADER_LENGTH,CP_UTF8);
			pGhost = FindGhost(gname);
			if ( ! pGhost ) {
				pGhost = new CSharedValueGhost(gname);
				m_ghost_values.push_back(pGhost);
			}
		}
		else if ( strncmp(buf_decode,"NAME_*",HEADER_LENGTH) == 0 ) {
			if ( pGhost && name.size() ) {
				pGhost->Add(name,L"");
				name = L"";
			}
			name = SAORI_FUNC::MultiByteToUnicode(buf_decode+HEADER_LENGTH,CP_UTF8);
		}
		else if ( strncmp(buf_decode,"VALUE*",HEADER_LENGTH) == 0 ) {
			if ( pGhost && name.size() ) {
				value = SAORI_FUNC::MultiByteToUnicode(buf_decode+HEADER_LENGTH,CP_UTF8);
				pGhost->Add(name,value);
				name = L"";
				value = L"";
			}
		}
	}

	fclose(fp);

	return true;
}

void CSharedValue::Save(void)
{
	FILE *fp = _wfopen(checkAndModifyPathW(FILE_NAME).c_str(),L"w");
	if ( ! fp ) {
		return;
	}

	fprintf(fp,"SVD2\n");
	size_t n = m_ghost_values.size();

	for ( size_t i = 0 ; i < n ; ++i ) {
		m_ghost_values[i]->Save(fp);
	}

	fclose(fp);
}


bool CSharedValue::unload(void)
{
	Save();

	size_t n = m_ghost_values.size();
	for ( size_t i = 0 ; i < n ; ++i ) {
		delete m_ghost_values[i];
	}
	m_ghost_values.clear();

	return true;
}

void CSharedValue::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	out.result_code = SAORIRESULT_BAD_REQUEST;

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSecondChange") == 0 ) {
		time_t t = time(NULL);

		if ( (t - m_last_time) >= 55 ) { //60ちょっとマイナス
			Save();
			m_last_time = t;
		}

		return;
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnGhostBoot") == 0 ) {
		if ( in.args.size() >= 2 ) {
			out.result_code = SAORIRESULT_NO_CONTENT;
			CSharedValueGhost *pG = FindGhost(in.args[1]);
			if ( pG ) {
				out.result_code = SAORIRESULT_OK;
				event = L"OnSharedValueReadNotify";
				event_option = L"notify";

				out.values.push_back(in.args[1]);
				pG->PushBack(out.values);
			}
		}
		return;
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSharedValueWrite") == 0 ) {
		if ( in.args.size() >= 2 ) {
			out.result_code = SAORIRESULT_NO_CONTENT;

			CSharedValueGhost *pG = FindGhost(sender);
			if ( ! pG ) {
				pG = new CSharedValueGhost(sender);
				m_ghost_values.push_back(pG);
			}

			size_t m = in.args.size();

			for ( size_t j = 0 ; j + 1 < m ; j+=2 ) {
				pG->Add(in.args[j],in.args[j+1]);
			}
		}
		return;
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSharedValueWriteEx") == 0 ) {
		if ( in.args.size() >= 3 ) {
			out.result_code = SAORIRESULT_NO_CONTENT;

			CSharedValueGhost *pG = FindGhost(in.args[0]);
			if ( ! pG ) {
				pG = new CSharedValueGhost(in.args[0]);
				m_ghost_values.push_back(pG);
			}

			size_t m = in.args.size();

			for ( size_t j = 1 ; j + 1 < m ; j+=2 ) {
				pG->Add(in.args[j],in.args[j+1]);
			}
		}
		return;
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSharedValueRead") == 0 ) {
		if ( in.args.size() >= 2 ) {
			event = L"OnSharedValueRead";
			out.values.push_back(in.args[0]);
			out.result_code = SAORIRESULT_OK;

			CSharedValueGhost *pG = FindGhost(in.args[0]);

			if ( wcsicmp(in.args[1].c_str(),L"__SYSTEM_ALL_DATA__") == 0 ) {
				if ( pG ) {
					pG->PushBack(out.values);
				}
			}
			else {
				string_t v;
				size_t m = in.args.size();

				for ( size_t j = 1 ; j < m ; ++j ) {
					v = L"";
					if ( pG ) {
						v = pG->Get(in.args[j]);
					}
					out.values.push_back(in.args[j]);
					out.values.push_back(v);
				}
			}
		}
		return;
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"property.get") == 0 || wcsicmp(in.id.c_str(),L"OnGetProperty") == 0 ) {
		if ( in.args.size() >= 1 ) {
			// ghost(ゴースト名).key(キー名) をパース
			const string_t &prop = in.args[0];
			string_t ghost_name;
			string_t key_name;

			// "ghost(" で始まるか確認
			if ( wcsnicmp(prop.c_str(), L"ghost(", 6) == 0 ) {
				size_t ghost_end = prop.find(L").", 6);
				if ( ghost_end != string_t::npos ) {
					ghost_name = prop.substr(6, ghost_end - 6);
					// ").key(" の後
					size_t key_start = ghost_end + 2;
					if ( wcsnicmp(prop.c_str() + key_start, L"key(", 4) == 0 ) {
						key_start += 4;
						size_t key_end = prop.find(L")", key_start);
						if ( key_end != string_t::npos ) {
							key_name = prop.substr(key_start, key_end - key_start);
						}
					}
				}
			}

			if ( ghost_name.size() > 0 && key_name.size() > 0 ) {
				CSharedValueGhost *pG = FindGhost(ghost_name);
				if ( pG ) {
					const string_t &v = pG->Get(key_name);
					if ( v.size() > 0 ) {
						out.result = v;
						out.result_code = SAORIRESULT_OK;
					}
					else {
						out.result_code = SAORIRESULT_NO_CONTENT;
					}
				}
				else {
					out.result_code = SAORIRESULT_NO_CONTENT;
				}
			}
		}
		return;
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"property.set") == 0 || wcsicmp(in.id.c_str(),L"OnSetProperty") == 0 ) {
		if ( in.args.size() >= 2 ) {
			// ghost(ゴースト名).key(キー名) をパース
			const string_t &prop = in.args[0];
			string_t ghost_name;
			string_t key_name;

			// "ghost(" で始まるか確認
			if ( wcsnicmp(prop.c_str(), L"ghost(", 6) == 0 ) {
				size_t ghost_end = prop.find(L").", 6);
				if ( ghost_end != string_t::npos ) {
					ghost_name = prop.substr(6, ghost_end - 6);
					// ").key(" の後
					size_t key_start = ghost_end + 2;
					if ( wcsnicmp(prop.c_str() + key_start, L"key(", 4) == 0 ) {
						key_start += 4;
						size_t key_end = prop.find(L")", key_start);
						if ( key_end != string_t::npos ) {
							key_name = prop.substr(key_start, key_end - key_start);
						}
					}
				}
			}

			if ( ghost_name.size() > 0 && key_name.size() > 0 ) {
				CSharedValueGhost *pG = FindGhost(ghost_name);
				if ( ! pG ) {
					pG = new CSharedValueGhost(ghost_name);
					m_ghost_values.push_back(pG);
				}
				pG->Add(key_name, in.args[1]);
				out.result_code = SAORIRESULT_NO_CONTENT;
			}
		}
		return;
	}

	//--------------------------------------------------------
	if ( wcsicmp(in.id.c_str(),L"OnSharedValueGhostList") == 0 ) {
		event = L"OnSharedValueGhostList";
		out.result_code = SAORIRESULT_OK;

		size_t n = m_ghost_values.size();
		for ( size_t i = 0 ; i < n ; ++i ) {
			out.values.push_back(m_ghost_values[i]->GetGhost());
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

