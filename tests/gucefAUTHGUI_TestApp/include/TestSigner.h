/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_AUTH_TESTAPP_TESTSIGNER_H
#define GUCEF_AUTH_TESTAPP_TESTSIGNER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_ILINKSIGNER_H
#include "gucefAUTH_ILinkSigner.h"
#define GUCEF_AUTH_ILINKSIGNER_H
#endif /* GUCEF_AUTH_ILINKSIGNER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A profile key made for a test: an Ed25519 key pair from the signature codec and a key id derived from the public key */
class CTestSigner : public GUCEF::AUTH::ILinkSigner
{
    public:

    /** @param keyIdPrefix Gives the key id a readable start; the rest is derived from the public key */
    explicit CTestSigner( const GUCEF::CORE::CString& keyIdPrefix = "profile" );

    virtual ~CTestSigner();

    bool IsValid( void ) const;

    /** Counts the signatures made, to prove that a signer was or was not asked */
    GUCEF::UInt32 GetSignCount( void ) const;

    virtual GUCEF::CORE::CString GetKeyId( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::CString GetPublicKeyHex( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Sign( const void* data, GUCEF::UInt32 dataSize, GUCEF::CORE::CDynamicBuffer& signature ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    GUCEF::UInt8 m_seed[ 32 ];
    GUCEF::CORE::CString m_publicKeyHex;
    GUCEF::CORE::CString m_keyId;
    GUCEF::UInt32 m_signCount;
    bool m_valid;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTSIGNER_H ? */
