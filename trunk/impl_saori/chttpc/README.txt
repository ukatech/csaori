[en.US_UTF-8 (English)]

chttpc is a rewrite of httpc SAORI of The Maintenance Shop's satoriya project ( http://ms.shillest.net/satoriya.xhtml )
by using CSAORI framework. There is part of httpc source code(e.g. Thread class), and htmlcxx ( http://htmlcxx.sf.net/ )
for parsing HTML.

Usage:
* No Argument: Check for Internet connection, Return: 1 - OK, 0 - NG
* 1 Argument(URL): download and return contents.
* More than 1 Argument:
 o First one must be URL (Local file can be opened with "file:" prefix, base path is same place as chttpc.dll, e.g.: file:C:\bootlog.txt)
 o options afterwards:
  + translate - try to tranlate html to sakura script
  + strip - remove all html tags
  + codepage=<codepage name> - specfiy code page (e.g. big5, x-sjis. 
              chttpc will detect HTML meta charset by default. 50001 for auto detection.)
  + save=<filename> - save downloaded content as "filename" (same place as chttpc.dll)
  + saveUtf8=<filename> - save downloaded content in UTF-8 encoding as "filename" (same place as chttpc.dll, start from b1)
  + saveParsed=<filename> - save parsed content as "filename"(same place as chttpc.dll)
  + start=<string> and end=<string> - return string between start and end
  + hwnd=<hwnd> and id=<string> - Run the job in background. It will notify through DirectSSTP to hwnd 
                                  using OnchttpcNotify event (ref0: id, ref1: content, ref2: url)
  + noOutput - return nothing (since a2)
  + removeTags=<tag>[,<tag>]  - remove listed html tags from translation (< and > are not needed, e.g. removeTags=a,img) (since r3)
  + multiSearch - make multiple start/end search, results separated with 0x2 (since r4)


known bug:
* translate can't translate nested tags correctly (e.g. <li><a>) (shoulde be fixed in beta 1)

Changelog:
20091205 - Release 5
           add ability of opening local file with "file:" prefix
20090829 - Release 4
           add multiSearch option
20090806 - Release 3
           add removeTags option
20090804 - Release 2
           make strip and translate work with start= and end=
           convert non translated output to one line string
20090802 - Release 1
(for logs before Release 1 Please refer to svn log)

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
 o 第一個必須為網址 (本機檔案可由 "file:" 前輟開啟，基底路徑為與chttpc.dll同位置, 如：file:C:\bootlog.txt)
 o 之後的選項：
  + translate - 嘗試轉換html為sakura script
  + strip - 去除html tag
  + codepage=<編碼名稱> - 指定來源編碼 (如: big5、x-sjis。預設chttpc會偵測HTML編碼。指定50001會自動偵測。)
  + save=<檔名> - 把剛下載的檔案存為「檔名」(與chttpc.dll同位置)
  + saveUtf8=<檔名> - 把剛下載的檔案以UTF-8編碼存為「檔名」(與chttpc.dll同位置，b1新增)
  + saveParsed=<檔名> - 把處理好的數據存為「檔名」(與chttpc.dll同位置)
  + start=<字串> 及 end=<字串> - 回傳在 start 和 end 之間字串
  + hwnd=<hwnd> 及 id=<字串> - 背景執行。會經 DirectSSTP 向 hwnd 回傳 OnchttpcNotify (ref0: id, ref1: 回傳值, ref2: 網址)
  + noOutput - 不回傳 (a2新增)
  + removeTags=<標籤>[,<標籤>]  - 不轉換列出的html標籤(不需要填 < 和 > ，例如 removeTags=a,img) (r3新增)
  + multiSearch - 進行多次 start/end 搜尋, 回傳結果以 0x2 分隔 (r4新增)


已知bug：
* translate 未能正確處理巢狀標籤 (如 <li><a>) (相信在beta 1中已修正)

更新：
20091205 - Release 5
           增加開啟本機檔案的能力(以 "file:" 前輟)
20090829 - Release 4
           增加 multiSearch 選項
20090806 - Release 3
           增加 removeTags 選項
20090804 - Release 2
           讓 strip 和 translate 可以跟 start= 和 end= 同時使用
           轉換不翻譯的輸出為一行字串
20090802 - Release 1
(Release 1 前的記錄請參看 svn log)

讓balloon變成瀏覽器! (讓balloon中的連結有效)
代碼 (文):
OnAnchorSelect
{
        _res = FUNCTIONEX("chttpc.dll",reference0,'translate')
        _res;
}

(End of zh.TW_UTF-8 Readme)