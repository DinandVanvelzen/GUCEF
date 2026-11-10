/*
 *  imgupscalepluginNCNN: GUCEF image plugin that upscales pictures with Real-ESRGAN neural networks running on ncnn
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CIMAGEUPSCALERREGISTRY_H
#include "gucefIMAGE_CImageUpscalerRegistry.h"
#define GUCEF_IMAGE_CIMAGEUPSCALERREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGEUPSCALERREGISTRY_H ? */

#ifndef GUCEF_IMGUPSCALEPLUGINNCNN_CREALESRGANIMAGEUPSCALER_H
#include "imgupscalepluginNCNN_CRealEsrganImageUpscaler.h"
#define GUCEF_IMGUPSCALEPLUGINNCNN_CREALESRGANIMAGEUPSCALER_H
#endif /* GUCEF_IMGUPSCALEPLUGINNCNN_CREALESRGANIMAGEUPSCALER_H ? */

#include "imgupscalepluginNCNN_pluginAPI.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMGUPSCALEPLUGINNCNN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static CRealEsrganImageUpscaler* g_upscaler = GUCEF_NULL;

/*-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGUPSCALEPLUGINNCNN_ENTRY( GUCEFPlugin_Load )( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    // the GPU (Vulkan, or the CPU without one) is started by the upscaler when it is first used
    g_upscaler = GUCEF_NEW CRealEsrganImageUpscaler();
    if ( !IMAGE::CImageGlobal::Instance()->GetImageUpscalerRegistry().Register( g_upscaler ) )
    {
        GUCEF_DELETE g_upscaler;
        g_upscaler = GUCEF_NULL;
        return 0;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGUPSCALEPLUGINNCNN_ENTRY( GUCEFPlugin_Unload )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    if ( GUCEF_NULL != g_upscaler )
    {
        IMAGE::CImageGlobal::Instance()->GetImageUpscalerRegistry().Unregister( g_upscaler->GetName() );
        g_upscaler->ReleaseGpu();
        GUCEF_DELETE g_upscaler;
        g_upscaler = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGUPSCALEPLUGINNCNN_ENTRY( GUCEFPlugin_GetVersion )( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 1;
    versionInfo->minor = 0;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGUPSCALEPLUGINNCNN_ENTRY( GUCEFPlugin_GetCopyright )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0; ncnn (Tencent) and the Real-ESRGAN models (Xintao Wang) BSD-3-Clause";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
IMGUPSCALEPLUGINNCNN_ENTRY( GUCEFPlugin_GetDescription )( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Image upscaler \"realesrgan\": Real-ESRGAN (realesr-animevideov3) on ncnn, with Vulkan or on the CPU";
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUPSCALEPLUGINNCNN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
