#include <pdh.h>
#include <pdhmsg.h>

#include "cpuusage.h"

static HMODULE hModulePdh = NULL;

#define SystemBasicInformation       0
#define SystemPerformanceInformation 2
#define SystemTimeInformation        3

#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct
{
    DWORD   dwUnknown1;
    ULONG   uKeMaximumIncrement;
    ULONG   uPageSize;
    ULONG   uMmNumberOfPhysicalPages;
    ULONG   uMmLowestPhysicalPage;
    ULONG   uMmHighestPhysicalPage;
    ULONG   uAllocationGranularity;
    PVOID   pLowestUserAddress;
    PVOID   pMmHighestUserAddress;
    ULONG   uKeActiveProcessors;
    BYTE    bKeNumberProcessors;
    BYTE    bUnknown2;
    WORD    wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
    LARGE_INTEGER   liIdleTime;
    DWORD           dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
    LARGE_INTEGER liKeBootTime;
    LARGE_INTEGER liKeSystemTime;
    LARGE_INTEGER liExpTimeZoneBias;
    ULONG         uCurrentTimeZoneId;
    DWORD         dwReserved;
} SYSTEM_TIME_INFORMATION;

typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);

typedef PDH_STATUS (WINAPI* defPdhOpenQueryW)(LPCWSTR, DWORD_PTR, PDH_HQUERY*);
typedef PDH_STATUS (WINAPI* defPdhAddCounterW)(PDH_HQUERY, LPCWSTR, DWORD_PTR, PDH_HCOUNTER*);
typedef PDH_STATUS (WINAPI* defPdhCollectQueryData)(PDH_HQUERY);
typedef PDH_STATUS (WINAPI* defPdhGetFormattedCounterValue)(PDH_HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
typedef PDH_STATUS (WINAPI* defPdhCloseQuery)(PDH_HQUERY);
typedef PDH_STATUS (WINAPI* defPdhRemoveCounter)(PDH_HCOUNTER);

static defPdhOpenQueryW pPdhOpenQueryW = NULL;
static defPdhAddCounterW pPdhAddCounterW = NULL;
static defPdhCollectQueryData pPdhCollectQueryData = NULL;
static defPdhGetFormattedCounterValue pPdhGetFormattedCounterValue = NULL;
static defPdhCloseQuery pPdhCloseQuery = NULL;
static defPdhRemoveCounter pPdhRemoveCounter = NULL;

void CPUUsage::CPUUsageThreadNT(void)
{
	SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo;
    SYSTEM_TIME_INFORMATION        SysTimeInfo;
    SYSTEM_BASIC_INFORMATION       SysBaseInfo;
    double                         dbIdleTime;
    double                         dbSystemTime;
    LONG                           status;
    LARGE_INTEGER                  liOldIdleTime = {0,0};
    LARGE_INTEGER                  liOldSystemTime = {0,0};
	PROCNTQSI                      NtQuerySystemInformation;
	
    NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(
		GetModuleHandle("ntdll"),
		"NtQuerySystemInformation"
		);
	
    if (!NtQuerySystemInformation) {
        m_usage = 0;
		return;
	}
	
    // get number of processors in the system
    status = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL);
    if (status != NO_ERROR) {
        m_usage = 0;
		return;
	}
    
    while ( ! m_thread_exit ) {
        // get new system time
		status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0);
        if (status!=NO_ERROR) {
			// get new CPU's idle time
			status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL);
			if (status != NO_ERROR) {		
				// if it's a first call - skip it
				if (liOldIdleTime.QuadPart != 0) {
					// CurrentValue = NewValue - OldValue
					dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
					dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
					
					// CurrentCpuIdle = IdleTime / SystemTime
					dbIdleTime = dbIdleTime / dbSystemTime;
					
					// CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
					dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;
					
					m_usage = (UINT)dbIdleTime;
				}
				
				// store new CPU's idle and system time
				liOldIdleTime = SysPerfInfo.liIdleTime;
				liOldSystemTime = SysTimeInfo.liKeSystemTime;
			}
		}
		
        // wait second
        WaitForSingleObject(m_h_event,1000);
    }
}


void CPUUsage::CPUUsageThread9X(void)
{
    HKEY hkey;
    DWORD dwDataSize;
    DWORD dwType;
    DWORD dwCpuUsage;
    UINT usage;
	
    // starting the counter
    if ( RegOpenKeyEx( HKEY_DYN_DATA,
		"PerfStats\\StartStat",
		0,KEY_ALL_ACCESS,
		&hkey ) != ERROR_SUCCESS) {
		m_usage = 0;
		return;
	}
	
    dwDataSize=sizeof(DWORD);
    RegQueryValueEx( hkey,
		"KERNEL\\CPUUsage",
		NULL,&dwType,
		(LPBYTE)&dwCpuUsage,
		&dwDataSize );
	
    RegCloseKey(hkey);
    
    // geting current counter's value
    if ( RegOpenKeyEx( HKEY_DYN_DATA,
		"PerfStats\\StatData",
		0,KEY_READ,
		&hkey ) != ERROR_SUCCESS) {
		m_usage = 0;
		return;
	}
	
	dwDataSize=sizeof(DWORD);
	RegQueryValueEx( hkey,
		"KERNEL\\CPUUsage",
		NULL,&dwType,
		(LPBYTE)&dwCpuUsage,
		&dwDataSize );

	WaitForSingleObject(m_h_event,1000);
	RegQueryValueEx( hkey,
		"KERNEL\\CPUUsage",
		NULL,&dwType,
		(LPBYTE)&dwCpuUsage,
		&dwDataSize );
	usage = (UINT)dwCpuUsage;
	
    RegCloseKey(hkey);
    
    // stoping the counter
    if ( RegOpenKeyEx( HKEY_DYN_DATA,
		"PerfStats\\StopStat",
		0,KEY_ALL_ACCESS,
		&hkey ) != ERROR_SUCCESS) {
		m_usage = 0;
		return;
	}
	
    dwDataSize=sizeof(DWORD);
    RegQueryValueEx( hkey,
		"KERNEL\\CPUUsage",
		NULL,&dwType,
		(LPBYTE)&dwCpuUsage,
		&dwDataSize );
	
    RegCloseKey(hkey);
}

void CPUUsage::CPUUsageThread()
{
	if ( ! m_is_osvi_got ) {
		m_is_osvi_got = true;
		
		ZeroMemory(&m_osvi, sizeof(OSVERSIONINFO));
		m_osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx (&m_osvi);
	}
	
	if(m_osvi.dwPlatformId == 1) { // Win 9x/ME
		CPUUsageThread9X();
	}
	else if(m_osvi.dwPlatformId == 2) { // Win NT
		CPUUsageThreadNT();
	}
}

CPUUsage::CPUUsage(void) : m_is_osvi_got(false) , m_usage(0) , m_thread_exit(0)
{
	m_h_event = ::CreateEvent(NULL,TRUE,FALSE,NULL);
}

CPUUsage::~CPUUsage(void)
{
	::CloseHandle(m_h_event);
}

void CPUUsage::Load(void)
{
}

void CPUUsage::Unload(void)
{
	::SetEvent(m_h_event);
}
