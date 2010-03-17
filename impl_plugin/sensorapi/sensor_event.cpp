#include "sensor_event.h"
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

STDMETHODIMP CHmSensorEvent::QueryInterface(REFIID iid, void** ppv)
{
	if (ppv == NULL) {
		return E_POINTER;
	}
	if (iid == __uuidof(IUnknown)) {
		*ppv = static_cast<IUnknown*>(this);
	}
	else if (iid == __uuidof(ISensorEvents)) {
		*ppv = static_cast<ISensorEvents*>(this);
	}
	else {
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

STDMETHODIMP_(ULONG) CHmSensorEvent::AddRef()
{
	return ::InterlockedIncrement(&m_cRef); 
}

STDMETHODIMP_(ULONG) CHmSensorEvent::Release()
{
	ULONG count = ::InterlockedDecrement(&m_cRef);
	if (count == 0) {
		delete this;
		return 0;
	}
	return count;
}

STDMETHODIMP CHmSensorEvent::OnEvent(
									 ISensor *pSensor,
									 REFGUID eventID,
									 IPortableDeviceValues *pEventData)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CHmSensorEvent::OnDataUpdated(ISensor *pSensor,ISensorDataReport *pDataReport)
{
	HRESULT hr = S_OK;

	if(NULL == pDataReport || NULL == pSensor) {
		return E_INVALIDARG;
	}

	pSensor->AddRef();
	pDataReport->AddRef();

	bool vbHuman = false;

	PROPVARIANT pvSensHm;
	PropVariantInit( &pvSensHm );
	hr = pDataReport->GetSensorValue( SENSOR_DATA_TYPE_HUMAN_PRESENCE, &pvSensHm );
	if ( SUCCEEDED( hr ) ) {
		vbHuman = pvSensHm.boolVal != 0;
	}
	PropVariantClear( &pvSensHm );

	SafeReleaseRef(pSensor);
	SafeReleaseRef(pDataReport);

	return hr;
}

STDMETHODIMP CHmSensorEvent::OnLeave(REFSENSOR_ID sensorID)
{
	HRESULT hr = S_OK;

	return hr;
}

STDMETHODIMP CHmSensorEvent::OnStateChanged(ISensor* pSensor,SensorState state)
{
	HRESULT hr = S_OK;
	/*SENSOR_CATEGORY_ID id = GUID_NULL;

	if(NULL == pSensor)
	{
	return E_INVALIDARG;
	}

	pSensor->AddRef();

	hr = pSensor->GetCategory(&id);

	if(SUCCEEDED(hr))
	{
	if(id == SENSOR_CATEGORY_BIOMETRIC)
	{
	if(state == SENSOR_STATE_READY)
	{
	}
	else if(state == SENSOR_STATE_ACCESS_DENIED)
	{
	}
	}
	}

	SafeReleaseRef(pSensor);*/  

	return hr;
}

