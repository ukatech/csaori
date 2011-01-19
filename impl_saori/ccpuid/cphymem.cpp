#include "cphymem.h"

// Non Object C functions

extern "C" {

// Functions in NTDLL that we dynamically locate
NTSTATUS (__stdcall *NtUnmapViewOfSection)(
		IN HANDLE  ProcessHandle,
		IN PVOID  BaseAddress
		);

NTSTATUS (__stdcall *NtOpenSection)(
		OUT PHANDLE  SectionHandle,
		IN ACCESS_MASK  DesiredAccess,
		IN POBJECT_ATTRIBUTES  ObjectAttributes
		);

NTSTATUS (__stdcall *NtMapViewOfSection)(
		IN HANDLE  SectionHandle,
		IN HANDLE  ProcessHandle,
		IN OUT PVOID  *BaseAddress,
		IN ULONG  ZeroBits,
		IN ULONG  CommitSize,
		IN OUT PLARGE_INTEGER  SectionOffset,	/* optional */
		IN OUT PULONG  ViewSize,
		IN SECTION_INHERIT  InheritDisposition,
		IN ULONG  AllocationType,
		IN ULONG  Protect
		);

VOID (__stdcall *RtlInitUnicodeString)(
		IN OUT PUNICODE_STRING  DestinationString,
		IN PCWSTR  SourceString
		);

ULONG (__stdcall *RtlNtStatusToDosError) (
		IN NTSTATUS Status
		);

// UnmapPhysicalMemory
// Maps a view of a section.
//
VOID UnmapPhysicalMemory( DWORD Address )
{
	NTSTATUS		status;

	status = NtUnmapViewOfSection( (HANDLE) -1, (PVOID) Address );
	if( !NT_SUCCESS(status)) {

//		PrintError("Unable to unmap view", status );
	}
}

// MapPhysicalMemory
// Maps a view of a section.
//
BOOLEAN MapPhysicalMemory( HANDLE PhysicalMemory,
							PDWORD Address, PDWORD Length,
							PDWORD VirtualAddress )
{
	NTSTATUS			ntStatus;
	PHYSICAL_ADDRESS	viewBase;
//	char				error[256];

	*VirtualAddress = 0;
	viewBase.QuadPart = (ULONGLONG) (*Address);
	ntStatus = NtMapViewOfSection (PhysicalMemory,
                               (HANDLE)-1,
                               (PVOID *)VirtualAddress,
                               0L,
                               *Length,
                               &viewBase,
                               Length,
                               ViewShare,
                               0,
                               PAGE_READONLY );

	if( !NT_SUCCESS( ntStatus )) {

//		sprintf( error, "Could not map view of %X length %X",
//				*Address, *Length );
//		PrintError( error, ntStatus );
		return FALSE;					
	}

	*Address = viewBase.LowPart;
	return TRUE;
}

// OpensPhysicalMemory
// This function opens the physical memory device. It
// uses the native API since 
//
HANDLE OpenPhysicalMemory()
{
	NTSTATUS		status;
	HANDLE			physmem;
	UNICODE_STRING	physmemString;
	OBJECT_ATTRIBUTES attributes;
	WCHAR			physmemName[] = L"\\device\\physicalmemory";

	RtlInitUnicodeString( &physmemString, physmemName );	

	InitializeObjectAttributes( &attributes, &physmemString,
								OBJ_CASE_INSENSITIVE, NULL, NULL );			
	status = NtOpenSection( &physmem, SECTION_MAP_READ, &attributes );

	if( !NT_SUCCESS( status )) {

//		PrintError( "Could not open \\device\\physicalmemory", status );
		return NULL;
	}

	return physmem;
}

// LocateNtdllEntryPoints
// Finds the entry points for all the functions we 
// need within NTDLL.DLL.
//
BOOLEAN LocateNtdllEntryPoints()
{
	if( !(RtlInitUnicodeString = (void (__stdcall *)(PUNICODE_STRING,PCWSTR)) GetProcAddress( GetModuleHandle("ntdll.dll"),
			"RtlInitUnicodeString" )) ) {

		return FALSE;
	}
	if( !(NtUnmapViewOfSection = (NTSTATUS (__stdcall *)(HANDLE,PVOID)) GetProcAddress( GetModuleHandle("ntdll.dll"),
			"NtUnmapViewOfSection" )) ) {

		return FALSE;
	}
	if( !(NtOpenSection = (NTSTATUS (__stdcall *)(PHANDLE,ACCESS_MASK,POBJECT_ATTRIBUTES)) GetProcAddress( GetModuleHandle("ntdll.dll"),
			"NtOpenSection" )) ) {

		return FALSE;
	}
	if( !(NtMapViewOfSection = (NTSTATUS (__stdcall *)(HANDLE,HANDLE,PVOID * ,ULONG,ULONG,PLARGE_INTEGER,PULONG,SECTION_INHERIT,ULONG,ULONG)) GetProcAddress( GetModuleHandle("ntdll.dll"),
			"NtMapViewOfSection" )) ) {

		return FALSE;
	}
	if( !(RtlNtStatusToDosError = (ULONG (__stdcall *)(NTSTATUS)) GetProcAddress( GetModuleHandle("ntdll.dll"),
			"RtlNtStatusToDosError" )) ) {

		return FALSE;
	}
	return TRUE;
}

}
// Non Object C functions Ends

//#define bsize 1024*64
const int CPhyMem::bsize = 1024*64*3;

PBYTE CPhyMem::MemSearch(PBYTE ToSearch,long size,const PBYTE StartAddr,const PBYTE endAddr)
{
	PBYTE addr=StartAddr;
	long i=0;

	for(addr;addr<endAddr;addr++)
	{
		for(i=0;i<size;i++) if(*(addr+i)!=*(ToSearch+i)) break;
		if((i==size)&&(*(addr+0x10)=='_')
			&&(*(addr+0x11)=='D')) 
			return addr;
	}
	return NULL;
}


bool CPhyMem::CopyDmiMemory(PBYTE buf,DWORD phyAddr,DWORD size)
{
	if((physmem = OpenPhysicalMemory()))
	{
		if(MapPhysicalMemory( physmem, &phyAddr, &size,
								&vaddress ))
		{
			memcpy((void*)buf,(void*)vaddress,size);
			UnmapPhysicalMemory( vaddress );
			CloseHandle( physmem );
			return true;
		}
		return false;
	}
	return false;
}
/////////////////////// Fetch type[0] to type[4] only

CPhyMem::CPhyMem()
{
	bool bResult;
	char *sm="_SM_";

	for(int i=0;i<4;i++)
		type[i]=NULL;

	paddress=(DWORD)0x000d0000;	// Real Physical Address

	if(LocateNtdllEntryPoints()) {
		bResult=CopyDmiMemory((PBYTE)buf,paddress,bsize);

		ii=MemSearch((PBYTE)sm,4,(PBYTE)buf,(PBYTE)(buf+bsize-1));
		unsigned int *tableaddress=(unsigned int *)(ii+0x18);
//		bResult=CopyDmiMemory((PBYTE)buf,(PBYTE)(*tableaddress),bsize);
//		bResult=CopyDmiMemory((PBYTE)buf,(PBYTE)0xf5a60,1024*64);
//		type[0]=(PBYTE)buf;

		type[0]=(PBYTE)buf+(*tableaddress-0x000d0000);
		PBYTE p=type[0]+*(type[0]+1);
		for(i=1;i<4;i++)
		{
			while(1) 
			{
				p++;
				if((*p==0)&&(*(p+1)==0)&&*(p+2)==i)
					break;
			}
			type[i]=p+2;
			p=type[i]+*(type[i]+1);
		}
	}
}


CPhyMem::~CPhyMem()
{
//	delete []buf;
}

PBYTE CPhyMem::GetType(int num)
{
    return type[num];

}

// Object Ends

BOOL GetDMIInfo(LPSTR dmiBIOSVendor, LPSTR dmiBIOSVersion, LPSTR dmiBIOSReleaseDate,
				LPSTR dmiSysManufacturer, LPSTR dmiSysProductName, LPSTR dmiSysVersion,
				LPSTR dmiSysSerialNumber, LPSTR dmiMBManufacturer, LPSTR dmiMBProduct,
				LPSTR dmiMBVersion, LPSTR dmiMBSerialNumber, LPSTR dmiChassisManufacturer,
				LPSTR dmiChassisVersion, LPSTR dmiChassisSerialNumber, LPSTR dmiChassisAssetTagNumber) {

	CPhyMem ptr;

	PBYTE p=ptr.GetType(0)+*(ptr.GetType(0)+0x1); 
	strcat(dmiBIOSVendor,(char*)p);
	p+=strlen((char*)p)+1;
	strcat(dmiBIOSVersion,(char*)p);
	p+=strlen((char*)p)+1;
	strcat(dmiBIOSReleaseDate,(char*)p);

	p=ptr.GetType(1)+*(ptr.GetType(1)+0x1);
	strcat(dmiSysManufacturer,(char*)p);
	p+=strlen((char*)p)+1;
	strcat(dmiSysProductName,(char*)p);
	p+=strlen((char*)p)+1;
	strcat(dmiSysVersion,(char*)p);
	p+=strlen((char*)p)+1;
	strcat(dmiSysSerialNumber,(char*)p);

	p=ptr.GetType(2)+*(ptr.GetType(2)+0x1);
	strcat(dmiMBManufacturer,(char*)p);
	p+=strlen((char*)p)+1;
	strcat(dmiMBProduct,(char*)p);
	p+=strlen((char*)p)+1;
	strcat(dmiMBVersion,(char*)p);
	p+=strlen((char*)p)+1;
	strcat(dmiMBSerialNumber,(char*)p);

	p=ptr.GetType(3)+*(ptr.GetType(3)+0x1);
	strcat(dmiChassisManufacturer,(char*)p);
	p+=strlen((char*)p)+1;
	strcat(dmiChassisVersion,(char*)p);
	p+=strlen((char*)p)+1;
	strcat(dmiChassisSerialNumber,(char*)p);
	p+=strlen((char*)p)+1;
	strcat(dmiChassisAssetTagNumber,(char*)p);

	return TRUE;
}