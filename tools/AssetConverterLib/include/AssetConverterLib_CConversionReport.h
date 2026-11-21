/*
 *  AssetConverterLib: the conversion report of a pack
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

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORTENTRY_H
#include "AssetConverterLib_CConversionReportEntry.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORTENTRY_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORTENTRY_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {
class CTask;
}; /* namespace CORE */
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The report of a conversion: per asset what was converted, approximated (with what and why) and skipped (with
 *  why), the results of the validators and the files written. It carries the pack and its licence record, and a
 *  refusal by the licence check. It is written as JSON (SaveJson(), read back with LoadJson()) and as a readable
 *  summary (ToSummaryText()).
 *
 *  The plugins write to the current asset: the pipeline selects it (SelectAsset()) before it calls them, and a cooker
 *  that works on several assets selects each in turn. A report is not thread-safe: the pipeline gives every task its
 *  own and merges them (Merge()) on the calling thread.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CConversionReport
{
    public:

    typedef GUCEF::vector< CConversionReportEntry > TEntryVector;

    CConversionReport( void );

    void SetPackId( const CORE::CString& packId );
    const CORE::CString& GetPackId( void ) const;

    void SetLicenceRecordId( const CORE::CString& recordId );
    const CORE::CString& GetLicenceRecordId( void ) const;

    /** The pack was refused by its licence record: nothing was converted */
    void SetRefused( const CORE::CString& reason );
    bool IsRefused( void ) const;
    const CORE::CString& GetRefusalReason( void ) const;

    /** The run was asked to stop (CIAssetPipelineObserver) before it was done: what is in the report is what finished */
    void SetCancelled( void );
    bool IsCancelled( void ) const;

    /**
     *  The task that converts the asset this report belongs to, for the importers and the readers they use: they poll
     *  IsCancellationRequested() at points where stopping is cheap (a child process, between payload blocks). The task must outlive
     *  the conversion; the task consumer sets it before the conversion and clears it afterwards. NULL for a conversion without a task
     */
    void SetCancellationSource( const CORE::CTask* task );
    bool IsCancellationRequested( void ) const;

    /** The check that tells the conversion to stop, for a caller that has no CORE::CTask (a job that is cancelled through an activity): a function with its context, polled at the same points */
    typedef bool (*TCancellationCheck)( void* context );
    void SetCancellationCheck( TCancellationCheck check, void* context );

    /** Where a long import tells how far it is, 0 to 1: a function with its context; the importers call ReportProgress() between the steps of a big file */
    typedef void (*TProgressSink)( void* context, Float32 fraction );
    void SetProgressSink( TProgressSink sink, void* context );
    void ReportProgress( Float32 fraction ) const;

    /** The intermediate of the pack was locked by another run (CIntermediateLock): nothing was converted; who holds it */
    void SetLocked( const CORE::CString& holder );
    bool IsLocked( void ) const;
    const CORE::CString& GetLockHolder( void ) const;

    /** The mode of the run (written when it is not full) */
    void SetMode( TPipelineMode mode );
    TPipelineMode GetMode( void ) const;

    /** The asset globs the run was restricted to (written when there are any): the report covers those assets only */
    void SetAssetFilter( const CORE::CString::StringVector& assetGlobs );
    const CORE::CString::StringVector& GetAssetFilter( void ) const;

    /** Makes the asset the current one (an entry is made for it when there is none); its entry */
    CConversionReportEntry& SelectAsset( const CORE::CString& assetPath );

    /** The current entry; NULL when no asset was selected */
    CConversionReportEntry* GetCurrentEntry( void );

    const CConversionReportEntry* FindEntry( const CORE::CString& assetPath ) const;
    CConversionReportEntry* FindEntry( const CORE::CString& assetPath );

    const TEntryVector& GetEntries( void ) const;

    /** Findings about the current asset; without one they go to the entry of the pack (asset path "") */
    void AddItem( TReportItemKind kind, const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message );
    void AddInfo( const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message );
    void AddApproximation( const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message );
    void AddSkipped( const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message );
    void AddWarning( const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message );
    void AddError( const CORE::CString& source, const CORE::CString& subject, const CORE::CString& message );

    /** A file written for the current asset: VFS paths; 'profile' is empty for a file of the intermediate */
    void AddOutputFile( const CORE::CString& path, const CORE::CString::StringVector& references, const CORE::CString& profile = CORE::CString::Empty );

    /** Takes the entries of the other report: an entry of an asset that is here already is replaced */
    void Merge( const CConversionReport& other );

    /** The number of assets with the status */
    UInt32 GetAssetCount( TAssetStatus status ) const;

    /** The number of findings of the kind over every asset */
    UInt32 GetItemCount( TReportItemKind kind ) const;

    /** Whether any asset failed, the pack was refused, the intermediate was locked or the run was cancelled */
    bool HasFailures( void ) const;

    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );

    bool SaveJson( const CORE::CString& vfsPath ) const;
    bool LoadJson( const CORE::CString& vfsPath );

    /** A readable summary: the pack, the counts per status and every finding that is not an info */
    CORE::CString ToSummaryText( void ) const;

    bool SaveSummary( const CORE::CString& vfsPath ) const;

    void Clear( void );

    private:

    CORE::CString m_packId;
    CORE::CString m_licenceRecordId;
    CORE::CString m_refusalReason;
    bool m_refused;
    bool m_cancelled;
    const CORE::CTask* m_cancellationSource;
    TCancellationCheck m_cancellationCheck;
    void* m_cancellationContext;
    TProgressSink m_progressSink;
    void* m_progressContext;
    bool m_locked;
    CORE::CString m_lockHolder;
    TPipelineMode m_mode;
    CORE::CString::StringVector m_assetFilter;
    TEntryVector m_entries;
    Int32 m_current;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */
