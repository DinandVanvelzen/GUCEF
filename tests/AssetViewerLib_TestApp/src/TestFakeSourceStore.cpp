/*
 *  AssetViewerLib_TestApp: tests of the AssetViewerLib module
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H
#include "gucefASSETSTORE_CAssetStoreTransferContext.h"
#define GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORETRANSFERCONTEXT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADPLAN_H
#include "gucefASSETSTORE_CAssetStoreDownloadPlan.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADPLAN_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADPLAN_H ? */

#include "TestFakeSourceStore.h"

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace GUCEF::ASSETSTORE;

namespace SOURCESTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CFakeSourceProvider::ProviderIdText = "fakesrc";

namespace {

/** The bytes of a file of the store: a pattern of the seed */
void
MakeBytes( UInt32 size, UInt32 seed, CDynamicBuffer& content )
{
    content.Clear();
    content.SetBufferSize( size > 0 ? size : 1 );
    UInt32 state = seed * 2654435761u + 1;
    UInt8* bytes = static_cast< UInt8* >( content.GetBufferPtr() );
    for ( UInt32 i=0; i<size; ++i )
    {
        state = state * 1103515245u + 12345u;
        bytes[ i ] = (UInt8) ( state >> 16 );
    }
    content.SetDataSize( size );
}

/** The seed is part of the id of a file: "<item>.<format>.<index>" */
UInt32
SeedOf( const CString& fileId )
{
    UInt32 seed = 17;
    for ( UInt32 i=0; i<fileId.Length(); ++i )
        seed = seed * 31 + (UInt8) fileId[ i ];
    return seed;
}

}

/*-------------------------------------------------------------------------*/

