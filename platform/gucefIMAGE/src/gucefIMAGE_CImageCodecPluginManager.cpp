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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECPLUGIN_H
#include "gucefIMAGE_CImageCodecPlugin.h"
#define GUCEF_IMAGE_CIMAGECODECPLUGIN_H
#endif /* GUCEF_IMAGE_CIMAGECODECPLUGIN_H ? */

#ifndef GUCEF_IMAGE_CPLUGINIMAGECODECITEM_H
#include "gucefIMAGE_CPluginImageCodecItem.h"
#define GUCEF_IMAGE_CPLUGINIMAGECODECITEM_H
#endif /* GUCEF_IMAGE_CPLUGINIMAGECODECITEM_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#include "gucefIMAGE_CImageCodecRegistry.h"
#define GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGECODECREGISTRY_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#include "gucefIMAGE_CImageCodecPluginManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageCodecPluginManager::CImageCodecPluginManager( void )
    : CPluginManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageCodecPluginManager::~CImageCodecPluginManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::TPluginPtr
CImageCodecPluginManager::RegisterPlugin( void* modulePtr                         ,
                                          CORE::TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    TImageCodecPluginPtr plugin( new CImageCodecPlugin() );
    if ( plugin->Link( modulePtr      ,
                       pluginMetaData ) )
    {
        CORE::CValueList pluginParams;
        pluginMetaData->GetParams( pluginParams );

        bool overrideExistingCodecs = false;
        overrideExistingCodecs = pluginParams.GetValueAlways( "overrideExistingCodecs", false ).AsBool( overrideExistingCodecs );

        CImageCodecRegistry& registry = CImageGlobal::Instance()->GetImageCodecRegistry();
        CORE::CStdCodecPlugin::CCodecList codecList;
        plugin->GetCodecList( codecList );

        CORE::CStdCodecPlugin::CCodecList::iterator i = codecList.find( CImageCodecPlugin::ImageCodecFamilyName );
        CORE::CStdCodecPlugin::CCodecFamilyList imageCodecList = (*i).second;

        CORE::CStdCodecPlugin::CCodecFamilyList::iterator n = imageCodecList.begin();
        while ( n != imageCodecList.end() )
        {
            bool isAlreadyRegistered = registry.IsRegistered( *n );
            if ( overrideExistingCodecs || !isAlreadyRegistered )
            {
                if ( isAlreadyRegistered && overrideExistingCodecs )
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ImageCodec " + *n + " is already registered but overrideExistingCodecs=True, will unregister current codec" );
                    registry.Unregister( *n );
                }
                
                TPluginImageCodecItemPtr codecItem( new CPluginImageCodecItem( plugin, (*n) ) );
                if ( !registry.TryRegister( codecItem->GetType(), codecItem ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to register image codec " + codecItem->GetType() );
                }
            }
            ++n;
        }

        return plugin;
    }
    return CORE::TPluginPtr();
}

/*-------------------------------------------------------------------------*/

bool
CImageCodecPluginManager::UnregisterPlugin( CORE::TPluginPtr plugin )
{GUCEF_TRACE;

    TImageCodecPluginPtr imageCodecPlugin = plugin.StaticCast< CImageCodecPlugin >();

    CImageCodecRegistry& registry = CImageGlobal::Instance()->GetImageCodecRegistry();
    CORE::CStdCodecPlugin::CCodecList codecList;
    imageCodecPlugin->GetCodecList( codecList );

    CORE::CStdCodecPlugin::CCodecList::iterator i = codecList.find( CImageCodecPlugin::ImageCodecFamilyName );
    if ( codecList.end() != i )
    {
        CORE::CStdCodecPlugin::CCodecFamilyList imageCodecList = (*i).second;

        CORE::CStdCodecPlugin::CCodecFamilyList::iterator n = imageCodecList.begin();
        while ( n != imageCodecList.end() )
        {
            // Codecs of this plugin may have been unregistered already, for example because
            // another plugin overrode them or because this plugin never registered them
            if ( registry.IsRegistered( (*n) ) )
                registry.Unregister( (*n) );
            ++n;
        }        
    }

    // The plugin is shut down and its codecs taken out of the codec registry before its module is unloaded,
    // later the module's code is gone
    return imageCodecPlugin.IsNULL() || imageCodecPlugin->Unlink();
}

/*-------------------------------------------------------------------------*/

CString
CImageCodecPluginManager::GetPluginType( void ) const
{GUCEF_TRACE;

    return "GucefImageCodecPlugin";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace IMAGE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
