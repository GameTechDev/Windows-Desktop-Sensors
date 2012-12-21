#include "StdAfx.h"
#include "SensorManagerEvents.h"
#include "MyGuids.h"

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::CSensorManagerEvents
//
// Description of function/method:
//        Constructor.
//
// Parameters:
//        CInclinometerDlg* dlg: pointer to parent dialog for callbacks
//
// Return Values:
//        None
//
///////////////////////////////////////////////////////////////////////////////
CSensorManagerEvents::CSensorManagerEvents()
{
    ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // Initializes COM

	m_SensorEvents = new CBaseSensorEvents(this );

	m_StatusGlobal = SENSOR_STATUS_NOTFOUND;
	
	RequestedSensors.clear();
	m_Properties.clear();
	m_AvailableSensors.clear();
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::~CSensorManagerEvents
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
CSensorManagerEvents::~CSensorManagerEvents()
{
	std::list<SensorRequest*> ::iterator it;
	it=RequestedSensors.begin();

	while (it != RequestedSensors.end()) 
	{
		delete *it;
		RequestedSensors.erase(it++);
	}

	std::map<SENSOR_ID, SensorProporties>::const_iterator	pISensorIter;

	pISensorIter = m_Properties.begin();
	while (pISensorIter != m_Properties.end()) 
	{
		free((*pISensorIter).second.m_Name);
		m_Properties.erase(pISensorIter++);
	}

	m_Properties.clear();
	m_AvailableSensors.clear();

	delete m_SensorEvents;

}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::QueryInterface
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
STDMETHODIMP CSensorManagerEvents::QueryInterface(REFIID riid, void** ppObject)
{
    HRESULT hr = S_OK;

    *ppObject = NULL;
    if (riid == __uuidof(ISensorManagerEvents))
    {
        *ppObject = reinterpret_cast<ISensorManagerEvents*>(this);
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
// CSensorManagerEvents::AddRef
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
ULONG _stdcall CSensorManagerEvents::AddRef()
{
    m_lRefCount++;
    return m_lRefCount; 
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::Release
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
ULONG _stdcall CSensorManagerEvents::Release()
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
// CSensorManagerEvents::Initialize
//
// Description of function/method:
//        Initialize the sensor data.
//
// Parameters:
//        none
//
// Return Values:
//        HRESULT S_OK on success
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSensorManagerEvents::Initialize(int NumSensorTypes, ...)
{
    HRESULT hr;
	va_list vl;
	SENSOR_TYPE_ID TypeID;


	va_start( vl, NumSensorTypes );

	// Step through the list.
	  for ( int x = 0; x < NumSensorTypes; x++ )        // Loop until all numbers are added
	  {
		TypeID = va_arg( vl, SENSOR_TYPE_ID );
		AddRequestedSensor(TypeID);
	}
   va_end( vl );




	hr = ::CoCreateInstance(CLSID_SensorManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_spISensorManager));
	if (FAILED(hr))
	{
		OutputDebugString(L"Unable to CoCreateInstance() the SensorManager.");
		return E_FAIL;
	}

    if (SUCCEEDED(hr))
    {
        hr = m_spISensorManager->SetEventSink(this);
        if (SUCCEEDED(hr))
        {
            // Find all Ambient Light Sensors
            ISensorCollection* spSensors;
            hr = m_spISensorManager->GetSensorsByCategory(SENSOR_CATEGORY_ALL, &spSensors);
			if(FAILED(hr))
			{
				OutputDebugString(L"Unable to find any sensors on the computer.");
				return E_FAIL;
			}
            if (SUCCEEDED(hr) && NULL != spSensors)
            {
                ULONG ulCount = 0;
                hr = spSensors->GetCount(&ulCount);
                if (SUCCEEDED(hr))
                {


					// Make a SensorCollection with only the sensors we want to get permission to access.
					ISensorCollection *pSensorCollection = NULL;
					HRESULT hr = ::CoCreateInstance(CLSID_SensorCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSensorCollection));
					if (FAILED(hr))
					{
						OutputDebugString(L"Unable to CoCreateInstance() a SensorCollection.");
						return E_FAIL;
					}
                    for(ULONG i=0; i < ulCount; i++)
                    {
                        ISensor* spSensor;
                        hr = spSensors->GetAt(i, &spSensor);
						SENSOR_TYPE_ID idType = GUID_NULL;
						hr = spSensor->GetType(&idType);

						std::list<SensorRequest*> ::iterator it;
						for (it=RequestedSensors.begin(); it!=RequestedSensors.end() ; ++it)
						{
							if((*it)->m_Type == idType)
								break;
						}
						if(it==RequestedSensors.end())
						{
							// we have never requested this sensor;
							continue;
						}

						pSensorCollection->Clear();
						pSensorCollection->Add(spSensor);
						// Have the SensorManager prompt the end-user for permission.
						hr = m_spISensorManager->RequestPermissions(NULL, pSensorCollection, TRUE);
						if (FAILED(hr))
						{
							Setstatus(GUID_NULL, SENSOR_STATUS_DISABLED);
							OutputDebugString(L"No permission to access Requested Sensor.");
						}
						spSensor->Release();
					}
					pSensorCollection->Release();

                    for(ULONG i=0; i < ulCount; i++)
                    {
                        ISensor* spSensor;
                        hr = spSensors->GetAt(i, &spSensor);
                        if (SUCCEEDED(hr))
                        {
							SensorState state = SENSOR_STATE_ERROR;
							hr = spSensor->GetState(&state);
							OnSensorEnter(spSensor,state);
							spSensor->Release();
                        }
                    }
                }
            }
			spSensors->Release();
        }
    }
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::Uninitialize
//
// Description of function/method:
//        Uninitialize the sensor data.
//
// Parameters:
//        none
//
// Return Values:
//        HRESULT S_OK on success
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSensorManagerEvents::Uninitialize()
{
    HRESULT hr = S_OK;


	std::map<SENSOR_ID, SensorProporties>::const_iterator	pISensorIter;
		
	if(!m_Properties.empty())
	{
		pISensorIter = m_Properties.begin();
		while (pISensorIter != m_Properties.end() ) 
		{
			RemoveSensor(((*pISensorIter).second).m_pSensor);
			pISensorIter++;
		}
	}


    if (NULL != m_spISensorManager)
    {
        hr = m_spISensorManager->SetEventSink(NULL);
		ULONG RefCnt = m_spISensorManager->Release();

    }
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::OnSensorEnter
//
// Description of function/method:
//        Implementation of ISensorManager.OnSensorEnter. Check if the sensor
//        is an Ambient Light Sensor and if so then add the sensor.
//
// Parameters:
//        ISensor* pSensor:  Sensor that has been installed
//        SensorState state: State of the sensor
//
// Return Values:
//        S_OK on success, else an error.
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSensorManagerEvents::OnSensorEnter(ISensor* pSensor, SensorState state)
{
    HRESULT hr = S_OK;

    if (NULL != pSensor)
    {
        SENSOR_TYPE_ID idType = GUID_NULL;
        hr = pSensor->GetType(&idType);
        if (SUCCEEDED(hr))
        {
			hr = AddSensor(pSensor);
        }	
    }
    else
    {
        hr = E_POINTER;
    }
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::AddSensor
//
// Description of function/method:
//        Helper function, sets up event sinking for a sensor and saves the
//        sensor.
//
// Parameters:
//        ISensor *pSensor: Input sensor
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSensorManagerEvents::AddSensor(ISensor *pSensor)
{
    HRESULT hr = S_OK;
    SENSOR_ID idSensor = GUID_NULL;
	std::map<SENSOR_ID, SensorProporties>::const_iterator	pISensorIter;


    if (NULL != pSensor)
    {
		SensorState state;
		pSensor->GetState(&state);

		SENSOR_TYPE_ID idType = GUID_NULL;
		hr = pSensor->GetType(&idType);


        if (SUCCEEDED(hr))
        {
            // Get the sensor's ID to be used as a key to store the sensor
            hr = pSensor->GetID(&idSensor);
	
            if (SUCCEEDED(hr))
            {
				std::list<SensorRequest*> ::iterator it;
				for (it=RequestedSensors.begin(); it!=RequestedSensors.end() ; ++it)
				{
					if((*it)->m_Type == idType)
						break;
				}
				if(it==RequestedSensors.end())
				{
					// we have never requested this sensor;
					return E_FAIL;
				}
		

				// Check for access permissions, request permission if necessary.
				if (state == SENSOR_STATE_ACCESS_DENIED)
				{
					return E_FAIL;
				}

				if (IsEqualIID(idType, SENSOR_TYPE_INCLINOMETER_3D))
				{
					if(CInclinometer::ValidateOutput(pSensor) != S_OK)
  						return E_FAIL;

				}
				else if (IsEqualIID(idType, SENSOR_TYPE_AGGREGATED_DEVICE_ORIENTATION))
				{
					if(COrientationDevice::ValidateOutput(pSensor) != S_OK)
  						return E_FAIL;
				}


				if(m_Properties.empty() || m_Properties.find(idSensor)==m_Properties.end())
				{
					GetSensorProperty(idSensor,pSensor,SENSOR_PROPERTY_FRIENDLY_NAME);
					if (IsEqualIID(idType, SENSOR_TYPE_INCLINOMETER_3D))
					{
						m_Properties[idSensor].m_Type = SENSOR_INCLINOMETER_3D;
						m_Properties[idSensor].m_SensorProcessor =&m_Inclinometer;
					}
					else if (IsEqualIID(idType, SENSOR_TYPE_AGGREGATED_DEVICE_ORIENTATION))
					{
						m_Properties[idSensor].m_Type = SENSOR_ORIENTATION;
						m_Properties[idSensor].m_SensorProcessor = &m_OrientationDevice;
					}
				}

				std::list<SENSOR_ID> ::iterator iSensors;
				for (iSensors=m_AvailableSensors.begin(); iSensors!=m_AvailableSensors.end() ; ++iSensors)
				{
					if(IsEqualIID(idSensor,(*iSensors)))
						break;
				}
				if(iSensors==m_AvailableSensors.end())
				{
					// First time this sensor has been found so add it
					m_AvailableSensors.push_back(idSensor);
				}

				
  

				hr = pSensor->SetEventSink(m_SensorEvents);

				GUID pguid[1];

				// Only get state changed notifications as we will directly poll the sensor
				pguid[0] = SENSOR_EVENT_STATE_CHANGED;
				hr = pSensor->SetEventInterest(pguid,1);


                // Enter the sensor into the map and take the ownership of its lifetime
                pSensor->AddRef(); // the sensor is released in the destructor

				m_Properties[idSensor].m_pSensor = pSensor;
            }
			ChangeSensitivity(pSensor);
			Setstatus(idSensor,SENSOR_STATUS_ACTIVE);
        }
    }
    else
    {
        hr = E_POINTER;
    }



    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::GetSensorProperty
//
// Description of function/method:
//        Retrieves sensor properties to be stored for future reference
//
// Parameters:
//        SENSOR_ID SensorID:	Unique ID for sensor used as Key to std::MAP 
//		  ISensor* pSensor:		Sensor to be queried 
//		  REFPROPERTYKEY pk:	Property to be set, currently only Name is stored
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSensorManagerEvents::GetSensorProperty(SENSOR_ID SensorID, ISensor* pSensor, REFPROPERTYKEY pk)
{

    HRESULT hr = S_OK;

    PROPVARIANT pv = {};
    hr = pSensor->GetProperty(pk, &pv);

    if(SUCCEEDED(hr))
    {
        if(pv.vt == VT_UI4)  // Number
        {
            wprintf_s(L"\nSensor integer value: %u\n", pv.ulVal);
        }
        else if(pv.vt == VT_LPWSTR)  // String
        {
			if(pk == SENSOR_PROPERTY_FRIENDLY_NAME)
			{
				int size = (int)wcslen(pv.pwszVal)+1;
				m_Properties[SensorID].m_Name = (WCHAR*)malloc(size*sizeof(WCHAR));
				wsprintf(m_Properties[SensorID].m_Name,L"%s",pv.pwszVal);
			}
        }
        else  // Interface or vector
        {
            wprintf_s(L"\nSensor property is a compound type. Unable to print value.");
        }
    }

    PropVariantClear(&pv);
    return hr;    
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::ChangeSensitivity
//
// Description of function/method:
//       Tweaks the sensors sensitivity
//
// Parameters:
//		  ISensor* pSensor:		Sensor to be set 
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSensorManagerEvents::ChangeSensitivity(ISensor* pSensor)
{
    HRESULT hr = S_OK;

    IPortableDeviceValues* pPropsToSet = NULL; // Input
    IPortableDeviceValues* pPropsReturn = NULL; // Output

    // Create the input object.
    hr = CoCreateInstance(__uuidof(PortableDeviceValues),
                            NULL,
                            CLSCTX_INPROC_SERVER,                           
                            IID_PPV_ARGS(&pPropsToSet));

    if(SUCCEEDED(hr))
    {
        // Add the current report interval property.
        hr = pPropsToSet->SetUnsignedIntegerValue(SENSOR_PROPERTY_CURRENT_REPORT_INTERVAL, 30);
    }

    if(SUCCEEDED(hr))
    {
        // Only setting a single property, here.
        hr = pSensor->SetProperties(pPropsToSet, &pPropsReturn);
    }

    // Test for failure.
    if(hr == S_FALSE)
    {
        HRESULT hrError = S_OK;
      
        // Check results for failure.
        hr = pPropsReturn->GetErrorValue(SENSOR_PROPERTY_CURRENT_REPORT_INTERVAL, &hrError);

        if(SUCCEEDED(hr))
        {
            // Print an error message.
			WCHAR szBuffer[256];
            // Print an error message.
            swprintf_s(szBuffer,256,L"\nSetting current report interval failed with error 0x%X\n", hrError);
			OutputDebugString(szBuffer);

            // Return the error code.
            hr = hrError;
        }
    }
    else if(hr == E_ACCESSDENIED)
    {
        // No permission. Take appropriate action.
    }

	pPropsToSet->Release();
    pPropsReturn->Release();

	return hr;
} 


///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::Setstatus
//
// Description of function/method:
//       Used to stor a global record of status of all sensors, even when removed
//
// Parameters:
//		  SENSOR_ID SensorID:		Store new status in global std::MAP
//		  SENSORSTATUS Status:		Current Status
//
// Return Values:
//        none
//
///////////////////////////////////////////////////////////////////////////////
void CSensorManagerEvents::Setstatus(SENSOR_ID SensorID, SENSORSTATUS Status)
{
	if(IsEqualGUID(SensorID, GUID_NULL))
	{
		m_StatusGlobal = Status;
	}
	else
	{
		m_Properties[SensorID].m_Status =  Status;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::GetStatus
//
// Description of function/method:
//       Return the status of a sensor even if its been removed
//
// Parameters:
//        SENSOR_ID SensorID:	Unique ID for sensor used as Key to std::MAP 
//
// Return Values:
//        SENSORSTATUS
//
///////////////////////////////////////////////////////////////////////////////
SENSORSTATUS CSensorManagerEvents::GetStatus(SENSOR_ID SensorID) 
{
	std::map<SENSOR_ID, SensorProporties>::const_iterator	pISensorIter;

	SENSORSTATUS Status;

	if(!m_Properties.empty())
	{
		if(IsEqualGUID(SensorID, GUID_NULL))
		{
			pISensorIter = m_Properties.begin();
		}
		else
		{
			pISensorIter = m_Properties.find(SensorID);
		}
			
		Status = (*pISensorIter).second.m_Status;
	}
	else
	{
		Status = m_StatusGlobal;
	}

	return Status;
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::AddRequestedSensor
//
// Description of function/method:
//       Add a new sensor type to be tracked
//
// Parameters:
//		  SENSOR_TYPE_ID Type:  ENUM of requested sensort type
//
// Return Values:
//        none
//
///////////////////////////////////////////////////////////////////////////////
void CSensorManagerEvents::AddRequestedSensor(SENSOR_TYPE_ID Type)
{
	RequestedSensors.push_back(new SensorRequest(Type));
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::RemoveSensor
//
// Description of function/method:
//        Helper function, clears the event sink for the sensor and then
//        releases the sensor.
//
// Parameters:
//        SENSOR_ID SensorID:	Unique ID for sensor used as Key to std::MAP 
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSensorManagerEvents::RemoveSensor(REFSENSOR_ID sensorID)
{
    HRESULT hr = S_OK;
	std::map<SENSOR_ID, SensorProporties>::iterator	pISensorIter;

	if(!m_Properties.empty())
	{
		pISensorIter = m_Properties.find(sensorID);		
			
		Setstatus(sensorID,SENSOR_STATUS_LOST);

		if((((*pISensorIter).second)).m_pSensor)
		{
			(((*pISensorIter).second)).m_pSensor->Release();
			(((*pISensorIter).second)).m_pSensor = NULL;
			OutputDebugString(L" Removed Sensor\n");
		}
	}


    return hr;
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::RemoveSensor
//
// Description of function/method:
//       Unhooks a sensor 
//
// Parameters:
//		  ISensor* pSensor
//
// Return Values:
//        S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSensorManagerEvents::RemoveSensor(ISensor* pSensor)
{
    HRESULT hr = S_OK;

    // Release the event and ISensor objecets
    if (NULL != pSensor)
    {
        hr = pSensor->SetEventSink(NULL); // This also decreases the ref count of the sink object.

        SENSOR_ID idSensor = GUID_NULL;
        hr = pSensor->GetID(&idSensor);

		RemoveSensor(idSensor);
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}



///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::GetDeviceName
//
// Description of function/method:
//       return previously stored device name from global std::map 
//
// Parameters:
//        SENSOR_ID SensorID:	Unique ID for sensor used as Key to std::MAP 
//
// Return Values:
//        WCHAR string containing device name from driver, or no device found
//
///////////////////////////////////////////////////////////////////////////////
WCHAR* CSensorManagerEvents::GetDeviceName(SENSOR_ID SensorID)
{
	std::map<SENSOR_ID, SensorProporties>::const_iterator	pISensorIter;
	static WCHAR* szNoDevice = L"No Device Found";

	WCHAR* pName;

	if(!m_Properties.empty())
	{
		if(IsEqualGUID(SensorID, GUID_NULL))
		{
			pISensorIter = m_Properties.begin();
		}
		else
		{
			pISensorIter = m_Properties.find(SensorID);
		}
			
		pName = (*pISensorIter).second.m_Name;
	}
	else
	{
		pName = szNoDevice;
	}

	return pName;
}


///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::GetDeviceName
//
// Description of function/method:
//       return previously stored sensor type
//
// Parameters:
//        SENSOR_ID SensorID:	Unique ID for sensor used as Key to std::MAP 
//
// Return Values:
//       SENSORTYPE or SENSOR_NONE
//
///////////////////////////////////////////////////////////////////////////////
SENSORTYPE CSensorManagerEvents::GetDeviceType(SENSOR_ID SensorID)
{
	std::map<SENSOR_ID, SensorProporties>::const_iterator	pISensorIter;
	SENSORTYPE Type;


	if(!m_Properties.empty())
	{
		if(IsEqualGUID(SensorID, GUID_NULL))
		{
			pISensorIter = m_Properties.begin();
		}
		else
		{
			pISensorIter = m_Properties.find(SensorID);
		}
			
		Type = (*pISensorIter).second.m_Type;
	}
	else
	{
		Type = SENSOR_NONE;
	}

	return Type;
}

///////////////////////////////////////////////////////////////////////////////
//
// CSensorManagerEvents::GetNumSensors
//
// Description of function/method:
//      sensor count or either spesific sensor type or all sensors
//
// Parameters:
//        SENSORTYPE Type: Type of Sensor to enumerate
//
// Return Values:
//       number of sensors of requested type
//
///////////////////////////////////////////////////////////////////////////////
int  CSensorManagerEvents::GetNumSensors(SENSORTYPE Type)
{
	int Num = 0;
	if(Type == SENSOR_NONE)
		return (int)m_Properties.size();

	std::map<SENSOR_ID, SensorProporties>::const_iterator	pISensorIter;
		
	if(!m_Properties.empty())
	{
		pISensorIter = m_Properties.begin();
		while (pISensorIter != m_Properties.end() ) 
		{
			if((*pISensorIter).second.m_Type == Type)
				Num++;
			pISensorIter++;
		}
	}

	return Num;
}

///////////////////////////////////////////////////////////////////////////////
// CSensorManagerEvents::GetSensor
//
// Description of function/method:
//       Gets a unique ID for a sensor
//
// Parameters:
//        int Num:   Sensor to retrieve
//		  SENSORTYPE Type: Type of sensors to enumerate from
//
// Return Values:
//       Unique ID
//
///////////////////////////////////////////////////////////////////////////////
SENSOR_ID CSensorManagerEvents::GetSensor(int Num, SENSORTYPE Type)
{
	SENSOR_ID  ID;
	bool found=false;

	if(!m_AvailableSensors.empty())
	{
		std::list<SENSOR_ID> ::iterator iSensors;

		iSensors = m_AvailableSensors.begin();

		while (iSensors != m_AvailableSensors.end() && !found) 
		{
			if((m_Properties[*iSensors].m_Type == Type) || (Type == SENSOR_NONE))
			{
				if(!Num)
				{
					ID = *iSensors;
					found = true;
				}
				Num--;
			}
			iSensors++;
		}

	}
	else
		ID = GUID_NULL;

	return ID;
}

///////////////////////////////////////////////////////////////////////////////
// CSensorManagerEvents::GetData
//
// Description of function/method:
//       Gets a unique ID for a sensor
//
// Parameters:
//        REFSENSOR_ID sensorID:  Unique ID to sensor
//		  void* pData:  pointer to returned data
//
// Return Values:
//           S_OK on success, else an error
//
///////////////////////////////////////////////////////////////////////////////
HRESULT CSensorManagerEvents::GetData(REFSENSOR_ID sensorID, void* pData)
{
    HRESULT hr = E_FAIL;

	std::map<SENSOR_ID, SensorProporties>::const_iterator	pISensorIter;
	ISensorDataReport* d = 0;

	pISensorIter = m_Properties.find(sensorID);
	if (pISensorIter != m_Properties.end())
	{	
		if(((*pISensorIter).second).m_pSensor)
		{
			hr = ((*pISensorIter).second).m_pSensor->GetData(&d);
		}

		if (FAILED(hr))
		{
			OutputDebugString(L" FAILED to get data\n");
			((*pISensorIter).second).m_SensorProcessor->SetDefaultData(pData);
		}
		else
		{
			((*pISensorIter).second).m_SensorProcessor->OnDataUpdated(((*pISensorIter).second).m_pSensor,d,pData);
			d->Release();
		}
	}
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    return hr;
}