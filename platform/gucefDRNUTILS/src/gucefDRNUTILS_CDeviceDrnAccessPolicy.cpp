/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CDeviceDrnAccessPolicy.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINK_H
#include "gucefDRN_CDRNPeerLink.h"
#define GUCEF_DRN_CDRNPEERLINK_H
#endif /* GUCEF_DRN_CDRNPEERLINK_H ? */

#ifndef GUCEF_DRN_CIDRNAUTHENTICATOR_H
#include "gucefDRN_CIDRNAuthenticator.h"
#define GUCEF_DRN_CIDRNAUTHENTICATOR_H
#endif /* GUCEF_DRN_CIDRNAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDeviceDrnAccessPolicy::CDeviceDrnAccessPolicy( const CDeviceRegistry& registry )
    : DRN::CIDRNAccessPolicy()
    , m_registry( registry )
    , m_lock()
    , m_rules()
    , m_refusedJoins( 0 )
    , m_shareLock()
    , m_shares()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CDeviceDrnAccessPolicy::~CDeviceDrnAccessPolicy()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

void
CDeviceDrnAccessPolicy::AddSubscribeRule( const CORE::CString& itemPattern, const CORE::CString::StringSet& roles )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_rules.size(); ++i )
    {
        if ( m_rules[ i ].pattern == itemPattern )
        {
            m_rules[ i ].roles = roles;
            return;
        }
    }
    SRule rule;
    rule.pattern = itemPattern;
    rule.roles = roles;
    m_rules.push_back( rule );
}

/*-------------------------------------------------------------------------*/

