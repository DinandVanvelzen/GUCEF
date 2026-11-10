/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefGUI_CSecretInputSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CSecretInputSettings::ConfigSectionName = "SecretInput";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSecretInputSettings::CSecretInputSettings( void )
    : CORE::CIConfigurable()
    , maxLength( DefaultMaxLength )
    , maskCodePoint( DefaultMaskCodePoint )
    , hideLength( true )
    , fixedMaskLength( DefaultFixedMaskLength )
    , allowCopy( false )
    , allowPaste( true )
    , allowReveal( false )
    , revealTimeoutInMs( DefaultRevealTimeoutInMs )
    , clearOnFocusLoss( false )
    , clearOnHide( true )
    , notifyCapsLock( false )
    , lastCharRevealInMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSecretInputSettings::CSecretInputSettings( const CSecretInputSettings& src )
    : CORE::CIConfigurable( src )
    , maxLength( src.maxLength )
    , maskCodePoint( src.maskCodePoint )
    , hideLength( src.hideLength )
    , fixedMaskLength( src.fixedMaskLength )
    , allowCopy( src.allowCopy )
    , allowPaste( src.allowPaste )
    , allowReveal( src.allowReveal )
    , revealTimeoutInMs( src.revealTimeoutInMs )
    , clearOnFocusLoss( src.clearOnFocusLoss )
    , clearOnHide( src.clearOnHide )
    , notifyCapsLock( src.notifyCapsLock )
    , lastCharRevealInMs( src.lastCharRevealInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSecretInputSettings::~CSecretInputSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSecretInputSettings&
CSecretInputSettings::operator=( const CSecretInputSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        maxLength = src.maxLength;
        maskCodePoint = src.maskCodePoint;
        hideLength = src.hideLength;
        fixedMaskLength = src.fixedMaskLength;
        allowCopy = src.allowCopy;
        allowPaste = src.allowPaste;
        allowReveal = src.allowReveal;
        revealTimeoutInMs = src.revealTimeoutInMs;
        clearOnFocusLoss = src.clearOnFocusLoss;
        clearOnHide = src.clearOnHide;
        notifyCapsLock = src.notifyCapsLock;
        lastCharRevealInMs = src.lastCharRevealInMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CSecretInputSettings::operator==( const CSecretInputSettings& other ) const
{GUCEF_TRACE;

    return maxLength == other.maxLength && maskCodePoint == other.maskCodePoint && hideLength == other.hideLength &&
           fixedMaskLength == other.fixedMaskLength && allowCopy == other.allowCopy && allowPaste == other.allowPaste &&
           allowReveal == other.allowReveal && revealTimeoutInMs == other.revealTimeoutInMs && clearOnFocusLoss == other.clearOnFocusLoss &&
           clearOnHide == other.clearOnHide && notifyCapsLock == other.notifyCapsLock && lastCharRevealInMs == other.lastCharRevealInMs;
}

/*-------------------------------------------------------------------------*/

void
CSecretInputSettings::Sanitize( void )
{GUCEF_TRACE;

    if ( 0 == maxLength )
        maxLength = DefaultMaxLength;
    if ( maxLength > AbsoluteMaxLength )
        maxLength = AbsoluteMaxLength;
    if ( maskCodePoint < 32 || 127 == maskCodePoint || ( maskCodePoint >= 0x80 && maskCodePoint < 0xA0 ) ||
         ( maskCodePoint >= 0xD800 && maskCodePoint <= 0xDFFF ) || maskCodePoint > 0x10FFFF )
        maskCodePoint = DefaultMaskCodePoint;
    if ( 0 == fixedMaskLength )
        fixedMaskLength = DefaultFixedMaskLength;
    if ( fixedMaskLength > AbsoluteMaxLength )
        fixedMaskLength = AbsoluteMaxLength;
    if ( 0 == revealTimeoutInMs )
        revealTimeoutInMs = DefaultRevealTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

bool
CSecretInputSettings::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;
    section->SetAttribute( "maxLength", maxLength );
    section->SetAttribute( "maskCodePoint", maskCodePoint );
    section->SetAttribute( "hideLength", hideLength );
    section->SetAttribute( "fixedMaskLength", fixedMaskLength );
    section->SetAttribute( "allowCopy", allowCopy );
    section->SetAttribute( "allowPaste", allowPaste );
    section->SetAttribute( "allowReveal", allowReveal );
    section->SetAttribute( "revealTimeoutInMs", revealTimeoutInMs );
    section->SetAttribute( "clearOnFocusLoss", clearOnFocusLoss );
    section->SetAttribute( "clearOnHide", clearOnHide );
    section->SetAttribute( "notifyCapsLock", notifyCapsLock );
    section->SetAttribute( "lastCharRevealInMs", lastCharRevealInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSecretInputSettings::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    maxLength = section->GetAttributeValueOrChildValueByName( "maxLength" ).AsUInt32( maxLength, true );
    maskCodePoint = section->GetAttributeValueOrChildValueByName( "maskCodePoint" ).AsUInt32( maskCodePoint, true );
    hideLength = section->GetAttributeValueOrChildValueByName( "hideLength" ).AsBool( hideLength, true );
    fixedMaskLength = section->GetAttributeValueOrChildValueByName( "fixedMaskLength" ).AsUInt32( fixedMaskLength, true );
    allowCopy = section->GetAttributeValueOrChildValueByName( "allowCopy" ).AsBool( allowCopy, true );
    allowPaste = section->GetAttributeValueOrChildValueByName( "allowPaste" ).AsBool( allowPaste, true );
    allowReveal = section->GetAttributeValueOrChildValueByName( "allowReveal" ).AsBool( allowReveal, true );
    revealTimeoutInMs = section->GetAttributeValueOrChildValueByName( "revealTimeoutInMs" ).AsUInt32( revealTimeoutInMs, true );
    clearOnFocusLoss = section->GetAttributeValueOrChildValueByName( "clearOnFocusLoss" ).AsBool( clearOnFocusLoss, true );
    clearOnHide = section->GetAttributeValueOrChildValueByName( "clearOnHide" ).AsBool( clearOnHide, true );
    notifyCapsLock = section->GetAttributeValueOrChildValueByName( "notifyCapsLock" ).AsBool( notifyCapsLock, true );
    lastCharRevealInMs = section->GetAttributeValueOrChildValueByName( "lastCharRevealInMs" ).AsUInt32( lastCharRevealInMs, true );
    Sanitize();
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSecretInputSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUI::CSecretInputSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
