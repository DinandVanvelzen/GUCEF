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

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H
#define GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_ILINKSIGNER_H
#include "gucefAUTH_ILinkSigner.h"
#define GUCEF_AUTH_ILINKSIGNER_H
#endif /* GUCEF_AUTH_ILINKSIGNER_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

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
 *  The profile key of a player identity as gucefAUTH sees it (AUTH::ILinkSigner): the user id is the key id, the
 *  Ed25519 public key of the identity is the key and Sign() signs with its private key. An identity is copyable and
 *  the interface is not, so this is the adapter: it holds a copy of the identity and is given the current one with
 *  SetIdentity(). An identity without a key pair has no public key and signs nothing.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CPlayerIdentitySigner : public AUTH::ILinkSigner
{
    public:

    CPlayerIdentitySigner( void );

    explicit CPlayerIdentitySigner( const CPlayerIdentity& identity );

    virtual ~CPlayerIdentitySigner();

    /** Takes the key of the identity */
    void SetIdentity( const CPlayerIdentity& identity );

    virtual CORE::CString GetKeyId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetPublicKeyHex( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Sign( const void* data, UInt32 dataSize, CORE::CDynamicBuffer& signature ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CPlayerIdentity m_identity;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITYSIGNER_H ? */
