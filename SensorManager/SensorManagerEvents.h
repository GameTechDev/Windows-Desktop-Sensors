#pragma once

#include "BaseSensorEvents.h"
#include "BaseSensor.h"
#include <map>
#include <list>

enum SENSORTYPE
{
   SENSOR_NONE,
    SENSOR_INCLINOMETER_3D,
    SENSOR_ORIENTATION,
};

enum SENSORSTATUS{
	SENSOR_STATUS_DISABLED = 0,
	SENSOR_STATUS_NOTFOUND,
	SENSOR_STATUS_ACTIVE,
	SENSOR_STATUS_LOST,
	SENSOR_STATUS_UNKNOWN
};


struct SensorProporties
{
	SENSORSTATUS m_Status;
	SENSORTYPE   m_Type;
	WCHAR*		m_Name;
	ISensor* m_pSensor;
	CBaseSensor* m_SensorProcessor;
};


class SensorRequest
{

public:
	SensorRequest(SENSOR_TYPE_ID Type)
	{
		m_Type = Type;
	};
		SENSOR_TYPE_ID m_Type;
	~SensorRequest(){};
};


class CSensorManagerEvents :   public ISensorManagerEvents
{
	SENSORSTATUS m_StatusGlobal;

    std::map<SENSOR_ID, SensorProporties> m_Properties;  
	std::list<SENSOR_ID> m_AvailableSensors;
	std::list<SensorRequest*> RequestedSensors;

	COrientationDevice m_OrientationDevice;
	CInclinometer  m_Inclinometer;

	HRESULT GetSensorProperty(SENSOR_ID SensorID,ISensor* pSensor, REFPROPERTYKEY pk);
	HRESULT ChangeSensitivity(ISensor* pSensor);
	void Setstatus(SENSOR_ID SensorID, SENSORSTATUS Status);
	void AddRequestedSensor(SENSOR_TYPE_ID Type);

public:
    // These three methods are for IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppObject );
    ULONG _stdcall AddRef();
    ULONG _stdcall Release();

    // Constructor and destructor
    CSensorManagerEvents();
    virtual ~CSensorManagerEvents();

    // Initialize and Uninitialize called by parent dialog
    HRESULT Initialize(int NumSensorTypes, ...);
    HRESULT Uninitialize();

    // ISensorManagerEvents method override
    STDMETHOD(OnSensorEnter)(ISensor* pSensor, SensorState state);
	HRESULT RemoveSensor(REFSENSOR_ID sensorID);

	SENSORSTATUS GetStatus(SENSOR_ID SensorID);
	WCHAR* GetDeviceName(SENSOR_ID SensorID);
	int  GetNumSensors(SENSORTYPE Type = SENSOR_NONE);
	SENSOR_ID GetSensor(int Num, SENSORTYPE Type = SENSOR_NONE);
	HRESULT GetData(REFSENSOR_ID sensorID, void* PData);
	SENSORTYPE GetDeviceType(SENSOR_ID SensorID);

private:
    // Member variable to implement IUnknown reference count
    LONG m_lRefCount;

    HRESULT AddSensor(ISensor* pSensor);

	HRESULT RemoveSensor(ISensor* pSensor);

    ISensorManager* m_spISensorManager;      // Global to keep reference for life of class
	CBaseSensorEvents* m_SensorEvents;
};