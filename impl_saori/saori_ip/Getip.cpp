#include <winsock.h>
#include <wchar.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "urlmon.lib")

using namespace std;

wstring string2wstring(string str) {
  wstring result;
  //获取缓冲区大小，并申请空间，缓冲区大小按字符计算
  int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
  WCHAR* buffer = new WCHAR[len + 1];
  //多字节编码转换成宽字节编码
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
  buffer[len] = '\0';  //添加字符串结尾
  //删除缓冲区并返回值
  result=buffer;
  delete[] buffer;
  return result;
}  

int GetLocalHostname(std::basic_string<wchar_t>& local_name){
	WSADATA wsaData = {0};
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
		return 0;  // WSAGetLastError();
	char szHostName[MAX_PATH] = {0};
	int nRetCode;
	nRetCode = gethostname(szHostName, sizeof(szHostName));
	if (nRetCode != 0) return 0;  // WSAGetLastError();
	local_name = string2wstring(szHostName);
	WSACleanup();
	return 1;
}
int GetLocalIP(std::basic_string<wchar_t>& local_ip) {
	WSADATA wsaData = {0};
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
		return 0; // WSAGetLastError();
	char szHostName[MAX_PATH] = {0};
	int nRetCode;
	nRetCode = gethostname(szHostName, sizeof(szHostName));
	PHOSTENT hostinfo;
	if (nRetCode != 0) return 0; // WSAGetLastError();
	hostinfo = gethostbyname(szHostName);
	local_ip = string2wstring(inet_ntoa(*(struct in_addr*)*hostinfo->h_addr_list));
	WSACleanup();
	return 1;
}
int GetInternetIP(std::basic_string<wchar_t>& Inernet_ip) {
	TCHAR szTempPath[_MAX_PATH] = {0}, szTempFile[MAX_PATH] = {0};
	std::string buffer;
	GetTempPath(MAX_PATH, szTempPath);
	UINT nResult = GetTempFileName(szTempPath, "ip", 0, szTempFile);
	int aret = URLDownloadToFile(NULL, "http://www.ipchicken.com/", szTempFile,
									   BINDF_GETNEWESTVERSION, NULL);
	if (aret == S_FALSE) return 0;
	FILE* fp;
	if (fopen_s(&fp, szTempFile, "rb") != 0) {
		return 0;
	}
	fseek(fp, 0, SEEK_END);	//摼摓暥審戝彫
	int ilength = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (ilength > 0) {
		buffer.resize(ilength);
		fread(&buffer[0], sizeof(TCHAR), ilength, fp);
		fclose(fp);
		DeleteFile(szTempFile);
		char* str_ip = strstr(&buffer[0], "Address:");
		if (str_ip != NULL) {
			char tmp[32]{};
			sscanf_s(str_ip + sizeof("Address:"), "%[^ ]", &tmp[0], 32);
			Inernet_ip = string2wstring(&tmp[0]);
		}
		return 1;
	} else {
		fclose(fp);
		DeleteFile(szTempFile);
		return 0;
	}
}