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
#include "BaseSensorEvents.h"
#include "SensorManagerEvents.h"


///////////////////////////////////////////////////////////////////////////////
//
// CBaseSensorEvents::CBaseSensorEvents
//
// Description of function/method:
//        Constructor.
//
// Parameters:
//        CInclinometerDlg* dlg: Parent dialog for callbacks
//        CInclinometerSensorManagerEvents* sensorManagerEvents:
//            Parent class for callbacks
//
// Return Values:
//        None
//
///////////////////////////////////////////////////////////////////////////////
CBaseSensorEvents::CBaseSensorEvents(CSensorManagerEvents* sensorManagerEvents)
{
    m_lRefCount = 1; //ref count initialized to 1
    m_pSensorManagerEvents = sensorManagerEvents;

}

///////////////////////////////////////////////////////////////////////////////
//
// CBaseSensorEvents::~CBaseSensorEvents
//
// Description of function/method:
//        Destructor. Clean up stored data.
//
// Parameters:
//        none
//
// Return Values:
//        None
//
///////////////////////////////////////////////////////////////////////////////
CBaseSensorEvents::~CBaseSensorEvents()
{

}

///////////////////////////////////////////////////////////////////////////////
//
// CBaseSensorEvents::QueryInterface
//
// Description of function/method:
//        IUnknown method, need to implement to support COM classes that
//        are inherited.
//
// Parameters:
//        REFIID riid:     Input. ID of the interface being requested. Either
//                         IUnknown or one of the two classes we inherit.
//        void** ppObject: Output. Address of pointer variable that receives
//                         the interface pointer requested in riid. Upon 
//                         successful return, *ppvObject contains the requested
//                         interface pointer to the object. If the object does
//                         not support the interface specified in iid,
//                         *ppvObject is set to NULL.
//
// Return Values:
//        S_OK on success, else E_NOINTERFACE
//
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CBaseSensorEvents::QueryInterface(REFIID riid, void** ppObject)
{
    HRESULT hr = S_OK;

    *ppObject = NULL;
    if (riid == __uuidof(ISensorEvents))
    {
        *ppObject = reinterpret_cast<ISensorEvents*>(this);
    }
    else if (riid == IID_IUnknown)
    {
        *ppObject = reinterpret_cast<IUnknown*>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
    {
        (reinterpret_cast<IUnknown*>(*ppObject))->AddRef();
    }

    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CBaseSensorEvents::AddRef
//
// Description of function/method:
//        Increments the reference count for an interface on an object.
//
// Parameters:
//        none
//
// Return Values:
//        Returns an integer from 1 to n, the value of the new reference count.
//
///////////////////////////////////////////////////////////////////////////////
ULONG _stdcall CBaseSensorEvents::AddRef()
{
    m_lRefCount++;
    return m_lRefCount; 
}

///////////////////////////////////////////////////////////////////////////////
//
// CBaseSensorEvents::Release
//
// Description of function/method:
//        Decrements the reference count for the calling interface on a object.
//        If the reference count on the object falls to 0, the object is freed
//        from memory.
//
// Parameters:
//        none
//
// Return Values:
//        Returns an integer from 1 to n, the value of the new reference count.
//
///////////////////////////////////////////////////////////////////////////////
ULONG _stdcall CBaseSensorEvents::Release()
{
    ULONG lRet = --m_lRefCount;

    if (m_lRefCount == 0)
    {
        delete this;
    }

    return lRet;
}

///////////////////////////////////////////////////////////////////////////////
//
// CBaseSensorEvents::OnStateChanged
//
// Description of function/method:
//        Implementation of ISensor.OnStateChanged. Called when permissions of
//        the sensor have changed, such as the sensor being disabled in control
//        panel. If the sensor is not SENSOR_STATE_READY then its lux value
//        should be ignored.
//
// Parameters:
//        ISensor* pSensor:  Sensor that has changed
//        SensorState state: State of the sensor
//
// Return Values:
//        S_OK
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CBaseSensorEvents::OnStateChanged(ISensor *pSensor, SensorState state)
{
    HRESULT hr = S_OK;



    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CBaseSensorEvents::OnDataUpdated
//
// Description of function/method:
//        Implementation of ISensor.OnDataUpdated.  Called when the sensor has
//        published new data.  This reads in the lux value from the report.
//
// Parameters:
//        ISensor* pSensor:            Sensor that has updated data.
//        ISensorDataReport *pNewData: New data to be read
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CBaseSensorEvents::OnDataUpdated(ISensor *pSensor, ISensorDataReport *pNewData)
{

    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
//
// CBaseSensorEvents::OnEvent
//
// Description of function/method:
//        Implementation of ISensor.OnEevent, a generic or custom sensor event.
//        OnDataUpdated is the event this sample uses to get sensor
//        information.
//
// Parameters:
//        ISensor* pSensor:                  Sensor that has the event.
//        GUID& eventID:                     Type of event.
//        IPortableDeviceValues *pEventData: Data to be read.
//
// Return Values:
//        S_OK
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CBaseSensorEvents::OnEvent(ISensor* /*pSensor*/, REFGUID /*eventID*/, IPortableDeviceValues* /*pEventData*/)
{
    // Not implemented
    return E_NOTIMPL;
}

///////////////////////////////////////////////////////////////////////////////
//
// CBaseSensorEvents::OnLeave
//
// Description of function/method:
//        Implementation of ISensor.OnLeave.  This sensor is being removed, so
//        it needs to be deleted and freed.
//
// Parameters:
//        REFSENSOR_ID sensorID: 
//
// Return Values:
//        S_OK
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CBaseSensorEvents::OnLeave(REFSENSOR_ID sensorID)
{
    HRESULT hr = S_OK;

    hr = m_pSensorManagerEvents->RemoveSensor(sensorID); // Callback into parent


    return hr;
}

