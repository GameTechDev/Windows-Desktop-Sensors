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
// COrientationDevice::ValidateOutput
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
HRESULT COrientationDevice::ValidateOutput(ISensor *pSensor)
{
	HRESULT hr;

	VARIANT_BOOL bSupported = VARIANT_FALSE;
	hr = pSensor->SupportsDataField(SENSOR_DATA_TYPE_ROTATION_MATRIX, &bSupported);
	if (FAILED(hr))
	{
		return hr;
	}

	if (bSupported == VARIANT_FALSE)
	{
		// This is not the sensor we want.
		return E_FAIL;
	}

	hr = pSensor->SupportsDataField(SENSOR_DATA_TYPE_TIMESTAMP, &bSupported);
	if (FAILED(hr))
	{
		return hr;
	}

	return hr;
}




///////////////////////////////////////////////////////////////////////////////
//
// COrientationDevice::OnDataUpdated
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
HRESULT COrientationDevice::OnDataUpdated(ISensor *pSensor, ISensorDataReport *pDataReport, void* pData)
{
    HRESULT hr = S_OK;
	OrientationData* OrientData = (OrientationData*)pData;

    if (NULL != pSensor && NULL != pDataReport)
    {
        SENSOR_ID idSensor = GUID_NULL;
        hr = pSensor->GetID(&idSensor);
        if (SUCCEEDED(hr))
        {
			PROPVARIANT pvRot;
			PROPVARIANT pvTimeStamp;

			PropVariantInit(&pvRot);
			PropVariantInit(&pvTimeStamp);

			hr = pDataReport->GetSensorValue(SENSOR_DATA_TYPE_ROTATION_MATRIX, &pvRot);
			if (SUCCEEDED(hr))
			{
				if (pvRot.vt == (VT_UI1|VT_VECTOR))
				{
					// Item count for the array.
					UINT cElement = pvRot.caub.cElems/sizeof(float);
					memcpy(OrientData->Matrix,pvRot.caub.pElems, pvRot.caub.cElems);
				}
			}

			hr = pDataReport->GetSensorValue(SENSOR_DATA_TYPE_TIMESTAMP, &pvTimeStamp);
			if (SUCCEEDED(hr))
			{
				OrientData->OrientationTime = ((unsigned __int64)pvTimeStamp.filetime.dwHighDateTime<<32) | pvTimeStamp.filetime.dwLowDateTime;
			}

			PropVariantClear(&pvRot);
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
// COrientationDevice::SetDefaultData
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
HRESULT COrientationDevice::SetDefaultData( void* pData)
{
	HRESULT hr = S_OK;
	OrientationData* OrientData = (OrientationData*)pData;

    OrientData->Matrix[0] = 1.0;
    OrientData->Matrix[1] = 0.0;
    OrientData->Matrix[2] = 0.0;


	OrientData->Matrix[3] = 0.0;
    OrientData->Matrix[4] = 1.0;
    OrientData->Matrix[5] = 0.0;


    OrientData->Matrix[6] = 0.0;
    OrientData->Matrix[7] = 0.0;
    OrientData->Matrix[8] = 1.0;

	return hr;
}



