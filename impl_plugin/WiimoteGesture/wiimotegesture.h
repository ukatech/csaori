#include "cplugin.h"
#include <deque>
#include "Wiimote.h"

#pragma once

#ifndef WIIMOTE_GESTURE_PLUGIN_INCLUDED
#define WIIMOTE_GESTURE_PLUGIN_INCLUDED

class POINT3D {
public:
	int x;
	int y;
	int z;

	POINT3D(void) : x(0), y(0), z(0) { }
	POINT3D(int px,int py,int pz) : x(px) , y(py) , z(pz) { }

	void operator-=(const POINT3D &p) {
		x -= p.x;
		y -= p.y;
		z -= p.z;
	}
	POINT3D operator-(const POINT3D &p) {
		POINT3D d = *this;
		d -= p;
		return d;
	}
};



class CWiimoteGesture : public CPLUGIN
{
private:
	volatile int thread_active;
	volatile int thread_running;
	HANDLE thread_handle;
	SAORI_FUNC::CCriticalSection thread_lock;

	CyberGarage::Wiimote *m_pWiimote;

	char current_button[32];
	char last_button[32];

	int timer_count;

public:
	CWiimoteGesture(void);
	~CWiimoteGesture();

	void ThreadProc(void);

	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();
};

#endif //WIIMOTE_GESTURE_PLUGIN_INCLUDED


