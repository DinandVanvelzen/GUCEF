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
#include "gucefASSETSTORE_CAssetSettingsRequest.h"

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

CAssetSettingsRequest::CAssetSettingsRequest( void )
    : CORE::CIDataNodeSerializable()
    , values()
    , keys()
    , caller()
    , callerName()
    , remoteClient( false )
    , fullRead( true )
    , mayChange( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetSettingsRequest::~CAssetSettingsRequest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetSettingsRequest::SetRemoteCaller( const CORE::CString& deviceId, const CORE::CString& deviceName, bool readsAll, bool changes )
{GUCEF_TRACE;

    caller = "device:" + deviceId;
    callerName = deviceName;
    remoteClient = true;
    fullRead = readsAll;
    mayChange = changes;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsRequest::Serialize( CORE::CDataNode& domRootNode                        ,
                                  const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    WriteTextMap( domRootNode, "values", values );
    bool success = domRootNode.SetValuesOfChildByName( "keys", keys );
    success = domRootNode.SetAttribute( "caller", caller ) && success;
    success = domRootNode.SetAttribute( "callerName", callerName ) && success;
    success = domRootNode.SetAttribute( "remoteClient", remoteClient ) && success;
    success = domRootNode.SetAttribute( "fullRead", fullRead ) && success;
    success = domRootNode.SetAttribute( "mayChange", mayChange ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetSettingsRequest::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                    const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    *this = CAssetSettingsRequest();
    ReadTextMap( domRootNode, "values", values );
    domRootNode.GetValuesOfChildByName( "keys", keys );
    caller = domRootNode.GetAttributeValueOrChildValueByName( "caller" ).AsString( caller, true );
    callerName = domRootNode.GetAttributeValueOrChildValueByName( "callerName" ).AsString( callerName, true );
    remoteClient = domRootNode.GetAttributeValueOrChildValueByName( "remoteClient" ).AsBool( remoteClient, true );
    fullRead = domRootNode.GetAttributeValueOrChildValueByName( "fullRead" ).AsBool( fullRead, true );
    mayChange = domRootNode.GetAttributeValueOrChildValueByName( "mayChange" ).AsBool( mayChange, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetSettingsRequest::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetSettingsRequest( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingsRequest::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetSettingsRequest::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetSettingsRequest";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetSettingsRequest::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetSettingsRequest";
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
