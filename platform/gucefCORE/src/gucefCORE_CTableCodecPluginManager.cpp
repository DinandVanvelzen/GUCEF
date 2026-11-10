/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "gucefCORE_CTableCodecPluginManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTableCodecPluginManager::CTableCodecPluginManager( void )
    : CPluginManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTableCodecPluginManager::~CTableCodecPluginManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CTableCodecPluginManager::GetPluginType( void ) const
{GUCEF_TRACE;

    return "GucefTableCodecPlugin";
}

/*-------------------------------------------------------------------------*/

TPluginPtr
CTableCodecPluginManager::RegisterPlugin( void* modulePtr                   ,
                                          TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    TTableCodecPluginPtr plugin( GUCEF_NEW CTableCodecPlugin() );
    if ( !plugin->Link( modulePtr, pluginMetaData ) )
        return TPluginPtr();

    if ( 0 == plugin->RegisterCodecs( CCoreGlobal::Instance()->GetTableCodecRegistry() ) )
    {
        GUCEF_ERROR_LOG( LOGLEVEL_NORMAL, "TableCodecPluginManager: Module " << pluginMetaData->GetModuleFilename() << " has no codec that could be registered" );
        plugin->UnregisterCodecs( CCoreGlobal::Instance()->GetTableCodecRegistry() );
        plugin->OnBeforeModuleUnload();
        return TPluginPtr();
    }

    return plugin.StaticCast< CIPlugin >();
}

/*-------------------------------------------------------------------------*/

bool
CTableCodecPluginManager::UnregisterPlugin( TPluginPtr plugin )
{GUCEF_TRACE;

    TTableCodecPluginPtr codecPlugin = plugin.StaticCast< CTableCodecPlugin >();
    if ( codecPlugin.IsNULL() )
        return false;

    // The codecs go from the registry first, nobody finds them there any more. The plugin keeps the module until none of
    // them is referenced: the plugin control asks CTableCodecPlugin::IsModuleInUse() before it unloads the module and
    // CTableCodecPlugin::OnBeforeModuleUnload() then shuts the module down
    codecPlugin->UnregisterCodecs( CCoreGlobal::Instance()->GetTableCodecRegistry() );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
