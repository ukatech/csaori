// modified from http://evotalk.net/blog/2007/02/strip-html-tag.html
#include <string>

using namespace std;

wstring replaceAll(wstring s, wstring f, wstring r) {
	unsigned int found = 0;
	while((found = s.find(f, found)) != string::npos) {
		s.replace(found, f.length(), r);
		found += r.size();
	}
	return s;
}

wstring overReplaceAll(wstring s, wstring f, wstring r) {
	unsigned int found = s.find(f);
	while(found != string::npos) {
		s.replace(found, f.length(), r);
		found = s.find(f);
	}
	return s;
}

wstring stripHTMLTags(wstring s) {
	while(true) {
		unsigned int left = s.find(L"<");
		unsigned int right = s.find(L">");
		if(left==string::npos || right==string::npos)
			break;
		s = s.erase(left, right - left + 1);
	}
	s = replaceAll(s, L"&lt;", L"<");
	s = replaceAll(s, L"&gt;", L">");
	s = replaceAll(s, L"&amp;", L"\\&");
	s = replaceAll(s, L"&apos;", L"\\'");
	s = replaceAll(s, L"&quot;", L"\\\"");
	s = replaceAll(s, L"&nbsp;", L" ");
	s = replaceAll(s, L"&raquo;", L">>");
	s = replaceAll(s, L"&copy;", L"(c)");
	// Etc...
	return s;
}

string replaceAll(string s, string f, string r) {
	unsigned int found = 0;
	while((found = s.find(f, found)) != string::npos) {
		s.replace(found, f.length(), r);
		found += r.size();
	}
	return s;
}

string overReplaceAll(string s, string f, string r) {
	unsigned int found = s.find(f);
	while(found != string::npos) {
		s.replace(found, f.length(), r);
		found = s.find(f);
	}
	return s;
}

string stripHTMLTags(string s) {
	while(true) {
		unsigned int left = s.find("<");
		unsigned int right = s.find(">");
		if(left==string::npos || right==string::npos)
			break;
		s = s.erase(left, right - left + 1);
	}
	s = replaceAll(s, "&lt;", "<");
	s = replaceAll(s, "&gt;", ">");
	s = replaceAll(s, "&amp;", "\\&");
	s = replaceAll(s, "&apos;", "\\'");
	s = replaceAll(s, "&quot;", "\\\"");
	s = replaceAll(s, "&nbsp;", " ");
	s = replaceAll(s, "&raquo;", ">>");
	s = replaceAll(s, "&copy;", "(c)");
	// Etc...
	return s;
}
