//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------
#ifndef __WINDOWS_SENSORS_H__
#define __WINDOWS_SENSORS_H__

#include <stdio.h>
#include "CPUT_DX11.h"
#include <D3D11.h>
#include <time.h>
#include "SensorManager\SensorManagerEvents.h"
#define INITGUID
#include "SensorManager\MyGuids.h"

// define some controls
const CPUTControlID ID_MAIN_PANEL = 10;
const CPUTControlID ID_SECONDARY_PANEL = 20;
const CPUTControlID ID_FULLSCREEN_BUTTON = 100;
const CPUTControlID ID_IGNORE_CONTROL_ID = -1;

const CPUTControlID ID_SELECT_SENSOR = 200;

const CPUTControlID ID_SENSOR_ZERO = 300;

#define MAX_VELOCITY 2400.0f
#define MIN_VELOCITY 0.0f

//-----------------------------------------------------------------------------
class WindowsSensors : public CPUT_DX11
{
private:
    float                   mfElapsedTime;
    CPUTCameraController   *mpCameraController;

    CPUTAssetSet           *mpShadowCameraSet;
    CPUTRenderTargetDepth  *mpShadowRenderTarget;


    CPUTAssetSet           *mpBikeSet;
    CPUTAssetSet           *mpLevelSet;
    CPUTAssetSet           *mpSkyboxSet;

    CSensorManagerEvents   *mpSensorManager;
    SENSOR_ID               mCurrentSensor;
    float3                  mSensorZero;

    Plane                   mWalls[4];

    CPUTModel              *mpBikeModel;
    CPUTText               *mpSensorText;
    CPUTText               *mpSensorZeroText;

    struct
    {
        float   velocity;
        float   angle;
        float   turnDelta;
        float   acceleration;
    }                       mBike;

public:
    WindowsSensors() 
        : mpLevelSet(NULL)
        , mpBikeSet(NULL)
        , mpCameraController(NULL)
        , mpShadowCameraSet(NULL)
        , mpBikeModel(NULL)
        , mpSkyboxSet(NULL)
        , mSensorZero(0.0f)
    {
    }
    ~WindowsSensors()
    {
        SAFE_DELETE(mpSensorManager);
        // Note: these two are defined in the base.  We release them because we addref them.
        SAFE_RELEASE(mpCamera);
        SAFE_RELEASE(mpShadowCamera);

        SAFE_RELEASE(mpBikeModel);
        SAFE_RELEASE(mpBikeSet);
        SAFE_RELEASE(mpLevelSet);
        SAFE_RELEASE(mpSkyboxSet);

        SAFE_DELETE( mpCameraController );
        SAFE_RELEASE(mpShadowCameraSet);
        SAFE_DELETE( mpShadowRenderTarget );
    }
    CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key);
    CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state);
    void                 HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl );

    void Create();
    void Render(double deltaSeconds);
    void Update(double deltaSeconds);
    void ResizeWindow(UINT width, UINT height);
};
#endif // __CPUT_SAMPLESTARTDX11_H__
