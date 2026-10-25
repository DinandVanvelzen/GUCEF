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

#ifndef GUCEF_MULTIPLAY_CLINKPROOF_H
#define GUCEF_MULTIPLAY_CLINKPROOF_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTH_CAUTHRESULT_H
#include "gucefAUTH_CAuthResult.h"
#define GUCEF_AUTH_CAUTHRESULT_H
#endif /* GUCEF_AUTH_CAUTHRESULT_H ? */

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
 *  What a client shows a lobby server of one account it is logged in with: the name of the provider that can check
 *  the proof (the login mechanism, for example "google"), and the proof of that login. The profile key signs it
 *  together with the challenge of the connection when the proof is sent (CGucefLobbyAuthenticator), so the signature
 *  is not part of this class. The proof is a credential until it expires: it is a secret string that does not show up
 *  in log streams and is not serialized.
 *
 *  The limits are those of the proof of the handshake, which has to fit one packet before the link is operational.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLinkProof
{
    public:

    static const UInt32 MaxCount = 4;               /**< proofs per connection */
    static const UInt32 MaxProviderNameLength = 64;
    static const UInt32 MaxKindLength = 32;
    static const UInt32 MaxDataSize = 3072;         /**< bytes of the proof data, the largest identity token fits */

    CORE::CString providerName;
    AUTH::CAuthProof proof;

    CLinkProof( void );

    CLinkProof( const CORE::CString& provider, const AUTH::CAuthProof& authProof );

    CLinkProof( const CLinkProof& src );

    ~CLinkProof();

    CLinkProof& operator=( const CLinkProof& src );

    /** Whether it names a provider and carries a proof of a size the handshake takes */
    bool IsSendable( void ) const;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CLinkProof > TLinkProofVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CLINKPROOF_H ? */
