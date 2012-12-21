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
#include "WindowsSensors.h"
#include "CPUTRenderTarget.h"

//
// Screen auto-rotation defines
// We load the "SetDisplayAutoRotationPreferences" function with GetProcAddress so one
// binary will run in both Windows 7 and Windows 8
//
#if (WINVER <= 0x0601 /* _WIN32_WINNT_WIN7 */ )
typedef enum ORIENTATION_PREFERENCE {

    ORIENTATION_PREFERENCE_NONE              = 0x0,

    ORIENTATION_PREFERENCE_LANDSCAPE         = 0x1,

    ORIENTATION_PREFERENCE_PORTRAIT          = 0x2,

    ORIENTATION_PREFERENCE_LANDSCAPE_FLIPPED = 0x4,

    ORIENTATION_PREFERENCE_PORTRAIT_FLIPPED  = 0x8

} ORIENTATION_PREFERENCE;
typedef BOOL (WINAPI *pSDARP)(ORIENTATION_PREFERENCE orientation);

static pSDARP SetDisplayAutoRotationPreferences = NULL;
#endif // #if (WINVER <= 0x0601)

static const UINT SHADOW_WIDTH_HEIGHT = 4096;

// set file to open
static cString g_OpenFilePath;
static cString g_OpenShaderPath;
static cString g_OpenFileName;

extern float3 gLightDir;
extern char *gpDefaultShaderSource;

// Application entry point.  Execution begins here.
//-----------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Prevent unused parameter compiler warnings
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

#ifdef DEBUG
    // tell VS to report leaks at any exit of the program
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
    CPUTResult result=CPUT_SUCCESS;
    int returnCode=0;

    // create an instance of my sample
    WindowsSensors *pSample = new WindowsSensors();
    
    // We make the assumption we are running from the executable's dir in
    // the CPUT SampleStart directory or it won't be able to use the relative paths to find the default
    // resources    
    cString ResourceDirectory;
    CPUTOSServices::GetOSServices()->GetExecutableDirectory(&ResourceDirectory);
    ResourceDirectory.append(_L("CPUT\\resources\\"));
    
    // Initialize the system and give it the base CPUT resource directory (location of GUI images/etc)
    // For now, we assume it's a relative directory from the executable directory.  Might make that resource
    // directory location an env variable/hardcoded later
    pSample->CPUTInitialize(ResourceDirectory); 

    // window and device parameters
    CPUTWindowCreationParams params;
    params.deviceParams.refreshRate         = 1;
    params.deviceParams.swapChainBufferCount= 1;
    params.deviceParams.swapChainFormat     = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    params.deviceParams.swapChainUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;

    // parse out the parameter settings or reset them to defaults if not specified
    cString AssetFilename;
    cString CommandLine(lpCmdLine);
    pSample->CPUTParseCommandLine(CommandLine, &params, &AssetFilename);       

    // parse out the filename of the .set file to open (if one was given)
    if(AssetFilename.size())
    {
        // strip off the target .set file, and parse it's full path
        cString PathAndFilename, Drive, Dir, Ext;  
        g_OpenFilePath.clear();
        
        // resolve full path, and check to see if the file actually exists
        CPUTOSServices::GetOSServices()->ResolveAbsolutePathAndFilename(AssetFilename, &PathAndFilename);
        result = CPUTOSServices::GetOSServices()->DoesFileExist(PathAndFilename);
        if(CPUTFAILED(result))
        {
            cString ErrorMessage = _L("File specified in the command line does not exist: \n")+PathAndFilename;
            #ifndef DEBUG
            DEBUGMESSAGEBOX(_L("Error loading command line file"), ErrorMessage);
            #else
            ASSERT(false, ErrorMessage);
            #endif
        }
        
        // now parse through the path and removing the trailing \\Asset\\ directory specification
        CPUTOSServices::GetOSServices()->SplitPathAndFilename(PathAndFilename, &Drive, &Dir, &g_OpenFileName, &Ext);
        // strip off any trailing \\ 
        size_t index=Dir.size()-1;
        if(Dir[index]=='\\' || Dir[index]=='/')
        {
            index--;
        }

        // strip off \\Asset
        for(size_t ii=index; ii>=0; ii--)
        {
            if(Dir[ii]=='\\' || Dir[ii]=='/')
            {
                Dir = Dir.substr(0, ii+1); 
                g_OpenFilePath = Drive+Dir; 
                index=ii;
                break;
            }
        }        
        
        // strip off \\<setname> 
        for(size_t ii=index; ii>=0; ii--)
        {
            if(Dir[ii]=='\\' || Dir[ii]=='/')
            {
                Dir = Dir.substr(0, ii+1); 
                g_OpenShaderPath = Drive + Dir + _L("\\Shader\\");
                break;
            }
        }
    }

    // create the window and device context
    result = pSample->CPUTCreateWindowAndContext(_L("CPUTWindow DirectX 11"), params);
    ASSERT( CPUTSUCCESS(result), _L("CPUT Error creating window and context.") );
    
    // start the main message loop
    returnCode = pSample->CPUTMessageLoop();

	pSample->DeviceShutdown();

    // cleanup resources
    SAFE_DELETE(pSample);

    return returnCode;
}

