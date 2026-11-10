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

#ifndef GUCEF_AUTH_ILINKSIGNER_H
#define GUCEF_AUTH_ILINKSIGNER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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
 *  The profile key as gucefAUTH needs it: an id, an Ed25519 public key and the ability to sign with the private key.
 *  The module does not know where the key lives; gucefMULTIPLAY implements this on its player identity (the user id
 *  is the id, the "ed25519" key of the profile is the key), a test implements it on a generated key. Verifying a
 *  signature needs no implementation: it only takes the public key (CLinkRecord::VerifySignature()).
 */
class GUCEF_AUTH_EXPORT_CPP ILinkSigner
{
    public:

    ILinkSigner( void );

    virtual ~ILinkSigner();

    /** The id of the profile the key belongs to: what link records call the profile key id */
    virtual CORE::CString GetKeyId( void ) const = 0;

    /** The Ed25519 public key as 64 hex digits */
    virtual CORE::CString GetPublicKeyHex( void ) const = 0;

    /** Signs with Ed25519: signature receives the 64 bytes. False when there is no key or no codec */
    virtual bool Sign( const void* data, UInt32 dataSize, CORE::CDynamicBuffer& signature ) = 0;

    private:

    ILinkSigner( const ILinkSigner& src );
    ILinkSigner& operator=( const ILinkSigner& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_ILINKSIGNER_H ? */
