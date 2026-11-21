/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_WEBSESSIONS_H
#define GUCEF_LABLINK_WEBSESSIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A login of the website */
struct SWebSession
{
    GUCEF::CORE::CString user;                  /**< the user name; empty for a login with the token */
    bool viaToken;
    bool persistent;                            /**< "stay signed in": the cookie outlives the browser */
    GUCEF::UInt64 createdInMs;                  /**< monotonic */
    GUCEF::UInt64 lastUsedInMs;
    GUCEF::UInt64 endsInMs;                     /**< the absolute end */
    GUCEF::UInt32 idleTimeoutInMs;

    SWebSession( void ) : user(), viaToken( false ), persistent( false ), createdInMs( 0 ), lastUsedInMs( 0 ), endsInMs( 0 ), idleTimeoutInMs( 0 ) {}
};

/**
 *  The sessions of the website, in memory only: a restart of the hub means logging in again. A session id is 32
 *  random bytes as hex; the table keys sessions by the SHA-256 of their id, so neither a lookup nor a dump of the
 *  table gives an id away. Bounded: at most maxPerUser sessions per user and maxSessions in all; a new session takes
 *  the place of the one used longest ago, of the same user first. Times are passed in, monotonic milliseconds.
 *  Used on one thread, that of the website.
 */
class CWebSessionTable
{
    public:

    CWebSessionTable( GUCEF::UInt32 maxSessions, GUCEF::UInt32 maxPerUser );

    /** @return false without randomness; id is the new session id */
    bool Create( const GUCEF::CORE::CString& user, bool viaToken, bool persistent, GUCEF::UInt64 nowInMs,
                 GUCEF::UInt32 idleTimeoutInMs, GUCEF::UInt64 lifetimeInMs, GUCEF::CORE::CString& id );

    /** Finds a session that has not ended and counts it as used now (the idle timeout slides) */
    bool Use( const GUCEF::CORE::CString& id, GUCEF::UInt64 nowInMs, SWebSession& session );

    bool Remove( const GUCEF::CORE::CString& id );

    /** Ends every session of the user. @return how many */
    GUCEF::UInt32 RemoveUser( const GUCEF::CORE::CString& user );

    /** Ends the sessions that are idle too long or past their end. @return how many */
    GUCEF::UInt32 Sweep( GUCEF::UInt64 nowInMs );

    GUCEF::UInt32 GetCount( void ) const;

    GUCEF::UInt32 GetCountOfUser( const GUCEF::CORE::CString& user ) const;

    private:

    typedef GUCEF::map< GUCEF::CORE::CString, SWebSession > TSessionMap;

    static GUCEF::CORE::CString KeyOf( const GUCEF::CORE::CString& id );
    static bool HasEnded( const SWebSession& session, GUCEF::UInt64 nowInMs );
    void RemoveLeastRecentlyUsed( const GUCEF::CORE::CString* user );

    TSessionMap m_sessions;
    GUCEF::UInt32 m_maxSessions;
    GUCEF::UInt32 m_maxPerUser;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_WEBSESSIONS_H ? */
