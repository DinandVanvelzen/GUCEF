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

#ifndef GUCEF_LABLINK_CIGATEWAYTOKENS_H
#define GUCEF_LABLINK_CIGATEWAYTOKENS_H

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

#include "gucefLabLink_Store.h"

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

/**
 *  The tokens of the CI gateway on the hub, one per approved machine: 32 random bytes as hex, made when the machine
 *  first asks for its CI configuration and the same on every later request. Kept in "ci-gateway-tokens" of the state
 *  directory (CStore: readable by its owner only, replaced atomically), one "<machine id><TAB><token>" per line.
 *
 *  For the gateway, which runs on the hub box, the hash file (--ci-gateway-tokens, default ci/gateway-tokens of the
 *  state directory) holds the SHA-256 of every token: "<sha256 hex><TAB><machine id>" per line, comments start with
 *  '#'. The gateway reads it with CORE::ReadOwnerOnlyFile(), so it is mode 0600 and, with a gateway user
 *  (--ci-gateway-user) that exists, owned by that user (Linux; given away before it replaces the old file); otherwise
 *  it belongs to the account of the hub. It is replaced atomically whenever a token is made or removed, and written
 *  only when its content differs. Without tokens and without such a file none is made. Its directory is made (0755)
 *  when it is missing and inside the state directory; a directory elsewhere has to exist, and has to let the gateway
 *  user in: that is the decision of the installer.
 *
 *  The service and the command line (--revoke) both change the tokens: every call reads the file again. Thread safe
 *  within the process.
 */
class CCiGatewayTokens
{
    public:

    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CString > TTokenMap;   /**< machine id -> token */

    /** The most machines that have a token */
    static const GUCEF::UInt32 MaxMachines = 256;

    /**
     *  @param hashFilePath     the hash file; empty: ci/gateway-tokens of the state directory
     *  @param gatewayUser      the user the hash file is given to; empty: the account of the hub
     */
    explicit CCiGatewayTokens( CStore& store                                                     ,
                               const GUCEF::CORE::CString& hashFilePath = GUCEF::CORE::CString::Empty ,
                               const GUCEF::CORE::CString& gatewayUser = GUCEF::CORE::CString::Empty  );

    /**
     *  The token of the machine; made when it has none yet.
     *  @param made     whether it is new
     *  @param full     set when there is none because MaxMachines have one
     *  @return false when there is none: too many machines, no randomness, or the file cannot be written; problem says which
     */
    bool GetOrMake( const GUCEF::CORE::CString& machineId, GUCEF::CORE::CString& token, bool& made, bool& full, GUCEF::CORE::CString& problem );

    /** Takes the tokens of the machines away and writes the hash file again; removed is how many there were */
    bool Remove( const GUCEF::CORE::CString::StringVector& machineIds, GUCEF::UInt32& removed, GUCEF::CORE::CString& problem );

    /**
     *  Keeps the tokens of the machines that are approved, takes the others away, and writes the hash file again when
     *  it is not what the tokens give (also when it is missing). removed is how many tokens were taken away
     */
    bool Reconcile( const GUCEF::vector< SApproval >& approvals, GUCEF::UInt32& removed, GUCEF::CORE::CString& problem );

    /** The tokens of the file; an empty map when there is no file */
    bool Load( TTokenMap& tokens, GUCEF::CORE::CString& problem ) const;

    /** The hash file: the one given, else <state dir>/ci/gateway-tokens */
    GUCEF::CORE::CString GetHashFilePath( void ) const;

    /** What the hash file holds for the tokens */
    static GUCEF::CORE::CString HashFileContent( const TTokenMap& tokens );

    private:

    CCiGatewayTokens( const CCiGatewayTokens& src );
    CCiGatewayTokens& operator=( const CCiGatewayTokens& src );

    bool Save( const TTokenMap& tokens, GUCEF::CORE::CString& problem );

    bool WriteHashFile( const TTokenMap& tokens, GUCEF::CORE::CString& problem );

    /** Logs a problem with the hash file the first time it occurs */
    void ReportOnce( const GUCEF::CORE::CString& problem, bool error );

    CStore& m_store;
    GUCEF::CORE::CString m_hashFilePath;
    GUCEF::CORE::CString m_gatewayUser;
    GUCEF::CORE::CString::StringSet m_reported;     /**< the problems with the hash file that were logged */
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_CIGATEWAYTOKENS_H ? */
