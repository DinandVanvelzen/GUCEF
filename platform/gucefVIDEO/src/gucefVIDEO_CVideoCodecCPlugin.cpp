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

#ifndef GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H
#include "gucefVIDEO_CVideoCodecPluginManager.h"
#define GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H
#endif /* GUCEF_VIDEO_CVIDEOCODECPLUGINMANAGER_H ? */

#include "gucefVIDEO_CVideoCodecCPlugin.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIVideoCodecCPluginClient::~CIVideoCodecCPluginClient()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVideoCodecCPlugin::CVideoCodecCPlugin( void )
    : CORE::CIPlugin()
    , m_api()
    , m_moduleHandle( GUCEF_NULL )
    , m_plugdata( GUCEF_NULL )
    , m_isLinked( false )
    , m_metaData()
    , m_registeredCodecTypes()
    , m_sources()
    , m_codecInstances()
    , m_callLock()
{GUCEF_TRACE;

    memset( &m_api, 0, sizeof( m_api ) );
}

/*-------------------------------------------------------------------------*/

CVideoCodecCPlugin::~CVideoCodecCPlugin()
{GUCEF_TRACE;

    Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CVideoCodecCPlugin::Link( void* modulePtr, CORE::TPluginMetaDataPtr pluginMetaData )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_callLock );

    if ( m_isLinked || GUCEF_NULL == modulePtr )
        return false;

    // The parameter sizes are only used to find decorated names of stdcall exports
    #define GUCEF_VIDEOCODECPLUG_RESOLVE( funcName, paramBytes ) \
        m_api.funcName = (TVIDEOCODECPLUGFPTR_##funcName) CORE::GetFunctionAddress( modulePtr, "VIDEOCODECPLUG_" #funcName, paramBytes ).funcPtr;

    GUCEF_VIDEOCODECPLUG_RESOLVE( Init, 2*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Shutdown, 1*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Name, 1*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Copyright, 1*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Version, 1*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Type_Count, 1*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Type, 1*sizeof(void*)+4 )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Create_Codec, 3*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Destroy_Codec, 2*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Src_Open, 7*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Src_Get_Audio_Track_Format, 4*sizeof(void*)+4 )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Src_Decode_Frame, 6*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Src_Seek_Frame, 3*sizeof(void*)+8 )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Src_Close, 3*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Decode_Meta_Data, 4*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Snk_Open, 7*sizeof(void*)+4 )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Snk_Write_Frame, 4*sizeof(void*) )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Snk_Write_Audio, 4*sizeof(void*)+8 )
    GUCEF_VIDEOCODECPLUG_RESOLVE( Snk_Close, 3*sizeof(void*)+4 )

    #undef GUCEF_VIDEOCODECPLUG_RESOLVE

    if ( GUCEF_NULL == m_api.Init || GUCEF_NULL == m_api.Shutdown || GUCEF_NULL == m_api.Name ||
         GUCEF_NULL == m_api.Copyright || GUCEF_NULL == m_api.Version || GUCEF_NULL == m_api.Type_Count ||
         GUCEF_NULL == m_api.Type || GUCEF_NULL == m_api.Create_Codec || GUCEF_NULL == m_api.Destroy_Codec ||
         GUCEF_NULL == m_api.Src_Open || GUCEF_NULL == m_api.Src_Get_Audio_Track_Format || GUCEF_NULL == m_api.Src_Decode_Frame ||
         GUCEF_NULL == m_api.Src_Close )
    {
        // Not necessarily an error: modules are offered to every plugin manager when their type is not given
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecCPlugin: Module " << CORE::PointerToString( modulePtr ) << " does not have the functions of an video codec plugin" );
        memset( &m_api, 0, sizeof( m_api ) );
        return false;
    }

    // encoding takes all four sink functions; with one missing the plugin only decodes
    if ( GUCEF_NULL == m_api.Snk_Open || GUCEF_NULL == m_api.Snk_Write_Frame || GUCEF_NULL == m_api.Snk_Write_Audio || GUCEF_NULL == m_api.Snk_Close )
    {
        m_api.Snk_Open = GUCEF_NULL;
        m_api.Snk_Write_Frame = GUCEF_NULL;
        m_api.Snk_Write_Audio = GUCEF_NULL;
        m_api.Snk_Close = GUCEF_NULL;
    }

    TGucefCoreCApi libraryCApi;
    GUCEF_CORE_LinkCApi( &libraryCApi );
    m_plugdata = GUCEF_NULL;
    if ( 0 == m_api.Init( &m_plugdata, &libraryCApi ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecCPlugin: Init of module " << CORE::PointerToString( modulePtr ) << " reported an error" );
        memset( &m_api, 0, sizeof( m_api ) );
        m_plugdata = GUCEF_NULL;
        return false;
    }

    m_moduleHandle = modulePtr;
    m_isLinked = true;

    m_metaData = CORE::TPluginMetaDataStoragePtr( GUCEF_NEW CORE::CPluginMetaData( *pluginMetaData ) );
    m_metaData->SetDescription( GetName() );
    m_metaData->SetCopyright( GetCopyright() );
    m_metaData->SetVersion( GetVersion() );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecCPlugin: Linked \"" << GetName() << "\" version " << CORE::VersionToString( GetVersion() ) <<
                      " offering " << m_api.Type_Count( m_plugdata ) << " codec type(s)" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVideoCodecCPlugin::Unlink( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_callLock );

    if ( !m_isLinked )
        return true;

    if ( !m_sources.empty() || !m_codecInstances.empty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "VideoCodecCPlugin: Unlinking \"" << GetName() << "\" while " << m_sources.size() <<
                           " source(s) and " << m_codecInstances.size() << " codec instance(s) still use it, they are closed now" );
    }

    // Sources first: they use the codec instances
    TClientSet sources;
    sources.swap( m_sources );
    for ( TClientSet::iterator i = sources.begin(); i != sources.end(); ++i )
        (*i)->OnPluginUnlink( m_api, &m_plugdata );

    TClientSet codecInstances;
    codecInstances.swap( m_codecInstances );
    for ( TClientSet::iterator i = codecInstances.begin(); i != codecInstances.end(); ++i )
        (*i)->OnPluginUnlink( m_api, &m_plugdata );

    m_api.Shutdown( &m_plugdata );

    memset( &m_api, 0, sizeof( m_api ) );
    m_plugdata = GUCEF_NULL;
    m_isLinked = false;
    // The handle and the meta data stay: plugin control unloads the module with them after we are unlinked
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVideoCodecCPlugin::IsLinked( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_callLock );
    return m_isLinked;
}

