/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#define GUCEF_IMAGE_CIMAGEGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_IMAGE_MACROS_H
#include "gucefIMAGE_macros.h"
#define GUCEF_IMAGE_MACROS_H
#endif /* GUCEF_IMAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CImageCodecPluginManager;
class CImageCodecRegistry;
class CImageUpscalerRegistry;
class CMmpxImageUpscaler;

/*-------------------------------------------------------------------------*/

/**
 *  Singular singleton providing access to all global Image systems
 */
class GUCEF_IMAGE_EXPORT_CPP CImageGlobal
{
    public:

    static CImageGlobal* Instance( void );

    CImageCodecPluginManager& GetImageCodecPluginManager( void );

    CImageCodecRegistry& GetImageCodecRegistry( void );

    /** The image upscalers: "mmpx" built in, others from plugins */
    CImageUpscalerRegistry& GetImageUpscalerRegistry( void );

    private:
    friend class CModule;

    static void Deinstance( void );

    private:

    CImageGlobal( void );

    ~CImageGlobal();

    void Initialize( void );

    private:

    CImageCodecPluginManager* m_imageCodecPluginManager;
    CImageCodecRegistry* m_imageCodecRegistry;
    CImageUpscalerRegistry* m_imageUpscalerRegistry;
    CMmpxImageUpscaler* m_mmpxUpscaler;
    CORE::CCodecRegistry::TICodecPtr m_pngCodec;    /**< the built-in "png" codec, registered in the codec registry of gucefCORE */
    
    static CImageGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 16-02-2007 :
        - Dinand: Added this class

---------------------------------------------------------------------------*/
