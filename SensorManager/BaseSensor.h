/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

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

