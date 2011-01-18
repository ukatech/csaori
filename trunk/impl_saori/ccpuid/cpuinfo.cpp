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
CPUID_01_EBX_t	uBasic2Flags;
CPUID_01_ECX_t	uExtFlags;
CPUID_01_EDX_t	uExt2Flags;
CPUID_02_t	uTLBDesc1;
CPUID_02_t	uTLBDesc2;
CPUID_02_t	uTLBDesc3;
CPUID_02_t	uTLBDesc4;
CPUID_80000001_ECX_t	u8ExtFlags;
CPUID_80000001_EDX_t	u8Ext2Flags;
CPUID_80000005_ECX_t	uL1ICSize;
CPUID_80000005_EDX_t	uL1DCSize;
CPUID_80000006_ECX_t	uL2Size;
CPUID_80000006_EDX_t	uL3Size;
char		sCPUBranding[65];
char		sCPUVendor[16];
unsigned long uHighestCPUID;
char		sVMBranding[16];
unsigned long uHighestVMCPUID;

void identifyTLBDesc(unsigned char desc) {
	switch(desc) {
		case 0x6:
			uL1ICSize.iL1ICSize = 8;
			break;
		case 0x8:
			uL1ICSize.iL1ICSize = 16;
			break;
		case 0x30:
			uL1ICSize.iL1ICSize = 32;
			break;

		case 0xA:
		case 0x66:
			uL1DCSize.iL1DCSize = 8;
			break;
		case 0xC:
		case 0x60:
		case 0x67:
			uL1DCSize.iL1DCSize = 16;
			break;
		case 0x2C:
		case 0x68:
			uL1DCSize.iL1DCSize = 32;
			break;

		case 0x39:
		case 0x3B:
		case 0x41:
		case 0x79:
			uL2Size.iL2Size = 128;
			break;
		case 0x3A:
			uL2Size.iL2Size = 192;
			break;
		case 0x3C:
		case 0x42:
		case 0x7A:
		case 0x82:
			uL2Size.iL2Size = 256;
			break;
		case 0x3D:
			uL2Size.iL2Size = 384;
			break;
		case 0x3E:
		case 0x43:
		case 0x7B:
		case 0x7F:
		case 0x83:
		case 0x86:
			uL2Size.iL2Size = 512;
			break;
		case 0x44:
		case 0x78:
		case 0x7C:
		case 0x84:
		case 0x87:
			uL2Size.iL2Size = 1024;
			break;
		case 0x45:
		case 0x7D:
		case 0x85:
			uL2Size.iL2Size = 2048;
			break;
		case 0x48:
			uL2Size.iL2Size = 3072;
			break;
		case 0x4E:
			uL2Size.iL2Size = 6144;
			break;

		case 0x22:
			uL3Size.iL3Size = 512;
			break;
		case 0x23:
			uL3Size.iL3Size = 1024;
			break;
		case 0x25:
			uL3Size.iL3Size = 2048;
			break;
		case 0x29:
		case 0x46:
		case 0x49:
			uL3Size.iL3Size = 4096;
			break;
		case 0x4A:
			uL3Size.iL3Size = 6144;
			break;
		case 0x47:
		case 0x4B:
			uL3Size.iL3Size = 8192;
			break;
		case 0x4C:
			uL3Size.iL3Size = 12288;
			break;
		case 0x4D:
			uL3Size.iL3Size = 16384;
			break;
	}
}

