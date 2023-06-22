#include <elc/bignum>
#include <elc/numerical_representation>
#include <elc/string>
#include <string>

using namespace elc;
using std::wstring;

string to_elc_string(const wstring&str) {
	size_t size = elc::char_set::get_utf16_to_utf32_size((const char16_t*)str.c_str(),str.size());
	string ret(size);
	elc::char_set::utf16_to_utf32(ret.writeable_data(), (const char16_t*)str.c_str(), str.size());
	return ret;
}
wstring to_wstring(const string& str) {
	size_t size = elc::char_set::get_utf32_to_utf16_size((const char32_t*)str.c_str(),str.size());
	wstring ret(size, '\0');
	elc::char_set::utf32_to_utf16((char16_t*)ret.data(), (const char32_t*)str.c_str(), str.size());
	return ret;
}

bigfloat from_string(const wstring& str) {
	return from_string_get<bigfloat>(to_elc_string(str));
}
wstring to_wstring(const bigfloat& num) {
	return to_wstring(to_string(num));
}

std::wstring bignum_cacl(std::wstring a, std::wstring b, wchar_t opt) {
	bigfloat a1 = from_string(a);
	bigfloat b1 = from_string(b);
	bigfloat c;
	switch (opt) {
	case L'+':
		c = a1 + b1;
		break;
	case L'-':
		c = a1 - b1;
		break;
	case L'*':
		c = a1 * b1;
		break;
	case L'/':
		c = a1 / b1;
		break;
	default:
		return L"";
	}
	return to_wstring(c);
}