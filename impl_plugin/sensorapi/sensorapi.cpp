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
#include <shobjidl.h>
#include <propvarutil.h>

#include <new>
#include <algorithm>

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
static const wchar_t* SensorDataTypeToString(const PROPERTYKEY &id);

static void GetSensorDefString(ISensor *pSens,string_t &str);
static void GetDataReportStringTable(ISensorDataReport *pDataReport,std::vector<string_t> &vec);

/*===============================================================
	インスタンス作成（csaori_baseから呼ばれる）
===============================================================*/

CSAORIBase* CreateInstance(void)
{
	return new CSensorAPIPlugin();
}

/*===============================================================
	ユーティリティ
===============================================================*/

//キーGUIDのmap用
static bool operator< (const GUID &a,const GUID &b) {
	return memcmp(&a,&b,sizeof(a)) < 0;
}

//COM開放ラッパー
template <typename T> static void SafeReleaseRef(T* &pT)
{
	if ( pT ) {
		pT->Release();
		pT = NULL;
	}
}

/*===============================================================
	センサー情報保持クラス
===============================================================*/
class CSensorAPIData {
private:
	ISensor *s;
	CSensorEvent *e;
	volatile bool updated;

	std::vector<string_t> data;
	std::vector<string_t> data_update;

public:
	CSensorAPIData(ISensor *ps,CSensorEvent *pe) : s(ps),e(pe),updated(false) {
		s->AddRef();
		s->SetEventSink(e);
	}
	~CSensorAPIData() {
		s->SetEventSink(NULL);
		s->Release();
	}

	inline void SetSensor(ISensor *ps) { s = ps; }
	inline ISensor* GetSensor(void) { return s; }

	inline bool CheckUpdateAndClear(void) {
		if ( ! updated ) {
			return false;
		}
		updated = false;
		if ( data == data_update ) {
			return false;
		}
		data_update = data;
		return true;
	}

	void UpdateData(std::vector<string_t> &s) {
		if ( s == data ) { return; }
		data.swap(s);
		updated = true;
	}

	std::vector<string_t> &GetData(void) { return data; }
};

/*===============================================================
	初期化(DllMain縛り)
===============================================================*/
CSensorAPIPlugin::CSensorAPIPlugin(void) : m_pSnsMng(NULL),m_pSnsEvt(NULL),m_pSnsMngEvt(NULL)
		,m_sensorlist_updated(false)
{
}

CSensorAPIPlugin::~CSensorAPIPlugin()
{
}

/*===============================================================
	初期化(DllMainとは別)
===============================================================*/

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
					SensorState state;
					pSens->GetState(&state);
					AddSensor(pSens,state);
				}
			}
		}
	}

	m_pSnsMng->SetEventSink(m_pSnsMngEvt);
	return true;
}

/*===============================================================
	終了(DllMainとは別)
===============================================================*/

bool CSensorAPIPlugin::unload()
{
	{ //ロック開放ブレース
		SAORI_FUNC::CCriticalSectionLock lock(m_lock);
		for ( sensor_data_map::iterator itr = m_sensorlist.begin() ; 
				itr != m_sensorlist.end() ; ++itr ) {
			delete itr->second;
		}
		m_sensorlist.clear();

		for ( sensor_data_map::iterator itr = m_sensorlist_denied.begin() ; 
				itr != m_sensorlist_denied.end() ; ++itr ) {
			delete itr->second;
		}
		m_sensorlist_denied.clear();
	}

	m_pSnsMng->SetEventSink(NULL);

	SafeReleaseRef(m_pSnsMng);

	SafeReleaseRef(m_pSnsMngEvt);
	SafeReleaseRef(m_pSnsEvt);

	return true;
}

/*===============================================================
	コマンド実行
===============================================================*/

void CSensorAPIPlugin::exec(const CSAORIInput &in, CSAORIOutput &out)
{
	out.result_code = SAORIRESULT_NO_CONTENT;
	//--------------------------------------------------------
	if ( _wcsicmp(in.id.c_str(),L"OnSecondChange") == 0 ) {
		OnSecondChange(in,out);
		return;
	}
	//--------------------------------------------------------
	if ( _wcsicmp(in.id.c_str(),L"OnMenuExec") == 0 ) {
		OnMenuExec(in,out);
		return;
	}
}