void
CDeviceDrnAccessPolicy::ClearSubscribeRules( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_rules.clear();
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::MayJoinAs( const DRN::CDRNAuthResult& auth, CORE::CString& reason ) const
{GUCEF_TRACE;

    CDeviceRecord device;
    if ( auth.userId.IsNULLOrEmpty() || !m_registry.Find( auth.userId, device ) )
    {
        reason = "the device is not registered";
        return false;
    }
    if ( !device.IsActive() )
    {
        reason = CORE::CString( "the device is " ) + DeviceStatusToString( device.status );
        return false;
    }
    if ( device.roles.empty() )
    {
        reason = "the device has no role";
        return false;
    }
    UInt8 registered[ 32 ];
    if ( 32 != auth.publicKey.GetDataSize() || !DRN::CDRNCrypto::FromHex( device.publicKey, registered, 32 ) )
    {
        reason = "the token does not name the key of the device";
        return false;
    }
    if ( !DRN::CDRNCrypto::ConstantTimeEquals( registered, auth.publicKey.GetConstBufferPtr(), 32 ) )
    {
        reason = "the key of the token is not the key of the device";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::MayJoin( const DRN::CDRNPeerLink& link, const CORE::CString& sessionName )
{GUCEF_TRACE;

    CORE::CString reason;
    if ( MayJoinAs( link.GetAuthResult(), reason ) )
        return true;
    {
        MT::CScopeMutex lock( m_lock );
        ++m_refusedJoins;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeviceDrnAccessPolicy: refused the join of \"" << link.GetAuthResult().userId << "\" to session \"" << sessionName << "\": " << reason );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::MaySubscribe( UInt32 peerId, const CORE::CString& itemName )
{GUCEF_TRACE;

    // without the authentication of the member nothing is known of its device
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::MayWrite( UInt32 peerId, const CORE::CString& itemName, UInt32 keyIndex )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::MaySubscribeAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& itemName )
{GUCEF_TRACE;

    CDeviceRecord device;
    if ( auth.userId.IsNULLOrEmpty() || !m_registry.Find( auth.userId, device ) || !device.IsActive() )
        return false;

    MT::CScopeMutex lock( m_lock );
    const SRule* best = GUCEF_NULL;
    for ( UInt32 i=0; i<m_rules.size(); ++i )
    {
        const SRule& rule = m_rules[ i ];
        if ( rule.pattern == itemName )
        {
            best = &rule;
            break;
        }
        if ( itemName.WildcardEquals( rule.pattern, '*', true ) && ( GUCEF_NULL == best || rule.pattern.Length() > best->pattern.Length() ) )
            best = &rule;
    }
    if ( GUCEF_NULL == best )
        return false;
    return best->roles.empty() ? !device.roles.empty() : device.HasAnyRole( best->roles );
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::MayWriteAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& itemName, UInt32 keyIndex )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

void
CDeviceDrnAccessPolicy::SetShare( const CORE::CString& share, const CORE::CString::StringSet& roles, CIRemoteShareDecision* decision )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_shareLock );
    SShare& entry = m_shares[ share ];
    entry.roles = roles;
    entry.decision = decision;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::RemoveShare( const CORE::CString& share )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_shareLock );
    return 0 != m_shares.erase( share );
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::HasShare( const CORE::CString& share ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_shareLock );
    return m_shares.find( share ) != m_shares.end();
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::ResolveShareDevice( const DRN::CDRNAuthResult& auth, const SShare& share, CDeviceRecord& device ) const
{GUCEF_TRACE;

    // the registry at this call decides: a device revoked or without the role since the join has nothing
    if ( auth.userId.IsNULLOrEmpty() || !m_registry.Find( auth.userId, device ) || !device.IsActive() || device.roles.empty() )
        return false;
    if ( share.roles.find( "*" ) != share.roles.end() )
        return true;
    return device.HasAnyRole( share.roles );
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::MayReadResourceAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                           bool sharedByProvider )
{GUCEF_TRACE;

    CORE::CString refusal;
    return CheckReadResourceAs( peerId, auth, providerName, resourceName, sharedByProvider, refusal );
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::CheckReadResourceAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                             bool sharedByProvider, CORE::CString& refusal )
{GUCEF_TRACE;

    // a caller of this form cannot be told to ask again: for it a read that is to be tried later is not allowed
    bool retryLater = false;
    return DecideReadResourceAs( peerId, auth, providerName, resourceName, sharedByProvider, refusal, retryLater );
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::DecideReadResourceAs( UInt32 /* peerId */, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                              bool sharedByProvider, CORE::CString& refusal, bool& retryLater )
{GUCEF_TRACE;

    retryLater = false;
    MT::CScopeMutex lock( m_shareLock );
    TShareMap::const_iterator i = m_shares.find( providerName );
    if ( i == m_shares.end() )
        return sharedByProvider;
    CDeviceRecord device;
    if ( !sharedByProvider || !ResolveShareDevice( auth, (*i).second, device ) || GUCEF_NULL == (*i).second.decision )
        return false;
    return (*i).second.decision->CheckRead( device, providerName, resourceName, refusal, retryLater );
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::MayBrowseResourceAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                             bool sharedByProvider )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_shareLock );
    TShareMap::const_iterator i = m_shares.find( providerName );
    if ( i == m_shares.end() )
        return DRN::CIDRNAccessPolicy::MayBrowseResourceAs( peerId, auth, providerName, resourceName, sharedByProvider );
    CDeviceRecord device;
    if ( !sharedByProvider || !ResolveShareDevice( auth, (*i).second, device ) || GUCEF_NULL == (*i).second.decision )
        return false;
    return (*i).second.decision->MayBrowse( device, providerName, resourceName );
}

/*-------------------------------------------------------------------------*/

bool
CDeviceDrnAccessPolicy::MayListEntryAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& entryName,
                                        bool isDirectory )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_shareLock );
    TShareMap::const_iterator i = m_shares.find( providerName );
    if ( i == m_shares.end() )
        return true;
    CDeviceRecord device;
    if ( !ResolveShareDevice( auth, (*i).second, device ) || GUCEF_NULL == (*i).second.decision )
        return false;
    return (*i).second.decision->MayListEntry( device, providerName, entryName, isDirectory );
}

/*-------------------------------------------------------------------------*/

UInt32
CDeviceDrnAccessPolicy::GetRefusedJoinCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_refusedJoins;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
