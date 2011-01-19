[UTF-8 English]
CCPUID (CSAORI saori_cpuid)

CCPUID is a saori_cpuid rewritten by using CSAORI.
CCPUID is fully compatible with saori_cpuid.dll by Ewi Nanase, see saori_cpuid_README.txt for details.

New Functions:
cpu.features - list all CCPUID known CPU features (short name) with space separated.
cpu.cache - return CPU caches sizes in KB with space separated, format: "L1-Data L1-Inst L2 L3"
cpu.usage - return CPU usage (0 - 100)
mem.viraex - return available extended virtual memory (over 4GB memory) (by ukiya)
cpu.logicalprocessors - return logical processor count
cpu.vmguest - return if it is in Virtual Machine or not. (Supports VMWare 7, KVM, Xen HVM)
cpu.vmbrand - return brand name of Virtual Machine
dmi.bios.vendor - return DMI BIOS Vendor string
dmi.bios.version - return DMI BIOS Version string
dmi.bios.releasedate - return DMI BIOS Release Date string
dmi.system.manufacturer - return DMI System Manufacturer string
dmi.system.productname - return DMI System Product Name string
dmi.system.version - return DMI System Version string
dmi.system.sn - return DMI System Serial Number string
dmi.mb.manufacturer - return DMI Mainboard Manufacturer string
dmi.mb.product - return DMI Mainboard Product Name string
dmi.mb.version - return DMI Mainboard Version string
dmi.mb.sn - return DMI Mainboard Serial Number string
dmi.chassis.manufacturer - return DMI Chassis Manufacturer string
dmi.chassis.version - return DMI Chassis Version string
dmi.chassis.sn - return DMI Chassis Serial Number string
dmi.chassis.product - return DMI Chassis Asset Tag string

cpu.ptype return value supplementation:
0: Original OEM processor
1: Intel(R) OverDrive(R) processor
2: Dual processor(Not applicable to Intel486 processors)
3: Intel(R) reserved

dmi.* functions supplementation:
They will work in NT based systems (NT/2000/XP/2003/Vista/7) only.
Win9X is not supported. It may require Administrator right in order to access physical menory.

Version History:
	- add dmi.* functions
	- add cpu.vmguest, cpu.vmbrand
	- add cpu.logicalprocessors
	- add Intel Brand Index detection
	- add Cache size detection of Intel CPUs
20100420 - Release 4
	- add mem.viraex
	- add "cpu.usage" function
	- remove "Microsoft " from os.name
	- fix mem.*(except mem.os) to match description in saori_cpuid_README.txt (All in MB, not in bytes)
20090903 - Release 3
	- add fallback when branding can't be retrieved from CPUID
	- fix: don't try to retrieve CPUID information which is higher than reported highest CPUID
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
CPUID Sample: Determines CPU Capabilities
 http://msdn.microsoft.com/en-us/library/xs6aek1h(VS.80).aspx
如何获得CPU使用率 (How to get CPU usage)
 http://www.4oa.com/Article/html/6/33/487/2005/17522.html

[UTF-8 Chinese Traditional]
ccpuid是用CSAORI把saori_cpuid的功能重寫。
而這次是完全跟saori_cpuid兼容的，再加上新指令。

新指令：
cpu.features - 列出所有CCPUID已知的CPU功能的簡寫，並以空格隔開回傳。
cpu.cache - 回傳所有 CPU 的快取大小 (KB) 並以空格隔開，格式: "L1-Data L1-Inst L2 L3"
cpu.usage - 回傳 CPU 使用率 (0 - 100)
mem.viraex - 回傳可用的延伸虛擬記憶體 (超過 4GB 的虛擬記憶體) (ukiya)
cpu.logicalprocessors - 回傳邏輯處理器數目

cpu.ptype 回傳值補充:
0: Original OEM processor
1: Intel(R) OverDrive(R) processor
2: Dual processor(Not applicable to Intel486 processors)
3: Intel(R) reserved

版本歷史:
	- 新增 cpu.logicalprocessors
	- 新增 Intel 品牌號偵測
	- 新增 Intel CPU 的快取大小偵測
20100420 - Release 4
	- 新增 mem.viraex
	- 新增 "cpu.usage" 功能
	- 自 os.name 中移除 "Microsoft "
	- 修正 mem.*(除 mem.os) 讓功能和 saori_cpuid_README.txt 中的描述一致(皆為 MB，非位元組)
20090903 - Release 3
	- 新增當 CCPUID 不能從 CPUID 中獲得 CPU 名稱時的應變策略
	- 修正：不嘗試獲得高於最高 CPUID 代碼的 CPUID 資訊
20090831 - Release 2
	- 新增 "cpu.cache" 功能
	- 修正 cpu.clockex 回傳 ".000" 為結尾的問題
20090830 - Release 1
	- 初版

鳴謝:
CCPUID 由 Roy Tam 所寫。
部份源程式碼修改自以下的代碼：
 CPUID.C - V0.2 - Jim Howes <jimhowes@ (exactly the same again).net>
CPU Speed 功能源自:
 http://www.rohitab.com/discuss/lofiversion/index.php/t26562.html
Getting the System Version:
 http://msdn.microsoft.com/en-us/library/ms724429(VS.85).aspx
CPUID Sample: Determines CPU Capabilities
 http://msdn.microsoft.com/en-us/library/xs6aek1h(VS.80).aspx
如何获得CPU使用率
 http://www.4oa.com/Article/html/6/33/487/2005/17522.html