CFakeTransferSource::CFakeTransferSource( CFakeSourceProvider& provider )
    : CIAssetStoreTransferSource()
    , m_provider( provider )
{

}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeTransferSource::RunTransfer( const CAssetStoreTransfer& transfer, CAssetStoreTransferContext& context )
{
    CDynamicBuffer content;
    if ( !m_provider.GetContent( transfer.fileId, content ) )
        return ASSETSTORE_STATUS_NOT_FOUND;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const UInt64 total = content.GetDataSize();
    UInt64 written = context.GetResumeOffset() < total ? context.GetResumeOffset() : 0;
    const UInt32 chunk = m_provider.GetChunkSize();
    while ( written < total )
    {
        if ( context.IsStopRequested() )
            return ASSETSTORE_STATUS_CANCELLED;
        UInt64 next = written + chunk < total ? written + chunk : total;
        if ( next > m_provider.GetAllowedBytes() )
        {
            MT::ThreadDelay( 5 );
            continue;
        }
        CDynamicBuffer part;
        part.CopyFrom( static_cast< UInt32 >( next ), content.GetConstBufferPtr() );
        if ( !vfs.StoreAsFile( context.GetPartFilePath(), part, 0, true ) )
            return ASSETSTORE_STATUS_IO_ERROR;
        written = next;
        context.ReportFileBytes( written );
        MT::ThreadDelay( 2 );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CFakeSourceProvider::CFakeSourceProvider( void )
    : CIAssetStoreProvider()
    , m_items()
    , m_source( *this )
    , m_signedIn( true )
    , m_allowedBytes( (UInt64) 1 << 62 )
    , m_lock()
{

}

/*-------------------------------------------------------------------------*/

CFakeSourceProvider::~CFakeSourceProvider()
{
    WaitForAsyncOperations();
}

/*-------------------------------------------------------------------------*/

void
CFakeSourceProvider::AddItem( const CString& itemId, const CString& title, const CString& seller, const CString& listingType,
                              const vector< SFormatSpec >& formats, const CString& licenceName, const CString& thumbnailUrl )
{
    CAssetStoreItem item;
    item.providerId = ProviderIdText;
    item.itemId = itemId;
    item.title = title;
    item.seller = seller;
    item.listingType = listingType;
    item.licenceName = licenceName;
    item.thumbnailUrl = thumbnailUrl;
    if ( !licenceName.IsNULLOrEmpty() )
        item.licenceRestrictions.insert( "noMachineLearning" );
    for ( size_t f=0; f<formats.size(); ++f )
    {
        CAssetStoreFormat format;
        format.formatId = formats[ f ].formatId;
        format.name = formats[ f ].name;
        format.group = formats[ f ].group;
        for ( size_t i=0; i<formats[ f ].files.size(); ++i )
        {
            CAssetStoreFile file;
            file.fileId = itemId + "." + format.formatId + "." + UInt32ToString( (UInt32) i );
            file.name = formats[ f ].files[ i ].name;
            file.size = formats[ f ].files[ i ].size;
            format.files.push_back( file );
        }
        item.formats.push_back( format );
    }
    MT::CScopeMutex lock( m_lock );
    m_items.push_back( item );
}

/*-------------------------------------------------------------------------*/

void
CFakeSourceProvider::SetSignedIn( bool signedIn )
{
    {
        MT::CScopeMutex lock( m_lock );
        m_signedIn = signedIn;
    }
    NotifySessionStateChanged();
}

/*-------------------------------------------------------------------------*/

void
CFakeSourceProvider::AllowBytes( UInt64 bytes )
{
    MT::CScopeMutex lock( m_lock );
    m_allowedBytes = bytes;
}

/*-------------------------------------------------------------------------*/

UInt64
CFakeSourceProvider::GetAllowedBytes( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_allowedBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeSourceProvider::GetChunkSize( void ) const
{
    return 16384;
}

/*-------------------------------------------------------------------------*/

bool
CFakeSourceProvider::GetContent( const CString& fileId, CDynamicBuffer& content ) const
{
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_items.size(); ++i )
    {
        for ( size_t f=0; f<m_items[ i ].formats.size(); ++f )
        {
            const CAssetStoreFile* file = m_items[ i ].formats[ f ].FindFile( fileId );
            if ( GUCEF_NULL != file )
            {
                MakeBytes( (UInt32) file->size, SeedOf( fileId ), content );
                return true;
            }
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CFakeSourceProvider::GetProviderId( void ) const
{
    return ProviderIdText;
}

/*-------------------------------------------------------------------------*/

CString
CFakeSourceProvider::GetDisplayName( void ) const
{
    return "Fake Store";
}

/*-------------------------------------------------------------------------*/

CString
CFakeSourceProvider::GetAuthProviderName( void ) const
{
    return "fake-auth";
}

/*-------------------------------------------------------------------------*/

bool
CFakeSourceProvider::IsSignedIn( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_signedIn;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeSourceProvider::QueryLibrary( const CAssetStoreQuery& query, CAssetStoreLibraryPage& page )
{
    MT::CScopeMutex lock( m_lock );
    if ( !m_signedIn )
        return ASSETSTORE_STATUS_NOT_SIGNED_IN;
    page.Clear();
    const UInt32 first = query.cursor.IsNULLOrEmpty() ? 0 : StringToUInt32( query.cursor, 0 );
    const UInt32 pageSize = query.pageSize > 0 ? query.pageSize : 50;
    UInt32 index = 0;
    for ( size_t i=0; i<m_items.size(); ++i )
    {
        if ( !query.listingType.IsNULLOrEmpty() && m_items[ i ].listingType != query.listingType )
            continue;
        if ( !query.text.IsNULLOrEmpty() && m_items[ i ].title.Lowercase().HasSubstr( query.text.Lowercase() ) < 0 )
            continue;
        if ( index >= first && page.items.size() < pageSize )
            page.items.push_back( m_items[ i ] );
        ++index;
    }
    page.hasTotal = true;
    page.total = index;
    if ( first + pageSize < index )
        page.nextCursor = UInt32ToString( first + pageSize );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeSourceProvider::GetItem( const CString& itemId, CAssetStoreItem& item )
{
    MT::CScopeMutex lock( m_lock );
    if ( !m_signedIn )
        return ASSETSTORE_STATUS_NOT_SIGNED_IN;
    for ( size_t i=0; i<m_items.size(); ++i )
    {
        if ( m_items[ i ].itemId == itemId )
        {
            item = m_items[ i ];
            return ASSETSTORE_STATUS_OK;
        }
    }
    return ASSETSTORE_STATUS_NOT_FOUND;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeSourceProvider::PlanDownload( const CString& itemId, const CString& formatId, const CString::StringVector& fileIds, CAssetStoreDownloadPlan& plan )
{
    CAssetStoreItem item;
    const TAssetStoreStatus status = GetItem( itemId, item );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    const CAssetStoreFormat* format = item.FindFormat( formatId );
    if ( GUCEF_NULL == format )
        return ASSETSTORE_STATUS_NOT_FOUND;
    plan.providerId = ProviderIdText;
    plan.itemId = itemId;
    plan.formatId = formatId;
    plan.fileIds = fileIds;
    plan.transfers.clear();
    for ( size_t i=0; i<format->files.size(); ++i )
    {
        bool wanted = fileIds.empty();
        for ( size_t w=0; w<fileIds.size(); ++w )
            wanted = wanted || fileIds[ w ] == format->files[ i ].fileId;
        if ( !wanted )
            continue;
        CAssetStoreTransfer transfer;
        transfer.fileId = format->files[ i ].fileId;
        transfer.url = "fake://" + transfer.fileId;
        transfer.size = format->files[ i ].size;
        transfer.targetName = format->files[ i ].name;
        plan.transfers.push_back( transfer );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CIAssetStoreTransferSource*
CFakeSourceProvider::GetTransferSource( void )
{
    return &m_source;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace SOURCESTEST */

/*-------------------------------------------------------------------------*/