int identifyCPU()
{
	

	iCPUFlagsLoaded = 1;
	memset(sCPUBranding,0,65);
	memset(sCPUVendor,0,16);
	memset(sVMBranding,0,16);

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
		mov	uBasic2Flags,ebx
		mov	uExtFlags,ecx
		mov	uExt2Flags,edx
	}
	if(uExtFlags.iVirt) {
		_asm {
			mov      eax, 40000000h
			cpuid
			mov     DWORD PTR [sVMBranding+0],ebx  // Get the VM branding string
			mov     DWORD PTR [sVMBranding+4],ecx
			mov     DWORD PTR [sVMBranding+8],edx
			mov     uHighestVMCPUID,eax             // Get highest VM CPUID number
		}
	}

	if(uHighestCPUID >= 2) {
		_asm {								  // Get the TLB descriptors
			mov	eax, 02h
			cpuid
			mov	uTLBDesc1,eax
			mov	uTLBDesc2,ebx
			mov	uTLBDesc3,ecx
			mov	uTLBDesc4,edx
		}
	}

	if(uHighestCPUID >= 0x80000001) {
		_asm {								  // Get the extended flags
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
	else if(uBasic2Flags.iBrandIndex) {		  // no CPU Name retrieved from CPUID, but with BrandIndex (i.e. Older Intel Processors)
		switch (uBasic2Flags.iBrandIndex) {
			case 1:
			case 10:
			case 15:
			case 20:
				strcpy (sCPUBranding, "Intel Celeron");
				break;
			case 2:
			case 4:
				strcpy (sCPUBranding, "Intel Pentium III");
				break;
			case 3:
				strcpy (sCPUBranding, "Intel Pentium III Xeon(tm) / Celeron");
				break;
			case 6:
				strcpy (sCPUBranding, "Intel Pentium III-M Mobile");
				break;
			case 7:
			case 19:
			case 23:
				strcpy (sCPUBranding, "Intel Celeron Mobile");
				break;
			case 8:
			case 9:
				strcpy (sCPUBranding, "Intel Pentium 4");
				break;
			case 11:
				strcpy (sCPUBranding, "Intel Xeon(tm) / Xeon(tm) MP");
				break;
			case 12:
				strcpy (sCPUBranding, "Intel Xeon(tm) MP");
				break;
			case 14:
				strcpy (sCPUBranding, "Intel Pentium 4 Mobile / Xeon(tm)");
				break;
			case 17:
			case 21:
				strcpy (sCPUBranding, "Intel Genuine Mobile");
				break;
			case 18:
				strcpy (sCPUBranding, "Intel Celeron M");
				break;
			case 22:
				strcpy (sCPUBranding, "Intel Pentium M");
				break;
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
	if (!strncmp("GenuineIntel", sCPUVendor, 12)) {
		if(uTLBDesc1.desc[0] != 0x00) {
			int i;
		    for(i = 1; i < 4; i++) { /* decode each descriptor */
		        if(uTLBDesc1.desc[3] & 0x80) { /* invalid descriptors */
		            break;
		         } /* invalid descriptors */
		        identifyTLBDesc(uTLBDesc1.desc[i]);
		     } /* decode each descriptor */
		    for(i = 0; i < 4; i++) { /* decode each descriptor */
		        if(uTLBDesc2.desc[3] & 0x80) { /* invalid descriptors */
		            break;
		         } /* invalid descriptors */
		        identifyTLBDesc(uTLBDesc2.desc[i]);
		     } /* decode each descriptor */
		    for(i = 0; i < 4; i++) { /* decode each descriptor */
		        if(uTLBDesc3.desc[3] & 0x80) { /* invalid descriptors */
		            break;
		         } /* invalid descriptors */
		        identifyTLBDesc(uTLBDesc3.desc[i]);
		     } /* decode each descriptor */
		    for(i = 0; i < 4; i++) { /* decode each descriptor */
		        if(uTLBDesc4.desc[3] & 0x80) { /* invalid descriptors */
		            break;
		         } /* invalid descriptors */
		        identifyTLBDesc(uTLBDesc4.desc[i]);
		     } /* decode each descriptor */
       	}
	} else {
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
	}

	return(0);
}

/* CPUID main()

int main(int argc, char *argv[])

{
	char	sMsg[2048];
	char	cacheinfo[256];

	identifyCPU();

	sprintf(sMsg,"%s %s\n\nHighest CPUID supported: %X\nBrandID: %d, CLFLUSHLineSize: %d, LogicalProcessors: %d, InitialAPICID: %d\nProcessor ID Family %d Model %d Stepping %d\nFlags: ",
				(*sCPUVendor)?sCPUVendor:"<unknown>",
				(*sCPUBranding)?sCPUBranding:"<No Name>",uHighestCPUID,
				uBasic2Flags.iBrandIndex,uBasic2Flags.iCLFLUSHLineSize,uBasic2Flags.iLogicalProcessors,uBasic2Flags.iInitialAPICID,
				uBasicFlags.iExtendedFamilyID<<4 | uBasicFlags.iFamilyID,
				uBasicFlags.iExtendedModelID<<4  | uBasicFlags.iModelID,
				uBasicFlags.iSteppingID);

	if(  uExt2Flags.iFPU  ) strcat(sMsg, "FPU ");		// Integrated FPU
	if(  uExt2Flags.iVME  ) strcat(sMsg, "VME ");		// Virtual Memory Extensions
	if(  uExt2Flags.iDE   ) strcat(sMsg, "DE ");		// Debugging extensions
	if(  uExt2Flags.iPSE  ) strcat(sMsg, "PSE ");		// Page Size Extensions
	if(  uExt2Flags.iTSC  ) strcat(sMsg, "TSC ");		// Time Stamp COunter
	if(  uExt2Flags.iMSR  ) strcat(sMsg, "MSR ");		// Model specific registers, RDMSR/WRMSR instructions
	if(  uExt2Flags.iPAE  ) strcat(sMsg, "PAE ");		// Physical Address Extension
	if(  uExt2Flags.iMCE  ) strcat(sMsg, "MCE ");		// Machine Check Exception
	if(  uExt2Flags.iCX8  ) strcat(sMsg, "CX8 ");		// CMPXCHG8B instruction
	if(  uExt2Flags.iAPIC ) strcat(sMsg, "APIC ");		// On-chip APIC
	if(  uExt2Flags.iSEP  ) strcat(sMsg, "SEP ");		// SYSENTER and SYSEXIT instructions and MSRs
	if(  uExt2Flags.iMTRR ) strcat(sMsg, "MTRR ");		// Memory Type Range Registers
	if(  uExt2Flags.iPGE  ) strcat(sMsg, "PGE ");		// PTE Global Bit
	if(  uExt2Flags.iMCA  ) strcat(sMsg, "MCA ");		// Machine Check Architecture
	if(  uExt2Flags.iCMOV ) strcat(sMsg, "CMOV ");		// Conditional MOV instruction
	if(  uExt2Flags.iPAT  ) strcat(sMsg, "PAT ");		// Page Attribute Table
	if(  uExt2Flags.iPSE36) strcat(sMsg, "PSE36 ");		// 36-bit page size extension
	if(  uExt2Flags.iPSN  ) strcat(sMsg, "PSN ");		// Processor serial number
	if(  uExt2Flags.iCFLSH) strcat(sMsg, "CFLSH ");		// CLFLUSH instruction
	if(  uExt2Flags.iDS   ) strcat(sMsg, "DS ");		// Debug Store
	if(  uExt2Flags.iACPI ) strcat(sMsg, "ACPI ");		// Thermal monitor and software-controlled-clock functions
	if(  uExt2Flags.iMMX  ) strcat(sMsg, "MMX ");		// MMX present
	if(  uExt2Flags.iFXSR ) strcat(sMsg, "FXSR ");		// FXSAVE and FXRESTORE instructions
	if(  uExt2Flags.iSSE  ) strcat(sMsg, "SSE ");		// Streaming SIMD Extensions
	if(  uExt2Flags.iSSE2 ) strcat(sMsg, "SSE2 ");		// Streaming SIMD Extensions 2 present
	if(   uExtFlags.iSSE3 ) strcat(sMsg, "SSE3 ");		// Streaming SIMD Extensions 3 present
	if(   uExtFlags.iSSSE3) strcat(sMsg, "SSSE3 ");		// Supplemental Streaming SIMD Extensions 3 present
	if(   uExtFlags.iSSE41) strcat(sMsg, "SSE4.1 ");	// Streaming SIMD Extensions 4.1 present
	if(   uExtFlags.iSSE42) strcat(sMsg, "SSE4.2 ");	// Streaming SIMD Extensions 4.2 present
	if(   uExtFlags.iAES  ) strcat(sMsg, "AES ");		// AES Instruction Extensions present
	if(   uExtFlags.iAVX  ) strcat(sMsg, "AVX ");		// Advanced Vector Extensions present
	if(  u8ExtFlags.iSSE4A) strcat(sMsg, "SSE4A ");		// Streaming SIMD Extensions 4A present
	if(  u8ExtFlags.imSSE ) strcat(sMsg, "maSSE ");		// Misaligned SSE
	if( u8Ext2Flags.i3DN  ) strcat(sMsg, "3DNow! ");	// 3DNow!
	if( u8Ext2Flags.i3DNEx) strcat(sMsg, "3DNow!+ ");	// 3DNow!+
	if( u8Ext2Flags.iMMXEx) strcat(sMsg, "MMX+ ");		// MMX+
	if(  uExt2Flags.iSS   ) strcat(sMsg, "SS ");		// Self Snoop
	if(  uExt2Flags.iHTT  ) strcat(sMsg, "HTT ");		// Hyperthreading
	if(  uExt2Flags.iTM   ) strcat(sMsg, "TM ");		// Thermal Monitoring
	if(   uExtFlags.iTM2  ) strcat(sMsg,"TM2 ");		// Thermal Monitoring 2
	if(  uExt2Flags.iPBE  ) strcat(sMsg, "PBE ");		// Pending Break Enable (FERR#/PBE# pins)
	if(   uExtFlags.iMWAIT) strcat(sMsg, "MWAIT ");		// MONITOR/MWAIT instructions
	if(   uExtFlags.iQDS  ) strcat(sMsg, "QDS ");		// CPL Qualified Debug Store present
	if(   uExtFlags.iEST  ) strcat(sMsg, "EIST ");		// Enhanced SpeedStep
	if(   uExtFlags.iCID  ) strcat(sMsg, "CID ");		// Context-ID
	if(   uExtFlags.iVMX  ) strcat(sMsg, "VMX ");		// Virtual Machine Extensions
	if(  u8ExtFlags.iSVM  ) strcat(sMsg, "SVM ");		// SVM
	if( u8Ext2Flags.iLM   ) strcat(sMsg, "x86-64 ");	// 64-bit technology
	if( u8Ext2Flags.iNX   ) strcat(sMsg, "NX ");		// No Execute
	if(  u8ExtFlags.iLAHF ) strcat(sMsg, "LAHF ");		// LAHF
	if(   uExtFlags.iVirt ) strcat(sMsg, "VIRT ");		// Virtualized Environment

	sprintf(cacheinfo,"\n\nCache Info:\nL1 I-Cache: %d, L1 D-Cache: %d\nL2 Cache: %d, L3 Cache: %d",
			uL1ICSize.iL1ICSize,uL1DCSize.iL1DCSize,
			uL2Size.iL2Size,uL3Size.iL3Size);
	strcat(sMsg,cacheinfo);

	if(*sVMBranding) {
		strcat(sMsg,"\nVM branding: ");
		strcat(sMsg,sVMBranding);
	}

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