#include <elc/bignum>
#include <elc/numerical_representation>
#include <elc/string>
#include <string>
#include <elc/code_realm>

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

bigfloat from_string(const wstring& str,auto& convert_state) {
	return from_string_get<bigfloat>(to_elc_string(str),convert_state);
}
wstring to_wstring(const bigfloat& num) {
	return to_wstring(to_string(num));
}

std::wstring bignum_cacl(std::wstring opt_b,std::wstring a, std::wstring b) {
	decltype(from_string_get<bigfloat>)::state_t convert_state{};
	bigfloat a1 = from_string(a,convert_state);
	if(convert_state.error())
		return{};
	convert_state.reset();
	bigfloat b1 = from_string(b,convert_state);
	if(convert_state.error())
		return{};
	bigfloat c;
	auto opt=to_elc_string(opt_b);

	if(opt==es"+")
		c=a1+b1;
	elseif(opt==es"-")
		c=a1-b1;
	elseif(opt==es"*")
		c=a1*b1;
	elseif(opt==es"/")
		c=a1/b1;
	elseif(opt==es"pi"){
		if(is_negative(a1))
			return {};
		c=pi_with_epsilon(abs(a1));
	}
	else{
		if(is_negative(b1))
			return {};
		if(opt==es"sin")
			c=sin(a1,abs(b1));
		elseif(opt==es"cos")
			c=cos(a1,abs(b1));
		elseif(opt==es"tan")
			c=tan(a1,abs(b1));
		elseif(opt==es"arccos")
			c=arccos(a1,abs(b1));
		elseif(opt==es"arcsin")
			c=arcsin(a1,abs(b1));
		elseif(opt==es"arctan")
			c=arctan(a1,abs(b1));
		elseif(opt==es"cot")
			c=cot(a1,abs(b1));
		elseif(opt==es"sec")
			c=sec(a1,abs(b1));
		elseif(opt==es"csc")
			c=csc(a1,abs(b1));
		elseif(opt==es"arccot")
			c=arccot(a1,abs(b1));
		elseif(opt==es"arcsec")
			c=arcsec(a1,abs(b1));
		elseif(opt==es"arccsc")
			c=arccsc(a1,abs(b1));
		elseif(opt==es"sqrt")
			c=sqrt(a1,abs(b1));
		else
			return{};
	}
	return to_wstring(c);
}