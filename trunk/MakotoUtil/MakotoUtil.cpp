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
			out.result = SAORI_FUNC::intToString(GetAsyncKeyState(_wtol(in.args[1].c_str())) != 0);
		} else {
			out.result_code = SAORIRESULT_BAD_REQUEST;
		}
	}
	else if (in.args[0] == L"inputbox") {
		WIN32INPUTBOX_PARAM param;
		int flags = (in.args.size() >= 5) ? _wtol(in.args[4].c_str()) : 0;
		if(flags==2) // NUMERIC
		{
			definputbox_dlg[149] =32;
			definputbox_dlg[148] =128;
			param.bMultiline=false;
		}
		else if(flags==3) // PASSWORD
		{
			definputbox_dlg[149] =0;
			definputbox_dlg[148] =160;
			param.bMultiline=false;
		}
		else
		{
			definputbox_dlg[149] =0;
			definputbox_dlg[148] =128;
			param.bMultiline= (flags==1) ? true : false;  // MULTILINE
		}

		param.hInstance = (HINSTANCE)module_handle;
		param.szPrompt = (in.args.size() >= 2) ? in.args[1].c_str() : _T("Prompt:");
		param.szTitle = (in.args.size() >= 3) ? in.args[2].c_str() : _T("MakotoUtil");
		TCHAR buf[MAX_PATH] = _T("");
		if (in.args.size() >= 4) {
			_tcscat(buf,in.args[3].c_str());
		}
		param.szResult = buf;
		param.nResultSize = MAX_PATH;
		param.DlgTemplateData = definputbox_dlg;

		CWin32InputBox::InputBoxEx(&param);

		string_t txt = buf;
		txt = replaceAll(txt,L"\r\n",L"\2");

		out.result_code = SAORIRESULT_OK;
		out.result = txt;
//		out.result = SAORI_FUNC::intToString(CWin32InputBox::InputBoxEx(&param));
	}
	else if (in.args[0] == L"messagebox") {
		if (in.args.size() >= 2) {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(MessageBox(0,in.args[1].c_str(),(in.args.size() >= 3) ? in.args[2].c_str() : _T("MakotoUtil"),(in.args.size() >= 4) ? _wtol(in.args[3].c_str()) : MB_OK));
		} else {
			out.result_code = SAORIRESULT_BAD_REQUEST;
		}
	}
	else {
		out.result_code = SAORIRESULT_BAD_REQUEST;
	}
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

