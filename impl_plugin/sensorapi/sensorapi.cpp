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

static const wchar_t* SensorStateToString(const SensorState s);
static const wchar_t* SensorTypeToString(const SENSOR_TYPE_ID &id);
static void GetSensorDefString(ISensor *pSens,string_t &str);

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
				}
			}
		}
	}

	m_pSnsMng->SetEventSink(m_pSnsMngEvt);
	return true;
}

bool CSensorAPIPlugin::unload()
{
	for ( std::map<SENSOR_ID,ISensor*>::iterator itr = m_sensorlist.begin() ; 
			itr != m_sensorlist.end() ; ++itr ) {
		itr->second->SetEventSink(NULL);
		itr->second->Release();
	}
	m_sensorlist.clear();

	for ( std::map<SENSOR_ID,ISensor*>::iterator itr = m_sensorlist_denied.begin() ; 
			itr != m_sensorlist_denied.end() ; ++itr ) {
		itr->second->SetEventSink(NULL);
		itr->second->Release();
	}
	m_sensorlist_denied.clear();

	m_pSnsMng->SetEventSink(NULL);

	SafeReleaseRef(m_pSnsMng);

	SafeReleaseRef(m_pSnsMngEvt);
	SafeReleaseRef(m_pSnsEvt);

	return true;
}

void CSensorAPIPlugin::exec(const CSAORIInput &in, CSAORIOutput &out)
{
	if ( _wcsicmp(in.id.c_str(),L"OnSecondChange") == 0 ) {
		if ( m_sensorlist_updated ) {
			string_t tmp;

			event = L"OnSensorListUpdate";
			target = L"__SYSTEM_ALL_GHOST__";

			for ( std::map<SENSOR_ID,ISensor*>::iterator itr = m_sensorlist.begin() ; 
					itr != m_sensorlist.end() ; ++itr ) {
				GetSensorDefString(itr->second,tmp);
				out.values.push_back(tmp);
			}
	
			m_sensorlist_updated = false;
			out.result_code = SAORIRESULT_OK;
		}
	}
	else {
		out.result_code = SAORIRESULT_NO_CONTENT;
	}
}

void CSensorAPIPlugin::AddSensor(ISensor *pS)
{
	SENSOR_ID id;
	pS->GetID(&id);

	SensorState s;
	pS->GetState(&s);

	if ( s == SENSOR_STATE_ACCESS_DENIED ) {
		std::map<SENSOR_ID,ISensor*>::iterator itr = m_sensorlist_denied.find(id);
		if ( itr != m_sensorlist_denied.end() ) {
			m_sensorlist_denied[id] = pS;
			return;
		}

		pS->AddRef();
		pS->SetEventSink(m_pSnsEvt);
		m_sensorlist_denied[id] = pS;
	}
	else {
		std::map<SENSOR_ID,ISensor*>::iterator itr = m_sensorlist.find(id);
		if ( itr != m_sensorlist.end() ) {
			m_sensorlist[id] = pS;
			return;
		}

		pS->AddRef();
		pS->SetEventSink(m_pSnsEvt);
		m_sensorlist[id] = pS;
		m_sensorlist_updated = true;
	}
}

void CSensorAPIPlugin::DeleteSensor(const SENSOR_ID &id)
{
	std::map<SENSOR_ID,ISensor*>::iterator itr = m_sensorlist.find(id);
	if ( itr != m_sensorlist.end() ) {
		itr->second->SetEventSink(NULL);
		itr->second->Release();
		m_sensorlist.erase(itr);
		m_sensorlist_updated = true;
		return;
	}

	itr = m_sensorlist_denied.find(id);
	if ( itr != m_sensorlist_denied.end() ) {
		itr->second->SetEventSink(NULL);
		itr->second->Release();
		m_sensorlist_denied.erase(itr);
		return;
	}
}

static const wchar_t* SensorStateToString(const SensorState s)
{
	switch ( s ) {
	case SENSOR_STATE_READY:
		return L"ready";
	case SENSOR_STATE_NOT_AVAILABLE:
		return L"not_available";
	case SENSOR_STATE_NO_DATA:
		return L"no_data";
	case SENSOR_STATE_INITIALIZING:
		return L"initializing";
	case SENSOR_STATE_ACCESS_DENIED:
		return L"access_denied";
	default:
		return L"error";
	}
}

static const wchar_t* SensorTypeToString(const SENSOR_TYPE_ID &id)
{

#define DEFINE_SENSOR_TABLE(s) {SENSOR_TYPE_ ## s,L ## #s},

	static const struct { const SENSOR_TYPE_ID id; const wchar_t * const str; } sensor_type_table[] = {
		DEFINE_SENSOR_TABLE(LOCATION_GPS)
		DEFINE_SENSOR_TABLE(LOCATION_STATIC)
		DEFINE_SENSOR_TABLE(LOCATION_LOOKUP)
		DEFINE_SENSOR_TABLE(LOCATION_TRIANGULATION)
		DEFINE_SENSOR_TABLE(LOCATION_OTHER)
		DEFINE_SENSOR_TABLE(LOCATION_BROADCAST)
		DEFINE_SENSOR_TABLE(LOCATION_DEAD_RECKONING)
		DEFINE_SENSOR_TABLE(ENVIRONMENTAL_TEMPERATURE)
		DEFINE_SENSOR_TABLE(ENVIRONMENTAL_ATMOSPHERIC_PRESSURE)
		DEFINE_SENSOR_TABLE(ENVIRONMENTAL_HUMIDITY)
		DEFINE_SENSOR_TABLE(ENVIRONMENTAL_WIND_SPEED)
		DEFINE_SENSOR_TABLE(ENVIRONMENTAL_WIND_DIRECTION)
		DEFINE_SENSOR_TABLE(ACCELEROMETER_1D)
		DEFINE_SENSOR_TABLE(ACCELEROMETER_2D)
		DEFINE_SENSOR_TABLE(ACCELEROMETER_3D)
		DEFINE_SENSOR_TABLE(MOTION_DETECTOR)
		DEFINE_SENSOR_TABLE(GYROMETER_1D)
		DEFINE_SENSOR_TABLE(GYROMETER_2D)
		DEFINE_SENSOR_TABLE(GYROMETER_3D)
		DEFINE_SENSOR_TABLE(SPEEDOMETER)
		DEFINE_SENSOR_TABLE(COMPASS_1D)
		DEFINE_SENSOR_TABLE(COMPASS_2D)
		DEFINE_SENSOR_TABLE(COMPASS_3D)
		DEFINE_SENSOR_TABLE(INCLINOMETER_1D)
		DEFINE_SENSOR_TABLE(INCLINOMETER_2D)
		DEFINE_SENSOR_TABLE(INCLINOMETER_3D)
		DEFINE_SENSOR_TABLE(DISTANCE_1D)
		DEFINE_SENSOR_TABLE(DISTANCE_2D)
		DEFINE_SENSOR_TABLE(DISTANCE_3D)
		DEFINE_SENSOR_TABLE(VOLTAGE)
		DEFINE_SENSOR_TABLE(CURRENT)
		DEFINE_SENSOR_TABLE(CAPACITANCE)
		DEFINE_SENSOR_TABLE(RESISTANCE)
		DEFINE_SENSOR_TABLE(INDUCTANCE)
		DEFINE_SENSOR_TABLE(ELECTRICAL_POWER)
		DEFINE_SENSOR_TABLE(POTENTIOMETER)
		DEFINE_SENSOR_TABLE(BOOLEAN_SWITCH)
		DEFINE_SENSOR_TABLE(MULTIVALUE_SWITCH)
		DEFINE_SENSOR_TABLE(FORCE)
		DEFINE_SENSOR_TABLE(SCALE)
		DEFINE_SENSOR_TABLE(PRESSURE)
		DEFINE_SENSOR_TABLE(STRAIN)
		DEFINE_SENSOR_TABLE(HUMAN_PRESENCE)
		DEFINE_SENSOR_TABLE(HUMAN_PROXIMITY)
		DEFINE_SENSOR_TABLE(TOUCH)
		DEFINE_SENSOR_TABLE(AMBIENT_LIGHT)
		DEFINE_SENSOR_TABLE(RFID_SCANNER)
		DEFINE_SENSOR_TABLE(BARCODE_SCANNER)
	};

	for ( ULONG i = 0 ; i < (sizeof(sensor_type_table)/sizeof(sensor_type_table[0])) ; ++i ) {
		if ( sensor_type_table[i].id == id ) {
			return sensor_type_table[i].str;
		}
	}
	return L"";
}

static void GetSensorDefString(ISensor *pSens,string_t &str)
{
	str.erase();

	wchar_t txt[256];

	SENSOR_ID guid;
	pSens->GetID(&guid);
	swprintf(txt,_countof(txt),L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",guid.Data1,guid.Data2,guid.Data3,guid.Data4[0],guid.Data4[1],guid.Data4[2],guid.Data4[3],guid.Data4[4],guid.Data4[5],guid.Data4[6],guid.Data4[7]);

	str += txt;
	str += L"\1";

	BSTR name;
	pSens->GetFriendlyName(&name);
	str += name;
	str += L"\1";

	SENSOR_TYPE_ID type;
	pSens->GetType(&type);
	str += SensorTypeToString(type);
	str += L"\1";

	SensorState s;
	pSens->GetState(&s);
	str += SensorStateToString(s);
}
