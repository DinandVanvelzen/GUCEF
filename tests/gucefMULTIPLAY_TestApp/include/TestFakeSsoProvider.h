/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTFAKESSOPROVIDER_H
#define GUCEF_MULTIPLAY_TESTAPP_TESTFAKESSOPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A login provider that needs no network: a login completes at once with the account namespace:subject, and the proof
 *  it gives is a text "fake-proof;<namespace>;<subject>;<expires ms>;<padding>" which Verify() takes apart, so a server
 *  in a test can verify what a client of a test showed. The proof can be made for any subject and any expiry
 *  (MakeProof()) to test what a server does with proofs that are old, forged or for other accounts.
 */
class CFakeSsoProvider : public GUCEF::AUTH::CIAuthProvider
{
    public:

    GUCEF::CORE::CString name;
    GUCEF::CORE::CString accountNamespace;
    GUCEF::CORE::CString subject;
    GUCEF::CORE::CString displayName;
    GUCEF::UInt32 capabilities;
    bool refuseProofs;                  /**< Verify() says no to everything */
    GUCEF::UInt32 padding;              /**< characters of padding in the proofs it makes */
    GUCEF::UInt32 verifyCount;
    GUCEF::UInt64 lifetimeMs;           /**< how long the proofs of its logins are good */

    explicit CFakeSsoProvider( const GUCEF::CORE::CString& providerName = "fakesso" )
        : GUCEF::AUTH::CIAuthProvider()
        , name( providerName )
        , accountNamespace( "fake" )
        , subject( "player-1" )
        , displayName( "Fake Player" )
        , capabilities( GUCEF::AUTH::AUTHCAP_BROWSER | GUCEF::AUTH::AUTHCAP_SERVER_VERIFIABLE )
        , refuseProofs( false )
        , padding( 0 )
        , verifyCount( 0 )
        , lifetimeMs( 3600000 )
    {
    }

    virtual ~CFakeSsoProvider() {}

    /** The proof of a login of the account that expires at the given time */
    GUCEF::CORE::CString MakeProof( const GUCEF::CORE::CString& proofSubject, GUCEF::UInt64 expiresAtMs, GUCEF::UInt32 extraPadding = 0 ) const
    {
        GUCEF::CORE::CString filler;
        for ( GUCEF::UInt32 i=0; i<padding + extraPadding; ++i )
            filler += "x";
        return "fake-proof;" + accountNamespace + ";" + proofSubject + ";" + GUCEF::CORE::UInt64ToString( expiresAtMs ) + ";" + filler;
    }

    virtual const GUCEF::CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE { return name; }

    virtual GUCEF::UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return capabilities; }

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool TrySilentLogin( GUCEF::CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        future = GUCEF::CORE::CFutureResult( GUCEF::CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        return false;
    }

    virtual bool BeginLogin( const GUCEF::AUTH::CAuthOptions& options, GUCEF::CORE::CFutureResult& future ) GUCEF_VIRTUAL_OVERRIDE
    {
        GUCEF::CORE::CFutureResultSource source;
        future = source.GetFuture();
        GUCEF::UInt64 expiresAtMs = GUCEF::AUTH::GetEpochTimeInMs() + lifetimeMs;
        GUCEF::AUTH::CAuthResult result;
        result.accountRef = GUCEF::AUTH::CAccountRef( accountNamespace, subject );
        result.displayName = displayName;
        result.expiresAtMs = expiresAtMs;
        result.proof = GUCEF::AUTH::CAuthProof( GUCEF::AUTH::CAuthProof::KindOidcIdToken, MakeProof( subject, expiresAtMs ) );
        return CompleteLogin( source, result );
    }

    virtual bool Verify( const GUCEF::AUTH::CAuthProof& proof, GUCEF::AUTH::CAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++verifyCount;
        if ( refuseProofs || GUCEF::AUTH::CAuthProof::KindOidcIdToken != proof.kind )
            return false;
        GUCEF::CORE::CString::StringVector parts = proof.data.Reveal().ParseElements( ';', true );
        if ( parts.size() < 4 || "fake-proof" != parts[ 0 ] || accountNamespace != parts[ 1 ] )
            return false;
        if ( GUCEF::CORE::StringToUInt64( parts[ 3 ], 0 ) <= GUCEF::AUTH::GetEpochTimeInMs() )
            return false;
        result = GUCEF::AUTH::CAuthResult();
        result.providerName = name;
        result.accountRef = GUCEF::AUTH::CAccountRef( parts[ 1 ], parts[ 2 ] );
        return result.accountRef.IsValid();
    }

    virtual bool SaveConfig( GUCEF::CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool LoadConfig( const GUCEF::CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "CFakeSsoProvider";
        return classTypeName;
    }
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::CORE::CTFactory< GUCEF::AUTH::CIAuthProvider, CFakeSsoProvider, GUCEF::MT::CMutex > TFakeSsoProviderFactory;

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTFAKESSOPROVIDER_H ? */
