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
#ifndef TESTPACKINDEXDOCUMENT_H
#include "TestPackIndexDocument.h"
#define TESTPACKINDEXDOCUMENT_H
#endif /* TESTPACKINDEXDOCUMENT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#include "gucefASSETSTORE_CAssetNameHints.h"
#define GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#endif /* GUCEF_ASSETSTORE_CASSETNAMEHINTS_H ? */

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
    , m_indexMode( INDEX_UNSUPPORTED )
    , m_indexCalls( 0 )
    , m_indexBuilds( 0 )
    , m_failingAssets()
    , m_lastRequest()
    , m_hold( false )
    , m_active( 0 )
    , m_maxActive( 0 )
    , m_activeThreads( 0 )
    , m_maxActiveThreads( 0 )
    , m_threadPoolNames()
    , m_startOrder()
    , m_activeAtStart()
    , m_lock()
{
}

/*-------------------------------------------------------------------------*/

void
CTestConverterHook::SetHold( bool hold )
{
    MT::CScopeMutex lock( m_lock );
    m_hold = hold;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestConverterHook::GetActiveCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_active;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestConverterHook::GetMaxActiveCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_maxActive;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestConverterHook::GetMaxActiveThreads( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_maxActiveThreads;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringSet
CTestConverterHook::GetThreadPoolNames( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_threadPoolNames;
}

/*-------------------------------------------------------------------------*/

void
CTestConverterHook::ResetConcurrency( void )
{
    MT::CScopeMutex lock( m_lock );
    m_maxActive = m_active;
    m_maxActiveThreads = m_activeThreads;
    m_threadPoolNames.clear();
    m_startOrder.clear();
    m_activeAtStart.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CTestConverterHook::GetStartOrder( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_startOrder;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestConverterHook::GetActiveAtStart( const CORE::CString& packId ) const
{
    MT::CScopeMutex lock( m_lock );
    GUCEF::map< CORE::CString, UInt32 >::const_iterator i = m_activeAtStart.find( packId );
    return i == m_activeAtStart.end() ? 0 : (*i).second;
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
    // the run counts as active, with its threads, from here to its end
    const UInt32 threads = request.maxThreads > 0 ? request.maxThreads : 1;
    {
        MT::CScopeMutex lock( m_lock );
        ++m_active;
        m_activeThreads += threads;
        if ( m_active > m_maxActive )
            m_maxActive = m_active;
        if ( m_activeThreads > m_maxActiveThreads )
            m_maxActiveThreads = m_activeThreads;
        if ( !request.threadPoolName.IsNULLOrEmpty() )
            m_threadPoolNames.insert( request.threadPoolName );
        m_lastRequest.maxThreads = request.maxThreads;
        m_lastRequest.threadPoolName = request.threadPoolName;
        m_startOrder.push_back( request.pack.packId );
        m_activeAtStart[ request.pack.packId ] = m_active;
    }
    bool held = true;
    while ( held )
    {
        {
            MT::CScopeMutex lock( m_lock );
            held = m_hold;
        }
        if ( held && GUCEF_NULL != request.observer && request.observer->IsCancelRequested() )
            held = false;
        if ( held )
            MT::ThreadDelay( 5 );
    }
    TAssetStoreStatus status = ASSETSTORE_STATUS_FAILED;
    try
    {
        status = ConvertPackNow( request, assets, outcome );
    }
    catch ( ... )
    {
        MT::CScopeMutex lock( m_lock );
        --m_active;
        m_activeThreads -= threads;
        throw;
    }
    MT::CScopeMutex lock( m_lock );
    --m_active;
    m_activeThreads -= threads;
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CTestConverterHook::ConvertPackNow( const CAssetConversionRequest& request, TAssetRecordVector& assets, CAssetConversionOutcome& outcome )
{
    EMode mode;
    EIndexMode indexMode;
    UInt32 version;
    {
        MT::CScopeMutex lock( m_lock );
        ++m_calls;
        mode = m_mode;
        indexMode = m_indexMode;
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

    // converts the models: an intermediate file per model and, unless the request says no cook, a cooked file; a cook only run
    // cooks the intermediate files that are there. The report lists the assets of the run with their status, as the converter does
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CString::StringSet failing;
    {
        MT::CScopeMutex lock( m_lock );
        failing = m_failingAssets;
        m_lastRequest.cookOnly = request.cookOnly;
        m_lastRequest.noCook = request.noCook;
        m_lastRequest.reconvert = request.reconvert;
        m_lastRequest.assetGlobs = request.assetGlobs;
        m_lastRequest.convertedVfsFolder = request.convertedVfsFolder;
        m_lastRequest.cookedVfsFolder = request.cookedVfsFolder;
        m_lastRequest.keepPartialResult = request.keepPartialResult;
    }
    const bool cook = !request.noCook && !request.cookedVfsFolder.IsNULLOrEmpty();
    if ( request.cookOnly && !vfs.DirExists( request.convertedVfsFolder ) )
    {
        outcome.status = ASSETSTORE_STATUS_NOT_FOUND;
        outcome.message = "there is no intermediate to cook";
        return outcome.status;
    }
    const CORE::CString folders[] = { request.convertedVfsFolder, cook ? request.cookedVfsFolder : CORE::CString::Empty };
    for ( UInt32 f=0; f<2; ++f )
    {
        if ( folders[ f ].IsNULLOrEmpty() || vfs.DirExists( folders[ f ] ) )
            continue;
        // the parents first: the folder of the profile is below "converted/"
        CORE::CString parent = folders[ f ].ReplaceChar( '\\', '/' );
        const Int32 slash = parent.HasChar( '/', false );
        if ( slash > 0 )
        {
            parent = parent.SubstrToIndex( (UInt32) slash, true );
            if ( !vfs.DirExists( parent ) )
                vfs.CreateDir( parent );
        }
        if ( !vfs.CreateDir( folders[ f ] ) )
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
    UInt32 failed = 0;
    UInt32 written = 0;
    CORE::CString reportAssets;
    for ( size_t a=0; a<assets.size(); ++a )
    {
        assets[ a ].convertedPath.Clear();
        assets[ a ].convertedFormat.Clear();
        if ( !CanConvert( assets[ a ] ) )
            continue;
        const CORE::CString relative = request.GetSourceRelativePath( assets[ a ] );
        if ( !request.IsInScope( relative ) )
            continue;
        if ( GUCEF_NULL != request.observer && request.observer->IsCancelRequested() )
        {
            outcome.status = ASSETSTORE_STATUS_CANCELLED;
            outcome.message = "the test converter was cancelled";
            return outcome.status;
        }
        if ( MODE_FAIL_MIDWAY == mode && written >= 1 )
        {
            outcome.status = ASSETSTORE_STATUS_IO_ERROR;
            outcome.message = "the test converter stopped half way: the storage refused a write";
            return outcome.status;
        }
        const CORE::CString file = assets[ a ].name + ".gltf";
        CORE::CString status = request.cookOnly ? CORE::CString( "cached" ) : CORE::CString( "converted" );
        if ( failing.end() != failing.find( assets[ a ].name ) )
        {
            status = "failed";
            ++failed;
        }
        else
        {
            if ( request.cookOnly && !vfs.FileExists( request.convertedVfsFolder + "/" + file ) )
                status = "failed";
            if ( !request.cookOnly && !WriteVfsFile( request.convertedVfsFolder + "/" + file, "converted " + assets[ a ].name + " v" + CORE::ToString( version ) ) )
            {
                outcome.status = ASSETSTORE_STATUS_IO_ERROR;
                outcome.message = "the file " + file + " cannot be written";
                return outcome.status;
            }
            if ( "failed" != status && cook && !WriteVfsFile( request.cookedVfsFolder + "/" + assets[ a ].name + ".mesh", "cooked " + assets[ a ].name + " v" + CORE::ToString( version ) ) )
            {
                outcome.status = ASSETSTORE_STATUS_IO_ERROR;
                outcome.message = "the cooked file of " + file + " cannot be written";
                return outcome.status;
            }
            if ( "failed" == status )
                ++failed;
            else
            {
                assets[ a ].convertedPath = request.convertedPathPrefix + "/" + file;
                assets[ a ].convertedFormat = "gltf";
                ++done;
                ++written;
            }
        }
        reportAssets += CORE::CString( reportAssets.IsNULLOrEmpty() ? "" : ", " ) + "{ \"asset\": \"" + relative + "\", \"status\": \"" + status + "\" }";
        if ( GUCEF_NULL != request.observer )
            request.observer->OnConversionProgress( done, total, assets[ a ].originalPath );
    }
    const CORE::CString reportPath = request.reportVfsPath.IsNULLOrEmpty() ? request.convertedVfsFolder + "/report.json" : request.reportVfsPath;
    WriteVfsFile( reportPath, "{ \"converted\": " + CORE::ToString( request.cookOnly ? 0 : done ) + ", \"counts\": { \"" + ( request.cookOnly ? "cached" : "converted" ) + "\": " +
                  CORE::ToString( done ) + ", \"failed\": " + CORE::ToString( failed ) + " }, \"assets\": [ " + reportAssets + " ] }" );
    if ( INDEX_BUILD == indexMode && !request.cookOnly )
    {
        // the writer step of the pipeline: the conversion leaves the document of the pack in the intermediate folder
        CAssetPackIndexRequest indexRequest;
        indexRequest.pack = request.pack;
        indexRequest.sourceVfsPath = request.sourceVfsPath;
        indexRequest.sourcePrefix = request.sourcePrefix;
        indexRequest.intermediateVfsFolder = request.convertedVfsFolder;
        CAssetPackIndexOutcome indexOutcome;
        MakeIndexDocument( indexRequest, indexOutcome, false );
    }
    outcome.record.reportPath = request.convertedPathPrefix + "/report.json";
    outcome.record.convertedCount = done;
    outcome.record.failedCount = failed;
    if ( failed > 0 )
    {
        outcome.problems.push_back( CORE::ToString( failed ) + " asset(s) failed" );
        outcome.status = ASSETSTORE_STATUS_FAILED;
        outcome.message = CORE::ToString( failed ) + " error(s) in the conversion report";
        if ( !request.keepPartialResult )
        {
            for ( size_t a=0; a<assets.size(); ++a )
            {
                assets[ a ].convertedPath.Clear();
                assets[ a ].convertedFormat.Clear();
            }
        }
        return outcome.status;
    }
    outcome.status = ASSETSTORE_STATUS_OK;
    outcome.message = CORE::ToString( done ) + " asset(s) converted";
    return outcome.status;
}

/*-------------------------------------------------------------------------*/

void
CTestConverterHook::SetFailingAssets( const CORE::CString::StringSet& assetNames )
{
    MT::CScopeMutex lock( m_lock );
    m_failingAssets = assetNames;
}

/*-------------------------------------------------------------------------*/

CTestConverterHook::SLastRequest
CTestConverterHook::GetLastRequest( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_lastRequest;
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

/*-------------------------------------------------------------------------*/

void
CTestConverterHook::SetIndexMode( EIndexMode mode )
{
    MT::CScopeMutex lock( m_lock );
    m_indexMode = mode;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestConverterHook::GetIndexCallCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_indexCalls;
}

/*-------------------------------------------------------------------------*/

UInt32
CTestConverterHook::GetIndexBuildCount( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_indexBuilds;
}

/*-------------------------------------------------------------------------*/

namespace {

CORE::CString
KindOfExtension( const CORE::CString& extension )
{
    if ( "fbx" == extension || "gltf" == extension || "obj" == extension )
        return "model";
    if ( "png" == extension || "tga" == extension || "jpg" == extension )
        return "texture";
    if ( "wav" == extension || "ogg" == extension )
        return "audio";
    return "other";
}

CORE::CString
RegistryKindOfStoreKind( const CORE::CString& kind )
{
    if ( "static-model" == kind || "skeletal-model" == kind )
        return "model";
    if ( "texture" == kind || "material" == kind || "animation" == kind )
        return kind;
    if ( "music" == kind || "sfx" == kind )
        return "audio";
    return "other";
}

}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CTestConverterHook::BuildPackIndex( const CAssetPackIndexRequest& request, CAssetPackIndexOutcome& outcome )
{
    EIndexMode mode;
    {
        MT::CScopeMutex lock( m_lock );
        ++m_indexCalls;
        mode = m_indexMode;
    }
    if ( INDEX_UNSUPPORTED == mode )
        return CIAssetConverterHook::BuildPackIndex( request, outcome );
    outcome = CAssetPackIndexOutcome();
    outcome.packId = request.pack.packId;
    if ( INDEX_FAIL == mode )
    {
        outcome.status = ASSETSTORE_STATUS_FAILED;
        outcome.message = "the test converter does not build indexes";
        return outcome.status;
    }
    return MakeIndexDocument( request, outcome, true );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CTestConverterHook::MakeIndexDocument( const CAssetPackIndexRequest& request, CAssetPackIndexOutcome& outcome, bool countBuild )
{
    UInt32 version;
    {
        MT::CScopeMutex lock( m_lock );
        version = m_version;
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString folder = request.intermediateVfsFolder;
    const CORE::CString manifestPath = folder + "/pack-index.json";
    outcome.documentPath = manifestPath;
    if ( vfs.FileExists( manifestPath ) && !request.reindex && !request.reanalyse && !request.derive )
    {
        outcome.reused = true;
        outcome.status = ASSETSTORE_STATUS_OK;
        outcome.message = "the pack index stands";
        return outcome.status;
    }

    CTestPackIndexDocument document;
    document.packName = request.pack.packId;
    document.packKey = "key-" + request.pack.packId;
    document.indexKey = document.packKey + "-v" + CORE::ToString( version ) + ( request.derive ? "-derived" : "" );
    document.fileCount = (UInt64) request.pack.fileCount;
    document.totalBytes = (UInt64) request.pack.sizeBytes;
    document.derived = request.derive;
    for ( CORE::CString::StringSet::const_iterator r=request.pack.licence.restrictions.begin(); r!=request.pack.licence.restrictions.end(); ++r )
        document.restrictions.push_back( (*r) );
    if ( request.derive )
    {
        for ( size_t a=0; a<request.assets.size(); ++a )
        {
            const CAssetRecord& asset = request.assets[ a ];
            CORE::CString path = asset.originalPath;
            if ( !request.sourcePrefix.IsNULLOrEmpty() && 0 == path.HasSubstr( request.sourcePrefix, true ) )
                path = path.CutChars( request.sourcePrefix.Length(), true );
            CTestPackIndexResource resource( path, RegistryKindOfStoreKind( asset.kind ), CORE::CString::Empty, (UInt64) asset.sizeBytes );
            if ( asset.model.isSet )
                resource.Stat( "triangles", (UInt64) asset.model.triangles ).Stat( "vertices", (UInt64) asset.model.vertices );
            resource.state = asset.analysed ? "analysed" : "indexed";
            document.resources.push_back( resource );
        }
    }
    else
    {
        VFS::CVFS::TStringVector files;
        vfs.GetFileList( files, request.sourceVfsPath, true, true, CORE::CString::Empty, GUCEF_UINT32MAX );
        CORE::CString root = request.sourceVfsPath.ReplaceChar( '\\', '/' );
        while ( root.Length() > 0 && '/' == root.C_String()[ 0 ] )
            root = root.CutChars( 1, true );
        for ( size_t f=0; f<files.size(); ++f )
        {
            CORE::CString path = files[ f ].ReplaceChar( '\\', '/' );
            while ( path.Length() > 0 && '/' == path.C_String()[ 0 ] )
                path = path.CutChars( 1, true );
            if ( 0 == path.HasSubstr( root + "/", true ) )
                path = path.CutChars( root.Length() + 1, true );
            const CORE::CString name = path.SubstrToChar( '/', false );
            if ( "pack.json" == name )
                continue;
            const Int32 slash = path.HasChar( '/', false );
            const CORE::CString subGroup = slash < 0 ? CORE::CString::Empty : path.SubstrToIndex( (UInt32) slash, true ).Lowercase();
            CTestPackIndexResource resource( path, KindOfExtension( CAssetNameHints::GetExtension( name ).Lowercase() ), subGroup, (UInt64) vfs.GetFileSize( files[ f ] ) );
            document.resources.push_back( resource );
        }
    }
    // the parents of the folder first
    {
        CORE::CString::StringVector missing;
        CORE::CString parent = folder.ReplaceChar( '\\', '/' );
        while ( !parent.IsNULLOrEmpty() && !vfs.DirExists( parent ) )
        {
            missing.push_back( parent );
            const Int32 slash = parent.HasChar( '/', false );
            parent = slash > 0 ? parent.SubstrToIndex( (UInt32) slash, true ) : CORE::CString::Empty;
        }
        for ( size_t m=missing.size(); m>0; --m )
            vfs.CreateDir( missing[ m-1 ] );
    }
    if ( document.WriteTo( folder ).IsNULLOrEmpty() )
    {
        outcome.status = ASSETSTORE_STATUS_IO_ERROR;
        outcome.message = "the pack index cannot be written in " + folder;
        return outcome.status;
    }
    if ( countBuild )
    {
        MT::CScopeMutex lock( m_lock );
        ++m_indexBuilds;
    }
    outcome.built = true;
    outcome.derived = request.derive;
    outcome.status = ASSETSTORE_STATUS_OK;
    outcome.message = "the pack index was built";
    return outcome.status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/
