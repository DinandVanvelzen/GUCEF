/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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
#include "gucefASSETSTORE_CAssetSettingState.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetSettingState::SourceSite = "site";
const CORE::CString CAssetSettingState::SourceOverridden = "overridden";
const CORE::CString CAssetSettingState::SourceDefault = "default";

/*-------------------------------------------------------------------------*/

CAssetSettingState::CAssetSettingState( void )
    : CORE::CIDataNodeSerializable()
    , descriptor()
    , value()
    , hasPendingValue( false )
    , pendingValue()
    , siteValue()
    , source( SourceDefault )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSettingState::~CAssetSettingState()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingState::Serialize( CORE::CDataNode& domRootNode                        ,
                               const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = descriptor.Serialize( domRootNode, settings );
    success = domRootNode.SetAttribute( "value", value ) && success;
    success = domRootNode.SetAttribute( "hasPendingValue", hasPendingValue ) && success;
    success = domRootNode.SetAttribute( "pendingValue", pendingValue ) && success;
    success = domRootNode.SetAttribute( "siteValue", siteValue ) && success;
    success = domRootNode.SetAttribute( "source", source ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingState::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                 const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetSettingState();
    const bool success = descriptor.Deserialize( domRootNode, settings );
    value = domRootNode.GetAttributeValueOrChildValueByName( "value" ).AsString( value, true );
    hasPendingValue = domRootNode.GetAttributeValueOrChildValueByName( "hasPendingValue" ).AsBool( hasPendingValue, true );
    pendingValue = domRootNode.GetAttributeValueOrChildValueByName( "pendingValue" ).AsString( pendingValue, true );
    siteValue = domRootNode.GetAttributeValueOrChildValueByName( "siteValue" ).AsString( siteValue, true );
    source = domRootNode.GetAttributeValueOrChildValueByName( "source" ).AsString( source, true );
    return success;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSettingState::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSettingState( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingState::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingState::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetSettingState";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSettingState::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSettingState";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
