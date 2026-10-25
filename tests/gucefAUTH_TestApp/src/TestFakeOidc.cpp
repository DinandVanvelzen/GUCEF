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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CIHttpServerRequestHandler.h"
#define GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#include "TestText.h"
#include "TestFakeOidc.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CFakeOidcHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CFakeOidcHandler( void ) : WEB::CIHttpServerRequestHandler(), m_provider( GUCEF_NULL ) {}
    virtual ~CFakeOidcHandler() {}

    void SetProvider( CFakeOidcProvider* provider ) { m_provider = provider; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == m_provider )
        {
            response.statusCode = 503;
            return true;
        }
        m_provider->HandleRequest( request, response );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    CFakeOidcProvider* m_provider;
};

/*-------------------------------------------------------------------------*/

class CFakeOidcHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeOidcHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeOidcHandler, MT::CMutex > TBase;

    explicit CFakeOidcHandlerFactory( CFakeOidcProvider* provider ) : TBase(), m_provider( provider ) {}
    virtual ~CFakeOidcHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        CFakeOidcHandler* handler = static_cast< CFakeOidcHandler* >( product.GetPointerAlways() );
        if ( GUCEF_NULL != handler )
            handler->SetProvider( m_provider );
        return product;
    }

    private:

    CFakeOidcHandlerFactory( const CFakeOidcHandlerFactory& src );
    CFakeOidcHandlerFactory& operator=( const CFakeOidcHandlerFactory& src );

    CFakeOidcProvider* m_provider;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeTokenSpec::CFakeTokenSpec( void )
    : issuer()
    , subject( "sub-1000" )
    , audiences()
    , setAzp( false )
    , azp()
    , expiresInSeconds( 3600 )
    , setNotBefore( false )
    , notBeforeInSeconds( 0 )
    , setIssuedAt( true )
    , issuedAtInSeconds( 0 )
    , includeNonce( true )
    , nonceOverride()
    , algorithm( "RS256" )
    , keyId()
    , omitKeyId( false )
    , tamperSignature( false )
    , signWith()
    , tenantId()
    , objectId()
    , name( "Fake Player" )
    , picture( "https://pictures.invalid/p.png" )
    , includeAtHash( true )
    , atHashOverride()
    , addCrit( false )
    , extraPayloadMembers()
    , duplicateAlg( false )
{
}

/*-------------------------------------------------------------------------*/

CFakeBehavior::CFakeBehavior( void )
    : denyAuthorization( false )
    , requirePkce( true )
    , tokenDelayInMs( 0 )
    , tokenError()
    , tokenErrorStatus( 400 )
    , rotateRefreshTokens( true )
    , issueIdToken( true )
    , refreshIssuesIdToken( false )
    , devicePendingPolls( 1 )
    , deviceSlowDownAtPoll( 0 )
    , deviceOutcome( "ok" )
    , deviceInterval( 1 )
    , deviceExpiresIn( 600 )
    , deviceOmitInterval( false )
    , jwksCacheControl( "public, max-age=3600" )
    , jwksDelayInMs( 0 )
    , openIdAlwaysInvalid( false )
    , openIdSubject( "4200000042" )
    , openIdName()
    , openIdClaimedIdOverride()
    , openIdReturnToOverride()
    , openIdReturnToEdit()
    , openIdCancel( false )
    , openIdOpEndpointOverride()
    , openIdOmitSignedReturnTo( false )
    , openIdNonceAgeInSeconds( 0 )
    , openIdCheckStatus( 0 )
    , userinfoSubjectOverride()
    , userinfoFails( false )
{
}

/*-------------------------------------------------------------------------*/

CFakeStats::CFakeStats( void )
    : discovery( 0 ), jwks( 0 ), authorize( 0 ), token( 0 ), userinfo( 0 ), device( 0 ), devicePolls( 0 ), revoke( 0 )
    , openIdLogin( 0 ), openIdCheck( 0 ), verifyRequests( 0 ), pkceFailures( 0 ), codeReuse( 0 )
{
}

/*-------------------------------------------------------------------------*/

/** A number with leading zeros up to the width */
static CORE::CString
Pad( Int64 value, UInt32 width )
{
    CORE::CString text = CORE::Int64ToString( value );
    while ( text.Length() < width )
        text = "0" + text;
    return text;
}

/*-------------------------------------------------------------------------*/

/** 2010-05-12T20:31:54Z for the seconds since 1970 */
static CORE::CString
FormatNonceTime( Int64 seconds )
{
    Int64 days = seconds / 86400;
    Int64 rest = seconds % 86400;
    Int64 z = days + 719468;
    Int64 era = ( z >= 0 ? z : z - 146096 ) / 146097;
    Int64 doe = z - era * 146097;
    Int64 yoe = ( doe - doe / 1460 + doe / 36524 - doe / 146096 ) / 365;
    Int64 y = yoe + era * 400;
    Int64 doy = doe - ( 365 * yoe + yoe / 4 - yoe / 100 );
    Int64 mp = ( 5 * doy + 2 ) / 153;
    Int64 d = doy - ( 153 * mp + 2 ) / 5 + 1;
    Int64 m = mp < 10 ? mp + 3 : mp - 9;
    if ( m <= 2 )
        ++y;
    return Pad( y, 4 ) + "-" + Pad( m, 2 ) + "-" + Pad( d, 2 ) + "T" + Pad( rest / 3600, 2 ) + ":" + Pad( ( rest % 3600 ) / 60, 2 ) + ":" + Pad( rest % 60, 2 ) + "Z";
}

/*-------------------------------------------------------------------------*/

