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

#ifndef GUCEF_LABLINK_WEBSETTINGS_H
#define GUCEF_LABLINK_WEBSETTINGS_H

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The port of the website of the hub */
static const GUCEF::UInt16 DefaultWebPort = 47813;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The cost of a password hash; the record of a password carries the values it was made with */
struct SPasswordParams
{
    GUCEF::CORE::CString algorithm;             /**< "argon2id" or "pbkdf2-sha256" */
    GUCEF::UInt32 memoryInKiB;                  /**< argon2id */
    GUCEF::UInt32 iterations;                   /**< argon2id: passes over the memory; pbkdf2-sha256: iterations */
    GUCEF::UInt32 lanes;                        /**< argon2id */

    SPasswordParams( void )
        : algorithm( "argon2id" ), memoryInKiB( 65536 ), iterations( 3 ), lanes( 1 )
    {}
};

/**
 *  The website of the hub (cluster role): off unless enabled; see the README of the tool. The defaults are those
 *  the service runs with; the tests make the hash cheap and the times short.
 */
struct SWebSettings
{
    bool enabled;                               /**< --web */
    GUCEF::UInt16 port;                         /**< --web-port */
    GUCEF::CORE::CString listenAddress;         /**< --web-listen: a dotted IPv4 address; empty: every address */
    GUCEF::CORE::CString certificateFile;       /**< --web-cert: empty: tls/tls.crt of the MCP directory when it is there */
    GUCEF::CORE::CString privateKeyFile;        /**< --web-key: empty: tls/tls.key of the MCP directory when it is there */
    bool allowPlainHttp;                        /**< --web-allow-plain-http: serve without TLS when there is no certificate */
    GUCEF::CORE::CString tokenFile;             /**< --web-token-file: the bearer token, made for the owner when missing; empty: no token */

    GUCEF::UInt32 sessionIdleTimeoutInSecs;     /**< a session nobody used for this long ends */
    GUCEF::UInt32 sessionLifetimeInSecs;        /**< a session ends this long after the login, used or not */
    GUCEF::UInt32 staySignedInLifetimeInSecs;   /**< the same for a login with "stay signed in", whose cookie outlives the browser */
    GUCEF::UInt32 maxSessions;
    GUCEF::UInt32 maxSessionsPerUser;
    GUCEF::UInt32 minPasswordLength;
    GUCEF::UInt32 maxConcurrentHashes;          /**< password hashes that run at the same time, each on a thread of the pool */
    GUCEF::UInt32 maxQueuedHashes;              /**< logins that wait for a hash; more are answered with 503 */
    GUCEF::UInt32 maxConnections;
    GUCEF::UInt32 maxThrottleEntries;           /**< addresses and user names whose failed logins are counted, each */
    GUCEF::UInt32 maxAddressDelayInSecs;        /**< the longest wait an address is given after failed logins */
    GUCEF::UInt32 maxUserDelayInSecs;           /**< the same for a user name: short, so nobody can keep a user out */
    SPasswordParams passwordParams;             /**< what new password hashes are made with */

    SWebSettings( void )
        : enabled( false )
        , port( DefaultWebPort )
        , listenAddress()
        , certificateFile()
        , privateKeyFile()
        , allowPlainHttp( false )
        , tokenFile()
        , sessionIdleTimeoutInSecs( 8 * 3600 )
        , sessionLifetimeInSecs( 24 * 3600 )
        , staySignedInLifetimeInSecs( 30 * 24 * 3600 )
        , maxSessions( 256 )
        , maxSessionsPerUser( 16 )
        , minPasswordLength( 12 )
        , maxConcurrentHashes( 2 )
        , maxQueuedHashes( 16 )
        , maxConnections( 32 )
        , maxThrottleEntries( 1024 )
        , maxAddressDelayInSecs( 900 )
        , maxUserDelayInSecs( 30 )
        , passwordParams()
    {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_WEBSETTINGS_H ? */
