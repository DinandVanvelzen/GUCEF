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

#ifndef GUCEF_LABLINK_PASSWORDHASH_H
#define GUCEF_LABLINK_PASSWORDHASH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#include "gucefLabLink_WebSettings.h"

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
 *  The records of passwords: a hash with its own random salt and the parameters it was made with, as text
 *    $argon2id$m=<memory KiB>,t=<passes>,p=<lanes>$<salt>$<hash>
 *    $pbkdf2-sha256$i=<iterations>$<salt>$<hash>
 *  with the salt (16 bytes) and the hash (32 bytes) in base64url without padding. Argon2id is the key derivation codec
 *  "argon2id" of codecspluginMONOCYPHER; PBKDF2-HMAC-SHA256 of gucefCORE is the fallback when the codec is missing.
 *  A password is never kept: only its record.
 *
 *  The functions take as long as the parameters say (tenths of a second by default) and run on the calling thread:
 *  the website calls them from a task of a thread pool, the command line on its own thread.
 */
class CPasswordHash
{
    public:

    static const char* Argon2id;                /**< "argon2id" */
    static const char* Pbkdf2Sha256;            /**< "pbkdf2-sha256" */
    static const GUCEF::UInt32 SaltSize = 16;
    static const GUCEF::UInt32 HashSize = 32;
    static const GUCEF::UInt32 Pbkdf2Iterations = 600000;

    /** Whether the argon2id codec is loaded */
    static bool IsArgon2idAvailable( void );

    /** The parameters asked for, with PBKDF2 in place of argon2id when that codec is missing */
    static SPasswordParams Effective( const SPasswordParams& wanted );

    /** A record of the password with a new random salt. @return false without randomness or the algorithm */
    static bool MakeRecord( const GUCEF::CORE::CString& password, const SPasswordParams& params, GUCEF::CORE::CString& record );

    /**
     *  Whether the password is the one of the record; the comparison takes the same time wherever the hashes differ.
     *  @param needsUpgrade set when the record was made with other parameters than 'current'
     */
    static bool Verify( const GUCEF::CORE::CString& password, const GUCEF::CORE::CString& record, const SPasswordParams& current, bool& needsUpgrade );

    /** A record no password matches, with the cost of 'params': verified against for user names that do not exist */
    static GUCEF::CORE::CString MakeDummyRecord( const SPasswordParams& params );

    /** Reads a record. @return false when it is not one */
    static bool ParseRecord( const GUCEF::CORE::CString& record, SPasswordParams& params, GUCEF::CORE::CDynamicBuffer& salt, GUCEF::CORE::CDynamicBuffer& hash );

    /** Writes over the characters of the text, as far as this program can */
    static void Wipe( GUCEF::CORE::CString& text );

    private:

    static bool Derive( const GUCEF::CORE::CString& password, const SPasswordParams& params, const void* salt, GUCEF::UInt32 saltSize, GUCEF::UInt8* hash );
    static GUCEF::CORE::CString Format( const SPasswordParams& params, const void* salt, const void* hash );

    CPasswordHash( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_PASSWORDHASH_H ? */