CFakeOidcProvider::CFakeOidcProvider( void )
    : MT::CActiveObject()
    , m_lock()
    , m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
    , m_server( GUCEF_NULL )
    , m_factory( GUCEF_NULL )
    , m_port( 0 )
    , m_running( false )
    , m_issuerPath()
    , m_clientId( "fake-client-id" )
    , m_signingKeyId( "key-a" )
    , m_keys()
    , m_publishedKeys()
    , m_spec()
    , m_behavior()
    , m_stats()
    , m_lastAuthorize()
    , m_lastToken()
    , m_lastTokenAuthorization()
    , m_lastTokenContentType()
    , m_lastOpenIdCheck()
    , m_lastOpenIdCheckContentType()
    , m_revoked()
    , m_devicePollTimes()
    , m_codes()
    , m_devices()
    , m_refreshTokens()
    , m_accessTokens()
    , m_openIdNonces()
    , m_verifyTokens()
{
}

/*-------------------------------------------------------------------------*/

CFakeOidcProvider::~CFakeOidcProvider()
{
    Stop();
}

/*-------------------------------------------------------------------------*/

bool
CFakeOidcProvider::Start( void )
{
    if ( m_running )
        return true;
    CFakeOidcHandlerFactory* factory = GUCEF_NEW CFakeOidcHandlerFactory( this );
    m_factory = factory;
    WEB::CHttpServerSettings settings;
    settings.bindAddress = "127.0.0.1";
    settings.keepAliveConnections = false;
    settings.processRequestsAsync = false;
    settings.allowWebSocketUpgrades = false;
    settings.maxClientConnections = 64;
    m_server = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, factory, &settings );
    if ( !m_server->ListenOnPort( 0 ) )
        return false;
    m_port = m_server->GetBoundPort();
    m_running = true;
    return Activate( GUCEF_NULL, 1, 1 );
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::Stop( void )
{
    if ( !m_running )
        return;
    m_running = false;
    Deactivate( false, true );
    if ( GUCEF_NULL != m_server )
    {
        m_server->Close();
        GUCEF_DELETE m_server;
        m_server = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_factory )
    {
        GUCEF_DELETE static_cast< CFakeOidcHandlerFactory* >( m_factory );
        m_factory = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CFakeOidcProvider::AsLockable( void ) const
{
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CFakeOidcProvider::OnThreadStart( void* taskdata )
{
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFakeOidcProvider::OnThreadCycle( void* taskdata )
{
    m_pulseGenerator->ForceDirectPulse();
    return false;
}

/*-------------------------------------------------------------------------*/

UInt16
CFakeOidcProvider::GetPort( void ) const
{
    return m_port;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::GetBaseUrl( void ) const
{
    return "http://127.0.0.1:" + CORE::ToString( m_port );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::GetIssuer( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return GetBaseUrl() + m_issuerPath;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::SetIssuerPath( const CORE::CString& path )
{
    MT::CScopeMutex lock( m_lock );
    m_issuerPath = path;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::SetClientId( const CORE::CString& clientId )
{
    MT::CScopeMutex lock( m_lock );
    m_clientId = clientId;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::SetSigningKey( const CORE::CString& keyId, const CTestRsaKey* key )
{
    MT::CScopeMutex lock( m_lock );
    m_signingKeyId = keyId;
    m_keys[ keyId ] = key;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::PublishKey( const CORE::CString& keyId, const CTestRsaKey* key )
{
    MT::CScopeMutex lock( m_lock );
    m_keys[ keyId ] = key;
    m_publishedKeys[ keyId ] = key;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::UnpublishKey( const CORE::CString& keyId )
{
    MT::CScopeMutex lock( m_lock );
    m_publishedKeys.erase( keyId );
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::SetTokenSpec( const CFakeTokenSpec& spec )
{
    MT::CScopeMutex lock( m_lock );
    m_spec = spec;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::SetBehavior( const CFakeBehavior& behavior )
{
    MT::CScopeMutex lock( m_lock );
    m_behavior = behavior;
}

/*-------------------------------------------------------------------------*/

CFakeBehavior
CFakeOidcProvider::GetBehavior( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_behavior;
}

/*-------------------------------------------------------------------------*/

CFakeStats
CFakeOidcProvider::GetStats( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_stats;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringMap
CFakeOidcProvider::GetLastAuthorizeParameters( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_lastAuthorize;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringMap
CFakeOidcProvider::GetLastTokenParameters( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_lastToken;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::GetLastTokenAuthorization( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_lastTokenAuthorization;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::GetLastTokenContentType( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_lastTokenContentType;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringMap
CFakeOidcProvider::GetLastOpenIdCheck( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_lastOpenIdCheck;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::GetLastOpenIdCheckContentType( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_lastOpenIdCheckContentType;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CFakeOidcProvider::GetRevokedTokens( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_revoked;
}

/*-------------------------------------------------------------------------*/

vector< UInt64 >
CFakeOidcProvider::GetDevicePollTimes( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_devicePollTimes;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::MakeRandom( void ) const
{
    UInt8 bytes[ 24 ];
    CORE::CRandom::GetRandomBytes( bytes, sizeof( bytes ) );
    CORE::CString text;
    CORE::Base64UrlEncode( bytes, sizeof( bytes ), text );
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::IssuerFor( const CORE::CString& tenant ) const
{
    CORE::CString path = m_issuerPath.ReplaceSubstr( "{tid}", tenant );
    return GetBaseUrl() + path;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::MintLocked( const CFakeTokenSpec& spec, const CORE::CString& nonce, const CORE::CString& accessToken ) const
{
    UInt64 now = AUTH::GetEpochTimeInMs() / 1000;
    CORE::CString tenant = spec.tenantId;
    CORE::CString issuer = spec.issuer.IsNULLOrEmpty() ? IssuerFor( tenant.IsNULLOrEmpty() ? CORE::CString( "tenant-0001" ) : tenant ) : spec.issuer;

    CORE::CString keyId = spec.keyId.IsNULLOrEmpty() ? m_signingKeyId : spec.keyId;
    CORE::CString header = "{\"alg\":" + QuoteJson( spec.algorithm );
    if ( spec.duplicateAlg )
        header += ",\"alg\":\"RS256\"";
    if ( !spec.omitKeyId )
        header += ",\"kid\":" + QuoteJson( keyId );
    header += ",\"typ\":\"JWT\"";
    if ( spec.addCrit )
        header += ",\"crit\":[\"exp\"]";
    header += "}";

    CORE::CString payload = "{\"iss\":" + QuoteJson( issuer ) + ",\"sub\":" + QuoteJson( spec.subject );
    CORE::CString::StringVector audiences = spec.audiences;
    if ( audiences.empty() )
        audiences.push_back( m_clientId );
    if ( 1 == audiences.size() )
        payload += ",\"aud\":" + QuoteJson( audiences[ 0 ] );
    else
    {
        payload += ",\"aud\":[";
        for ( size_t i=0; i<audiences.size(); ++i )
            payload += ( i > 0 ? "," : "" ) + QuoteJson( audiences[ i ] );
        payload += "]";
    }
    payload += ",\"exp\":" + CORE::Int64ToString( (Int64) now + spec.expiresInSeconds );
    if ( spec.setIssuedAt )
        payload += ",\"iat\":" + CORE::Int64ToString( (Int64) now + spec.issuedAtInSeconds );
    if ( spec.setNotBefore )
        payload += ",\"nbf\":" + CORE::Int64ToString( (Int64) now + spec.notBeforeInSeconds );
    if ( spec.setAzp )
        payload += ",\"azp\":" + QuoteJson( spec.azp );
    if ( spec.includeNonce )
    {
        CORE::CString useNonce = spec.nonceOverride.IsNULLOrEmpty() ? nonce : spec.nonceOverride;
        if ( !useNonce.IsNULLOrEmpty() )
            payload += ",\"nonce\":" + QuoteJson( useNonce );
    }
    if ( spec.includeAtHash && !accessToken.IsNULLOrEmpty() )
    {
        CORE::CString atHash = spec.atHashOverride;
        if ( atHash.IsNULLOrEmpty() )
        {
            UInt8 digest[ AUTH::CAuthCrypto::DigestSize ];
            AUTH::CAuthCrypto::Sha256( accessToken.C_String(), AUTH::GetByteLength( accessToken ), digest );
            CORE::Base64UrlEncode( digest, AUTH::CAuthCrypto::DigestSize / 2, atHash );
        }
        payload += ",\"at_hash\":" + QuoteJson( atHash );
    }
    if ( !spec.tenantId.IsNULLOrEmpty() )
        payload += ",\"tid\":" + QuoteJson( spec.tenantId );
    if ( !spec.objectId.IsNULLOrEmpty() )
        payload += ",\"oid\":" + QuoteJson( spec.objectId );
    if ( !spec.name.IsNULLOrEmpty() )
        payload += ",\"name\":" + QuoteJson( spec.name );
    if ( !spec.picture.IsNULLOrEmpty() )
        payload += ",\"picture\":" + QuoteJson( spec.picture );
    if ( !spec.extraPayloadMembers.IsNULLOrEmpty() )
        payload += "," + spec.extraPayloadMembers;
    payload += "}";

    CORE::CString headerPart, payloadPart;
    CORE::Base64UrlEncode( header.C_String(), AUTH::GetByteLength( header ), headerPart );
    CORE::Base64UrlEncode( payload.C_String(), AUTH::GetByteLength( payload ), payloadPart );
    CORE::CString signingInput = headerPart + "." + payloadPart;

    if ( spec.algorithm == "none" )
        return signingInput + ".";

    CORE::CString signaturePart;
    if ( spec.algorithm != "RS256" )
    {
        // An algorithm that is not accepted. HS256 is the attack of using the public key as the secret of a MAC: that is done here, with the key set's own key
        UInt8 mac[ CORE::CHmacSha256::MacSize ];
        memset( mac, 0x5A, sizeof( mac ) );
        TKeyMap::const_iterator found = m_keys.find( m_signingKeyId );
        if ( spec.algorithm == "HS256" && found != m_keys.end() )
            CORE::CHmacSha256::Mac( (*found).second->GetModulus().GetConstBufferPtr(), (*found).second->GetModulus().GetDataSize(), signingInput.C_String(), AUTH::GetByteLength( signingInput ), mac );
        CORE::Base64UrlEncode( mac, sizeof( mac ), signaturePart );
        return signingInput + "." + signaturePart;
    }

    const CTestRsaKey* key = GUCEF_NULL;
    if ( spec.signWith == "evil" )
    {
        TKeyMap::const_iterator evil = m_keys.find( "evil" );
        if ( evil != m_keys.end() )
            key = (*evil).second;
    }
    else
    {
        TKeyMap::const_iterator found = m_keys.find( m_signingKeyId );
        if ( found != m_keys.end() )
            key = (*found).second;
    }
    if ( GUCEF_NULL == key )
        return CORE::CString::Empty;

    CORE::CDynamicBuffer signature;
    key->SignSha256( signingInput.C_String(), AUTH::GetByteLength( signingInput ), signature );
    CORE::Base64UrlEncode( signature.GetConstBufferPtr(), signature.GetDataSize(), signaturePart );
    if ( spec.tamperSignature )
    {
        // a character in the middle changes, the length and the encoding stay right
        CORE::CString tampered = AUTH::MakeString( signaturePart.C_String(), 20 );
        tampered += signaturePart.C_String()[ 20 ] == 'A' ? 'B' : 'A';
        tampered += signaturePart.C_String() + 21;
        signaturePart = tampered;
    }
    return signingInput + "." + signaturePart;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::MintIdToken( const CFakeTokenSpec& spec, const CORE::CString& nonce, const CORE::CString& accessToken ) const
{
    MT::CScopeMutex lock( m_lock );
    return MintLocked( spec, nonce, accessToken );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::IssueRefreshToken( const CORE::CString& subject )
{
    MT::CScopeMutex lock( m_lock );
    CORE::CString token = "refresh-" + MakeRandom();
    m_refreshTokens[ token ] = subject;
    return token;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::IssueAccessToken( const CORE::CString& subject )
{
    MT::CScopeMutex lock( m_lock );
    CORE::CString token = "access-" + MakeRandom();
    m_accessTokens[ token ] = subject;
    return token;
}

/*-------------------------------------------------------------------------*/

bool
CFakeOidcProvider::IsRefreshTokenValid( const CORE::CString& refreshToken ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_refreshTokens.find( refreshToken ) != m_refreshTokens.end();
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::RegisterVerifyToken( const CORE::CString& token, const CORE::CString& appId, const CORE::CString& userId, bool valid, Int64 expiresAt )
{
    MT::CScopeMutex lock( m_lock );
    SVerifyToken& entry = m_verifyTokens[ token ];
    entry.appId = appId;
    entry.userId = userId;
    entry.valid = valid;
    entry.expiresAt = expiresAt;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::SendJson( WEB::CHttpResponseData& response, int status, const CORE::CString& body )
{
    response.statusCode = status;
    response.contentType = "application/json";
    response.content.Clear( true );
    response.content.CopyFrom( AUTH::GetByteLength( body ), body.C_String() );
    response.cacheability = "no-store";
    response.keepConnectionsAlive = false;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::SendError( WEB::CHttpResponseData& response, int status, const CORE::CString& error )
{
    SendJson( response, status, "{\"error\":" + QuoteJson( error ) + "}" );
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::Redirect( WEB::CHttpResponseData& response, const CORE::CString& location )
{
    response.statusCode = 302;
    response.otherHeaders[ "Location" ] = location;
    response.keepConnectionsAlive = false;
}

/*-------------------------------------------------------------------------*/

/** Form bodies and query strings; JSON bodies with camel case names are turned into the snake case names of the standards */
bool
CFakeOidcProvider::ReadParameters( const WEB::CHttpRequestData& request, CORE::CString::StringMap& parameters ) const
{
    parameters.clear();
    bool duplicate = false;
    CORE::CString body = AUTH::MakeString( static_cast< const char* >( request.content.GetConstBufferPtr() ), request.content.GetDataSize() );
    if ( request.contentRepresentation.HasSubstr( "json" ) >= 0 )
    {
        CORE::CDataNode root;
        if ( !AUTH::CAuthJson::Parse( body, 65536, 6, false, root ) )
            return false;
        CORE::CString::StringMap raw;
        AUTH::CAuthJson::Flatten( AUTH::CAuthJson::GetDocument( root ), raw );
        for ( CORE::CString::StringMap::const_iterator i = raw.begin(); i != raw.end(); ++i )
        {
            CORE::CString snake;
            const char* p = (*i).first.C_String();
            for ( UInt32 c=0; c<AUTH::GetByteLength( (*i).first ); ++c )
            {
                if ( p[ c ] >= 'A' && p[ c ] <= 'Z' )
                {
                    snake += '_';
                    snake += (char) ( p[ c ] - 'A' + 'a' );
                }
                else
                    snake += p[ c ];
            }
            parameters[ snake ] = (*i).second;
        }
        return true;
    }
    if ( !body.IsNULLOrEmpty() )
        return AUTH::CAuthUrl::ParseQuery( body, parameters, duplicate );
    return AUTH::CAuthUrl::ParseQuery( request.requestUriParams, parameters, duplicate );
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::HandleRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    MT::CScopeMutex lock( m_lock );
    const CORE::CString& path = request.requestUri;
    bool post = request.requestType == "POST";

    if ( path.EndsWith( "/.well-known/openid-configuration" ) )
        HandleDiscovery( response );
    else
    if ( path == "/jwks" || path == "/jwks.json" || path.EndsWith( "/keys" ) )
        HandleJwks( response );
    else
    if ( path == "/authorize" )
        HandleAuthorize( request, response );
    else
    if ( path == "/token" && post )
        HandleToken( request, response );
    else
    if ( path == "/device" && post )
        HandleDevice( request, response );
    else
    if ( path == "/userinfo" )
        HandleUserInfo( request, response, "oidc" );
    else
    if ( path == "/me" )
        HandleUserInfo( request, response, "facebook" );
    else
    if ( path == "/profile" )
        HandleUserInfo( request, response, "amazon" );
    else
    if ( path == "/redirect" )
        Redirect( response, GetBaseUrl() + "/token" );
    else
    if ( path == "/revoke" && post )
        HandleRevoke( request, response );
    else
    if ( path == "/openid/login" )
        HandleOpenId( request, response );
    else
    if ( path == "/debug_token" )
        HandleVerify( request, response, true );
    else
    if ( path == "/tokeninfo" )
        HandleVerify( request, response, false );
    else
        response.statusCode = 404;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::HandleDiscovery( WEB::CHttpResponseData& response )
{
    ++m_stats.discovery;
    CORE::CString base = GetBaseUrl();
    CORE::CString issuer = m_issuerPath.HasSubstr( "{tid}" ) >= 0 ? base + m_issuerPath.ReplaceSubstr( "{tid}", "{tenantid}" ) : base + m_issuerPath;
    CORE::CString body = "{\"issuer\":" + QuoteJson( issuer ) +
                         ",\"authorization_endpoint\":" + QuoteJson( base + "/authorize" ) +
                         ",\"token_endpoint\":" + QuoteJson( base + "/token" ) +
                         ",\"device_authorization_endpoint\":" + QuoteJson( base + "/device" ) +
                         ",\"userinfo_endpoint\":" + QuoteJson( base + "/userinfo" ) +
                         ",\"jwks_uri\":" + QuoteJson( base + "/jwks" ) +
                         ",\"revocation_endpoint\":" + QuoteJson( base + "/revoke" ) +
                         ",\"response_types_supported\":[\"code\"],\"code_challenge_methods_supported\":[\"S256\"]}";
    SendJson( response, 200, body );
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::HandleJwks( WEB::CHttpResponseData& response )
{
    ++m_stats.jwks;
    if ( m_behavior.jwksDelayInMs > 0 )
        MT::ThreadDelay( m_behavior.jwksDelayInMs );
    CORE::CString body = "{\"keys\":[";
    bool first = true;
    for ( TKeyMap::const_iterator i = m_publishedKeys.begin(); i != m_publishedKeys.end(); ++i )
    {
        if ( !first )
            body += ",";
        first = false;
        body += (*i).second->ToJwk( (*i).first );
    }
    body += "]}";
    SendJson( response, 200, body );
    // what the test says, and nothing when it says nothing
    response.cacheability = m_behavior.jwksCacheControl;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::HandleAuthorize( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    ++m_stats.authorize;
    CORE::CString::StringMap parameters;
    bool duplicate = false;
    if ( !AUTH::CAuthUrl::ParseQuery( request.requestUriParams, parameters, duplicate ) || duplicate )
    {
        response.statusCode = 400;
        return;
    }
    m_lastAuthorize = parameters;

    CORE::CString redirectUri = parameters[ "redirect_uri" ];
    bool loopback = redirectUri.StartsWith( "http://127.0.0.1:" ) || redirectUri.StartsWith( "http://localhost:" );
    bool customScheme = redirectUri.StartsWith( "com.example.game:" );
    if ( parameters[ "response_type" ] != "code" || parameters[ "client_id" ] != m_clientId || ( !loopback && !customScheme ) || parameters[ "state" ].IsNULLOrEmpty() )
    {
        response.statusCode = 400;
        return;
    }
    CORE::CString location = redirectUri + ( redirectUri.HasChar( '?' ) >= 0 ? "&" : "?" );
    if ( m_behavior.denyAuthorization )
    {
        Redirect( response, location + "error=access_denied&state=" + EncodeUrlText( parameters[ "state" ] ) );
        return;
    }

    SCode code;
    code.challenge = parameters[ "code_challenge" ];
    if ( m_behavior.requirePkce && ( code.challenge.IsNULLOrEmpty() || parameters[ "code_challenge_method" ] != "S256" ) )
    {
        response.statusCode = 400;
        return;
    }
    code.redirectUri = redirectUri;
    code.clientId = parameters[ "client_id" ];
    code.nonce = parameters[ "nonce" ];
    code.subject = m_spec.subject;
    code.used = false;
    CORE::CString codeValue = "code-" + MakeRandom();
    m_codes[ codeValue ] = code;
    Redirect( response, location + "code=" + EncodeUrlText( codeValue ) + "&state=" + EncodeUrlText( parameters[ "state" ] ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeOidcProvider::IssueTokens( const SCode* code, const CORE::CString& subject, const CORE::CString& nonce, bool withIdToken, bool newRefreshToken, const CORE::CString& oldRefreshToken )
{
    CORE::CString accessToken = "access-" + MakeRandom();
    m_accessTokens[ accessToken ] = subject;

    CORE::CString body = "{\"access_token\":" + QuoteJson( accessToken ) + ",\"token_type\":\"Bearer\",\"expires_in\":3600,\"scope\":\"openid profile\"";
    if ( newRefreshToken )
    {
        CORE::CString refreshToken = "refresh-" + MakeRandom();
        m_refreshTokens[ refreshToken ] = subject;
        body += ",\"refresh_token\":" + QuoteJson( refreshToken );
    }
    if ( withIdToken )
    {
        CFakeTokenSpec spec = m_spec;
        spec.subject = subject;
        CORE::CString idToken = MintLocked( spec, nonce, accessToken );
        body += ",\"id_token\":" + QuoteJson( idToken );
    }
    body += "}";
    return body;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::HandleToken( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    ++m_stats.token;
    CORE::CString::StringMap parameters;
    if ( !ReadParameters( request, parameters ) )
    {
        SendError( response, 400, "invalid_request" );
        return;
    }
    m_lastToken = parameters;
    m_lastTokenContentType = request.contentRepresentation;
    CORE::CString::StringMap::const_iterator authorization = request.otherHeaders.find( "authorization" );
    m_lastTokenAuthorization = authorization == request.otherHeaders.end() ? CORE::CString::Empty : (*authorization).second;

    if ( m_behavior.tokenDelayInMs > 0 )
        MT::ThreadDelay( m_behavior.tokenDelayInMs );
    if ( !m_behavior.tokenError.IsNULLOrEmpty() )
    {
        SendError( response, m_behavior.tokenErrorStatus, m_behavior.tokenError );
        return;
    }

    CORE::CString grant = parameters[ "grant_type" ];
    if ( grant == "authorization_code" )
    {
        TCodeMap::iterator found = m_codes.find( parameters[ "code" ] );
        if ( found == m_codes.end() )
        {
            SendError( response, 400, "invalid_grant" );
            return;
        }
        SCode& code = (*found).second;
        if ( code.used )
        {
            ++m_stats.codeReuse;
            SendError( response, 400, "invalid_grant" );
            return;
        }
        code.used = true;
        if ( parameters[ "client_id" ] != code.clientId || parameters[ "redirect_uri" ] != code.redirectUri )
        {
            SendError( response, 400, "invalid_grant" );
            return;
        }
        if ( !code.challenge.IsNULLOrEmpty() || m_behavior.requirePkce )
        {
            // The verifier has to be a verifier, and hash to the challenge of the authorization request
            CORE::CString verifier = parameters[ "code_verifier" ];
            UInt32 size = AUTH::GetByteLength( verifier );
            bool valid = size >= 43 && size <= 128;
            const char* p = verifier.C_String();
            for ( UInt32 i=0; valid && i<size; ++i )
            {
                char c = p[ i ];
                valid = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '-' == c || '.' == c || '_' == c || '~' == c;
            }
            CORE::CString challenge;
            if ( valid )
            {
                UInt8 digest[ AUTH::CAuthCrypto::DigestSize ];
                AUTH::CAuthCrypto::Sha256( verifier.C_String(), size, digest );
                CORE::Base64UrlEncode( digest, AUTH::CAuthCrypto::DigestSize, challenge );
            }
            if ( !valid || challenge != code.challenge )
            {
                ++m_stats.pkceFailures;
                SendError( response, 400, "invalid_grant" );
                return;
            }
        }
        SendJson( response, 200, IssueTokens( &code, code.subject, code.nonce, m_behavior.issueIdToken, true, CORE::CString::Empty ) );
        return;
    }

    if ( grant == "refresh_token" )
    {
        CORE::CString refreshToken = parameters[ "refresh_token" ];
        TTextMap::iterator found = m_refreshTokens.find( refreshToken );
        if ( found == m_refreshTokens.end() )
        {
            SendError( response, 400, "invalid_grant" );
            return;
        }
        CORE::CString subject = (*found).second;
        if ( m_behavior.rotateRefreshTokens )
            m_refreshTokens.erase( found );
        SendJson( response, 200, IssueTokens( GUCEF_NULL, subject, CORE::CString::Empty, m_behavior.refreshIssuesIdToken, m_behavior.rotateRefreshTokens, refreshToken ) );
        return;
    }

    if ( grant == "urn:ietf:params:oauth:grant-type:device_code" )
    {
        ++m_stats.devicePolls;
        m_devicePollTimes.push_back( AUTH::GetMonotonicTimeInMs() );
        TDeviceMap::iterator found = m_devices.find( parameters[ "device_code" ] );
        if ( found == m_devices.end() )
        {
            SendError( response, 400, "invalid_grant" );
            return;
        }
        SDevice& device = (*found).second;
        ++device.polls;
        if ( m_behavior.deviceSlowDownAtPoll > 0 && device.polls == m_behavior.deviceSlowDownAtPoll && !device.slowDownSent )
        {
            device.slowDownSent = true;
            SendError( response, 400, "slow_down" );
            return;
        }
        if ( device.polls <= m_behavior.devicePendingPolls )
        {
            SendError( response, 400, "authorization_pending" );
            return;
        }
        if ( m_behavior.deviceOutcome == "denied" )
        {
            SendError( response, 400, "access_denied" );
            return;
        }
        if ( m_behavior.deviceOutcome == "expired" )
        {
            SendError( response, 400, "expired_token" );
            return;
        }
        // the device flow has no nonce
        CFakeTokenSpec saved = m_spec;
        m_spec.includeNonce = false;
        CORE::CString tokens = IssueTokens( GUCEF_NULL, m_spec.subject, CORE::CString::Empty, m_behavior.issueIdToken, true, CORE::CString::Empty );
        m_spec = saved;
        SendJson( response, 200, tokens );
        return;
    }
    SendError( response, 400, "unsupported_grant_type" );
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::HandleDevice( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    ++m_stats.device;
    CORE::CString::StringMap parameters;
    if ( !ReadParameters( request, parameters ) || parameters[ "client_id" ] != m_clientId )
    {
        SendError( response, 400, "invalid_client" );
        return;
    }
    CORE::CString deviceCode = "device-" + MakeRandom();
    SDevice device;
    device.polls = 0;
    device.slowDownSent = false;
    device.clientId = parameters[ "client_id" ];
    m_devices[ deviceCode ] = device;
    CORE::CString body = "{\"device_code\":" + QuoteJson( deviceCode ) +
                         ",\"user_code\":\"WDJB-MJHT\",\"verification_uri\":" + QuoteJson( GetBaseUrl() + "/device/activate" ) +
                         ",\"verification_uri_complete\":" + QuoteJson( GetBaseUrl() + "/device/activate?user_code=WDJB-MJHT" ) +
                         ",\"expires_in\":" + CORE::UInt32ToString( m_behavior.deviceExpiresIn );
    if ( !m_behavior.deviceOmitInterval )
        body += ",\"interval\":" + CORE::UInt32ToString( m_behavior.deviceInterval );
    body += "}";
    SendJson( response, 200, body );
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::HandleUserInfo( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response, const CORE::CString& shape )
{
    ++m_stats.userinfo;
    CORE::CString::StringMap::const_iterator authorization = request.otherHeaders.find( "authorization" );
    CORE::CString token = authorization == request.otherHeaders.end() ? CORE::CString::Empty : (*authorization).second;
    if ( token.StartsWith( "Bearer " ) )
        token = token.CutChars( 7, true );
    TTextMap::const_iterator found = m_accessTokens.find( token );
    TVerifyMap::const_iterator verify = m_verifyTokens.find( token );
    if ( m_behavior.userinfoFails || ( found == m_accessTokens.end() && verify == m_verifyTokens.end() ) )
    {
        SendError( response, 401, "invalid_token" );
        return;
    }
    CORE::CString subject = found != m_accessTokens.end() ? (*found).second : (*verify).second.userId;
    if ( !m_behavior.userinfoSubjectOverride.IsNULLOrEmpty() )
        subject = m_behavior.userinfoSubjectOverride;
    if ( shape == "facebook" )
        SendJson( response, 200, "{\"id\":" + QuoteJson( subject ) + ",\"name\":\"Fb Player\",\"picture\":{\"data\":{\"url\":\"https://pictures.invalid/fb.png\"}}}" );
    else
    if ( shape == "amazon" )
        SendJson( response, 200, "{\"user_id\":" + QuoteJson( subject ) + ",\"name\":\"Amazon Player\",\"email\":\"a@example.invalid\"}" );
    else
        SendJson( response, 200, "{\"sub\":" + QuoteJson( subject ) + ",\"name\":\"Info Player\",\"picture\":\"https://pictures.invalid/u.png\"}" );
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::HandleRevoke( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    ++m_stats.revoke;
    CORE::CString::StringMap parameters;
    ReadParameters( request, parameters );
    CORE::CString token = parameters[ "token" ];
    m_revoked.push_back( token );
    m_refreshTokens.erase( token );
    SendJson( response, 200, "{}" );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringMap
CFakeOidcProvider::MakeAssertion( const CORE::CString& returnTo )
{
    MT::CScopeMutex lock( m_lock );
    return MakeAssertionLocked( returnTo );
}

/*-------------------------------------------------------------------------*/

/** The return address as a provider that is not honest, or a relay that is not, would give it back */
static CORE::CString
EditReturnTo( const CORE::CString& returnTo, const CORE::CString& edit )
{
    if ( edit.IsNULLOrEmpty() )
        return returnTo;
    Int32 questionMark = returnTo.HasChar( '?' );
    if ( questionMark < 0 )
        return returnTo;
    CORE::CString base = returnTo.SubstrToIndex( (UInt32) questionMark, true );
    CORE::CString::StringMap parameters;
    bool duplicate = false;
    AUTH::CAuthUrl::ParseQuery( returnTo.CutChars( (UInt32) questionMark + 1, true ), parameters, duplicate );
    if ( edit == "port" )
        parameters[ "port" ] = CORE::ToString( CORE::StringToUInt16( parameters[ "port" ] ) + 1 );
    else
    if ( edit == "state" )
        parameters[ "s" ] = parameters[ "s" ] + "x";
    else
    if ( edit == "extra" )
        parameters[ "x" ] = "1";
    else
    if ( edit == "host" )
        base = base.ReplaceSubstr( AUTH::CAuthUrl::GetOrigin( base ), "https://evil.example" );
    else
    if ( edit == "path" )
        base += "x";
    else
    if ( edit == "scheme" )
        base = base.ReplaceSubstr( "https://", "http://" );
    else
    if ( edit == "loopback" )
        return "http://127.0.0.1:" + parameters[ "port" ] + "/callback?state=" + parameters[ "s" ];
    return base + "?" + AUTH::CAuthUrl::EncodeForm( parameters );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringMap
CFakeOidcProvider::MakeAssertionLocked( const CORE::CString& returnTo )
{
    // The time in the nonce, the way a provider writes it
    Int64 seconds = (Int64) ( AUTH::GetEpochTimeInMs() / 1000 ) - m_behavior.openIdNonceAgeInSeconds;
    CORE::CString nonce = FormatNonceTime( seconds ) + MakeRandom();

    CORE::CString base = GetBaseUrl();
    CORE::CString claimedId = m_behavior.openIdClaimedIdOverride.IsNULLOrEmpty() ? base + "/openid/id/" + m_behavior.openIdSubject : m_behavior.openIdClaimedIdOverride;
    CORE::CString::StringMap fields;
    fields[ "openid.ns" ] = AUTH::COpenId2Assertion::Namespace;
    fields[ "openid.mode" ] = "id_res";
    fields[ "openid.op_endpoint" ] = m_behavior.openIdOpEndpointOverride.IsNULLOrEmpty() ? base + "/openid/login" : m_behavior.openIdOpEndpointOverride;
    fields[ "openid.claimed_id" ] = claimedId;
    fields[ "openid.identity" ] = claimedId;
    fields[ "openid.return_to" ] = m_behavior.openIdReturnToOverride.IsNULLOrEmpty() ? EditReturnTo( returnTo, m_behavior.openIdReturnToEdit ) : m_behavior.openIdReturnToOverride;
    fields[ "openid.response_nonce" ] = nonce;
    fields[ "openid.assoc_handle" ] = "1234567890";
    fields[ "openid.signed" ] = m_behavior.openIdOmitSignedReturnTo ? "signed,op_endpoint,claimed_id,identity,response_nonce,assoc_handle" : "signed,op_endpoint,claimed_id,identity,return_to,response_nonce,assoc_handle";
    fields[ "openid.sig" ] = MakeRandom();
    if ( !m_behavior.openIdName.IsNULLOrEmpty() )
        fields[ "openid.test_name" ] = m_behavior.openIdName;
    m_openIdNonces[ nonce ] = fields[ "openid.sig" ];
    return fields;
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::HandleOpenId( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response )
{
    CORE::CString::StringMap parameters;
    if ( request.requestType == "POST" )
    {
        ++m_stats.openIdCheck;
        ReadParameters( request, parameters );
        m_lastOpenIdCheck = parameters;
        m_lastOpenIdCheckContentType = request.contentRepresentation;
        if ( 0 != m_behavior.openIdCheckStatus )
        {
            SendError( response, m_behavior.openIdCheckStatus, "unavailable" );
            return;
        }
        CORE::CString text = "ns:http://specs.openid.net/auth/2.0\n";
        bool valid = !m_behavior.openIdAlwaysInvalid && parameters[ "openid.mode" ] == "check_authentication";
        TTextMap::iterator nonce = m_openIdNonces.find( parameters[ "openid.response_nonce" ] );
        // The nonce is known, the signature is the one that was given, and it is used once
        if ( valid && ( nonce == m_openIdNonces.end() || (*nonce).second != parameters[ "openid.sig" ] ) )
            valid = false;
        if ( valid )
            m_openIdNonces.erase( nonce );
        text += valid ? "is_valid:true\n" : "is_valid:false\n";
        response.statusCode = 200;
        response.contentType = "text/plain";
        response.content.CopyFrom( AUTH::GetByteLength( text ), text.C_String() );
        response.keepConnectionsAlive = false;
        return;
    }

    ++m_stats.openIdLogin;
    bool duplicate = false;
    if ( !AUTH::CAuthUrl::ParseQuery( request.requestUriParams, parameters, duplicate ) || parameters[ "openid.mode" ] != "checkid_setup" ||
         parameters[ "openid.ns" ] != AUTH::COpenId2Assertion::Namespace )
    {
        response.statusCode = 400;
        return;
    }
    m_lastAuthorize = parameters;
    CORE::CString returnTo = parameters[ "openid.return_to" ];
    // OpenID 2.0 rules: the realm is the start of the return address, the return address is on the loopback address of the player or on https
    CORE::CString realm = parameters[ "openid.realm" ];
    if ( !( returnTo.StartsWith( "http://127.0.0.1:" ) || returnTo.StartsWith( "https://" ) ) || realm.IsNULLOrEmpty() || !returnTo.StartsWith( realm ) )
    {
        response.statusCode = 400;
        return;
    }
    if ( m_behavior.openIdCancel )
    {
        CORE::CString::StringMap cancelled;
        cancelled[ "openid.ns" ] = AUTH::COpenId2Assertion::Namespace;
        cancelled[ "openid.mode" ] = "cancel";
        Redirect( response, returnTo + ( returnTo.HasChar( '?' ) >= 0 ? "&" : "?" ) + AUTH::CAuthUrl::EncodeForm( cancelled ) );
        return;
    }
    CORE::CString::StringMap fields = MakeAssertionLocked( returnTo );
    CORE::CString location = returnTo + ( returnTo.HasChar( '?' ) >= 0 ? "&" : "?" ) + AUTH::CAuthUrl::EncodeForm( fields );
    Redirect( response, location );
}

/*-------------------------------------------------------------------------*/

void
CFakeOidcProvider::HandleVerify( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response, bool facebook )
{
    ++m_stats.verifyRequests;
    CORE::CString::StringMap parameters;
    bool duplicate = false;
    AUTH::CAuthUrl::ParseQuery( request.requestUriParams, parameters, duplicate );
    if ( facebook )
    {
        // debug_token: the app access token is APPID|SECRET
        CORE::CString appToken = parameters[ "access_token" ];
        if ( appToken.HasChar( '|' ) < 0 || appToken.SubstrToChar( '|', true ) != m_clientId )
        {
            SendJson( response, 400, "{\"error\":{\"message\":\"Invalid OAuth access token\",\"type\":\"OAuthException\",\"code\":190}}" );
            return;
        }
        TVerifyMap::const_iterator found = m_verifyTokens.find( parameters[ "input_token" ] );
        if ( found == m_verifyTokens.end() )
        {
            SendJson( response, 200, "{\"data\":{\"is_valid\":false,\"error\":{\"code\":190,\"message\":\"Invalid OAuth access token\"}}}" );
            return;
        }
        const SVerifyToken& token = (*found).second;
        SendJson( response, 200, CORE::CString( "{\"data\":{\"app_id\":" ) + QuoteJson( token.appId ) + ",\"type\":\"USER\",\"is_valid\":" + ( token.valid ? "true" : "false" ) +
                                 ",\"user_id\":" + QuoteJson( token.userId ) + ",\"expires_at\":" + CORE::Int64ToString( token.expiresAt ) + "}}" );
        return;
    }

    TVerifyMap::const_iterator found = m_verifyTokens.find( parameters[ "access_token" ] );
    if ( found == m_verifyTokens.end() || !(*found).second.valid )
    {
        SendError( response, 400, "invalid_token" );
        return;
    }
    SendJson( response, 200, "{\"iss\":\"https://www.amazon.com\",\"aud\":" + QuoteJson( (*found).second.appId ) + ",\"app_id\":\"amzn1.application.test\",\"user_id\":" + QuoteJson( (*found).second.userId ) + ",\"exp\":3000,\"iat\":600}" );
}

/*-------------------------------------------------------------------------*/
