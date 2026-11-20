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

#ifndef GUCEF_ASSETSTORE_CASSETPACKREGENERATION_H
#define GUCEF_ASSETSTORE_CASSETPACKREGENERATION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H
#include "gucefASSETSTORE_CAssetPackConversion.h"
#define GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKCONVERSION_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H
#include "gucefASSETSTORE_CAssetPackGenerations.h"
#define GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONVERSIONREPORTSUMMARY_H
#include "gucefASSETSTORE_CAssetConversionReportSummary.h"
#define GUCEF_ASSETSTORE_CASSETCONVERSIONREPORTSUMMARY_H
#endif /* GUCEF_ASSETSTORE_CASSETCONVERSIONREPORTSUMMARY_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#include "gucefASSETSTORE_CIAssetConversionObserver.h"
#define GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H
#endif /* GUCEF_ASSETSTORE_CIASSETCONVERSIONOBSERVER_H ? */

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

/**
 *  The regeneration of the outputs of a stored pack, on the calling thread: the intermediate resources made again from the
 *  originals (the importers and exporters run with the cache ignored, and the cookers after them unless asked not to), or the
 *  engine target resources made again from the intermediate that is there (a cook only run), for the whole pack or for the
 *  assets that match a list of paths and globs. It is a conversion of the pack (CAssetPackConversion) that writes to a staging
 *  folder of the location ("<location>/staging/regen-<pack id>-<job>") instead of the folders of the pack; when the run ends
 *  without errors (or with errors that the caller accepts) the staged folders are put in place in one swap and the folders they
 *  replace become the previous generation (CAssetPackGenerations), together with the state of the catalogue they go with, and
 *  then the catalogue is written as for any conversion. A run that fails, is refused or is cancelled leaves the folders of the
 *  pack and the catalogue as they were (an entry of the ingest history says what happened). Rollback() exchanges the previous
 *  generation and the current one, files and catalogue state.
 *
 *  The originals are never written: a keep policy of "converted" does not remove originals here. Packs of the earlier storage
 *  layout are refused (they have no generations). The outcome of the hook is compared with the report before the run: Report()
 *  holds the counts before and after and the assets that improved, regressed or still fail, for the caller to verify the fix.
 */
class GUCEF_ASSETSTORE_PRIVATE_CPP CAssetPackRegeneration : public CAssetPackConversion
{
    public:

    enum EMode
    {
        MODE_INTERMEDIATE = 0   ,   /**< the intermediate resources (and the cooked ones after them unless cook is false) */
        MODE_TARGET                 /**< the engine target resources from the intermediate that is there */
    };

    static const UInt32 MaxListedChanges = 50;

    /** 'observer' takes the progress and gives the cancel request of the run (not owned, may be GUCEF_NULL: then 'cancel' is asked) */
    CAssetPackRegeneration( CPrimaryAssetStore& store, const CORE::CFutureResultSource* cancel, CIAssetConversionObserver* observer );

    virtual ~CAssetPackRegeneration() GUCEF_VIRTUAL_OVERRIDE;

    /** The job id and the audit text of the history entries ("caller=...;scope=...") */
    void SetJob( const CORE::CString& jobId, const CORE::CString& origin, const CORE::CString& audit );

    /**
     *  Regenerates the outputs of a pack. 'assetGlobs' limits the run to assets (paths below the original folder of the pack, or
     *  globs; empty: the whole pack); 'cook' (MODE_INTERMEDIATE) also makes the cooked resources again; 'cookTargets' limits the
     *  cookers; 'acceptErrors' puts the outputs of a run with errors in place too (the assets that failed have no output then).
     *  'made' (who, why, when) is kept with the generation the run replaces. The status is that of the outcome; Report() says
     *  what changed.
     */
    TAssetStoreStatus Regenerate( const CORE::CString& packId, EMode mode, const CORE::CString::StringVector& assetGlobs, bool cook,
                                  const CORE::CString::StringVector& cookTargets, bool acceptErrors, const CORE::CDataNode& made,
                                  CAssetConversionOutcome& outcome );

    /** Exchanges the previous generation of the outputs of the pack and the current one; NOT_FOUND when there is no previous generation */
    TAssetStoreStatus Rollback( const CORE::CString& packId, const CORE::CDataNode& made, CAssetConversionOutcome& outcome );

    /**
     *  What the last Regenerate() or Rollback() did: "before" and "after" (the counts of the reports), "changes" (see
     *  CAssetConversionReportSummary::Compare()), "replaced" (whether the outputs were put in place), "areas", "reportPath" (the VFS
     *  path of the report of the pack now), "previousGeneration" (the VFS folder of the kept generation, empty when none)
     */
    const CORE::CDataNode& GetReport( void ) const;

    /** Whether the pack can be regenerated in the mode, and why not; 'intermediateBytes' and 'cookedBytes' are the sizes of its outputs now */
    static TAssetStoreStatus Check( CPrimaryAssetStore& store, const CORE::CString& packId, EMode mode, CORE::CString& problem,
                                    UInt64& intermediateBytes, UInt64& cookedBytes, bool& hasPrevious );

    /** The size of the files below a VFS folder */
    static UInt64 GetFolderBytes( const CORE::CString& vfsFolder );

    protected:

    virtual TAssetStoreStatus OnPrepare( const CAssetPackRecord& pack, const TAssetRecordVector& assets, const CORE::CString& locationRoot,
                                         CAssetConversionRequest& request, CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus OnConverted( const CAssetConversionRequest& request, TAssetStoreStatus& hookStatus, CAssetConversionOutcome& hookOutcome,
                                           const TAssetRecordVector& assets, TAssetRecordVector& converted, CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE;
    virtual TAssetStoreStatus OnCommit( const CAssetConversionRequest& request, TAssetStoreStatus hookStatus, const CAssetPackRecord& before,
                                        const TAssetRecordVector& beforeAssets, CORE::CString& message ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnCommitted( bool persisted ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    void Reset( void );
    void FinishReport( void );
    void DiscardStaging( void );

    CAssetPackRegeneration( const CAssetPackRegeneration& src );              /**< not implemented */
    CAssetPackRegeneration& operator=( const CAssetPackRegeneration& src );   /**< not implemented */

    EMode m_mode;
    CORE::CString::StringVector m_globs;
    bool m_cook;
    CORE::CString::StringVector m_targets;
    bool m_acceptErrors;
    CORE::CDataNode m_made;
    CAssetPackGenerations* m_generations;
    CORE::CString m_stagingRoot;
    CORE::CString::StringVector m_areas;
    CORE::CString m_currentReport;
    CORE::CString m_stagedReport;
    CAssetPackRecord m_pack;
    CAssetConversionReportSummary m_before;
    CAssetConversionReportSummary m_after;
    bool m_ran;
    bool m_replaced;
    bool m_committed;
    CORE::CDataNode m_report;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPACKREGENERATION_H ? */
