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

#include <math.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPENAMES_H
#include "gucefWEB_WebViewTypeNames.h"
#define GUCEF_WEB_WEBVIEWTYPENAMES_H
#endif /* GUCEF_WEB_WEBVIEWTYPENAMES_H ? */

#include "gucefWEB_CWebViewSettings.h"

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

const UInt32 CWebViewSettings::DefaultWidth;
const UInt32 CWebViewSettings::DefaultHeight;
const UInt32 CWebViewSettings::MaxSize;
const UInt32 CWebViewSettings::DefaultMaxScriptResultBytes;
const UInt32 CWebViewSettings::DefaultDecisionTimeoutMs;

/*-------------------------------------------------------------------------*/

CWebViewSettings::CWebViewSettings( void )
    : CORE::CIConfigurable()
    , profile()
    , mode( WEBVIEWMODE_OFFSCREEN )
    , width( DefaultWidth )
    , height( DefaultHeight )
    , deviceScale( 1.0f )
    , userAgent()
    , hardwareAcceleration( WEBVIEWHWACCEL_NEVER )
    , transparentBackground( false )
    , consoleMessages( false )
    , inspector( false )
    , maxScriptResultBytes( DefaultMaxScriptResultBytes )
    , decisionTimeoutMs( DefaultDecisionTimeoutMs )
    , policy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewSettings::~CWebViewSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebViewSettings::Sanitize( bool forSignIn )
{GUCEF_TRACE;

    bool unchanged = profile.Sanitize();
    if ( 0 == width || width > MaxSize )
    {
        width = 0 == width ? 1 : MaxSize;
        unchanged = false;
    }
    if ( 0 == height || height > MaxSize )
    {
        height = 0 == height ? 1 : MaxSize;
        unchanged = false;
    }
    // NaN fails every comparison and becomes 1 as well
    if ( !( deviceScale >= 0.25f && deviceScale <= 8.0f ) )
    {
        deviceScale = ( deviceScale > 8.0f ) ? 8.0f : ( deviceScale > 0.0f && deviceScale < 0.25f ? 0.25f : 1.0f );
        unchanged = false;
    }
    if ( forSignIn )
    {
        if ( inspector || consoleMessages )
        {
            inspector = false;
            consoleMessages = false;
            unchanged = false;
        }
        if ( !policy.Sanitize( true ) )
            unchanged = false;
    }
    return unchanged;
}

/*-------------------------------------------------------------------------*/

void
CWebViewSettings::ToDeviceSize( UInt32 logicalWidth, UInt32 logicalHeight, Float32 scale, UInt32& deviceWidth, UInt32& deviceHeight )
{GUCEF_TRACE;

    if ( !( scale > 0.0f ) )
        scale = 1.0f;
    deviceWidth = (UInt32) ceil( (Float64) logicalWidth * scale - 0.001 );
    deviceHeight = (UInt32) ceil( (Float64) logicalHeight * scale - 0.001 );
    if ( 0 == deviceWidth && 0 != logicalWidth )
        deviceWidth = 1;
    if ( 0 == deviceHeight && 0 != logicalHeight )
        deviceHeight = 1;
}

/*-------------------------------------------------------------------------*/

void
CWebViewSettings::GetDeviceSize( UInt32& deviceWidth, UInt32& deviceHeight ) const
{GUCEF_TRACE;

    ToDeviceSize( width, height, deviceScale, deviceWidth, deviceHeight );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    bool ok = config.SetAttribute( "mode", WebViewModeToString( mode ) );
    ok = config.SetAttribute( "width", width ) && ok;
    ok = config.SetAttribute( "height", height ) && ok;
    ok = config.SetAttribute( "deviceScale", deviceScale ) && ok;
    ok = config.SetAttribute( "userAgent", userAgent ) && ok;
    ok = config.SetAttribute( "hardwareAcceleration", WebViewHardwareAccelerationToString( hardwareAcceleration ) ) && ok;
    ok = config.SetAttribute( "transparentBackground", transparentBackground ) && ok;
    ok = config.SetAttribute( "consoleMessages", consoleMessages ) && ok;
    ok = config.SetAttribute( "inspector", inspector ) && ok;
    ok = config.SetAttribute( "maxScriptResultBytes", maxScriptResultBytes ) && ok;
    ok = config.SetAttribute( "decisionTimeoutMs", decisionTimeoutMs ) && ok;
    CORE::CDataNode* profileNode = config.FindOrAddChild( "profile" );
    ok = GUCEF_NULL != profileNode && profile.SaveConfig( *profileNode ) && ok;
    CORE::CDataNode* policyNode = config.FindOrAddChild( "policy" );
    ok = GUCEF_NULL != policyNode && policy.SaveConfig( *policyNode ) && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    mode = WebViewModeFromString( config.GetAttributeValueOrChildValueByName( "mode" ).AsString(), mode );
    width = config.GetAttributeValueOrChildValueByName( "width" ).AsUInt32( width, true );
    height = config.GetAttributeValueOrChildValueByName( "height" ).AsUInt32( height, true );
    deviceScale = config.GetAttributeValueOrChildValueByName( "deviceScale" ).AsFloat32( deviceScale, true );
    userAgent = config.GetAttributeValueOrChildValueByName( "userAgent" ).AsString( userAgent, false );
    hardwareAcceleration = WebViewHardwareAccelerationFromString( config.GetAttributeValueOrChildValueByName( "hardwareAcceleration" ).AsString(), hardwareAcceleration );
    transparentBackground = config.GetAttributeValueOrChildValueByName( "transparentBackground" ).AsBool( transparentBackground, true );
    consoleMessages = config.GetAttributeValueOrChildValueByName( "consoleMessages" ).AsBool( consoleMessages, true );
    inspector = config.GetAttributeValueOrChildValueByName( "inspector" ).AsBool( inspector, true );
    maxScriptResultBytes = config.GetAttributeValueOrChildValueByName( "maxScriptResultBytes" ).AsUInt32( maxScriptResultBytes, true );
    decisionTimeoutMs = config.GetAttributeValueOrChildValueByName( "decisionTimeoutMs" ).AsUInt32( decisionTimeoutMs, true );
    const CORE::CDataNode* profileNode = config.FindChild( "profile" );
    if ( GUCEF_NULL != profileNode )
        profile.LoadConfig( *profileNode );
    const CORE::CDataNode* policyNode = config.FindChild( "policy" );
    if ( GUCEF_NULL != policyNode )
        policy.LoadConfig( *policyNode );
    Sanitize( false );
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CWebViewSettings";
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
