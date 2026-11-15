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
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#include "gucefASSETSTORE_CAssetStoreDownloadJob.h"

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

CAssetStoreDownloadJob::CAssetStoreDownloadJob( void )
    : CORE::CIDataNodeSerializable()
    , jobId()
    , sequence( 0 )
    , providerId()
    , itemId()
    , itemTitle()
    , formatId()
    , fileIds()
    , destination()
    , forIngest( false )
    , state( ASSETDOWNLOAD_STATE_QUEUED )
    , status( ASSETSTORE_STATUS_OK )
    , message()
    , files()
    , attempts( 0 )
    , createdAtMs( 0 )
    , updatedAtMs( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetStoreDownloadJob::~CAssetStoreDownloadJob()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CAssetStoreDownloadJob::GetTotalBytes( void ) const
{GUCEF_TRACE;

    UInt64 total = 0;
    for ( size_t i=0; i<files.size(); ++i )
        total += files[ i ].size;
    return total;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetStoreDownloadJob::GetBytesDone( void ) const
{GUCEF_TRACE;

    UInt64 total = 0;
    for ( size_t i=0; i<files.size(); ++i )
        total += files[ i ].bytesDone;
    return total;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreDownloadJob::GetFilePath( const CAssetStoreJobFile& file ) const
{GUCEF_TRACE;

    return CORE::CombinePath( destination, file.name ).ReplaceChar( '\\', '/' );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreDownloadJob::Serialize( CORE::CDataNode& domRootNode                        ,
                                   const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = domRootNode.SetAttribute( "jobId", jobId );
    success = domRootNode.SetAttribute( "sequence", sequence ) && success;
    success = domRootNode.SetAttribute( "providerId", providerId ) && success;
    success = domRootNode.SetAttribute( "itemId", itemId ) && success;
    success = domRootNode.SetAttribute( "itemTitle", itemTitle ) && success;
    success = domRootNode.SetAttribute( "formatId", formatId ) && success;
    success = domRootNode.SetValuesOfChildByName( "fileIds", fileIds ) && success;
    success = domRootNode.SetAttribute( "destination", destination ) && success;
    success = domRootNode.SetAttribute( "forIngest", forIngest ) && success;
    success = domRootNode.SetAttribute( "state", CORE::CString( AssetDownloadJobStateToString( state ) ) ) && success;
    success = domRootNode.SetAttribute( "status", CORE::CString( AssetStoreStatusToString( status ) ) ) && success;
    success = domRootNode.SetAttribute( "message", message ) && success;
    success = domRootNode.SetAttribute( "attempts", attempts ) && success;
    success = domRootNode.SetAttribute( "createdAtMs", createdAtMs ) && success;
    success = domRootNode.SetAttribute( "updatedAtMs", updatedAtMs ) && success;
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
CAssetStoreDownloadJob::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                     const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    jobId = domRootNode.GetAttributeValueOrChildValueByName( "jobId" ).AsString();
    if ( jobId.IsNULLOrEmpty() )
        return false;
    sequence = domRootNode.GetAttributeValueOrChildValueByName( "sequence" ).AsUInt64( 0, true );
    providerId = domRootNode.GetAttributeValueOrChildValueByName( "providerId" ).AsString();
    itemId = domRootNode.GetAttributeValueOrChildValueByName( "itemId" ).AsString();
    itemTitle = domRootNode.GetAttributeValueOrChildValueByName( "itemTitle" ).AsString();
    formatId = domRootNode.GetAttributeValueOrChildValueByName( "formatId" ).AsString();
    domRootNode.GetValuesOfChildByName( "fileIds", fileIds );
    destination = domRootNode.GetAttributeValueOrChildValueByName( "destination" ).AsString();
    forIngest = domRootNode.GetAttributeValueOrChildValueByName( "forIngest" ).AsBool( false, true );
    state = AssetDownloadJobStateFromString( domRootNode.GetAttributeValueOrChildValueByName( "state" ).AsString() );
    status = AssetStoreStatusFromString( domRootNode.GetAttributeValueOrChildValueByName( "status" ).AsString( "ok" ) );
    message = domRootNode.GetAttributeValueOrChildValueByName( "message" ).AsString();
    attempts = domRootNode.GetAttributeValueOrChildValueByName( "attempts" ).AsUInt32( 0, true );
    createdAtMs = domRootNode.GetAttributeValueOrChildValueByName( "createdAtMs" ).AsInt64( 0, true );
    updatedAtMs = domRootNode.GetAttributeValueOrChildValueByName( "updatedAtMs" ).AsInt64( 0, true );
    files.clear();
    const CORE::CDataNode* filesNode = domRootNode.FindChild( "files" );
    if ( GUCEF_NULL != filesNode )
    {
        CORE::CDataNode::const_iterator i = filesNode->ConstBegin();
        while ( i != filesNode->ConstEnd() )
        {
            CAssetStoreJobFile file;
            if ( file.Deserialize( *(*i), settings ) )
                files.push_back( file );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetStoreDownloadJob::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetStoreDownloadJob( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreDownloadJob::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreDownloadJob";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreDownloadJob::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetStoreDownloadJob::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETSTORE::CAssetStoreDownloadJob";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
