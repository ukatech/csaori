#include "cpuusage.h"

UINT CPUUsageNT(void)
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

    if (!NtQuerySystemInformation)
        return 0;

    // get number of processors in the system
    status = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL);
    if (status != NO_ERROR)
        return 0;
    
    while(1)
    {
        // get new system time
    status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0);
        if (status!=NO_ERROR)
            return 0;

        // get new CPU's idle time
        status = NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL);
        if (status != NO_ERROR)
            return 0;

        // if it's a first call - skip it
       if (liOldIdleTime.QuadPart != 0)
       {
            // CurrentValue = NewValue - OldValue
            dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
            dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);

            // CurrentCpuIdle = IdleTime / SystemTime
            dbIdleTime = dbIdleTime / dbSystemTime;

            // CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
            dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;

            return (UINT)dbIdleTime;
       }

        // store new CPU's idle and system time
        liOldIdleTime = SysPerfInfo.liIdleTime;
        liOldSystemTime = SysTimeInfo.liKeSystemTime;

        // wait half second
        Sleep(500);
    }
}


UINT CPUUsage9X(void)
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
                       &hkey ) != ERROR_SUCCESS)
       return 0;

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
                       &hkey ) != ERROR_SUCCESS)
       return 0;

        dwDataSize=sizeof(DWORD);
        RegQueryValueEx( hkey,
                         "KERNEL\\CPUUsage",
                         NULL,&dwType,
                         (LPBYTE)&dwCpuUsage,
                         &dwDataSize );
        Sleep(1000);
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
                       &hkey ) != ERROR_SUCCESS)
       return 0;

    dwDataSize=sizeof(DWORD);
    RegQueryValueEx( hkey,
                     "KERNEL\\CPUUsage",
                     NULL,&dwType,
                     (LPBYTE)&dwCpuUsage,
                     &dwDataSize );

    RegCloseKey(hkey);

    return usage;
}

static bool g_is_osvi_got = false;
static OSVERSIONINFO g_osvi;

UINT CPUUsage() {
	if ( ! g_is_osvi_got ) {
		g_is_osvi_got = true;

		ZeroMemory(&g_osvi, sizeof(OSVERSIONINFO));
		g_osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx (&g_osvi);
	}
	
	if(g_osvi.dwPlatformId == 1) { // Win 9x/ME
		return CPUUsage9X();
	} else if(g_osvi.dwPlatformId == 2) { // Win NT
		return CPUUsageNT();
	} else {
		return 0;
	}
}

