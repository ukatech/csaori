#include "csaori.h"
#include "winversion.h"
#include "cpuinfo.h"
using namespace std;

//------------------------------------------------------------------------------
//CSAORI
//------------------------------------------------------------------------------

// Static variables
// OS
static char osname[256];
static char osver[20];
static unsigned long osbuild;
static short osGot = 0;
// CPU
static unsigned long dwNumberOfProcessors = 0;


void CSAORI::exec(const CSAORIInput& in,CSAORIOutput& out)
{
	if (in.args.size() <= 0) {
		out.result_code = SAORIRESULT_NO_CONTENT;
		return;
	}

	if (in.args[0] == L"platform") {
		char *czPath = new char [_MAX_DIR];
		string platformname;
		GetModuleFileName(NULL , czPath, _MAX_DIR);
		platformname = czPath;
		delete czPath;
		int lastslash = platformname.find_last_of("\\");
		platformname = platformname.substr(lastslash+1);

		out.result_code = SAORIRESULT_OK;
		out.result = SAORI_FUNC::MultiByteToUnicode(platformname,0);
	}
	else if (in.args[0].find(L"mem.") != string::npos) {
		MEMORYSTATUS	meminfo;
		GlobalMemoryStatus(&meminfo);

		if (in.args[0] == L"mem.os") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(meminfo.dwMemoryLoad);
		}
		else if (in.args[0] == L"mem.phyt") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(meminfo.dwTotalPhys);
		}
		else if (in.args[0] == L"mem.phya") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(meminfo.dwAvailPhys);
		}
		else if (in.args[0] == L"mem.pagt") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(meminfo.dwTotalPageFile);
		}
		else if (in.args[0] == L"mem.paga") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(meminfo.dwAvailPageFile);
		}
		else if (in.args[0] == L"mem.virt") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(meminfo.dwTotalVirtual);
		}
		else if (in.args[0] == L"mem.vira") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(meminfo.dwAvailVirtual);
		}
		else {
			out.result_code = SAORIRESULT_BAD_REQUEST;
		}
	}
	else if (in.args[0].find(L"os.") != string::npos) {
		if(!osGot) {
			GetOSDisplayString(osname, osver, &osbuild);
			osGot = 1;
		}
		if (in.args[0] == L"os.name") {
			string osn = osname;
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::MultiByteToUnicode(osn);
		}
		else if (in.args[0] == L"os.version") {
			string osv = osver;
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::MultiByteToUnicode(osv);
		}
		else if (in.args[0] == L"os.build") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(osbuild);
		}
	}
	else if (in.args[0].find(L"cpu.") != string::npos) {
		if(!iCPUFlagsLoaded) {
			identifyCPU();

			SYSTEM_INFO stCpuInfo;
			GetSystemInfo(&stCpuInfo);
			dwNumberOfProcessors = stCpuInfo.dwNumberOfProcessors;
		}

		if (in.args[0] == L"cpu.num") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(dwNumberOfProcessors);
		}
		// CPUID
		else if (in.args[0] == L"cpu.vender") {
			string cpuv = (*sCPUVendor)?sCPUVendor:"<unknown>";
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::MultiByteToUnicode(cpuv);
		}
		else if (in.args[0] == L"cpu.name") {
			string cpuv = (*sCPUBranding)?sCPUBranding:"<No Name>";
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::MultiByteToUnicode(cpuv);
		}
		else if (in.args[0] == L"cpu.ptype") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(uBasicFlags.iProcessorType);
		}
		else if (in.args[0] == L"cpu.family") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(uBasicFlags.iExtendedFamilyID<<4 | uBasicFlags.iFamilyID);
		}
		else if (in.args[0] == L"cpu.model") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(uBasicFlags.iExtendedModelID<<4  | uBasicFlags.iModelID);
		}
		else if (in.args[0] == L"cpu.stepping") {
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::intToString(uBasicFlags.iSteppingID);
		}
		else if (in.args[0] == L"cpu.mmx") {
			out.result_code = SAORIRESULT_OK;
			out.result = uExt2Flags.iMMX ? L"Ready" : L"Not Ready";
		}
		else if (in.args[0] == L"cpu.sse") {
			out.result_code = SAORIRESULT_OK;
			out.result = uExt2Flags.iSSE ? L"Ready" : L"Not Ready";
		}
		else if (in.args[0] == L"cpu.sse2") {
			out.result_code = SAORIRESULT_OK;
			out.result = uExt2Flags.iSSE2 ? L"Ready" : L"Not Ready";
		}
		else if (in.args[0] == L"cpu.tdn") {
			out.result_code = SAORIRESULT_OK;
			out.result = u8Ext2Flags.i3DN ? L"Ready" : L"Not Ready";
		}
		else if (in.args[0] == L"cpu.mmx+") {
			out.result_code = SAORIRESULT_OK;
			out.result = u8Ext2Flags.iMMXEx ? L"Ready" : L"Not Ready";
		}
		else if (in.args[0] == L"cpu.tdn+") {
			out.result_code = SAORIRESULT_OK;
			out.result = u8Ext2Flags.i3DNEx ? L"Ready" : L"Not Ready";
		}
		else if (in.args[0] == L"cpu.htt") {
			out.result_code = SAORIRESULT_OK;
			out.result = uExt2Flags.iHTT ? L"Ready" : L"Not Ready";
		}
		else if (in.args[0] == L"cpu.features") {
			out.result = L"";
			out.result_code = SAORIRESULT_OK;

			if( uExt2Flags.iFPU  ) out.result.append(L"FPU ");	// Integrated FPU
			if( uExt2Flags.iVME  ) out.result.append(L"VME ");	// Virtual Memory Extensions
			if( uExt2Flags.iDE   ) out.result.append(L"DE ");	// Debugging extensions
			if( uExt2Flags.iPSE  ) out.result.append(L"PSE ");	// Page Size Extensions
			if( uExt2Flags.iTSC  ) out.result.append(L"TSC ");	// Time Stamp COunter
			if( uExt2Flags.iMSR  ) out.result.append(L"MSR ");	// Model specific registers, RDMSR/WRMSR instructions
			if( uExt2Flags.iPAE  ) out.result.append(L"PAE ");	// Physical Address Extension
			if( uExt2Flags.iMCE  ) out.result.append(L"MCE ");	// Machine Check Exception
			if( uExt2Flags.iCX8  ) out.result.append(L"CX8 ");	// CMPXCHG8B instruction
			if( uExt2Flags.iAPIC ) out.result.append(L"APIC ");	// On-chip APIC
			if( uExt2Flags.iSEP  ) out.result.append(L"SEP ");	// SYSENTER and SYSEXIT instructions and MSRs
			if( uExt2Flags.iMTRR ) out.result.append(L"MTRR ");	// Memory Type Range Registers
			if( uExt2Flags.iPGE  ) out.result.append(L"PGE ");	// PTE Global Bit
			if( uExt2Flags.iMCA  ) out.result.append(L"MCA ");	// Machine Check Architecture
			if( uExt2Flags.iCMOV ) out.result.append(L"CMOV ");	// Conditional MOV instruction
			if( uExt2Flags.iPAT  ) out.result.append(L"PAT ");	// Page Attribute Table
			if( uExt2Flags.iPSE36) out.result.append(L"PSE36 ");	// 36-bit page size extension
			if( uExt2Flags.iPSN  ) out.result.append(L"PSN ");	// Processor serial number
			if( uExt2Flags.iCFLSH) out.result.append(L"CFLSH ");	// CLFLUSH instruction
			if( uExt2Flags.iDS   ) out.result.append(L"DS ");	// Debug Store
			if( uExt2Flags.iACPI ) out.result.append(L"ACPI ");	// Thermal monitor and software-controlled-clock functions
			if( uExt2Flags.iMMX  ) out.result.append(L"MMX ");	// MMX present
			if( uExt2Flags.iFXSR ) out.result.append(L"FXSR ");	// FXSAVE and FXRESTORE instructions
			if( uExt2Flags.iSSE  ) out.result.append(L"SSE ");	// Streaming SIMD Extensions
			if( uExt2Flags.iSSE2 ) out.result.append(L"SSE2 ");	// Streaming SIMD Extensions 2 present
			if( uExtFlags.iSSE3  ) out.result.append(L"SSE3 ");	// Streaming SIMD Extensions 3 present
			if( uExtFlags.iSSSE3  ) out.result.append(L"SSSE3 ");	// Supplemental Streaming SIMD Extensions 3 present
			if( uExtFlags.iSSE41  ) out.result.append(L"SSE4.1 ");	// Streaming SIMD Extensions 4.1 present
			if( uExtFlags.iSSE42  ) out.result.append(L"SSE4.2 ");	// Streaming SIMD Extensions 4.2 present
			if( u8ExtFlags.iSSE4A ) out.result.append(L"SSE4A ");	// Streaming SIMD Extensions 4A present
			if( u8ExtFlags.imSSE ) out.result.append(L"maSSE ");	// Misaligned SSE
			if( u8Ext2Flags.i3DN ) out.result.append(L"3DNow! ");	// 3DNow!
			if( u8Ext2Flags.i3DNEx ) out.result.append(L"3DNow!+ ");	// 3DNow!+
			if( u8Ext2Flags.iMMXEx ) out.result.append(L"MMX+ ");	// MMX+
			if( uExt2Flags.iSS   ) out.result.append(L"SS ");	// Self Snoop
			if( uExt2Flags.iHTT  ) out.result.append(L"HTT ");	// Hyperthreading
			if( uExt2Flags.iTM   ) out.result.append(L"TM ");	// Thermal Monitoring
			if( uExtFlags.iTM2 ) out.result.append(L"TM2 ");	// Thermal Monitoring 2
			if( uExt2Flags.iPBE  ) out.result.append(L"PBE ");	// Pending Break Enable (FERR#/PBE# pins)
			if( uExtFlags.iMWAIT ) out.result.append(L"MWAIT ");	// MONITOR/MWAIT instructions
			if( uExtFlags.iQDS ) out.result.append(L"QDS ");	// CPL Qualified Debug Store present
			if( uExtFlags.iEST ) out.result.append(L"EST ");	// Enhanced SpeedStep
			if( uExtFlags.iCID ) out.result.append(L"CID ");	// Context-ID
			if( uExtFlags.iVMX ) out.result.append(L"VMX ");	// Virtual Machine Extensions
			if( u8ExtFlags.iSVM ) out.result.append(L"SVM ");	// LAHF
			if( u8Ext2Flags.iLM ) out.result.append(L"x86-64 ");	// 64-bit technology
			if( u8Ext2Flags.iNX ) out.result.append(L"NX ");	// No Execute
			if( u8ExtFlags.iLAHF ) out.result.append(L"LAHF ");	// LAHF
		}
		else if (in.args[0] == L"cpu.cache") {
			char tmptxt[30];
			string outtxt;
			sprintf(tmptxt,"%d %d %d %d",uL1DCSize.iL1DCSize,uL1ICSize.iL1ICSize,uL2Size.iL2Size,uL3Size.iL3Size);
			outtxt = tmptxt;
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::MultiByteToUnicode(outtxt);
		}
		// CPU Speed
		else if (in.args[0] == L"cpu.clock") {
			char tmptxt[10];
			string outtxt;
			sprintf(tmptxt,"%d",(int)GetCPUSpeed(100));
			outtxt = tmptxt;
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::MultiByteToUnicode(outtxt);
		}
		else if (in.args[0] == L"cpu.clockex") {
			char tmptxt[10];
			string outtxt;
			sprintf(tmptxt,"%.3f",GetCPUSpeed(250));
			outtxt = tmptxt;
			out.result_code = SAORIRESULT_OK;
			out.result = SAORI_FUNC::MultiByteToUnicode(outtxt);
		}
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

