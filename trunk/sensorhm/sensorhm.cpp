#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "csaori.h"

#include <sensorsapi.h>
#include <sensors.h>

static int GetSensorState();

bool CSAORI::load()
{
	return true;
}

bool CSAORI::unload()
{
	return true;
}

void CSAORI::exec(const CSAORIInput &in, CSAORIOutput &out)
{
	out.result_code = SAORIRESULT_OK;
	int result = GetSensorState();

	if ( result < 0 ) {
		out.result = L"not found";
	}
	else if ( result > 0 ) {
		out.result = L"1";
	}
	else {
		out.result = L"0";
	}
}

int GetSensorState()
{
	ISensorManager* psmSnsMng;

	HRESULT hrRes = CoCreateInstance( CLSID_SensorManager, 0, CLSCTX_ALL, __uuidof( ISensorManager ), ( void** )&psmSnsMng );
	if ( ! SUCCEEDED( hrRes ) ) {
		psmSnsMng = NULL;
    }

	if ( ! psmSnsMng ) {
		return -1;
	}

	ISensorCollection*	piscSensCol = NULL;
	psmSnsMng->GetSensorsByCategory( SENSOR_CATEGORY_BIOMETRIC, &piscSensCol );
	if ( !piscSensCol ) {
		psmSnsMng->Release();
		return -1;
	}

	hrRes = psmSnsMng->GetSensorsByType( SENSOR_TYPE_HUMAN_PRESENCE, &piscSensCol );
	if ( !piscSensCol ) {
		psmSnsMng->Release();
		return -1;
	}

	ULONG ulMaxCnt = 0;
	hrRes = piscSensCol->GetCount( &ulMaxCnt );
	if ( !ulMaxCnt ) {
		piscSensCol->Release();
		psmSnsMng->Release();
		return -1;
	}

	unsigned int uiCnt = 0;

	VARIANT_BOOL vbHuman = FALSE;

	for ( uiCnt = 0 ; uiCnt < ulMaxCnt ; uiCnt++ ) {
		ISensor* pisSens = NULL;
		hrRes = piscSensCol->GetAt( uiCnt, &pisSens );

		if ( !pisSens ) {
			continue;
		}

		SENSOR_TYPE_ID stiSnsId;
		pisSens->GetType( &stiSnsId );

		if ( stiSnsId == GUID_NULL ) {
			piscSensCol->Release();
			return -1;
		}

		ISensorDataReport* pDataReport;
		hrRes = pisSens->GetData( &pDataReport );

		if ( hrRes == E_INVALIDARG ) {
			piscSensCol->Release();
			psmSnsMng->Release();
			return -1;
		}
		else if ( hrRes == HRESULT_FROM_WIN32( ERROR_NO_DATA ) ) {
			piscSensCol->Release();
			psmSnsMng->Release();
			return -1;
		}
		else if ( hrRes == E_ACCESSDENIED ) {
			piscSensCol->Release();
			psmSnsMng->Release();
			return -1;
		}

		if ( SUCCEEDED ( hrRes ) ) {
			PROPVARIANT pvSensHm;
			PropVariantInit( &pvSensHm );
			hrRes = pDataReport->GetSensorValue( SENSOR_DATA_TYPE_HUMAN_PRESENCE, &pvSensHm );
			if ( SUCCEEDED( hrRes ) ) {
				vbHuman = pvSensHm.boolVal;
			}
			PropVariantClear( &pvSensHm );

			pisSens->Release();
			pisSens = NULL;
			break;
        }

		pisSens->Release();
		pisSens = NULL;
	}

	piscSensCol->Clear();
	piscSensCol->Release();
	piscSensCol = NULL;
	
	psmSnsMng->Release();

	if  ( vbHuman == VARIANT_FALSE ) {
		return 0;
	}
	else {
		return 1;
	}
}


