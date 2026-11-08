/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefCOMCORE_CRpcRoleAccessPolicy.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcRoleAccessPolicy::CRpcRoleAccessPolicy( void )
    : CIRpcAccessPolicy()
    , CORE::CIConfigurable()
    , m_lock()
    , m_rules()
    , m_requireAuthenticated( true )
    , m_trustedTransports()
{GUCEF_TRACE;

    m_trustedTransports.insert( "loopback" );
}

/*-------------------------------------------------------------------------*/

CRpcRoleAccessPolicy::~CRpcRoleAccessPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRpcRoleAccessPolicy::AddRule( const CORE::CString& pathPattern, const CORE::CString::StringSet& roles )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    SRule rule;
    rule.pattern = pathPattern;
    rule.roles = roles;
    m_rules.push_back( rule );
}

/*-------------------------------------------------------------------------*/

void
CRpcRoleAccessPolicy::ClearRules( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_rules.clear();
}

/*-------------------------------------------------------------------------*/

void
CRpcRoleAccessPolicy::SetRequireAuthenticated( bool require )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_requireAuthenticated = require;
}

/*-------------------------------------------------------------------------*/

void
CRpcRoleAccessPolicy::SetTrustedTransports( const CORE::CString::StringSet& transports )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_trustedTransports = transports;
}

/*-------------------------------------------------------------------------*/

bool
CRpcRoleAccessPolicy::IsTrustedOrAuthenticated( const CRpcPrincipal& principal ) const
{GUCEF_TRACE;

    if ( !m_requireAuthenticated || principal.isAuthenticated )
        return true;
    return m_trustedTransports.find( principal.transport ) != m_trustedTransports.end();
}

/*-------------------------------------------------------------------------*/

bool
CRpcRoleAccessPolicy::HasAnyRole( const CRpcPrincipal& principal, const CORE::CString::StringSet& roles )
{GUCEF_TRACE;

    if ( roles.find( "*" ) != roles.end() )
        return true;
    CORE::CString::StringSet::const_iterator i = roles.begin();
    for ( ; i != roles.end(); ++i )
        if ( principal.HasRole( *i ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CRpcRoleAccessPolicy::MayBindContract( const CRpcPrincipal& principal, const CRpcContract& contract ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !IsTrustedOrAuthenticated( principal ) )
        return false;
    if ( m_rules.empty() )
        return true;

    // bindable when a rule could allow one of the contract's methods
    for ( UInt32 r=0; r<m_rules.size(); ++r )
    {
        if ( !HasAnyRole( principal, m_rules[ r ].roles ) )
            continue;
        for ( UInt32 s=0; s<contract.GetServiceCount(); ++s )
        {
            const CRpcServiceDescriptor& service = contract.GetServiceAt( s );
            for ( UInt32 m=0; m<service.GetMethodCount(); ++m )
                if ( service.GetMethodAt( m ).GetPath().WildcardEquals( m_rules[ r ].pattern, '*', true ) )
                    return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

TRpcAccessDecision
CRpcRoleAccessPolicy::AuthorizeCall( const CRpcPrincipal& principal, const CRpcContract& contract,
                                     const CRpcServiceDescriptor& service, const CRpcMethodDescriptor& method,
                                     const CRpcMetadata& metadata, CORE::CString& reason ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !IsTrustedOrAuthenticated( principal ) )
    {
        reason = "an authenticated principal is required";
        return RPC_ACCESS_UNAUTHENTICATED;
    }
    if ( m_rules.empty() )
        return RPC_ACCESS_ALLOW;
    for ( UInt32 r=0; r<m_rules.size(); ++r )
    {
        if ( method.GetPath().WildcardEquals( m_rules[ r ].pattern, '*', true ) && HasAnyRole( principal, m_rules[ r ].roles ) )
            return RPC_ACCESS_ALLOW;
    }
    reason = "no rule allows " + method.GetPath() + " for " + principal.subject;
    return RPC_ACCESS_DENY;
}

/*-------------------------------------------------------------------------*/

bool
CRpcRoleAccessPolicy::MayQueryRequest( const CRpcPrincipal& principal, const CORE::CString& ownerSubject, const CRpcRequestId& id ) const
{GUCEF_TRACE;

    return principal.subject == ownerSubject;
}

/*-------------------------------------------------------------------------*/

bool
CRpcRoleAccessPolicy::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    config.SetAttribute( "requireAuthenticated", m_requireAuthenticated );
    CORE::CDataNode* trusted = config.AddChild( "trustedTransports", GUCEF_DATATYPE_ARRAY );
    CORE::CString::StringSet::const_iterator t = m_trustedTransports.begin();
    for ( ; t != m_trustedTransports.end(); ++t )
        trusted->AddValueAsChild( *t );
    CORE::CDataNode* rules = config.AddChild( "rules", GUCEF_DATATYPE_ARRAY );
    for ( UInt32 r=0; r<m_rules.size(); ++r )
    {
        CORE::CDataNode* rule = rules->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        rule->SetAttribute( "path", m_rules[ r ].pattern );
        CORE::CDataNode* roles = rule->AddChild( "roles", GUCEF_DATATYPE_ARRAY );
        CORE::CString::StringSet::const_iterator i = m_rules[ r ].roles.begin();
        for ( ; i != m_rules[ r ].roles.end(); ++i )
            roles->AddValueAsChild( *i );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcRoleAccessPolicy::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_requireAuthenticated = config.GetAttributeValueOrChildValueByName( "requireAuthenticated" ).AsBool( m_requireAuthenticated );
    const CORE::CDataNode* trusted = config.FindChild( "trustedTransports" );
    if ( GUCEF_NULL != trusted )
    {
        m_trustedTransports.clear();
        CORE::CDataNode::const_iterator i = trusted->ConstBegin();
        for ( ; i != trusted->ConstEnd(); ++i )
            m_trustedTransports.insert( (*i)->GetValue().AsString() );
    }
    const CORE::CDataNode* rules = config.FindChild( "rules" );
    if ( GUCEF_NULL != rules )
    {
        m_rules.clear();
        CORE::CDataNode::const_iterator i = rules->ConstBegin();
        for ( ; i != rules->ConstEnd(); ++i )
        {
            SRule rule;
            rule.pattern = (*i)->GetAttributeValue( "path" ).AsString();
            const CORE::CDataNode* roles = (*i)->FindChild( "roles" );
            if ( GUCEF_NULL != roles )
            {
                CORE::CDataNode::const_iterator r = roles->ConstBegin();
                for ( ; r != roles->ConstEnd(); ++r )
                    rule.roles.insert( (*r)->GetValue().AsString() );
            }
            m_rules.push_back( rule );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcRoleAccessPolicy::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcRoleAccessPolicy";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
