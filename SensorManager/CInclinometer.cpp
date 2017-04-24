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
#include "StdAfx.h"
#include "BaseSensor.h"
#include "MyGuids.h"


///////////////////////////////////////////////////////////////////////////////
//
// CInclinometer::ValidateOutput
//
// Description of function/method:
//        Validate standard 3 axis floating point output format is supported
//
// Parameters:
//        ISensor *pSensor: Sesnor to validate
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CInclinometer::ValidateOutput(ISensor *pSensor)
{
	HRESULT hr = S_OK;

	// Examine the SupportsDataField for SENSOR_DATA_TYPE_LIGHT_LEVEL_LUX.
	VARIANT_BOOL bSupported = VARIANT_FALSE;
	pSensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_X_DEGREES, &bSupported);
	if (FAILED(hr))
	{
		return hr;
	}

	if (bSupported == VARIANT_FALSE)
	{
		// This is not the sensor we want.
		return E_FAIL;
	}
	hr = pSensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_Y_DEGREES, &bSupported);
	if (FAILED(hr))
	{
		return hr;
	}

	if (bSupported == VARIANT_FALSE)
	{
		return E_FAIL;
	}

	hr = pSensor->SupportsDataField(SENSOR_DATA_TYPE_TILT_Z_DEGREES, &bSupported);
	if (FAILED(hr))
	{
		return hr;
	}

	if (bSupported == VARIANT_FALSE)
	{
		return E_FAIL;
	}

	hr = pSensor->SupportsDataField(SENSOR_DATA_TYPE_TIMESTAMP, &bSupported);
	if (FAILED(hr))
	{
		return hr;
	}

	if (bSupported == VARIANT_FALSE)
	{
		return E_FAIL;
	}
	return hr;
}



///////////////////////////////////////////////////////////////////////////////
//
// CInclinometer::OnDataUpdated
//
// Description of function/method:
//        Helper function, get data from a sensor and updates the lux
//
// Parameters:
//        ISensor *pSensor:               Input sensor
//        ISensorDataReport* pDataReport: The data to be read
//		  void* pData:					  pointer to return the data to
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CInclinometer::OnDataUpdated(ISensor *pSensor, ISensorDataReport *pDataReport, void* pData)
{
    HRESULT hr = S_OK;
	InclinometerData* mapTilt = (InclinometerData*)pData;

    if (NULL != pSensor && NULL != pDataReport)
    {
        SENSOR_ID idSensor = GUID_NULL;
        hr = pSensor->GetID(&idSensor);
        if (SUCCEEDED(hr))
        {
            PROPVARIANT pvTilt;
			PROPVARIANT pvTimeStamp;
            PropVariantInit(&pvTilt);
			PropVariantInit(&pvTimeStamp);

            hr = pDataReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_X_DEGREES, &pvTilt);
            if (SUCCEEDED(hr))
            {
                // Save the lux value into our member variable
                mapTilt->X_Tilt = V_R4(&pvTilt);
            }
            PropVariantClear(&pvTilt);
            PropVariantInit(&pvTilt);
            hr = pDataReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_Y_DEGREES, &pvTilt);
            if (SUCCEEDED(hr))
            {
                // Save the lux value into our member variable
                mapTilt->Y_Tilt = V_R4(&pvTilt);
            }
            PropVariantClear(&pvTilt);
            PropVariantInit(&pvTilt);
            hr = pDataReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_Z_DEGREES, &pvTilt);
            if (SUCCEEDED(hr))
            {
                // Save the lux value into our member variable
                mapTilt->Z_Tilt = V_R4(&pvTilt);
            }
            PropVariantClear(&pvTilt);

			hr = pDataReport->GetSensorValue(SENSOR_DATA_TYPE_TIMESTAMP, &pvTimeStamp);
			if (SUCCEEDED(hr))
			{
				mapTilt->InclinometerTime = ((unsigned __int64)pvTimeStamp.filetime.dwHighDateTime<<32) | pvTimeStamp.filetime.dwLowDateTime;
			}
            PropVariantClear(&pvTimeStamp);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CInclinometer::SetDefaultData
//
// Description of function/method:
//        fills out default values when sensor not available
//
// Parameters:
//        void* pData:		pointer to output data
//
// Return Values:
//        S_OK on success, else an error
//
/////////////////////////////////////////////////////////////////////////////////
HRESULT CInclinometer::SetDefaultData( void* pData)
{
    HRESULT hr = S_OK;
	InclinometerData* mapTilt = (InclinometerData*)pData;
	
	mapTilt->X_Tilt = -1.0;
    mapTilt->Y_Tilt = -1.0;
    mapTilt->Z_Tilt = -1.0;
	
	return hr;
}