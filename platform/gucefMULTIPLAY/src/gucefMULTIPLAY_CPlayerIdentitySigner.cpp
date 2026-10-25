/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#include "gucefMULTIPLAY_CPlayerIdentitySigner.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPlayerIdentitySigner::CPlayerIdentitySigner( void )
    : AUTH::ILinkSigner()
    , m_identity()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPlayerIdentitySigner::CPlayerIdentitySigner( const CPlayerIdentity& identity )
    : AUTH::ILinkSigner()
    , m_identity( identity )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPlayerIdentitySigner::~CPlayerIdentitySigner()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPlayerIdentitySigner::SetIdentity( const CPlayerIdentity& identity )
{GUCEF_TRACE;

    m_identity = identity;
    // Only the key is of interest, not the proofs of the logins
    m_identity.linkProofs.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPlayerIdentitySigner::GetKeyId( void ) const
{GUCEF_TRACE;

    return m_identity.userId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPlayerIdentitySigner::GetPublicKeyHex( void ) const
{GUCEF_TRACE;

    return m_identity.publicKey.Lowercase();
}

/*-------------------------------------------------------------------------*/

bool
CPlayerIdentitySigner::Sign( const void* data, UInt32 dataSize, CORE::CDynamicBuffer& signature )
{GUCEF_TRACE;

    if ( !m_identity.HasKeyPair() )
        return false;
    UInt8 bytes[ CPlayerIdentity::SignatureSize ];
    if ( !m_identity.Sign( data, dataSize, bytes ) )
        return false;
    signature.SetDataSize( 0 );
    signature.Append( bytes, sizeof( bytes ), true );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
