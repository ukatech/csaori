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
CPUID_80000005_ECX_t	uL1ICSize;
CPUID_80000005_EDX_t	uL1DCSize;
CPUID_80000006_ECX_t	uL2Size;
CPUID_80000006_EDX_t	uL3Size;
char		sCPUBranding[65];
char		sCPUVendor[16];

int identifyCPU()
{
	unsigned long uHighestCPUID;

	iCPUFlagsLoaded = 1;
	memset(sCPUBranding,0,65);
	memset(sCPUVendor,0,16);

	_asm {
		mov      eax, 0
		cpuid
		mov     DWORD PTR [sCPUVendor+0],ebx  // Get the manufacturer string
		mov     DWORD PTR [sCPUVendor+4],edx
		mov     DWORD PTR [sCPUVendor+8],ecx
		mov      eax, 80000000h
		cpuid
		mov     uHighestCPUID,eax             // Get highest extended CPUID number
	}

	_asm {									  // Get the basic flags
		mov	eax, 01h
		cpuid
		mov	uBasicFlags,eax
		mov	uExtFlags,ecx
		mov	uExt2Flags,edx
	}

	if(uHighestCPUID >= 0x80000001) {
		_asm {								  // Get the extended frlags
			mov	eax, 80000001h
			cpuid
			mov	u8ExtFlags,ecx
			mov	u8Ext2Flags,edx
		}
	}
	
	if(uHighestCPUID >= 0x80000004) {
		_asm {								  // Get CPU Name from CPUID
			mov eax, 80000002h
			cpuid
			mov DWORD PTR [sCPUBranding+ 0],eax
			mov DWORD PTR [sCPUBranding+ 4],ebx
			mov DWORD PTR [sCPUBranding+ 8],ecx
			mov DWORD PTR [sCPUBranding+12],edx
			mov eax, 80000003h
			cpuid
			mov DWORD PTR [sCPUBranding+16],eax
			mov DWORD PTR [sCPUBranding+20],ebx
			mov DWORD PTR [sCPUBranding+24],ecx
			mov DWORD PTR [sCPUBranding+28],edx
			mov eax, 80000004h
			cpuid
			mov DWORD PTR [sCPUBranding+32],eax
			mov DWORD PTR [sCPUBranding+36],ebx
			mov DWORD PTR [sCPUBranding+40],ecx
			mov DWORD PTR [sCPUBranding+44],edx
		}
	}
	else {									  // If no CPU Name retrieved from CPUID (i.e. Old Processors),
		if(*sCPUVendor) {					  // then determine CPU family from Vender/Family/Model
			if (!strncmp("AuthenticAMD", sCPUVendor, 12)) {
				switch (uBasicFlags.iFamilyID) { // extract family code
					case 4: // Am486/AM5x86
						strcpy (sCPUBranding, "AMD Am486");
						break;

					case 5: // K6
						switch (uBasicFlags.iModelID) {// extract model code
							case 0:
							case 1:
							case 2:
							case 3:
								strcpy (sCPUBranding, "AMD K5");
								break;
							case 6:
							case 7:
								strcpy (sCPUBranding, "AMD K6");
								break;
							case 8:
								strcpy (sCPUBranding, "AMD K6-2");
								break;
							case 9:
							case 10:
							case 11:
							case 12:
							case 13:
							case 14:
							case 15:
								strcpy (sCPUBranding, "AMD K6-3");
								break;
						}
						break;

					case 6: // Athlon
						// No model numbers are currently defined
						strcpy (sCPUBranding, "AMD Athlon");
						break;
				}
			}
			else if (!strncmp("GenuineIntel", sCPUVendor, 12)) {
				switch (uBasicFlags.iFamilyID) { // extract family code
					case 4:
						switch (uBasicFlags.iModelID) { // extract model code
							case 0:
							case 1:
								strcpy (sCPUBranding, "INTEL 486DX");
								break;
							case 2:
								strcpy (sCPUBranding, "INTEL 486SX");
								break;
							case 3:
								strcpy (sCPUBranding, "INTEL 486DX2");
								break;
							case 4:
								strcpy (sCPUBranding, "INTEL 486SL");
								break;
							case 5:
								strcpy (sCPUBranding, "INTEL 486SX2");
								break;
							case 7:
								strcpy (sCPUBranding, "INTEL 486DX2E");
								break;
							case 8:
								strcpy (sCPUBranding, "INTEL 486DX4");
								break;
						}
						break;

					case 5:
						switch (uBasicFlags.iModelID) { // extract model code
							case 1:
							case 2:
							case 3:
								strcpy (sCPUBranding, "INTEL Pentium");
								break;
							case 4:
								strcpy (sCPUBranding, "INTEL Pentium-MMX");
								break;
						}
						break;

					case 6:
						switch (uBasicFlags.iModelID) { // extract model code
							case 1:
								strcpy (sCPUBranding, "INTEL Pentium-Pro");
								break;
							case 3:
							case 5:
								strcpy (sCPUBranding, "INTEL Pentium-II");
								break;  // actual differentiation depends on cache settings
							case 6:
								strcpy (sCPUBranding, "INTEL Celeron");
								break;
							case 7:
							case 8:
							case 10:
								strcpy (sCPUBranding, "INTEL Pentium-III");
								break;  // actual differentiation depends on cache settings
							case 9: // *** Models begin this line are not confirm if those CPU do not return CPU Name with CPUID 0x80000002/3/4 ***
							case 0xD:
							case 0xE:
								strcpy (sCPUBranding, "INTEL Pentium-M");
								break;  // actual differentiation depends on cache settings
							case 0xF:
							case 0x16:
							case 0x17:
								strcpy (sCPUBranding, "INTEL Core 2");
								break;  // actual differentiation depends on cache settings
							case 0x1C:
								strcpy (sCPUBranding, "INTEL Atom");
								break;  // actual differentiation depends on cache settings
						}
						break;

					case 15: // family 15, extended family 0x00
						if(uBasicFlags.iExtendedFamilyID<<4 == 0) {
							switch (uBasicFlags.iModelID) {
								case 0:
									strcpy (sCPUBranding, "INTEL Pentium-4");
									break;
							}
						}
						break;
				}
			}
		}
	}
	
	if(uHighestCPUID >= 0x80000005) {		  // Get L1 Cache Sizes
		_asm {
			mov	eax, 80000005h
			cpuid
			mov	uL1ICSize,ecx
			mov	uL1DCSize,edx
		}
	}

	if(uHighestCPUID >= 0x80000006) {		  // Get L2/L3 Cache Sizes
		_asm {
			mov	eax, 80000006h
			cpuid
			mov	uL2Size,ecx
			mov	uL3Size,edx
		}
	}

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

double GetCPUSpeed(int interval)
{
    ULONGLONG ullStart, ullStop;
    ullStart = GetCycleCount();
    Sleep(interval);
    ullStop = GetCycleCount();
    return (static_cast<double>(static_cast<LONG>(ullStop - ullStart)) / (1000.0 * interval));
}

/* CPU Speed main()

int main()
{
    printf("Processor speed:\t%.2f MHz\n",    ((float)GetCPUSpeed()));
    return 0;
}


*/