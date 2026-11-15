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

#ifndef GUCEF_WEB_WEBVIEWTYPENAMES_H
#include "gucefWEB_WebViewTypeNames.h"
#define GUCEF_WEB_WEBVIEWTYPENAMES_H
#endif /* GUCEF_WEB_WEBVIEWTYPENAMES_H ? */

#include "gucefWEB_CWebViewPolicy.h"

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

CWebViewPolicy::CWebViewPolicy( void )
    : CORE::CIConfigurable()
    , navigation( WEBVIEWNAVIGATION_ANY )
    , allowList()
    , subframeHosts()
    , popups( WEBVIEWPOPUPS_REFUSE )
    , downloads( WEBVIEWDOWNLOADS_REFUSE )
    , permissions( WEBVIEWPERMISSIONS_DENYALL )
    , certificateErrors( WEBVIEWCERTERRORS_REFUSE )
    , scriptDialogs( WEBVIEWSCRIPTDIALOGS_SHOW )
    , defaultWorldScripts( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewPolicy::~CWebViewPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebViewPolicy::Sanitize( bool forSignIn )
{GUCEF_TRACE;

    bool unchanged = true;
    if ( forSignIn )
    {
        if ( WEBVIEWCERTERRORS_REFUSE != certificateErrors )
        {
            certificateErrors = WEBVIEWCERTERRORS_REFUSE;
            unchanged = false;
        }
        if ( defaultWorldScripts )
        {
            defaultWorldScripts = false;
            unchanged = false;
        }
    }
    return unchanged;
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewPolicy::DecideNavigation( const CString& url, bool mainFrame, bool byApplication ) const
{GUCEF_TRACE;

    if ( url.Equals( "about:blank", false ) )
        return WEBVIEWDECISION_ALLOW;
    if ( url.Lowercase().StartsWith( "data:" ) )
        return ( byApplication && mainFrame ) ? WEBVIEWDECISION_ALLOW : WEBVIEWDECISION_REFUSE;

    CString scheme, host;
    UInt16 port = 0;
    bool hasUserInfo = false;
    if ( !CWebViewHostAllowList::SplitUrl( url, scheme, host, port, hasUserInfo ) || hasUserInfo )
        return WEBVIEWDECISION_REFUSE;
    if ( "http" != scheme && "https" != scheme )
        return WEBVIEWDECISION_REFUSE;
    if ( WEBVIEWNAVIGATION_ANY == navigation )
        return WEBVIEWDECISION_ALLOW;
    if ( allowList.IsHostAllowed( scheme, host, port ) )
        return WEBVIEWDECISION_ALLOW;
    if ( !mainFrame && subframeHosts.IsHostAllowed( scheme, host, port ) )
        return WEBVIEWDECISION_ALLOW;
    return WEBVIEWDECISION_REFUSE;
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewPolicy::DecidePopup( const CString& url ) const
{GUCEF_TRACE;

    switch ( popups )
    {
        case WEBVIEWPOPUPS_SAMEVIEW:
            return WEBVIEWDECISION_ALLOW == DecideNavigation( url, true, false ) ? WEBVIEWDECISION_ALLOW : WEBVIEWDECISION_REFUSE;
        case WEBVIEWPOPUPS_ASK:
            return WEBVIEWDECISION_ALLOW == DecideNavigation( url, true, false ) ? WEBVIEWDECISION_ASK : WEBVIEWDECISION_REFUSE;
        default:
            return WEBVIEWDECISION_REFUSE;
    }
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewPolicy::DecideDownload( void ) const
{GUCEF_TRACE;

    return WEBVIEWDOWNLOADS_ASK == downloads ? WEBVIEWDECISION_ASK : WEBVIEWDECISION_REFUSE;
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewPolicy::DecidePermission( TWebViewPermissionKind kind ) const
{GUCEF_TRACE;

    return WEBVIEWPERMISSIONS_ASK == permissions ? WEBVIEWDECISION_ASK : WEBVIEWDECISION_REFUSE;
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewPolicy::DecideCertificateError( void ) const
{GUCEF_TRACE;

    return WEBVIEWCERTERRORS_ASK == certificateErrors ? WEBVIEWDECISION_ASK : WEBVIEWDECISION_REFUSE;
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewPolicy::DecideScriptDialog( void ) const
{GUCEF_TRACE;

    return WEBVIEWSCRIPTDIALOGS_SHOW == scriptDialogs ? WEBVIEWDECISION_ASK : WEBVIEWDECISION_REFUSE;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewPolicy::IsScriptWorldAllowed( const CString& world ) const
{GUCEF_TRACE;

    return !world.IsNULLOrEmpty() || defaultWorldScripts;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewPolicy::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool ok = config.SetAttribute( "navigation", WebViewNavigationPolicyToString( navigation ) );
    ok = config.SetAttribute( "popups", WebViewPopupPolicyToString( popups ) ) && ok;
    ok = config.SetAttribute( "downloads", WebViewDownloadPolicyToString( downloads ) ) && ok;
    ok = config.SetAttribute( "permissions", WebViewPermissionPolicyToString( permissions ) ) && ok;
    ok = config.SetAttribute( "certificateErrors", WebViewCertificateErrorPolicyToString( certificateErrors ) ) && ok;
    ok = config.SetAttribute( "scriptDialogs", WebViewScriptDialogPolicyToString( scriptDialogs ) ) && ok;
    ok = config.SetAttribute( "defaultWorldScripts", defaultWorldScripts ) && ok;
    CORE::CDataNode* allowNode = config.FindOrAddChild( "allowList" );
    ok = GUCEF_NULL != allowNode && allowList.SaveConfig( *allowNode ) && ok;
    CORE::CDataNode* subframeNode = config.FindOrAddChild( "subframeHosts" );
    ok = GUCEF_NULL != subframeNode && subframeHosts.SaveConfig( *subframeNode ) && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewPolicy::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    navigation = WebViewNavigationPolicyFromString( config.GetAttributeValueOrChildValueByName( "navigation" ).AsString(), navigation );
    popups = WebViewPopupPolicyFromString( config.GetAttributeValueOrChildValueByName( "popups" ).AsString(), popups );
    downloads = WebViewDownloadPolicyFromString( config.GetAttributeValueOrChildValueByName( "downloads" ).AsString(), downloads );
    permissions = WebViewPermissionPolicyFromString( config.GetAttributeValueOrChildValueByName( "permissions" ).AsString(), permissions );
    certificateErrors = WebViewCertificateErrorPolicyFromString( config.GetAttributeValueOrChildValueByName( "certificateErrors" ).AsString(), certificateErrors );
    scriptDialogs = WebViewScriptDialogPolicyFromString( config.GetAttributeValueOrChildValueByName( "scriptDialogs" ).AsString(), scriptDialogs );
    defaultWorldScripts = config.GetAttributeValueOrChildValueByName( "defaultWorldScripts" ).AsBool( defaultWorldScripts, true );
    const CORE::CDataNode* allowNode = config.FindChild( "allowList" );
    if ( GUCEF_NULL != allowNode )
        allowList.LoadConfig( *allowNode );
    const CORE::CDataNode* subframeNode = config.FindChild( "subframeHosts" );
    if ( GUCEF_NULL != subframeNode )
        subframeHosts.LoadConfig( *subframeNode );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewPolicy::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CWebViewPolicy";
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
