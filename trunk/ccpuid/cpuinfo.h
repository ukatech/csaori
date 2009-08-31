#ifndef __CPUINFO_H
#define __CPUINFO_H

#include <stdio.h>
#include <string.h>
#include <windows.h>


typedef struct CPUID_01_ValuesInEAX
{
	unsigned int	iSteppingID : 4;
	unsigned int	iModelID : 4;
	unsigned int	iFamilyID : 4;
	unsigned int	iProcessorType : 2;
	unsigned int	Reserved_14 : 2;
	unsigned int	iExtendedModelID : 4;
	unsigned int	iExtendedFamilyID : 8;
	unsigned int	Reserved_28 : 3;
	unsigned int	iProcessorBrandString : 1;
} CPUID_01_EAX_t;

typedef struct CPUID_01_ValuesInECX
{
	unsigned int	iSSE3 : 1;
	unsigned int	iReserved_1 : 2;
	unsigned int	iMWAIT : 1;
	unsigned int	iQDS : 1;
	unsigned int	iVMX : 1;
	unsigned int	iSMX : 1;
	unsigned int	iEST : 1;
	unsigned int	iTM2 : 1;
	unsigned int	iSSSE3 : 1;
	unsigned int	iCID : 1;
	unsigned int	Reserved_11 : 1;
	unsigned int	i256FMA : 1;
	unsigned int	iCHG16B : 1;
	unsigned int	ixTPRUpd : 1;
	unsigned int	iPDMSR : 1;
	unsigned int	Reserved_16 : 2;
	unsigned int	iDCA : 1;
	unsigned int	iSSE41 : 1;
	unsigned int	iSSE42 : 1;
	unsigned int	ix2APIC : 1;
	unsigned int	iMOVBE : 1;
	unsigned int	iPOPCNT : 1;
	unsigned int	Reserved_23 : 1;
	unsigned int	iAES : 1;
	unsigned int	iXSAVE : 1;
	unsigned int	iOSXSAVE : 1;
	unsigned int	iIAV256 : 1;
	unsigned int	Reserved_28 : 3;
} CPUID_01_ECX_t;

typedef struct CPUID_01_ValuesInEDX
{
	unsigned int	iFPU : 1;
	unsigned int	iVME : 1;
	unsigned int	iDE  : 1;
	unsigned int	iPSE : 1;
	unsigned int	iTSC : 1;
	unsigned int	iMSR : 1;
	unsigned int	iPAE : 1;
	unsigned int	iMCE : 1;
	unsigned int	iCX8 : 1;
	unsigned int	iAPIC: 1;
	unsigned int	Reserved_10 : 1;
	unsigned int	iSEP : 1;
	unsigned int	iMTRR: 1;
	unsigned int	iPGE : 1;
	unsigned int	iMCA : 1;
	unsigned int	iCMOV: 1;
	unsigned int	iPAT : 1;
	unsigned int	iPSE36:1;
	unsigned int	iPSN : 1;
	unsigned int	iCFLSH:1;
	unsigned int	Reserved_20 : 1;
	unsigned int	iDS  : 1;
	unsigned int	iACPI: 1;
	unsigned int	iMMX : 1;
	unsigned int	iFXSR: 1;
	unsigned int	iSSE : 1;
	unsigned int	iSSE2: 1;
	unsigned int	iSS  : 1;
	unsigned int	iHTT : 1;
	unsigned int	iTM  : 1;
	unsigned int	Reserved_30 : 1;
	unsigned int	iPBE : 1;
} CPUID_01_EDX_t;
typedef struct CPUID_80000001_ValuesInECX
{
	unsigned int	iLAHF : 1;
	unsigned int	iCmpLegacy : 1;
	unsigned int	iSVM  : 1;
	unsigned int	iExtApicSpace : 1;
	unsigned int	iAMCr8 : 1;
	unsigned int	iLZCNT : 1;
	unsigned int	iSSE4A : 1;
	unsigned int	imSSE : 1;
	unsigned int	iPREFETCH : 1;
	unsigned int	iOSVW : 1;
	unsigned int	iIBS : 1;
	unsigned int	iSSE5 : 1;
	unsigned int	iSKINIT : 1;
	unsigned int	iWDT : 1;
	unsigned int	Reserved_14 : 18;
} CPUID_80000001_ECX_t;

typedef struct CPUID_80000001_ValuesInEDX
{
	unsigned int	iFPU : 1;
	unsigned int	iVME : 1;
	unsigned int	iDE  : 1;
	unsigned int	iPSE : 1;
	unsigned int	iTSC : 1;
	unsigned int	iMSR : 1;
	unsigned int	iPAE : 1;
	unsigned int	iMCE : 1;
	unsigned int	iCX8 : 1;
	unsigned int	iAPIC: 1;
	unsigned int	Reserved_10 : 1;
	unsigned int	iSYSCALL : 1;
	unsigned int	iMTRR: 1;
	unsigned int	iPGE : 1;
	unsigned int	iMCA : 1;
	unsigned int	iCMOV: 1;
	unsigned int	iPAT : 1;
	unsigned int	iPSE36:1;
	unsigned int	Reserved_18 : 1;
	unsigned int	Reserved_19 : 1;
	unsigned int	iNX : 1;
	unsigned int	Reserved_21 : 1;
	unsigned int	iMMXEx: 1;
	unsigned int	iMMX : 1;
	unsigned int	iFXSR: 1;
	unsigned int	iFFXSR : 1;
	unsigned int	iPg1GB: 1;
	unsigned int	iRDTSCP  : 1;
	unsigned int	Reserved_28 : 1;
	unsigned int	iLM  : 1;
	unsigned int	i3DNEx : 1;
	unsigned int	i3DN : 1;
} CPUID_80000001_EDX_t;

typedef struct CPUID_80000005_ValuesInECX
{
	unsigned int	Reserved_0 : 24;
	unsigned int	iL1ICSize : 8;
} CPUID_80000005_ECX_t;

typedef struct CPUID_80000005_ValuesInEDX
{
	unsigned int	Reserved_0 : 24;
	unsigned int	iL1DCSize : 8;
} CPUID_80000005_EDX_t;

typedef struct CPUID_80000006_ValuesInECX
{
	unsigned int	Reserved_0 : 16;
	unsigned int	iL2Size : 16;
} CPUID_80000006_ECX_t;

typedef struct CPUID_80000006_ValuesInEDX
{
	unsigned int	Reserved_0 : 18;
	unsigned int	iL3Size : 14;
} CPUID_80000006_EDX_t;

extern	int		iCPUFlagsLoaded;
extern	CPUID_01_EAX_t	uBasicFlags;
extern	CPUID_01_ECX_t	uExtFlags;
extern	CPUID_01_EDX_t	uExt2Flags;
extern	CPUID_80000001_ECX_t	u8ExtFlags;
extern	CPUID_80000001_EDX_t	u8Ext2Flags;
extern	CPUID_80000005_ECX_t	uL1ICSize;
extern	CPUID_80000005_EDX_t	uL1DCSize;
extern	CPUID_80000006_ECX_t	uL2Size;
extern	CPUID_80000006_EDX_t	uL3Size;
extern	char		sCPUBranding[65];
extern	char		sCPUVendor[16];

int identifyCPU();
ULONGLONG GetCycleCount();
float GetCPUSpeed(int interval);

#endif /* __CPUINFO_H */