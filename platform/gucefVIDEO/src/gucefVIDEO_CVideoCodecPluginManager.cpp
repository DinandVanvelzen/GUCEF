/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VIDEO_CVIDEOGLOBAL_H
#include "gucefVIDEO_CVideoGlobal.h"
#define GUCEF_VIDEO_CVIDEOGLOBAL_H
#endif /* GUCEF_VIDEO_CVIDEOGLOBAL_H ? */

#ifndef GUCEF_VIDEO_CVIDEOCODECREGISTRY_H
#include "gucefVIDEO_CVideoCodecRegistry.h"
#define GUCEF_VIDEO_CVIDEOCODECREGISTRY_H
#endif /* GUCEF_VIDEO_CVIDEOCODECREGISTRY_H ? */

#ifndef GUCEF_VIDEO_CVIDEOCODECCPLUGINCODEC_H
#include "gucefVIDEO_CVideoCodecCPluginCodec.h"
#define GUCEF_VIDEO_CVIDEOCODECCPLUGINCODEC_H
#endif /* GUCEF_VIDEO_CVIDEOCODECCPLUGINCODEC_H ? */

#include "gucefVIDEO_CVideoCodecPluginManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CVideoCodecPluginManager::PluginType = "GucefVideoCodecPlugin";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVideoCodecPluginManager::CVideoCodecPluginManager( void )
    : CORE::CPluginManager()
    , m_plugins()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVideoCodecPluginManager::~CVideoCodecPluginManager()
{GUCEF_TRACE;

    UnlinkAll();
}

/*-------------------------------------------------------------------------*/

CString
CVideoCodecPluginManager::GetPluginType( void ) const
{GUCEF_TRACE;

    return PluginType;
}

/*-------------------------------------------------------------------------*/

CORE::TPluginPtr
CVideoCodecPluginManager::RegisterPlugin( void* modulePtr, CORE::TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    TVideoCodecCPluginPtr plugin( GUCEF_NEW CVideoCodecCPlugin() );
    if ( !plugin->Link( modulePtr, pluginMetaData ) )
        return CORE::TPluginPtr();

    CVideoCodecRegistry& registry = CVideoGlobal::Instance()->GetVideoCodecRegistry();
    CVideoCodecCPlugin::TStringVector codecTypes = plugin->GetCodecTypes();
    CVideoCodecCPlugin::TStringVector registeredTypes;
    for ( CVideoCodecCPlugin::TStringVector::iterator i = codecTypes.begin(); i != codecTypes.end(); ++i )
    {
        CString codecType = (*i).Lowercase();
        CVideoCodecRegistry::TVideoCodecPtr codec;
        if ( !CVideoCodecCPluginCodec::Create( plugin, codecType, codec ) )
            continue;

        if ( registry.TryRegister( codecType, codec ) )
        {
            registeredTypes.push_back( codecType );
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecPluginManager: Registered codec type \"" << codecType << "\" of plugin \"" << plugin->GetName() << "\"" );
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecPluginManager: Codec type \"" << codecType << "\" of plugin \"" << plugin->GetName() << "\" is already taken, the plugin does not get it" );
        }
    }

    if ( registeredTypes.empty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecPluginManager: Plugin \"" << plugin->GetName() << "\" has no codec type that could be registered" );
        plugin->Unlink();
        return CORE::TPluginPtr();
    }
    plugin->SetRegisteredCodecTypes( registeredTypes );

    MT::CObjectScopeLock lock( this );
    m_plugins.insert( plugin );
    return plugin.StaticCast< CORE::CIPlugin >();
}

/*-------------------------------------------------------------------------*/

void
CVideoCodecPluginManager::UnregisterCodecs( CVideoCodecCPlugin& plugin )
{GUCEF_TRACE;

    // The registry can be gone at shutdown
    CVideoCodecRegistry& registry = CVideoGlobal::Instance()->GetVideoCodecRegistry();
    const CVideoCodecCPlugin::TStringVector& codecTypes = plugin.GetRegisteredCodecTypes();
    for ( CVideoCodecCPlugin::TStringVector::const_iterator i = codecTypes.begin(); i != codecTypes.end(); ++i )
        registry.TryUnregister( (*i) );
    plugin.SetRegisteredCodecTypes( CVideoCodecCPlugin::TStringVector() );
}

/*-------------------------------------------------------------------------*/

bool
CVideoCodecPluginManager::UnregisterPlugin( CORE::TPluginPtr plugin )
{GUCEF_TRACE;

    TVideoCodecCPluginPtr codecPlugin = plugin.StaticCast< CVideoCodecCPlugin >();
    if ( codecPlugin.IsNULL() )
        return false;

    UnregisterCodecs( *codecPlugin );

    // Codecs others still hold and their open sources are released here, the module is unloaded after this
    codecPlugin->Unlink();

    MT::CObjectScopeLock lock( this );
    m_plugins.erase( codecPlugin );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVideoCodecPluginManager::UnlinkAll( void )
{GUCEF_TRACE;

    TPluginSet plugins;
    {
        MT::CObjectScopeLock lock( this );
        plugins.swap( m_plugins );
    }

    for ( TPluginSet::iterator i = plugins.begin(); i != plugins.end(); ++i )
    {
        TVideoCodecCPluginPtr plugin = (*i);
        UnregisterCodecs( *plugin );
        plugin->Unlink();
    }
}

/*-------------------------------------------------------------------------*/

CVideoCodecPluginManager::TVideoCodecCPluginPtr
CVideoCodecPluginManager::GetPluginForCodecType( const CString& codecType ) const
{GUCEF_TRACE;

    CString lcType = codecType.Lowercase();

    MT::CObjectScopeLock lock( this );
    for ( TPluginSet::const_iterator i = m_plugins.begin(); i != m_plugins.end(); ++i )
    {
        const CVideoCodecCPlugin::TStringVector& codecTypes = (*i)->GetRegisteredCodecTypes();
        for ( CVideoCodecCPlugin::TStringVector::const_iterator n = codecTypes.begin(); n != codecTypes.end(); ++n )
        {
            if ( (*n) == lcType )
                return (*i);
        }
    }
    return TVideoCodecCPluginPtr();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
