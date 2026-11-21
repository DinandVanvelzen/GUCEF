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
#include "gucefASSETSTORE_CAssetFileSet.h"

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

CAssetFileSet::CAssetFileSet( void )
    : CORE::CIDataNodeSerializable()
    , assetId()
    , packId()
    , mainVfsPath()
    , mainStatus( ASSETSTORE_STATUS_NOT_FOUND )
    , mainMessage()
    , convertedVfsPath()
    , convertedStatus( ASSETSTORE_STATUS_NOT_FOUND )
    , convertedMessage()
    , files()
    , totalBytes( 0 )
    , relationCount( 0 )
    , companionsTruncated( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetFileSet::~CAssetFileSet()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetFileSet::Clear( void )
{GUCEF_TRACE;

    *this = CAssetFileSet();
}

/*-------------------------------------------------------------------------*/

bool
CAssetFileSet::Serialize( CORE::CDataNode& domRootNode                        ,
                         const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "assetId", assetId ) && success;
    success = domRootNode.SetAttribute( "packId", packId ) && success;
    success = domRootNode.SetAttribute( "mainVfsPath", mainVfsPath ) && success;
    success = domRootNode.SetAttribute( "mainStatus", CORE::CString( AssetStoreStatusToString( mainStatus ) ) ) && success;
    success = domRootNode.SetAttribute( "mainMessage", mainMessage ) && success;
    success = domRootNode.SetAttribute( "convertedVfsPath", convertedVfsPath ) && success;
    success = domRootNode.SetAttribute( "convertedStatus", CORE::CString( AssetStoreStatusToString( convertedStatus ) ) ) && success;
    success = domRootNode.SetAttribute( "convertedMessage", convertedMessage ) && success;
    CORE::CDataNode* filesNode = domRootNode.AddChild( "files", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; GUCEF_NULL != filesNode && i<files.size(); ++i )
        success = files[ i ].Serialize( *filesNode->AddChild( "file", GUCEF_DATATYPE_OBJECT ), settings ) && success;
    success = domRootNode.SetAttribute( "totalBytes", totalBytes ) && success;
    success = domRootNode.SetAttribute( "relationCount", relationCount ) && success;
    if ( companionsTruncated )
        success = domRootNode.SetAttribute( "companionsTruncated", true ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetFileSet::Deserialize( const CORE::CDataNode& domRootNode                  ,
                           const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    Clear();
    CORE::CString text;
    assetId = domRootNode.GetAttributeValueOrChildValueByName( "assetId" ).AsString( assetId );
    packId = domRootNode.GetAttributeValueOrChildValueByName( "packId" ).AsString( packId );
    mainVfsPath = domRootNode.GetAttributeValueOrChildValueByName( "mainVfsPath" ).AsString( mainVfsPath );
    text = domRootNode.GetAttributeValueOrChildValueByName( "mainStatus" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        mainStatus = AssetStoreStatusFromString( text );
    mainMessage = domRootNode.GetAttributeValueOrChildValueByName( "mainMessage" ).AsString( mainMessage );
    convertedVfsPath = domRootNode.GetAttributeValueOrChildValueByName( "convertedVfsPath" ).AsString( convertedVfsPath );
    text = domRootNode.GetAttributeValueOrChildValueByName( "convertedStatus" ).AsString();
    if ( !text.IsNULLOrEmpty() )
        convertedStatus = AssetStoreStatusFromString( text );
    convertedMessage = domRootNode.GetAttributeValueOrChildValueByName( "convertedMessage" ).AsString( convertedMessage );
    const CORE::CDataNode* filesNode = domRootNode.FindChild( "files" );
    if ( GUCEF_NULL != filesNode )
    {
        CORE::CDataNode::const_iterator i = filesNode->ConstBegin();
        while ( i != filesNode->ConstEnd() )
        {
            CAssetFileRecord entry;
            if ( entry.Deserialize( *(*i), settings ) )
                files.push_back( entry );
            ++i;
        }
    }
    totalBytes = domRootNode.GetAttributeValueOrChildValueByName( "totalBytes" ).AsUInt64( totalBytes, true );
    relationCount = domRootNode.GetAttributeValueOrChildValueByName( "relationCount" ).AsUInt32( relationCount, true );
    companionsTruncated = domRootNode.GetAttributeValueOrChildValueByName( "companionsTruncated" ).AsBool( false, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetFileSet::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetFileSet( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetFileSet::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetFileSet::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetFileSet";
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetFileSet::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetFileSet";
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
