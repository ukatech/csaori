#ifndef __CHARSET_CONVERTER_H__
#define __CHARSET_CONVERTER_H__

#include <iconv.h>
#include <string>
#include <stdexcept>

namespace htmlcxx
{
	class CharsetConverter
	{
		public:
			class Exception : public std::runtime_error
			{
				public:
					Exception(const std::string &arg)
						: std::runtime_error(arg) {}
			};
			
			CharsetConverter(const std::string &from, const std::string &to) throw (Exception);
			~CharsetConverter();
			
			std::string convert(const std::string &input);

		private:
			iconv_t mIconvDescriptor;;
	};
}

#endif
