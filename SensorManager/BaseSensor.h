#pragma once
// ****************************************************************************
// Sensors related includes
// ****************************************************************************
#include <sensors.h>
#include <sensorsapi.h>

struct InclinometerData
{
	float X_Tilt;
	float Y_Tilt;
	float Z_Tilt;
	__int64 InclinometerTime;
};

struct OrientationData
{
	float Matrix[9];
	__int64 OrientationTime;
};

class CBaseSensor
{
public:

    // Constructor and destructor
	CBaseSensor();
	virtual ~CBaseSensor();

    STDMETHOD(OnDataUpdated)(ISensor* pSensor, ISensorDataReport* pNewData, void*pData) = 0;
    STDMETHOD(SetDefaultData)(void*pData) = 0;

};


class CInclinometer : public CBaseSensor
{
public:

	static HRESULT ValidateOutput(ISensor *pSensor);
    STDMETHOD(OnDataUpdated)(ISensor* pSensor, ISensorDataReport* pNewData, void*pData);
    STDMETHOD(SetDefaultData)(void*pData);

};

class COrientationDevice : public CBaseSensor
{
public:

	static HRESULT ValidateOutput(ISensor *pSensor);
    STDMETHOD(OnDataUpdated)(ISensor* pSensor, ISensorDataReport* pNewData, void*pData);
    STDMETHOD(SetDefaultData)(void*pData);

};

