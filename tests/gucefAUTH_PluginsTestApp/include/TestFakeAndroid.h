/*
 *  gucefAUTH_PluginsTestApp: Application for testing the gucefAUTH plugins
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

#ifndef GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEANDROID_H
#define GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEANDROID_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H
#include "authpluginANDROID_CAndroidAuthBridge.h"
#define GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H
#endif /* GUCEF_AUTHPLUGIN_ANDROID_CANDROIDAUTHBRIDGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The Java side of the test: it answers the way GucefAuthBridge does, at once inside the call, or later from a thread of
 *  its own (a thread of the JVM), or not at all. The id token it builds is a real compact JWT with the claims the test
 *  sets, signed with a signature that means nothing: a client reads its own token without verifying it.
 */
class CFakeAndroid
{
    public:

    enum EDelivery
    {
        DELIVER_SYNC = 0,       /**< inside the call that starts the request */
        DELIVER_THREAD,         /**< from a thread of its own after deliveryDelayInMs */
        DELIVER_NEVER
    };

    enum ENonce
    {
        NONCE_ECHO = 0,         /**< the id token has the nonce that was asked for */
        NONCE_OMIT,
        NONCE_WRONG
    };

    bool googlePlay;
    bool amazon;
    bool googleStarts;
    bool amazonStarts;
    EDelivery delivery;
    GUCEF::UInt32 deliveryDelayInMs;

    // the answer for Google
    GUCEF::AUTHPLUGIN::ANDROIDAUTH::EAndroidAuthStatus googleStatus;
    bool withAuthCode;
    bool withIdToken;
    GUCEF::CORE::CString authCode;
    GUCEF::CORE::CString sub;
    GUCEF::CORE::CString aud;
    GUCEF::CORE::CString iss;
    GUCEF::Int64 expiresInSeconds;
    ENonce nonceBehaviour;
    GUCEF::CORE::CString name;
    GUCEF::CORE::CString picture;
    GUCEF::CORE::CString googleDisplayName;
    GUCEF::CORE::CString rawIdToken;        /**< when not empty this is the id token, whatever the claims say */

    // the answer for Amazon
    GUCEF::AUTHPLUGIN::ANDROIDAUTH::EAndroidAuthStatus amazonStatus;
    GUCEF::CORE::CString amazonToken;
    GUCEF::CORE::CString amazonUserId;
    GUCEF::CORE::CString amazonName;

    struct SGoogleCall
    {
        GUCEF::UInt32 requestId;
        GUCEF::AUTHPLUGIN::ANDROIDAUTH::EAndroidSignInMode mode;
        GUCEF::CORE::CString clientId;
        bool useCredentialManager;
        GUCEF::CORE::CString nonce;
    };

    struct SAmazonCall
    {
        GUCEF::UInt32 requestId;
        GUCEF::AUTHPLUGIN::ANDROIDAUTH::EAndroidSignInMode mode;
        GUCEF::CORE::CString scopes;
    };

    GUCEF::vector< SGoogleCall > googleCalls;
    GUCEF::vector< SAmazonCall > amazonCalls;
    GUCEF::vector< GUCEF::UInt32 > cancelled;
    GUCEF::UInt32 googleSignOutCalls;
    GUCEF::UInt32 amazonSignOutCalls;
    GUCEF::vector< GUCEF::CORE::CString > openedUrls;
    bool openTabResult;

    CFakeAndroid( void )
        : googlePlay( true ), amazon( true ), googleStarts( true ), amazonStarts( true ), delivery( DELIVER_SYNC ), deliveryDelayInMs( 10 )
        , googleStatus( GUCEF::AUTHPLUGIN::ANDROIDAUTH::ANDROIDAUTH_OK ), withAuthCode( true ), withIdToken( true )
        , authCode( "SERVER-AUTH-CODE-4/0AbCdEf-DO-NOT-LOG" ), sub( "109876543210987654321" ), aud( "web-client-id.apps.example.invalid" )
        , iss( "https://accounts.google.com" ), expiresInSeconds( 3600 ), nonceBehaviour( NONCE_ECHO ), name( "Gina Google" )
        , picture( "https://lh3.example.invalid/photo.jpg" ), googleDisplayName(), rawIdToken()
        , amazonStatus( GUCEF::AUTHPLUGIN::ANDROIDAUTH::ANDROIDAUTH_OK ), amazonToken( "Atza|AMAZON-ACCESS-TOKEN-DO-NOT-LOG" )
        , amazonUserId( "amzn1.account.AFAKEUSERID123" ), amazonName( "Amy Amazon" )
        , googleCalls(), amazonCalls(), cancelled(), googleSignOutCalls( 0 ), amazonSignOutCalls( 0 ), openedUrls(), openTabResult( true )
        , m_threads()
    {}

