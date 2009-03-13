[UTF-8 Encoding]

[English]
screenshot.dll is a SAORI plugin for ukagaka.

Usage:

- Capture Screen into 24bit BMP file:
* Input
 * Argument0: output filename
* Output
 * Result: OK
 * If failed
  - Result: NG
  - Value0: Error. RetCode=<code>

Known Issue:
(currently none)

Version History:
 20090313-1927:
 - Windows with transparency will be captured
 - output file will be in same directory of dll
 20090312-2014:
 - update CSaori to SVN r65
 - use ANSI version of File I/O API due to limitation of CSaori
 - fix not closing file after write
 20090310-2137:
 - Initial Version

Credits:

screenshot.dll SAORI is based on ReactOS screenshot.exe code.

Copyright(C) ReactOS.org 1998-2009.
Copyright(C) Roy 2009. Some Rights Reserved.
Licence: GPLv2

Author's website:  某廢柴的廢棄日記 ( http://scrappedblog.blogspot.com/ )

[End of English Readme]

[Traditional Chinese / 繁體中文(zh_TW)]
screenshot.dll 是一個伺か(ukagaka)的 SAORI 插件。

使用方法:

- 截圖並儲存至24bit BMP檔中
* 輸入
 * Argument0: 輸出檔名
* 輸出
 * Result: OK
 * 如果函數失敗
  - Result: NG
  - Value0: Error. RetCode=<code>

已知問題:
(暫時沒有)

版本紀錄:
 20090313-1927:
 - 有透明度的視窗將會被截取到
 - 輸出檔案將會和 DLL 位於相同位置
 20090312-2014:
 - 更新 CSaori 至 SVN r65
 - 因 CSaori 的限制而 ANSI 版的檔案輸入輸出 API
 - 修正輸出檔案沒有關閉的問題
 20090310-2137:
 - 初版


鳴謝:

screenshot.dll SAORI 是基於 ReactOS 的截圖軟件的代碼。



版權所有(C) ReactOS.org 1998-2009。
版權所有(C) Roy 2009。 保留部份版權。
授權條款: GPLv2 ( http://creativecommons.org/licenses/by-sa/3.0/deed.zh_TW )

作者網站:  某廢柴的廢棄日記 ( http://scrappedblog.blogspot.com/ )

[End of Traditional Chinese Readme]

