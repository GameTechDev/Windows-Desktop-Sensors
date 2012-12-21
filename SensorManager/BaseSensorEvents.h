#pragma once

// ****************************************************************************
// Sensors related includes
// ****************************************************************************
#include <sensors.h>
#include <sensorsapi.h>

// Forward declarations.
class CSensorManagerEvents;

class CBaseSensorEvents :   public ISensorEvents
{
protected:

public:
    // These three methods are for IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppObject );
    ULONG _stdcall AddRef();
    ULONG _stdcall Release();

    // Constructor and destructor
    CBaseSensorEvents(CSensorManagerEvents* sensorManagerEvents);
    virtual ~CBaseSensorEvents();

    // ISensorEvents method overrides
    STDMETHOD(OnStateChanged)(ISensor* pSensor, SensorState state);
    STDMETHOD(OnDataUpdated)(ISensor* pSensor, ISensorDataReport* pNewData);
    STDMETHOD(OnEvent)(ISensor* pSensor, REFGUID eventID, IPortableDeviceValues* pEventData);
    STDMETHOD(OnLeave)(REFSENSOR_ID sensorID);

protected:
    // Member variable to implement IUnknown reference count
    LONG m_lRefCount;

    CSensorManagerEvents* m_pSensorManagerEvents; // Parent class for callbacks
};





// ****************************************************************************
// Compare function required for GUID's if std::Map is to work
// ****************************************************************************
inline bool operator<( const GUID & lhs, const GUID & rhs ) 
{ 
	return  (memcmp(&lhs, &rhs, sizeof(GUID)) < 0)?1:0; 
} 
