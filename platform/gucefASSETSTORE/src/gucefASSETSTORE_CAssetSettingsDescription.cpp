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
#include "gucefASSETSTORE_CAssetSettingsDescription.h"

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

CAssetSettingsDescription::CAssetSettingsDescription( void )
    : CORE::CIDataNodeSerializable()
    , status( ASSETSTORE_STATUS_OK )
    , message()
    , changesAllowed( false )
    , callerMayChange( false )
    , canChange( false )
    , changeRefusal()
    , persistent( false )
    , revision( 0 )
    , entries()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSettingsDescription::~CAssetSettingsDescription()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetSettingsDescription::Clear( void )
{GUCEF_TRACE;

    *this = CAssetSettingsDescription();
}

/*-------------------------------------------------------------------------*/

const CAssetSettingState*
CAssetSettingsDescription::Find( const CORE::CString& key ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<entries.size(); ++i )
        if ( entries[ i ].descriptor.key == key )
            return &entries[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsDescription::Serialize( CORE::CDataNode& domRootNode                        ,
                                      const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) );
    success = domRootNode.SetAttribute( "message", message ) && success;
    success = domRootNode.SetAttribute( "changesAllowed", changesAllowed ) && success;
    success = domRootNode.SetAttribute( "callerMayChange", callerMayChange ) && success;
    success = domRootNode.SetAttribute( "canChange", canChange ) && success;
    success = domRootNode.SetAttribute( "changeRefusal", changeRefusal ) && success;
    success = domRootNode.SetAttribute( "persistent", persistent ) && success;
    success = domRootNode.SetAttribute( "revision", revision ) && success;
    CORE::CDataNode* list = domRootNode.AddChild( "entries", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == list )
        return false;
    for ( size_t i=0; i<entries.size(); ++i )
        success = entries[ i ].Serialize( *list->AddChild( "entry", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsDescription::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                        const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    const CORE::CString statusText = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString( CORE::CString::Empty, true );
    if ( !statusText.IsNULLOrEmpty() )
        status = AssetStoreStatusFromString( statusText );
    message = domRootNode.GetAttributeValueOrChildValueByName( "message" ).AsString( message, true );
    changesAllowed = domRootNode.GetAttributeValueOrChildValueByName( "changesAllowed" ).AsBool( changesAllowed, true );
    callerMayChange = domRootNode.GetAttributeValueOrChildValueByName( "callerMayChange" ).AsBool( callerMayChange, true );
    canChange = domRootNode.GetAttributeValueOrChildValueByName( "canChange" ).AsBool( canChange, true );
    changeRefusal = domRootNode.GetAttributeValueOrChildValueByName( "changeRefusal" ).AsString( changeRefusal, true );
    persistent = domRootNode.GetAttributeValueOrChildValueByName( "persistent" ).AsBool( persistent, true );
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt64( revision, true );
    const CORE::CDataNode* list = domRootNode.FindChild( "entries" );
    if ( GUCEF_NULL != list )
    {
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
        {
            CAssetSettingState entry;
            if ( entry.Deserialize( *(*i), settings ) )
                entries.push_back( entry );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSettingsDescription::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSettingsDescription( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingsDescription::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingsDescription::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetSettingsDescription";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSettingsDescription::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSettingsDescription";
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
