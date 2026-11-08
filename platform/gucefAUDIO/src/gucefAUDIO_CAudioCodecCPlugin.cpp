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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_C_API_IMPL_H
#include "gucefCORE_c_api_impl.h"
#define GUCEF_CORE_C_API_IMPL_H
#endif /* GUCEF_CORE_C_API_IMPL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H
#include "gucefAUDIO_CAudioCodecPluginManager.h"
#define GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H
#endif /* GUCEF_AUDIO_CAUDIOCODECPLUGINMANAGER_H ? */

#include "gucefAUDIO_CAudioCodecCPlugin.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIAudioCodecCPluginClient::~CIAudioCodecCPluginClient()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioCodecCPlugin::CAudioCodecCPlugin( void )
    : CORE::CIPlugin()
    , m_api()
    , m_moduleHandle( GUCEF_NULL )
    , m_plugdata( GUCEF_NULL )
    , m_isLinked( false )
    , m_threadingModel( GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK )
    , m_metaData()
    , m_registeredCodecTypes()
    , m_streams()
    , m_codecInstances()
    , m_callLock()
{GUCEF_TRACE;

    memset( &m_api, 0, sizeof( m_api ) );
}

/*-------------------------------------------------------------------------*/

CAudioCodecCPlugin::~CAudioCodecCPlugin()
{GUCEF_TRACE;

    Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CAudioCodecCPlugin::Link( void* modulePtr, CORE::TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_callLock );

    if ( m_isLinked || GUCEF_NULL == modulePtr )
        return false;

    // The parameter sizes are only used to find decorated names of stdcall exports
    #define GUCEF_AUDIOCODECPLUG_RESOLVE( funcName, paramBytes ) \
        m_api.funcName = (TAUDIOCODECPLUGFPTR_##funcName) CORE::GetFunctionAddress( modulePtr, "AUDIOCODECPLUG_" #funcName, paramBytes ).funcPtr;

    GUCEF_AUDIOCODECPLUG_RESOLVE( Init, 2*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Shutdown, 1*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Name, 1*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Copyright, 1*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Version, 1*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Type_Count, 1*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Type, 1*sizeof(void*)+4 )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Create_Codec, 3*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Destroy_Codec, 2*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Src_Open, 7*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Src_Read_Frames, 5*sizeof(void*)+4 )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Src_Seek, 3*sizeof(void*)+8 )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Src_Close, 3*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Decode_Meta_Data, 4*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Encode, 6*sizeof(void*)+8 )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Threading_Model, 1*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Src_Meta_Data, 4*sizeof(void*) )
    GUCEF_AUDIOCODECPLUG_RESOLVE( Probe, 4*sizeof(void*) )

    #undef GUCEF_AUDIOCODECPLUG_RESOLVE

    if ( GUCEF_NULL == m_api.Init || GUCEF_NULL == m_api.Shutdown || GUCEF_NULL == m_api.Name ||
         GUCEF_NULL == m_api.Copyright || GUCEF_NULL == m_api.Version || GUCEF_NULL == m_api.Type_Count ||
         GUCEF_NULL == m_api.Type || GUCEF_NULL == m_api.Create_Codec || GUCEF_NULL == m_api.Destroy_Codec ||
         GUCEF_NULL == m_api.Src_Open || GUCEF_NULL == m_api.Src_Read_Frames || GUCEF_NULL == m_api.Src_Close )
    {
        // Not necessarily an error: modules are offered to every plugin manager when their type is not given
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AudioCodecCPlugin: Module " << CORE::PointerToString( modulePtr ) << " does not have the functions of an audio codec plugin" );
        memset( &m_api, 0, sizeof( m_api ) );
        return false;
    }

    TGucefCoreCApi libraryCApi;
    GUCEF_CORE_LinkCApi( &libraryCApi );
    m_plugdata = GUCEF_NULL;
    if ( 0 == m_api.Init( &m_plugdata, &libraryCApi ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioCodecCPlugin: Init of module " << CORE::PointerToString( modulePtr ) << " reported an error" );
        memset( &m_api, 0, sizeof( m_api ) );
        m_plugdata = GUCEF_NULL;
        return false;
    }

    m_moduleHandle = modulePtr;
    m_isLinked = true;

    // The optional threading declaration. Anything but a clear "per source" keeps the one call at a time rule
    m_threadingModel = GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK;
    if ( GUCEF_NULL != m_api.Threading_Model )
    {
        const UInt32 declared = m_api.Threading_Model( m_plugdata );
        if ( GUCEF_AUDIOCODEC_THREADING_PER_SOURCE == declared )
            m_threadingModel = GUCEF_AUDIOCODEC_THREADING_PER_SOURCE;
        else
        if ( GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK != declared )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioCodecCPlugin: Module " << CORE::PointerToString( modulePtr ) << " declares unknown threading model " << declared << ", using one call at a time" );
    }

    m_metaData = CORE::TPluginMetaDataStoragePtr( GUCEF_NEW CORE::CPluginMetaData( *pluginMetaData ) );
    m_metaData->SetDescription( GetName() );
    m_metaData->SetCopyright( GetCopyright() );
    m_metaData->SetVersion( GetVersion() );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AudioCodecCPlugin: Linked \"" << GetName() << "\" version " << CORE::VersionToString( GetVersion() ) <<
                      " offering " << m_api.Type_Count( m_plugdata ) << " codec type(s)" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioCodecCPlugin::Unlink( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_callLock );

    if ( !m_isLinked )
        return true;

    if ( !m_streams.empty() || !m_codecInstances.empty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AudioCodecCPlugin: Unlinking \"" << GetName() << "\" while " << m_streams.size() <<
                           " stream(s) and " << m_codecInstances.size() << " codec instance(s) still use it, they are closed now" );
    }

    // Streams first: they use the codec instances
    TClientSet streams;
    streams.swap( m_streams );
    for ( TClientSet::iterator i = streams.begin(); i != streams.end(); ++i )
        (*i)->OnPluginUnlink( m_api, &m_plugdata );

    TClientSet codecInstances;
    codecInstances.swap( m_codecInstances );
    for ( TClientSet::iterator i = codecInstances.begin(); i != codecInstances.end(); ++i )
        (*i)->OnPluginUnlink( m_api, &m_plugdata );

    m_api.Shutdown( &m_plugdata );

    memset( &m_api, 0, sizeof( m_api ) );
    m_plugdata = GUCEF_NULL;
    m_isLinked = false;
    m_threadingModel = GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK;
    // The handle and the meta data stay: plugin control unloads the module with them after we are unlinked
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioCodecCPlugin::IsLinked( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_callLock );
    return m_isLinked;
}