/*===============================================================
	OnSecondChange
===============================================================*/
void CSensorAPIPlugin::OnSecondChange(const CSAORIInput &in, CSAORIOutput &out)
{
	if ( m_sensorlist_updated ) {
		string_t tmp;

		event = L"OnSensorListUpdate";
		target = L"__SYSTEM_ALL_GHOST__";

		{ //ロック開放ブレース
			SAORI_FUNC::CCriticalSectionLock lock(m_lock);
			for ( sensor_data_map::iterator itr = m_sensorlist.begin() ; 
					itr != m_sensorlist.end() ; ++itr ) {
				GetSensorDefString(itr->second->GetSensor(),tmp);
				out.values.push_back(tmp);
			}
			m_sensorlist_updated = false;
		}

		out.result_code = SAORIRESULT_OK;
		return;
	}

	{ //ロック開放ブレース
		SAORI_FUNC::CCriticalSectionLock lock(m_lock);
		for ( sensor_data_map::iterator itr = m_sensorlist.begin() ; 
			itr != m_sensorlist.end() ; ++itr ) {

			if ( itr->second->CheckUpdateAndClear() ) {
				event = L"OnSensorDataUpdate";
				target = L"__SYSTEM_ALL_GHOST__";

				string_t str;
				GetSensorDefString(itr->second->GetSensor(),str);

				out.values.push_back(str);

				std::vector<string_t> &data = itr->second->GetData();
				out.values.insert(out.values.end(),data.begin(),data.end());

				out.result_code = SAORIRESULT_OK;
				return;
			}
		}
	}
}

/*===============================================================
	OnMenuExec
===============================================================*/
void CSensorAPIPlugin::OnMenuExec(const CSAORIInput &in, CSAORIOutput &out)
{
	IOpenControlPanel *pCpl;
	HRESULT hrRes = CoCreateInstance( CLSID_OpenControlPanel, 0, CLSCTX_ALL, IID_PPV_ARGS(&pCpl) );
	if ( ! SUCCEEDED( hrRes ) ) {
		pCpl = NULL;
	}
	if ( pCpl ) {
		pCpl->Open(L"Microsoft.LocationAndOtherSensors",NULL,NULL);
		pCpl->Release();
	}
}

/*===============================================================
	センサー情報追加
===============================================================*/

void CSensorAPIPlugin::AddSensor(ISensor *pS,SensorState s)
{
	SENSOR_ID id;
	pS->GetID(&id);

	SAORI_FUNC::CCriticalSectionLock lock(m_lock);

	if ( s == SENSOR_STATE_ACCESS_DENIED ) {
		sensor_data_map::iterator itr = m_sensorlist_denied.find(id);
		if ( itr != m_sensorlist_denied.end() ) {
			m_sensorlist_denied[id]->SetSensor(pS);
			return;
		}
		m_sensorlist_denied[id] = new CSensorAPIData(pS,m_pSnsEvt);
	}
	else {
		sensor_data_map::iterator itr = m_sensorlist.find(id);
		if ( itr != m_sensorlist.end() ) {
			m_sensorlist[id]->SetSensor(pS);
			return;
		}

		m_sensorlist[id] = new CSensorAPIData(pS,m_pSnsEvt);
		m_sensorlist_updated = true;
	}
}

/*===============================================================
	センサーデータ変更
===============================================================*/

void CSensorAPIPlugin::DataUpdateSensor(ISensor *pS,ISensorDataReport *pD)
{
	SENSOR_ID id;
	pS->GetID(&id);

	SAORI_FUNC::CCriticalSectionLock lock(m_lock);

	sensor_data_map::iterator itr = m_sensorlist.find(id);
	if ( itr == m_sensorlist.end() ) {
		return;
	}

	std::vector<string_t> ref;
	GetDataReportStringTable(pD,ref);

	itr->second->UpdateData(ref);
}

