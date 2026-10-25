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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_CSecretString.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSecretString::CSecretString( void )
    : m_secret()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSecretString::CSecretString( const CORE::CString& secret )
    : m_secret( secret )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSecretString::CSecretString( const CSecretString& src )
    : m_secret( src.m_secret )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSecretString::~CSecretString()
{GUCEF_TRACE;

    Wipe();
}

/*-------------------------------------------------------------------------*/

CSecretString&
CSecretString::operator=( const CSecretString& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        Wipe();
        m_secret = src.m_secret;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CSecretString::Wipe( void )
{GUCEF_TRACE;

    UInt32 size = GetByteLength( m_secret );
    if ( size > 0 )
        CAuthCrypto::Wipe( m_secret.C_String(), size );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSecretString::Reveal( void ) const
{GUCEF_TRACE;

    return m_secret;
}

/*-------------------------------------------------------------------------*/

void
CSecretString::Set( const CORE::CString& secret )
{GUCEF_TRACE;

    Wipe();
    m_secret = secret;
}

/*-------------------------------------------------------------------------*/

void
CSecretString::Clear( void )
{GUCEF_TRACE;

    Wipe();
    m_secret.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CSecretString::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_secret.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

UInt32
CSecretString::Length( void ) const
{GUCEF_TRACE;

    return GetByteLength( m_secret );
}

/*-------------------------------------------------------------------------*/

bool
CSecretString::operator==( const CSecretString& other ) const
{GUCEF_TRACE;

    // Compared as digests: neither the content nor the length leaks through the time it takes
    UInt8 digestA[ CAuthCrypto::DigestSize ];
    UInt8 digestB[ CAuthCrypto::DigestSize ];
    CAuthCrypto::Sha256( m_secret.C_String(), GetByteLength( m_secret ), digestA );
    CAuthCrypto::Sha256( other.m_secret.C_String(), GetByteLength( other.m_secret ), digestB );
    bool sameDigest = CAuthCrypto::ConstantTimeEquals( digestA, digestB, CAuthCrypto::DigestSize );
    return sameDigest && GetByteLength( m_secret ) == GetByteLength( other.m_secret );
}

/*-------------------------------------------------------------------------*/

bool
CSecretString::operator!=( const CSecretString& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

CORE::CVariantStream&
operator<<( CORE::CVariantStream& stream, const CSecretString& secret )
{GUCEF_TRACE;

    if ( secret.IsEmpty() )
        stream << "[empty]";
    else
        stream << "[redacted]";
    return stream;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
