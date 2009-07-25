#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>

#include <string>
#include <vector>
#include <map>

using namespace std;

#define CIH_OK 0
#define CIH_FAIL 1

class CInetHelper{
public:
	static int getUrlContent(const char* url, wstring& out);
	static bool checkInternet();
private:
	static HINTERNET getInternet();
};

