[UTF-8 Encoding]

[English]
UDS(UpDownSpeed) is a SAORI plugin for ukagaka.

Usage:

- List all network interface:
* Input
 * Argument0: ListInt
 * Argument1: 0 for Shiori that supports Value[n], 1(non-zero) for Shiori that not support Value[n].
* Output
 * If Argument1 = 0:
  - Result: OK
  - Value[n]: Interface name
 * If Argument1 = non-zero:
  - Result: String of Index and Interface name in "[idx]: [name]\n" style
 * If failed
  - Result: NG
  - Value0: Error. RetCode=<code>

- Get Up/down speed
* Input
 * Argument0: UpDownSpeed
 * Argument1: Interface index (start from 0, see "List all network interface")
 * Argument2: measurement (1 to 20, bigger number for faster response but less accurate)
 * Argument3: 0 for Shiori that supports Value[n], 1(non-zero) for Shiori that not support Value[n].
* Output
 * If Argument3 = 0:
  - Result: OK
  - Value0: Download speed (bytes per scrond)
  - Value1: Upload speed (bytes per scrond)
 * If Argument3 = non-zero:
  - Result: String of Up/Down speed in "Down: [down_speed] Bps, Up: [up_speed] Bps." style
 * If failed
  - Result: NG
  - Value0: Error. RetCode=<code>

Known Issue:
(currently none)

Version History:
 20080531-1359:
 - Resolve "Error loading Iphlpapi.dll" after first call.
 - Resolve failure of loading in yaya shori.
 20070804-1028:
 - Resolves "Get Up/down speed needs 1 second to measure."
 20070804-0224:
 - Initial Version

Credits:

UDS (UpDownSpeed) SAORI is modified from KSM's SAORI Sample.

IP helper code snippets are taken from:
Retrieve the number of bytes sent/received (and other useful infos) for any network adapter
http://www.codeproject.com/internet/BytesSentReceived.asp

Copyright(C) Roy 2007, 2008. Some Rights Reserved.
Licence: CC-SA 3.0 ( http://creativecommons.org/licenses/by-sa/3.0/deed.zh_TW )

Author's website:  某廢柴的廢棄日記 ( http://scrappedblog.blogspot.com/ )

[End of English Readme]

[Traditional Chinese / 繁體中文(zh_TW)]
UDS(UpDownSpeed) 是一個伺か(ukagaka)的 SAORI 插件。

使用方法:

- 列出所有網卡：
* 輸入
 * Argument0: ListInt
 * Argument1: 當Shiori支援Value[n]時應為 0 ，否則為非 0。
* 輸出
 * 如果 Argument1 為 0:
  - Result: OK
  - Value[n]: 網卡名稱
 * 如果 Argument1 為非 0:
  - Result: 一段含有網卡名稱和索引鍵且格式為 "[idx]: [name]\n" 的字串
 * 如果函數失敗
  - Result: NG
  - Value0: Error. RetCode=<code>

- 獲得上載/下載速度
* 輸入
 * Argument0: UpDownSpeed
 * Argument1: 網卡索引鍵 (由 0 開始的索引鍵, 詳見 「列出所有網卡」)
 * Argument2: 量度係數。 (1 至 20，係數愈大反應愈快但是愈不準確。)
 * Argument3: 當Shiori支援Value[n]時應為 0 ，否則為非 0。
* 輸出
 * 如果 Argument3 為 0:
  - Result: OK
  - Value0: 下載速度 (位元組/每秒)
  - Value1: 上載速度 (位元組/每秒)
 * 如果 Argument3 為非 0:
  - Result: 一段含有上載/下載速度且格式為 "Down: [down_speed] Bps, Up: [up_speed] Bps." 的字串
 * 如果函數失敗
  - Result: NG
  - Value0: Error. RetCode=<code>

已知問題:
(暫時沒有)

版本紀錄:
 20080531-1359:
 - 修正首次呼叫後出現「Error loading Iphlpapi.dll」的問題。
 - 修正 yaya 中無法載入的問題。
 20070804-1028:
 - 修正「獲得上載/下載速度需用一秒時間量度。」的問題。
 20070804-0224:
 - 初版


鳴謝:

UDS (UpDownSpeed) SAORI 是由 kousuimao(KSM) 的 SAORI 範例中修改出來的。

IP helper 程式片段是由這裡抽取使用的:
Retrieve the number of bytes sent/received (and other useful infos) for any network adapter
http://www.codeproject.com/internet/BytesSentReceived.asp

版權所有(C) Roy 2007, 2008。 保留部份版權。
授權條款: 創用CC-SA 3.0 ( http://creativecommons.org/licenses/by-sa/3.0/deed.zh_TW )

作者網站:  某廢柴的廢棄日記 ( http://scrappedblog.blogspot.com/ )

[End of Traditional Chinese Readme]

