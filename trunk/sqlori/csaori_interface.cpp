#include "csaori.h"
#include "sqlite3.h"
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
		res = sql->open(in, module_path);
	}
	else if (in.args[0] == L"EXEC") {
		res = sql->exec(in);
	}
	else if (in.args[0] == L"CLOSE") {
		res = sql->close();
	}

	out.result_code = SAORIRESULT_OK;
	out.result = SAORI_FUNC::intToString(res) + L"\1" + SAORI_FUNC::MultiByteToUnicode(sql->result(), CP_UTF8);
	
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
	return true;
}

