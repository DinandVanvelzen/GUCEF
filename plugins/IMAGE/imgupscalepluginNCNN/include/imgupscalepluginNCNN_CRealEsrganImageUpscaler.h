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

#ifndef GUCEF_IMGUPSCALEPLUGINNCNN_CREALESRGANIMAGEUPSCALER_H
#define GUCEF_IMGUPSCALEPLUGINNCNN_CREALESRGANIMAGEUPSCALER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_IMAGE_CIMAGEUPSCALER_H
#include "gucefIMAGE_CImageUpscaler.h"
#define GUCEF_IMAGE_CIMAGEUPSCALER_H
#endif /* GUCEF_IMAGE_CIMAGEUPSCALER_H ? */

#ifndef GUCEF_IMGUPSCALEPLUGINNCNN_MACROS_H
#include "imgupscalepluginNCNN_macros.h"
#define GUCEF_IMGUPSCALEPLUGINNCNN_MACROS_H
#endif /* GUCEF_IMGUPSCALEPLUGINNCNN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ncnn { class Net; }

namespace GUCEF {
namespace IMGUPSCALEPLUGINNCNN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Image upscaler "realesrgan": the Real-ESRGAN network for anime and drawn art (realesr-animevideov3, Xintao Wang,
 *  BSD-3-Clause) on ncnn, 2, 3 or 4 times. It runs with Vulkan on a GPU when the system has one, on the CPU
 *  otherwise. The picture is worked on in tiles with a margin, so a large picture needs little memory and the
 *  progress is told per tile; alpha is scaled bilinearly.
 *
 *  The models are data files of the plugin, read through the VFS from "imgupscalepluginNCNN/models".
 */
class IMGUPSCALEPLUGINNCNN_PUBLIC_CPP CRealEsrganImageUpscaler : public IMAGE::CImageUpscaler
{
    public:

    static const char* const ModelDir;

    CRealEsrganImageUpscaler( void );

    virtual ~CRealEsrganImageUpscaler();

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TFactorVector GetFactors( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Upscale( const IMAGE::CPixelMap& source, UInt32 factor, IMAGE::TPixelMapPtr& result, IMAGE::CIUpscaleProgress* progress ) GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the networks run on a GPU (Vulkan); the first question starts the GPU instance */
    bool IsUsingGpu( void ) const;

    /** The size of the tiles the picture is worked on in, in source pixels; 0 for the whole picture at once */
    void SetTileSize( UInt32 tileSize );

    /** Lets go of the networks, before the GPU instance goes */
    void ReleaseNetworks( void );

    /** Ends the GPU instance when this upscaler started it; the networks go first */
    void ReleaseGpu( void );

    private:

    struct SModel
    {
        ncnn::Net* net;
        CORE::CDynamicBuffer param;     /**< the text of the network, 0 terminated */
        CORE::CDynamicBuffer weights;   /**< the network refers to the weights, they live as long as it does */
    };
    typedef struct SModel TModel;

    TModel* GetModel( UInt32 factor );

    /** The GPU instance is made when a network is needed first, not when the plugin is loaded: making it is slow and tells the
     *  devices it finds on the console. Without a usable GPU the networks run on the CPU */
    void StartGpu( void ) const;

    static void ReleaseNetworksAtExit( void );

    CRealEsrganImageUpscaler( const CRealEsrganImageUpscaler& src );
    CRealEsrganImageUpscaler& operator=( const CRealEsrganImageUpscaler& src );

    private:

    TModel* m_models[ 5 ];      /**< by factor */
    UInt32 m_tileSize;
    mutable bool m_gpuStarted;
    mutable bool m_gpuInstance;         /**< this upscaler made the GPU instance and ends it */
    mutable bool m_useGpu;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMGUPSCALEPLUGINNCNN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMGUPSCALEPLUGINNCNN_CREALESRGANIMAGEUPSCALER_H ? */