// Handle OnCreation events
//-----------------------------------------------------------------------------
void WindowsSensors::Create()
{    
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    gLightDir.normalize();

    // TODO: read from cmd line, using these as defaults
    pAssetLibrary->SetMediaDirectoryName(_L("Media\\"));

    CPUTGuiControllerDX11 *pGUI = CPUTGetGuiController();

    //
    // Create some controls
    //    
    pGUI->CreateButton(_L("Fullscreen"), ID_FULLSCREEN_BUTTON, ID_MAIN_PANEL);
    

    //
    // Create Static text
    //
    pGUI->CreateText( _L("F1 for Help"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("[Escape] to quit application"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("A,S,D,F - move camera position"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("Q - camera position up"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("E - camera position down"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);
    pGUI->CreateText( _L("mouse + right click - camera look location"), ID_IGNORE_CONTROL_ID, ID_SECONDARY_PANEL);

    pGUI->SetActivePanel(ID_MAIN_PANEL);
    pGUI->DrawFPS(true);

    // Add our programatic (and global) material parameters
    CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerFrameValues"), _L("$cbPerFrameValues") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("cbPerModelValues"), _L("#cbPerModelValues") );
    CPUTMaterial::mGlobalProperties.AddValue( _L("_Shadow"), _L("$shadow_depth") );

    // Create default shaders
    CPUTPixelShaderDX11  *pPS       = CPUTPixelShaderDX11::CreatePixelShaderFromMemory(            _L("$DefaultShader"), CPUT_DX11::mpD3dDevice,          _L("PSMain"), _L("ps_4_0"), gpDefaultShaderSource );
    CPUTPixelShaderDX11  *pPSNoTex  = CPUTPixelShaderDX11::CreatePixelShaderFromMemory(   _L("$DefaultShaderNoTexture"), CPUT_DX11::mpD3dDevice, _L("PSMainNoTexture"), _L("ps_4_0"), gpDefaultShaderSource );
    CPUTVertexShaderDX11 *pVS       = CPUTVertexShaderDX11::CreateVertexShaderFromMemory(          _L("$DefaultShader"), CPUT_DX11::mpD3dDevice,          _L("VSMain"), _L("vs_4_0"), gpDefaultShaderSource );
    CPUTVertexShaderDX11 *pVSNoTex  = CPUTVertexShaderDX11::CreateVertexShaderFromMemory( _L("$DefaultShaderNoTexture"), CPUT_DX11::mpD3dDevice, _L("VSMainNoTexture"), _L("vs_4_0"), gpDefaultShaderSource );

    // We just want to create them, which adds them to the library.  We don't need them any more so release them, leaving refCount at 1 (only library owns a ref)
    SAFE_RELEASE(pPS);
    SAFE_RELEASE(pPSNoTex);
    SAFE_RELEASE(pVS);
    SAFE_RELEASE(pVSNoTex);

    // load shadow casting material+sprite object
    cString ExecutableDirectory;
    CPUTOSServices::GetOSServices()->GetExecutableDirectory(&ExecutableDirectory);
    pAssetLibrary->SetMediaDirectoryName(  ExecutableDirectory+_L(".\\Media\\"));
    mpShadowRenderTarget = new CPUTRenderTargetDepth();
    mpShadowRenderTarget->CreateRenderTarget( cString(_L("$shadow_depth")), SHADOW_WIDTH_HEIGHT, SHADOW_WIDTH_HEIGHT, DXGI_FORMAT_D32_FLOAT );

    int width, height;
    CPUTOSServices::GetOSServices()->GetClientDimensions(&width, &height);

    // Call ResizeWindow() because it creates some resources that our blur material needs (e.g., the back buffer)
    ResizeWindow(width, height);

    CPUTRenderStateBlockDX11 *pBlock = new CPUTRenderStateBlockDX11();
    CPUTRenderStateDX11 *pStates = pBlock->GetState();

    // Override default sampler desc for our default shadowing sampler
    pStates->SamplerDesc[0].Filter         = D3D11_FILTER_COMPARISON_ANISOTROPIC;
    pStates->SamplerDesc[0].MaxAnisotropy  = 16;

    pStates->SamplerDesc[1].Filter         = D3D11_FILTER_COMPARISON_ANISOTROPIC;
    pStates->SamplerDesc[1].AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
    pStates->SamplerDesc[1].AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
    pStates->SamplerDesc[1].ComparisonFunc = D3D11_COMPARISON_GREATER;
    pStates->SamplerDesc[1].MaxAnisotropy  = 16;
    pBlock->CreateNativeResources();
    CPUTAssetLibrary::GetAssetLibrary()->AddRenderStateBlock( _L("$DefaultRenderStates"), pBlock );
    pBlock->Release(); // We're done with it.  The library owns it now.

    //
    // Load models
    //
    g_OpenFilePath = _L("Media\\bikeBlueMerged\\");
    g_OpenFileName = _L("bikeBlueMerged");
    pAssetLibrary->SetMediaDirectoryName( g_OpenFilePath );
    mpBikeSet = pAssetLibrary->GetAssetSet( g_OpenFileName );
    mpBikeSet->GetAssetByIndex(0, (CPUTRenderNode**)&mpBikeModel);

    g_OpenFilePath = _L("Media\\floorWalls\\");
    g_OpenFileName = _L("floorwalls");
    pAssetLibrary->SetMediaDirectoryName( g_OpenFilePath );
    mpLevelSet = pAssetLibrary->GetAssetSet( g_OpenFileName );

    g_OpenFilePath = _L("Media\\skyBox\\");
    g_OpenFileName = _L("skyBox");
    pAssetLibrary->SetMediaDirectoryName( g_OpenFilePath );
    mpSkyboxSet = pAssetLibrary->GetAssetSet( g_OpenFileName );

    //
	// If no cameras were created from the model sets then create a default simple camera
	// and add it to the camera array.
	//
    if( mpLevelSet && mpLevelSet->GetCameraCount() )
    {
        mpCamera = mpLevelSet->GetFirstCamera();
        mpCamera->AddRef(); // TODO: why?  Shouldn't need this.
    }
    else
    {
        mpCamera = new CPUTCamera();
        CPUTAssetLibraryDX11::GetAssetLibrary()->AddCamera( _L("SampleStart Camera"), mpCamera );

        mpCamera->SetPosition( 0.0f, 50.0f, 250.0f );
        mpCamera->LookAt(0.0f, 0.0f, 0.0f);
        // Set the projection matrix for all of the cameras to match our window.
        // TODO: this should really be a viewport matrix.  Otherwise, all cameras will have the same FOV and aspect ratio, etc instead of just viewport dimensions.
        mpCamera->SetAspectRatio(((float)width)/((float)height));
    }
    mpCamera->SetFov(DegToRad(60.0f)); // TODO: Fix converter's FOV bug (Maya generates cameras for which fbx reports garbage for fov)
    mpCamera->SetFarPlaneDistance(100000.0f);
    mpCamera->Update();

    mpCameraController = new CPUTCameraControllerFPS();
    mpCameraController->SetCamera(mpCamera);
    mpCameraController->SetLookSpeed(0.004f);
    mpCameraController->SetMoveSpeed(20.0f);

    //
    // Create shadow camera
    //
    float3 lookAtPoint(0.0f, 0.0f, 0.0f);
    float3 half(1.0f, 1.0f, 1.0f);
    if( mpLevelSet )
    {
        mpLevelSet->GetBoundingBox( &lookAtPoint, &half );
    }
    float length = half.length();

    mpShadowCamera = new CPUTCamera();
    mpShadowCamera->SetFov(DegToRad(45));
    mpShadowCamera->SetAspectRatio(1.0f);
    float fov = mpShadowCamera->GetFov();
    float tanHalfFov = tanf(fov * 0.5f);
    float cameraDistance = length/tanHalfFov;
    float nearDistance = cameraDistance * 0.1f;
    mpShadowCamera->SetNearPlaneDistance(nearDistance);
    mpShadowCamera->SetFarPlaneDistance(2.0f * cameraDistance);
    CPUTAssetLibraryDX11::GetAssetLibrary()->AddCamera( _L("ShadowCamera"), mpShadowCamera );
    float3 shadowCameraPosition = lookAtPoint - gLightDir * cameraDistance;
    mpShadowCamera->SetPosition( shadowCameraPosition );
    mpShadowCamera->LookAt( lookAtPoint.x, lookAtPoint.y, lookAtPoint.z );
    mpShadowCamera->Update();

    //
    // Initialize Sensors
    //
    mpSensorManager = new CSensorManagerEvents();
    mpSensorManager->Initialize(1, SENSOR_TYPE_INCLINOMETER_3D);
    int numInclinometers = mpSensorManager->GetNumSensors(SENSOR_INCLINOMETER_3D);
    mCurrentSensor = mpSensorManager->GetSensor(0, SENSOR_INCLINOMETER_3D);

    if(numInclinometers)
    {
        SENSOR_ID id = mpSensorManager->GetSensor(0,SENSOR_INCLINOMETER_3D);
        CPUTDropdown* dropdown = NULL;
        pGUI->CreateDropdown(mpSensorManager->GetDeviceName(id), ID_SELECT_SENSOR, ID_MAIN_PANEL, &dropdown);

        for(int ii=1;ii<numInclinometers;++ii)
        {
            id = mpSensorManager->GetSensor(ii,SENSOR_INCLINOMETER_3D);
            dropdown->AddSelectionItem(mpSensorManager->GetDeviceName(id));
        }
		UINT uiSelectedItem;
		dropdown->GetSelectedItem(uiSelectedItem);
		mCurrentSensor = mpSensorManager->GetSensor(uiSelectedItem-1,SENSOR_INCLINOMETER_3D);
    }

    //
    // Disable auto-rotation of the screen (to prevent the screen orientation from switching when driving intensely)
    // The fuction SetDisplayAutoRotationPreferences doesn't exist in Windows 7. In order to allow the binary to run
    // in both Windows 7 and Windows 8, we instead load the function using GetProcAddress.
    //
    // If we're building with the Windows 8 SDK, we just directly call the function.
    //
#if (WINVER <= 0x0601)

    SetDisplayAutoRotationPreferences = (pSDARP) GetProcAddress(GetModuleHandle(TEXT("user32.dll")),
                                                                "SetDisplayAutoRotationPreferences");
    if(SetDisplayAutoRotationPreferences)
    {
        SetDisplayAutoRotationPreferences(ORIENTATION_PREFERENCE_LANDSCAPE);
    }
#else // #if (WINVER > 0x0601)
    SetDisplayAutoRotationPreferences(ORIENTATION_PREFERENCE_LANDSCAPE);
#endif


    //
    // Set up the UI
    //
    pGUI->CreateButton(_L("Zero Sensor"), ID_SENSOR_ZERO, ID_MAIN_PANEL);
    pGUI->CreateText(_L("\tX\tY\tZ"), ID_IGNORE_CONTROL_ID, ID_MAIN_PANEL);
    pGUI->CreateText(_L( "Raw:\tN/A\tN/A\tN/A"), ID_IGNORE_CONTROL_ID, ID_MAIN_PANEL, &mpSensorText);
    pGUI->CreateText(_L("Zero:\tN/A\tN/A\tN/A"), ID_IGNORE_CONTROL_ID, ID_MAIN_PANEL, &mpSensorZeroText);

    //
    // Set up level
    //
    memset(&mBike, 0, sizeof(mBike));

    // Create walls
    mWalls[0] = float4( 1.0f, 0.0f,  0.0f, -6300.0f);
    mWalls[1] = float4( 0.0f, 0.0f,  1.0f, -6300.0f);
    mWalls[2] = float4(-1.0f, 0.0f,  0.0f, -6300.0f);
    mWalls[3] = float4( 0.0f, 0.0f, -1.0f, -6300.0f);
}

//-----------------------------------------------------------------------------
void WindowsSensors::Update(double deltaSeconds)
{
    float elapsedTime = (float)deltaSeconds;
    InclinometerData sensorData = {0};

    //
    // Poll the sensors
    //
    if(mpSensorManager->GetStatus(mCurrentSensor) == SENSOR_STATUS_ACTIVE)
    {
        mpSensorManager->GetData(mCurrentSensor, &sensorData);
        TCHAR buffer[256];
        swprintf(buffer, 256, _L("Raw:\t%.2f\t%.2f\t%.2f"), sensorData.X_Tilt, sensorData.Y_Tilt, sensorData.Z_Tilt);
        mpSensorText->SetText(buffer);

        // Modify the raw data by the zero
        sensorData.X_Tilt -= mSensorZero.x;
        sensorData.Y_Tilt -= mSensorZero.y;
        sensorData.Z_Tilt -= mSensorZero.z;
        
        sensorData.X_Tilt = DegToRad(sensorData.X_Tilt);
        sensorData.Y_Tilt = DegToRad(sensorData.Y_Tilt);
        sensorData.Z_Tilt = DegToRad(sensorData.Z_Tilt);
    }
    else
    {
        // If the sensor(s) is disconnected, zero out the data
        sensorData.X_Tilt = 0.0f;
        sensorData.Y_Tilt = 0.0f;
        sensorData.Z_Tilt = 0.0f;
        mSensorZero.x     = 0.0f;
        mSensorZero.y     = 0.0f;
        mSensorZero.z     = 0.0f;
    }

    //
    // Update camera
    //
    float3 bikePosition = mpBikeModel->GetPosition();
    float3 camPosition = *mpBikeModel->GetWorldMatrix() * float4(-600.0f, 250.0f, 0.0f, 1.0f);
    float3 idealPosition = mpBikeModel->GetPosition() + camPosition;
    float3 realPosition = mpCamera->GetPosition();
    float3 deltaPosition = (idealPosition-realPosition) * elapsedTime;
    float3 newPosition = realPosition + deltaPosition;
    if(newPosition.y < 1.0f) 
        newPosition.y = 1.0f;
    mpCamera->SetPosition(newPosition);
    mpCamera->LookAt(bikePosition.x, bikePosition.y, bikePosition.z);
    mpCameraController->Update(elapsedTime);

    //
    // Update bike
    //
    if(mBike.velocity > MIN_VELOCITY)
    {
        mBike.turnDelta = -sensorData.Y_Tilt * elapsedTime;
        mBike.angle -= mBike.turnDelta;
    }
    float4x4 dirMat = float4x4RotationY(-mBike.angle);
    float4 forward = dirMat * float4(1.0f, 0.0f, 0.0f, 0.0f);

    // Acceleration
    mBike.acceleration = -sensorData.X_Tilt * 1000.0f;
    float sign = (mBike.acceleration < 0.0f) ? -1.0f : 1.0f;
    mBike.acceleration = fabsf(mBike.acceleration);
    mBike.acceleration -= 150.0f; // Acceleration dampening
    mBike.acceleration *= sign;

    // Velocity
    mBike.velocity += (mBike.acceleration * elapsedTime);
    if(mBike.velocity < MIN_VELOCITY)
        mBike.velocity = MIN_VELOCITY;
    if(mBike.velocity > MAX_VELOCITY)
        mBike.velocity = MAX_VELOCITY;
        
    float3 newBikePosition = bikePosition + (float3(forward) * mBike.velocity * elapsedTime);

    float tilt = -sensorData.Y_Tilt;
    if(tilt < -0.8f) tilt = -0.8f;
    if(tilt >  0.8f) tilt =  0.8f;

    // Wall collision
    for(int ii = 0; ii < 4; ++ii)
    {
        float start = DistanceToPlane(mWalls[ii], bikePosition);
        float end = DistanceToPlane(mWalls[ii], newBikePosition);
        if((start < 0 && end >= 0) || (start >= 0 && end < 0)) // Signs are different, we crossed the wall
        {
            // "Bounce" off the wall
            float bounce = abs(start)/(abs(start) + abs(end));
            bikePosition = bikePosition + (newBikePosition-bikePosition) * bounce;
            float3 reflect = float3(mWalls[ii].x * 2.0f * end, 
                                    mWalls[ii].y * 2.0f * end,
                                    mWalls[ii].z * 2.0f * end);
            newBikePosition = newBikePosition - reflect;

            // Reset the bikes anagle
            float3 dir = normalize(newBikePosition-bikePosition);
            mBike.angle = -atan2f(dir.z, dir.x);
        }
    }

    float4x4 x = float4x4RotationX(tilt);
    float4x4 y = float4x4RotationY(mBike.angle);
    float4x4 z = float4x4Identity();
    float4x4 translation = float4x4Translation(newBikePosition);
    float4x4 transform = x * y * z * translation;
    mpBikeModel->SetParentMatrix(transform);
}

// Handle keyboard events
//-----------------------------------------------------------------------------
CPUTEventHandledCode WindowsSensors::HandleKeyboardEvent(CPUTKey key)
{
    static bool panelToggle = false;
    CPUTEventHandledCode    handled = CPUT_EVENT_UNHANDLED;
    cString fileName;
    CPUTGuiControllerDX11*  pGUI = CPUTGetGuiController();

    switch(key)
    {
    case KEY_F1:
        panelToggle = !panelToggle;
        if(panelToggle)
        {
            pGUI->SetActivePanel(ID_SECONDARY_PANEL);
        }
        else
        {
            pGUI->SetActivePanel(ID_MAIN_PANEL);
        }
        handled = CPUT_EVENT_HANDLED;
        break;
    case KEY_ESCAPE:
        handled = CPUT_EVENT_HANDLED;
        Shutdown();
        break;
    }

    // pass it to the camera controller
    if(handled == CPUT_EVENT_UNHANDLED)
    {
        handled = mpCameraController->HandleKeyboardEvent(key);
    }
    return handled;
}


// Handle mouse events
//-----------------------------------------------------------------------------
CPUTEventHandledCode WindowsSensors::HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state)
{
    if( mpCameraController )
    {
        return mpCameraController->HandleMouseEvent(x, y, wheel, state);
    }
    return CPUT_EVENT_UNHANDLED;
}

// Handle any control callback events
//-----------------------------------------------------------------------------
void WindowsSensors::HandleCallbackEvent( CPUTEventID Event, CPUTControlID ControlID, CPUTControl *pControl )
{
    UNREFERENCED_PARAMETER(Event);
    cString SelectedItem;
    static bool resize = false;

    switch(ControlID)
    {
    case ID_FULLSCREEN_BUTTON:
        CPUTToggleFullScreenMode();
        break;
    case ID_SELECT_SENSOR:
        {
            CPUTDropdown* pDropDown = (CPUTDropdown*)pControl;
            UINT uiSelectedItem;
            pDropDown->GetSelectedItem(uiSelectedItem);
            mCurrentSensor = mpSensorManager->GetSensor(uiSelectedItem-1,SENSOR_INCLINOMETER_3D);
            break;
        }
    case ID_SENSOR_ZERO:
        {
            InclinometerData sensorData = {0};
            if(mpSensorManager->GetStatus(mCurrentSensor) == SENSOR_STATUS_ACTIVE)
            {
                mpSensorManager->GetData(mCurrentSensor, &sensorData);
                TCHAR buffer[256];
                swprintf(buffer, 256, _L("Zero:\t%.2f\t%.2f\t%.2f"), sensorData.X_Tilt, sensorData.Y_Tilt, sensorData.Z_Tilt);
                mpSensorZeroText->SetText(buffer);

                mSensorZero.x = sensorData.X_Tilt;
                mSensorZero.y = sensorData.Y_Tilt;
                mSensorZero.z = sensorData.Z_Tilt;
            }
        }
        break;
    default:
        break;
    }
}

// Handle resize events
//-----------------------------------------------------------------------------
void WindowsSensors::ResizeWindow(UINT width, UINT height)
{
    CPUTAssetLibrary *pAssetLibrary = CPUTAssetLibrary::GetAssetLibrary();

    // Before we can resize the swap chain, we must release any references to it.
    // We could have a "AssetLibrary::ReleaseSwapChainResources(), or similar.  But,
    // Generic "release all" works, is simpler to implement/maintain, and is not performance critical.
    pAssetLibrary->ReleaseTexturesAndBuffers();

    // Resize the CPUT-provided render target
    CPUT_DX11::ResizeWindow( width, height );

    // Resize any application-specific render targets here
    if( mpCamera ) mpCamera->SetAspectRatio(((float)width)/((float)height));

    pAssetLibrary->RebindTexturesAndBuffers();
}

//-----------------------------------------------------------------------------
void WindowsSensors::Render(double deltaSeconds)
{
    CPUTRenderParametersDX renderParams(mpContext);

    //*******************************
    // Draw the shadow scene
    //*******************************
    CPUTCamera *pLastCamera = mpCamera;
    mpCamera = renderParams.mpCamera = mpShadowCamera;
    mpShadowRenderTarget->SetRenderTarget( renderParams, 0, 0.0f, true );

    mpLevelSet->RenderShadowRecursive(renderParams);
    mpBikeSet->RenderShadowRecursive(renderParams);

    mpShadowRenderTarget->RestoreRenderTarget(renderParams);
    mpCamera = renderParams.mpCamera = pLastCamera;

    // Save the light direction to a global so we can set it to a constant buffer later (TODO: have light do this)
    gLightDir = mpShadowCamera->GetLook();

    // Clear back buffer
    const float clearColor[] = { 0.0993f, 0.0993f, 0.0993f, 1.0f };
    mpContext->ClearRenderTargetView( mpBackBufferRTV,  clearColor );
    mpContext->ClearDepthStencilView( mpDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

    renderParams.mRenderOnlyVisibleModels = false;
    mpLevelSet->RenderRecursive(renderParams);
    mpBikeSet->RenderRecursive(renderParams);
    mpSkyboxSet->RenderRecursive(renderParams);

    CPUTDrawGUI();
}


char *gpDefaultShaderSource =  "\n\
// ********************************************************************************************************\n\
struct VS_INPUT\n\
{\n\
    float3 Pos      : POSITION; // Projected position\n\
    float3 Norm     : NORMAL;\n\
    float2 Uv       : TEXCOORD0;\n\
};\n\
struct PS_INPUT\n\
{\n\
    float4 Pos      : SV_POSITION;\n\
    float3 Norm     : NORMAL;\n\
    float2 Uv       : TEXCOORD0;\n\
    float4 LightUv  : TEXCOORD1;\n\
    float3 Position : TEXCOORD2; // Object space position \n\
};\n\
// ********************************************************************************************************\n\
    Texture2D    TEXTURE0 : register( t0 );\n\
    SamplerState SAMPLER0 : register( s0 );\n\
    Texture2D    _Shadow  : register( t1 );\n\
    SamplerComparisonState SAMPLER1 : register( s1 );\n\
// ********************************************************************************************************\n\
cbuffer cbPerModelValues\n\
{\n\
    row_major float4x4 World : WORLD;\n\
    row_major float4x4 WorldViewProjection : WORLDVIEWPROJECTION;\n\
    row_major float4x4 InverseWorld : INVERSEWORLD;\n\
              float4   LightDirection;\n\
              float4   EyePosition;\n\
    row_major float4x4 LightWorldViewProjection;\n\
};\n\
// ********************************************************************************************************\n\
// TODO: Note: nothing sets these values yet\n\
cbuffer cbPerFrameValues\n\
{\n\
    row_major float4x4  View;\n\
    row_major float4x4  Projection;\n\
};\n\
// ********************************************************************************************************\n\
PS_INPUT VSMain( VS_INPUT input )\n\
{\n\
    PS_INPUT output = (PS_INPUT)0;\n\
    output.Pos      = mul( float4( input.Pos, 1.0f), WorldViewProjection );\n\
    output.Position = mul( float4( input.Pos, 1.0f), World ).xyz;\n\
    // TODO: transform the light into object space instead of the normal into world space\n\
    output.Norm = mul( input.Norm, (float3x3)World );\n\
    output.Uv   = float2(input.Uv.x, input.Uv.y);\n\
    output.LightUv   = mul( float4( input.Pos, 1.0f), LightWorldViewProjection );\n\
    return output;\n\
}\n\
// ********************************************************************************************************\n\
float4 PSMain( PS_INPUT input ) : SV_Target\n\
{\n\
    float3  lightUv = input.LightUv.xyz / input.LightUv.w;\n\
    lightUv.xy = lightUv.xy * 0.5f + 0.5f; // TODO: Move scale and offset to matrix.\n\
    lightUv.y  = 1.0f - lightUv.y;\n\
    float   shadowAmount = _Shadow.SampleCmp( SAMPLER1, lightUv, lightUv.z );\n\
    float3 normal         = normalize(input.Norm);\n\
    float  nDotL          = saturate( dot( normal, -LightDirection ) );\n\
    float3 eyeDirection   = normalize(input.Position - EyePosition);\n\
    float3 reflection     = reflect( eyeDirection, normal );\n\
    float  rDotL          = saturate(dot( reflection, -LightDirection ));\n\
    float3 specular       = pow(rDotL, 16.0f);\n\
    specular              = min( shadowAmount, specular );\n\
    float4 diffuseTexture = TEXTURE0.Sample( SAMPLER0, input.Uv );\n\
    float ambient = 0.05;\n\
    float3 result = (min(shadowAmount, nDotL)+ambient) * diffuseTexture + specular;\n\
    return float4( result, 1.0f );\n\
}\n\
\n\
// ********************************************************************************************************\n\
struct VS_INPUT_NO_TEX\n\
{\n\
    float3 Pos      : POSITION; // Projected position\n\
    float3 Norm     : NORMAL;\n\
};\n\
struct PS_INPUT_NO_TEX\n\
{\n\
    float4 Pos      : SV_POSITION;\n\
    float3 Norm     : NORMAL;\n\
    float4 LightUv  : TEXCOORD1;\n\
    float3 Position : TEXCOORD0; // Object space position \n\
};\n\
// ********************************************************************************************************\n\
PS_INPUT_NO_TEX VSMainNoTexture( VS_INPUT_NO_TEX input )\n\
{\n\
    PS_INPUT_NO_TEX output = (PS_INPUT_NO_TEX)0;\n\
    output.Pos      = mul( float4( input.Pos, 1.0f), WorldViewProjection );\n\
    output.Position = mul( float4( input.Pos, 1.0f), World ).xyz;\n\
    // TODO: transform the light into object space instead of the normal into world space\n\
    output.Norm = mul( input.Norm, (float3x3)World );\n\
    output.LightUv   = mul( float4( input.Pos, 1.0f), LightWorldViewProjection );\n\
    return output;\n\
}\n\
// ********************************************************************************************************\n\
float4 PSMainNoTexture( PS_INPUT_NO_TEX input ) : SV_Target\n\
{\n\
    float3 lightUv = input.LightUv.xyz / input.LightUv.w;\n\
    float2 uv = lightUv.xy * 0.5f + 0.5f;\n\
    float2 uvInvertY = float2(uv.x, 1.0f-uv.y);\n\
    float shadowAmount = _Shadow.SampleCmp( SAMPLER1, uvInvertY, lightUv.z );\n\
    float3 eyeDirection = normalize(input.Position - EyePosition.xyz);\n\
    float3 normal       = normalize(input.Norm);\n\
    float  nDotL = saturate( dot( normal, -normalize(LightDirection.xyz) ) );\n\
    nDotL = shadowAmount * nDotL;\n\
    float3 reflection   = reflect( eyeDirection, normal );\n\
    float  rDotL        = saturate(dot( reflection, -LightDirection.xyz ));\n\
    float  specular     = 0.2f * pow( rDotL, 4.0f );\n\
    specular = min( shadowAmount, specular );\n\
    return float4( (nDotL + specular).xxx, 1.0f);\n\
}\n\
";


