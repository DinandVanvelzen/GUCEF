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

#ifndef GUCEF_DRN_CIDRNACCESSPOLICY_H
#define GUCEF_DRN_CIDRNACCESSPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

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

class CDRNPeerLink;
class CDRNAuthResult;

/**
 *  Decides what a member of a session may do. Every join, subscribe and write is checked.
 *  Peers are identified by their peer id in the session; the link gives access to what its authentication
 *  established (user id, role).
 *
 *  DRN asks MaySubscribeAs() and MayWriteAs(), which carry the authentication of the member (its role); by default
 *  they ask MaySubscribe() and MayWrite(). A policy that enforces roles overrides them.
 */
class GUCEF_DRN_EXPORT_CPP CIDRNAccessPolicy
{
    public:

    CIDRNAccessPolicy( void );

    virtual ~CIDRNAccessPolicy();

    virtual bool MayJoin( const CDRNPeerLink& link, const CORE::CString& sessionName ) = 0;

    virtual bool MaySubscribe( UInt32 peerId, const CORE::CString& itemName ) = 0;

    virtual bool MayWrite( UInt32 peerId, const CORE::CString& itemName, UInt32 keyIndex ) = 0;

    /** Subscribing with what the member's authentication established, by default MaySubscribe() */
    virtual bool MaySubscribeAs( UInt32 peerId, const CDRNAuthResult& auth, const CORE::CString& itemName );

    /** Writing with what the member's authentication established, by default MayWrite() */
    virtual bool MayWriteAs( UInt32 peerId, const CDRNAuthResult& auth, const CORE::CString& itemName, UInt32 keyIndex );

    private:

    CIDRNAccessPolicy( const CIDRNAccessPolicy& src );
    CIDRNAccessPolicy& operator=( const CIDRNAccessPolicy& src );
};

/*-------------------------------------------------------------------------*/

/**
 *  The default policy: everyone may join and subscribe, writes are left to the authority modes of the items.
 *  Roles are enforced when they are set: only links whose authentication established one of the join roles may
 *  join, only members with one of the write roles may write, only members with one of the subscribe roles may
 *  subscribe. Empty sets (the default) allow every role, also none.
 */
class GUCEF_DRN_EXPORT_CPP CDRNDefaultAccessPolicy : public CIDRNAccessPolicy
{
    public:

    CDRNDefaultAccessPolicy( void );

    virtual ~CDRNDefaultAccessPolicy();

    void SetJoinRoles( const CORE::CString::StringSet& roles );
    const CORE::CString::StringSet& GetJoinRoles( void ) const;

    void SetSubscribeRoles( const CORE::CString::StringSet& roles );
    const CORE::CString::StringSet& GetSubscribeRoles( void ) const;

    void SetWriteRoles( const CORE::CString::StringSet& roles );
    const CORE::CString::StringSet& GetWriteRoles( void ) const;

    virtual bool MayJoin( const CDRNPeerLink& link, const CORE::CString& sessionName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MaySubscribe( UInt32 peerId, const CORE::CString& itemName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MayWrite( UInt32 peerId, const CORE::CString& itemName, UInt32 keyIndex ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MaySubscribeAs( UInt32 peerId, const CDRNAuthResult& auth, const CORE::CString& itemName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool MayWriteAs( UInt32 peerId, const CDRNAuthResult& auth, const CORE::CString& itemName, UInt32 keyIndex ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CString::StringSet m_joinRoles;
    CORE::CString::StringSet m_subscribeRoles;
    CORE::CString::StringSet m_writeRoles;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CIDRNACCESSPOLICY_H ? */
