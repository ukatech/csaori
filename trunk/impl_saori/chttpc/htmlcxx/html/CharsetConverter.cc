#include <cstdio>
#include <cstring>
#include <cerrno>
#include "CharsetConverter.h"

using namespace std;
using namespace htmlcxx;

CharsetConverter::CharsetConverter(const string &from, const string &to) throw (Exception)
{
	mIconvDescriptor = iconv_open(from.c_str(), to.c_str());
	if (mIconvDescriptor == (iconv_t)(-1))
	{
		const char *error_str = strerror(errno);
		int size = strlen(error_str) + from.length() + to.length() + 26;
		char error[size];
		snprintf(error, size, "Can't convert from %s to %s: %s", from.c_str(), to.c_str(), error_str);
		throw Exception(error);
	}
}

CharsetConverter::~CharsetConverter()
{
	iconv_close(mIconvDescriptor);
}

string CharsetConverter::convert(const string &input)
{
	const char *inbuf = input.c_str();
	size_t inbytesleft = input.length();

	size_t outbuf_len = input.length();
	char *outbuf_start = new char[outbuf_len];
	char *outbuf = outbuf_start;
	size_t outbytesleft = outbuf_len;

	size_t ret;
	while (1) {
		ret = iconv(mIconvDescriptor, const_cast<char**>(&inbuf), &inbytesleft, &outbuf, &outbytesleft);
		if (ret == 0) break;
		if (ret == (size_t)-1 && errno == E2BIG) return string();

		//				fprintf(stderr, "invalid byte: %d\n", inbuf - input.c_str());

		inbuf++; inbytesleft--;
	}

	string out(outbuf_start, outbuf_len - outbytesleft);

	delete [] outbuf_start;

	return out;
}
