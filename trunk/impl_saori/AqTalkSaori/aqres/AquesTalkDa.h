//////////////////////////////////////////////////////////////////////
/*!	@class	CAquesTalkDa

	@brief	規則音声合成エンジン AquesTalkDa

  音声記号列から音声波形データを生成し、サウンドデバイスに出力する


	@author	N.Yamazaki (Aquest)

	@date	2006/05/08	N.Yamazaki	Creation
	@date	2006/10/03	N.Yamazaki	Ver.2.0
	@date	2006/10/12	N.Yamazaki	Ver.2.1 .defファイルの使用と__stdcallの定義
*/
//	このソースコードの利用条件については、
//	同梱の AqLicense.txt をご覧ください
//  COPYRIGHT (C) 2006 AQUEST CORP.
//////////////////////////////////////////////////////////////////////
#if !defined(_AQUESTALKDA_H_)
#define _AQUESTALKDA_H_
#ifdef __cplusplus
extern "C"{
#endif

#if defined(AQUESTALKDA_EXPORTS)
#undef	DllExport
#define DllExport	__declspec( dllexport )
#else
#define DllExport
#endif

#include <windows.h>


/////////////////////////////////////////////
//!	音声を合成して出力（同期タイプ）
//!	音声の再生が終了するまで戻らない
//! @param	koe[in]		音声記号列（NULL終端）
//! @param	iSpeed[in]	発話速度 [%] 50-300 の間で指定
//!	@return	0:正常終了　それ以外：エラーコード
DllExport	int __stdcall AquesTalkDa_PlaySync(const char *koe, int iSpeed=100);



////////////////////////////////////////////////////////////////////////
//  以下、非同期タイプの関数
////////////////////////////////////////////////////////////////////////
typedef void		*H_AQTKDA;	// 音声合成エンジンのハンドル 同期タイプの関数で使用する

/////////////////////////////////////////////
//!	音声合成エンジンのインスタンスを生成（非同期タイプ）
//! @return	音声合成エンジンのハンドルを返す
DllExport	H_AQTKDA __stdcall AquesTalkDa_Create();

/////////////////////////////////////////////
//!	音声合成エンジンのインスタンスを解放（非同期タイプ）
//! @param	hMe[in]		音声合成エンジンのハンドル AquesTalkDa_Create()で生成
DllExport	void __stdcall AquesTalkDa_Release(H_AQTKDA hMe);

/////////////////////////////////////////////
//!	音声を合成して出力（非同期タイプ）
//!	音声波形生成後に、すぐに戻る
//!	hWndを指定すると再生終了後、msgに指定したメッセージがPostされる。
//!	再生終了前にAquesTalkDa_Play()を呼び出して、連続的に再生させることも可能。
//!	
//! @param	hMe[in]		音声合成エンジンのハンドル AquesTalkDa_Create()で生成
//! @param	koe[in]		音声記号列（NULL終端）
//! @param	iSpeed[in]	発話速度 [%] 50-300 の間で指定
//! @param	hWnd[in]	終了メッセージ送出先ウィンドウハンドル
//! @param	msg[in]		終了メッセージ
//! @param	dwUser[in]	任意のユーザパラメータ(メッセージのlParam に設定される）
//!	@return	0:正常終了　それ以外：エラーコード
DllExport	int __stdcall AquesTalkDa_Play(H_AQTKDA hMe, const char *koe, int iSpeed=100, HWND hWnd=0, unsigned long msg=0, unsigned long dwUser=0);

/////////////////////////////////////////////
//!	再生の中止 
//! AquesTalkDa_Play()で再生中に、再生を中断する。
//!	再生中(再生待ちを含む）であり、終了メッセージ送出先が指定されていたなら、
//! 終了メッセージがPostされる。
//! @param	hMe[in]		音声合成エンジンのハンドル AquesTalkDa_Create()で生成
DllExport	void __stdcall AquesTalkDa_Stop(H_AQTKDA hMe);

/////////////////////////////////////////////
//!	再生中か否か
//! @param	hMe[in]		音声合成エンジンのハンドル AquesTalkDa_Create()で生成
//! @return 1:再生中 0:再生中でない
DllExport	int __stdcall AquesTalkDa_IsPlay(H_AQTKDA hMe);



#ifdef __cplusplus
}
#endif
#endif // !defined(_AQUESTALKDA_H_)
