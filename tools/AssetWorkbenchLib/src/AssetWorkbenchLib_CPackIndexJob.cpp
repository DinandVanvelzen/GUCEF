/*
 *  AssetWorkbenchLib: builds or reads the pack index of a pack in the background for the resource view
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

#ifndef ASSETWORKBENCHLIB_CASSETLOADCONTROL_H
#include "AssetWorkbenchLib_CAssetLoadControl.h"
#define ASSETWORKBENCHLIB_CASSETLOADCONTROL_H
#endif /* ASSETWORKBENCHLIB_CASSETLOADCONTROL_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSHELL_H
#include "AssetWorkbenchLib_CWorkbenchShell.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSHELL_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSHELL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#include "AssetConverterLib_CAssetConverterGlobal.h"
#define ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H
#endif /* ASSETCONVERTERLIB_CASSETCONVERTERGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#include "AssetConverterLib_CAssetPluginRegistry.h"
#define ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H
#endif /* ASSETCONVERTERLIB_CASSETPLUGINREGISTRY_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKLISTING_H
#include "AssetConverterLib_CAssetPackListing.h"
#define ASSETCONVERTERLIB_CASSETPACKLISTING_H
#endif /* ASSETCONVERTERLIB_CASSETPACKLISTING_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXUPDATER_H
#include "AssetConverterLib_CPackIndexUpdater.h"
#define ASSETCONVERTERLIB_CPACKINDEXUPDATER_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXUPDATER_H ? */

#ifndef ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H
#include "AssetConverterLib_CPackIndexLocator.h"
#define ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXLOCATOR_H ? */

#include "AssetWorkbenchLib_CPackIndexJob.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPackIndexJob::CPackIndexJob( CWorkbenchShell& shell, const CORE::CString& vfsFolder, const ASSETCONVERTER::CPackRedoRequest& redo )
    : CIAssetLoadJob()
    , ASSETCONVERTER::CIPackIndexObserver()
    , m_shell( shell )
    , m_folder( vfsFolder )
    , m_redo( redo )
    , m_document()
    , m_result()
    , m_control( GUCEF_NULL )
    , m_isPack( false )
    , m_ok( false )
    , m_problem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackIndexJob::~CPackIndexJob()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool CPackIndexJob::ReportsProgress( void ) const { return true; }
const CORE::CString& CPackIndexJob::GetFolder( void ) const { return m_folder; }
const ASSETCONVERTER::CPackIndexDocument& CPackIndexJob::GetDocument( void ) const { return m_document; }
const ASSETCONVERTER::CPackIndexRunResult& CPackIndexJob::GetResult( void ) const { return m_result; }
bool CPackIndexJob::IsPack( void ) const { return m_isPack; }
CORE::CString CPackIndexJob::GetProblem( void ) const { return m_problem; }

/*-------------------------------------------------------------------------*/

void
CPackIndexJob::OnProgress( const CORE::CString& phase, UInt32 done, UInt32 total )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_control && total > 0 )
        m_control->SetProgress( (Float32) done / (Float32) total );
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexJob::IsCancelRequested( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_control && m_control->IsCancelled();
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexJob::Prepare( CAssetLoadControl& control )
{GUCEF_TRACE;

    m_control = &control;
    ASSETCONVERTER::CAssetPluginRegistry& registry = ASSETCONVERTER::CAssetConverterGlobal::Instance()->GetPluginRegistry();

    // the pack as it is listed, and what the recognisers make of it
    ASSETCONVERTER::CAssetPackListing listing;
    if ( !listing.LoadFromVfs( m_folder, 200000 ) )
    {
        m_problem = "the folder " + m_folder + " cannot be listed";
        m_control = GUCEF_NULL;
        return false;
    }
    ASSETCONVERTER::CAssetPackProfile profile;
    m_isPack = registry.RecognisePack( listing, profile ) && profile.IsRecognised();
    if ( !m_isPack )
    {
        m_control = GUCEF_NULL;
        return true;
    }

    ASSETCONVERTER::CPackIndexBuildOptions buildOptions;
    buildOptions.packName = ASSETCONVERTER::CVfsPaths::NameOf( m_folder );
    buildOptions.toolVersion = CORE::UInt32ToString( ASSETCONVERTER::CAssetConverterGlobal::GetConverterVersion() );
    buildOptions.outputRoot = "intermediate";
    buildOptions.observer = this;
    GUCEF::vector< ASSETCONVERTER::TAssetImporterPtr > importers;
    registry.GetImporters( importers );
    for ( size_t i=0; i<importers.size(); ++i )
        buildOptions.plugins[ "recogniser:" + importers[ i ]->GetName() ] = importers[ i ]->GetVersion();

    ASSETCONVERTER::CPackIndexUpdateOptions updateOptions;
    updateOptions.level = m_redo.reanalyse || m_redo.retryFailed ? ASSETCONVERTER::CPackIndexUpdateOptions::LEVEL_ANALYSIS : ASSETCONVERTER::CPackIndexUpdateOptions::LEVEL_STRUCTURE;
    updateOptions.redo = m_redo;
    updateOptions.packVfsRoot = m_folder;
    if ( !ASSETCONVERTER::CPackIndexLocator::MountUserCache( updateOptions.userCacheRoot ) )
    {
        m_problem = "the folder of the pack index cache cannot be mounted";
        m_control = GUCEF_NULL;
        return false;
    }

    ASSETCONVERTER::CPackIndexUpdater updater( registry.GetKinds() );
    m_ok = updater.EnsureIndex( listing, profile, buildOptions, updateOptions, CORE::CString::Empty, m_document, m_result );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "PackIndexJob: " << m_folder << ": " << m_result.ToSummaryText() );
    if ( !m_ok && !m_result.cancelled )
        m_problem = m_result.message;
    m_control = GUCEF_NULL;
    return m_ok || m_result.cancelled;
}

/*-------------------------------------------------------------------------*/

CIAssetLoadJob::EStepResult
CPackIndexJob::Step( void )
{GUCEF_TRACE;

    if ( !m_isPack )
        return STEP_DONE;
    if ( !m_ok )
        return STEP_FAILED;
    m_shell.ApplyPackIndex( m_folder, m_document, m_result );
    return STEP_DONE;
}

/*-------------------------------------------------------------------------*/

void
CPackIndexJob::Discard( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
