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

#ifndef GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H
#define GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_COMCORE_CIRPCACCESSPOLICY_H
#include "gucefCOMCORE_CIRpcAccessPolicy.h"
#define GUCEF_COMCORE_CIRPCACCESSPOLICY_H
#endif /* GUCEF_COMCORE_CIRPCACCESSPOLICY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The default access policy (gucefCOMCORE RPC design 10.7): rules from configuration map method path patterns
 *  ("invest.desk.Commands/*", "gucef.sample.notes.Notes/Get", "*") to role sets. Without rules every call is allowed;
 *  with rules a call needs a rule that matches its path and a role of the principal in it (deny by default).
 *  requireAuthenticated (default true) refuses unauthenticated principals on every transport except those listed as
 *  trusted (default "loopback"). Contracts may be bound when some rule could allow one of their methods.
 *
 *  Configuration: { "requireAuthenticated": true, "trustedTransports": [ "loopback" ],
 *                   "rules": [ { "path": "gucef.sample.notes.Notes/*", "roles": [ "reader" ] } ] }
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcRoleAccessPolicy : public CIRpcAccessPolicy ,
                                                      public CORE::CIConfigurable
{
    public:

    CRpcRoleAccessPolicy( void );
    virtual ~CRpcRoleAccessPolicy() GUCEF_VIRTUAL_OVERRIDE;

    /** A rule: principals with one of the roles may call methods whose path matches the pattern ("*" wildcards) */
    void AddRule( const CORE::CString& pathPattern, const CORE::CString::StringSet& roles );
    void ClearRules( void );
    void SetRequireAuthenticated( bool require );
    void SetTrustedTransports( const CORE::CString::StringSet& transports );

    virtual bool MayBindContract( const CRpcPrincipal& principal, const CRpcContract& contract ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual TRpcAccessDecision AuthorizeCall( const CRpcPrincipal& principal, const CRpcContract& contract,
                                              const CRpcServiceDescriptor& service, const CRpcMethodDescriptor& method,
                                              const CRpcMetadata& metadata, CORE::CString& reason ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool MayQueryRequest( const CRpcPrincipal& principal, const CORE::CString& ownerSubject, const CRpcRequestId& id ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SRule
    {
        CORE::CString pattern;
        CORE::CString::StringSet roles;
    };
    typedef GUCEF::vector< SRule > TRuleVector;

    bool IsTrustedOrAuthenticated( const CRpcPrincipal& principal ) const;
    static bool HasAnyRole( const CRpcPrincipal& principal, const CORE::CString::StringSet& roles );

    mutable MT::CMutex m_lock;
    TRuleVector m_rules;
    bool m_requireAuthenticated;
    CORE::CString::StringSet m_trustedTransports;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H ? */
