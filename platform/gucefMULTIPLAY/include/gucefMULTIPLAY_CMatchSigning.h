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

#ifndef GUCEF_MULTIPLAY_CMATCHSIGNING_H
#define GUCEF_MULTIPLAY_CMATCHSIGNING_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Signs and verifies the records of a match with the Ed25519 key of a player identity.
 *
 *  What is signed is never the bare record: the signed message is a fixed domain tag, the kind of the record
 *  ("preview", "info", "proposal", ...), the match id and then the bytes of the record. The match id is the
 *  signing context, so a signed record of one match is no record of another match, and the kind keeps a
 *  signature made for one kind of record from being presented as another. Signatures travel as 128 hex characters.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchSigning
{
    public:

    static const UInt32 SignatureHexSize = 128;
    static const UInt32 PublicKeyHexSize = 64;
    static const UInt32 Sha256HexSize = 64;

    static const char* KindInfo;            /**< the match record the creator signs */
    static const char* KindAcceptance;
    static const char* KindPreview;
    static const char* KindProposal;
    static const char* KindVote;
    static const char* KindRoster;          /**< the signed table of a game that has no record yet (CMatchRoster) */

    /** Signs the record bytes for the match, false without a key pair or when the match id is not one */
    static bool Sign( const CPlayerIdentity& identity   ,
                      const char* kind                  ,
                      const CORE::CString& matchId      ,
                      const void* data                  ,
                      UInt32 dataSize                   ,
                      CORE::CString& signatureHex       );

    /** Whether the signature of the record bytes is by the key, for exactly this kind of record and this match */
    static bool Verify( const CORE::CString& publicKeyHex   ,
                        const char* kind                    ,
                        const CORE::CString& matchId        ,
                        const void* data                    ,
                        UInt32 dataSize                     ,
                        const CORE::CString& signatureHex   );

    /** SHA-256 of the bytes as 64 lower case hex characters */
    static CORE::CString Sha256Hex( const void* data, UInt32 dataSize );

    /** Whether the text is a hex string of exactly the given number of characters */
    static bool IsHexOfLength( const CORE::CString& text, UInt32 length );

    private:

    static void BuildMessage( const char* kind, const CORE::CString& matchId, const void* data, UInt32 dataSize, CORE::CDynamicBuffer& message );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHSIGNING_H ? */
