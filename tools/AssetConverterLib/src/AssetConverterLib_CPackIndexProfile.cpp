/*
 *  AssetConverterLib: the recognised pack profile as the pack index stores it
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#include "AssetConverterLib_CPackIndexProfile.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackIndexProfile::CPackIndexProfile( void )
    : packType()
    , packTypeName()
    , engine()
    , engineVersion()
    , recognisedBy()
    , confidence( 0 )
    , showcaseCandidates()
    , groupingHints()
    , noisePatterns()
    , groupRules()
    , presentationDefault()
    , recognitionKey()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPackIndexProfile::SetFromProfile( const CAssetPackProfile& profile )
{GUCEF_TRACE;

    packType = profile.packType;
    packTypeName = profile.packTypeName;
    engine = profile.engine;
    engineVersion = profile.engineVersion;
    recognisedBy = profile.recognisedBy;
    confidence = profile.confidence;
    showcaseCandidates = profile.showcaseCandidates;
    groupingHints = profile.groupingHints;
    noisePatterns = profile.noisePatterns;
}

/*-------------------------------------------------------------------------*/

void
CPackIndexProfile::GetProfile( CAssetPackProfile& profile ) const
{GUCEF_TRACE;

    profile.packType = packType;
    profile.packTypeName = packTypeName;
    profile.engine = engine;
    profile.engineVersion = engineVersion;
    profile.recognisedBy = recognisedBy;
    profile.confidence = confidence;
    profile.showcaseCandidates = showcaseCandidates;
    profile.groupingHints = groupingHints;
    profile.noisePatterns = noisePatterns;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexProfile::IsRecognised( void ) const
{GUCEF_TRACE;

    return !packType.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexProfile::operator==( const CPackIndexProfile& other ) const
{GUCEF_TRACE;

    return packType == other.packType && packTypeName == other.packTypeName && engine == other.engine && engineVersion == other.engineVersion &&
           recognisedBy == other.recognisedBy && confidence == other.confidence && showcaseCandidates == other.showcaseCandidates &&
           groupingHints == other.groupingHints && noisePatterns == other.noisePatterns && groupRules == other.groupRules &&
           presentationDefault == other.presentationDefault && recognitionKey == other.recognitionKey;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexProfile::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "packType", packType );
    CPackIndexSerial::SetText( node, "packTypeName", packTypeName );
    CPackIndexSerial::SetText( node, "engine", engine );
    CPackIndexSerial::SetText( node, "engineVersion", engineVersion );
    CPackIndexSerial::SetText( node, "recognisedBy", recognisedBy );
    node.SetAttribute( "confidence", confidence );
    CPackIndexSerial::SetList( node, "showcaseCandidates", showcaseCandidates );
    CPackIndexSerial::SetList( node, "groupingHints", groupingHints );
    CPackIndexSerial::SetList( node, "noisePatterns", noisePatterns );
    if ( !groupRules.empty() )
    {
        CORE::CDataNode* list = node.AddChild( "groupRules", GUCEF_DATATYPE_ARRAY );
        for ( size_t i=0; i<groupRules.size(); ++i )
            groupRules[ i ].Serialize( *list->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT ) );
    }
    if ( !presentationDefault.IsNULLOrEmpty() )
        CPackIndexSerial::SetText( *node.AddChild( "presentation", GUCEF_DATATYPE_OBJECT ), "default", presentationDefault );
    CPackIndexSerial::SetText( node, "recognitionKey", recognitionKey );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexProfile::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    *this = CPackIndexProfile();
    packType = CPackIndexSerial::GetText( node, "packType" );
    packTypeName = CPackIndexSerial::GetText( node, "packTypeName" );
    engine = CPackIndexSerial::GetText( node, "engine" );
    engineVersion = CPackIndexSerial::GetText( node, "engineVersion" );
    recognisedBy = CPackIndexSerial::GetText( node, "recognisedBy" );
    confidence = (UInt32) CPackIndexSerial::GetCount( node, "confidence" );
    CPackIndexSerial::GetList( node, "showcaseCandidates", showcaseCandidates );
    CPackIndexSerial::GetList( node, "groupingHints", groupingHints );
    CPackIndexSerial::GetList( node, "noisePatterns", noisePatterns );
    const CORE::CDataNode* list = node.FindChild( "groupRules" );
    if ( GUCEF_NULL != list )
    {
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        while ( i != list->ConstEnd() )
        {
            CPackGroupRule rule;
            if ( rule.Deserialize( *(*i) ) )
                groupRules.push_back( rule );
            ++i;
        }
    }
    const CORE::CDataNode* presentation = node.FindChild( "presentation" );
    if ( GUCEF_NULL != presentation )
        presentationDefault = CPackIndexSerial::GetText( *presentation, "default" );
    recognitionKey = CPackIndexSerial::GetText( node, "recognitionKey" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
