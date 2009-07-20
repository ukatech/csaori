/*-------------------------------------------------------------
	シリアル通信ライブラリ
	Easy RS-232C LIBrary "erslib.h"  by I.N. を
	大いに参考にしながらC++で簡易にクラス化しました
-------------------------------------------------------------*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define COMLIBFLAG_110			0x00000001
#define COMLIBFLAG_300			0x00000002
#define COMLIBFLAG_600			0x00000003
#define COMLIBFLAG_1200			0x00000004
#define COMLIBFLAG_2400			0x00000005
#define COMLIBFLAG_4800			0x00000006
#define COMLIBFLAG_9600			0x00000007
#define COMLIBFLAG_14400		0x00000008
#define COMLIBFLAG_19200		0x00000009
#define COMLIBFLAG_38400		0x0000000A
#define COMLIBFLAG_56000		0x0000000B
#define COMLIBFLAG_57600		0x0000000C
#define COMLIBFLAG_115200		0x0000000D
#define COMLIBFLAG_128000		0x0000000E
#define COMLIBFLAG_256000		0x0000000F

#define COMLIBFLAG_1			0x00000010
#define COMLIBFLAG_15			0x00000020
#define COMLIBFLAG_2			0x00000030

#define COMLIBFLAG_NO			0x00000100
#define COMLIBFLAG_ODD			0x00000200
#define COMLIBFLAG_EVEN			0x00000300
#define COMLIBFLAG_MARK			0x00000400
#define COMLIBFLAG_SPACE		0x00000500

#define COMLIBFLAG_4			0x00001000
#define COMLIBFLAG_5			0x00002000
#define COMLIBFLAG_6			0x00003000
#define COMLIBFLAG_7			0x00004000
#define COMLIBFLAG_8			0x00005000

#define COMLIBFLAG_DTR_N		0x00010000
#define COMLIBFLAG_DTR_Y		0x00020000
#define COMLIBFLAG_DTR_H		0x00030000

#define COMLIBFLAG_RTS_N		0x00100000
#define COMLIBFLAG_RTS_Y		0x00200000
#define COMLIBFLAG_RTS_H		0x00300000
#define COMLIBFLAG_RTS_T		0x00400000

#define COMLIBFLAG_CTS_Y		0x01000000
#define COMLIBFLAG_CTS_N		0x02000000
#define COMLIBFLAG_DSR_Y		0x04000000
#define COMLIBFLAG_DSR_N		0x08000000

#define COMLIBFLAG_X_Y			0x10000000
#define COMLIBFLAG_X_N			0x20000000

#define COMLIBFLAG_NO_FLOW_CONTROL (COMLIBFLAG_CTS_N|COMLIBFLAG_DSR_N|COMLIBFLAG_DTR_N|COMLIBFLAG_RTS_N|COMLIBFLAG_X_N)


class CSerialCOM
{
private:
	HANDLE m_serial;
	COMMPROP m_prop;

public:
	CSerialCOM(void);
	~CSerialCOM(void);

	bool Open(int port,int recv_size,int send_size);
	bool Close(void);

	bool SetConfig(DWORD flag);

	bool SetTimeout(int recv_total,int recv_interval,int send);

	size_t Send(const char *text);
	size_t Recv(char *buf,size_t bufsize);
	int  Getc(void);

	int GetType(void) { return m_prop.dwProvSubType; }
	bool IsSerialPort(void);

	size_t GetReceivedSize(void);
};
