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

    /**
     *  Reading a resource of a provider this node serves (CDRNResourceSessionComponent). 'sharedByProvider' is what the
     *  share rules the provider was registered with decided (CDRNResourceShareRules: role and subtree). Asked before
     *  the name is resolved, so a refusal says nothing about whether it exists.
     *  By default the share rules decide: nothing is readable unless the application shared it
     */
    virtual bool MayReadResourceAs( UInt32 peerId, const CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                    bool sharedByProvider );

    /**
     *  Reading a resource, with the reason of a refusal: what the component asks. A policy that has something to say to the
     *  member about a refusal overrides this and sets 'refusal' (plain words, shown to the member); the default asks
     *  MayReadResourceAs() and leaves 'refusal' empty, which makes the component say that the resource is not shared with
     *  the member.
     */
    virtual bool CheckReadResourceAs( UInt32 peerId, const CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                      bool sharedByProvider, CORE::CString& refusal );

    /**
     *  Reading a resource, with the reason of a refusal and whether the refusal is temporary: what the component asks. When
     *  'retryLater' is set the member is not refused: it is told that the resource is busy (the BUSY status, with 'refusal' as
     *  the words) and the client asks again. A policy uses it for a condition that mends by itself, such as data it has not
     *  got yet and is getting; a refusal that does not mend stays a refusal. The default sets 'retryLater' to false and asks
     *  CheckReadResourceAs(), so a policy that does not override this behaves as before.
     */
    virtual bool DecideReadResourceAs( UInt32 peerId, const CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                       bool sharedByProvider, CORE::CString& refusal, bool& retryLater );

    /**
     *  Seeing a name of a share through the namespace operations (STAT, and LIST of a directory): whether it exists and
     *  what it is, not its bytes. 'resourceName' is relative to the share, the root of a share is the empty name;
     *  'sharedByProvider' is what the share rules decided (role, and a subtree of the share: a name above a shared
     *  subtree may be browsed, only what lies in it may be read). Asked before anything is resolved, so a refusal says
     *  nothing about what exists. By default the same as MayReadResourceAs(): a policy that restricts what is read
     *  restricts what is seen as well; override it to let a role see names and sizes without being able to read.
     */
    virtual bool MayBrowseResourceAs( UInt32 peerId, const CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& resourceName,
                                      bool sharedByProvider );

    /**
     *  Hides an entry of a share: asked for every entry of a listing, for the names of STAT and OPEN and for every
     *  directory above them. A hidden entry is left out of listings and is NOT_FOUND to STAT and OPEN, as if it did not
     *  exist. 'entryName' is relative to the share. Default: nothing is hidden.
     */
    virtual bool MayListEntryAs( UInt32 peerId, const CDRNAuthResult& auth, const CORE::CString& providerName, const CORE::CString& entryName,
                                 bool isDirectory );

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
