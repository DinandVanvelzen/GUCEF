/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPENAMES_H
#include "gucefWEB_WebViewTypeNames.h"
#define GUCEF_WEB_WEBVIEWTYPENAMES_H
#endif /* GUCEF_WEB_WEBVIEWTYPENAMES_H ? */

#include "gucefWEB_CWebViewProfileSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CWebViewProfileSettings::MaxProfileIdLength;

/*-------------------------------------------------------------------------*/

CWebViewProfileSettings::CWebViewProfileSettings( void )
    : CORE::CIConfigurable()
    , profileId( "default" )
    , ephemeral( false )
    , dataDirectory()
    , proxyMode( WEBVIEWPROXY_SYSTEM )
    , proxyHost()
    , proxyPort( 0 )
    , proxyIgnoreHosts()
    , thirdPartyCookies( WEBVIEWTHIRDPARTYCOOKIES_BLOCK )
    , trackingPrevention( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewProfileSettings::~CWebViewProfileSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebViewProfileSettings::IsValidProfileId( const CString& id )
{GUCEF_TRACE;

    if ( id.IsNULLOrEmpty() || "." == id || ".." == id )
        return false;
    const UInt32 count = id.ByteSize() - 1;
    if ( count > MaxProfileIdLength )
        return false;
    const char* c = id.C_String();
    for ( UInt32 i=0; i<count; ++i )
    {
        const char ch = c[ i ];
        const bool valid = ( ch >= 'a' && ch <= 'z' ) || ( ch >= 'A' && ch <= 'Z' ) || ( ch >= '0' && ch <= '9' ) || '.' == ch || '_' == ch || '-' == ch;
        if ( !valid )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewProfileSettings::Sanitize( void )
{GUCEF_TRACE;

    bool unchanged = true;
    if ( !IsValidProfileId( profileId ) )
    {
        profileId = "default";
        unchanged = false;
    }
    if ( WEBVIEWPROXY_EXPLICIT == proxyMode && ( proxyHost.IsNULLOrEmpty() || 0 == proxyPort ) )
    {
        proxyMode = WEBVIEWPROXY_NONE;
        unchanged = false;
    }
    return unchanged;
}

/*-------------------------------------------------------------------------*/

CString
CWebViewProfileSettings::ResolveDataDirectory( const CString& engineName, const CString& applicationName ) const
{GUCEF_TRACE;

    if ( ephemeral )
        return CString::Empty;
    if ( !dataDirectory.IsNULLOrEmpty() )
        return CORE::RelativePath( dataDirectory );
    CString path = "$APPDATADIR$";
    CORE::AppendToPath( path, applicationName.IsNULLOrEmpty() ? CString( "GUCEF" ) : applicationName );
    CORE::AppendToPath( path, "WebView" );
    CORE::AppendToPath( path, engineName );
    CORE::AppendToPath( path, IsValidProfileId( profileId ) ? profileId : CString( "default" ) );
    return CORE::RelativePath( path );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewProfileSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool ok = config.SetAttribute( "profileId", profileId );
    ok = config.SetAttribute( "ephemeral", ephemeral ) && ok;
    ok = config.SetAttribute( "dataDirectory", dataDirectory ) && ok;
    ok = config.SetAttribute( "proxy", WebViewProxyModeToString( proxyMode ) ) && ok;
    ok = config.SetAttribute( "proxyHost", proxyHost ) && ok;
    ok = config.SetAttribute( "proxyPort", proxyPort ) && ok;
    ok = config.SetAttribute( "proxyIgnoreHosts", CString::Empty.Combine( proxyIgnoreHosts, ',' ) ) && ok;
    ok = config.SetAttribute( "thirdPartyCookies", WebViewThirdPartyCookiesToString( thirdPartyCookies ) ) && ok;
    ok = config.SetAttribute( "trackingPrevention", trackingPrevention ) && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewProfileSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    profileId = config.GetAttributeValueOrChildValueByName( "profileId" ).AsString( profileId, true );
    ephemeral = config.GetAttributeValueOrChildValueByName( "ephemeral" ).AsBool( ephemeral, true );
    dataDirectory = config.GetAttributeValueOrChildValueByName( "dataDirectory" ).AsString( dataDirectory, false );
    proxyMode = WebViewProxyModeFromString( config.GetAttributeValueOrChildValueByName( "proxy" ).AsString(), proxyMode );
    proxyHost = config.GetAttributeValueOrChildValueByName( "proxyHost" ).AsString( proxyHost, true );
    proxyPort = (UInt16) config.GetAttributeValueOrChildValueByName( "proxyPort" ).AsUInt32( proxyPort, true );
    const CString ignore = config.GetAttributeValueOrChildValueByName( "proxyIgnoreHosts" ).AsString();
    if ( !ignore.IsNULLOrEmpty() )
        proxyIgnoreHosts = ignore.ParseElements( ',', false );
    thirdPartyCookies = WebViewThirdPartyCookiesFromString( config.GetAttributeValueOrChildValueByName( "thirdPartyCookies" ).AsString(), thirdPartyCookies );
    trackingPrevention = config.GetAttributeValueOrChildValueByName( "trackingPrevention" ).AsBool( trackingPrevention, true );
    Sanitize();
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewProfileSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CWebViewProfileSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
