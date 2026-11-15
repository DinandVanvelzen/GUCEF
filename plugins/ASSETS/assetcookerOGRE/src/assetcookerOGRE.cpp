/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#include "OgrePrerequisites.h"

#ifndef GUCEF_ASSETCOOKER_OGRE_COGRECOOKER_H
#include "assetcookerOGRE_COgreCooker.h"
#define GUCEF_ASSETCOOKER_OGRE_COGRECOOKER_H
#endif /* GUCEF_ASSETCOOKER_OGRE_COGRECOOKER_H ? */

#include "assetcookerOGRE.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

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
    if ( !registry.RegisterCooker( ASSETCONVERTER::TAssetCookerPtr( GUCEF_NEW COgreCooker() ) ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "assetcookerOGRE: The cooker \"" << COgreCooker::TargetName << "\" is registered already" );
        return 0;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "assetcookerOGRE: Loaded, cooking for Ogre " << OGRE_VERSION_MAJOR << "." << OGRE_VERSION_MINOR << "." << OGRE_VERSION_PATCH );
    return 1;
}

/*--------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    ASSETCONVERTER::CAssetConverterGlobal* global = ASSETCONVERTER::CAssetConverterGlobal::InstanceIfCreated();
    if ( GUCEF_NULL != global )
    {
        // Ogre is taken down while the plugin's code is still there, whoever still holds the cooker
        ASSETCONVERTER::TAssetCookerPtr cooker;
        if ( global->GetPluginRegistry().TryGetCooker( COgreCooker::TargetName, cooker ) && !cooker.IsNULL() )
        {
            COgreCooker* ogreCooker = dynamic_cast< COgreCooker* >( cooker.GetPointerAlways() );
            if ( GUCEF_NULL != ogreCooker )
                ogreCooker->Release();
        }
        global->GetPluginRegistry().UnregisterCooker( COgreCooker::TargetName );
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

    return "Copyright (C) Dinand Vanvelzen, Apache License 2.0. Uses Ogre, MIT license";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "GUCEF asset converter plugin: cooks the intermediate into the meshes, skeletons, material scripts, DotScene files, colliders and textures of Ogre 1.x";
}

/*--------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetLinkBackModuleDependencies( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{GUCEF_TRACE;

    return "gucefCORE,gucefVFS,AssetConverterLib,pluginglueOGRE";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