    ~CFakeAndroid()
    {
        for ( size_t i=0; i<m_threads.size(); ++i )
        {
            m_threads[ i ]->Deactivate( false, true );
            delete m_threads[ i ];
        }
    }

    GUCEF::AUTHPLUGIN::ANDROIDAUTH::TAndroidShim MakeShim( void )
    {
        using namespace GUCEF::AUTHPLUGIN::ANDROIDAUTH;
        TAndroidShim shim;
        shim.context = this;
        shim.isGooglePlayAvailable = &CFakeAndroid::IsGooglePlayAvailable;
        shim.isAmazonAvailable = &CFakeAndroid::IsAmazonAvailable;
        shim.googleSignIn = &CFakeAndroid::GoogleSignIn;
        shim.amazonSignIn = &CFakeAndroid::AmazonSignIn;
        shim.cancelRequest = &CFakeAndroid::CancelRequest;
        shim.googleSignOut = &CFakeAndroid::GoogleSignOut;
        shim.amazonSignOut = &CFakeAndroid::AmazonSignOut;
        shim.openCustomTab = &CFakeAndroid::OpenCustomTab;
        return shim;
    }

    /** A compact JWT with the claims; the signature is not one */
    static GUCEF::CORE::CString MakeJwt( const GUCEF::CORE::CString& payloadJson )
    {
        GUCEF::CORE::CString header;
        GUCEF::CORE::CString payload;
        const char* headerJson = "{\"alg\":\"RS256\",\"typ\":\"JWT\",\"kid\":\"fake\"}";
        GUCEF::CORE::Base64UrlEncode( headerJson, (GUCEF::UInt32) strlen( headerJson ), header, false );
        GUCEF::CORE::Base64UrlEncode( payloadJson.C_String(), payloadJson.ByteSize() - 1, payload, false );
        return header + "." + payload + ".c2lnbmF0dXJlLURPLU5PVC1MT0c";
    }

    GUCEF::CORE::CString BuildIdToken( const GUCEF::CORE::CString& nonce ) const
    {
        if ( !rawIdToken.IsNULLOrEmpty() )
            return rawIdToken;
        GUCEF::CORE::CString json = "{\"iss\":\"" + iss + "\",\"aud\":\"" + aud + "\",\"sub\":\"" + sub + "\"";
        json += ",\"exp\":" + GUCEF::CORE::UInt64ToString( (GUCEF::UInt64) ( (GUCEF::Int64) GUCEF::AUTH::GetEpochTimeInMs() / 1000 + expiresInSeconds ) );
        json += ",\"name\":\"" + name + "\",\"picture\":\"" + picture + "\"";
        if ( NONCE_ECHO == nonceBehaviour && !nonce.IsNULLOrEmpty() )
            json += ",\"nonce\":\"" + nonce + "\"";
        if ( NONCE_WRONG == nonceBehaviour )
            json += ",\"nonce\":\"not-the-nonce\"";
        json += "}";
        return MakeJwt( json );
    }

    private:

    class CDelivery : public GUCEF::MT::CActiveObject
    {
        public:

        CDelivery( const GUCEF::AUTHPLUGIN::ANDROIDAUTH::TAndroidAuthResult& result, GUCEF::UInt32 delayInMs, bool* taken )
            : GUCEF::MT::CActiveObject(), m_result( result ), m_delayInMs( delayInMs ), m_taken( taken ) {}

        virtual const GUCEF::MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

        protected:

        virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

        virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
        {
            GUCEF::MT::ThreadDelay( m_delayInMs );
            bool taken = GUCEF::AUTHPLUGIN::ANDROIDAUTH::CAndroidAuthBridge::Instance()->DeliverResult( m_result );
            if ( GUCEF_NULL != m_taken )
                *m_taken = taken;
            return true;
        }

        private:

        GUCEF::AUTHPLUGIN::ANDROIDAUTH::TAndroidAuthResult m_result;
        GUCEF::UInt32 m_delayInMs;
        bool* m_taken;
    };

    static CFakeAndroid* Self( void* context ) { return static_cast< CFakeAndroid* >( context ); }

    static bool IsGooglePlayAvailable( void* context ) { return Self( context )->googlePlay; }
    static bool IsAmazonAvailable( void* context ) { return Self( context )->amazon; }

