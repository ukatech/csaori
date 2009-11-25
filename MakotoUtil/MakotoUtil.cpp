#include "csaori.h"
#include "Win32InputBox.h"
#include "resource.h"

string_t replaceAll(string_t s, string_t f, string_t r) {
	unsigned int found = 0;
	while((found = s.find(f, found)) != std::string::npos) {
		s.replace(found, f.length(), r);
		found += r.size();
	}
	return s;
}

//------------------------------------------------------------------------------
//CSAORI
//------------------------------------------------------------------------------

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	if (in.args.size() <= 0) {
		out.result_code = SAORIRESULT_NO_CONTENT;
		return;
	}

	if (in.args[0] == L"keydown") {
		if (in.args.size() == 2) {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString((GetAsyncKeyState(_wtol(in.args[1].c_str())) & 0x8000) == 0x8000);
		}
	}
	else if (in.args[0] == L"inputbox") {
		WIN32INPUTBOX_PARAM param;

		param.hInstance = (HINSTANCE)module_handle;
		param.szPrompt = (in.args.size() >= 2) ? in.args[1].c_str() : _T("Prompt:");
		param.szTitle = (in.args.size() >= 3) ? in.args[2].c_str() : _T("MakotoUtil");
		TCHAR buf[MAX_PATH] = _T("");
		if (in.args.size() >= 4) {
			_tcscat(buf,in.args[3].c_str());
		}
		param.szResult = buf;
		param.nResultSize = MAX_PATH;
		param.DlgTemplateName = MAKEINTRESOURCE(IDD_INPUTBOX);
		param.bMultiline = (in.args.size() >= 5) ? _wtol(in.args[4].c_str()) :false;
		CWin32InputBox::InputBoxEx(&param);

		string_t txt = buf;
		txt = replaceAll(txt,L"\r\n",L"\2");

		out.result_code = SAORIRESULT_OK;
		out.result = txt;
	}
	
//	out.result = L"Charset = " + SAORI_FUNC::CHARSETtoString(in.charset);
//	out.charset = CHARSET_UTF_8;
}

bool CSAORI::load()
{
	// Environment's default locale
	setlocale( LC_ALL, "");

	return true;
}

bool CSAORI::unload()
{
	return true;
}

