#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sensorsapi.h>

#pragma once

#ifndef INCLUDE_SENSOR_EVENT
#define INCLUDE_SENSOR_EVENT

class CHmSensorEvent : public ISensorEvents
{
public:

    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    //
    // ISensorEvents methods.
    //

    STDMETHODIMP OnEvent(
            ISensor *pSensor,
            REFGUID eventID,
            IPortableDeviceValues *pEventData);

    STDMETHODIMP OnDataUpdated(
            ISensor *pSensor,
            ISensorDataReport *pDataReport);

    STDMETHODIMP OnLeave(
            REFSENSOR_ID sensorID);

    STDMETHODIMP OnStateChanged(
            ISensor* pSensor,
            SensorState state);

    private:
        long m_cRef;

};

#endif
