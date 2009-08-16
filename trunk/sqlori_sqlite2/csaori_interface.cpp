#include "csaori.h"
#include "sqlite.h"
#include "sqlori.h"

static Sqlori *sql;
//------------------------------------------------------------------------------
//CSAORI
//------------------------------------------------------------------------------

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	int res = SQLITE_OK;
	sql->clear();

	if (in.args.size() <= 0) {
		out.result_code = SAORIRESULT_NO_CONTENT;
		return;
	}

	if (in.args[0] == L"OPEN") {
		if (in.args.size() == 1) {
			res = sql->open(SAORI_FUNC::UnicodeToMultiByte(module_path + L"sqlori.db", CP_ACP));
		}
		else {
			res = sql->open(SAORI_FUNC::UnicodeToMultiByte(module_path + in.args[1], CP_ACP));
		}
	}
	else if (in.args[0] == L"EXEC") {
		if (in.args.size() >= 2) {
			res = sql->exec(SAORI_FUNC::UnicodeToMultiByte(in.args[1], in.codepage));
		}
	}
	else if (in.args[0] == L"CLOSE") {
		res = sql->close();
	}
	else if (in.args[0] == L"SQLITE_VERSION") {
		out.result_code = SAORIRESULT_OK;
		out.result = SAORI_FUNC::intToString(SQLITE_OK) + L"\2Version\2" + SAORI_FUNC::MultiByteToUnicode(sqlite_libversion(), in.codepage);
		return;
	}

	out.result_code = SAORIRESULT_OK;
	out.result = SAORI_FUNC::intToString(res) + L"\2" + SAORI_FUNC::MultiByteToUnicode(sql->result(), in.codepage);
	
//	out.result = L"Charset = " + SAORI_FUNC::CHARSETtoString(in.charset);
//	out.charset = CHARSET_UTF_8;
}

bool CSAORI::load()
{
	// Environment's default locale
	setlocale( LC_ALL, "");

	sql = new Sqlori();
	return true;
}

bool CSAORI::unload()
{
	sql->close();
	delete sql;
	return true;
}

