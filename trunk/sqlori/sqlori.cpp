#include "csaori.h"
#include "sqlite3.h"
#include "sqlori.h"

int Sqlori::open(const std::string &path)
{
	int res = 0;
	char *errMsg = NULL;

	if (close())
		return SQLITE_ERROR;

	res = sqlite3_open(path.c_str(), &_db);

	if (res != SQLITE_OK) {
		_result = sqlite3_errmsg(_db);
		sqlite3_close(_db);
		_db = NULL;
		return SQLITE_ERROR;
	}

	return SQLITE_OK;
}

int Sqlori::exec(const std::string &command)
{
	int res = 0;
	char *errMsg = NULL;

	if (_db) {
		res = sqlite3_exec(_db, command.c_str(), &callBack, this, &errMsg);
		if (res != SQLITE_OK) {
			_result = errMsg;
			sqlite3_free(errMsg);
		}
		return SQLITE_OK;
	}
	else {
		_result = "no database connection";
		return SQLITE_ERROR;
	}
}

int Sqlori::close()
{
	int res = sqlite3_close(_db);

	if (res != SQLITE_OK) {
		_result = sqlite3_errmsg(_db);
		return SQLITE_ERROR;
	}
	_db = NULL;
	return SQLITE_OK;
}

int Sqlori::callBack(void *callee, int size, char **values, char **fields)
{
	std::string &result = ((Sqlori *)callee)->_result;

	if (result.empty()) {
		result += fields[0];
		for (int i = 1; i < size; i++) {
			result += "\3";
			result += fields[i];
		}
		result += "\2";

		result += values[0];
		for (int i = 1; i < size; i++) {
			result += "\3";
			result += values[i];
		}
	}
	else {
		for (int i = 0; i < size; i++) {
			result += "\3";
			result += values[i];
		}
	}

	return SQLITE_OK;
}

