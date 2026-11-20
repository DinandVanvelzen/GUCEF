/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_MCPCI_CGATEWAYTOKENS_H
#define GUCEF_MCPCI_CGATEWAYTOKENS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The tokens a gateway accepts, each for one machine:
 *
 *    - the tokens file (--gateway-tokens-file) that the lablink hub keeps: one line per machine,
 *      "<SHA-256 of the token as 64 hexadecimal digits><TAB><machine id>", lines starting with # are comments.
 *      It is read through CORE::ReadOwnerOnlyFile() and read again when its time or size changes, and at least
 *      every 30 seconds, so that a machine that is revoked (its line removed) is refused from then on. A file that
 *      cannot be read accepts nobody.
 *    - optionally one static token (--token-file), for the administrator, as machine "admin"
 *
 *  Clients send "Authorization: Bearer <token>". The given token is hashed and its digest compared with every
 *  digest in constant time, so neither the time nor the logs tell anything about the tokens. Tokens are never kept,
 *  only their digests.
 *
 *  Thread-safe.
 */
class CGatewayTokens
{
    public:

    /** The machine id of the static token */
    static const char* AdminMachineId;

    CGatewayTokens( void );

    /** The tokens file; it is read at once. @return false with the reason when it cannot be read now */
    bool SetTokensFile( const GUCEF::CORE::CString& path, GUCEF::CORE::CString& problem );

    /** The static token, empty for none; only its digest is kept */
    void SetStaticToken( const GUCEF::CORE::CString& token );

    /** How long the tokens file is trusted before its modification time is looked at again; default 1000 */
    void SetRecheckInterval( GUCEF::UInt32 intervalInMs );

    /**
     *  Checks the value of an Authorization header.
     *  @param machineId    the machine of the token when it is accepted
     *  @return whether the header carries an accepted bearer token
     */
    bool Authenticate( const GUCEF::CORE::CString& authorizationHeader, GUCEF::CORE::CString& machineId );

    /** The number of machines of the tokens file, after the last read */
    GUCEF::UInt32 GetMachineCount( void );

    bool HasStaticToken( void ) const;

    /** The SHA-256 of a token as 64 lower case hexadecimal digits, the form of the tokens file */
    static GUCEF::CORE::CString DigestHex( const GUCEF::CORE::CString& token );

    private:

    class CEntry
    {
        public:

        GUCEF::UInt8 digest[ 32 ];
        GUCEF::CORE::CString machineId;

        CEntry( void );
    };
    typedef GUCEF::vector< CEntry > TEntryVector;

    /** Reads the tokens file again when it changed; with the lock held */
    void ReloadIfChanged( GUCEF::UInt64 nowInMs );

    /** Reads the tokens file; with the lock held */
    bool Load( GUCEF::CORE::CString& problem );

    static bool ParseHexDigest( const GUCEF::CORE::CString& hex, GUCEF::UInt8* digest );

    GUCEF::CORE::CString m_tokensFile;
    TEntryVector m_entries;
    GUCEF::UInt64 m_fileTimeInMs;
    GUCEF::UInt64 m_fileSize;
    GUCEF::UInt64 m_readAtInMs;
    GUCEF::UInt64 m_checkedAtInMs;
    GUCEF::UInt32 m_recheckIntervalInMs;
    bool m_hasStaticToken;
    GUCEF::UInt8 m_staticDigest[ 32 ];
    bool m_loadFailed;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CGATEWAYTOKENS_H ? */
