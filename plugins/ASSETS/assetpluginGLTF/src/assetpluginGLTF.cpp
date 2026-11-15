/*
 *  assetpluginGLTF: GUCEF asset converter plugin: glTF 2.0 import, export and validation
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

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFIMPORTER_H
#include "assetpluginGLTF_CGltfImporter.h"
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFIMPORTER_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFIMPORTER_H ? */

#ifndef GUCEF_ASSETPLUGIN_GLTF_CGLTFEXPORTER_H
#include "assetpluginGLTF_CGltfExporter.h"
#define GUCEF_ASSETPLUGIN_GLTF_CGLTFEXPORTER_H
#endif /* GUCEF_ASSETPLUGIN_GLTF_CGLTFEXPORTER_H ? */

#include "assetpluginGLTF.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

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

    ASSETCONVERTER::CAssetPluginRegistry& registry = global->GetPluginRegistry();
    const bool importer = registry.RegisterImporter( ASSETCONVERTER::TAssetImporterPtr( GUCEF_NEW CGltfImporter() ) );
    const bool exporter = registry.RegisterExporter( ASSETCONVERTER::TAssetExporterPtr( GUCEF_NEW CGltfExporter() ) );
    if ( !importer || !exporter )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "assetpluginGLTF: The importer or the exporter \"gltf\" is registered already" );
        return 0;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "assetpluginGLTF: Loaded, using cgltf 1.15" );
    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetConverterGlobal* global = ASSETCONVERTER::CAssetConverterGlobal::InstanceIfCreated();
    if ( GUCEF_NULL != global )
    {
        global->GetPluginRegistry().UnregisterImporter( "gltf" );
        global->GetPluginRegistry().UnregisterExporter( "gltf" );
    }
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

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0. Uses cgltf, MIT license";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "GUCEF asset converter plugin: imports glTF 2.0 files, writes the intermediate as glTF with a binary buffer and validates what it writes";
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

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
