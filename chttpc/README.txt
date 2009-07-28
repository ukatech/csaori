[en.US_UTF-8 (English)]

chttpc is a rewrite of httpc SAORI of The Maintenance Shop's satoriya project ( http://ms.shillest.net/satoriya.xhtml )
by using CSAORI framework. There is part of httpc source code(e.g. Thread class), and htmlcxx ( http://htmlcxx.sf.net/ )
for parsing HTML.

Usage:
* No Argument: Check for Internet connection, Return: 1 - OK, 0 - NG
* 1 Argument(URL): download and return contents.
* More than 1 Argument:
 o First one must be URL
 o options afterwards:
  + translate - try to tranlate html to sakura script
  + strip - remove all html tags
  + codepage=<codepage name> - specfiy code page (e.g. big5, x-sjis)
  + save=<filename> - save downloaded content as "filename" (same place as chttpc.dll)
  + saveParsed=<filename> - save parsed content as "filename"(same place as chttpc.dll)
  + start=<string> and end=<string> - return string between start and end
  + hwnd=<hwnd> and id=<string> - Run the job in background. It will notify through DirectSSTP to hwnd 
                                  using OnchttpcNotify event (ref0: id, ref1: content)
  + noOutput - return nothing (start from a2)


known bug:
* translate can't translate nested tags correctly (e.g. <li><a>)

Changelog:
(Please refer to svn log)

Make balloon as a browser! (Make links useful inside balloon)
code (AYA):
OnAnchorSelect
{
        _res = FUNCTIONEX("chttpc.dll",reference0,'translate')
        _res;
}

(End of en.US_UTF-8 Readme)

[zh.TW_UTF-8 (正體中文)]

chttpc 是把 整備班的 satoriya project ( http://ms.shillest.net/satoriya.xhtml ) 中的 httpc SAORI
以 CSAORI 重寫而成的。當中有部份 httpc 源程式碼(如 Thread class)，還有 htmlcxx ( http://htmlcxx.sf.net/ )
作為分析 HTML 之用。

用法：
* 無引數: 檢查能否上網，回傳: 1 - OK, 0 - NG
* 一個引數(網址): 下載並直接回傳
* 多過一個引數：
 o 第一個必須為網址
 o 之後的選項：
  + translate - 嘗試轉換html為sakura script
  + strip - 去除html tag
  + codepage=<編碼名稱> - 指定來源編碼 (如: big5、x-sjis)
  + save=<檔名> - 把剛下載的檔案存為「檔名」(與chttpc.dll同位置)
  + saveParsed=<檔名> - 把處理好的數據存為「檔名」(與chttpc.dll同位置)
  + start=<字串> 及 end=<字串> - 回傳在 start 和 end 之間字串
  + hwnd=<hwnd> 及 id=<字串> - 背景執行。會經 DirectSSTP 向 hwnd 回傳 OnchttpcNotify (ref0: id, ref1: 回傳值)
  + noOutput - 不回傳 (a2新增)


已知bug：
* translate 未能正確處理巢狀標籤 (如 <li><a>)

更新：
(請參看svn log)

讓balloon變成瀏覽器! (讓balloon中的連結有效)
代碼 (文):
OnAnchorSelect
{
        _res = FUNCTIONEX("chttpc.dll",reference0,'translate')
        _res;
}

(End of zh.TW_UTF-8 Readme)