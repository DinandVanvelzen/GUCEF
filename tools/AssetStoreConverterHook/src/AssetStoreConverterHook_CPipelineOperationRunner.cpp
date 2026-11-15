/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#include "AssetStoreConverterHook_CPipelineOperationCall.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEOPERATIONCALL_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H
#include "AssetStoreConverterHook_CPipelineInspectionOperations.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEINSPECTIONOPERATIONS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H
#include "AssetStoreConverterHook_CPipelineLicenceOperations.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINELICENCEOPERATIONS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEANALYSISOPERATIONS_H
#include "AssetStoreConverterHook_CPipelineAnalysisOperations.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEANALYSISOPERATIONS_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEANALYSISOPERATIONS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINERUNOPERATIONS_H
#include "AssetStoreConverterHook_CPipelineRunOperations.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINERUNOPERATIONS_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINERUNOPERATIONS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINEAUTHORINGOPERATIONS_H
#include "AssetStoreConverterHook_CPipelineAuthoringOperations.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINEAUTHORINGOPERATIONS_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINEAUTHORINGOPERATIONS_H ? */

#ifndef ASSETSTORECONVERTERHOOK_CPIPELINERESOURCEOPERATIONS_H
#include "AssetStoreConverterHook_CPipelineResourceOperations.h"
#define ASSETSTORECONVERTERHOOK_CPIPELINERESOURCEOPERATIONS_H
#endif /* ASSETSTORECONVERTERHOOK_CPIPELINERESOURCEOPERATIONS_H ? */

#include "AssetStoreConverterHook_CPipelineOperationRunner.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPipelineOperationRunner::CPipelineOperationRunner( void )
    : m_lock()
    , m_settings()
    , m_plans()
    , m_changes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPipelineOperationRunner::~CPipelineOperationRunner()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CPipelineOperationRunner::SetSettings( const CPipelineOperationSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

CPipelineOperationSettings
CPipelineOperationRunner::GetSettings( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CPipelineOperationRunner::IsSupported( const CORE::CString& operation ) const
{GUCEF_TRACE;

    return ASSETSTORE::CAssetPipelineOperations::IsKnown( operation ) && !ASSETSTORE::CAssetPipelineOperations::IsRender( operation );
}

/*-------------------------------------------------------------------------*/

UInt32
CPipelineOperationRunner::GetPendingChangeCount( void ) const
{GUCEF_TRACE;

    return m_changes.GetCount();
}

/*-------------------------------------------------------------------------*/

ASSETSTORE::TAssetStoreStatus
CPipelineOperationRunner::Run( const ASSETSTORE::CAssetPipelineRequest& request   ,
                               ASSETSTORE::CIAssetConversionObserver* observer    ,
                               ASSETSTORE::CAssetServiceDocument& result          ,
                               const CORE::CString& threadPoolName                ,
                               UInt32 threads                                     )
{GUCEF_TRACE;

    typedef ASSETSTORE::CAssetPipelineOperations TOps;

    result.Clear();
    CPipelineOperationCall call( request, observer, result, GetSettings() );
    if ( !IsSupported( request.operation ) )
        return call.Fail( CPipelineOperationCall::ErrorUnsupported, "the converter has no operation \"" + request.operation + "\"" );
    CORE::CString problem;
    if ( !call.settings.IsValid( problem ) )
        return call.Fail( CPipelineOperationCall::ErrorBackendUnavailable, "the pipeline operations are not configured: " + problem );
    if ( TOps::IsWrite( request.operation ) && call.principal.IsNULLOrEmpty() )
        return call.Fail( CPipelineOperationCall::ErrorInvalidArguments, "a write needs the principal it is made for" );

    try
    {
        const CORE::CString& op = request.operation;
        if ( TOps::PackList == op )            return CPipelineInspectionOperations::PackList( call );
        if ( TOps::PackGet == op )             return CPipelineInspectionOperations::PackGet( call );
        if ( TOps::PackAssets == op )          return CPipelineInspectionOperations::PackAssets( call );
        if ( TOps::AssetReport == op )         return CPipelineInspectionOperations::AssetReport( call );
        if ( TOps::PackActivity == op )        return CPipelineInspectionOperations::PackActivity( call );
        if ( TOps::ExplainReimport == op )     return CPipelineInspectionOperations::ExplainReimport( call );
        if ( TOps::CookOutputs == op )         return CPipelineInspectionOperations::CookOutputs( call );
        if ( TOps::PipelinePlugins == op )     return CPipelineInspectionOperations::PipelinePlugins( call );
        if ( TOps::LicenceCheck == op )        return CPipelineLicenceOperations::LicenceCheck( call );
        if ( TOps::LicenceDraftRecord == op )  return CPipelineLicenceOperations::LicenceDraftRecord( call );
        if ( TOps::ConvertPlan == op )         return CPipelineAnalysisOperations::ConvertPlan( call, m_plans );
        if ( TOps::CompareRoutes == op )       return CPipelineAnalysisOperations::CompareRoutes( call );
        if ( TOps::MaterialMapSuggest == op )  return CPipelineAnalysisOperations::MaterialMapSuggest( call );
        if ( TOps::ManifestPropose == op )     return CPipelineAnalysisOperations::ManifestPropose( call );
        if ( TOps::ConvertPack == op )         return CPipelineRunOperations::ConvertPack( call, threadPoolName, threads );
        if ( TOps::ConvertFile == op )         return CPipelineRunOperations::ConvertFile( call, threadPoolName, threads );
        if ( TOps::CookPack == op )            return CPipelineRunOperations::CookPack( call, threadPoolName, threads );
        if ( TOps::ValidatePack == op )        return CPipelineRunOperations::ValidatePack( call, threadPoolName, threads );
        if ( TOps::ManifestCreate == op )      return CPipelineAuthoringOperations::ManifestCreate( call, m_changes );
        if ( TOps::ManifestEdit == op )        return CPipelineAuthoringOperations::ManifestEdit( call, m_changes );
        if ( TOps::OverrideWrite == op )       return CPipelineAuthoringOperations::OverrideWrite( call, m_changes );
        if ( TOps::OverrideRemove == op )      return CPipelineAuthoringOperations::OverrideRemove( call, m_changes );
        if ( TOps::ReadResource == op )        return CPipelineResourceOperations::ReadResource( call, m_plans );
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CPipelineOperationRunner: An exception ended the operation " << request.operation );
        result.document.Clear();
        result.document.SetName( "result" );
        result.document.SetNodeType( GUCEF_DATATYPE_OBJECT );
        return call.Fail( CPipelineOperationCall::ErrorBackendUnavailable, "the operation " + request.operation + " ended in an exception" );
    }
    return call.Fail( CPipelineOperationCall::ErrorUnsupported, "the converter has no operation \"" + request.operation + "\"" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
