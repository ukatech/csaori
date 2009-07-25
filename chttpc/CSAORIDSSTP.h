#include "csaori.h"

class CSAORIDSSTP{
public:
	unsigned int codepage;
	HWND hwnd;
	string_t sender;
	string_t event;
	std::vector<string_t> refs;
	std::map<string_t,string_t> opts;

	string_t toString();
	void send(string_t &sstp);
};

