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

#include <algorithm>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */
#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_COMCORE_CDATABASETRANSACTION_H
#include "gucefCOMCORE_CDatabaseTransaction.h"
#define GUCEF_COMCORE_CDATABASETRANSACTION_H
#endif /* GUCEF_COMCORE_CDATABASETRANSACTION_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#include "gucefASSETSTORE_CAssetContentAnalyzerRegistry.h"
#define GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETANALYSISBATCH_H
#include "gucefASSETSTORE_CAssetAnalysisBatch.h"
#define GUCEF_ASSETSTORE_CASSETANALYSISBATCH_H
#endif /* GUCEF_ASSETSTORE_CASSETANALYSISBATCH_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDIGEST_H
#include "gucefASSETSTORE_CAssetDigest.h"
#define GUCEF_ASSETSTORE_CASSETDIGEST_H
#endif /* GUCEF_ASSETSTORE_CASSETDIGEST_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#include "gucefASSETSTORE_CAssetNameHints.h"
#define GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#endif /* GUCEF_ASSETSTORE_CASSETNAMEHINTS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#include "gucefASSETSTORE_CAssetStoreSafeNames.h"
#define GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H ? */
#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H
#include "gucefASSETSTORE_CAssetStoreItemNormalizer.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEMNORMALIZER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETDISKPACKNORMALIZER_H
#include "gucefASSETSTORE_CAssetDiskPackNormalizer.h"
#define GUCEF_ASSETSTORE_CASSETDISKPACKNORMALIZER_H
#endif /* GUCEF_ASSETSTORE_CASSETDISKPACKNORMALIZER_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#include "gucefASSETSTORE_CIAssetConverterHook.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERTERHOOK_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H
#include "gucefASSETSTORE_CAssetPackConversion.h"
#define GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETITEMREVISION_H
#include "gucefASSETSTORE_CAssetItemRevision.h"
#define GUCEF_ASSETSTORE_CASSETITEMREVISION_H
#endif /* GUCEF_ASSETSTORE_CASSETITEMREVISION_H ? */

