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

#include "gucefDRNUTILS_CDeviceAccessRules.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICEDESCRIPTOR_H
#include "gucefCOMCORE_CRpcServiceDescriptor.h"
#define GUCEF_COMCORE_CRPCSERVICEDESCRIPTOR_H
#endif /* GUCEF_COMCORE_CRPCSERVICEDESCRIPTOR_H ? */

#ifndef GUCEF_COMCORE_CRPCMETHODDESCRIPTOR_H
#include "gucefCOMCORE_CRpcMethodDescriptor.h"
#define GUCEF_COMCORE_CRPCMETHODDESCRIPTOR_H
#endif /* GUCEF_COMCORE_CRPCMETHODDESCRIPTOR_H ? */

#ifndef GUCEF_COMCORE_CRPCREQUESTID_H
#include "gucefCOMCORE_CRpcRequestId.h"
#define GUCEF_COMCORE_CRPCREQUESTID_H
#endif /* GUCEF_COMCORE_CRPCREQUESTID_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#include "gucefDRNUTILS_CRemoteServiceDetailCodes.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEDETAILCODES_H ? */

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

const char* const CDeviceAccessRules::SubjectPrefix = "drn:";

/*-------------------------------------------------------------------------*/

namespace {

CORE::CString::StringSet
ParseList( const CORE::CString& csv )
{GUCEF_TRACE;

    CORE::CString::StringSet result;
    CORE::CString::StringVector items = csv.ParseElements( ',', false );
    for ( UInt32 i=0; i<items.size(); ++i )
    {
        CORE::CString item = items[ i ].Trim( true ).Trim( false );
        if ( !item.IsNULLOrEmpty() )
            result.insert( item );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
JoinList( const CORE::CString::StringSet& items )
{GUCEF_TRACE;

    CORE::CString text;
    CORE::CString::StringSet::const_iterator i = items.begin();
    for ( ; i != items.end(); ++i )
    {
        if ( !text.IsNULLOrEmpty() )
            text += ',';
        text += *i;
    }
    return text;
}

}

/*-------------------------------------------------------------------------*/

CDeviceAccessRules::CDeviceAccessRules( const CDeviceRegistry& registry )
    : COMCORE::CIRpcAccessPolicy()
    , m_registry( registry )
    , m_lock()
    , m_rules()
    , m_trustedTransports()
    , m_refused( 0 )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CDeviceAccessRules::~CDeviceAccessRules()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

void
CDeviceAccessRules::AddRule( const CORE::CString& pathPattern, const CORE::CString::StringSet& roles, const CORE::CString::StringSet& kinds )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( UInt32 i=0; i<m_rules.size(); ++i )
    {
        if ( m_rules[ i ].pattern == pathPattern )
        {
            m_rules[ i ].roles = roles;
            m_rules[ i ].kinds = kinds;
            return;
        }
    }
    SRule rule;
    rule.pattern = pathPattern;
    rule.roles = roles;
    rule.kinds = kinds;
    m_rules.push_back( rule );
}

/*-------------------------------------------------------------------------*/

void
CDeviceAccessRules::AddRule( const CORE::CString& pathPattern, const CORE::CString& rolesCsv, const CORE::CString& kindsCsv )
{GUCEF_TRACE;

    AddRule( pathPattern, ParseList( rolesCsv ), ParseList( kindsCsv ) );
}

/*-------------------------------------------------------------------------*/

bool
CDeviceAccessRules::RemoveRule( const CORE::CString& pathPattern )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TRuleVector::iterator i = m_rules.begin();
    for ( ; i != m_rules.end(); ++i )
    {
        if ( (*i).pattern == pathPattern )
        {
            m_rules.erase( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CDeviceAccessRules::ClearRules( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_rules.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CDeviceAccessRules::GetRuleCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_rules.size();
}

/*-------------------------------------------------------------------------*/

void
CDeviceAccessRules::SetTrustedTransports( const CORE::CString::StringSet& transports )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_trustedTransports = transports;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDeviceAccessRules::DeviceIdOfPrincipal( const COMCORE::CRpcPrincipal& principal )
{GUCEF_TRACE;

    static const UInt32 prefixLength = 4;
    if ( principal.subject.Length() <= prefixLength )
        return CORE::CString::Empty;
    if ( principal.subject.SubstrToIndex( prefixLength, true ) != SubjectPrefix )
        return CORE::CString::Empty;
    return principal.subject.CutChars( prefixLength, true );
}

/*-------------------------------------------------------------------------*/

const CDeviceAccessRules::SRule*
CDeviceAccessRules::FindRule( const CORE::CString& methodPath ) const
{GUCEF_TRACE;

    // an exact path wins; otherwise the longest pattern that matches
    const SRule* best = GUCEF_NULL;
    for ( UInt32 i=0; i<m_rules.size(); ++i )
    {
        const SRule& rule = m_rules[ i ];
        if ( rule.pattern == methodPath )
            return &rule;
        if ( methodPath.WildcardEquals( rule.pattern, '*', true ) )
        {
            if ( GUCEF_NULL == best || rule.pattern.Length() > best->pattern.Length() )
                best = &rule;
        }
    }
    return best;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceAccessRules::IsTrusted( const COMCORE::CRpcPrincipal& principal ) const
{GUCEF_TRACE;

    return m_trustedTransports.find( principal.transport ) != m_trustedTransports.end();
}

/*-------------------------------------------------------------------------*/

bool
CDeviceAccessRules::Authorize( const COMCORE::CRpcPrincipal& principal, const CORE::CString& methodPath, CORE::CString& reason, CDeviceRecord* device ) const
{GUCEF_TRACE;

    SRule rule;
    {
        MT::CScopeMutex lock( m_lock );
        if ( IsTrusted( principal ) )
            return true;
        const SRule* found = FindRule( methodPath );
        if ( GUCEF_NULL == found )
        {
            reason = CORE::CString( CRemoteServiceDetailCodes::NoRule ) + ": no rule allows " + methodPath;
            ++m_refused;
            return false;
        }
        rule = *found;
    }

    CORE::CString deviceId = DeviceIdOfPrincipal( principal );
    CDeviceRecord record;
    if ( !principal.isAuthenticated || deviceId.IsNULLOrEmpty() || !m_registry.Find( deviceId, record ) )
    {
        reason = CORE::CString( CRemoteServiceDetailCodes::DeviceUnknown ) + ": the caller is not a paired device";
        MT::CScopeMutex lock( m_lock );
        ++m_refused;
        return false;
    }
    if ( GUCEF_NULL != device )
        *device = record;
    if ( !record.IsActive() )
    {
        reason = CORE::CString( CRemoteServiceDetailCodes::DeviceNotActive ) + ": the device " + deviceId + " is " + DeviceStatusToString( record.status );
        MT::CScopeMutex lock( m_lock );
        ++m_refused;
        return false;
    }
    if ( !rule.kinds.empty() && rule.kinds.find( record.kind ) == rule.kinds.end() )
    {
        reason = CORE::CString( CRemoteServiceDetailCodes::DeviceKind ) + ": " + methodPath + " is not for devices of kind " + record.kind;
        MT::CScopeMutex lock( m_lock );
        ++m_refused;
        return false;
    }
    if ( !rule.roles.empty() && !record.HasAnyRole( rule.roles ) )
    {
        reason = CORE::CString( CRemoteServiceDetailCodes::RoleRequired ) + ": " + methodPath + " needs one of the roles " + JoinList( rule.roles );
        MT::CScopeMutex lock( m_lock );
        ++m_refused;
        return false;
    }
    if ( record.roles.empty() )
    {
        reason = CORE::CString( CRemoteServiceDetailCodes::RoleRequired ) + ": the device " + deviceId + " has no role";
        MT::CScopeMutex lock( m_lock );
        ++m_refused;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceAccessRules::MayBindContract( const COMCORE::CRpcPrincipal& principal, const COMCORE::CRpcContract& contract ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( IsTrusted( principal ) )
            return true;
    }
    for ( UInt32 s=0; s<contract.GetServiceCount(); ++s )
    {
        const COMCORE::CRpcServiceDescriptor& service = contract.GetServiceAt( s );
        for ( UInt32 m=0; m<service.GetMethodCount(); ++m )
        {
            CORE::CString reason;
            if ( Authorize( principal, service.GetMethodAt( m ).GetPath(), reason ) )
                return true;
        }
    }
    // a refused bind is not a refused call: the counter counts calls only
    return false;
}

/*-------------------------------------------------------------------------*/

COMCORE::TRpcAccessDecision
CDeviceAccessRules::AuthorizeCall( const COMCORE::CRpcPrincipal& principal       ,
                                   const COMCORE::CRpcContract& contract         ,
                                   const COMCORE::CRpcServiceDescriptor& service ,
                                   const COMCORE::CRpcMethodDescriptor& method   ,
                                   const COMCORE::CRpcMetadata& metadata         ,
                                   CORE::CString& reason                         ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( IsTrusted( principal ) )
            return COMCORE::RPC_ACCESS_ALLOW;
    }
    if ( !principal.isAuthenticated )
    {
        reason = CORE::CString( CRemoteServiceDetailCodes::DeviceUnknown ) + ": an authenticated device is required";
        MT::CScopeMutex lock( m_lock );
        ++m_refused;
        return COMCORE::RPC_ACCESS_UNAUTHENTICATED;
    }
    if ( Authorize( principal, method.GetPath(), reason ) )
        return COMCORE::RPC_ACCESS_ALLOW;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DeviceAccessRules: refused " << method.GetPath() << " for " << principal.subject << ": " << reason );
    return COMCORE::RPC_ACCESS_DENY;
}

/*-------------------------------------------------------------------------*/

bool
CDeviceAccessRules::MayQueryRequest( const COMCORE::CRpcPrincipal& principal, const CORE::CString& ownerSubject, const COMCORE::CRpcRequestId& id ) const
{GUCEF_TRACE;

    return principal.subject == ownerSubject;
}

/*-------------------------------------------------------------------------*/

UInt32
CDeviceAccessRules::GetRefusedCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_refused;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
