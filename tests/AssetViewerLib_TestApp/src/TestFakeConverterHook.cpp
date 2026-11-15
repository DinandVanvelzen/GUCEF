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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "TestFakeConverterHook.h"

using namespace GUCEF;
using namespace GUCEF::ASSETSTORE;

namespace SOURCESTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFakeConverterHook::CFakeConverterHook( void )
    : CIAssetConverterHook()
    , m_refuse( false )
    , m_calls( 0 )
{
}

/*-------------------------------------------------------------------------*/

CFakeConverterHook::~CFakeConverterHook()
{
}

/*-------------------------------------------------------------------------*/

void
CFakeConverterHook::SetRefuse( bool refuse )
{
    m_refuse = refuse;
}

/*-------------------------------------------------------------------------*/

UInt32
CFakeConverterHook::GetCallCount( void ) const
{
    return m_calls;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeConverterHook::GetName( void ) const
{
    return "fake converter";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CFakeConverterHook::GetProfileName( void ) const
{
    return "fake";
}

/*-------------------------------------------------------------------------*/

bool
CFakeConverterHook::CanConvert( const CAssetRecord& asset ) const
{
    return asset.kind == "static-model" || asset.kind == "skeletal-model";
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CFakeConverterHook::ConvertPack( const CAssetConversionRequest& request, TAssetRecordVector& assets, CAssetConversionOutcome& outcome )
{
    ++m_calls;
    outcome.record.profile = GetProfileName();
    outcome.record.converters.insert( CAssetConversionRecord::MakeConverterEntry( "importer:fake", 1 ) );
    if ( m_refuse )
    {
        outcome.status = ASSETSTORE_STATUS_REFUSED;
        outcome.message = "the licence record does not allow it";
        return outcome.status;
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    UInt32 total = 0;
    for ( size_t a=0; a<assets.size(); ++a )
        if ( CanConvert( assets[ a ] ) )
            ++total;
    UInt32 done = 0;
    for ( size_t a=0; a<assets.size(); ++a )
    {
        assets[ a ].convertedPath.Clear();
        assets[ a ].convertedFormat.Clear();
        if ( !CanConvert( assets[ a ] ) )
            continue;
        if ( GUCEF_NULL != request.observer && request.observer->IsCancelRequested() )
        {
            outcome.status = ASSETSTORE_STATUS_CANCELLED;
            outcome.message = "the fake converter was cancelled";
            return outcome.status;
        }
        const CORE::CString relative = request.GetSourceRelativePath( assets[ a ] );
        const CORE::CString file = assets[ a ].name + ".gltf";
        if ( !vfs.DirExists( request.convertedVfsFolder ) )
        {
            CORE::CString parent = request.convertedVfsFolder;
            const Int32 slash = parent.HasChar( '/', false );
            if ( slash > 0 && !vfs.DirExists( parent.SubstrToIndex( (UInt32) slash, true ) ) )
                vfs.CreateDir( parent.SubstrToIndex( (UInt32) slash, true ) );
            vfs.CreateDir( request.convertedVfsFolder );
        }
        if ( !vfs.CopyFile( request.sourceVfsPath + "/" + relative, request.convertedVfsFolder + "/" + file, true ) )
        {
            outcome.status = ASSETSTORE_STATUS_IO_ERROR;
            outcome.message = "the file " + relative + " cannot be copied";
            return outcome.status;
        }
        assets[ a ].convertedPath = request.convertedPathPrefix + "/" + file;
        assets[ a ].convertedFormat = "gltf";
        ++done;
        if ( GUCEF_NULL != request.observer )
            request.observer->OnConversionProgress( done, total, relative );
    }
    outcome.record.convertedCount = done;
    outcome.status = ASSETSTORE_STATUS_OK;
    outcome.message = CORE::ToString( done ) + " asset(s) converted";
    return outcome.status;
}

/*-------------------------------------------------------------------------*/

bool
CFakeConverterHook::IsConversionStale( const CAssetConversionRecord& recorded, CORE::CString& reason ) const
{
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace SOURCESTEST */

/*-------------------------------------------------------------------------*/