    void Deliver( const GUCEF::AUTHPLUGIN::ANDROIDAUTH::TAndroidAuthResult& result )
    {
        if ( DELIVER_SYNC == delivery )
        {
            GUCEF::AUTHPLUGIN::ANDROIDAUTH::CAndroidAuthBridge::Instance()->DeliverResult( result );
        }
        else
        if ( DELIVER_THREAD == delivery )
        {
            CDelivery* thread = new CDelivery( result, deliveryDelayInMs, GUCEF_NULL );
            m_threads.push_back( thread );
            thread->Activate( GUCEF_NULL, 1, 1 );
        }
    }

    static bool GoogleSignIn( void* context, GUCEF::UInt32 requestId, GUCEF::AUTHPLUGIN::ANDROIDAUTH::EAndroidSignInMode mode,
                              const char* serverClientId, bool useCredentialManager, const char* nonce )
    {
        using namespace GUCEF::AUTHPLUGIN::ANDROIDAUTH;
        CFakeAndroid* self = Self( context );
        SGoogleCall call;
        call.requestId = requestId;
        call.mode = mode;
        call.clientId = serverClientId;
        call.useCredentialManager = useCredentialManager;
        call.nonce = nonce;
        self->googleCalls.push_back( call );
        if ( !self->googleStarts )
            return false;

        TAndroidAuthResult result;
        result.requestId = requestId;
        result.status = self->googleStatus;
        if ( ANDROIDAUTH_OK == self->googleStatus )
        {
            if ( self->withAuthCode )
                result.authCode.Set( self->authCode );
            if ( self->withIdToken && useCredentialManager )
                result.idToken.Set( self->BuildIdToken( call.nonce ) );
            result.displayName = self->googleDisplayName;
        }
        else
        {
            result.message = "fake: the Java side says no";
        }
        self->Deliver( result );
        return true;
    }

    static bool AmazonSignIn( void* context, GUCEF::UInt32 requestId, GUCEF::AUTHPLUGIN::ANDROIDAUTH::EAndroidSignInMode mode, const char* scopes )
    {
        using namespace GUCEF::AUTHPLUGIN::ANDROIDAUTH;
        CFakeAndroid* self = Self( context );
        SAmazonCall call;
        call.requestId = requestId;
        call.mode = mode;
        call.scopes = scopes;
        self->amazonCalls.push_back( call );
        if ( !self->amazonStarts )
            return false;

        TAndroidAuthResult result;
        result.requestId = requestId;
        result.status = self->amazonStatus;
        if ( ANDROIDAUTH_OK == self->amazonStatus )
        {
            if ( !self->amazonToken.IsNULLOrEmpty() )
                result.accessToken.Set( self->amazonToken );
            result.accountId = self->amazonUserId;
            result.displayName = self->amazonName;
        }
        else
        {
            result.message = "fake: Login with Amazon says no";
        }
        self->Deliver( result );
        return true;
    }

    static void CancelRequest( void* context, GUCEF::UInt32 requestId ) { Self( context )->cancelled.push_back( requestId ); }
    static void GoogleSignOut( void* context ) { ++Self( context )->googleSignOutCalls; }
    static void AmazonSignOut( void* context ) { ++Self( context )->amazonSignOutCalls; }

    static bool OpenCustomTab( void* context, const char* url )
    {
        CFakeAndroid* self = Self( context );
        self->openedUrls.push_back( url );
        return self->openTabResult;
    }

    private:

    GUCEF::vector< CDelivery* > m_threads;
};

/*-------------------------------------------------------------------------*/

/** Puts a fake Java side in the bridge for the time it exists and takes it out again */
class CScopedAndroid
{
    public:

    explicit CScopedAndroid( CFakeAndroid& fake )
        : m_previous( GUCEF::AUTHPLUGIN::ANDROIDAUTH::CAndroidAuthBridge::Instance()->GetShim() )
    {
        GUCEF::AUTHPLUGIN::ANDROIDAUTH::CAndroidAuthBridge::Instance()->SetShim( fake.MakeShim() );
    }

    ~CScopedAndroid()
    {
        GUCEF::AUTHPLUGIN::ANDROIDAUTH::CAndroidAuthBridge::Instance()->SetShim( m_previous );
    }

    private:

    CScopedAndroid( const CScopedAndroid& src );
    CScopedAndroid& operator=( const CScopedAndroid& src );

    GUCEF::AUTHPLUGIN::ANDROIDAUTH::TAndroidShim m_previous;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_PLUGINSTESTAPP_TESTFAKEANDROID_H ? */
