
#include "cplugin.h"

#pragma once

#ifndef SENSOR_API_PLUGIN_INCLUDED
#define SENSOR_API_PLUGIN_INCLUDED

#include <sensorsapi.h>
#include <sensors.h>

#include <map>

class CSensorEvent;
class CSensorManagerEvent;

bool operator< (const GUID &a,const GUID &b);

class CSensorAPIPlugin : public CPLUGIN
{
private:
	ISensorManager* m_pSnsMng;
	CSensorEvent* m_pSnsEvt;
	CSensorManagerEvent* m_pSnsMngEvt;

	std::map<SENSOR_ID,ISensor*> m_sensorlist;
	std::map<SENSOR_ID,ISensor*> m_sensorlist_denied;

	bool m_sensorlist_updated;

public:
	CSensorAPIPlugin(void) : m_pSnsMng(NULL),m_pSnsEvt(NULL),m_pSnsMngEvt(NULL),m_sensorlist_updated(false) {
	}
	~CSensorAPIPlugin() {
	}

	virtual void exec(const CSAORIInput& in,CSAORIOutput& out);
	virtual bool unload();
	virtual bool load();

	void AddSensor(ISensor *pS);
	void DeleteSensor(const SENSOR_ID &id);
};

#endif //SENSOR_API_PLUGIN_INCLUDED