/*-------------------------------------------------------------------------*/

CORE::TPluginMetaDataPtr
CAudioCodecCPlugin::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

void*
CAudioCodecCPlugin::GetModulePointer( void )
{GUCEF_TRACE;

    return m_moduleHandle;
}

/*-------------------------------------------------------------------------*/

CString
CAudioCodecCPlugin::GetName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_callLock );
    if ( m_isLinked )
    {
        const char* name = m_api.Name( m_plugdata );
        if ( GUCEF_NULL != name )
            return name;
    }
    if ( !m_metaData.IsNULL() )
        return m_metaData->GetDescription();
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CString
CAudioCodecCPlugin::GetCopyright( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_callLock );
    if ( m_isLinked )
    {
        const char* copyright = m_api.Copyright( m_plugdata );
        if ( GUCEF_NULL != copyright )
            return copyright;
    }
    if ( !m_metaData.IsNULL() )
        return m_metaData->GetCopyright();
    return CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::TVersion
CAudioCodecCPlugin::GetVersion( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_callLock );
    if ( m_isLinked )
    {
        const CORE::TVersion* version = m_api.Version( m_plugdata );
        if ( GUCEF_NULL != version )
            return *version;
    }
    if ( !m_metaData.IsNULL() )
        return m_metaData->GetVersion();

    CORE::TVersion none;
    memset( &none, 0, sizeof( none ) );
    return none;
}

/*-------------------------------------------------------------------------*/

CAudioCodecCPlugin::TStringVector
CAudioCodecCPlugin::GetCodecTypes( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_callLock );

    TStringVector types;
    if ( m_isLinked )
    {
        UInt32 typeCount = m_api.Type_Count( m_plugdata );
        for ( UInt32 i=0; i<typeCount; ++i )
        {
            const char* type = m_api.Type( m_plugdata, i );
            if ( GUCEF_NULL != type && '\0' != *type )
                types.push_back( type );
        }
    }
    return types;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioCodecCPlugin::GetThreadingModel( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_callLock );
    return m_threadingModel;
}

/*-------------------------------------------------------------------------*/

const CAudioCodecCPlugin::TStringVector&
CAudioCodecCPlugin::GetRegisteredCodecTypes( void ) const
{GUCEF_TRACE;

    return m_registeredCodecTypes;
}

/*-------------------------------------------------------------------------*/

void
CAudioCodecCPlugin::SetRegisteredCodecTypes( const TStringVector& codecTypes )
{GUCEF_TRACE;

    m_registeredCodecTypes = codecTypes;
}

/*-------------------------------------------------------------------------*/

const MT::CMutex&
CAudioCodecCPlugin::GetCallLock( void ) const
{GUCEF_TRACE;

    return m_callLock;
}

/*-------------------------------------------------------------------------*/

const TGucefAudioCodecPluginApi*
CAudioCodecCPlugin::GetLinkedApi( void ) const
{GUCEF_TRACE;

    return m_isLinked ? &m_api : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void**
CAudioCodecCPlugin::GetPlugData( void )
{GUCEF_TRACE;

    return &m_plugdata;
}

/*-------------------------------------------------------------------------*/

void
CAudioCodecCPlugin::RegisterClient( CIAudioCodecCPluginClient* client, bool isStream )
{GUCEF_TRACE;

    if ( GUCEF_NULL == client || !m_isLinked )
        return;

    if ( isStream )
        m_streams.insert( client );
    else
        m_codecInstances.insert( client );
}

/*-------------------------------------------------------------------------*/

void
CAudioCodecCPlugin::UnregisterClient( CIAudioCodecCPluginClient* client )
{GUCEF_TRACE;

    m_streams.erase( client );
    m_codecInstances.erase( client );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
