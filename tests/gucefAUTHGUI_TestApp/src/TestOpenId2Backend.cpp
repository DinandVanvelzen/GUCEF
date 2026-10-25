/*
 *  gucefAUTHGUI_TestApp: Application for testing gucefAUTHGUI
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

#include "TestOpenId2Backend.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

const CORE::CString CTestOpenId2Profile::ProviderName = "testopenid";
const CORE::CString CTestOpenId2Profile::NameField = "openid.test_name";

static CTestOpenId2Backend* g_backend = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestOpenId2Profile::CTestOpenId2Profile( void )
    : AUTH::IOpenId2ProviderProfile()
    , relayAllowed( true )
    , readsName( false )
    , hintInstalled( false )
    , hintRunning( false )
    , hintName()
    , hintLookups( 0 )
    , forgets( 0 )
{

}

/*-------------------------------------------------------------------------*/

CTestOpenId2Profile::~CTestOpenId2Profile()
{

}

/*-------------------------------------------------------------------------*/

void
CTestOpenId2Profile::Reset( void )
{
    relayAllowed = true;
    readsName = false;
    hintInstalled = false;
    hintRunning = false;
    hintName.Clear();
    hintLookups = 0;
    forgets = 0;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTestOpenId2Profile::GetProviderName( void ) const
{
    return ProviderName;
}

/*-------------------------------------------------------------------------*/

bool
CTestOpenId2Profile::ExtractSubject( const CORE::CString& endpoint ,
                                     const CORE::CString& claimedId ,
                                     const CORE::CString& identity  ,
                                     CORE::CString& subject         ) const
{
    subject.Clear();
    if ( identity != claimedId )
        return false;
    CORE::CString prefix = AUTH::CAuthUrl::GetOrigin( endpoint ) + "/openid/id/";
    UInt32 prefixSize = AUTH::GetByteLength( prefix );
    UInt32 size = AUTH::GetByteLength( claimedId );
    if ( prefixSize <= 11 || size <= prefixSize || size > prefixSize + 32 || 0 != memcmp( claimedId.C_String(), prefix.C_String(), prefixSize ) )
        return false;
    const char* text = claimedId.C_String() + prefixSize;
    for ( UInt32 i=0; i<size-prefixSize; ++i )
        if ( !( ( text[ i ] >= '0' && text[ i ] <= '9' ) || ( text[ i ] >= 'a' && text[ i ] <= 'z' ) ) )
            return false;
    subject = AUTH::MakeString( text, size - prefixSize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTestOpenId2Profile::IsRelayAllowed( void ) const
{
    return relayAllowed;
}

/*-------------------------------------------------------------------------*/

bool
CTestOpenId2Profile::GetDisplayName( const AUTH::COpenId2Assertion& assertion, CORE::CString& displayName ) const
{
    displayName.Clear();
    if ( !readsName )
        return false;
    displayName = assertion.Get( NameField );
    return !displayName.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CTestOpenId2Profile::GetDetectedAccountHint( CORE::CString& displayName, bool& isRunning ) const
{
    ++hintLookups;
    displayName.Clear();
    isRunning = false;
    if ( !hintInstalled )
        return false;
    displayName = hintName;
    isRunning = hintRunning;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTestOpenId2Profile::ForgetDetectedAccountHint( void )
{
    ++forgets;
}

/*-------------------------------------------------------------------------*/

AUTH::CAuthProviderDescriptor
MakeTestOpenId2Descriptor( void )
{
    AUTH::CAuthProviderDescriptor d;
    d.name = CTestOpenId2Profile::ProviderName;
    d.displayName = "TestOpenId";
    d.accountProviderId = CTestOpenId2Profile::ProviderName;
    d.protocol = AUTH::AUTHPROTOCOL_OPENID2;
    d.idScope = AUTH::AUTHIDSCOPE_GLOBAL;
    d.capabilities = AUTH::AUTHCAP_BROWSER | AUTH::AUTHCAP_SERVER_VERIFIABLE;
    d.authorizationEndpoint = "https://openid.test.invalid/openid/login";
    d.subjectClaim = "testid";
    d.displayNameClaim = "name";
    d.publicClient = true;
    d.usesPkce = false;
    d.notes = "OpenID 2.0 provider of the tests";
    return d;
}

/*-------------------------------------------------------------------------*/

CTestOpenId2Backend::CTestOpenId2Backend( void )
    : m_profile()
{
    AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
    global->GetDescriptorRegistry().RegisterDescriptor( MakeTestOpenId2Descriptor() );
    global->GetOpenId2Profiles().Register( &m_profile );
    global->RegisterDescriptorProvider( CTestOpenId2Profile::ProviderName );
    global->GetManager().RefreshProviders();
}

/*-------------------------------------------------------------------------*/

CTestOpenId2Backend::~CTestOpenId2Backend()
{
    AUTH::CAuthGlobal* global = AUTH::CAuthGlobal::Instance();
    global->GetManager().RemoveProvider( CTestOpenId2Profile::ProviderName );
    global->UnregisterDescriptorProvider( CTestOpenId2Profile::ProviderName );
    global->GetOpenId2Profiles().Unregister( CTestOpenId2Profile::ProviderName );
    global->GetDescriptorRegistry().UnregisterDescriptor( CTestOpenId2Profile::ProviderName );
}

/*-------------------------------------------------------------------------*/

CTestOpenId2Profile&
CTestOpenId2Backend::GetProfile( void )
{
    return m_profile;
}

/*-------------------------------------------------------------------------*/

CTestOpenId2Profile&
TestOpenId2Profile( void )
{
    return g_backend->GetProfile();
}

/*-------------------------------------------------------------------------*/

void
SetTestOpenId2Backend( CTestOpenId2Backend* backend )
{
    g_backend = backend;
}

/*-------------------------------------------------------------------------*/
