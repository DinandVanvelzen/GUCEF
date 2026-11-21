/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H
#include "gucefDRN_CDRNResourceIoTaskConsumer.h"
#define GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H
#endif /* GUCEF_DRN_CDRNRESOURCEIOTASKCONSUMER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSHAREWARMJOB_H
#include "gucefASSETSTORE_CAssetShareWarmJob.h"
#define GUCEF_ASSETSTORE_CASSETSHAREWARMJOB_H
#endif /* GUCEF_ASSETSTORE_CASSETSHAREWARMJOB_H ? */

#include "gucefASSETSTORE_CAssetShareDecision.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The paths whose warming waits for the pool at one time */
const UInt32 MaxQueuedWarms = 256;

/**
 *  Whether a name of a share stays inside the share: not empty, no empty, "." or ".." part, no backslash, colon or control character, no
 *  leading separator. The facility refuses such names before this class sees them; the check is repeated because the VFS path made from a
 *  name goes to the gate and to the service.
 */
bool
IsNameInsideShare( const CORE::CString& name )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        return false;
    const UInt32 length = name.Length();
    UInt32 partStart = 0;
    for ( UInt32 i=0; i<=length; ++i )
    {
        const char c = i < length ? name[ i ] : '/';
        if ( '\\' == c || ':' == c || (unsigned char) c < 0x20 || 0x7F == (unsigned char) c )
            return false;
        if ( '/' == c )
        {
            const UInt32 partLength = i - partStart;
            if ( 0 == partLength )
                return false;
            if ( 1 == partLength && '.' == name[ partStart ] )
                return false;
            if ( 2 == partLength && '.' == name[ partStart ] && '.' == name[ partStart + 1 ] )
                return false;
            partStart = i + 1;
        }
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetShareDecision::CAssetShareDecision( const TAssetStoreServicePtr& service, CAssetFileServingGate& gate, const CORE::CString::StringSet& learningRoles )
    : DRNUTILS::CIRemoteShareDecision()
    , m_service( service )
    , m_gate( gate )
    , m_learningRoles( learningRoles )
    , m_lock()
    , m_roots()
    , m_queued()
    , m_shutdown( false )
    , m_refusals( 0 )
    , m_notKept( 0 )
    , m_warms( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetShareDecision::~CAssetShareDecision()
{GUCEF_TRACE;

    Shutdown( 30000 );
}

/*-------------------------------------------------------------------------*/

void
CAssetShareDecision::SetShareRoot( const CORE::CString& share, const CORE::CString& vfsRoot )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_roots[ share ] = vfsRoot;
}

/*-------------------------------------------------------------------------*/

void
CAssetShareDecision::RemoveShareRoot( const CORE::CString& share )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_roots.erase( share );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetShareDecision::GetShareRoot( const CORE::CString& share ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    GUCEF::map< CORE::CString, CORE::CString >::const_iterator i = m_roots.find( share );
    return i == m_roots.end() ? CORE::CString::Empty : (*i).second;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetShareDecision::MakeServicePath( const CORE::CString& share, const CORE::CString& name ) const
{GUCEF_TRACE;

    const CORE::CString root = GetShareRoot( share );
    if ( root.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    return name.IsNULLOrEmpty() ? root : root + "/" + name;
}

/*-------------------------------------------------------------------------*/

CAssetFileServingGate::SCaller
CAssetShareDecision::MakeCaller( const CORE::CString& deviceId, const CORE::CString::StringSet& roles, const CORE::CString::StringSet& learningRoles )
{GUCEF_TRACE;

    CAssetFileServingGate::SCaller caller;
    caller.deviceId = deviceId;
    caller.owner = roles.find( "owner" ) != roles.end();
    caller.mcp = roles.find( "mcp" ) != roles.end();
    caller.learning = false;
    for ( CORE::CString::StringSet::const_iterator i = learningRoles.begin(); i != learningRoles.end(); ++i )
        if ( roles.find( (*i) ) != roles.end() )
            caller.learning = true;
    return caller;
}

/*-------------------------------------------------------------------------*/

bool
CAssetShareDecision::MayRead( const DRNUTILS::CDeviceRecord& device, const CORE::CString& share, const CORE::CString& name, CORE::CString& refusal )
{GUCEF_TRACE;

    // a caller that cannot be told to ask again is refused for the moment
    bool retryLater = false;
    return CheckRead( device, share, name, refusal, retryLater );
}

/*-------------------------------------------------------------------------*/

bool
CAssetShareDecision::CheckRead( const DRNUTILS::CDeviceRecord& device, const CORE::CString& share, const CORE::CString& name, CORE::CString& refusal, bool& retryLater )
{GUCEF_TRACE;

    retryLater = false;
    const CORE::CString servicePath = IsNameInsideShare( name ) ? MakeServicePath( share, name ) : CORE::CString::Empty;
    if ( servicePath.IsNULLOrEmpty() )
    {
        refusal = "There is no such file";
        return false;
    }
    const CAssetFileServingGate::SCaller caller = MakeCaller( device.deviceId, device.roles, m_learningRoles );

    // the gate looks at the pack of the path for a caller that needs a look (and not for the machine role, which it refuses at once): the
    // node's thread does not ask the catalogue for it, so the answer has to be kept already
    if ( caller.NeedsLook() && !( caller.mcp && !caller.owner ) )
    {
        CORE::CString::StringSet flags;
        bool inPack = false;
        if ( !m_service->TryGetRestrictionsOfPathKept( servicePath, flags, inPack ) )
        {
            ++m_notKept;
            QueueWarm( servicePath );
            refusal = "The licence of this file is being looked up, ask again in a moment";
            retryLater = true;
            return false;
        }
    }
    CORE::CString denial;
    if ( CAssetFileServingGate::ALLOWED != m_gate.CheckFile( caller, servicePath, denial ) )
    {
        // the licence gate: the blocks of a file are for the owner devices; the machine role sees metadata only
        ++m_refusals;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetShareDecision: a read of " << servicePath << " is refused for the device " << device.deviceId << ": " << denial );
        refusal = denial;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetShareDecision::Warm( const CORE::CString& servicePath )
{GUCEF_TRACE;

    if ( m_service.IsNULL() || servicePath.IsNULLOrEmpty() )
        return;
    CORE::CString::StringSet flags;
    bool inPack = false;
    if ( m_service->TryGetRestrictionsOfPathKept( servicePath, flags, inPack ) )
        return;
    m_service->GetRestrictionsOfPath( servicePath, flags );
    ++m_warms;
}

/*-------------------------------------------------------------------------*/

bool
CAssetShareDecision::QueueWarm( const CORE::CString& servicePath )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( m_shutdown || m_queued.size() >= MaxQueuedWarms )
            return false;
        if ( !m_queued.insert( servicePath ).second )
            return true;
    }
    DRN::CIDRNResourceIoJobPtr job( GUCEF_NEW CAssetShareWarmJob( *this, servicePath ) );
    const CORE::CFutureResult queued = DRN::CDRNResourceIoTaskConsumer::Queue( CORE::CString::Empty, job );
    if ( queued.HasNoFuture() )
    {
        WarmDone( servicePath );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetShareDecision::WarmDone( const CORE::CString& servicePath )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_queued.erase( servicePath );
}

/*-------------------------------------------------------------------------*/

void
CAssetShareDecision::Shutdown( UInt32 maxWaitMs )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_shutdown = true;
    }
    for ( UInt32 waited=0; waited<maxWaitMs; waited+=2 )
    {
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_queued.empty() )
                return;
        }
        MT::ThreadDelay( 2 );
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetShareDecision::GetRefusalCount( void ) const
{GUCEF_TRACE;

    return m_refusals.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetShareDecision::GetNotKeptCount( void ) const
{GUCEF_TRACE;

    return m_notKept.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetShareDecision::GetWarmCount( void ) const
{GUCEF_TRACE;

    return m_warms.Load();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