/*===============================================================
	センサー状態変更
===============================================================*/

void CSensorAPIPlugin::StateChangeSensor(ISensor *pS,SensorState s)
{
	SENSOR_ID id;
	pS->GetID(&id);

	SAORI_FUNC::CCriticalSectionLock lock(m_lock);

	if ( s == SENSOR_STATE_ACCESS_DENIED ) {
		sensor_data_map::iterator itr = m_sensorlist.find(id);
		if ( itr != m_sensorlist.end() ) {
			m_sensorlist_denied[id] = itr->second;
			m_sensorlist.erase(itr);

			m_sensorlist_updated = true;
		}
	}
	else {
		sensor_data_map::iterator itr = m_sensorlist_denied.find(id);
		if ( itr != m_sensorlist_denied.end() ) {
			m_sensorlist[id] = itr->second;
			m_sensorlist_denied.erase(itr);

			m_sensorlist_updated = true;
		}
	}
}

/*===============================================================
	センサー停止
===============================================================*/

void CSensorAPIPlugin::DeleteSensor(const SENSOR_ID &id)
{
	SAORI_FUNC::CCriticalSectionLock lock(m_lock);

	sensor_data_map::iterator itr = m_sensorlist.find(id);
	if ( itr != m_sensorlist.end() ) {
		delete itr->second;
		m_sensorlist.erase(itr);
		m_sensorlist_updated = true;
		return;
	}

	itr = m_sensorlist_denied.find(id);
	if ( itr != m_sensorlist_denied.end() ) {
		delete itr->second;
		m_sensorlist_denied.erase(itr);
		return;
	}
}

/*===============================================================
	状態を文字列に変換
===============================================================*/

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

/*===============================================================
	タイプを文字列に変換
===============================================================*/

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

	for ( ULONG i = 0 ; i < _countof(sensor_type_table) ; ++i ) {
		if ( sensor_type_table[i].id == id ) {
			return sensor_type_table[i].str;
		}
	}
	return L"";
}

/*===============================================================
	データタイプを文字列に変換
===============================================================*/

