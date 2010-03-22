
#include "cplugin.h"

#pragma once

#ifndef SENSOR_API_PLUGIN_INCLUDED
#define SENSOR_API_PLUGIN_INCLUDED

#include <sensorsapi.h>
#include <sensors.h>

#include <map>

class CSensorEvent;
class CSensorManagerEvent;
class CSensorAPIData;

bool operator< (const GUID &a,const GUID &b);

typedef std::map<SENSOR_ID,CSensorAPIData*> sensor_data_map;

class CSensorAPIPlugin : public CPLUGIN
{
private:
	ISensorManager* m_pSnsMng;
	CSensorEvent* m_pSnsEvt;
	CSensorManagerEvent* m_pSnsMngEvt;

	sensor_data_map m_sensorlist;
	sensor_data_map m_sensorlist_denied;

	volatile bool m_sensorlist_updated;

	SAORI_FUNC::CCriticalSection m_lock;

public:
	CSensorAPIPlugin(void);
	~CSensorAPIPlugin();

	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();

	void AddSensor(ISensor *pS,SensorState state);
	void StateChangeSensor(ISensor *pS,SensorState state);
	void DataUpdateSensor(ISensor *pS,ISensorDataReport *pD);
	void DeleteSensor(const SENSOR_ID &id);

	void OnSecondChange(const CSAORIInput &in, CSAORIOutput &out);
	void OnMenuExec(const CSAORIInput &in, CSAORIOutput &out);
};

#endif //SENSOR_API_PLUGIN_INCLUDED

