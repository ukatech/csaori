/*-------------------------------------------------------------
	シリアル通信ライブラリ
	Easy RS-232C LIBrary "erslib.h"  by I.N. を
	大いに参考にしながらC++で簡易にクラス化しました
-------------------------------------------------------------*/

#include <stdio.h>
#include <wchar.h>
#include <tchar.h>

#include "serial.h"

bool CSerialCOM::Open(int n,int recv_size,int send_size)
{
	if ( m_serial ) { return true; }

	TCHAR comname[32];

	if(n<1 || n>128) return false;

	_stprintf(comname,_T("\\\\.\\COM%d"),n);

	m_serial = ::CreateFile(comname,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING | FILE_FLAG_OVERLAPPED,0,NULL);
	
	// オープンに失敗したとき
	if(m_serial==INVALID_HANDLE_VALUE)	return false;

	//送受信バッファの設定
	::SetupComm(m_serial,recv_size,send_size);

	//通信のデフォルト設定
	SetConfig(COMLIBFLAG_9600|COMLIBFLAG_1|COMLIBFLAG_NO|COMLIBFLAG_8|
		COMLIBFLAG_X_N|COMLIBFLAG_CTS_N|COMLIBFLAG_DSR_N|COMLIBFLAG_DTR_Y|COMLIBFLAG_RTS_Y);
	SetTimeout(1000,1000,1000);

	return true;
}

bool CSerialCOM::Close(void)
{
	if ( ! m_serial ) { return true; }

	::PurgeComm(m_serial,PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	::CloseHandle(m_serial);

	m_serial = NULL;

	return true;
}


bool CSerialCOM::SetConfig(DWORD data)
{
	if ( ! m_serial ) { return false; }

	DCB dcb;
	dcb.DCBlength = sizeof(dcb);

	int d;
	static const int baud[16]={0,CBR_110,CBR_300,CBR_600,CBR_1200,CBR_2400,CBR_4800,CBR_9600,CBR_14400,CBR_19200,CBR_38400,CBR_56000,CBR_57600,CBR_115200,CBR_128000,CBR_256000};
	static const int stopbit[4]={0,ONESTOPBIT,ONE5STOPBITS,TWOSTOPBITS};
	static const int parity[6]={0,NOPARITY,ODDPARITY,EVENPARITY,MARKPARITY,SPACEPARITY};
	static const int bytesize[6]={0,4,5,6,7,8};
	static const int dtr[4]={0,DTR_CONTROL_DISABLE,DTR_CONTROL_ENABLE,DTR_CONTROL_HANDSHAKE};
	static const int rts[5]={0,RTS_CONTROL_DISABLE,RTS_CONTROL_ENABLE,RTS_CONTROL_HANDSHAKE,RTS_CONTROL_TOGGLE};

	::GetCommState(m_serial, &dcb);

	// Baud rate
	d= data & 0xF; if (d) dcb.BaudRate=baud[d];
	
	// Stop bit
	d=(data & 0x30)>>4;	if (d) dcb.StopBits=stopbit[d];

	// Parity
	d=(data & 0x700)>>8;
	if (d) {
		dcb.Parity=parity[d];
		if(d>1){
			dcb.fParity=TRUE;
			dcb.fErrorChar=TRUE;
			dcb.ErrorChar=' ';
		}else{
			dcb.fParity=FALSE;
			dcb.fErrorChar=FALSE;
		}
	}
	
	// Byte size
	d=(data & 0x7000)>>12; if (d) dcb.ByteSize=bytesize[d];
	
	// Dtr control
	d=(data & 0x30000)>>16; if (d) dcb.fDtrControl=dtr[d];

	// Rts control
	d=(data & 0x700000)>>20; if (d) dcb.fRtsControl=rts[d];

	// CTS control
	if(data &  COMLIBFLAG_CTS_Y) dcb.fOutxCtsFlow=TRUE;
	if(data &  COMLIBFLAG_CTS_N) dcb.fOutxCtsFlow=FALSE;

	// DSR control
	if(data &  COMLIBFLAG_DSR_Y) dcb.fOutxDsrFlow=TRUE;
	if(data &  COMLIBFLAG_DSR_N) dcb.fOutxDsrFlow=FALSE;

	// X control
	d=(data & 0x30000000);
	if(d==COMLIBFLAG_X_Y){
		dcb.fTXContinueOnXoff=FALSE;
		dcb.fOutX=TRUE;
		dcb.fInX=TRUE;
	}else if(d==COMLIBFLAG_X_N){
		dcb.fTXContinueOnXoff=FALSE;
		dcb.fOutX=FALSE;
		dcb.fInX=FALSE;
	}
	dcb.fAbortOnError = FALSE;

	if (!::SetCommState(m_serial, &dcb)) return false;

	::SetCommMask(m_serial,EV_BREAK|EV_CTS|EV_DSR|EV_ERR|EV_RING|EV_RLSD|EV_RXCHAR);

	return true;
}


bool CSerialCOM::SetTimeout(int rto,int interval,int sto)
{
	if ( ! m_serial ) { return false; }

	COMMTIMEOUTS ct;

	::GetCommTimeouts(m_serial,&ct);

	if ( rto == 0 && interval == 0 ) { //blocking(timeout disabled)
		ct.ReadIntervalTimeout=0;
		ct.ReadTotalTimeoutMultiplier=0;
		ct.ReadTotalTimeoutConstant=0;
	}
	else if ( rto == MAXDWORD && interval == 0 ) { //nonblocking
		ct.ReadIntervalTimeout=MAXDWORD;
		ct.ReadTotalTimeoutMultiplier=0;
		ct.ReadTotalTimeoutConstant=0;
	}
	else {
		ct.ReadIntervalTimeout=interval;
		if ( interval ) {
			ct.ReadTotalTimeoutMultiplier=1;
			ct.ReadTotalTimeoutConstant=interval;
		}
		else {
			ct.ReadTotalTimeoutMultiplier=0;
			ct.ReadTotalTimeoutConstant=0;
		}
	}

	if ( sto == 0 ) {
		ct.WriteTotalTimeoutMultiplier=0;
		ct.WriteTotalTimeoutConstant=0;
	}
	else {
		ct.WriteTotalTimeoutMultiplier=1;
		ct.WriteTotalTimeoutConstant=sto;
	}
	
	if(!::SetCommTimeouts(m_serial,&ct)) return false;
	return true;
}


size_t CSerialCOM::Send(const char *text)
{
	DWORD m;
	::WriteFile(m_serial,text,strlen(text),&m,NULL);
	return m;
}

size_t CSerialCOM::Recv(char *buf,size_t bufsize)
{
	DWORD m;
	::ReadFile(m_serial,buf,bufsize,&m,NULL);
	return m;
}


int  CSerialCOM::Getc(void)
{
	char buf[2] = {0,0};
	if ( Recv(buf,1) == 0 ) {
		return EOF;
	}
	return buf[0];
}


size_t CSerialCOM::GetReceivedSize(void)
{
	COMSTAT cs;
	DWORD err;
	::ClearCommError(m_serial,&err,&cs);
	return cs.cbInQue;
}

