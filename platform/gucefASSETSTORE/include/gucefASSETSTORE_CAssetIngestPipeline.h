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

#ifndef GUCEF_ASSETSTORE_CASSETINGESTPIPELINE_H
#define GUCEF_ASSETSTORE_CASSETINGESTPIPELINE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTJOB_H
#include "gucefASSETSTORE_CAssetIngestJob.h"
#define GUCEF_ASSETSTORE_CASSETINGESTJOB_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTJOB_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETINGESTRESULT_H
#include "gucefASSETSTORE_CAssetIngestResult.h"
#define GUCEF_ASSETSTORE_CASSETINGESTRESULT_H
#endif /* GUCEF_ASSETSTORE_CASSETINGESTRESULT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#include "gucefASSETSTORE_CAssetCatalogue.h"
#define GUCEF_ASSETSTORE_CASSETCATALOGUE_H
#endif /* GUCEF_ASSETSTORE_CASSETCATALOGUE_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CPrimaryAssetStore;

/**
 *  The ingest pipeline of the primary store (section 8), run on the calling thread:
 *    1. stage: a download is in the staging area of its location already; a pack from disk is copied into
 *       "<location>/staging/<ingest>/content" or referenced in place (then it has to be inside a location)
 *    2. identify: every file with its SHA-256 and size; the content of archives the VFS can mount is taken in; a
 *       pack whose source item, or all of whose file digests, the catalogue has already is a duplicate (skipped or
 *       kept by the duplicate policy)
 *    3. normalise: the metadata normaliser of the store (or the generic one, or the one for disk) for the pack; for
 *       every asset its keywords and, by the category rules on its name, its category
 *    4. analyse: the content analysers, as bounded pool tasks; resumable per asset
 *    5. convert: the converter hook when the keep policy asks for converted content, else "conversion pending". A
 *       pack the converter refuses (its licence record does not allow it) stays as original with the state
 *       "conversion refused", one whose conversion ends with errors as original with the state "conversion pending";
 *       neither is a failure of the ingest. A cancelled conversion, or one that cannot run, fails the step: the pack
 *       stays staged and Resume() converts it again
 *    6. place: into "<location>/original/<pack folder>/" (and "intermediate/" and "gucef-ogre3d/", CPackStorageLayout)
 *    7. index: the rows of the pack in one transaction where the backend has them, else pack row first ("indexing"),
 *       then the rows that refer to it, then the pack row again
 *    8. record: the ingest history; the staging folder of the ingest goes
 *  The state of the ingest is saved as its manifest after every step; a failed step leaves the pack staged (state
 *  failed with the reason in the catalogue) and Resume() continues from that step.
 *
 *  An ingest that runs as a pool task of the store gives the source of its future: a request to cancel the future is
 *  seen between the steps and by the conversion, which then ends the step with CANCELLED.
 */
class GUCEF_ASSETSTORE_PRIVATE_CPP CAssetIngestPipeline
{
    public:

    explicit CAssetIngestPipeline( CPrimaryAssetStore& store, const CORE::CFutureResultSource* cancel = GUCEF_NULL );

    TAssetStoreStatus Start( const CAssetIngestRequest& request, CAssetIngestResult& result );

    TAssetStoreStatus Resume( const CORE::CString& ingestId, CAssetIngestResult& result );

    /** The VFS path of the manifest of an ingest that is staged, empty when there is none */
    static CORE::CString FindManifest( const CPrimaryAssetStore& store, const CORE::CString& ingestId );

    private:

    TAssetStoreStatus Run( CAssetIngestJob& job, CAssetIngestResult& result );
    TAssetStoreStatus RunStep( TAssetIngestStep step, CAssetIngestJob& job, CAssetIngestResult& result, CORE::CString& message );

    TAssetStoreStatus Stage( CAssetIngestJob& job, CORE::CString& message );
    TAssetStoreStatus Identify( CAssetIngestJob& job, CAssetIngestResult& result, CORE::CString& message );
    TAssetStoreStatus Normalise( CAssetIngestJob& job, CORE::CString& message );
    TAssetStoreStatus Analyse( CAssetIngestJob& job, CORE::CString& message );
    TAssetStoreStatus Convert( CAssetIngestJob& job, CORE::CString& message );
    TAssetStoreStatus Place( CAssetIngestJob& job, CORE::CString& message );
    TAssetStoreStatus Index( CAssetIngestJob& job, CORE::CString& message );
    TAssetStoreStatus IndexRows( CAssetIngestJob& job, const CAssetIngestJob& rowsJob, CORE::CString& message );
    TAssetStoreStatus Record( CAssetIngestJob& job, const CORE::CString& outcome, TAssetIngestStep step, CORE::CString& message );

    /** The library sync: the store facts of an ingested pack (pack_sources) and the packs of the same item and format it supersedes */
    void RecordSourceFacts( CAssetIngestJob& job );

    /** The library sync: a download whose files duplicate a pack of the same item confirms the revision of that pack (design section 4.6) */
    void RevalidateDuplicate( CAssetIngestJob& job, const CORE::CString& duplicateOfPackId );

    bool IsCancelRequested( void ) const;
    bool IsInsideArchive( const CAssetIngestJob& job, const CAssetRecord& asset ) const;
    bool SaveManifest( const CAssetIngestJob& job ) const;
    CORE::CString ResolveAssetPath( const CAssetIngestJob& job, const CAssetRecord& asset ) const;
    void MountArchives( CAssetIngestJob& job );
    void UnmountArchives( void );
    TAssetStoreStatus FindDuplicate( const CAssetIngestJob& job, CORE::CString& duplicateOfPackId ) const;
    void RepairOriginals( CAssetIngestJob& job, const CORE::CString& packId, CORE::CString::StringVector& repaired );
    TAssetStoreStatus WritePackRow( const CAssetIngestJob& job, TAssetPackState state, const CORE::CString& reason );
    void BuildRows( const CAssetIngestJob& job, GUCEF::map< CORE::CString, CAssetCatalogue::TRowVector >& rows ) const;
    void DeriveFromContent( CAssetIngestJob& job ) const;
    void AddAsset( CAssetIngestJob& job, const CORE::CString& relativePath, const CORE::CString& vfsPath, Int64 size, const CORE::CString& digest ) const;

    public:

    /** The files below a VFS folder, recursively, as paths relative to it */
    static void ListFiles( const CORE::CString& vfsFolder, CORE::CString::StringVector& relativePaths );

    /** Copies the files below a VFS folder to another folder (the folders are made); false when a file could not be copied */
    static bool CopyTree( const CORE::CString& fromFolder, const CORE::CString& toFolder );

    private:

    static CORE::CString MakeId( const CORE::CString& prefix );

    CAssetIngestPipeline( const CAssetIngestPipeline& src );              /**< not implemented */
    CAssetIngestPipeline& operator=( const CAssetIngestPipeline& src );   /**< not implemented */

    private:

    CPrimaryAssetStore& m_store;
    const CORE::CFutureResultSource* m_cancel;      /**< not owned, may be GUCEF_NULL */
    CORE::CString::StringMap m_archiveMounts;       /**< archive relative path to its mount point */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETINGESTPIPELINE_H ? */
