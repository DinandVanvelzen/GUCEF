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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */
#ifndef GUCEF_WEB_CWEBJSON_H
#include "gucefWEB_CWebJson.h"
#define GUCEF_WEB_CWEBJSON_H
#endif /* GUCEF_WEB_CWEBJSON_H ? */
#ifndef GUCEF_AUTH_CWEBSESSION_H
#include "gucefAUTH_CWebSession.h"
#define GUCEF_AUTH_CWEBSESSION_H
#endif /* GUCEF_AUTH_CWEBSESSION_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CWebSession::SessionProviderName = "assetstore-web-session";
const CORE::CString CWebSession::FormatName = "gucef.webSession";

/*-------------------------------------------------------------------------*/

namespace {

/** Overwrites the text that held a secret before it is let go */
void
WipeText( CORE::CString& text )
{
    if ( !text.IsNULLOrEmpty() )
    {
        char* buffer = text.C_String();
        if ( GUCEF_NULL != buffer )
            memset( buffer, 0, text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
    }
    text.Clear();
}

}

/*-------------------------------------------------------------------------*/

CWebSession::CWebSession( void )
    : storeId()
    , capturedAtUtcInMs( 0 )
    , cookies()
    , userAgent()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebSession::~CWebSession()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

UInt64
CWebSession::NowUtcInMs( void )
{GUCEF_TRACE;

    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

bool
CWebSession::IsEmpty( void ) const
{GUCEF_TRACE;

    return cookies.empty();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSession::GetCookieCount( void ) const
{GUCEF_TRACE;

    return (UInt32) cookies.size();
}

/*-------------------------------------------------------------------------*/

bool
CWebSession::HasCookie( const CORE::CString& name ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<cookies.size(); ++i )
        if ( cookies[ i ].name == name && !cookies[ i ].value.IsEmpty() )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWebSession::Clear( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<cookies.size(); ++i )
        cookies[ i ].value.Clear();
    cookies.clear();
    capturedAtUtcInMs = 0;
    userAgent.Clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSession::RemoveExpiredCookies( UInt64 nowUtcInMs )
{GUCEF_TRACE;

    UInt32 removed = 0;
    TWebSessionCookieVector::iterator i = cookies.begin();
    while ( i != cookies.end() )
    {
        if ( (*i).IsExpired( nowUtcInMs ) )
        {
            (*i).value.Clear();
            i = cookies.erase( i );
            ++removed;
        }
        else
            ++i;
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

bool
CWebSession::ToSecret( AUTH::CSecretString& secret ) const
{GUCEF_TRACE;

    CORE::CDataNode tree( GUCEF_DATATYPE_OBJECT );
    tree.SetAttribute( "format", FormatName );
    tree.SetAttribute( "version", (UInt32) 1 );
    tree.SetAttribute( "storeId", storeId );
    tree.SetAttribute( "capturedAtMs", capturedAtUtcInMs );
    if ( !userAgent.IsNULLOrEmpty() )
        tree.SetAttribute( "userAgent", userAgent );
    CORE::CDataNode* list = tree.AddChild( "cookies", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<cookies.size(); ++i )
    {
        CORE::CDataNode* cookieNode = list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
        cookies[ i ].WriteTo( *cookieNode );
    }

    CORE::CString text;
    const bool success = WEB::CWebJson::ToText( tree, text );
    if ( success )
        secret.Set( text );
    WipeText( text );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CWebSession::FromSecret( const AUTH::CSecretString& secret )
{GUCEF_TRACE;

    Clear();
    CORE::CDataNode tree;
    if ( secret.IsEmpty() || !WEB::CWebJson::FromText( secret.Reveal(), tree ) )
        return false;
    const CORE::CDataNode* root = &tree;
    if ( 0 == tree.GetAttCount() && 1 == tree.GetNrOfDirectChildNodes() && GUCEF_NULL == tree.FindChild( "cookies" ) )
        root = *tree.ConstBegin();
    if ( root->GetAttributeValue( "format" ).AsString() != FormatName )
        return false;

    storeId = root->GetAttributeValue( "storeId" ).AsString();
    capturedAtUtcInMs = root->GetAttributeValue( "capturedAtMs" ).AsUInt64( 0 );
    userAgent = root->GetAttributeValue( "userAgent" ).AsString();
    const CORE::CDataNode* list = root->FindChild( "cookies" );
    if ( GUCEF_NULL != list )
    {
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
        {
            CWebSessionCookie cookie;
            if ( cookie.ReadFrom( *(*i) ) )
                cookies.push_back( cookie );
        }
    }
    RemoveExpiredCookies( NowUtcInMs() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSession::SaveTo( AUTH::ICredentialStore& store ) const
{GUCEF_TRACE;

    if ( storeId.IsNULLOrEmpty() )
        return false;
    AUTH::CSecretString secret;
    if ( !ToSecret( secret ) )
        return false;
    return store.Store( SessionProviderName, storeId, secret );
}

/*-------------------------------------------------------------------------*/

bool
CWebSession::LoadFrom( AUTH::ICredentialStore& store, const CORE::CString& storeIdToLoad )
{GUCEF_TRACE;

    Clear();
    AUTH::CSecretString secret;
    if ( !store.Load( SessionProviderName, storeIdToLoad, secret ) )
        return false;
    if ( !FromSecret( secret ) || storeId != storeIdToLoad )
    {
        Clear();
        return false;
    }
    return !IsEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CWebSession::RemoveFrom( AUTH::ICredentialStore& store, const CORE::CString& storeIdToRemove )
{GUCEF_TRACE;

    return store.Remove( SessionProviderName, storeIdToRemove );
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSession::ApplyTo( WEB::CHttpCookieJar& jar ) const
{GUCEF_TRACE;

    const UInt64 now = NowUtcInMs();
    UInt32 applied = 0;
    for ( size_t i=0; i<cookies.size(); ++i )
    {
        if ( cookies[ i ].IsExpired( now ) )
            continue;
        WEB::CHttpCookie cookie;
        cookies[ i ].ToHttpCookie( cookie );
        jar.SetCookie( cookie );
        WipeText( cookie.value );
        ++applied;
    }
    return applied;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebSession::Describe( void ) const
{GUCEF_TRACE;

    CORE::CString text = "web session of " + ( storeId.IsNULLOrEmpty() ? CORE::CString( "an unnamed store" ) : storeId ) +
                         ": " + CORE::ToString( GetCookieCount() ) + " cookies";
    if ( capturedAtUtcInMs > 0 )
    {
        CORE::CDateTime captured;
        captured.FromUnixEpochBasedTicksInMillisecs( capturedAtUtcInMs );
        text += ", captured " + captured.ToIso8601DateTimeString( true, true );
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CORE::CVariantStream&
operator<<( CORE::CVariantStream& stream, const CWebSession& session )
{GUCEF_TRACE;

    stream << session.Describe();
    return stream;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
