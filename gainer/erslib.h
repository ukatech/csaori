// Easy RS-232C LIBrary "erslib.h"  by I.N.
// Modified by CSAORI Project

#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

//パリティエラー時の置き換え文字
#define ERS_PECHAR (_T(' '))

#define ERSLIBMAXPORT 256
int ers_initdone[ERSLIBMAXPORT]={
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

#define ERS_110			0x00000001
#define ERS_300			0x00000002
#define ERS_600			0x00000003
#define ERS_1200		0x00000004
#define ERS_2400		0x00000005
#define ERS_4800		0x00000006
#define ERS_9600		0x00000007
#define ERS_14400		0x00000008
#define ERS_19200		0x00000009
#define ERS_38400		0x0000000A
#define ERS_56000		0x0000000B
#define ERS_57600		0x0000000C
#define ERS_115200		0x0000000D
#define ERS_128000		0x0000000E
#define ERS_256000		0x0000000F

#define ERS_1			0x00000010
#define ERS_15			0x00000020
#define ERS_2			0x00000030

#define ERS_NO			0x00000100
#define ERS_ODD			0x00000200
#define ERS_EVEN		0x00000300
#define ERS_MARK		0x00000400
#define ERS_SPACE		0x00000500

#define ERS_4			0x00001000
#define ERS_5			0x00002000
#define ERS_6			0x00003000
#define ERS_7			0x00004000
#define ERS_8			0x00005000

#define ERS_DTR_N		0x00010000
#define ERS_DTR_Y		0x00020000
#define ERS_DTR_H		0x00030000

#define ERS_RTS_N		0x00100000
#define ERS_RTS_Y		0x00200000
#define ERS_RTS_H		0x00300000
#define ERS_RTS_T		0x00400000

#define ERS_CTS_Y		0x01000000
#define ERS_CTS_N		0x02000000
#define ERS_DSR_Y		0x04000000
#define ERS_DSR_N		0x08000000

#define ERS_X_Y			0x10000000
#define ERS_X_N			0x20000000

//ハンドル値
HANDLE ers_hcom[ERSLIBMAXPORT];
DWORD ers_hevt[ERSLIBMAXPORT];

#define ERSHCOMn (ers_hcom[n-1])
#define ERSHEVTn (ers_hevt[n-1])

// パラメータ n チェック用補助関数
// n が範囲外であったりCOMnが未初期化の場合１が戻る．
int ers_check(int n)
{
	if(n<1 || n>ERSLIBMAXPORT) return 1;
	if(!ers_initdone[n-1]) return 1;
	return 0;
}

//XON/XOFFしきい値の設定		ver.1.3
// xoff : バッファの残りサイズ（バイト単位）
// xon  : バッファ内のデータ数（バイト単位）
int ERS_XoffXon(int n, int xoff, int xon)
{
	DCB dcb;

	if(ers_check(n)) return 1;

	GetCommState(ERSHCOMn, &dcb);
	dcb.XoffLim=xoff;
	dcb.XonLim=xon;
	if (!SetCommState(ERSHCOMn, &dcb)) return 2;
	return 0;
}

// 通信パラメータの設定		ver.1.3
int ERS_Config(int n, unsigned int data)
{
	DCB dcb;
	int d;
	int baud[16]={0,CBR_110,CBR_300,CBR_600,CBR_1200,CBR_2400,CBR_4800,CBR_9600,CBR_14400,CBR_19200,CBR_38400,CBR_56000,CBR_57600,CBR_115200,CBR_128000,CBR_256000};
	int stopbit[4]={0,ONESTOPBIT,ONE5STOPBITS,TWOSTOPBITS};
	int parity[6]={0,NOPARITY,ODDPARITY,EVENPARITY,MARKPARITY,SPACEPARITY};
	int bytesize[6]={0,4,5,6,7,8};
	int dtr[4]={0,DTR_CONTROL_DISABLE,DTR_CONTROL_ENABLE,DTR_CONTROL_HANDSHAKE};
	int rts[5]={0,RTS_CONTROL_DISABLE,RTS_CONTROL_ENABLE,RTS_CONTROL_HANDSHAKE,RTS_CONTROL_TOGGLE};

	if(ers_check(n)) return 1;

	GetCommState(ERSHCOMn, &dcb);

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
			dcb.ErrorChar=ERS_PECHAR;
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
	if(data &  ERS_CTS_Y) dcb.fOutxCtsFlow=TRUE;
	if(data &  ERS_CTS_N) dcb.fOutxCtsFlow=FALSE;

	// DSR control
	if(data &  ERS_DSR_Y) dcb.fOutxDsrFlow=TRUE;
	if(data &  ERS_DSR_N) dcb.fOutxDsrFlow=FALSE;

	// X control
	d=(data & 0x30000000);
	if(d==ERS_X_Y){
		dcb.fTXContinueOnXoff=FALSE;
		dcb.fOutX=TRUE;
		dcb.fInX=TRUE;
	}else if(d==ERS_X_N){
		dcb.fTXContinueOnXoff=FALSE;
		dcb.fOutX=FALSE;
		dcb.fInX=FALSE;
	}
	dcb.fAbortOnError = FALSE;

	if (!SetCommState(ERSHCOMn, &dcb)) return 2;

	return 0;
}

// 受信タイムアウト時間の設定(ms単位)	ver.1.3
int ERS_RecvTimeOut(int n, int rto, int interval)
{
	COMMTIMEOUTS ct;

	if(ers_check(n)) return 1;
	GetCommTimeouts(ERSHCOMn,&ct);

	ct.ReadIntervalTimeout=interval;
	ct.ReadTotalTimeoutMultiplier=rto/interval;
	ct.ReadTotalTimeoutConstant=interval;
	
	if(!SetCommTimeouts(ERSHCOMn,&ct)) return 2;
	return 0;
}

// 送信タイムアウト時間の設定(ms単位)	ver.1.3
int ERS_SendTimeOut(int n, int sto)
{
	COMMTIMEOUTS ct;
	
	if(ers_check(n)) return 1;
	GetCommTimeouts(ERSHCOMn,&ct);

	ct.WriteTotalTimeoutMultiplier=1;
	ct.WriteTotalTimeoutConstant=sto;
	
	if(!SetCommTimeouts(ERSHCOMn,&ct)) return 2;
	return 0;
}

//==================== シリアル通信の開始 ====================
// n=1~256...COM1~COM256         ver.1.7
int ERS_Open(int n, int recv_size, int send_size)
{
	TCHAR comname[32];

	if(n<1 || n>ERSLIBMAXPORT) return 1;
	if(ers_initdone[n-1]) return 2;

	_stprintf(comname,_T("\\\\.\\COM%d"),n);

	ERSHCOMn=CreateFile(comname,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	
	// オープンに失敗したとき
	if(ERSHCOMn==INVALID_HANDLE_VALUE)	return 3;

	//送受信バッファの設定
	SetupComm(ERSHCOMn,recv_size,send_size);

	ers_initdone[n-1]=1;

	//通信のデフォルト設定
	ERS_Config(n,ERS_9600|ERS_1|ERS_NO|ERS_8|ERS_X_N|ERS_CTS_N|ERS_DSR_N|ERS_DTR_Y|ERS_RTS_Y);
	ERS_RecvTimeOut(n,1000,1000);
	ERS_SendTimeOut(n,1000);
	return 0;
}

// ==================== シリアル通信の終了 ====================
int ERS_Close(int n)
{
	if(ers_check(n)) return 1;
	PurgeComm(ERSHCOMn,PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	if(!CloseHandle(ERSHCOMn)) return 2;
	ers_initdone[n-1]=0;
	return 0;
}

// シリアル通信の終了（すべて閉じる）
void ERS_CloseAll(void)
{
	int n;
	for(n=1;n<=ERSLIBMAXPORT;n++){
		if (ers_initdone[n-1]) ERS_Close(n);
	}
}

// 受信バッファのデータ数(バイト)を調べる
int ERS_CheckRecv(int n)
{
	COMSTAT cs;
	DWORD err;
	if(ers_check(n)) return 0;
	ClearCommError(ERSHCOMn,&err,&cs);
	return cs.cbInQue;
}

// 送信バッファのデータ数(バイト)を調べる
int ERS_CheckSend(int n)
{
	COMSTAT cs;
	DWORD err;
	if(ers_check(n)) return 0;
	ClearCommError(ERSHCOMn,&err,&cs);
	return cs.cbOutQue;
}

//データ受信
int ERS_Recv(int n, void *buf, int size)
{
	DWORD m;
	if(ers_check(n)) return 0;
	ReadFile(ERSHCOMn,buf,size,&m,NULL);
	return m;
}

//１バイト受信 ver.1.6
int ERS_Getc(int n)
{
	DWORD m;
	int c=0;
	if(ers_check(n)) return 0;
	ReadFile(ERSHCOMn,&c,1,&m,NULL);
	if(!m)return EOF;
	return c;
}

//文字列受信 ver.1.6
//(最初の\r|\n|\0まで読む)
int ERS_Gets(int n, char *s, int size)
{
	DWORD m;
	int cnt=0;
	char c;

	if(ers_check(n)) return 0;
	if(!size) return 0;

	for(;;){
		ReadFile(ERSHCOMn,&c,1,&m,NULL);
		if(!m || c=='\r'|| c=='\n' || c=='\0')break;
		*s++=c;
		cnt++;
		if(size>0 && cnt>=size-1)break;
	}
	*s='\0';
	if(m)cnt++;
	return cnt;
}

//データ送信
int ERS_Send(int n, void *buf, int size)
{
	DWORD m;
	if(ers_check(n)) return 0;
	WriteFile(ERSHCOMn,buf,size,&m,NULL);
	return m;
}

//１バイト送信 ver.1.6
int ERS_Putc(int n, int c)
{
	DWORD m;
	if(ers_check(n)) return 0;
	WriteFile(ERSHCOMn,&c,1,&m,NULL);
	return m;
}

//文字列送信 ver.1.6
//(終端\0は\nへ変換する)
int ERS_Puts(int n, char *s)
{
	DWORD m;
	char c;
	int cnt=0;

	if (ers_check(n)) return 0;
	while(c=*s++){
		WriteFile(ERSHCOMn,&c,1,&m,NULL);
		if(!m)return cnt;
		cnt++;
	}
	c='\n';
	WriteFile(ERSHCOMn,&c,1,&m,NULL);
	if(!m)return cnt;
	cnt++;
	return cnt;
}

//COMポートへのprintf() 1.7
int ERS_Printf(int n, char *format, ...)
{
	char buf[256];
	va_list vl;

	va_start(vl,format);

	vsprintf(buf,format,vl);

	va_end(vl);

	return ERS_Puts(n,buf);
}

//受信バッファのクリア
int ERS_ClearRecv(int n)
{
	if(ers_check(n)) return 1;
	PurgeComm(ERSHCOMn,PURGE_RXCLEAR);		
	return 0;
}

//送信バッファのクリア
int ERS_ClearSend(int n)
{
	if(ers_check(n)) return 1;
	PurgeComm(ERSHCOMn,PURGE_TXCLEAR);		
	return 0;
}

//任意のボーレートを設定 ver.1.5
int ERS_BaudRate(int n, int baudrate)
{
	DCB dcb;

	if(ers_check(n)) return 1;
	GetCommState(ERSHCOMn,&dcb);
	dcb.BaudRate=baudrate;
	if (!SetCommState(ERSHCOMn, &dcb)) return 2;
	return 0;
}

//ダイアログを使用して設定する ver.1.7  非WindowsCEのみ
//メモ：CommConfigDialog()は'\\.\'がつく名前に対応しない？
int ERS_ConfigDialog(int n)
{
	COMMCONFIG cc;
	DWORD size;
	TCHAR comname[32];

	if(ers_check(n)) return 1;

	_stprintf(comname,_T("COM%d"),n);

	GetCommConfig(ERSHCOMn,&cc,&size);
	CommConfigDialog(comname,NULL,&cc);
	if (!SetCommConfig(ERSHCOMn,&cc,sizeof(cc)))return 2;
	return 0;
}
