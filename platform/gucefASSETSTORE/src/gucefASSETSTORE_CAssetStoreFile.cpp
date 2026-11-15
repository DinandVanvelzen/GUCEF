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
#include "gucefASSETSTORE_CAssetStoreFile.h"

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

CAssetStoreFile::CAssetStoreFile( void )
    : CORE::CIDataNodeSerializable()
    , fileId()
    , name()
    , size( 0 )
    , digest()
    , digestAlgorithm()
    , isArchive( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreFile::CAssetStoreFile( const CAssetStoreFile& src )
    : CORE::CIDataNodeSerializable( src )
    , fileId( src.fileId )
    , name( src.name )
    , size( src.size )
    , digest( src.digest )
    , digestAlgorithm( src.digestAlgorithm )
    , isArchive( src.isArchive )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreFile::~CAssetStoreFile()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreFile&
CAssetStoreFile::operator=( const CAssetStoreFile& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIDataNodeSerializable::operator=( src );
        fileId = src.fileId;
        name = src.name;
        size = src.size;
        digest = src.digest;
        digestAlgorithm = src.digestAlgorithm;
        isArchive = src.isArchive;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreFile::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "fileId", fileId );
    success = domRootNode.SetAttribute( "name", name ) && success;
    success = domRootNode.SetAttribute( "size", size ) && success;
    if ( !digest.IsNULLOrEmpty() )
    {
        success = domRootNode.SetAttribute( "digest", digest ) && success;
        success = domRootNode.SetAttribute( "digestAlgorithm", digestAlgorithm ) && success;
    }
    if ( isArchive )
        success = domRootNode.SetAttribute( "isArchive", true ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreFile::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    fileId = domRootNode.GetAttributeValueOrChildValueByName( "fileId" ).AsString();
    name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString();
    size = domRootNode.GetAttributeValueOrChildValueByName( "size" ).AsUInt64( 0, true );
    digest = domRootNode.GetAttributeValueOrChildValueByName( "digest" ).AsString().Lowercase();
    digestAlgorithm = domRootNode.GetAttributeValueOrChildValueByName( "digestAlgorithm" ).AsString().Lowercase();
    isArchive = domRootNode.GetAttributeValueOrChildValueByName( "isArchive" ).AsBool( false, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreFile::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreFile( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreFile::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreFile";
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
