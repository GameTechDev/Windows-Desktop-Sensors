////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
////////////////////////////////////////////////////////////////////////////////
//============================================================================
// Name        : CPUT-linux.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

#include <stdio.h>
#include <stdlib.h>


using namespace std;

#include <stdio.h>

// windows specific headers
//#include <windows.h>
//#define DIRECTX

#ifdef DIRECTX
#include "CPUT_DX11.h"
#elif OPENGL31_WIN
#include "CPUT_OGL31.h"
#else
#include "CPUT-OGL31X.h"
#endif

/*
//-----------------------------------------------------------------------------
void OnKeyboardEvent(eKey key)
{
    if(KEY_Q == key)
    {
        printf("Q pressed");
        CPUTShutdown();
    }
    else
        printf("Key pressed");
}
*/
#ifdef DIRECTX
// DirectX 11 render callback
//-----------------------------------------------------------------------------
void Render(ID3D11DeviceContext* pImmediateContext, IDXGISwapChain* pSwapChain, ID3D11RenderTargetView* pRenderTargetView)
{
    /*
    DXOBJ* model = CPUTLoadModel();


    meshStruct mesh = CPUTLoadMesh();

    meshStruct
    {
        Vec3 vert;
        Vec3 Normal;
        Vec3
    }
    */

    // blue for DirectX
    float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; //red,green,blue,alpha
    pImmediateContext->ClearRenderTargetView( pRenderTargetView, ClearColor );

//    DXUTDrawGui();

    pSwapChain->Present( 0, 0 );
}

#elif OPENGL31_WIN
// OpenGL render callback
//-----------------------------------------------------------------------------
void Render(HDC hdc)
{
    glClearColor (0.5, 0.0, 0.5, 1.0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    SwapBuffers(hdc);

}
#else

#endif


//-----------------------------------------------------------------------------
int  main( )
{
    CPUTInitialize();
/*
    // Set callbacks
    CPUTSetKeyboardCallBack(OnKeyboardEvent);
    CPUTSetRenderCallback(Render);


    // Create gui
//    CPUTCreateGroup(IDC_NAMEGROUP);
//    CPUTAddButton(x,y,"push me", IDC_PUSHME, IDC_NAMEGROUP);
*/
    // create the context
    CPUTCreateContext();


    // start the main loop
    CPUTStart();

    return CPUTReturnCode();
}

