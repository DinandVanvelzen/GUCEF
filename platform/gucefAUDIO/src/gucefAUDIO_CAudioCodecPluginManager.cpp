/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECREGISTRY_H
#include "gucefAUDIO_CAudioCodecRegistry.h"
#define GUCEF_AUDIO_CAUDIOCODECREGISTRY_H
#endif /* GUCEF_AUDIO_CAUDIOCODECREGISTRY_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H
#include "gucefAUDIO_CAudioCodecCPluginCodec.h"
#define GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H
#endif /* GUCEF_AUDIO_CAUDIOCODECCPLUGINCODEC_H ? */

#include "gucefAUDIO_CAudioCodecPluginManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CAudioCodecPluginManager::PluginType = "GucefAudioCodecPlugin";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioCodecPluginManager::CAudioCodecPluginManager( void )
    : CORE::CPluginManager()
    , m_plugins()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioCodecPluginManager::~CAudioCodecPluginManager()
{GUCEF_TRACE;

    UnlinkAll();
}

/*-------------------------------------------------------------------------*/

CString
CAudioCodecPluginManager::GetPluginType( void ) const
{GUCEF_TRACE;

    return PluginType;
}

/*-------------------------------------------------------------------------*/

CORE::TPluginPtr
CAudioCodecPluginManager::RegisterPlugin( void* modulePtr, CORE::TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    TAudioCodecCPluginPtr plugin( GUCEF_NEW CAudioCodecCPlugin() );
    if ( !plugin->Link( modulePtr, pluginMetaData ) )
        return CORE::TPluginPtr();

    CAudioCodecRegistry& registry = CAudioGlobal::Instance()->GetAudioCodecRegistry();
    CAudioCodecCPlugin::TStringVector codecTypes = plugin->GetCodecTypes();
    CAudioCodecCPlugin::TStringVector registeredTypes;
    for ( CAudioCodecCPlugin::TStringVector::iterator i = codecTypes.begin(); i != codecTypes.end(); ++i )
    {
        CString codecType = (*i).Lowercase();
        CAudioCodecRegistry::TAudioCodecPtr codec;
        if ( !CAudioCodecCPluginCodec::Create( plugin, codecType, codec ) )
            continue;

        if ( registry.TryRegister( codecType, codec ) )
        {
            registeredTypes.push_back( codecType );
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AudioCodecPluginManager: Registered codec type \"" << codecType << "\" of plugin \"" << plugin->GetName() << "\"" );
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioCodecPluginManager: Codec type \"" << codecType << "\" of plugin \"" << plugin->GetName() << "\" is already taken, the plugin does not get it" );
        }
    }

    if ( registeredTypes.empty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioCodecPluginManager: Plugin \"" << plugin->GetName() << "\" has no codec type that could be registered" );
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
CAudioCodecPluginManager::UnregisterCodecs( CAudioCodecCPlugin& plugin )
{GUCEF_TRACE;

    // The registry can be gone at shutdown
    CAudioCodecRegistry& registry = CAudioGlobal::Instance()->GetAudioCodecRegistry();
    const CAudioCodecCPlugin::TStringVector& codecTypes = plugin.GetRegisteredCodecTypes();
    for ( CAudioCodecCPlugin::TStringVector::const_iterator i = codecTypes.begin(); i != codecTypes.end(); ++i )
        registry.TryUnregister( (*i) );
    plugin.SetRegisteredCodecTypes( CAudioCodecCPlugin::TStringVector() );
}

/*-------------------------------------------------------------------------*/

bool
CAudioCodecPluginManager::UnregisterPlugin( CORE::TPluginPtr plugin )
{GUCEF_TRACE;

    TAudioCodecCPluginPtr codecPlugin = plugin.StaticCast< CAudioCodecCPlugin >();
    if ( codecPlugin.IsNULL() )
        return false;

    UnregisterCodecs( *codecPlugin );

    // Codecs others still hold and their open streams are released here, the module is unloaded after this
    codecPlugin->Unlink();

    MT::CObjectScopeLock lock( this );
    m_plugins.erase( codecPlugin );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioCodecPluginManager::UnlinkAll( void )
{GUCEF_TRACE;

    TPluginSet plugins;
    {
        MT::CObjectScopeLock lock( this );
        plugins.swap( m_plugins );
    }

    for ( TPluginSet::iterator i = plugins.begin(); i != plugins.end(); ++i )
    {
        TAudioCodecCPluginPtr plugin = (*i);
        UnregisterCodecs( *plugin );
        plugin->Unlink();
    }
}

/*-------------------------------------------------------------------------*/

CAudioCodecPluginManager::TAudioCodecCPluginPtr
CAudioCodecPluginManager::GetPluginForCodecType( const CString& codecType ) const
{GUCEF_TRACE;

    CString lcType = codecType.Lowercase();

    MT::CObjectScopeLock lock( this );
    for ( TPluginSet::const_iterator i = m_plugins.begin(); i != m_plugins.end(); ++i )
    {
        const CAudioCodecCPlugin::TStringVector& codecTypes = (*i)->GetRegisteredCodecTypes();
        for ( CAudioCodecCPlugin::TStringVector::const_iterator n = codecTypes.begin(); n != codecTypes.end(); ++n )
        {
            if ( (*n) == lcType )
                return (*i);
        }
    }
    return TAudioCodecCPluginPtr();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
