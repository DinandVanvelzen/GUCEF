/*
 *  assetpluginUNITY: GUCEF asset converter plugin: Unity project and package import
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYIMPORTER_H
#include "assetpluginUNITY_CUnityImporter.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYIMPORTER_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYIMPORTER_H ? */

#include "assetpluginUNITY.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetConverterGlobal* global = ASSETCONVERTER::CAssetConverterGlobal::Instance();
    if ( GUCEF_NULL == global )
        return 0;

    if ( !global->GetPluginRegistry().RegisterImporter( ASSETCONVERTER::TAssetImporterPtr( GUCEF_NEW CUnityImporter() ) ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "assetpluginUNITY: The importer \"unity\" is registered already" );
        return 0;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "assetpluginUNITY: Loaded" );
    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetConverterGlobal* global = ASSETCONVERTER::CAssetConverterGlobal::InstanceIfCreated();
    if ( GUCEF_NULL != global )
        global->GetPluginRegistry().UnregisterImporter( "unity" );
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    versionInfo->major = 1;
    versionInfo->minor = 0;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "GUCEF asset converter plugin: imports Unity projects and packages: prefabs, scenes, materials, textures, colliders, LOD groups and animation clips; model files through the importers of their format";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefCORE,gucefVFS,AssetConverterLib";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
