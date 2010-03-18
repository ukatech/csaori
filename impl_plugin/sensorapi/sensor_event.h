#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sensorsapi.h>

#pragma once

#ifndef INCLUDE_SENSOR_EVENT
#define INCLUDE_SENSOR_EVENT

class CSensorAPIPlugin;
class CSensorEvent;

//********** SENSOR MANAGER EVENTS **********//
class CSensorManagerEvent : public ISensorManagerEvents
{
public:
	CSensorManagerEvent(CSensorAPIPlugin *pa,CSensorEvent *ps) : p(pa),s(ps) {
	}

	// Common Functions.
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// ISensorManagerEvents Specific.
	STDMETHODIMP OnSensorEnter(ISensor *pSensor,SensorState state);

private:
	CSensorAPIPlugin *p;
	CSensorEvent *s;
	long m_cRef;
};

//********** SENSOR EVENTS **********//

class CSensorEvent : public ISensorEvents
{
public:
	CSensorEvent(CSensorAPIPlugin *pa) : p(pa) {
	}

	// Common Functions.
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// ISensorEvents Specific.
	STDMETHODIMP OnEvent(ISensor *pSensor,REFGUID eventID,IPortableDeviceValues *pEventData);
	STDMETHODIMP OnDataUpdated(ISensor *pSensor,ISensorDataReport *pDataReport);
	STDMETHODIMP OnLeave(REFSENSOR_ID sensorID);
	STDMETHODIMP OnStateChanged(ISensor* pSensor,SensorState state);

private:
	CSensorAPIPlugin *p;
	long m_cRef;
};

#endif
