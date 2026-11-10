/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CPBKDF2HMACSHA256_H
#define GUCEF_CORE_CPBKDF2HMACSHA256_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  PBKDF2 with HMAC-SHA256 as its pseudo random function (RFC 8018 section 5.2), for keys and verifiers derived
 *  from a password: SCRAM-SHA-256 (RFC 7677), approval PINs and the like. The key schedule of the password is
 *  computed once per call, every iteration costs two SHA-256 compressions.
 *
 *  The number of iterations is the caller's choice and its whole defence against guessing; a password verifier
 *  stored on disk wants several hundred thousand (OWASP recommends 600 000 for PBKDF2-HMAC-SHA256).
 */
class GUCEF_CORE_PUBLIC_CPP CPbkdf2HmacSha256
{
    public:

    /**
     *  Derives 'outputSize' bytes from the password and the salt. Any UInt32 size is within the RFC 8018 limit of
     *  (2^32 - 1) blocks of 32 bytes.
     *  The intermediate values are wiped before the call returns.
     *  @return false when 'iterations' is 0, 'outputSize' is 0 or 'output' is GUCEF_NULL; nothing is written then
     */
    static bool Derive( const void* password ,
                        UInt32 passwordSize  ,
                        const void* salt     ,
                        UInt32 saltSize      ,
                        UInt32 iterations    ,
                        UInt8* output        ,
                        UInt32 outputSize    );

    private:

    CPbkdf2HmacSha256( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CPBKDF2HMACSHA256_H ? */
