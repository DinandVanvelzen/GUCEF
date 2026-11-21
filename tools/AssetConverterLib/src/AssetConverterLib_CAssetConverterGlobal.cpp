/*
 *  AssetConverterLib: the global systems of the converter library
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_MATH_CMATHGLOBAL_H
#include "gucefMATH_CMathGlobal.h"
#define GUCEF_MATH_CMATHGLOBAL_H
#endif /* GUCEF_MATH_CMATHGLOBAL_H ? */

#ifndef GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H
#include "gucefGEOMETRY_CGeometryGlobal.h"
#define GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H
#endif /* GUCEF_GEOMETRY_CGEOMETRYGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H
#include "AssetConverterLib_CAssetConversionTaskConsumer.h"
#define ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERSIONTASKCONSUMER_H ? */

#include "AssetConverterLib_CAssetConverterGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static CAssetConverterGlobal* g_instance = GUCEF_NULL;
static bool g_unloaded = false;     /**< set when the library unloaded: the global is not created again until it loads */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetConverterGlobal*
CAssetConverterGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        if ( g_unloaded )
            return GUCEF_NULL;

        // the modules this one depends on first
        CORE::CCoreGlobal::Instance();
        VFS::CVfsGlobal::Instance();
        IMAGE::CImageGlobal::Instance();
        MATH::CMathGlobal::Instance();
        GEOMETRY::CGeometryGlobal::Instance();

        g_instance = GUCEF_NEW CAssetConverterGlobal();
        g_instance->Initialize();
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

CAssetConverterGlobal*
CAssetConverterGlobal::InstanceIfCreated( void )
{GUCEF_TRACE;

    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CAssetConverterGlobal::AllowCreation( void )
{GUCEF_TRACE;

    g_unloaded = false;
}

/*-------------------------------------------------------------------------*/

void
CAssetConverterGlobal::Deinstance( void )
{GUCEF_TRACE;

    g_unloaded = true;
    CAssetConverterGlobal* global = g_instance;
    g_instance = GUCEF_NULL;
    GUCEF_DELETE global;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetConverterGlobal::GetConverterVersion( void )
{GUCEF_TRACE;

    // 2: the sidecar holds the normalized relations of the asset ("relations")
    return 2;
}

/*-------------------------------------------------------------------------*/

CAssetConverterGlobal::CAssetConverterGlobal( void )
    : m_pluginRegistry( GUCEF_NULL )
    , m_taskFactory( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetConverterGlobal::Initialize( void )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Initializing AssetConverterLib global systems" );

    m_pluginRegistry = GUCEF_NEW CAssetPluginRegistry();
    m_pluginRegistry->RegisterBuiltIns();

    m_taskFactory = GUCEF_NEW TAssetConversionTaskConsumerFactory();
    CORE::CCoreGlobal::Instance()->GetTaskManager().RegisterTaskConsumerFactory( CAssetConversionTaskConsumer::TaskType, m_taskFactory );

    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "AssetConverterLib" );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetConverterLib global systems initialized" );
}

/*-------------------------------------------------------------------------*/

CAssetConverterGlobal::~CAssetConverterGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Shutting down AssetConverterLib global systems" );
    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "AssetConverterLib" );

    // reverse order of creation
    CORE::CCoreGlobal::Instance()->GetTaskManager().UnregisterTaskConsumerFactory( CAssetConversionTaskConsumer::TaskType );
    GUCEF_DELETE m_taskFactory;
    m_taskFactory = GUCEF_NULL;

    GUCEF_DELETE m_pluginRegistry;
    m_pluginRegistry = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAssetPluginRegistry&
CAssetConverterGlobal::GetPluginRegistry( void )
{GUCEF_TRACE;

    return *m_pluginRegistry;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
