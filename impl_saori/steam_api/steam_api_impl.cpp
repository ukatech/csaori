#include <string>
#include <windows.h>

std::wstring base_getSteamPath() {
	// Get the Steam path from the registry
	// 32-bit: HKEY_LOCAL_MACHINE\SOFTWARE\Valve\Steam
	// 64-bit: HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Valve\Steam
	HKEY	hKey;
	LSTATUS status;
	DWORD	type;
	status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\Valve\\Steam", 0, KEY_READ, &hKey);
	if(status != ERROR_SUCCESS) {
		status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Valve\\Steam", 0, KEY_READ, &hKey);
		if(status != ERROR_SUCCESS)
			return {};
	}
	std::wstring path;
	DWORD		 size = MAX_PATH * sizeof(wchar_t);
	path.resize(MAX_PATH);
	status = RegQueryValueExW(hKey, L"InstallPath", NULL, &type, (LPBYTE)path.data(), &size);
	if(status != ERROR_SUCCESS)
		return {};
	path.resize(wcslen(path.c_str()));
	RegCloseKey(hKey);
	//检查路径是否存在
	if(GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES)
		return {};
	return path;
}

std::wstring GetSteamPath() {
	static auto path = base_getSteamPath();
	return path;
}

std::string GetSteamMostRecentUserPersonaName() {
	using namespace std;
	auto file = GetSteamPath() + L"\\config\\loginusers.vdf";
	//read file as utf-8
	FILE* fp = _wfopen(file.c_str(), L"rb, ccs=UTF-8");
	if(!fp)
		return {};
	//反复readline
	string line;
	string name;
	while(true) {
		line.clear();
		char8_t c;
		while(true) {
			if(fread(&c, sizeof(c), 1, fp) != 1)
				goto end;
			if(c == '\n')
				break;
			line += c;
		}
		//移除前置tab
		line.erase(0, line.find_first_not_of("\t "sv));
		//解析
		if(line.starts_with("\"PersonaName\""sv)) {
			line.erase(0, 13);
			line.erase(0, line.find_first_not_of("\t "sv));
			name = line.substr(1, line.size() - 2);		  //去掉引号
		}
		else if(line.starts_with("\"MostRecent\""sv)) {
			//if is 1
			if(line.ends_with("\"1\""sv))
				break;
		}
	}
end:
	fclose(fp);
	return name;
}
