#include "csaori.h"
#include "winversion.h"
#include "cpuinfo.h"
#include "cpuusage.h"

//------------------------------------------------------------------------------
//CSAORI
//------------------------------------------------------------------------------

bool GetInfoFromString(const string_t &in,string_t &out);

void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	if (in.args.size() <= 0) {
		out.result_code = SAORIRESULT_NO_CONTENT;
		return;
	}

	size_t n = in.args.size();

	string_t outs;
	size_t success_count = 0;

	for ( size_t i = 0 ; i < n ; ++i ) {
		outs = L"";
		if ( GetInfoFromString(in.args[i],outs) ) {
			++success_count;
		}
		out.values.push_back(outs);
	}

	if ( success_count ) {
		out.result_code = SAORIRESULT_OK;
		out.result = out.values[0];
		if ( out.result[0] == L'!' ) {
			out.result.erase(0,1);
		}
	}
	else {
		out.result_code = SAORIRESULT_BAD_REQUEST;
		out.result = L"param error";
	}

}

bool CSAORI::load()
{
	return true;
}

bool CSAORI::unload()
{
	return true;
}

bool GetInfoFromString(const string_t &in,string_t &out)
{
	// Static variables
	// OS
	static char osname[256];
	static char osver[20];
	static unsigned long osbuild;
	static short osGot = 0;
	// CPU
	static unsigned long dwNumberOfProcessors = 0;

	// saori_cpuid compatible commands
	if (in == L"platform") {				// platform: basename of caller exe name
		char *czPath = new char [_MAX_DIR];
		std::string platformname;
		GetModuleFileName(NULL , czPath, _MAX_DIR);
		platformname = czPath;
		delete czPath;
		int lastslash = platformname.find_last_of("\\");
		platformname = platformname.substr(lastslash+1);

		out = SAORI_FUNC::MultiByteToUnicode(platformname,0);
		return true;
	}

	if (in.find(L"mem.") != std::string::npos) { // memory commands
		MEMORYSTATUSEX	meminfoex;
		ZeroMemory(&meminfoex,sizeof(meminfoex));
		meminfoex.dwLength=sizeof(meminfoex);

		//for over 4GB(3GB?) Memory
		//if GlobalMemoryStatusEx exists, use it otherwise use GlobalMemoryStatus
		BOOL (__stdcall*GMSEx)(LPMEMORYSTATUSEX) = 0;

		HINSTANCE hIL = ::LoadLibrary("kernel32.dll");
		GMSEx = (BOOL(__stdcall*)(LPMEMORYSTATUSEX))::GetProcAddress(hIL, "GlobalMemoryStatusEx");

		if(GMSEx){
			GMSEx(&meminfoex);
		}else{
			MEMORYSTATUS	meminfo;
			ZeroMemory(&meminfo,sizeof(meminfo));
			::GlobalMemoryStatus(&meminfo);
			meminfoex.dwMemoryLoad = meminfo.dwMemoryLoad;
			meminfoex.ullTotalPhys = meminfo.dwTotalPhys;
			meminfoex.ullAvailPhys = meminfo.dwAvailPhys;
			meminfoex.ullTotalPageFile = meminfo.dwTotalPageFile;
			meminfoex.ullAvailPageFile = meminfo.dwAvailPageFile;
			meminfoex.ullTotalVirtual = meminfo.dwTotalVirtual;
			meminfoex.ullAvailVirtual = meminfo.dwAvailVirtual;
			meminfoex.ullAvailExtendedVirtual = 0;
		}

		::FreeLibrary(hIL);

		if (in == L"mem.os") {				// mem.os: OS memory usage (0 - 100)
			out = SAORI_FUNC::numToString(meminfoex.dwMemoryLoad);
			return true;
		}
		if (in == L"mem.phyt") {		// mem.phyt: Total Physical Memory
			out = SAORI_FUNC::numToString(meminfoex.ullTotalPhys/1048576);
			return true;
		}
		if (in == L"mem.phya") {		// mem.phya: Available Physical Memory
			out = SAORI_FUNC::numToString(meminfoex.ullAvailPhys/1048576);
			return true;
		}
		if (in == L"mem.pagt") {		// mem.pagt: Total Page file Size
			out = SAORI_FUNC::numToString(meminfoex.ullTotalPageFile/1048576);
			return true;
		}
		if (in == L"mem.paga") {		// mem.paga: Available Page file Size
			out = SAORI_FUNC::numToString(meminfoex.ullAvailPageFile/1048576);
			return true;
		}
		if (in == L"mem.virt") {		// mem.virt: Total Virtual Memory
			out = SAORI_FUNC::numToString(meminfoex.ullTotalVirtual/1048576);
			return true;
		}
		if (in == L"mem.vira") {		// mem.vira: Available Virtual Memory
			out = SAORI_FUNC::numToString(meminfoex.ullAvailVirtual/1048576);
			return true;
		}
		if (in == L"mem.viraex") {		// mem.vira: Available Extended Virtual Memory
			out = SAORI_FUNC::numToString(meminfoex.ullAvailExtendedVirtual/1048576);
			return true;
		}

		out = L"!param error";
		return false;
	}

	if (in.find(L"os.") != std::string::npos) { // OS comands
		if(!osGot) {
			GetOSDisplayString(osname, osver, &osbuild);
			osGot = 1;
		}
		if (in == L"os.name") {				// os.name: OS Name
			out = SAORI_FUNC::MultiByteToUnicode(osname);
			return true;
		}
		if (in == L"os.version") {		// os.version: OS Version
			out = SAORI_FUNC::MultiByteToUnicode(osver);
			return true;
		}
		if (in == L"os.build") {		// os.build: OS Build number
			out = SAORI_FUNC::intToString(osbuild);
			return true;
		}

		out = L"!param error";
		return false;
	}

	if (in.find(L"cpu.") != std::string::npos) { // CPU commands
		if(!iCPUFlagsLoaded) {
			identifyCPU();

			SYSTEM_INFO stCpuInfo;
			GetSystemInfo(&stCpuInfo);
			dwNumberOfProcessors = stCpuInfo.dwNumberOfProcessors;
		}

		if (in == L"cpu.num") {				// cpu.num: CPU/Core Numbers

			out = SAORI_FUNC::intToString(dwNumberOfProcessors);
			return true;
		}
		// CPUID
		if (in == L"cpu.vender" || in == L"cpu.vendor" ) {		// cpu.vender: CPU Vender Name
			std::string cpuv = (*sCPUVendor)?sCPUVendor:"<unknown>";

			out = SAORI_FUNC::MultiByteToUnicode(cpuv);
			return true;
		}
		if (in == L"cpu.name") {		// cpu.name: CPU Name
			std::string cpuv = (*sCPUBranding)?sCPUBranding:"<No Name>";

			out = SAORI_FUNC::MultiByteToUnicode(cpuv);
			return true;
		}
		if (in == L"cpu.ptype") {		// cpu.ptype: CPU Processor Type (Intel)

			out = SAORI_FUNC::intToString(uBasicFlags.iProcessorType);
			return true;
		}
		if (in == L"cpu.family") {		// cpu.family: CPU Family number

			out = SAORI_FUNC::intToString(uBasicFlags.iExtendedFamilyID<<4 | uBasicFlags.iFamilyID);
			return true;
		}
		if (in == L"cpu.model") {		// cpu.model: CPU Model number

			out = SAORI_FUNC::intToString(uBasicFlags.iExtendedModelID<<4  | uBasicFlags.iModelID);
			return true;
		}
		if (in == L"cpu.stepping") {	// cpu.stepping: CPU Stepping Number

			out = SAORI_FUNC::intToString(uBasicFlags.iSteppingID);
			return true;
		}
		if (in == L"cpu.mmx") {		// cpu.mmx: If CPU support MMX

			out = uExt2Flags.iMMX ? L"Ready" : L"Not Ready";
			return true;
		}
		if (in == L"cpu.sse") {		// cpu.sse: If CPU support SSE

			out = uExt2Flags.iSSE ? L"Ready" : L"Not Ready";
			return true;
		}
		if (in == L"cpu.sse2") {		// cpu.sse2: If CPU support SSE2

			out = uExt2Flags.iSSE2 ? L"Ready" : L"Not Ready";
			return true;
		}
		if (in == L"cpu.tdn") {		// cpu.tdn: If CPU support 3DNow!

			out = u8Ext2Flags.i3DN ? L"Ready" : L"Not Ready";
			return true;
		}
		if (in == L"cpu.mmx+") {		// cpu.mmx+: If CPU support MMX+

			out = u8Ext2Flags.iMMXEx ? L"Ready" : L"Not Ready";
			return true;
		}
		if (in == L"cpu.tdn+") {		// cpu.tdn+: If CPU support 3DNow!+

			out = u8Ext2Flags.i3DNEx ? L"Ready" : L"Not Ready";
			return true;
		}
		if (in == L"cpu.htt") {		// cpu.htt: If CPU support Hyper Thread Technology

			out = uExt2Flags.iHTT ? L"Ready" : L"Not Ready";
			return true;
		}
		// New CPU functions
		if (in == L"cpu.features") {	// cpu.features: List all known CPU features (space separated)
			out = L"";

			if(  uExt2Flags.iFPU  ) out.append(L"FPU ");		// Integrated FPU
			if(  uExt2Flags.iVME  ) out.append(L"VME ");		// Virtual Memory Extensions
			if(  uExt2Flags.iDE   ) out.append(L"DE ");			// Debugging extensions
			if(  uExt2Flags.iPSE  ) out.append(L"PSE ");		// Page Size Extensions
			if(  uExt2Flags.iTSC  ) out.append(L"TSC ");		// Time Stamp COunter
			if(  uExt2Flags.iMSR  ) out.append(L"MSR ");		// Model specific registers, RDMSR/WRMSR instructions
			if(  uExt2Flags.iPAE  ) out.append(L"PAE ");		// Physical Address Extension
			if(  uExt2Flags.iMCE  ) out.append(L"MCE ");		// Machine Check Exception
			if(  uExt2Flags.iCX8  ) out.append(L"CX8 ");		// CMPXCHG8B instruction
			if(  uExt2Flags.iAPIC ) out.append(L"APIC ");		// On-chip APIC
			if(  uExt2Flags.iSEP  ) out.append(L"SEP ");		// SYSENTER and SYSEXIT instructions and MSRs
			if(  uExt2Flags.iMTRR ) out.append(L"MTRR ");		// Memory Type Range Registers
			if(  uExt2Flags.iPGE  ) out.append(L"PGE ");		// PTE Global Bit
			if(  uExt2Flags.iMCA  ) out.append(L"MCA ");		// Machine Check Architecture
			if(  uExt2Flags.iCMOV ) out.append(L"CMOV ");		// Conditional MOV instruction
			if(  uExt2Flags.iPAT  ) out.append(L"PAT ");		// Page Attribute Table
			if(  uExt2Flags.iPSE36) out.append(L"PSE36 ");		// 36-bit page size extension
			if(  uExt2Flags.iPSN  ) out.append(L"PSN ");		// Processor serial number
			if(  uExt2Flags.iCFLSH) out.append(L"CFLSH ");		// CLFLUSH instruction
			if(  uExt2Flags.iDS   ) out.append(L"DS ");			// Debug Store
			if(  uExt2Flags.iACPI ) out.append(L"ACPI ");		// Thermal monitor and software-controlled-clock functions
			if(  uExt2Flags.iMMX  ) out.append(L"MMX ");		// MMX present
			if(  uExt2Flags.iFXSR ) out.append(L"FXSR ");		// FXSAVE and FXRESTORE instructions
			if(  uExt2Flags.iSSE  ) out.append(L"SSE ");		// Streaming SIMD Extensions
			if(  uExt2Flags.iSSE2 ) out.append(L"SSE2 ");		// Streaming SIMD Extensions 2 present
			if(   uExtFlags.iSSE3 ) out.append(L"SSE3 ");		// Streaming SIMD Extensions 3 present
			if(   uExtFlags.iSSSE3) out.append(L"SSSE3 ");		// Supplemental Streaming SIMD Extensions 3 present
			if(   uExtFlags.iSSE41) out.append(L"SSE4.1 ");		// Streaming SIMD Extensions 4.1 present
			if(   uExtFlags.iSSE42) out.append(L"SSE4.2 ");		// Streaming SIMD Extensions 4.2 present
			if(   uExtFlags.iAES  ) out.append(L"AES ");		// AES Instruction Extensions present
			if(   uExtFlags.iAVX  ) out.append(L"AVX ");		// Advanced Vector Extensions present
			if(  u8ExtFlags.iSSE4A) out.append(L"SSE4A ");		// Streaming SIMD Extensions 4A present
			if(  u8ExtFlags.imSSE ) out.append(L"maSSE ");		// Misaligned SSE
			if( u8Ext2Flags.i3DN  ) out.append(L"3DNow! ");		// 3DNow!
			if( u8Ext2Flags.i3DNEx) out.append(L"3DNow!+ ");	// 3DNow!+
			if( u8Ext2Flags.iMMXEx) out.append(L"MMX+ ");		// MMX+
			if(  uExt2Flags.iSS   ) out.append(L"SS ");			// Self Snoop
			if(  uExt2Flags.iHTT  ) out.append(L"HTT ");		// Hyperthreading
			if(  uExt2Flags.iTM   ) out.append(L"TM ");			// Thermal Monitoring
			if(   uExtFlags.iTM2  ) out.append(L"TM2 ");		// Thermal Monitoring 2
			if(  uExt2Flags.iPBE  ) out.append(L"PBE ");		// Pending Break Enable (FERR#/PBE# pins)
			if(   uExtFlags.iMWAIT) out.append(L"MWAIT ");		// MONITOR/MWAIT instructions
			if(   uExtFlags.iQDS  ) out.append(L"QDS ");		// CPL Qualified Debug Store present
			if(   uExtFlags.iEST  ) out.append(L"EIST ");		// Enhanced SpeedStep
			if(   uExtFlags.iCID  ) out.append(L"CID ");		// Context-ID
			if(   uExtFlags.iVMX  ) out.append(L"VMX ");		// Virtual Machine Extensions
			if(  u8ExtFlags.iSVM  ) out.append(L"SVM ");		// SVM
			if( u8Ext2Flags.iLM   ) out.append(L"x86-64 ");		// 64-bit technology
			if( u8Ext2Flags.iNX   ) out.append(L"NX ");			// No Execute
			if(  u8ExtFlags.iLAHF ) out.append(L"LAHF ");		// LAHF
			if(   uExtFlags.iVirt ) out.append(L"VIRT ");		// Virtualized Environment

			return true;
		}
		if (in == L"cpu.cache") {		// cpu.cache: List CPU Caches in "L1-DataCache L1-InstCache L2 L3" form (without quote)
			char tmptxt[30];
			std::string outtxt;
			sprintf(tmptxt,"%d %d %d %d",uL1DCSize.iL1DCSize,uL1ICSize.iL1ICSize,uL2Size.iL2Size,uL3Size.iL3Size);
			outtxt = tmptxt;

			out = SAORI_FUNC::MultiByteToUnicode(outtxt);
			return true;
		}

		if (in == L"cpu.logicalprocessors") {		// cpu.logicalprocessors: CPU Logical Processors

			out = uBasic2Flags.iLogicalProcessors ? SAORI_FUNC::intToString(uBasic2Flags.iLogicalProcessors) : L"1";
			return true;
		}

		if (in == L"cpu.usage") {		// cpu.usage: Return CPU Usage

			out = SAORI_FUNC::intToString(CPUUsage());
			return true;
		}

		if (in == L"cpu.vmguest") {		// cpu.vmguest: Is running in virtualized environment
			out = uExtFlags.iVirt ? L"Yes" : L"No";
			return true;
		}
		if (in == L"cpu.vmbrand") {		// cpu.vmbrand: Virtualized environment brand name
			std::string vmbrand = (*sVMBranding)?sVMBranding:(uExtFlags.iVirt ? "<No Name>" : "Not in VM");

			out = SAORI_FUNC::MultiByteToUnicode(vmbrand);
			return true;
		}

		// CPU Speed functions (saori_cpuid compatible)
		if (in == L"cpu.clock") {		// cpu.clock: measure CPU speed in 100ms, return CPU speed in MHz (integer)
			char tmptxt[10];
			std::string outtxt;
			sprintf(tmptxt,"%d",(int)GetCPUSpeed(100));
			outtxt = tmptxt;

			out = SAORI_FUNC::MultiByteToUnicode(outtxt);
			return true;
		}

		if (in == L"cpu.clockex") {	// cpu.clockex: measure CPU speed in 250ms, return CPU speed in MHz (3 decimal)
			char tmptxt[10];
			std::string outtxt;
			sprintf(tmptxt,"%.3f",GetCPUSpeed(250));
			outtxt = tmptxt;

			out = SAORI_FUNC::MultiByteToUnicode(outtxt);
			return true;
		}

		out = L"!param error";
		return false;
	}


	out = L"!param error";
	return false;
}
