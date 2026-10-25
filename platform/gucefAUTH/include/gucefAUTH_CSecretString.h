/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#define GUCEF_AUTH_CSECRETSTRING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Text that must never end up in a log, a message or a file by accident: tokens and other secrets.
 *  The text is not reachable except through Reveal(), there is no conversion to a string, and the stream form
 *  writes "[redacted]" in its place, in log streams too. The memory is overwritten when the secret is cleared,
 *  assigned over or destroyed.
 */
class GUCEF_AUTH_EXPORT_CPP CSecretString
{
    public:

    CSecretString( void );

    explicit CSecretString( const CORE::CString& secret );

    CSecretString( const CSecretString& src );

    ~CSecretString();

    CSecretString& operator=( const CSecretString& src );

    /** The secret itself. Use it only where the secret has to go, never for a message */
    const CORE::CString& Reveal( void ) const;

    void Set( const CORE::CString& secret );

    /** Overwrites the memory of the secret and empties it */
    void Clear( void );

    bool IsEmpty( void ) const;

    /** The number of bytes of the secret */
    UInt32 Length( void ) const;

    /** Same secret, compared in time that does not depend on where they differ */
    bool operator==( const CSecretString& other ) const;

    bool operator!=( const CSecretString& other ) const;

    private:

    void Wipe( void );

    private:

    CORE::CString m_secret;
};

/*-------------------------------------------------------------------------*/

/** Log streams and variant streams get "[redacted]" for a secret that has content and "[empty]" for one that has not */
GUCEF_AUTH_EXPORT_CPP CORE::CVariantStream&
operator<<( CORE::CVariantStream& stream, const CSecretString& secret );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */
