#include "sensor_event.h"
#include "sensorapi.h"
#include <sensors.h>

//////////WINDOWS DEFINE///////////////////////////
//includeÇÃÇ†Ç∆Ç…Ç®Ç¢ÇƒÇÀÅI
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

template <class T> inline void SafeReleaseRef(T* &pT)
{
	pT->Release();
	pT = NULL;
}

//********** SENSOR MANAGER EVENTS **********//

STDMETHODIMP CSensorManagerEvent::QueryInterface(REFIID iid, void** ppv)
{
	if (ppv == NULL) {
		return E_POINTER;
	}
	if (iid == __uuidof(IUnknown)) {
		AddRef();
		*ppv = static_cast<IUnknown*>(this);
	}
	else if (iid == __uuidof(ISensorManagerEvents)) {
		AddRef();
		*ppv = static_cast<ISensorManagerEvents*>(this);
	}
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	return S_OK;
}

STDMETHODIMP_(ULONG) CSensorManagerEvent::AddRef()
{
	return ::InterlockedIncrement(&m_cRef); 
}

STDMETHODIMP_(ULONG) CSensorManagerEvent::Release()
{
	ULONG count = ::InterlockedDecrement(&m_cRef);
	if (count == 0) {
		delete this;
		return 0;
	}
	return count;
}

//--------- SPECIFIC ----------//

STDMETHODIMP CSensorManagerEvent::OnSensorEnter(ISensor *pSensor,SensorState state)
{
	p->AddSensor(pSensor);
	return S_OK;
}

//********** SENSOR EVENTS **********//

STDMETHODIMP CSensorEvent::QueryInterface(REFIID iid, void** ppv)
{
	if (ppv == NULL) {
		return E_POINTER;
	}
	if (iid == __uuidof(IUnknown)) {
		AddRef();
		*ppv = static_cast<IUnknown*>(this);
	}
	else if (iid == __uuidof(ISensorEvents)) {
		AddRef();
		*ppv = static_cast<ISensorEvents*>(this);
	}
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	return S_OK;
}

STDMETHODIMP_(ULONG) CSensorEvent::AddRef()
{
	return ::InterlockedIncrement(&m_cRef); 
}

STDMETHODIMP_(ULONG) CSensorEvent::Release()
{
	ULONG count = ::InterlockedDecrement(&m_cRef);
	if (count == 0) {
		delete this;
		return 0;
	}
	return count;
}

//--------- SPECIFIC ----------//

STDMETHODIMP CSensorEvent::OnEvent(ISensor *pSensor,REFGUID eventID,IPortableDeviceValues *pEventData)
{
	return S_OK;
}

STDMETHODIMP CSensorEvent::OnDataUpdated(ISensor *pSensor,ISensorDataReport *pDataReport)
{
	return S_OK;
}

STDMETHODIMP CSensorEvent::OnLeave(REFSENSOR_ID sensorID)
{
	p->DeleteSensor(sensorID);
	return S_OK;
}

STDMETHODIMP CSensorEvent::OnStateChanged(ISensor* pSensor,SensorState state)
{
	return S_OK;
}

