/*
	CPUID.C - V0.2 - Jim Howes <jimhowes@ (exactly the same again).net>

	This code comes with NO WARRANTY.
	If it breaks you get to keep all of the pieces.

	This code may look like it was knocked up in ten minutes by
	a crazed psychopathic C coder
	This is because that is precisely how it was created.

	The _asm syntax is suitable for MSVC 6.0

	In a SMP system, there is no way (in this code) to tell which CPU
	you are physically running on.  In an Intel SMP system this probably
	does not matter, as all CPUs should be the same model and stepping.

	Running this code on anything prior to (whatever first responded
	to 0x80000003 in EAX on CPUID) will probably do wierd things.
*/

// CPU Speed functions are copied from:
// http://www.rohitab.com/discuss/lofiversion/index.php/t26562.html
#include "cpuinfo.h"

int		iCPUFlagsLoaded = 0;
CPUID_01_EAX_t	uBasicFlags;
CPUID_01_ECX_t	uExtFlags;
CPUID_01_EDX_t	uExt2Flags;
CPUID_80000001_ECX_t	u8ExtFlags;
CPUID_80000001_EDX_t	u8Ext2Flags;
char		sCPUBranding[65];
char		sCPUVendor[16];

int identifyCPU()
{
	unsigned long *uBrand = (unsigned long *)sCPUBranding;
	unsigned long u0, u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11;

	iCPUFlagsLoaded = 1;
	memset(sCPUBranding,0,65);
	memset(sCPUVendor,0,16);

	_asm {
		mov      eax, 0
		cpuid
		mov     DWORD PTR [sCPUVendor+0],ebx  // Stash the manufacturer string for later
		mov     DWORD PTR [sCPUVendor+4],edx
		mov     DWORD PTR [sCPUVendor+8],ecx
	}

	_asm {
		mov	eax, 01h
		cpuid
		mov	uBasicFlags,eax
		mov	uExtFlags,ecx
		mov	uExt2Flags,edx
		mov eax, 80000001h
		cpuid
		mov	u8ExtFlags,ecx
		mov	u8Ext2Flags,edx
	}

	_asm {
		mov eax, 80000002h
		cpuid
		mov u0,eax
		mov u1,ebx
		mov u2,ecx
		mov u3,edx
		mov eax, 80000003h
		cpuid
		mov u4,eax
		mov u5,ebx
		mov u6,ecx
		mov u7,edx
		mov eax, 80000004h
		cpuid
		mov u8,eax
		mov u9,ebx
		mov u10,ecx
		mov u11,edx
	}
	uBrand[0] = u0;
	uBrand[1] = u1;
	uBrand[2] = u2;
	uBrand[3] = u3;
	uBrand[4] = u4;
	uBrand[5] = u5;
	uBrand[6] = u6;
	uBrand[7] = u7;
	uBrand[8] = u8;
	uBrand[9] = u9;
	uBrand[10] = u10;
	uBrand[11] = u11;

	return(0);
}

