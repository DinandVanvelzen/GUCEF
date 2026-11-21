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
#include "gucefASSETSTORE_CAssetSettingChange.h"

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

const CORE::CString CAssetSettingChange::AppliedLive = "live";
const CORE::CString CAssetSettingChange::AppliedRestart = "restart";
const CORE::CString CAssetSettingChange::AppliedUnchanged = "unchanged";

/*-------------------------------------------------------------------------*/

CAssetSettingChange::CAssetSettingChange( void )
    : CORE::CIDataNodeSerializable()
    , key()
    , oldValue()
    , newValue()
    , applied( AppliedUnchanged )
    , valuesShown( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSettingChange::~CAssetSettingChange()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetSettingChange::HideValues( void )
{GUCEF_TRACE;

    oldValue.Clear();
    newValue.Clear();
    valuesShown = false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingChange::Serialize( CORE::CDataNode& domRootNode                        ,
                                const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "key", key );
    success = domRootNode.SetAttribute( "oldValue", oldValue ) && success;
    success = domRootNode.SetAttribute( "newValue", newValue ) && success;
    success = domRootNode.SetAttribute( "applied", applied ) && success;
    success = domRootNode.SetAttribute( "valuesShown", valuesShown ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingChange::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                  const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetSettingChange();
    key = domRootNode.GetAttributeValueOrChildValueByName( "key" ).AsString( key, true );
    oldValue = domRootNode.GetAttributeValueOrChildValueByName( "oldValue" ).AsString( oldValue, true );
    newValue = domRootNode.GetAttributeValueOrChildValueByName( "newValue" ).AsString( newValue, true );
    applied = domRootNode.GetAttributeValueOrChildValueByName( "applied" ).AsString( applied, true );
    valuesShown = domRootNode.GetAttributeValueOrChildValueByName( "valuesShown" ).AsBool( valuesShown, true );
    return !key.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSettingChange::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSettingChange( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingChange::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingChange::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetSettingChange";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSettingChange::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSettingChange";
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
