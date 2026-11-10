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

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHBINARYWRITER_H
#include "gucefMULTIPLAY_CMatchBinaryWriter.h"
#define GUCEF_MULTIPLAY_CMATCHBINARYWRITER_H
#endif /* GUCEF_MULTIPLAY_CMATCHBINARYWRITER_H ? */
#ifndef GUCEF_MULTIPLAY_CMATCHBINARYREADER_H
#include "gucefMULTIPLAY_CMatchBinaryReader.h"
#define GUCEF_MULTIPLAY_CMATCHBINARYREADER_H
#endif /* GUCEF_MULTIPLAY_CMATCHBINARYREADER_H ? */

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#include "gucefMULTIPLAY_CMatchSigning.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CMatchSigning::KindInfo = "info";
const char* CMatchSigning::KindAcceptance = "accept";
const char* CMatchSigning::KindPreview = "preview";
const char* CMatchSigning::KindProposal = "proposal";
const char* CMatchSigning::KindRoster = "roster";
const char* CMatchSigning::KindVote = "vote";

static const char MatchSigningDomainTag[] = "GUCEF-MATCH-1";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMatchSigning::BuildMessage( const char* kind, const CORE::CString& matchId, const void* data, UInt32 dataSize, CORE::CDynamicBuffer& message )
{GUCEF_TRACE;

    // every part is length prefixed, so no part can be moved into its neighbour
    CMatchBinaryWriter writer( message );
    writer.WriteString( MatchSigningDomainTag );
    writer.WriteString( kind );
    writer.WriteString( matchId );
    writer.WriteBytes( data, dataSize );
}

/*-------------------------------------------------------------------------*/

bool
CMatchSigning::Sign( const CPlayerIdentity& identity   ,
                     const char* kind                  ,
                     const CORE::CString& matchId      ,
                     const void* data                  ,
                     UInt32 dataSize                   ,
                     CORE::CString& signatureHex       )
{GUCEF_TRACE;

    signatureHex.Clear();
    if ( GUCEF_NULL == kind || !CMatchId::IsValid( matchId ) || !identity.HasKeyPair() )
        return false;

    CORE::CDynamicBuffer message;
    BuildMessage( kind, matchId, data, dataSize, message );
    UInt8 signature[ CPlayerIdentity::SignatureSize ];
    if ( !identity.Sign( message.GetConstBufferPtr(), message.GetDataSize(), signature ) )
        return false;
    signatureHex = DRN::CDRNCrypto::ToHex( signature, CPlayerIdentity::SignatureSize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMatchSigning::Verify( const CORE::CString& publicKeyHex   ,
                       const char* kind                    ,
                       const CORE::CString& matchId        ,
                       const void* data                    ,
                       UInt32 dataSize                     ,
                       const CORE::CString& signatureHex   )
{GUCEF_TRACE;

    if ( GUCEF_NULL == kind || !CMatchId::IsValid( matchId ) ||
         !IsHexOfLength( publicKeyHex, PublicKeyHexSize ) || !IsHexOfLength( signatureHex, SignatureHexSize ) )
        return false;

    UInt8 key[ CPlayerIdentity::KeySize ];
    UInt8 signature[ CPlayerIdentity::SignatureSize ];
    if ( !DRN::CDRNCrypto::FromHex( publicKeyHex, key, CPlayerIdentity::KeySize ) ||
         !DRN::CDRNCrypto::FromHex( signatureHex, signature, CPlayerIdentity::SignatureSize ) )
        return false;

    CORE::CDynamicBuffer message;
    BuildMessage( kind, matchId, data, dataSize, message );
    return DRN::CDRNCrypto::Verify( key, message.GetConstBufferPtr(), message.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchSigning::Sha256Hex( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    UInt8 digest[ DRN::CDRNCrypto::HashSize ];
    DRN::CDRNCrypto::Sha256( data, dataSize, digest );
    return DRN::CDRNCrypto::ToHex( digest, DRN::CDRNCrypto::HashSize );
}

/*-------------------------------------------------------------------------*/

bool
CMatchSigning::IsHexOfLength( const CORE::CString& text, UInt32 length )
{GUCEF_TRACE;

    const char* p = text.C_String();
    if ( strlen( p ) != length )
        return false;
    for ( UInt32 i=0; i<length; ++i )
    {
        char c = p[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) ) )
            return false;
    }
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
