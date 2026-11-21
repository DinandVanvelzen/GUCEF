/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CIDRNPEERLIMITPOLICY_H
#define GUCEF_DRN_CIDRNPEERLIMITPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CDRNPEERLIMITS_H
#include "gucefDRN_CDRNPeerLimits.h"
#define GUCEF_DRN_CDRNPEERLIMITS_H
#endif /* GUCEF_DRN_CDRNPEERLIMITS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNAuthResult;

/**
 *  Decides the limits of one member of a session on the serving side of the resource transfers, from the per-member
 *  values that are configured (CDRNResourceTransferSettings). It is the place where a later policy can give a member more
 *  or less than the configured values (a credit for those that share more, a penalty, a role): the component
 *  (CDRNResourceSessionComponent::SetPeerLimitPolicy()) asks it per member and keeps the answer for a short time, it
 *  is not asked per message or per chunk.
 *
 *  Only the per-member limits pass through here. The server-wide limits (those of all members together) are never changed
 *  by a policy: a member is held to the lower of its effective limit and the server-wide one.
 *
 *  The default gives every member the configured values unchanged. The policy is called under the node lock, on the
 *  node's thread: it must be quick and must not call into the node.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNPeerLimitPolicy
{
    public:

    CIDRNPeerLimitPolicy( void );

    virtual ~CIDRNPeerLimitPolicy();

    /**
     *  The limits that apply to the member 'peerId' (with what its authentication established) when its configured
     *  limits are 'configured'. The answer goes into 'effective', which holds 'configured' when called.
     *  0 is no maximum. Default: nothing changes.
     */
    virtual void GetEffectiveLimits( UInt32 peerId, const CDRNAuthResult& auth, const CDRNPeerLimits& configured, CDRNPeerLimits& effective );

    private:

    CIDRNPeerLimitPolicy( const CIDRNPeerLimitPolicy& src );
    CIDRNPeerLimitPolicy& operator=( const CIDRNPeerLimitPolicy& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNPEERLIMITPOLICY_H ? */
