[UTF-8 English]
CCPUID (CSAORI saori_cpuid)

CCPUID is a saori_cpuid rewritten by using CSAORI.
CCPUID is fully compatible with saori_cpuid.dll by Ewi Nanase, see saori_cpuid_README.txt for details.

New Functions:
cpu.features - list all CCPUID known CPU features (short name) with space separated.
cpu.cache - return CPU caches sizes in KB with space separated, format: "L1-Data L1-Inst L2 L3"

Version History:
20090831 - Release 2
	- add "cpu.cache" function
	- fix cpu.clockex always return with ".000" ending problem
20090830 - Release 1
	- initial release

Credits:
CCPUID by Roy Tam.
Part of code are modified from the orginal codes:
 CPUID.C - V0.2 - Jim Howes <jimhowes@ (exactly the same again).net>
CPU Speed functions are copied from:
 http://www.rohitab.com/discuss/lofiversion/index.php/t26562.html
Getting the System Version:
 http://msdn.microsoft.com/en-us/library/ms724429(VS.85).aspx

[UTF-8 Chinese Traditional]
ccpuid是用CSAORI把saori_cpuid的功能重寫。
而這次是完全跟saori_cpuid兼容的，再加上新指令。

新指令：
cpu.features - 列出所有CCPUID已知的CPU功能的簡寫，並以空格隔開回傳。
cpu.cache - 回傳所有 CPU 的快取大小 (KB) 並以空格隔開，格式: "L1-Data L1-Inst L2 L3"

版本歷史:
20090831 - Release 2
	- 新增 "cpu.cache" 功能
	- 修正 cpu.clockex 回傳 ".000" 為結尾的問題
20090830 - Release 1
	- 初版

鳴謝:
CCPUID 由 Roy Tam 所寫。
部份源程式碼改自以下的代碼：
 CPUID.C - V0.2 - Jim Howes <jimhowes@ (exactly the same again).net>
CPU Speed 功能源自:
 http://www.rohitab.com/discuss/lofiversion/index.php/t26562.html
Getting the System Version:
 http://msdn.microsoft.com/en-us/library/ms724429(VS.85).aspx