#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */
#include "gucefASSETSTORE_CAssetIngestPipeline.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The bytes of the text without the terminating zero, for every string format of CString */
inline UInt32
ByteLengthOf( const CORE::CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

Int64
NowUtcMs( void )
{
    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

CORE::CString
NormalizeVfsPath( const CORE::CString& path )
{
    CORE::CString normalized = path.ReplaceChar( '\\', '/' );
    while ( ByteLengthOf( normalized ) > 0 && '/' == normalized.C_String()[ 0 ] )
        normalized = normalized.CutChars( 1, true );
    while ( ByteLengthOf( normalized ) > 0 && '/' == normalized.C_String()[ ByteLengthOf( normalized )-1 ] )
        normalized = normalized.CutChars( 1, false );
    return normalized;
}

CORE::CString
JoinVfs( const CORE::CString& a, const CORE::CString& b )
{
    const CORE::CString left = NormalizeVfsPath( a );
    const CORE::CString right = NormalizeVfsPath( b );
    if ( left.IsNULLOrEmpty() )
        return right;
    if ( right.IsNULLOrEmpty() )
        return left;
    return left + "/" + right;
}

CORE::CString
ParentOf( const CORE::CString& path )
{
    const CORE::CString normalized = NormalizeVfsPath( path );
    const Int32 slash = normalized.HasChar( '/', false );
    return slash > 0 ? normalized.SubstrToIndex( (UInt32) slash, true ) : CORE::CString::Empty;
}

CORE::CString
LastPartOf( const CORE::CString& path )
{
    const CORE::CString normalized = NormalizeVfsPath( path );
    const Int32 slash = normalized.HasChar( '/', false );
    return slash >= 0 ? normalized.CutChars( (UInt32) slash + 1, true ) : normalized;
}

/** The name of a pack that comes from disk: the title of the request when it has one (an upload is staged in a folder of its own, "files"), else the last part of its path */
CORE::CString
SourceNameOf( const CAssetIngestJob& job )
{
    return job.request.sourceId == CPrimaryAssetStoreSettings::DiskSourceId && !job.request.item.title.IsNULLOrEmpty() ? job.request.item.title : LastPartOf( job.request.contentPath );
}

CORE::CString
HexOf( UInt64 value, UInt32 digits )
{
    static const char hexDigits[] = "0123456789abcdef";
    CORE::CString text;
    for ( Int32 d=(Int32) digits-1; d>=0; --d )
        text += hexDigits[ ( value >> ( d * 4 ) ) & 0xF ];
    return text;
}

bool
IsHeldFormatKind( const CORE::CString& kind )
{
    return CAssetNameHints::IsModelKind( kind ) || kind == "animation" || kind == "scene" || kind == "prefab" || kind == "template" || kind == "material";
}

/** The observer of the conversion of an ingest: the cancel request of the job's future, the progress as events of the store */
class CIngestConversionObserver : public CIAssetConversionObserver
{
    public:

    CIngestConversionObserver( CPrimaryAssetStore& store, const CORE::CString& ingestId, const CORE::CString& packId, const CORE::CFutureResultSource* cancel )
        : CIAssetConversionObserver()
        , m_store( store )
        , m_ingestId( ingestId )
        , m_packId( packId )
        , m_cancel( cancel )
    {

    }

    virtual bool IsCancelRequested( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NULL != m_cancel && m_cancel->IsCancellationRequested();
    }

    virtual void OnConversionProgress( UInt32 done, UInt32 total, const CORE::CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_store.NotifyConversionEvent( CPrimaryAssetStore::ConversionProgressEvent, m_ingestId, m_packId, true, done, total, assetPath, ASSETSTORE_STATUS_OK,
                                       "Converted " + CORE::ToString( done ) + " of " + CORE::ToString( total ) );
    }

    private:

    CPrimaryAssetStore& m_store;
    CORE::CString m_ingestId;
    CORE::CString m_packId;
    const CORE::CFutureResultSource* m_cancel;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetIngestPipeline::CAssetIngestPipeline( CPrimaryAssetStore& store, const CORE::CFutureResultSource* cancel )
    : m_store( store )
    , m_cancel( cancel )
    , m_archiveMounts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestPipeline::IsCancelRequested( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_cancel && m_cancel->IsCancellationRequested();
}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestPipeline::IsInsideArchive( const CAssetIngestJob& job, const CAssetRecord& asset ) const
{GUCEF_TRACE;

    CORE::CString relative = asset.originalPath;
    if ( !job.contentPrefix.IsNULLOrEmpty() && 0 == relative.HasSubstr( job.contentPrefix, true ) )
        relative = relative.CutChars( job.contentPrefix.Length(), true );
    for ( size_t a=0; a<job.archives.size(); ++a )
        if ( 0 == relative.HasSubstr( job.archives[ a ] + "/", true ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetIngestPipeline::MakeId( const CORE::CString& prefix )
{GUCEF_TRACE;

    return prefix + HexOf( (UInt64) NowUtcMs(), 11 ) + HexOf( CORE::CRandom::GetRandomUInt32(), 8 );
}

/*-------------------------------------------------------------------------*/

void
CAssetIngestPipeline::ListFiles( const CORE::CString& vfsFolder, CORE::CString::StringVector& relativePaths )
{GUCEF_TRACE;

    relativePaths.clear();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TStringVector files;
    vfs.GetFileList( files, vfsFolder, true, true, CORE::CString::Empty, GUCEF_UINT32MAX );
    const CORE::CString folder = NormalizeVfsPath( vfsFolder );
    CORE::CString::StringSet unique;
    for ( size_t f=0; f<files.size(); ++f )
    {
        CORE::CString path = NormalizeVfsPath( files[ f ] );
        if ( !folder.IsNULLOrEmpty() && 0 == path.HasSubstr( folder + "/", true ) )
            path = path.CutChars( folder.Length() + 1, true );
        if ( !path.IsNULLOrEmpty() )
            unique.insert( path );
    }
    relativePaths.assign( unique.begin(), unique.end() );
}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestPipeline::CopyTree( const CORE::CString& fromFolder, const CORE::CString& toFolder )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CString::StringVector files;
    ListFiles( fromFolder, files );
    for ( size_t f=0; f<files.size(); ++f )
    {
        const CORE::CString target = JoinVfs( toFolder, files[ f ] );
        const CORE::CString parent = ParentOf( target );
        if ( !parent.IsNULLOrEmpty() && !vfs.DirExists( parent ) )
            vfs.CreateDir( parent );
        if ( !vfs.CopyFile( JoinVfs( fromFolder, files[ f ] ), target, true ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetIngestPipeline::FindManifest( const CPrimaryAssetStore& store, const CORE::CString& ingestId )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CPrimaryAssetStoreSettings settings = store.GetSettings();
    for ( size_t l=0; l<settings.locations.size(); ++l )
    {
        const CORE::CString path = JoinVfs( JoinVfs( settings.locations[ l ].vfsRoot, "staging/" + ingestId ), CAssetIngestJob::ManifestFileName );
        if ( vfs.FileExists( path ) )
            return path;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CAssetIngestPipeline::SaveManifest( const CAssetIngestJob& job ) const
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( job.stagingFolder ) )
        vfs.CreateDir( job.stagingFolder );
    CORE::CDataNode root( "ingest", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNodeSerializableSettings settings;
    if ( !job.Serialize( root, settings ) )
        return false;
    return vfs.StoreAsFile( job.GetManifestPath(), root, "json", true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetIngestPipeline::ResolveAssetPath( const CAssetIngestJob& job, const CAssetRecord& asset ) const
{GUCEF_TRACE;

    CORE::CString relative = asset.originalPath;
    if ( !job.contentPrefix.IsNULLOrEmpty() && 0 == relative.HasSubstr( job.contentPrefix, true ) )
        relative = relative.CutChars( job.contentPrefix.Length(), true );

    // a file inside an archive: through the mount of the archive
    CORE::CString::StringMap::const_iterator m = m_archiveMounts.begin();
    while ( m != m_archiveMounts.end() )
    {
        if ( 0 == relative.HasSubstr( (*m).first + "/", true ) )
            return JoinVfs( (*m).second, relative.CutChars( (*m).first.Length() + 1, true ) );
        ++m;
    }
    return JoinVfs( job.contentRoot, relative );
}

/*-------------------------------------------------------------------------*/

void
CAssetIngestPipeline::MountArchives( CAssetIngestJob& job )
{GUCEF_TRACE;

    UnmountArchives();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( size_t a=0; a<job.archives.size(); ++a )
    {
        const CORE::CString archivePath = JoinVfs( job.contentRoot, job.archives[ a ] );
        CORE::CString archiveType;
        if ( !vfs.TryGetArchiveTypeForFile( archivePath, archiveType ) )
            continue;
        const CORE::CString mountName = "assetstore-ingest-" + job.ingestId + "-" + CORE::ToString( (UInt32) a );
        const CORE::CString mountPoint = "assetstore-ingest/" + job.ingestId + "/" + CORE::ToString( (UInt32) a );
        if ( vfs.MountArchive( mountName, archivePath, archiveType, mountPoint, false, false ) )
            m_archiveMounts[ job.archives[ a ] ] = mountPoint;
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetIngestPipeline::UnmountArchives( void )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    CORE::CString::StringMap::const_iterator m = m_archiveMounts.begin();
    while ( m != m_archiveMounts.end() )
    {
        CORE::CString::StringVector parts = (*m).second.ParseElements( '/', false );
        if ( parts.size() >= 3 )
            vfs.UnmountArchiveByName( "assetstore-ingest-" + parts[ 1 ] + "-" + parts[ 2 ] );
        ++m;
    }
    m_archiveMounts.clear();
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::Start( const CAssetIngestRequest& request, CAssetIngestResult& result )
{GUCEF_TRACE;

    const CPrimaryAssetStoreSettings settings = m_store.GetSettings();
    CAssetIngestJob job;
    job.ingestId = MakeId( "ig" );
    job.request = request;
    job.request.sourceId = request.sourceId.IsNULLOrEmpty() ? CPrimaryAssetStoreSettings::DiskSourceId : request.sourceId.Lowercase();
    job.startedAtMs = NowUtcMs();

    const CAssetSourcePolicy policy = settings.GetPolicy( job.request.sourceId );
    job.locationId = !request.locationId.IsNULLOrEmpty() ? request.locationId : ( !policy.locationId.IsNULLOrEmpty() ? policy.locationId : settings.defaultLocationId );
    if ( job.request.sourceId != CPrimaryAssetStoreSettings::DiskSourceId )
    {
        // a download: its location is the one whose staging area holds it
        const CORE::CString location = m_store.FindLocationOfVfsPath( request.contentPath );
        if ( !location.IsNULLOrEmpty() )
            job.locationId = location;
    }
    job.pack.packId = MakeId( "pk" );
    job.pack.sourceId = job.request.sourceId;
    job.pack.locationId = job.locationId;
    job.pack.keep = request.keep.IsNULLOrEmpty() ? policy.keep : AssetKeepPolicyFromString( request.keep, policy.keep );
    job.pack.state = ASSETPACK_STATE_STAGED;

    result = CAssetIngestResult();
    result.ingestId = job.ingestId;
    result.packId = job.pack.packId;

    const CORE::CString locationRoot = m_store.GetLocationRoot( job.locationId );
    if ( locationRoot.IsNULLOrEmpty() )
    {
        result.status = ASSETSTORE_STATUS_NOT_FOUND;
        result.message = "the storage location " + job.locationId + " does not exist";
        return result.status;
    }
    if ( !m_store.IsLocationOnline( job.locationId ) )
    {
        result.status = ASSETSTORE_STATUS_UNAVAILABLE;
        result.message = "the storage location " + job.locationId + " is not reachable";
        return result.status;
    }
    job.stagingFolder = JoinVfs( locationRoot, "staging/" + job.ingestId );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: Ingest " << job.ingestId << " of " << request.contentPath << " from source " << job.request.sourceId );
    return Run( job, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::Resume( const CORE::CString& ingestId, CAssetIngestResult& result )
{GUCEF_TRACE;

    result = CAssetIngestResult();
    result.ingestId = ingestId;
    const CORE::CString manifestPath = FindManifest( m_store, ingestId );
    if ( manifestPath.IsNULLOrEmpty() )
    {
        result.status = ASSETSTORE_STATUS_NOT_FOUND;
        result.message = "there is no staged ingest " + ingestId;
        return result.status;
    }
    CORE::CDataNode root;
    CAssetIngestJob job;
    CORE::CDataNodeSerializableSettings settings;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( root, manifestPath, "json" ) || !job.Deserialize( root, settings ) )
    {
        result.status = ASSETSTORE_STATUS_IO_ERROR;
        result.message = "the manifest " + manifestPath + " cannot be read";
        return result.status;
    }
    result.packId = job.pack.packId;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: Resuming ingest " << ingestId << " after step " << AssetIngestStepToString( job.lastStep ) );
    job.failedStep = ASSETINGEST_STEP_NONE;
    job.failureStatus = ASSETSTORE_STATUS_OK;
    job.failureMessage.Clear();
    return Run( job, result );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::RunStep( TAssetIngestStep step, CAssetIngestJob& job, CAssetIngestResult& result, CORE::CString& message )
{GUCEF_TRACE;

    switch ( step )
    {
        case ASSETINGEST_STEP_STAGE: return Stage( job, message );
        case ASSETINGEST_STEP_IDENTIFY: return Identify( job, result, message );
        case ASSETINGEST_STEP_NORMALISE: return Normalise( job, message );
        case ASSETINGEST_STEP_ANALYSE: return Analyse( job, message );
        case ASSETINGEST_STEP_CONVERT: return Convert( job, message );
        case ASSETINGEST_STEP_PLACE: return Place( job, message );
        case ASSETINGEST_STEP_INDEX: return Index( job, message );
        case ASSETINGEST_STEP_RECORD: return Record( job, "ingested", ASSETINGEST_STEP_RECORD, message );
        default: return ASSETSTORE_STATUS_OK;
    }
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::Run( CAssetIngestJob& job, CAssetIngestResult& result )
{GUCEF_TRACE;

    if ( job.lastStep >= ASSETINGEST_STEP_IDENTIFY )
        MountArchives( job );

    for ( int s=(int) job.lastStep + 1; s<=(int) ASSETINGEST_STEP_RECORD; ++s )
    {
        const TAssetIngestStep step = (TAssetIngestStep) s;
        CORE::CString message;
        TAssetStoreStatus status = ASSETSTORE_STATUS_FAILED;
        if ( step <= ASSETINGEST_STEP_PLACE && IsCancelRequested() )
        {
            // asked to stop: the pack stays staged and a resume continues from this step
            status = ASSETSTORE_STATUS_CANCELLED;
            message = "the ingest was cancelled";
        }
        else
        {
            try
            {
                status = RunStep( step, job, result, message );
            }
            catch ( ... )
            {
                status = ASSETSTORE_STATUS_FAILED;
                message = "an exception ended the step";
            }
        }

        if ( ASSETSTORE_STATUS_DUPLICATE == status )
        {
            // not ingested again: the duplicate is recorded and the staged ingest goes
            UnmountArchives();
            CORE::CString recordMessage;
            m_store.RemovePackRows( job.pack.packId );
            job.problems.push_back( "a duplicate of pack " + result.duplicateOfPackId );
            Record( job, "duplicate", step, recordMessage );
            RevalidateDuplicate( job, result.duplicateOfPackId );
            if ( job.request.repairOriginals && job.request.verifyPackId == result.duplicateOfPackId )
                RepairOriginals( job, result.duplicateOfPackId, result.repairedFiles );
            result.status = ASSETSTORE_STATUS_DUPLICATE;
            result.message = message;
            result.problems = job.problems;
            m_store.NotifyIngestEvent( CPrimaryAssetStore::IngestFinishedEvent, job.ingestId, job.pack.packId, step, status, message, job.request.downloadJobId );
            return status;
        }
        if ( ASSETSTORE_STATUS_OK != status )
        {
            UnmountArchives();
            job.failedStep = step;
            job.failureStatus = status;
            job.failureMessage = message;
            SaveManifest( job );
            if ( step > ASSETINGEST_STEP_STAGE || !job.pack.title.IsNULLOrEmpty() )
                WritePackRow( job, ASSETPACK_STATE_FAILED, "step " + CORE::CString( AssetIngestStepToString( step ) ) + ": " + message );

            CORE::CString recordMessage;
            CAssetIngestJob failed( job );
            failed.problems.push_back( message );
            Record( failed, "failed", step, recordMessage );

            result.status = status;
            result.state = ASSETPACK_STATE_FAILED;
            result.failedStep = step;
            result.message = message;
            result.problems = job.problems;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: Ingest " << job.ingestId << " failed in step " << AssetIngestStepToString( step ) << ": " << message );
            m_store.NotifyIngestEvent( CPrimaryAssetStore::IngestFailedEvent, job.ingestId, job.pack.packId, step, status, message, job.request.downloadJobId );
            return status;
        }
        job.lastStep = step;
        if ( step < ASSETINGEST_STEP_RECORD )
            SaveManifest( job );
        m_store.NotifyIngestEvent( CPrimaryAssetStore::IngestProgressEvent, job.ingestId, job.pack.packId, step, status, CORE::CString::Empty, job.request.downloadJobId );
    }
    UnmountArchives();

    result.status = ASSETSTORE_STATUS_OK;
    result.state = job.pack.state;
    result.assetCount = (UInt32) job.assets.size();
    result.problems = job.problems;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: Ingested pack " << job.pack.packId << " \"" << job.pack.title << "\" with " << job.assets.size() << " asset(s)" );
    m_store.NotifyIngestEvent( CPrimaryAssetStore::IngestFinishedEvent, job.ingestId, job.pack.packId, ASSETINGEST_STEP_DONE, ASSETSTORE_STATUS_OK, CORE::CString::Empty, job.request.downloadJobId );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::Stage( CAssetIngestJob& job, CORE::CString& message )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CPrimaryAssetStoreSettings settings = m_store.GetSettings();
    const CORE::CString contentPath = NormalizeVfsPath( job.request.contentPath );
    if ( contentPath.IsNULLOrEmpty() || ( !vfs.DirExists( contentPath ) && !vfs.FileExists( contentPath ) ) )
    {
        message = "there is nothing at " + job.request.contentPath;
        return ASSETSTORE_STATUS_NOT_FOUND;
    }
    if ( !vfs.DirExists( job.stagingFolder ) && !vfs.CreateDir( job.stagingFolder ) )
    {
        message = "the staging folder " + job.stagingFolder + " cannot be made";
        return ASSETSTORE_STATUS_IO_ERROR;
    }

    // the details of a store item when only its id is known
    CAssetStoreItem& item = job.request.item;
    if ( item.itemId.IsNULLOrEmpty() && !job.request.itemId.IsNULLOrEmpty() )
    {
        item.itemId = job.request.itemId;
        TAssetStoreProviderPtr provider;
        if ( CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( job.request.sourceId, provider ) )
        {
            CAssetStoreItem details;
            if ( ASSETSTORE_STATUS_OK == provider->GetItem( job.request.itemId, details ) )
                item = details;
            else
                job.problems.push_back( "the details of item " + job.request.itemId + " could not be had from the store" );
        }
    }
    if ( item.providerId.IsNULLOrEmpty() )
        item.providerId = job.request.sourceId;

    const bool isFile = vfs.FileExists( contentPath ) && !vfs.DirExists( contentPath );
    if ( job.request.sourceId != CPrimaryAssetStoreSettings::DiskSourceId )
    {
        // a download in the staging area of its location
        job.contentRoot = contentPath;
        job.contentInStaging = true;
        job.inPlace = false;
    }
    else
    {
        const bool copy = isFile || ( job.request.copyContent < 0 ? settings.copyDiskPacks : 0 != job.request.copyContent );
        if ( copy )
        {
            const CORE::CString target = JoinVfs( job.stagingFolder, "content" );
            if ( !vfs.DirExists( target ) )
                vfs.CreateDir( target );
            const bool copied = isFile ? vfs.CopyFile( contentPath, JoinVfs( target, LastPartOf( contentPath ) ), true ) : CopyTree( contentPath, target );
            if ( !copied )
            {
                message = "the pack could not be copied into the staging area";
                return ASSETSTORE_STATUS_IO_ERROR;
            }
            job.contentRoot = target;
            job.contentInStaging = true;
            job.inPlace = false;
        }
        else
        {
            const CORE::CString location = m_store.FindLocationOfVfsPath( contentPath );
            if ( location.IsNULLOrEmpty() )
            {
                message = "a pack referenced in place has to be inside a storage location: " + contentPath;
                return ASSETSTORE_STATUS_INVALID_ARGUMENT;
            }
            job.locationId = location;
            job.pack.locationId = location;
            job.contentRoot = contentPath;
            job.contentInStaging = false;
            job.inPlace = true;
            const CORE::CString root = NormalizeVfsPath( m_store.GetLocationRoot( location ) );
            job.pack.rootPath = contentPath.Length() > root.Length() ? contentPath.CutChars( root.Length() + 1, true ) : CORE::CString::Empty;
        }
    }
    job.contentPrefix.Clear();
    job.pack.title = item.title.IsNULLOrEmpty() ? CAssetDiskPackNormalizer::TitleFromName( LastPartOf( contentPath ) ) : item.title;
    job.pack.sourceItemId = item.itemId;
    return WritePackRow( job, ASSETPACK_STATE_STAGED, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CAssetIngestPipeline::AddAsset( CAssetIngestJob& job, const CORE::CString& relativePath, const CORE::CString& vfsPath, Int64 size, const CORE::CString& digest ) const
{GUCEF_TRACE;

    CAssetRecord asset;
    CAssetDigest idDigest( CAssetDigest::Sha256 );
    const CORE::CString key = job.pack.packId + "\n" + relativePath;
    idDigest.Update( key.C_String(), ByteLengthOf( key ) );
    asset.assetId = "as" + idDigest.FinalHex().SubstrToIndex( 20, true );
    asset.packId = job.pack.packId;
    asset.name = CAssetNameHints::GetBaseName( relativePath );
    asset.kind = CAssetNameHints::GuessKind( relativePath );
    asset.sourceFormat = CAssetNameHints::FormatFromExtension( CAssetNameHints::GetExtension( relativePath ) );
    asset.originalPath = relativePath;
    asset.sizeBytes = size;
    asset.digest = digest;
    CAssetFileRecord file;
    file.path = relativePath;
    file.sizeBytes = size;
    file.digest = digest;
    file.role = "main";
    asset.files.push_back( file );
    job.assets.push_back( asset );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::Identify( CAssetIngestJob& job, CAssetIngestResult& result, CORE::CString& message )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    job.assets.clear();
    job.archives.clear();
    job.pack.sizeBytes = 0;
    job.pack.fileCount = 0;

    CORE::CString::StringVector files;
    ListFiles( job.contentRoot, files );
    for ( size_t f=0; f<files.size(); ++f )
    {
        const CORE::CString& relative = files[ f ];
        const CORE::CString lastPart = LastPartOf( relative );
        if ( lastPart == CAssetIngestJob::ManifestFileName || lastPart.Lowercase() == "pack.json" || '.' == lastPart.C_String()[ 0 ] ||
             CAssetNameHints::GetExtension( lastPart ) == "part" )
            continue;
        CORE::CString reason;
        if ( !CAssetStoreSafeNames::IsSafeRelativeName( relative, reason ) )
        {
            job.problems.push_back( "skipped the file \"" + relative + "\": " + reason );
            continue;
        }
        const CORE::CString path = JoinVfs( job.contentRoot, relative );
        CORE::CString digest;
        UInt64 size = 0;
        if ( !CAssetDigest::ComputeVfsFileDigest( path, CAssetDigest::Sha256, digest, &size ) )
        {
            message = "the file " + path + " cannot be read";
            return ASSETSTORE_STATUS_IO_ERROR;
        }
        AddAsset( job, relative, path, (Int64) size, digest );
        job.pack.sizeBytes += (Int64) size;
        ++job.pack.fileCount;
        if ( CAssetNameHints::IsArchiveExtension( CAssetNameHints::GetExtension( relative ) ) )
            job.archives.push_back( relative );
    }

    // the content of the archives the VFS can mount
    MountArchives( job );
    CORE::CString::StringMap::const_iterator m = m_archiveMounts.begin();
    while ( m != m_archiveMounts.end() )
    {
        CORE::CString::StringVector inner;
        ListFiles( (*m).second, inner );
        for ( size_t i=0; i<inner.size(); ++i )
        {
            CORE::CString reason;
            if ( !CAssetStoreSafeNames::IsSafeRelativeName( inner[ i ], reason ) )
            {
                job.problems.push_back( "skipped the file \"" + inner[ i ] + "\" of the archive " + (*m).first + ": " + reason );
                continue;
            }
            CORE::CString digest;
            UInt64 size = 0;
            const CORE::CString path = JoinVfs( (*m).second, inner[ i ] );
            if ( !CAssetDigest::ComputeVfsFileDigest( path, CAssetDigest::Sha256, digest, &size ) )
            {
                job.problems.push_back( "the file \"" + inner[ i ] + "\" of the archive " + (*m).first + " cannot be read" );
                continue;
            }
            AddAsset( job, (*m).first + "/" + inner[ i ], path, (Int64) size, digest );
        }
        ++m;
    }
    if ( job.assets.empty() )
    {
        message = "the pack has no files";
        return ASSETSTORE_STATUS_NOT_FOUND;
    }

    // duplicates
    CORE::CString duplicateOf;
    const TAssetStoreStatus status = FindDuplicate( job, duplicateOf );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        message = "the catalogue cannot be searched for duplicates";
        return status;
    }
    if ( !duplicateOf.IsNULLOrEmpty() )
    {
        const CPrimaryAssetStoreSettings settings = m_store.GetSettings();
        TAssetDuplicatePolicy policy = job.request.duplicatePolicy.IsNULLOrEmpty() ? settings.duplicatePolicy : AssetDuplicatePolicyFromString( job.request.duplicatePolicy, settings.duplicatePolicy );
        if ( !job.request.verifyPackId.IsNULLOrEmpty() )
            policy = ASSETDUPLICATE_SKIP;      // a verified download again of the same files adds nothing
        result.duplicateOfPackId = duplicateOf;
        if ( ASSETDUPLICATE_SKIP == policy )
        {
            message = "the pack is in the catalogue already as " + duplicateOf;
            return ASSETSTORE_STATUS_DUPLICATE;
        }
        job.problems.push_back( "kept as a duplicate of pack " + duplicateOf );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::FindDuplicate( const CAssetIngestJob& job, CORE::CString& duplicateOfPackId ) const
{GUCEF_TRACE;

    duplicateOfPackId.Clear();
    CAssetCatalogue& catalogue = m_store.GetCatalogue();

    // a download again of the originals of a pack: the same files (digests, as often as they occur) are that pack, whatever the
    // revision says; other files are a new pack, never a duplicate of another one
    if ( !job.request.verifyPackId.IsNULLOrEmpty() )
    {
        TAssetRecordVector stored;
        const TAssetStoreStatus status = m_store.GetAssets( job.request.verifyPackId, stored );
        if ( ASSETSTORE_STATUS_OK != status )
            return status;
        CORE::CString::StringVector storedDigests;
        CORE::CString::StringVector newDigests;
        for ( size_t a=0; a<stored.size(); ++a )
            if ( !stored[ a ].digest.IsNULLOrEmpty() )
                storedDigests.push_back( stored[ a ].digest.Lowercase() );
        for ( size_t a=0; a<job.assets.size(); ++a )
            if ( !job.assets[ a ].digest.IsNULLOrEmpty() )
                newDigests.push_back( job.assets[ a ].digest.Lowercase() );
        std::sort( storedDigests.begin(), storedDigests.end() );
        std::sort( newDigests.begin(), newDigests.end() );
        if ( !storedDigests.empty() && storedDigests == newDigests )
            duplicateOfPackId = job.request.verifyPackId;
        return ASSETSTORE_STATUS_OK;
    }

    // the same item of the same store: with the store facts of the library sync (schema 5) a pack of another format or of
    // another revision of the item is not the same; a download of the sync is the same only as a current pack of its format
    // with the revision it expects (a newer revision is a new version of the item)
    if ( job.pack.sourceId != CPrimaryAssetStoreSettings::DiskSourceId && !job.request.item.itemId.IsNULLOrEmpty() )
    {
        GUCEF::map< CORE::CString, CAssetPackSourceRecord > factsOfPacks;
        CAssetSyncMarkStore& syncStore = m_store.GetSyncStore();
        if ( syncStore.IsAvailable() )
        {
            TAssetPackSourceRecordVector facts;
            if ( ASSETSTORE_STATUS_OK == syncStore.GetPackSourcesOfItem( job.pack.sourceId, job.request.item.itemId, facts ) )
                for ( size_t f=0; f<facts.size(); ++f )
                    factsOfPacks[ facts[ f ].packId ] = facts[ f ];
        }
        const bool fromSync = !job.request.expectedRevision.IsNULLOrEmpty();
        CORE::CDataTableQuery query;
        query.WhereEquals( "source_id", CORE::CVariant( job.pack.sourceId ) );
        query.WhereEquals( "source_item_id", CORE::CVariant( job.request.item.itemId ) );
        CAssetCatalogue::TRowVector rows;
        const TAssetStoreStatus status = catalogue.Select( "packs", query, rows );
        if ( ASSETSTORE_STATUS_OK != status )
            return status;
        for ( size_t r=0; r<rows.size(); ++r )
        {
            const CORE::CString packId = CAssetCatalogue::GetText( rows[ r ], "pack_id" );
            const CORE::CString state = CAssetCatalogue::GetText( rows[ r ], "state" );
            const CORE::CString version = CAssetCatalogue::GetText( rows[ r ], "source_version" );
            if ( packId == job.pack.packId || !AssetPackStateIsIngested( state ) )
                continue;
            GUCEF::map< CORE::CString, CAssetPackSourceRecord >::const_iterator fact = factsOfPacks.find( packId );
            const bool hasFact = fact != factsOfPacks.end();
            if ( hasFact && !(*fact).second.IsCurrent() )
                continue;
            if ( hasFact && !(*fact).second.MatchesFormat( job.request.formatId ) )
                continue;
            if ( fromSync )
            {
                if ( hasFact && CAssetItemRevision::COMPARISON_SAME == CAssetItemRevision::Compare( (*fact).second.revision, job.request.expectedRevision ) )
                {
                    duplicateOfPackId = packId;
                    return ASSETSTORE_STATUS_OK;
                }
                continue;
            }
            if ( version == job.request.item.version )
            {
                duplicateOfPackId = packId;
                return ASSETSTORE_STATUS_OK;
            }
        }
    }

    // the same files
    CORE::CString::StringSet digests;
    for ( size_t a=0; a<job.assets.size(); ++a )
        digests.insert( job.assets[ a ].digest );
    CAssetCatalogue::TRowVector rows;
    const TAssetStoreStatus status = catalogue.SelectIn( "assets", "digest", digests, rows );
    if ( ASSETSTORE_STATUS_OK != status )
        return status;
    CORE::CString::StringMapSet digestsOfPack;
    for ( size_t r=0; r<rows.size(); ++r )
    {
        const CORE::CString packId = CAssetCatalogue::GetText( rows[ r ], "pack_id" );
        if ( packId != job.pack.packId )
            digestsOfPack[ packId ].insert( CAssetCatalogue::GetText( rows[ r ], "digest" ) );
    }
    CORE::CString::StringMapSet::const_iterator p = digestsOfPack.begin();
    while ( p != digestsOfPack.end() )
    {
        if ( (*p).second.size() == digests.size() )
        {
            CAssetPackRecord other;
            if ( ASSETSTORE_STATUS_OK == m_store.GetPack( (*p).first, other ) &&
                 AssetPackStateIsIngested( other.state ) )
            {
                duplicateOfPackId = (*p).first;
                return ASSETSTORE_STATUS_OK;
            }
        }
        ++p;
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::Normalise( CAssetIngestJob& job, CORE::CString& message )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CAssetVocabulary& vocabulary = m_store.GetVocabulary();

    CAssetNormalizationInput input;
    input.sourceId = job.pack.sourceId;
    input.item = job.request.item;
    input.packName = SourceNameOf( job );
    const CORE::CString packJsonPath = JoinVfs( job.contentRoot, "pack.json" );
    if ( vfs.FileExists( packJsonPath ) )
    {
        input.hasPackJson = vfs.LoadFile( input.packJson, packJsonPath, "json" );
        if ( !input.hasPackJson )
            job.problems.push_back( "the pack.json of the pack cannot be read" );
    }
    for ( size_t a=0; a<job.assets.size(); ++a )
        input.fileNames.push_back( job.assets[ a ].originalPath );

    // the normaliser of the store, else the generic one
    CAssetStoreItemNormalizer itemNormalizer;
    CAssetDiskPackNormalizer diskNormalizer;
    CIAssetMetadataNormalizer* normalizer = &diskNormalizer;
    if ( job.pack.sourceId != CPrimaryAssetStoreSettings::DiskSourceId )
    {
        normalizer = &itemNormalizer;
        TAssetStoreProviderPtr provider;
        if ( CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( job.pack.sourceId, provider ) && GUCEF_NULL != provider->GetMetadataNormalizer() )
            normalizer = provider->GetMetadataNormalizer();
    }

    // what the pipeline set stays as it is
    CAssetPackRecord pack;
    pack.packId = job.pack.packId;
    pack.sourceId = job.pack.sourceId;
    pack.locationId = job.pack.locationId;
    pack.rootPath = job.pack.rootPath;
    pack.keep = job.pack.keep;
    pack.sizeBytes = job.pack.sizeBytes;
    pack.fileCount = job.pack.fileCount;
    pack.state = job.pack.state;
    const TAssetStoreStatus status = normalizer->Normalize( input, vocabulary, pack );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        message = "the metadata normaliser " + normalizer->GetName() + " failed";
        return status;
    }
    if ( pack.title.IsNULLOrEmpty() )
        pack.title = job.pack.title;
    job.pack = pack;

    // the assets: keywords of their names, their category by the rules on their names, the pack's otherwise
    const CAssetKeywordNormalizer& keywords = vocabulary.GetKeywordNormalizer();
    const CORE::CString packCategory = job.pack.categories.empty() ? CORE::CString::Empty : *job.pack.categories.begin();
    for ( size_t a=0; a<job.assets.size(); ++a )
    {
        CAssetRecord& asset = job.assets[ a ];
        asset.keywords.clear();
        keywords.Normalize( asset.name, "name", CIAssetMetadataNormalizer::NameWeight, asset.keywords );
        keywords.Normalize( asset.kind, "derived", CIAssetMetadataNormalizer::DerivedWeight, asset.keywords );
        CORE::CString::StringSet words;
        for ( size_t k=0; k<asset.keywords.size(); ++k )
            if ( asset.keywords[ k ].field == "name" )
                words.insert( asset.keywords[ k ].keyword );
        CORE::CString::StringVector names;
        names.push_back( asset.name );
        CAssetClassification classification;
        vocabulary.ApplyRules( words, names, CORE::CString::Empty, classification );
        asset.categoryId = classification.categories.empty() ? packCategory : *classification.categories.begin();
        if ( !classification.kind.IsNULLOrEmpty() && asset.kind == "other" )
            asset.kind = classification.kind;
        job.pack.kinds.insert( asset.kind );
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CAssetIngestPipeline::DeriveFromContent( CAssetIngestJob& job ) const
{GUCEF_TRACE;

    // names of the pack: base model name to its index
    GUCEF::map< CORE::CString, size_t > models;
    for ( size_t a=0; a<job.assets.size(); ++a )
        if ( CAssetNameHints::IsModelKind( job.assets[ a ].kind ) )
            models[ job.assets[ a ].name.Lowercase() ] = a;

    Int64 maxTriangles = -1;
    for ( size_t a=0; a<job.assets.size(); ++a )
    {
        CAssetRecord& asset = job.assets[ a ];
        if ( asset.texture.isSet && !asset.texture.role.IsNULLOrEmpty() )
        {
            job.pack.pbrRoles.insert( asset.texture.role );
            if ( asset.texture.role == "orm" )
            {
                job.pack.pbrRoles.insert( "ao" );
                job.pack.pbrRoles.insert( "roughness" );
                job.pack.pbrRoles.insert( "metallic" );
            }
        }
        if ( !CAssetNameHints::IsModelKind( asset.kind ) || !asset.model.isSet )
            continue;
        if ( asset.model.triangles > maxTriangles )
            maxTriangles = asset.model.triangles;

        // collision meshes and levels of detail that go with a model by their names
        const CORE::CString name = asset.name.Lowercase();
        if ( CAssetNameHints::IsCollisionName( name ) )
            continue;
        CORE::CString base = name;
        if ( CAssetNameHints::GetLodIndex( name ) >= 0 )
            base = name.SubstrToIndex( (UInt32) name.HasSubstr( "_lod", false ), true );
        for ( GUCEF::map< CORE::CString, size_t >::const_iterator m=models.begin(); m!=models.end(); ++m )
        {
            const CORE::CString& other = (*m).first;
            if ( other == name )
                continue;
            if ( CAssetNameHints::IsCollisionName( other ) && other.HasSubstr( base, true ) >= 0 )
                asset.model.hasCollision = true;
            if ( 0 == other.HasSubstr( base + "_lod", true ) && CAssetNameHints::GetLodIndex( other ) >= 0 && asset.model.lods < CAssetNameHints::GetLodIndex( other ) + 1 )
                asset.model.lods = CAssetNameHints::GetLodIndex( other ) + 1;
        }
    }

    // a pack of light models only is low poly
    if ( maxTriangles >= 0 && maxTriangles <= 3000 )
        job.pack.artStyles.insert( "low-poly" );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::Analyse( CAssetIngestJob& job, CORE::CString& message )
{GUCEF_TRACE;

    const CPrimaryAssetStoreSettings settings = m_store.GetSettings();
    CORE::CString::StringVector paths;
    for ( size_t a=0; a<job.assets.size(); ++a )
        paths.push_back( ResolveAssetPath( job, job.assets[ a ] ) );

    CAssetAnalysisBatch batch( CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry(), job.assets, paths );
    const TAssetStoreStatus status = batch.Run( settings.analysisThreadPoolName, settings.analysisConcurrency );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        message = batch.GetMessage();
        return status;
    }
    for ( size_t a=0; a<job.assets.size(); ++a )
        job.pack.analyzers.insert( job.assets[ a ].analyzers.begin(), job.assets[ a ].analyzers.end() );
    DeriveFromContent( job );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::Convert( CAssetIngestJob& job, CORE::CString& message )
{GUCEF_TRACE;

    if ( ASSETKEEP_ORIGINAL == job.pack.keep )
        return ASSETSTORE_STATUS_OK;

    CIAssetConverterHook* hook = m_store.GetConverterHook();
    bool convertible = false;
    for ( size_t a=0; GUCEF_NULL != hook && a<job.assets.size() && !convertible; ++a )
        convertible = hook->CanConvert( job.assets[ a ] );
    if ( GUCEF_NULL == hook || !convertible )
    {
        // kept as original until a converter can do it: marked for conversion later
        job.pack.state = ASSETPACK_STATE_CONVERSION_PENDING;
        return ASSETSTORE_STATUS_OK;
    }
    if ( job.inPlace )
    {
        // the converted files would go into the folder of the pack, which the store does not write to
        job.pack.state = ASSETPACK_STATE_CONVERSION_PENDING;
        job.pack.stateReason = "the pack is referenced in place: it cannot be converted";
        job.problems.push_back( job.pack.stateReason );
        return ASSETSTORE_STATUS_OK;
    }

    const CORE::CString profile = hook->GetProfileName();
    CAssetConversionRequest request;
    request.pack = job.pack;
    request.sourceVfsPath = job.contentRoot;
    request.sourcePrefix = job.contentPrefix;
    request.convertedVfsFolder = JoinVfs( job.stagingFolder, CPackStorageLayout::IntermediateArea );
    request.convertedPathPrefix = CPackStorageLayout::IntermediateArea;
    request.cookedVfsFolder = JoinVfs( job.stagingFolder, CPackStorageLayout::CookedArea );
    request.cookedPathPrefix = CPackStorageLayout::CookedArea;
    CIngestConversionObserver observer( m_store, job.ingestId, job.pack.packId, m_cancel );
    request.observer = &observer;
    request.reconvert = false;
    request.maxThreads = job.request.conversionThreads;
    request.threadPoolName = job.request.conversionThreadPoolName;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: Ingest " << job.ingestId << " converts the pack with " << hook->GetName() << " for the profile " << profile );
    m_store.NotifyConversionEvent( CPrimaryAssetStore::ConversionProgressEvent, job.ingestId, job.pack.packId, true, 0, (UInt32) job.assets.size(), CORE::CString::Empty,
                                   ASSETSTORE_STATUS_OK, "Starting the conversion" );

    TAssetRecordVector converted = job.assets;
    CAssetConversionOutcome hookOutcome;
    hookOutcome.packId = job.pack.packId;
    hookOutcome.jobId = job.ingestId;
    TAssetStoreStatus hookStatus = ASSETSTORE_STATUS_FAILED;
    try
    {
        hookStatus = hook->ConvertPack( request, converted, hookOutcome );
    }
    catch ( ... )
    {
        hookStatus = ASSETSTORE_STATUS_FAILED;
        hookOutcome.message = "an exception ended the conversion";
    }
    return CAssetPackConversion::ApplyOutcome( hookStatus, hookOutcome, converted, job.pack, job.assets, job.problems, message );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::Place( CAssetIngestJob& job, CORE::CString& message )
{GUCEF_TRACE;

    if ( job.inPlace )
    {
        job.contentPrefix.Clear();
        return ASSETSTORE_STATUS_OK;
    }
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString locationRoot = m_store.GetLocationRoot( job.locationId );
    const bool sourceIsFile = vfs.FileExists( job.request.contentPath ) && !vfs.DirExists( job.request.contentPath );
    const CORE::CString sourceName = job.request.sourceId == CPrimaryAssetStoreSettings::DiskSourceId || sourceIsFile ?
                                     SourceNameOf( job ) : job.pack.title;
    // the folder an earlier attempt of this ingest chose (kept in the manifest before anything moved), else a new one
    const CPackStorageLayout earlier( job.pack.rootPath );
    const CPackStorageLayout layout( earlier.IsAreaLayout() ? earlier.GetStoredRootPath()
                                                            : CPackStorageLayout::MakeRootPath( m_store.ChoosePackFolder( job.locationId, job.pack.sourceId, sourceName, job.pack.packId ) ) );
    const CORE::CString originalFolder = layout.GetOriginalFolder( locationRoot );
    if ( earlier.IsAreaLayout() )
        m_store.ReservePackFolder( job.locationId, job.pack.packId, earlier.GetPackFolder() );
    else
    {
        // the choice is kept before the move: a restart in the middle of it places the pack in the same folder, never in a second one
        job.pack.rootPath = layout.GetStoredRootPath();
        SaveManifest( job );
    }

    UnmountArchives();
    const bool staged = !job.contentRoot.IsNULLOrEmpty() && vfs.DirExists( job.contentRoot );
    if ( staged && vfs.DirExists( originalFolder ) )
    {
        // the staged files are still there, so an earlier attempt did not finish its copy (a move is one rename): its part goes
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: " << job.ingestId << ": " << originalFolder << " holds an unfinished copy of an earlier attempt; it is made again" );
        vfs.DeleteDir( originalFolder, true );
    }
    if ( !vfs.DirExists( originalFolder ) )
    {
        if ( !vfs.MoveDir( job.contentRoot, originalFolder ) )
        {
            // another file system (a scratch staging area): copied, then the staged files go
            if ( !CopyTree( job.contentRoot, originalFolder ) )
            {
                // nothing half copied stays behind to be taken for the pack later
                vfs.DeleteDir( originalFolder, true );
                message = "the pack could not be placed in " + originalFolder;
                return ASSETSTORE_STATUS_IO_ERROR;
            }
            vfs.DeleteDir( job.contentRoot, true );
        }
    }
    const CORE::CString intermediateStaging = JoinVfs( job.stagingFolder, CPackStorageLayout::IntermediateArea );
    if ( vfs.DirExists( intermediateStaging ) && !vfs.MoveDir( intermediateStaging, layout.GetIntermediateFolder( locationRoot, CORE::CString::Empty ) ) )
        job.problems.push_back( "the intermediate files could not be placed" );
    const CORE::CString cookedStaging = JoinVfs( job.stagingFolder, CPackStorageLayout::CookedArea );
    if ( vfs.DirExists( cookedStaging ) && !vfs.MoveDir( cookedStaging, layout.GetCookedFolder( locationRoot, CORE::CString::Empty ) ) )
        job.problems.push_back( "the cooked files could not be placed" );

    job.contentRoot = originalFolder;
    job.contentPrefix = CPackStorageLayout::OriginalArea + "/";
    job.pack.rootPath = layout.GetStoredRootPath();
    for ( size_t a=0; a<job.assets.size(); ++a )
    {
        CAssetRecord& asset = job.assets[ a ];
        if ( 0 != asset.originalPath.HasSubstr( job.contentPrefix, true ) )
            asset.originalPath = job.contentPrefix + asset.originalPath;
        for ( size_t f=0; f<asset.files.size(); ++f )
            if ( 0 != asset.files[ f ].path.HasSubstr( job.contentPrefix, true ) )
                asset.files[ f ].path = job.contentPrefix + asset.files[ f ].path;
    }
    MountArchives( job );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CAssetIngestPipeline::BuildRows( const CAssetIngestJob& job, GUCEF::map< CORE::CString, CAssetCatalogue::TRowVector >& rows ) const
{GUCEF_TRACE;

    const CAssetPackRecord& pack = job.pack;
    const CORE::CVariant packId( pack.packId );

    CORE::CString::StringSet formats;
    for ( size_t a=0; a<job.assets.size(); ++a )
    {
        const CAssetRecord& asset = job.assets[ a ];
        if ( IsHeldFormatKind( asset.kind ) || asset.sourceFormat == "unreal" || asset.sourceFormat == "unity" )
            formats.insert( asset.sourceFormat );
    }
    for ( CORE::CString::StringSet::const_iterator f=pack.formatsConverted.begin(); f!=pack.formatsConverted.end(); ++f )
        formats.insert( (*f) );

    // the assets and what refers to them
    GUCEF::map< CORE::CString, CAssetCatalogue::TRow > keywordRows;
    for ( size_t a=0; a<job.assets.size(); ++a )
    {
        const CAssetRecord& asset = job.assets[ a ];
        const CORE::CVariant assetId( asset.assetId );
        CAssetCatalogue::TRow row;
        row[ "asset_id" ] = assetId;
        row[ "pack_id" ] = packId;
        row[ "kind" ] = CORE::CVariant( asset.kind );
        row[ "name" ] = CORE::CVariant( asset.name );
        row[ "original_path" ] = CORE::CVariant( asset.originalPath );
        row[ "converted_path" ] = CORE::CVariant( asset.convertedPath );
        row[ "source_format" ] = CORE::CVariant( asset.sourceFormat );
        row[ "converted_format" ] = CORE::CVariant( asset.convertedFormat );
        row[ "size_bytes" ] = CORE::CVariant( asset.sizeBytes );
        row[ "digest" ] = CORE::CVariant( asset.digest );
        row[ "category_id" ] = CORE::CVariant( asset.categoryId );
        rows[ "assets" ].push_back( row );

        if ( asset.model.isSet )
        {
            CAssetCatalogue::TRow props;
            props[ "asset_id" ] = assetId;
            props[ "pack_id" ] = packId;
            props[ "triangles" ] = CORE::CVariant( asset.model.triangles );
            props[ "vertices" ] = CORE::CVariant( asset.model.vertices );
            props[ "sub_meshes" ] = CORE::CVariant( asset.model.subMeshes );
            props[ "bones" ] = CORE::CVariant( asset.model.bones );
            props[ "animations" ] = CORE::CVariant( asset.model.animations );
            props[ "lods" ] = CORE::CVariant( asset.model.lods );
            props[ "bounds_x" ] = CORE::CVariant( asset.model.boundsX );
            props[ "bounds_y" ] = CORE::CVariant( asset.model.boundsY );
            props[ "bounds_z" ] = CORE::CVariant( asset.model.boundsZ );
            props[ "up_axis" ] = CORE::CVariant( asset.model.upAxis );
            props[ "unit" ] = CORE::CVariant( asset.model.unit );
            props[ "rigged" ] = CORE::CVariant( asset.model.rigged );
            props[ "animated" ] = CORE::CVariant( asset.model.animated );
            props[ "has_collision" ] = CORE::CVariant( asset.model.hasCollision );
            rows[ "asset_model_props" ].push_back( props );
        }
        if ( asset.texture.isSet )
        {
            CAssetCatalogue::TRow props;
            props[ "asset_id" ] = assetId;
            props[ "pack_id" ] = packId;
            props[ "width" ] = CORE::CVariant( asset.texture.width );
            props[ "height" ] = CORE::CVariant( asset.texture.height );
            props[ "resolution" ] = CORE::CVariant( asset.texture.width > asset.texture.height ? asset.texture.width : asset.texture.height );
            props[ "channels" ] = CORE::CVariant( asset.texture.channels );
            props[ "bit_depth" ] = CORE::CVariant( asset.texture.bitDepth );
            props[ "colour_space" ] = CORE::CVariant( asset.texture.colourSpace );
            props[ "compression" ] = CORE::CVariant( asset.texture.compression );
            props[ "role" ] = CORE::CVariant( asset.texture.role );
            rows[ "asset_texture_props" ].push_back( props );
        }
        if ( asset.material.isSet )
        {
            CAssetCatalogue::TRow props;
            props[ "asset_id" ] = assetId;
            props[ "pack_id" ] = packId;
            props[ "shading_model" ] = CORE::CVariant( asset.material.shadingModel );
            props[ "texture_count" ] = CORE::CVariant( asset.material.textureCount );
            props[ "pbr_roles" ] = CORE::CVariant( CAssetCatalogue::JoinSet( asset.material.pbrRoles ) );
            rows[ "asset_material_props" ].push_back( props );
        }
        if ( asset.animation.isSet )
        {
            CAssetCatalogue::TRow props;
            props[ "asset_id" ] = assetId;
            props[ "pack_id" ] = packId;
            props[ "length_s" ] = CORE::CVariant( asset.animation.lengthSeconds );
            props[ "frame_rate" ] = CORE::CVariant( asset.animation.frameRate );
            props[ "skeleton" ] = CORE::CVariant( asset.animation.skeleton );
            rows[ "asset_animation_props" ].push_back( props );
        }
        if ( asset.audio.isSet )
        {
            CAssetCatalogue::TRow props;
            props[ "asset_id" ] = assetId;
            props[ "pack_id" ] = packId;
            props[ "length_s" ] = CORE::CVariant( asset.audio.lengthSeconds );
            props[ "channels" ] = CORE::CVariant( asset.audio.channels );
            props[ "sample_rate" ] = CORE::CVariant( asset.audio.sampleRate );
            rows[ "asset_audio_props" ].push_back( props );
        }
        for ( size_t f=0; f<asset.files.size(); ++f )
        {
            CAssetCatalogue::TRow file;
            file[ "asset_id" ] = assetId;
            file[ "path" ] = CORE::CVariant( asset.files[ f ].path );
            file[ "pack_id" ] = packId;
            file[ "size_bytes" ] = CORE::CVariant( asset.files[ f ].sizeBytes );
            file[ "digest" ] = CORE::CVariant( asset.files[ f ].digest );
            file[ "role" ] = CORE::CVariant( asset.files[ f ].role );
            rows[ "asset_files" ].push_back( file );
        }
        for ( size_t k=0; k<asset.keywords.size(); ++k )
        {
            const CORE::CString key = asset.keywords[ k ].keyword + "\n" + asset.assetId;
            GUCEF::map< CORE::CString, CAssetCatalogue::TRow >::iterator existing = keywordRows.find( key );
            if ( existing != keywordRows.end() && CAssetCatalogue::GetInt( (*existing).second, "weight" ) >= asset.keywords[ k ].weight )
                continue;
            CAssetCatalogue::TRow keyword;
            keyword[ "keyword" ] = CORE::CVariant( asset.keywords[ k ].keyword );
            keyword[ "object_id" ] = assetId;
            keyword[ "object_type" ] = CORE::CVariant( CORE::CString( "asset" ) );
            keyword[ "pack_id" ] = packId;
            keyword[ "field" ] = CORE::CVariant( asset.keywords[ k ].field );
            keyword[ "weight" ] = CORE::CVariant( asset.keywords[ k ].weight );
            keyword[ "original" ] = CORE::CVariant( asset.keywords[ k ].original );
            keywordRows[ key ] = keyword;
        }
    }
    for ( size_t r=0; r<job.relations.size(); ++r )
    {
        CAssetCatalogue::TRow relation;
        relation[ "from_asset" ] = CORE::CVariant( job.relations[ r ].fromAssetId );
        relation[ "kind" ] = CORE::CVariant( job.relations[ r ].kind );
        relation[ "to_asset" ] = CORE::CVariant( job.relations[ r ].toAssetId );
        relation[ "pack_id" ] = packId;
        rows[ "relations" ].push_back( relation );
    }

    // the facets of the pack
    struct SFacet { const char* name; const CORE::CString::StringSet* values; };
    const SFacet facets[] =
    {
        { "theme", &pack.themes }, { "artStyle", &pack.artStyles }, { "pbrRole", &pack.pbrRoles },
        { "licenceRestriction", &pack.licence.restrictions }, { "format", &formats }
    };
    for ( UInt32 f=0; f<sizeof( facets ) / sizeof( facets[ 0 ] ); ++f )
    {
        for ( CORE::CString::StringSet::const_iterator v=facets[ f ].values->begin(); v!=facets[ f ].values->end(); ++v )
        {
            CAssetCatalogue::TRow facet;
            facet[ "object_id" ] = packId;
            facet[ "facet" ] = CORE::CVariant( CORE::CString( facets[ f ].name ) );
            facet[ "value" ] = CORE::CVariant( (*v) );
            facet[ "object_type" ] = CORE::CVariant( CORE::CString( "pack" ) );
            facet[ "pack_id" ] = packId;
            rows[ "facets" ].push_back( facet );
        }
    }

    // the keywords of the pack
    for ( size_t k=0; k<pack.keywords.size(); ++k )
    {
        const CORE::CString key = pack.keywords[ k ].keyword + "\n" + pack.packId;
        GUCEF::map< CORE::CString, CAssetCatalogue::TRow >::iterator existing = keywordRows.find( key );
        if ( existing != keywordRows.end() && CAssetCatalogue::GetInt( (*existing).second, "weight" ) >= pack.keywords[ k ].weight )
            continue;
        CAssetCatalogue::TRow keyword;
        keyword[ "keyword" ] = CORE::CVariant( pack.keywords[ k ].keyword );
        keyword[ "object_id" ] = packId;
        keyword[ "object_type" ] = CORE::CVariant( CORE::CString( "pack" ) );
        keyword[ "pack_id" ] = packId;
        keyword[ "field" ] = CORE::CVariant( pack.keywords[ k ].field );
        keyword[ "weight" ] = CORE::CVariant( pack.keywords[ k ].weight );
        keyword[ "original" ] = CORE::CVariant( pack.keywords[ k ].original );
        keywordRows[ key ] = keyword;
    }
    for ( GUCEF::map< CORE::CString, CAssetCatalogue::TRow >::const_iterator k=keywordRows.begin(); k!=keywordRows.end(); ++k )
        rows[ "keywords" ].push_back( (*k).second );

    // what the conversion did
    if ( pack.conversion.IsSet() )
    {
        CAssetCatalogue::TRow conversion;
        CAssetCatalogue::MakeConversionRow( pack.packId, pack.conversion, conversion );
        rows[ "pack_conversions" ].push_back( conversion );
    }

    // the licence
    CAssetCatalogue::TRow licence;
    licence[ "licence_id" ] = CORE::CVariant( pack.licence.licenceId.IsNULLOrEmpty() ? pack.licence.MakeLicenceId() : pack.licence.licenceId );
    licence[ "licence_class" ] = CORE::CVariant( pack.licence.licenceClass );
    licence[ "name" ] = CORE::CVariant( pack.licence.name );
    licence[ "url" ] = CORE::CVariant( pack.licence.url );
    licence[ "text" ] = CORE::CVariant( pack.licence.text );
    licence[ "restrictions" ] = CORE::CVariant( CAssetCatalogue::JoinSet( pack.licence.restrictions ) );
    rows[ "licences" ].push_back( licence );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::WritePackRow( const CAssetIngestJob& job, TAssetPackState state, const CORE::CString& reason )
{GUCEF_TRACE;

    const CAssetPackRecord& pack = job.pack;
    CORE::CString::StringSet formats;
    for ( size_t a=0; a<job.assets.size(); ++a )
        if ( IsHeldFormatKind( job.assets[ a ].kind ) || job.assets[ a ].sourceFormat == "unreal" || job.assets[ a ].sourceFormat == "unity" )
            formats.insert( job.assets[ a ].sourceFormat );

    CAssetCatalogue::TRowVector rows( 1 );
    CAssetCatalogue::TRow& row = rows[ 0 ];
    row[ "pack_id" ] = CORE::CVariant( pack.packId );
    row[ "source_id" ] = CORE::CVariant( pack.sourceId );
    row[ "source_item_id" ] = CORE::CVariant( pack.sourceItemId );
    row[ "source_version" ] = CORE::CVariant( pack.sourceVersion );
    row[ "source_url" ] = CORE::CVariant( pack.sourceUrl );
    row[ "title" ] = CORE::CVariant( pack.title );
    row[ "seller" ] = CORE::CVariant( pack.seller );
    row[ "description" ] = CORE::CVariant( pack.description );
    row[ "source_categories" ] = CORE::CVariant( CAssetCatalogue::JoinLines( pack.sourceCategories ) );
    row[ "source_tags" ] = CORE::CVariant( CAssetCatalogue::JoinLines( pack.sourceTags ) );
    row[ "categories" ] = CORE::CVariant( CAssetCatalogue::JoinSet( pack.categories ) );
    row[ "unmapped_categories" ] = CORE::CVariant( CAssetCatalogue::JoinSet( pack.unmappedCategories ) );
    row[ "kinds" ] = CORE::CVariant( CAssetCatalogue::JoinSet( pack.kinds ) );
    row[ "themes" ] = CORE::CVariant( CAssetCatalogue::JoinSet( pack.themes ) );
    row[ "art_styles" ] = CORE::CVariant( CAssetCatalogue::JoinSet( pack.artStyles ) );
    row[ "pbr_roles" ] = CORE::CVariant( CAssetCatalogue::JoinSet( pack.pbrRoles ) );
    row[ "licence_id" ] = CORE::CVariant( pack.licence.licenceId );
    row[ "licence_class" ] = CORE::CVariant( pack.licence.licenceClass );
    row[ "licence_restrictions" ] = CORE::CVariant( CAssetCatalogue::JoinSet( pack.licence.restrictions ) );
    row[ "formats_original" ] = CORE::CVariant( CAssetCatalogue::JoinSet( formats ) );
    row[ "formats_converted" ] = CORE::CVariant( CAssetCatalogue::JoinSet( pack.formatsConverted ) );
    row[ "keep" ] = CORE::CVariant( CORE::CString( AssetKeepPolicyToString( pack.keep ) ) );
    row[ "location_id" ] = CORE::CVariant( pack.locationId );
    row[ "root_path" ] = CORE::CVariant( pack.rootPath );
    row[ "size_bytes" ] = CORE::CVariant( pack.sizeBytes );
    row[ "file_count" ] = CORE::CVariant( pack.fileCount );
    row[ "asset_count" ] = CORE::CVariant( (Int64) job.assets.size() );
    row[ "ingested_ms" ] = CORE::CVariant( pack.ingestedAtMs );
    row[ "normalizer" ] = CORE::CVariant( pack.normalizer );
    row[ "analyzers" ] = CORE::CVariant( CAssetCatalogue::JoinSet( pack.analyzers ) );
    row[ "state" ] = CORE::CVariant( CORE::CString( AssetPackStateToString( state ) ) );
    row[ "state_reason" ] = CORE::CVariant( reason );
    return m_store.GetCatalogue().Write( "packs", rows );
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::Index( CAssetIngestJob& job, CORE::CString& message )
{GUCEF_TRACE;

    if ( ASSETPACK_STATE_CONVERSION_PENDING != job.pack.state && ASSETPACK_STATE_CONVERSION_REFUSED != job.pack.state )
        job.pack.state = ASSETPACK_STATE_INGESTED;
    job.pack.ingestedAtMs = NowUtcMs();
    job.pack.assetCount = (Int64) job.assets.size();

    // a keep policy of "converted" lets the originals of the assets that converted go, once the conversion ended without
    // errors: they are not in the rows, and their files go after the rows are written (the job keeps them until then)
    CAssetIngestJob rowsJob( job );
    CORE::CString::StringVector removals;
    if ( ASSETKEEP_CONVERTED == job.pack.keep && CAssetConversionRecord::OutcomeConverted == job.pack.conversion.outcome && !job.inPlace )
    {
        const CPackStorageLayout layout( job.pack.rootPath );
        const CORE::CString locationRoot = m_store.GetLocationRoot( job.locationId );
        for ( size_t a=0; a<job.assets.size(); ++a )
        {
            if ( job.assets[ a ].convertedPath.IsNULLOrEmpty() || job.assets[ a ].files.empty() || IsInsideArchive( job, job.assets[ a ] ) )
                continue;
            for ( size_t f=0; f<job.assets[ a ].files.size(); ++f )
                removals.push_back( layout.Resolve( locationRoot, job.assets[ a ].files[ f ].path ) );
            rowsJob.assets[ a ].files.clear();
        }
    }

    const TAssetStoreStatus status = IndexRows( job, rowsJob, message );
    if ( ASSETSTORE_STATUS_OK == status )
        m_store.ReleasePackFolder( job.pack.packId );
    if ( ASSETSTORE_STATUS_OK == status )
    {
        // the pack index: the document that the conversion wrote is loaded (work once); nothing is built here, the job index-pack
        // builds what is missing; a pack without an index is still an ingested pack
        CAssetPackIndexOptions indexOptions;
        indexOptions.notify = false;
        indexOptions.allowBuild = false;
        CAssetPackIndexOutcome indexOutcome;
        TAssetStoreStatus indexStatus = m_store.RunIndexPack( job.pack.packId, indexOptions, indexOutcome, m_cancel );
        if ( ASSETSTORE_STATUS_UNSUPPORTED == indexStatus && m_store.GetSettings().indexAfterIngest )
        {
            // the conversion wrote no document (the pack was not converted, or the converter makes none): the converter hook builds
            // the index now, as the job index-pack would; without a hook the pack stays without one
            indexOptions.allowBuild = true;
            indexOptions.notify = true;
            indexStatus = m_store.RunIndexPack( job.pack.packId, indexOptions, indexOutcome, m_cancel );
            if ( ASSETSTORE_STATUS_OK == indexStatus )
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: Ingest " << job.ingestId << " made the pack index of pack " << job.pack.packId << ": " << indexOutcome.resourceCount << " resource(s)" );
        }
        if ( ASSETSTORE_STATUS_OK != indexStatus && ASSETSTORE_STATUS_UNSUPPORTED != indexStatus )
            job.problems.push_back( "the pack index could not be made: " + indexOutcome.message );
    }
    if ( ASSETSTORE_STATUS_OK == status && !removals.empty() )
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        UnmountArchives();
        for ( size_t r=0; r<removals.size(); ++r )
            vfs.DeleteFile( removals[ r ], true );
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: Ingest " << job.ingestId << " removed the originals of " << (UInt32) removals.size() << " converted file(s): keep is \"converted\"" );
        MountArchives( job );
    }
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::IndexRows( CAssetIngestJob& job, const CAssetIngestJob& rowsJob, CORE::CString& message )
{GUCEF_TRACE;

    CAssetCatalogue& catalogue = m_store.GetCatalogue();

    GUCEF::map< CORE::CString, CAssetCatalogue::TRowVector > rows;
    BuildRows( rowsJob, rows );
    CORE::CString::StringVector childTables;
    CAssetCatalogue::GetPackChildTables( childTables );

    // one ingest indexes at a time: a client has one transaction
    MT::CScopeMutex indexLock( m_store.GetIndexLock() );
    const bool transactional = catalogue.HasTransactions();
    COMCORE::CIDatabaseClientPtr client = catalogue.GetClient();
    if ( transactional )
    {
        COMCORE::CDatabaseTransaction transaction( client );
        if ( !transaction.IsActive() )
        {
            message = "the transaction of the index cannot begin";
            return ASSETSTORE_STATUS_DATABASE_ERROR;
        }
        for ( size_t t=0; t<childTables.size(); ++t )
        {
            TAssetStoreStatus status = catalogue.DeleteWhereEquals( childTables[ t ], "pack_id", CORE::CVariant( job.pack.packId ) );
            if ( ASSETSTORE_STATUS_OK == status )
                status = catalogue.Write( childTables[ t ], rows[ childTables[ t ] ] );
            if ( ASSETSTORE_STATUS_OK != status )
            {
                message = "writing " + childTables[ t ] + " failed: " + catalogue.GetLastErrorMessage();
                return status;
            }
        }
        TAssetStoreStatus status = catalogue.Write( "licences", rows[ "licences" ] );
        if ( ASSETSTORE_STATUS_OK == status )
            status = WritePackRow( job, job.pack.state, job.pack.stateReason );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            message = "writing the pack failed: " + catalogue.GetLastErrorMessage();
            return status;
        }
        if ( CORE::DATATABLE_STATUS_OK != transaction.Commit() )
        {
            message = "the transaction of the index was not committed: " + client->GetLastErrorMessage();
            return ASSETSTORE_STATUS_DATABASE_ERROR;
        }
        return ASSETSTORE_STATUS_OK;
    }

    // without transactions: the pack row says "indexing" until the rows that refer to it are there
    TAssetStoreStatus status = WritePackRow( job, ASSETPACK_STATE_INDEXING, CORE::CString::Empty );
    for ( size_t t=0; ASSETSTORE_STATUS_OK == status && t<childTables.size(); ++t )
    {
        status = catalogue.DeleteWhereEquals( childTables[ t ], "pack_id", CORE::CVariant( job.pack.packId ) );
        if ( ASSETSTORE_STATUS_OK == status )
            status = catalogue.Write( childTables[ t ], rows[ childTables[ t ] ] );
        if ( ASSETSTORE_STATUS_OK != status )
            message = "writing " + childTables[ t ] + " failed: " + catalogue.GetLastErrorMessage();
    }
    if ( ASSETSTORE_STATUS_OK == status )
        status = catalogue.Write( "licences", rows[ "licences" ] );
    if ( ASSETSTORE_STATUS_OK == status )
        status = WritePackRow( job, job.pack.state, job.pack.stateReason );
    if ( ASSETSTORE_STATUS_OK != status && message.IsNULLOrEmpty() )
        message = "writing the pack failed: " + catalogue.GetLastErrorMessage();
    if ( ASSETSTORE_STATUS_OK != status )
        m_store.RemovePackRows( job.pack.packId );
    return status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetIngestPipeline::Record( CAssetIngestJob& job, const CORE::CString& outcome, TAssetIngestStep step, CORE::CString& message )
{GUCEF_TRACE;

    CAssetCatalogue::TRowVector rows( 1 );
    CAssetCatalogue::TRow& row = rows[ 0 ];
    row[ "entry_id" ] = CORE::CVariant( MakeId( "ih" ) );
    row[ "ingest_id" ] = CORE::CVariant( job.ingestId );
    row[ "pack_id" ] = CORE::CVariant( job.pack.packId );
    row[ "source_id" ] = CORE::CVariant( job.pack.sourceId );
    row[ "origin" ] = CORE::CVariant( job.request.downloadJobId.IsNULLOrEmpty() ? job.request.contentPath : "download " + job.request.downloadJobId );
    row[ "policy" ] = CORE::CVariant( "keep=" + CORE::CString( AssetKeepPolicyToString( job.pack.keep ) ) + ";location=" + job.locationId + ( job.inPlace ? ";inPlace" : "" ) );
    row[ "started_ms" ] = CORE::CVariant( job.startedAtMs );
    row[ "duration_ms" ] = CORE::CVariant( NowUtcMs() - job.startedAtMs );
    row[ "outcome" ] = CORE::CVariant( outcome );
    row[ "step" ] = CORE::CVariant( CORE::CString( AssetIngestStepToString( step ) ) );
    row[ "problems" ] = CORE::CVariant( CAssetCatalogue::JoinLines( job.problems ) );
    const TAssetStoreStatus status = m_store.GetCatalogue().Write( "ingest_history", rows );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        message = "the ingest history cannot be written: " + m_store.GetCatalogue().GetLastErrorMessage();
        return status;
    }

    if ( outcome == "ingested" )
        RecordSourceFacts( job );

    // a finished ingest leaves nothing in the staging area
    if ( outcome != "failed" )
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        if ( vfs.DirExists( job.stagingFolder ) )
            vfs.DeleteDir( job.stagingFolder, true );
        if ( outcome == "ingested" && job.request.sourceId != CPrimaryAssetStoreSettings::DiskSourceId && vfs.DirExists( job.request.contentPath ) )
        {
            CORE::CString::StringVector left;
            ListFiles( job.request.contentPath, left );
            if ( left.empty() )
                vfs.DeleteDir( job.request.contentPath, true );
        }
    }
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CAssetIngestPipeline::RecordSourceFacts( CAssetIngestJob& job )
{GUCEF_TRACE;

    const CAssetStoreItem& item = job.request.item;
    CAssetSyncMarkStore& syncStore = m_store.GetSyncStore();
    if ( job.pack.sourceId == CPrimaryAssetStoreSettings::DiskSourceId || item.itemId.IsNULLOrEmpty() || !syncStore.IsAvailable() )
        return;

    CAssetPackSourceRecord facts;
    facts.packId = job.pack.packId;
    facts.sourceId = job.pack.sourceId.Lowercase();
    facts.sourceItemId = item.itemId;
    facts.sourceFormatId = job.request.formatId;
    facts.revision = job.request.expectedRevision.IsNULLOrEmpty() ? CAssetItemRevision::GetEffectiveRevision( item ) : job.request.expectedRevision;
    facts.changedMs = item.changedAtMs > 0 ? item.changedAtMs : CAssetItemRevision::GetChangedMs( facts.revision );
    facts.syncedMs = NowUtcMs();
    facts.syncJobId = job.request.syncJobId;

    // the content: the SHA-256 over the sorted digests of the original files, newline joined
    CORE::CString::StringVector digests;
    for ( size_t a=0; a<job.assets.size(); ++a )
        if ( !job.assets[ a ].digest.IsNULLOrEmpty() )
            digests.push_back( job.assets[ a ].digest.Lowercase() );
    std::sort( digests.begin(), digests.end() );
    CORE::CString joined;
    for ( size_t d=0; d<digests.size(); ++d )
        joined += digests[ d ] + "\n";
    CAssetDigest contentDigest( CAssetDigest::Sha256 );
    contentDigest.Update( joined.C_String(), ByteLengthOf( joined ) );
    facts.contentDigest = contentDigest.FinalHex();

    TAssetStoreStatus status = syncStore.WritePackSource( facts );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        job.problems.push_back( "the store facts of the pack could not be written" );
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: The store facts of pack " << job.pack.packId << " could not be written: " << m_store.GetCatalogue().GetLastErrorMessage() );
        return;
    }
    UInt32 superseded = 0;
    status = syncStore.SupersedeOlderPacks( facts, facts.syncedMs, superseded );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: The older packs of item " << item.itemId << " could not be marked as superseded by pack " << job.pack.packId );
    }
    else
    if ( superseded > 0 )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: Pack " << job.pack.packId << " supersedes " << superseded << " older pack(s) of item " << item.itemId << " of store " << facts.sourceId );
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetIngestPipeline::RepairOriginals( CAssetIngestJob& job, const CORE::CString& packId, CORE::CString::StringVector& repaired )
{GUCEF_TRACE;

    // the original files of the pack that are missing, or whose size or digest is not what the catalogue says, are put back from the
    // verified download: the same content as the catalogue records, so an original is restored and never changed
    repaired.clear();
    CAssetPackRecord pack;
    TAssetRecordVector stored;
    if ( ASSETSTORE_STATUS_OK != m_store.GetPack( packId, pack ) || ASSETSTORE_STATUS_OK != m_store.GetAssets( packId, stored ) )
        return;
    const CPackStorageLayout layout( pack.rootPath );
    const CORE::CString locationRoot = m_store.GetLocationRoot( pack.locationId );
    const CORE::CString prefix = layout.GetOriginalPrefix() + "/";
    GUCEF::map< CORE::CString, const CAssetRecord* > downloaded;
    for ( size_t a=0; a<job.assets.size(); ++a )
        downloaded[ job.assets[ a ].digest.Lowercase() ] = &job.assets[ a ];
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( size_t a=0; a<stored.size(); ++a )
    {
        for ( size_t f=0; f<stored[ a ].files.size(); ++f )
        {
            const CAssetFileRecord& file = stored[ a ].files[ f ];
            if ( file.digest.IsNULLOrEmpty() || 0 != file.path.HasSubstr( prefix, true ) )
                continue;
            const CORE::CString target = layout.Resolve( locationRoot, file.path );
            bool intact = vfs.FileExists( target ) && ( file.sizeBytes < 0 || (Int64) vfs.GetFileSize( target ) == file.sizeBytes );
            if ( intact )
            {
                CORE::CString digest;
                intact = CAssetDigest::ComputeVfsFileDigest( target, CAssetDigest::Sha256, digest ) && digest.Lowercase() == file.digest.Lowercase();
            }
            if ( intact )
                continue;
            GUCEF::map< CORE::CString, const CAssetRecord* >::const_iterator source = downloaded.find( file.digest.Lowercase() );
            if ( source == downloaded.end() || (*source).second->files.empty() )
            {
                job.problems.push_back( "the original file " + file.path + " is damaged and the download has no file with its digest" );
                continue;
            }
            // a file of a staged archive cannot be put back on its own
            const CORE::CString from = JoinVfs( job.contentRoot, (*source).second->files[ 0 ].path );
            if ( !vfs.FileExists( from ) )
            {
                job.problems.push_back( "the original file " + file.path + " is damaged; its copy in the download is inside an archive and is not put back on its own" );
                continue;
            }
            const CORE::CString temporary = target + ".repair";
            if ( vfs.CopyFileContent( from, temporary, true ) && vfs.MoveFile( temporary, target, true ) )
            {
                repaired.push_back( file.path );
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetIngestPipeline: The original file " << file.path << " of pack " << packId << " was put back from a verified download" );
            }
            else
            {
                vfs.DeleteFile( temporary, true );
                job.problems.push_back( "the original file " + file.path + " could not be put back" );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CAssetIngestPipeline::RevalidateDuplicate( CAssetIngestJob& job, const CORE::CString& duplicateOfPackId )
{GUCEF_TRACE;

    const CAssetStoreItem& item = job.request.item;
    CAssetSyncMarkStore& syncStore = m_store.GetSyncStore();
    if ( job.request.expectedRevision.IsNULLOrEmpty() || duplicateOfPackId.IsNULLOrEmpty() || item.itemId.IsNULLOrEmpty() || !syncStore.IsAvailable() )
        return;
    CAssetPackSourceRecord facts;
    const TAssetStoreStatus found = syncStore.GetPackSource( duplicateOfPackId, facts );
    if ( ASSETSTORE_STATUS_OK == found )
    {
        // only a pack of the same item stands for the item; the same files under another item stay that item's
        if ( facts.sourceId != job.pack.sourceId.Lowercase() || facts.sourceItemId != item.itemId )
            return;
        if ( !facts.MatchesFormat( job.request.formatId ) )
            return;
        const Int64 changedMs = item.changedAtMs > 0 ? item.changedAtMs : CAssetItemRevision::GetChangedMs( job.request.expectedRevision );
        m_store.ConfirmSourceRevision( duplicateOfPackId, job.request.expectedRevision, changedMs );
        return;
    }
    if ( ASSETSTORE_STATUS_NOT_FOUND != found )
        return;

    // a pack from before the facts were kept: when it is a pack of the item it gets its facts now (adoption)
    CAssetPackRecord pack;
    if ( ASSETSTORE_STATUS_OK != m_store.GetPack( duplicateOfPackId, pack ) || pack.sourceId.Lowercase() != job.pack.sourceId.Lowercase() || pack.sourceItemId != item.itemId )
        return;
    facts.packId = duplicateOfPackId;
    facts.sourceId = job.pack.sourceId.Lowercase();
    facts.sourceItemId = item.itemId;
    facts.sourceFormatId = job.request.formatId;
    facts.revision = job.request.expectedRevision;
    facts.changedMs = item.changedAtMs;
    facts.syncedMs = NowUtcMs();
    facts.syncJobId = job.request.syncJobId;
    syncStore.WritePackSource( facts );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
