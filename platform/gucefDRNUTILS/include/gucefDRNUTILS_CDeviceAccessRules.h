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

#ifndef GUCEF_DRNUTILS_CDEVICEACCESSRULES_H
#define GUCEF_DRNUTILS_CDEVICEACCESSRULES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COMCORE_CIRPCACCESSPOLICY_H
#include "gucefCOMCORE_CIRpcAccessPolicy.h"
#define GUCEF_COMCORE_CIRPCACCESSPOLICY_H
#endif /* GUCEF_COMCORE_CIRPCACCESSPOLICY_H ? */

#ifndef GUCEF_COMCORE_CRPCPRINCIPAL_H
#include "gucefCOMCORE_CRpcPrincipal.h"
#define GUCEF_COMCORE_CRPCPRINCIPAL_H
#endif /* GUCEF_COMCORE_CRPCPRINCIPAL_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEREGISTRY_H
#include "gucefDRNUTILS_CDeviceRegistry.h"
#define GUCEF_DRNUTILS_CDEVICEREGISTRY_H
#endif /* GUCEF_DRNUTILS_CDEVICEREGISTRY_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The per-method access policy of a remote service over its device registry: a rule table of method path patterns
 *  ("contract.Service/Method", "contract.Service/*", "contract.*", "*"; an exact path wins over the longest pattern) with
 *  the roles and the device kinds each allows (an empty kind set allows every kind). A method without a rule is
 *  refused (deny by default).
 *  The caller is the device of the principal's subject ("drn:<deviceId>"); its roles and kind are read from the registry
 *  at every call, so a role change or a revocation applies at once; the roles a token carries are not used. A refusal's
 *  reason starts with the detail code (CRemoteServiceDetailCodes: ROLE_REQUIRED, DEVICE_KIND, DEVICE_NOT_ACTIVE,
 *  DEVICE_UNKNOWN, NO_RULE), because CIRpcAccessPolicy::AuthorizeCall() returns only a reason text.
 *  Principals of a trusted transport (SetTrustedTransports(), empty by default; "loopback" for in-process use) are
 *  allowed every call. Contracts may be bound when a rule could allow one of their methods to the caller.
 *  DURABLE request ids are visible to their own subject only. Thread safe; called on the protocol threads.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDeviceAccessRules : public COMCORE::CIRpcAccessPolicy
{
    public:

    static const char* const SubjectPrefix;     /**< "drn:" */

    explicit CDeviceAccessRules( const CDeviceRegistry& registry );
    virtual ~CDeviceAccessRules() GUCEF_VIRTUAL_OVERRIDE;

    /** A rule; a second rule of the same pattern replaces the first */
    void AddRule( const CORE::CString& pathPattern, const CORE::CString::StringSet& roles, const CORE::CString::StringSet& kinds );

    /** AddRule() with comma separated lists ("viewer,curator", "" for every kind) */
    void AddRule( const CORE::CString& pathPattern, const CORE::CString& rolesCsv, const CORE::CString& kindsCsv = CORE::CString::Empty );

    bool RemoveRule( const CORE::CString& pathPattern );
    void ClearRules( void );
    UInt32 GetRuleCount( void ) const;

    void SetTrustedTransports( const CORE::CString::StringSet& transports );

    /** The device id of a principal of this policy's form, empty for another */
    static CORE::CString DeviceIdOfPrincipal( const COMCORE::CRpcPrincipal& principal );

    /**
     *  The decision for a method path ("gucef.drnutils.session.Session/Hello"): true when allowed; otherwise the reason
     *  with the detail code first. 'device' (may be GUCEF_NULL) receives the caller's record when it is known
     */
    bool Authorize( const COMCORE::CRpcPrincipal& principal, const CORE::CString& methodPath, CORE::CString& reason, CDeviceRecord* device = GUCEF_NULL ) const;

    virtual bool MayBindContract( const COMCORE::CRpcPrincipal& principal, const COMCORE::CRpcContract& contract ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual COMCORE::TRpcAccessDecision AuthorizeCall( const COMCORE::CRpcPrincipal& principal       ,
                                                       const COMCORE::CRpcContract& contract         ,
                                                       const COMCORE::CRpcServiceDescriptor& service ,
                                                       const COMCORE::CRpcMethodDescriptor& method   ,
                                                       const COMCORE::CRpcMetadata& metadata         ,
                                                       CORE::CString& reason                         ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MayQueryRequest( const COMCORE::CRpcPrincipal& principal, const CORE::CString& ownerSubject, const COMCORE::CRpcRequestId& id ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Refusals since construction */
    UInt32 GetRefusedCount( void ) const;

    private:

    struct SRule
    {
        CORE::CString pattern;
        CORE::CString::StringSet roles;
        CORE::CString::StringSet kinds;
    };
    typedef GUCEF::vector< SRule > TRuleVector;

    CDeviceAccessRules( const CDeviceAccessRules& src );                /**< not implemented */
    CDeviceAccessRules& operator=( const CDeviceAccessRules& src );     /**< not implemented */

    const SRule* FindRule( const CORE::CString& methodPath ) const;
    bool IsTrusted( const COMCORE::CRpcPrincipal& principal ) const;

    const CDeviceRegistry& m_registry;
    mutable MT::CMutex m_lock;
    TRuleVector m_rules;
    CORE::CString::StringSet m_trustedTransports;
    mutable UInt32 m_refused;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDEVICEACCESSRULES_H ? */
