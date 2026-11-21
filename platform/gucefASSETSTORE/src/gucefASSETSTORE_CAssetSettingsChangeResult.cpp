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
#include "gucefASSETSTORE_CAssetSettingsChangeResult.h"

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

namespace {

/** A map of texts as an array of objects with "key" and "value": keys with dots survive every data tree codec */
void
WriteTextMap( CORE::CDataNode& parent, const char* name, const CORE::CString::StringMap& map )
{GUCEF_TRACE;

    CORE::CDataNode* list = parent.AddChild( name, GUCEF_DATATYPE_ARRAY );
    for ( CORE::CString::StringMap::const_iterator i = map.begin(); i != map.end(); ++i )
    {
        CORE::CDataNode* entry = list->AddChild( "entry", GUCEF_DATATYPE_OBJECT );
        entry->SetAttribute( "key", (*i).first );
        entry->SetAttribute( "value", (*i).second );
    }
}

void
ReadTextMap( const CORE::CDataNode& parent, const char* name, CORE::CString::StringMap& map )
{GUCEF_TRACE;

    map.clear();
    const CORE::CDataNode* list = parent.FindChild( name );
    if ( GUCEF_NULL == list )
        return;
    for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
    {
        const CORE::CString key = (*i)->GetAttributeValueOrChildValueByName( "key" ).AsString( CORE::CString::Empty, true );
        if ( !key.IsNULLOrEmpty() )
            map[ key ] = (*i)->GetAttributeValueOrChildValueByName( "value" ).AsString( CORE::CString::Empty, true );
    }
}

}

/*-------------------------------------------------------------------------*/

CAssetSettingsChangeResult::CAssetSettingsChangeResult( void )
    : CORE::CIDataNodeSerializable()
    , status( ASSETSTORE_STATUS_OK )
    , message()
    , operation()
    , errors()
    , changes()
    , noChange( false )
    , replayed( false )
    , caller()
    , callerName()
    , revision( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSettingsChangeResult::~CAssetSettingsChangeResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetSettingsChangeResult::Clear( void )
{GUCEF_TRACE;

    *this = CAssetSettingsChangeResult();
}

/*-------------------------------------------------------------------------*/

const CAssetSettingChange*
CAssetSettingsChangeResult::Find( const CORE::CString& key ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<changes.size(); ++i )
        if ( changes[ i ].key == key )
            return &changes[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsChangeResult::Serialize( CORE::CDataNode& domRootNode                        ,
                                       const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) );
    success = domRootNode.SetAttribute( "message", message ) && success;
    success = domRootNode.SetAttribute( "operation", operation ) && success;
    WriteTextMap( domRootNode, "errors", errors );
    CORE::CDataNode* list = domRootNode.AddChild( "changes", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == list )
        return false;
    for ( size_t i=0; i<changes.size(); ++i )
        success = changes[ i ].Serialize( *list->AddChild( "change", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetAttribute( "noChange", noChange ) && success;
    success = domRootNode.SetAttribute( "replayed", replayed ) && success;
    success = domRootNode.SetAttribute( "caller", caller ) && success;
    success = domRootNode.SetAttribute( "callerName", callerName ) && success;
    success = domRootNode.SetAttribute( "revision", revision ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsChangeResult::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                         const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    const CORE::CString statusText = domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString( CORE::CString::Empty, true );
    if ( !statusText.IsNULLOrEmpty() )
        status = AssetStoreStatusFromString( statusText );
    message = domRootNode.GetAttributeValueOrChildValueByName( "message" ).AsString( message, true );
    operation = domRootNode.GetAttributeValueOrChildValueByName( "operation" ).AsString( operation, true );
    ReadTextMap( domRootNode, "errors", errors );
    const CORE::CDataNode* list = domRootNode.FindChild( "changes" );
    if ( GUCEF_NULL != list )
    {
        for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
        {
            CAssetSettingChange change;
            if ( change.Deserialize( *(*i), settings ) )
                changes.push_back( change );
        }
    }
    noChange = domRootNode.GetAttributeValueOrChildValueByName( "noChange" ).AsBool( noChange, true );
    replayed = domRootNode.GetAttributeValueOrChildValueByName( "replayed" ).AsBool( replayed, true );
    caller = domRootNode.GetAttributeValueOrChildValueByName( "caller" ).AsString( caller, true );
    callerName = domRootNode.GetAttributeValueOrChildValueByName( "callerName" ).AsString( callerName, true );
    revision = domRootNode.GetAttributeValueOrChildValueByName( "revision" ).AsUInt64( revision, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSettingsChangeResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSettingsChangeResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingsChangeResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingsChangeResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetSettingsChangeResult";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSettingsChangeResult::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSettingsChangeResult";
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
