#pragma once

#ifndef __CPUUSAGE_H
#define __CPUUSAGE_H
#include <windows.h>

class CPUUsage {
private:
	bool m_is_osvi_got;
	OSVERSIONINFO m_osvi;
	UINT m_usage;

	volatile int m_thread_exit;
	HANDLE m_h_event;

	void CPUUsageThread9X();
	void CPUUsageThreadNT();

public:
	CPUUsage(void);
	~CPUUsage(void);

	void Load(void);
	void Unload(void);

	void CPUUsageThread();
};

#endif /* __CPUUSAGE_H */