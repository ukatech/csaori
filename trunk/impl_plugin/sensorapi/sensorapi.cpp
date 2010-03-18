//setlocale//
#ifdef _MSC_VER
#if _MSC_VER >= 1400
#pragma setlocale("japanese")
#endif
#endif
//setlocale end//

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <new>

#include "sensorapi.h"
#include "sensor_event.h"

//////////WINDOWS DEFINE///////////////////////////
#ifdef _WINDOWS
#ifdef _DEBUG
#include <crtdbg.h>
#define new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif
///////////////////////////////////////////////////

CSAORIBase* CreateInstance(void)
{
	return new CSensorAPIPlugin();
}

bool operator< (const GUID &a,const GUID &b) {
	return memcmp(&a,&b,sizeof(a)) < 0;
}

template <typename T> void SafeReleaseRef(T* &pT)
{
	if ( pT ) {
		pT->Release();
		pT = NULL;
	}
}

bool CSensorAPIPlugin::load()
{
	HRESULT hrRes = CoCreateInstance( CLSID_SensorManager, 0, CLSCTX_ALL, IID_PPV_ARGS(&m_pSnsMng) );
	if ( ! SUCCEEDED( hrRes ) ) {
		m_pSnsMng = NULL;
    }
	if ( ! m_pSnsMng ) {
		return false;
	}

	m_pSnsEvt = new CSensorEvent(this);
	if ( ! m_pSnsEvt ) {
		SafeReleaseRef(m_pSnsMng);
		return false;
	}
	m_pSnsEvt->AddRef();

	m_pSnsMngEvt = new CSensorManagerEvent(this,m_pSnsEvt);
	if ( ! m_pSnsMngEvt ) {
		SafeReleaseRef(m_pSnsEvt);
		SafeReleaseRef(m_pSnsMng);
		return false;
	}
	m_pSnsMngEvt->AddRef();

	ISensorCollection*	pSensCol = NULL;
	m_pSnsMng->GetSensorsByCategory(SENSOR_CATEGORY_ALL,&pSensCol);
	if ( pSensCol ) {
		ULONG ulMaxCnt = 0;
		hrRes = pSensCol->GetCount(&ulMaxCnt);
		if ( ulMaxCnt ) {
			for ( ULONG i = 0 ; i < ulMaxCnt ; i++ ) {
				ISensor* pSens = NULL;
				hrRes = pSensCol->GetAt(i,&pSens);
				if ( ! pSens ) { continue; }

				if ( m_pSnsEvt ) {
					AddSensor(pSens);
					pSens->SetEventSink(m_pSnsEvt);
				}
			}
		}
	}

	m_pSnsMng->SetEventSink(m_pSnsMngEvt);
	return true;
}

bool CSensorAPIPlugin::unload()
{
	SafeReleaseRef(m_pSnsMng);
	SafeReleaseRef(m_pSnsMngEvt);
	SafeReleaseRef(m_pSnsEvt);

	return true;
}

void CSensorAPIPlugin::exec(const CSAORIInput &in, CSAORIOutput &out)
{
	out.result_code = SAORIRESULT_NO_CONTENT;
}

void CSensorAPIPlugin::AddSensor(ISensor *pS)
{
	SENSOR_ID id;
	pS->GetID(&id);

	std::map<SENSOR_ID,ISensor*>::iterator itr = m_sensorlist.find(id);
	if ( itr != m_sensorlist.end() ) {
		m_sensorlist[id] = pS;
		return;
	}

	pS->AddRef();
	m_sensorlist[id] = pS;
}

void CSensorAPIPlugin::DeleteSensor(const SENSOR_ID &id)
{
	std::map<SENSOR_ID,ISensor*>::iterator itr = m_sensorlist.find(id);
	if ( itr == m_sensorlist.end() ) {
		return;
	}

	itr->second->Release();
	m_sensorlist.erase(itr);
}