static const wchar_t* SensorDataTypeToString(const PROPERTYKEY &id)
{

#define DEFINE_DATA_TYPE_TABLE(s) {SENSOR_DATA_TYPE_ ## s,L ## #s},

	static const struct { const PROPERTYKEY id; const wchar_t * const str; } data_type_table[] = {
		DEFINE_DATA_TYPE_TABLE(TIMESTAMP)

		DEFINE_DATA_TYPE_TABLE(LATITUDE_DEGREES)
		DEFINE_DATA_TYPE_TABLE(LONGITUDE_DEGREES)
		DEFINE_DATA_TYPE_TABLE(ALTITUDE_SEALEVEL_METERS)
		DEFINE_DATA_TYPE_TABLE(ALTITUDE_ELLIPSOID_METERS)
		DEFINE_DATA_TYPE_TABLE(SPEED_KNOTS)
		DEFINE_DATA_TYPE_TABLE(TRUE_HEADING_DEGREES)
		DEFINE_DATA_TYPE_TABLE(MAGNETIC_HEADING_DEGREES)
		DEFINE_DATA_TYPE_TABLE(MAGNETIC_VARIATION)
		DEFINE_DATA_TYPE_TABLE(FIX_QUALITY)
		DEFINE_DATA_TYPE_TABLE(FIX_TYPE)
		DEFINE_DATA_TYPE_TABLE(POSITION_DILUTION_OF_PRECISION)
		DEFINE_DATA_TYPE_TABLE(HORIZONAL_DILUTION_OF_PRECISION)
		DEFINE_DATA_TYPE_TABLE(VERTICAL_DILUTION_OF_PRECISION)
		DEFINE_DATA_TYPE_TABLE(SATELLITES_USED_COUNT)
		DEFINE_DATA_TYPE_TABLE(SATELLITES_USED_PRNS)
		DEFINE_DATA_TYPE_TABLE(SATELLITES_IN_VIEW)
		DEFINE_DATA_TYPE_TABLE(SATELLITES_IN_VIEW_PRNS)
		DEFINE_DATA_TYPE_TABLE(SATELLITES_IN_VIEW_ELEVATION)
		DEFINE_DATA_TYPE_TABLE(SATELLITES_IN_VIEW_AZIMUTH)
		DEFINE_DATA_TYPE_TABLE(SATELLITES_IN_VIEW_STN_RATIO)
		DEFINE_DATA_TYPE_TABLE(ERROR_RADIUS_METERS)
		DEFINE_DATA_TYPE_TABLE(ADDRESS1)
		DEFINE_DATA_TYPE_TABLE(ADDRESS2)
		DEFINE_DATA_TYPE_TABLE(CITY)
		DEFINE_DATA_TYPE_TABLE(STATE_PROVINCE)
		DEFINE_DATA_TYPE_TABLE(POSTALCODE)
		DEFINE_DATA_TYPE_TABLE(COUNTRY_REGION)
		DEFINE_DATA_TYPE_TABLE(ALTITUDE_ELLIPSOID_ERROR_METERS)
		DEFINE_DATA_TYPE_TABLE(ALTITUDE_SEALEVEL_ERROR_METERS)
		DEFINE_DATA_TYPE_TABLE(GPS_SELECTION_MODE)
		DEFINE_DATA_TYPE_TABLE(GPS_OPERATION_MODE)
		DEFINE_DATA_TYPE_TABLE(GPS_STATUS)
		DEFINE_DATA_TYPE_TABLE(GEOIDAL_SEPARATION)
		DEFINE_DATA_TYPE_TABLE(DGPS_DATA_AGE)
		DEFINE_DATA_TYPE_TABLE(ALTITUDE_ANTENNA_SEALEVEL_METERS)
		DEFINE_DATA_TYPE_TABLE(DIFFERENTIAL_REFERENCE_STATION_ID)
		DEFINE_DATA_TYPE_TABLE(NMEA_SENTENCE)
		DEFINE_DATA_TYPE_TABLE(SATELLITES_IN_VIEW_ID)

		DEFINE_DATA_TYPE_TABLE(TEMPERATURE_CELSIUS)
		DEFINE_DATA_TYPE_TABLE(RELATIVE_HUMIDITY_PERCENT)
		DEFINE_DATA_TYPE_TABLE(ATMOSPHERIC_PRESSURE_BAR)
		DEFINE_DATA_TYPE_TABLE(WIND_DIRECTION_DEGREES_ANTICLOCKWISE)
		DEFINE_DATA_TYPE_TABLE(WIND_SPEED_METERS_PER_SECOND)

		DEFINE_DATA_TYPE_TABLE(ACCELERATION_X_G)
		DEFINE_DATA_TYPE_TABLE(ACCELERATION_Y_G)
		DEFINE_DATA_TYPE_TABLE(ACCELERATION_Z_G)
		DEFINE_DATA_TYPE_TABLE(ANGULAR_ACCELERATION_X_DEGREES_PER_SECOND_SQUARED)
		DEFINE_DATA_TYPE_TABLE(ANGULAR_ACCELERATION_Y_DEGREES_PER_SECOND_SQUARED)
		DEFINE_DATA_TYPE_TABLE(ANGULAR_ACCELERATION_Z_DEGREES_PER_SECOND_SQUARED)
		DEFINE_DATA_TYPE_TABLE(SPEED_METERS_PER_SECOND)
		DEFINE_DATA_TYPE_TABLE(MOTION_STATE)

		DEFINE_DATA_TYPE_TABLE(TILT_X_DEGREES)
		DEFINE_DATA_TYPE_TABLE(TILT_Y_DEGREES)
		DEFINE_DATA_TYPE_TABLE(TILT_Z_DEGREES)
		DEFINE_DATA_TYPE_TABLE(MAGNETIC_HEADING_X_DEGREES)
		DEFINE_DATA_TYPE_TABLE(MAGNETIC_HEADING_Y_DEGREES)
		DEFINE_DATA_TYPE_TABLE(MAGNETIC_HEADING_Z_DEGREES)
		DEFINE_DATA_TYPE_TABLE(DISTANCE_X_METERS)
		DEFINE_DATA_TYPE_TABLE(DISTANCE_Y_METERS)
		DEFINE_DATA_TYPE_TABLE(DISTANCE_Z_METERS)

		DEFINE_DATA_TYPE_TABLE(BOOLEAN_SWITCH_STATE)
		DEFINE_DATA_TYPE_TABLE(MULTIVALUE_SWITCH_STATE)
		DEFINE_DATA_TYPE_TABLE(FORCE_NEWTONS)
		DEFINE_DATA_TYPE_TABLE(ABSOLUTE_PRESSURE_PASCAL)
		DEFINE_DATA_TYPE_TABLE(GAUGE_PRESSURE_PASCAL)
		DEFINE_DATA_TYPE_TABLE(STRAIN)
		DEFINE_DATA_TYPE_TABLE(WEIGHT_KILOGRAMS)

		DEFINE_DATA_TYPE_TABLE(HUMAN_PRESENCE)
		DEFINE_DATA_TYPE_TABLE(HUMAN_PROXIMITY_METERS)
		DEFINE_DATA_TYPE_TABLE(TOUCH_STATE)

		DEFINE_DATA_TYPE_TABLE(LIGHT_LEVEL_LUX)
		DEFINE_DATA_TYPE_TABLE(LIGHT_TEMPERATURE_KELVIN)
		DEFINE_DATA_TYPE_TABLE(LIGHT_CHROMACITY)

		DEFINE_DATA_TYPE_TABLE(RFID_TAG_40_BIT)

		DEFINE_DATA_TYPE_TABLE(VOLTAGE_VOLTS)
		DEFINE_DATA_TYPE_TABLE(CURRENT_AMPS)
		DEFINE_DATA_TYPE_TABLE(CAPACITANCE_FARAD)
		DEFINE_DATA_TYPE_TABLE(RESISTANCE_OHMS)
		DEFINE_DATA_TYPE_TABLE(INDUCTANCE_HENRY)
		DEFINE_DATA_TYPE_TABLE(ELECTRICAL_POWER_WATTS)
	};

	for ( ULONG i = 0 ; i < _countof(data_type_table) ; ++i ) {
		if ( data_type_table[i].id == id ) {
			return data_type_table[i].str;
		}
	}
	return L"";
}

