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
#include "gucefASSETSTORE_CAssetStoreFormat.h"

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

CAssetStoreFormat::CAssetStoreFormat( void )
    : CORE::CIDataNodeSerializable()
    , formatId()
    , name()
    , group()
    , files()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreFormat::~CAssetStoreFormat()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CAssetStoreFile*
CAssetStoreFormat::FindFile( const CORE::CString& fileId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<files.size(); ++i )
        if ( files[ i ].fileId == fileId )
            return &files[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetStoreFormat::GetTotalSize( void ) const
{GUCEF_TRACE;

    UInt64 total = 0;
    for ( size_t i=0; i<files.size(); ++i )
        total += files[ i ].size;
    return total;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreFormat::Serialize( CORE::CDataNode& domRootNode                        ,
                              const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "formatId", formatId );
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "group", group ) && success;
    CORE::CDataNode* filesNode = domRootNode.AddChild( "files", GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == filesNode )
        return false;
    for ( size_t i=0; i<files.size(); ++i )
    {
        CORE::CDataNode* fileNode = filesNode->AddChild( "file", GUCEF_DATATYPE_OBJECT );
        success = GUCEF_NULL != fileNode && files[ i ].Serialize( *fileNode, settings ) && success;
    }
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreFormat::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    formatId = domRootNode.GetAttributeValueOrChildValueByName( "formatId" ).AsString();
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString();
    group = domRootNode.GetAttributeValueOrChildValueByName( "group" ).AsString();
    files.clear();
    const CORE::CDataNode* filesNode = domRootNode.FindChild( "files" );
    if ( GUCEF_NULL != filesNode )
    {
        CORE::CDataNode::const_iterator i = filesNode->ConstBegin();
        while ( i != filesNode->ConstEnd() )
        {
            CAssetStoreFile file;
            if ( file.Deserialize( *(*i), settings ) )
                files.push_back( file );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreFormat::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreFormat( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreFormat::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreFormat";
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
