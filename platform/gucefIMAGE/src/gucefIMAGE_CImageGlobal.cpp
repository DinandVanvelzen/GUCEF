/*
 *  gucefIMAGE: GUCEF module providing image utilities
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_IMAGE_CGUIIMAGECODEC_H
#include "gucefIMAGE_CGUIImageCodec.h"
#define GUCEF_IMAGE_CGUIIMAGECODEC_H
#endif /* GUCEF_IMAGE_CGUIIMAGECODEC_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#include "gucefIMAGE_CImageCodecRegistry.h"
#define GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGECODECREGISTRY_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECPLUGINMANAGER_H
#include "gucefIMAGE_CImageCodecPluginManager.h"
#define GUCEF_IMAGE_CIMAGECODECPLUGINMANAGER_H
#endif /* GUCEF_IMAGE_CIMAGECODECPLUGINMANAGER_H ? */

#include "gucefIMAGE_CImageGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageGlobal* CImageGlobal::g_instance = NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageGlobal*
CImageGlobal::Instance()
{GUCEF_TRACE;

    if ( NULL == g_instance )
    {
        g_instance = new CImageGlobal();
        g_instance->Initialize();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CImageGlobal::Deinstance( void )
{GUCEF_TRACE;

    delete g_instance;
    g_instance = NULL;
}

/*-------------------------------------------------------------------------*/

void
CImageGlobal::Initialize( void )
{GUCEF_TRACE;

    // Ensure the Core systems are initialzed first since this module depends on Core
    CORE::CCoreGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Initializing gucefIMAGE Global systems" );

    CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CCodecRegistry::TCodecFamilyRegistryPtr registry; 
        
    if ( !codecRegistry.IsRegistered( "ImageCodec" ) )
    {
        registry = CORE::CCodecRegistry::TCodecFamilyRegistryTypedPtr( new CORE::CCodecRegistry::TCodecFamilyRegistry() );
        codecRegistry.Register( "ImageCodec", registry );
    }
    else
    {
        registry = codecRegistry.Lookup( "ImageCodec" );
    }

    // Register the dummy codec for our native format
    registry->Register( "gui", CORE::CCodecRegistry::TICodecTypedPtr( new CGUIImageCodec() ) );

    m_imageCodecRegistry = new CImageCodecRegistry();
    m_imageCodecPluginManager = new CImageCodecPluginManager();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefIMAGE Global systems initialized" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefIMAGE" );
}

/*-------------------------------------------------------------------------*/

CImageGlobal::CImageGlobal( void )
    : m_imageCodecPluginManager( NULL ) ,
      m_imageCodecRegistry( NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageGlobal::~CImageGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Shutting down gucefIMAGE global systems" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefIMAGE" );

    CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CCodecRegistry::TCodecFamilyRegistryPtr registry;
    
    if ( codecRegistry.IsRegistered( "ImageCodec" ) )
    {
        registry = codecRegistry.Lookup( "ImageCodec" );
        
        // unregister the dummy codec for our native format
        registry->Unregister( "gui" );
    }

    delete m_imageCodecPluginManager;
    m_imageCodecPluginManager = NULL;
    delete m_imageCodecRegistry;
    m_imageCodecRegistry = NULL;
}

/*-------------------------------------------------------------------------*/

CImageCodecPluginManager&
CImageGlobal::GetImageCodecPluginManager( void )
{GUCEF_TRACE;

    return *m_imageCodecPluginManager;
}

/*-------------------------------------------------------------------------*/

CImageCodecRegistry&
CImageGlobal::GetImageCodecRegistry( void )
{GUCEF_TRACE;

    return *m_imageCodecRegistry;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