/*===============================================================
	SensorListなどのReferenceをつくる
===============================================================*/

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

/*===============================================================
	DataReport->文字配列
===============================================================*/

static void GetDataReportStringTable(ISensorDataReport *pDataReport,std::vector<string_t> &vec)
{
	IPortableDeviceValues *pValues;
	if ( ! SUCCEEDED(pDataReport->GetSensorValues(NULL,&pValues)) ) {
		return;
	}

	PROPERTYKEY key;
	PROPVARIANT value;
	wchar_t txt[256];
	string_t ref;
	DWORD count;

	pValues->GetCount(&count);

	for ( DWORD i = 0 ; i < count ; ++i ) {
		if ( ! SUCCEEDED(pValues->GetAt(i,&key,&value)) ) {
			continue;
		}

		ref.erase();

		wchar_t *value_str;
		::PropVariantToStringAlloc(value,&value_str);
		if ( *value_str ) {
			swprintf(txt,_countof(txt),L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x,%u",
				key.fmtid.Data1,key.fmtid.Data2,key.fmtid.Data3,
				key.fmtid.Data4[0],key.fmtid.Data4[1],key.fmtid.Data4[2],key.fmtid.Data4[3],
				key.fmtid.Data4[4],key.fmtid.Data4[5],key.fmtid.Data4[6],key.fmtid.Data4[7],
				key.pid);
			ref += txt;
			ref += L"\1";

			ref += SensorDataTypeToString(key);
			ref += L"\1";

			ref += value_str;
		}

		::CoTaskMemFree(value_str);
		::PropVariantClear(&value);

		vec.push_back(ref);
	}
}