/* CPUID main()

int main(int argc, char *argv[])

{
	char	sMsg[2048];

	identifyCPU();

	sprintf(sMsg,"%s %s\n\nProcessor ID Family %d Model %d Stepping %d\nFlags: ",(*sCPUVendor)?sCPUVendor:"<unknown>",(*sCPUBranding)?sCPUBranding:"<No Name>",
								     uBasicFlags.iExtendedFamilyID<<4 | uBasicFlags.iFamilyID,
								     uBasicFlags.iExtendedModelID<<4  | uBasicFlags.iModelID,
								     uBasicFlags.iSteppingID);

	if( uExt2Flags.iFPU  ) strcat(sMsg, "FPU ");	// Integrated FPU
	if( uExt2Flags.iVME  ) strcat(sMsg, "VME ");	// Virtual Memory Extensions
	if( uExt2Flags.iDE   ) strcat(sMsg, "DE ");	// Debugging extensions
	if( uExt2Flags.iPSE  ) strcat(sMsg, "PSE ");	// Page Size Extensions
	if( uExt2Flags.iTSC  ) strcat(sMsg, "TSC ");	// Time Stamp COunter
	if( uExt2Flags.iMSR  ) strcat(sMsg, "MSR ");	// Model specific registers, RDMSR/WRMSR instructions
	if( uExt2Flags.iPAE  ) strcat(sMsg, "PAE ");	// Physical Address Extension
	if( uExt2Flags.iMCE  ) strcat(sMsg, "MCE ");	// Machine Check Exception
	if( uExt2Flags.iCX8  ) strcat(sMsg, "CX8 ");	// CMPXCHG8B instruction
	if( uExt2Flags.iAPIC ) strcat(sMsg, "APIC ");	// On-chip APIC
	if( uExt2Flags.iSEP  ) strcat(sMsg, "SEP ");	// SYSENTER and SYSEXIT instructions and MSRs
	if( uExt2Flags.iMTRR ) strcat(sMsg, "MTRR ");	// Memory Type Range Registers
	if( uExt2Flags.iPGE  ) strcat(sMsg, "PGE ");	// PTE Global Bit
	if( uExt2Flags.iMCA  ) strcat(sMsg, "MCA ");	// Machine Check Architecture
	if( uExt2Flags.iCMOV ) strcat(sMsg, "CMOV ");	// Conditional MOV instruction
	if( uExt2Flags.iPAT  ) strcat(sMsg, "PAT ");	// Page Attribute Table
	if( uExt2Flags.iPSE36) strcat(sMsg, "PSE36 ");	// 36-bit page size extension
	if( uExt2Flags.iPSN  ) strcat(sMsg, "PSN ");	// Processor serial number
	if( uExt2Flags.iCFLSH) strcat(sMsg, "CFLSH ");	// CLFLUSH instruction
	if( uExt2Flags.iDS   ) strcat(sMsg, "DS ");	// Debug Store
	if( uExt2Flags.iACPI ) strcat(sMsg, "ACPI ");	// Thermal monitor and software-controlled-clock functions
	if( uExt2Flags.iMMX  ) strcat(sMsg, "MMX ");	// MMX present
	if( uExt2Flags.iFXSR ) strcat(sMsg, "FXSR ");	// FXSAVE and FXRESTORE instructions
	if( uExt2Flags.iSSE  ) strcat(sMsg, "SSE ");	// Streaming SIMD Extensions
	if( uExt2Flags.iSSE2 ) strcat(sMsg, "SSE2 ");	// Streaming SIMD Extensions 2 present
	if( uExtFlags.iSSE3  ) strcat(sMsg, "SSE3 ");	// Streaming SIMD Extensions 3 present
	if( uExt2Flags.iSS   ) strcat(sMsg, "SS ");	// Self Snoop
	if( uExt2Flags.iHTT  ) strcat(sMsg, "HTT ");	// Hyperthreading
	if( uExt2Flags.iTM   ) strcat(sMsg, "TM ");	// Thermal Monitoring
	if( uExtFlags.iTM2 ) strcat(sMsg,"TM2 ");	// Thermal Monitoring 2
	if( uExt2Flags.iPBE  ) strcat(sMsg, "PBE ");	// Pending Break Enable (FERR#/PBE# pins)
	if( uExtFlags.iMWAIT ) strcat(sMsg, "MWAIT ");	// MONITOR/MWAIT instructions
	if( uExtFlags.iQDS ) strcat(sMsg, "QDS ");	// CPL Qualified Debug Store present
	if( uExtFlags.iEST ) strcat(sMsg, "EST ");	// Enhanced SpeedStep
	if( uExtFlags.iCID ) strcat(sMsg, "CID ");	// Context-ID
	if( uExtFlags.iVMX ) strcat(sMsg, "VMX ");	// Virtual Machine Extensions
	if( u8Ext2Flags.iLM ) strcat(sMsg, "EMT64 ");	// 64-bit technology
	if( u8Ext2Flags.iNX ) strcat(sMsg, "NX ");	// No Execute
	if( u8ExtFlags.iLAHF ) strcat(sMsg, "LAHF ");	// LAHF

	MessageBox(NULL,sMsg,"CPU Identification", MB_OK);
	return(0);
}

*/

_declspec(naked) ULONGLONG GetCycleCount()
{
    _asm rdtsc;
    _asm ret;
}

DWORD GetCPUSpeed(int interval)
{
    ULONGLONG ullStart, ullStop;
    ullStart = GetCycleCount();
    Sleep(interval);
    ullStop = GetCycleCount();
    return (DWORD)((ullStop - ullStart) / (1000 * interval));
}

/* CPU Speed main()

int main()
{
    printf("Processor speed:\t%.2f MHz\n",    ((float)GetCPUSpeed()));
    return 0;
}


*/