/*-------------------------------------------------------------------------*/

CORE::TPluginMetaDataPtr
CVideoCodecCPlugin::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

void*
CVideoCodecCPlugin::GetModulePointer( void )
{GUCEF_TRACE;

    return m_moduleHandle;
}

/*-------------------------------------------------------------------------*/

CString
CVideoCodecCPlugin::GetName( void ) const
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
CVideoCodecCPlugin::GetCopyright( void ) const
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
CVideoCodecCPlugin::GetVersion( void ) const
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

CVideoCodecCPlugin::TStringVector
CVideoCodecCPlugin::GetCodecTypes( void ) const
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

const CVideoCodecCPlugin::TStringVector&
CVideoCodecCPlugin::GetRegisteredCodecTypes( void ) const
{GUCEF_TRACE;

    return m_registeredCodecTypes;
}

/*-------------------------------------------------------------------------*/

void
CVideoCodecCPlugin::SetRegisteredCodecTypes( const TStringVector& codecTypes )
{GUCEF_TRACE;

    m_registeredCodecTypes = codecTypes;
}

/*-------------------------------------------------------------------------*/

const MT::CMutex&
CVideoCodecCPlugin::GetCallLock( void ) const
{GUCEF_TRACE;

    return m_callLock;
}

/*-------------------------------------------------------------------------*/

const TGucefVideoCodecPluginApi*
CVideoCodecCPlugin::GetLinkedApi( void ) const
{GUCEF_TRACE;

    return m_isLinked ? &m_api : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void**
CVideoCodecCPlugin::GetPlugData( void )
{GUCEF_TRACE;

    return &m_plugdata;
}

/*-------------------------------------------------------------------------*/

void
CVideoCodecCPlugin::RegisterClient( CIVideoCodecCPluginClient* client, bool isSource )
{GUCEF_TRACE;

    if ( GUCEF_NULL == client || !m_isLinked )
        return;

    if ( isSource )
        m_sources.insert( client );
    else
        m_codecInstances.insert( client );
}

/*-------------------------------------------------------------------------*/

void
CVideoCodecCPlugin::UnregisterClient( CIVideoCodecCPluginClient* client )
{GUCEF_TRACE;

    m_sources.erase( client );
    m_codecInstances.erase( client );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VIDEO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
