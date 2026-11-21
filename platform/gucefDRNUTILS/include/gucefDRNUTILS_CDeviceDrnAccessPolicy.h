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

#ifndef GUCEF_DRNUTILS_CDEVICEDRNACCESSPOLICY_H
#define GUCEF_DRNUTILS_CDEVICEDRNACCESSPOLICY_H

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

#ifndef GUCEF_DRN_CIDRNACCESSPOLICY_H
#include "gucefDRN_CIDRNAccessPolicy.h"
#define GUCEF_DRN_CIDRNACCESSPOLICY_H
#endif /* GUCEF_DRN_CIDRNACCESSPOLICY_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEREGISTRY_H
#include "gucefDRNUTILS_CDeviceRegistry.h"
#define GUCEF_DRNUTILS_CDEVICEREGISTRY_H
#endif /* GUCEF_DRNUTILS_CDEVICEREGISTRY_H ? */

#ifndef GUCEF_DRNUTILS_CIREMOTESHAREDECISION_H
#include "gucefDRNUTILS_CIRemoteShareDecision.h"
#define GUCEF_DRNUTILS_CIREMOTESHAREDECISION_H
#endif /* GUCEF_DRNUTILS_CIREMOTESHAREDECISION_H ? */

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
 *  The DRN access policy of a remote service's session: a member may join only when the user id of its link's
 *  authentication is a device of the registry that is ACTIVE with at least one role, and the public key of its token
 *  equals the registered key of the device (compared in constant time). Subscriptions to published items follow
 *  item name patterns ("svc/jobs", "svc/*", "*") with the roles that may subscribe (from the registry, at every
 *  subscription); an item without a rule is refused. Members never write (every MayWrite is refused): what a client
 *  changes goes through RPC commands. Called on the node thread; thread safe.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDeviceDrnAccessPolicy : public DRN::CIDRNAccessPolicy
{
    public:

    explicit CDeviceDrnAccessPolicy( const CDeviceRegistry& registry );
    virtual ~CDeviceDrnAccessPolicy() GUCEF_VIRTUAL_OVERRIDE;

    /** Members with one of the roles may subscribe to items whose name matches the pattern; the same pattern again replaces it */
    void AddSubscribeRule( const CORE::CString& itemPattern, const CORE::CString::StringSet& roles );
    void ClearSubscribeRules( void );

    virtual bool MayJoin( const DRN::CDRNPeerLink& link, const CORE::CString& sessionName ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool MaySubscribe( UInt32 peerId, const CORE::CString& itemName ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool MayWrite( UInt32 peerId, const CORE::CString& itemName, UInt32 keyIndex ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool MaySubscribeAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& itemName ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool MayWriteAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& itemName, UInt32 keyIndex ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Resource shares (the providers of CRemoteServiceHost::RegisterShare()): for a provider name that is a share, every hook of the
     *  node's resource facility resolves the member to its device in the registry at that call (the user id of the authentication, not
     *  the roles of the token): the device has to be ACTIVE and have one of the roles of the share ('*': any role), then the share rules of the
     *  facility have to share the name, then the decision of the share has the last word. A provider name that is no share is left to the
     *  facility's defaults. The decision is not owned; RemoveShare() returns after the calls under way into it.
     */
    void SetShare( const CORE::CString& share, const CORE::CString::StringSet& roles, CIRemoteShareDecision* decision );
    bool RemoveShare( const CORE::CString& share );
    bool HasShare( const CORE::CString& share ) const;

    virtual bool MayReadResourceAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                    bool sharedByProvider ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool CheckReadResourceAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                      bool sharedByProvider, CORE::CString& refusal ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool DecideReadResourceAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                       bool sharedByProvider, CORE::CString& refusal, bool& retryLater ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool MayBrowseResourceAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                      bool sharedByProvider ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool MayListEntryAs( UInt32 peerId, const DRN::CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& entryName,
                                 bool isDirectory ) GUCEF_VIRTUAL_OVERRIDE;

    /** The join decision on its own (tests): the device of the user id with the key of the token */
    bool MayJoinAs( const DRN::CDRNAuthResult& auth, CORE::CString& reason ) const;

    UInt32 GetRefusedJoinCount( void ) const;

    private:

    struct SRule
    {
        CORE::CString pattern;
        CORE::CString::StringSet roles;
    };
    typedef GUCEF::vector< SRule > TRuleVector;

    struct SShare
    {
        CORE::CString::StringSet roles;
        CIRemoteShareDecision* decision;
    };
    typedef GUCEF::map< CORE::CString, SShare > TShareMap;

    /** The device of the member now, when it is active and has a role of the share */
    bool ResolveShareDevice( const DRN::CDRNAuthResult& auth, const SShare& share, CDeviceRecord& device ) const;

    CDeviceDrnAccessPolicy( const CDeviceDrnAccessPolicy& src );                /**< not implemented */
    CDeviceDrnAccessPolicy& operator=( const CDeviceDrnAccessPolicy& src );     /**< not implemented */

    const CDeviceRegistry& m_registry;
    mutable MT::CMutex m_lock;
    TRuleVector m_rules;
    mutable UInt32 m_refusedJoins;
    mutable MT::CMutex m_shareLock;                 /**< held while a hook calls a decision */
    TShareMap m_shares;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDEVICEDRNACCESSPOLICY_H ? */
