#pragma once

#include <string>

struct sqlite3;
class CSAORIInput;
typedef std::basic_string<wchar_t> string_t;

class Sqlori {
private:
	sqlite3 *_db;
	std::string _result;

public:
	Sqlori(): _db(NULL) {}
	const std::string &result() { return _result; }

	static int callBack(void *argv, int size, char **values, char **fields);

	//std::string tableToString(char **fields, char **values, int col, int row = 0);
	void clear() { _result = ""; }

	int open(const CSAORIInput& in, const string_t &path);
	int exec(const CSAORIInput& in);
	int close();
};
