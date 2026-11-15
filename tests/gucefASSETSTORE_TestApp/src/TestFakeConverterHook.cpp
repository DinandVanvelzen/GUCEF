/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */
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
#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */

#include "TestFakeConverterHook.h"

using namespace GUCEF;
using namespace GUCEF::ASSETSTORE;

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTestConverterHook::CTestConverterHook( void )
    : CIAssetConverterHook()
    , m_mode( MODE_CONVERT )
    , m_version( 1 )
    , m_calls( 0 )
    , m_waits( 0 )
    , m_lock()
{
}

/*-------------------------------------------------------------------------*/

CTestConverterHook::~CTestConverterHook()
{
}

/*-------------------------------------------------------------------------*/

void
CTestConverterHook::SetMode( EMode mode )
{
    MT::CScopeMutex lock( m_lock );
    m_mode = mode;
}

/*-------------------------------------------------------------------------*/

void
CTestConverterHook::SetVersion( UInt32 version )
{
    MT::CScopeMutex lock( m_lock );
    m_version = version;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestConverterHook::GetCallCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_calls;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestConverterHook::GetStartedWaitCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_waits;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestConverterHook::GetName( void ) const
{
    return "test converter";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTestConverterHook::GetProfileName( void ) const
{
    return "test";
}

/*-------------------------------------------------------------------------*/

bool
CTestConverterHook::CanConvert( const CAssetRecord& asset ) const
{
    return asset.kind == "static-model" || asset.kind == "skeletal-model";
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CTestConverterHook::ConvertPack( const CAssetConversionRequest& request, TAssetRecordVector& assets, CAssetConversionOutcome& outcome )
{
    EMode mode;
    UInt32 version;
    {
        MT::CScopeMutex lock( m_lock );
        ++m_calls;
        mode = m_mode;
        version = m_version;
    }
    outcome.record.profile = GetProfileName();
    outcome.record.converters.insert( CAssetConversionRecord::MakeConverterEntry( "importer:test", version ) );

    switch ( mode )
    {
        case MODE_REFUSE:
        {
            outcome.status = ASSETSTORE_STATUS_REFUSED;
            outcome.message = "the licence record does not allow it";
            return outcome.status;
        }
        case MODE_FAIL:
        {
            outcome.problems.push_back( "SM_Wep_Sword_01.fbx: the mesh has no vertices" );
            outcome.status = ASSETSTORE_STATUS_FAILED;
            outcome.message = "1 error(s) in the conversion report";
            return outcome.status;
        }
        case MODE_IO_ERROR:
        {
            outcome.status = ASSETSTORE_STATUS_IO_ERROR;
            outcome.message = "the folder of the converted files cannot be made";
            return outcome.status;
        }
        case MODE_WAIT_FOR_CANCEL:
        {
            {
                MT::CScopeMutex lock( m_lock );
                ++m_waits;
            }
            for ( UInt32 waited=0; waited<20000; waited+=5 )
            {
                if ( GUCEF_NULL != request.observer && request.observer->IsCancelRequested() )
                {
                    outcome.status = ASSETSTORE_STATUS_CANCELLED;
                    outcome.message = "the test converter was cancelled";
                    return outcome.status;
                }
                MT::ThreadDelay( 5 );
            }
            outcome.status = ASSETSTORE_STATUS_FAILED;
            outcome.message = "the test converter was never asked to cancel";
            return outcome.status;
        }
        default:
        {
            break;
        }
    }

    // converts the models
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( request.convertedVfsFolder ) )
    {
        // the parents first: the folder of the profile is below "converted/"
        CORE::CString parent = request.convertedVfsFolder.ReplaceChar( '\\', '/' );
        const Int32 slash = parent.HasChar( '/', false );
        if ( slash > 0 )
        {
            parent = parent.SubstrToIndex( (UInt32) slash, true );
            if ( !vfs.DirExists( parent ) )
                vfs.CreateDir( parent );
        }
        if ( !vfs.CreateDir( request.convertedVfsFolder ) )
        {
            outcome.status = ASSETSTORE_STATUS_IO_ERROR;
            outcome.message = "the folder of the converted files cannot be made";
            return outcome.status;
        }
    }
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
            outcome.message = "the test converter was cancelled";
            return outcome.status;
        }
        const CORE::CString file = assets[ a ].name + ".gltf";
        if ( !WriteVfsFile( request.convertedVfsFolder + "/" + file, "converted " + assets[ a ].name ) )
        {
            outcome.status = ASSETSTORE_STATUS_IO_ERROR;
            outcome.message = "the file " + file + " cannot be written";
            return outcome.status;
        }
        assets[ a ].convertedPath = request.convertedPathPrefix + "/" + file;
        assets[ a ].convertedFormat = "gltf";
        ++done;
        if ( GUCEF_NULL != request.observer )
            request.observer->OnConversionProgress( done, total, assets[ a ].originalPath );
    }
    WriteVfsFile( request.convertedVfsFolder + "/report.json", "{ \"converted\": " + CORE::ToString( done ) + " }" );
    outcome.record.reportPath = request.convertedPathPrefix + "/report.json";
    outcome.record.convertedCount = done;
    outcome.status = ASSETSTORE_STATUS_OK;
    outcome.message = CORE::ToString( done ) + " asset(s) converted";
    return outcome.status;
}

/*-------------------------------------------------------------------------*/

bool
CTestConverterHook::IsConversionStale( const CAssetConversionRecord& recorded, CORE::CString& reason ) const
{
    UInt32 version;
    {
        MT::CScopeMutex lock( m_lock );
        version = m_version;
    }
    for ( CORE::CString::StringSet::const_iterator e=recorded.converters.begin(); e!=recorded.converters.end(); ++e )
    {
        CORE::CString name;
        UInt32 recordedVersion = 0;
        if ( CAssetConversionRecord::ParseConverterEntry( (*e), name, recordedVersion ) && name == "importer:test" && recordedVersion != version )
        {
            reason = "importer:test is at version " + CORE::ToString( version ) + ", the conversion was made with version " + CORE::ToString( recordedVersion );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
