//==============================================================================
// effect.hpp
// Copyright (C) SatisKia. All rights reserved.
//==============================================================================
#ifndef EFFECT_HPP
#define EFFECT_HPP

#include <windows.h>
#include <string.h>

typedef BOOL (__cdecl *EFFECT_LOAD_PROC  )(HGLOBAL /*path*/, long /*path_len*/);
typedef BOOL (__cdecl *EFFECT_UNLOAD_PROC)(void);
typedef BOOL (__cdecl *EFFECT_RUN_PROC   )(HGLOBAL /*plugin*/, long /*plugin_len*/, HGLOBAL /*param*/, long /*param_num*/);

//==============================================================================
// エフェクトモジュールにアクセスするクラス
//==============================================================================
class effect {

private:

	/* モジュールのハンドル */
	HMODULE _mod;

	/* エクスポートされた関数のアドレス */
	EFFECT_LOAD_PROC	__load;
	EFFECT_UNLOAD_PROC	__unload;
	EFFECT_RUN_PROC		__run;

private:

	HGLOBAL str_to_h(const char* str, long* len)
	{
		if ( str == NULL ) {
			*len = 0;
		} else {
			*len = strlen(str);
		}
		HGLOBAL h = NULL;
		if ( *len > 0 ) {
			h = ::GlobalAlloc(GMEM_FIXED, *len);
			memcpy((char*)h, (LPCTSTR)str, *len);
		}
		return h;
	}

public:

	effect(const char* path, const char* dllname)
	//==========================================================================
	//【機  能】コンストラクタ
	//--------------------------------------------------------------------------
	//【引  数】path     エフェクトモジュールファイルへのパス
	//          dllname  エフェクトモジュールファイル名
	//==========================================================================
	{
		// カレントディレクトリを設定する
		char saveDir[MAX_PATH + 1];
		::GetCurrentDirectory(MAX_PATH, saveDir);
		::SetCurrentDirectory(path);

		// モジュールを読み込む
		_mod = ::LoadLibrary(dllname);

		// カレントディレクトリを元に戻す
		::SetCurrentDirectory(saveDir);

		// エクスポートされた関数のアドレスを取得する
		if ( _mod ) {
			__load   = (EFFECT_LOAD_PROC  )::GetProcAddress(_mod, "_load"  );
			__unload = (EFFECT_UNLOAD_PROC)::GetProcAddress(_mod, "_unload");
			__run    = (EFFECT_RUN_PROC   )::GetProcAddress(_mod, "_run"   );
		} else {
			__load   = NULL;
			__unload = NULL;
			__run    = NULL;
		}

		// ロード
		if ( __load ) {
			long path_len;
			HGLOBAL path_h = str_to_h(path, &path_len);
			__load(path_h, path_len);
		}
	}

	virtual ~effect()
	//==========================================================================
	//【機  能】デストラクタ
	//==========================================================================
	{
		// アンロード
		if ( __unload ) {
			__unload();
		}

		// モジュールを解放する
		if ( _mod ) {
			::FreeLibrary(_mod);
		}
	}

	BOOL _run(const char* plugin, const long* param, long param_num)
	//==========================================================================
	//【機  能】プラグイン実行
	//--------------------------------------------------------------------------
	//【引  数】plugin     プラグイン DLL 名
	//                     拡張子”.dll”は付けても付けなくても良い
	//          param      プラグインに渡すパラメータの配列
	//          param_num  パラメータの数
	//--------------------------------------------------------------------------
	//【戻り値】成功時 TRUE、失敗時 FALSE
	//==========================================================================
	{
		if ( __run ) {
			long plugin_len;
			HGLOBAL plugin_h = str_to_h(plugin, &plugin_len);
			HGLOBAL param_h = ::GlobalAlloc(GMEM_FIXED, sizeof(long) * param_num);
			memcpy((long*)param_h, param, sizeof(long) * param_num);
			return __run(plugin_h, plugin_len, param_h, param_num);
		}
		return FALSE;
	}

};

#endif // !EFFECT_HPP
//==============================================================================
// End of effect.hpp
//==============================================================================
