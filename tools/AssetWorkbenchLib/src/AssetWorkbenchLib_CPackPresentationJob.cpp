/*
 *  AssetWorkbenchLib: the work of showing the presentation of a pack in the background
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

#include <Ogre.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef ASSETCONVERTERLIB_CCANONICALJSON_H
#include "AssetConverterLib_CCanonicalJson.h"
#define ASSETCONVERTERLIB_CCANONICALJSON_H
#endif /* ASSETCONVERTERLIB_CCANONICALJSON_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef ASSETWORKBENCHLIB_CPARTICLEPREVIEW_H
#include "AssetWorkbenchLib_CParticlePreview.h"
#define ASSETWORKBENCHLIB_CPARTICLEPREVIEW_H
#endif /* ASSETWORKBENCHLIB_CPARTICLEPREVIEW_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#include "AssetWorkbenchLib_CAssetTypeResolver.h"
#define ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H
#endif /* ASSETWORKBENCHLIB_CASSETTYPERESOLVER_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSHELL_H
#include "AssetWorkbenchLib_CWorkbenchShell.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSHELL_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSHELL_H ? */

#include "AssetWorkbenchLib_CPackPresentationJob.h"

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

CPackPresentationJob::SModel::SModel( void )
    : path(), vfsPath(), group(), relative(), prepared(), ok( false ), cooked(), probed( false ), useCooked( false ), cookedGroup(), cookedRelative()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackPresentationJob::MakeId( const CORE::CString& text )
{GUCEF_TRACE;

    // FNV-1a of 64 bits: a short, stable name for a folder of the cache
    const UInt64 prime = ( (UInt64) 0x100 << 32 ) | (UInt64) 0x1b3;
    UInt64 hash = ( (UInt64) 0xcbf29ce4 << 32 ) | (UInt64) 0x84222325;
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        hash ^= (UInt8) text[ i ];
        hash *= prime;
    }
    static const char digits[] = "0123456789abcdef";
    CORE::CString id;
    for ( int shift=60; shift>=0; shift-=4 )
        id += digits[ ( hash >> shift ) & 0xF ];
    return id;
}

/*-------------------------------------------------------------------------*/

CPackPresentationJob::CPackPresentationJob( CWorkbenchShell& shell, const CPackPresentation& presentation, const CORE::CString& packSource, const CORE::CString& cacheFolder )
    : CIAssetLoadJob()
    , m_shell( shell )
    , m_presentation( presentation )
    , m_cacheFolder( cacheFolder )
    , m_request()
    , m_result()
    , m_converts( !presentation.converter.IsNULLOrEmpty() )
    , m_models()
    , m_mainVfs()
    , m_mainGroup()
    , m_mainRelative()
    , m_mainPrepared()
    , m_step( 0 )
    , m_attached( 0 )
    , m_kept( false )
    , m_problem()
    , m_prefersCooked( shell.IsCookedOutputPreferred() )
    , m_forceCook( shell.m_cookForPresentation )
    , m_cookRequest()
    , m_cookMounted( false )
    , m_cookKept( false )
    , m_cooked()
    , m_cookedProbed( false )
    , m_useCooked( false )
    , m_cookedResult( false )
{GUCEF_TRACE;

    shell.m_cookForPresentation = false;
    if ( !m_prefersCooked )
        m_presentation.cookedNote = "Prefer cooked output is off";

    const CORE::CString root = presentation.packRoot.IsNULLOrEmpty() ? CORE::CString::Empty : presentation.packRoot + "/";
    if ( m_converts && !presentation.assets.empty() )
    {
        m_request.converter = presentation.converter;
        m_request.assetPath = presentation.assets[ 0 ];
        m_request.staticPreview = CPackPresentation::KIND_PORTRAIT == presentation.kind;
        // the scene of a pack has thousands of meshes: the preview of it is made of the geometry merged by material (the intermediate is as it is)
        m_request.mergeStatic = CPackPresentation::KIND_SCENE_DEMO == presentation.kind;
        m_request.id = MakeId( packSource + "|" + presentation.assets[ 0 ] + ( m_request.staticPreview ? "|static" : "" ) + ( m_request.mergeStatic ? "|merged" : "" ) );
        m_request.cacheFolder = CORE::CombinePath( cacheFolder, m_request.id );
        m_request.cook = CPackPresentation::KIND_EFFECT_FOUR_VIEW == presentation.kind;
        if ( "unreal" == presentation.converter && CORE::FileExists( CORE::RelativePath( packSource ) ) && !CORE::DirExists( CORE::RelativePath( packSource ) ) )
        {
            // the package is the source itself: its folder is mounted, the file is the only asset
            m_request.sourceDisk = packSource;
            const Int32 slash = packSource.HasChar( '/', false );
            m_request.assetPath = slash >= 0 ? packSource.SubstrFromRange( (UInt32) slash + 1, packSource.Length() ) : packSource;
        }
        else
        {
            m_request.sourceFolder = "/" + presentation.packRoot;
        }
        // the same asset cooked for Ogre, whole (the rig and the levels of detail stay): the cache folder of the cooked output of this asset
        m_cookRequest = m_request;
        m_cookRequest.staticPreview = false;
        m_cookRequest.mergeStatic = false;
        m_cookRequest.cook = true;
        m_cookRequest.id = MakeId( packSource + "|" + presentation.assets[ 0 ] + "|ogre" );
        m_cookRequest.cacheFolder = CORE::CombinePath( cacheFolder, m_cookRequest.id );
        if ( m_forceCook )
        {
            // Convert for Ogre: this request converts and cooks, and what it made is shown
            m_request = m_cookRequest;
            m_cookedResult = true;
        }
        else
        if ( m_prefersCooked && CPackPresentation::KIND_EFFECT_FOUR_VIEW != presentation.kind )
        {
            ProbeCookedPack( packSource, cacheFolder );
        }
        // the mounts are made here, on the thread of the interface, and the result folder is a mount of the workbench: its files are opened like any other
        if ( CPackAssetConverter::Mount( m_request ) )
            m_shell.m_mounter.Adopt( CPackAssetConverter::GetCacheMountPoint( m_request ) );
        else
            m_problem = "The folder for the conversion cannot be made";
    }
    else
    {
        for ( size_t i=0; i<presentation.assets.size(); ++i )
        {
            SModel model;
            model.path = presentation.assets[ i ];
            model.vfsPath = CAssetMounter::NormalizePath( root + presentation.assets[ i ] );
            model.group = shell.m_mounter.GetResourceGroup( model.vfsPath, model.relative );
            m_models.push_back( model );
        }
        if ( m_prefersCooked )
            ProbeCookedModels();
    }
}

/*-------------------------------------------------------------------------*/

CPackPresentationJob::~CPackPresentationJob()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackPresentationJob::ReportsProgress( void ) const
{GUCEF_TRACE;

    // a grid counts its models; the conversion of a scene is told by the importer how far it is
    return CPackPresentation::KIND_MODEL_GRID == m_presentation.kind || ( CPackPresentation::KIND_SCENE_DEMO == m_presentation.kind && m_converts );
}

/*-------------------------------------------------------------------------*/

const CPackPresentation&
CPackPresentationJob::GetPresentation( void ) const
{GUCEF_TRACE;

    return m_presentation;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPackPresentationJob::GetProblem( void ) const
{GUCEF_TRACE;

    return m_problem;
}

/*-------------------------------------------------------------------------*/

void
CPackPresentationJob::NoteCooked( const CCookedOutputResolver::SCookedOutput& cooked )
{GUCEF_TRACE;

    if ( m_cookedResult )
        return;
    m_presentation.cookedNote = CCookedOutputResolver::Describe( cooked ) + ( cooked.IsCurrent() ? CORE::CString::Empty : CORE::CString( ", use Convert for Ogre" ) );
}

/*-------------------------------------------------------------------------*/

void
CPackPresentationJob::TakeOverShownConversion( const CPackAssetConverter::SRequest& own )
{GUCEF_TRACE;

    // the folder of the conversion that is shown is the folder of this job (the same asset again): it stays mounted when the old view goes
    if ( m_shell.m_hasConversion && CPackAssetConverter::GetCacheMountPoint( m_shell.m_conversion ) == CPackAssetConverter::GetCacheMountPoint( own ) )
        m_shell.m_hasConversion = false;
}

/*-------------------------------------------------------------------------*/

void
CPackPresentationJob::ReleaseCookMount( void )
{GUCEF_TRACE;

    // the folder of a cooked conversion that was mounted for the look and is not shown goes again
    if ( m_cookMounted && !m_cookKept )
    {
        m_shell.m_mounter.Forget( CPackAssetConverter::GetCacheMountPoint( m_cookRequest ) );
        CPackAssetConverter::Unmount( m_cookRequest );
    }
    m_cookKept = true;
}

/*-------------------------------------------------------------------------*/

void
CPackPresentationJob::ProbeCookedPack( const CORE::CString& packSource, const CORE::CString& cacheFolder )
{GUCEF_TRACE;

    const CORE::CString& asset = m_presentation.assets[ 0 ];
    const CORE::CString root = m_presentation.packRoot.IsNULLOrEmpty() ? CORE::CString::Empty : m_presentation.packRoot + "/";
    const CCookedOutputResolver::SOptions options;

    // the layout of a store first; a pack of a local source has its conversion in the cache folder of the workbench. A package file that is the
    // pack (a character package) is the file of the original area of the store: its place in the VFS is the place of its mount
    CORE::CString storePath = CAssetMounter::NormalizePath( root + asset );
    CORE::CString storeAsset;
    if ( !m_request.sourceDisk.IsNULLOrEmpty() )
    {
        CORE::CString relativeToMount;
        const Int32 index = m_shell.m_mounter.FindMountOfDiskPath( CAssetMounter::NormalizeDiskPath( m_request.sourceDisk ), relativeToMount );
        if ( index >= 0 )
            storePath = CAssetMounter::NormalizePath( m_shell.m_mounter.GetMount( (UInt32) index ).mountPoint + "/" + relativeToMount );
    }
    else
    if ( !packSource.IsNULLOrEmpty() && !CORE::DirExists( CORE::RelativePath( packSource ) ) && !CORE::FileExists( CORE::RelativePath( packSource ) ) )
    {
        // the source of the mount is a VFS path (a package in the location of a store that the service mounted)
        storePath = CAssetMounter::NormalizePath( packSource );
    }
    CCookedOutputResolver::LocateInStore( storePath, storeAsset, options, m_cooked );
    if ( !m_cooked.IsLocated() && CORE::DirExists( CORE::RelativePath( m_cookRequest.cacheFolder ) ) && CPackAssetConverter::Mount( m_cookRequest ) )
    {
        const CORE::CString mountPoint = CPackAssetConverter::GetCacheMountPoint( m_cookRequest );
        // a conversion that is shown already holds its mount: this job does not own it
        const bool shownAlready = m_shell.m_hasConversion && CPackAssetConverter::GetCacheMountPoint( m_shell.m_conversion ) == mountPoint;
        if ( !shownAlready )
        {
            m_shell.m_mounter.Adopt( mountPoint );
            m_cookMounted = true;
        }
        const CORE::CString source = m_cookRequest.sourceDisk.IsNULLOrEmpty() ? ASSETCONVERTER::CVfsPaths::Join( m_cookRequest.sourceFolder, m_cookRequest.assetPath )
                                                                               : ASSETCONVERTER::CVfsPaths::Join( CPackAssetConverter::GetSourceMountPoint( m_cookRequest ), m_cookRequest.assetPath );
        CCookedOutputResolver::LocateInFolders( ASSETCONVERTER::CVfsPaths::Join( mountPoint, "intermediate" ), ASSETCONVERTER::CVfsPaths::Join( mountPoint, "out" ), m_cookRequest.assetPath, source, options, m_cooked );
        m_cooked.origin = "cache";
    }
    m_cookedProbed = m_cooked.IsLocated();
    if ( !m_cookedProbed )
        ReleaseCookMount();
}

/*-------------------------------------------------------------------------*/

void
CPackPresentationJob::ProbeCookedModels( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_models.size(); ++i )
    {
        SModel& model = m_models[ i ];
        model.probed = m_shell.ProbeCooked( model.vfsPath, model.cooked );
        // a model of the showcase is a mesh; the cooked scene of an asset is not
        if ( model.probed && "mesh" != CAssetTypeResolver::GetExtension( model.cooked.mainFile ).Lowercase() )
            model.probed = false;
        if ( model.probed )
        {
            model.cookedGroup = m_shell.m_mounter.GetResourceGroup( model.cooked.mainFile, model.cookedRelative );
            model.probed = !model.cookedGroup.IsNULLOrEmpty();
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CPackPresentationJob::PrepareModelFiles( SModel& model, CAssetLoadControl& control, bool& fromCooked )
{GUCEF_TRACE;

    fromCooked = false;
    if ( model.probed )
    {
        // reads the source: here, on the thread of the pool
        CCookedOutputResolver::Verify( model.cooked );
        model.useCooked = model.cooked.IsCurrent();
        if ( model.useCooked )
        {
            if ( CModelView::PrepareMesh( model.cookedGroup, model.cookedRelative, model.prepared, &control ) )
            {
                fromCooked = true;
                return true;
            }
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CPackPresentationJob: The cooked mesh " << model.cooked.mainFile << " cannot be prepared: the quick look follows" );
            model.useCooked = false;
        }
    }
    if ( model.group.IsNULLOrEmpty() )
        return false;
    return CModelView::PrepareMesh( model.group, model.relative, model.prepared, &control );
}

/*-------------------------------------------------------------------------*/

void
CPackPresentationJob::NoteCookedModels( UInt32 cooked, UInt32 prepared )
{GUCEF_TRACE;

    m_presentation.cooked = cooked > 0 && cooked == prepared;
    m_presentation.cookedNote.Clear();
    if ( !m_prefersCooked || m_presentation.cooked )
        return;
    if ( cooked > 0 )
    {
        m_presentation.cookedNote = CORE::UInt32ToString( cooked ) + " of " + CORE::UInt32ToString( prepared ) + " models are from the cooked output, the others a quick look";
        return;
    }
    // what the first model says: not converted, stale
    for ( size_t i=0; i<m_models.size(); ++i )
    {
        if ( m_models[ i ].ok )
        {
            const CCookedOutputResolver::SCookedOutput& first = m_models[ i ].cooked;
            m_presentation.cookedNote = CCookedOutputResolver::Describe( first ) + ( "store" == first.origin ? CORE::CString( ", use Convert for Ogre" ) : CORE::CString::Empty );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CPackPresentationJob::PrepareCookedPack( CAssetLoadControl& control )
{GUCEF_TRACE;

    // reads the source: here, on the thread of the pool
    CCookedOutputResolver::Verify( m_cooked );
    NoteCooked( m_cooked );
    if ( !m_cooked.IsCurrent() )
        return true;
    m_useCooked = true;
    m_presentation.cooked = true;
    m_presentation.cookedNote.Clear();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CPackPresentationJob: The cooked output of " << m_cooked.assetPath << " (" << m_cooked.origin << ") is current: " << m_cooked.mainFile );
    if ( CPackPresentation::KIND_PORTRAIT == m_presentation.kind )
    {
        ASSETCONVERTER::CCanonicalJson::Load( m_cooked.sidecarFile, m_result.sidecar );
        if ( m_cooked.sidecarFile.IsNULLOrEmpty() || !m_presentation.ReadCharacterHints( m_result.sidecar ) )
        {
            m_presentation.fallbackReason = "The character states no portrait view: the model is framed instead";
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CPackPresentationJob: " << m_presentation.fallbackReason );
        }
    }
    return !control.IsCancelled();
}

/*-------------------------------------------------------------------------*/

bool
CPackPresentationJob::PrepareConverted( CAssetLoadControl& control )
{GUCEF_TRACE;

    if ( !m_problem.IsNULLOrEmpty() )
        return false;
    // the cooked Ogre output of the asset when there is a current one: nothing is converted
    if ( m_cookedProbed )
    {
        if ( !PrepareCookedPack( control ) )
            return false;
        if ( m_useCooked )
            return true;
    }
    else
    if ( m_prefersCooked && !m_forceCook && CPackPresentation::KIND_EFFECT_FOUR_VIEW != m_presentation.kind )
    {
        NoteCooked( m_cooked );
    }
    if ( !CPackAssetConverter::Convert( m_request, m_result, &control ) )
    {
        m_problem = m_result.problem;
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CPackPresentationJob: " << m_result.summary << ", result " << m_result.mainFile );
    m_mainVfs = CAssetMounter::NormalizePath( m_result.mainFile );
    if ( CPackPresentation::KIND_PORTRAIT == m_presentation.kind )
    {
        if ( !m_presentation.ReadCharacterHints( m_result.sidecar ) )
        {
            m_presentation.fallbackReason = "The character states no portrait view: the model is framed instead";
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "CPackPresentationJob: " << m_presentation.fallbackReason );
        }
    }
    // the effects are read by the scene view on the thread of the renderer; a model is prepared here
    if ( m_cookedResult && CPackPresentation::KIND_EFFECT_FOUR_VIEW != m_presentation.kind )
        m_presentation.cooked = true;
    if ( CPackPresentation::KIND_EFFECT_FOUR_VIEW != m_presentation.kind && !m_cookedResult )
    {
        m_mainGroup = m_shell.m_mounter.GetResourceGroup( m_mainVfs, m_mainRelative );
        if ( m_mainGroup.IsNULLOrEmpty() )
        {
            m_problem = "The converted file " + m_mainVfs + " is not in a mount";
            return false;
        }
        if ( !CModelView::PrepareMesh( m_mainGroup, m_mainRelative, m_mainPrepared, &control ) )
        {
            m_problem = control.IsCancelled() ? CORE::CString( "Cancelled" ) : "The converted model " + m_mainVfs + " cannot be prepared";
            return false;
        }
    }
    return !control.IsCancelled();
}

/*-------------------------------------------------------------------------*/

bool
CPackPresentationJob::PrepareGrid( CAssetLoadControl& control )
{GUCEF_TRACE;

    UInt32 prepared = 0;
    UInt32 cooked = 0;
    for ( size_t i=0; i<m_models.size(); ++i )
    {
        if ( control.IsCancelled() )
            return false;
        control.SetProgress( (Float32) i / (Float32) m_models.size() );
        bool fromCooked = false;
        m_models[ i ].ok = PrepareModelFiles( m_models[ i ], control, fromCooked );
        if ( m_models[ i ].ok )
        {
            ++prepared;
            if ( fromCooked )
                ++cooked;
        }
        else
        if ( !m_models[ i ].group.IsNULLOrEmpty() )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CPackPresentationJob: The model " << m_models[ i ].path << " is left out: it cannot be prepared" );
    }
    if ( 0 == prepared )
    {
        m_problem = "None of the models of the pack can be read";
        return false;
    }
    NoteCookedModels( cooked, prepared );
    return !control.IsCancelled();
}

/*-------------------------------------------------------------------------*/

bool
CPackPresentationJob::Prepare( CAssetLoadControl& control )
{GUCEF_TRACE;

    switch ( m_presentation.kind )
    {
        case CPackPresentation::KIND_PORTRAIT:
        case CPackPresentation::KIND_SCENE_DEMO:
        case CPackPresentation::KIND_EFFECT_FOUR_VIEW:
        {
            if ( m_converts )
                return PrepareConverted( control );
            return !control.IsCancelled();
        }
        case CPackPresentation::KIND_MODEL_GRID:
        {
            return PrepareGrid( control );
        }
        case CPackPresentation::KIND_ANIMATION_PLAYER:
        {
            if ( !m_models.empty() )
            {
                bool fromCooked = false;
                m_models[ 0 ].ok = PrepareModelFiles( m_models[ 0 ], control, fromCooked );
                if ( m_models[ 0 ].ok )
                    NoteCookedModels( fromCooked ? 1 : 0, 1 );
            }
            return !control.IsCancelled();
        }
        default:
        {
            return !control.IsCancelled();
        }
    }
}

/*-------------------------------------------------------------------------*/

CIAssetLoadJob::EStepResult
CPackPresentationJob::Failed( const CORE::CString& problem )
{GUCEF_TRACE;

    m_problem = problem;
    m_shell.m_openStepFailed = true;
    m_shell.ReportProblem( m_presentation.packRoot, problem );
    ReleaseOwnMount();
    return STEP_FAILED;
}

/*-------------------------------------------------------------------------*/

CIAssetLoadJob::EStepResult
CPackPresentationJob::Done( void )
{GUCEF_TRACE;

    m_shell.FinishPackPresentation( m_presentation );
    // the folder of the conversion stays mounted as long as its result is shown
    if ( m_converts && m_problem.IsNULLOrEmpty() )
    {
        if ( m_useCooked )
        {
            // the cooked output is what is shown: the folder of the quick look is not needed, the cooked one of a local conversion stays
            ReleaseOwnMount();
            if ( m_cookMounted && !m_cookKept )
            {
                m_shell.KeepConversion( m_cookRequest );
                m_cookKept = true;
            }
        }
        else
        {
            m_shell.KeepConversion( m_request );
            m_kept = true;
            ReleaseCookMount();
        }
    }
    return STEP_DONE;
}

/*-------------------------------------------------------------------------*/

CIAssetLoadJob::EStepResult
CPackPresentationJob::StepConvertedModel( void )
{GUCEF_TRACE;

    if ( 0 == m_step++ )
    {
        TakeOverShownConversion( m_request );
        m_shell.UnloadViews();
        // the quick look says what the cooked output is, in the inspector of the model as well as in the presentation
        m_shell.m_sourceText = m_presentation.GetSourceDescription();
        if ( !m_shell.ShowModelLoad( m_mainVfs, &m_mainPrepared ) )
            return Failed( "The converted model cannot be shown" );
        return STEP_MORE;
    }
    m_shell.ShowModelFinish();
    m_shell.ApplyPresentationCamera( m_presentation );
    return Done();
}

/*-------------------------------------------------------------------------*/

CIAssetLoadJob::EStepResult
CPackPresentationJob::StepCookedScene( void )
{GUCEF_TRACE;

    const CORE::CString main = m_useCooked ? m_cooked.mainFile : m_mainVfs;
    const CORE::CString profileFolder = m_useCooked ? ASSETCONVERTER::CVfsPaths::Join( m_cooked.cookedFolder, m_cooked.profile )
                                                     : ASSETCONVERTER::CVfsPaths::Join( ASSETCONVERTER::CVfsPaths::Join( CPackAssetConverter::GetCacheMountPoint( m_request ), "out" ), "desktop" );
    // the folder that is shown already is the folder this job found: it stays mounted when the old view goes
    TakeOverShownConversion( m_useCooked ? m_cookRequest : m_request );
    m_shell.UnloadViews();
    // the files of a conversion that just ran are new to the resource group of their mount: it is made again with them
    if ( m_cookedResult )
        m_shell.m_mounter.InvalidateResourceGroup( main );
    // the scene names its meshes and the materials their pictures by file name: the folder of the cooked profile is a place of the group
    m_shell.m_mounter.AddResourceLocation( profileFolder );
    m_shell.m_sourceText = CCookedOutputResolver::GetCookedLabel();
    bool shown = false;
    if ( "scene" == CAssetTypeResolver::GetExtension( main ).Lowercase() )
    {
        shown = m_shell.ShowScene( main, false );
    }
    else
    {
        CORE::CString relative;
        const CORE::CString group = m_shell.m_mounter.GetResourceGroup( main, relative );
        if ( !group.IsNULLOrEmpty() )
            CParticlePreview::ParseMaterialsNear( group, relative );
        shown = m_shell.ShowModelLoad( main, GUCEF_NULL );
        if ( shown )
            m_shell.ShowModelFinish();
    }
    if ( !shown )
        return Failed( "The cooked output cannot be shown" );
    m_shell.ApplyPresentationCamera( m_presentation );
    m_presentation.cooked = true;
    return Done();
}

/*-------------------------------------------------------------------------*/

CIAssetLoadJob::EStepResult
CPackPresentationJob::StepConvertedScene( void )
{GUCEF_TRACE;

    TakeOverShownConversion( m_request );
    m_shell.UnloadViews();
    // the cooked materials name their pictures from the folder of the cooked profile
    if ( m_converts )
        m_shell.m_mounter.AddResourceLocation( CPackAssetConverter::GetCacheMountPoint( m_request ) + "/out/desktop" );
    const CORE::CString path = m_converts ? m_mainVfs : CAssetMounter::NormalizePath( ( m_presentation.packRoot.IsNULLOrEmpty() ? CORE::CString::Empty : m_presentation.packRoot + "/" ) + m_presentation.assets[ 0 ] );
    const bool particles = "particle" == CAssetTypeResolver::GetExtension( path ).Lowercase();
    if ( particles ? !m_shell.ShowParticles( path ) : !m_shell.ShowScene( path ) )
        return Failed( "The effect cannot be shown" );
    // an effect that has not run yet has no size: it is run for a moment to see how far it reaches, and the camera frames that
    if ( m_shell.m_particles.GetSystemCount() > 0 )
    {
        m_shell.m_particles.SetTime( 0.8f );
        COrbitController::SVector3 center;
        Float32 radius = 0.0f;
        if ( m_shell.m_particles.GetBoundingSphere( center, radius ) )
        {
            m_shell.m_controller.SetBounds( center, radius * 1.6f > 0.3f ? radius * 1.6f : 0.3f );
            m_shell.m_stage.SetGridUnit( radius > 0.0f ? radius / 2.0f : 0.1f );
        }
        m_shell.m_particles.SetTime( 0.0f );
    }
    m_shell.ApplyPresentationView( m_presentation.viewPreset );
    return Done();
}

/*-------------------------------------------------------------------------*/

CIAssetLoadJob::EStepResult
CPackPresentationJob::StepGrid( void )
{GUCEF_TRACE;

    if ( 0 == m_step )
    {
        m_shell.UnloadViews();
        ++m_step;
        return STEP_MORE;
    }
    // one model per piece
    while ( m_attached < m_models.size() && !m_models[ m_attached ].ok )
        ++m_attached;
    if ( m_attached < m_models.size() )
    {
        SModel& model = m_models[ m_attached++ ];
        if ( model.useCooked )
        {
            CParticlePreview::ParseMaterialsNear( model.cookedGroup, model.cookedRelative );
            m_shell.m_showcase.AddModel( model.cookedGroup, model.cookedRelative, model.path, model.prepared );
        }
        else
        {
            m_shell.m_showcase.AddModel( model.group, model.relative, model.path, model.prepared );
        }
        return STEP_MORE;
    }
    if ( 0 == m_shell.m_showcase.GetModelCount() )
        return Failed( "None of the models of the pack can be shown" );
    m_shell.FinishShowcase();
    return Done();
}

/*-------------------------------------------------------------------------*/

CIAssetLoadJob::EStepResult
CPackPresentationJob::StepSingle( void )
{GUCEF_TRACE;

    const CORE::CString first = m_models.empty() ? CORE::CString::Empty : m_models[ 0 ].vfsPath;
    switch ( m_presentation.kind )
    {
        case CPackPresentation::KIND_ANIMATION_PLAYER:
        {
            if ( 0 == m_step++ )
            {
                m_shell.UnloadViews();
                if ( !m_models.empty() && m_models[ 0 ].useCooked )
                {
                    m_shell.m_sourceText = CCookedOutputResolver::GetCookedLabel();
                    CParticlePreview::ParseMaterialsNear( m_models[ 0 ].cookedGroup, m_models[ 0 ].cookedRelative );
                    if ( !m_shell.ShowModelLoad( m_models[ 0 ].cooked.mainFile, &m_models[ 0 ].prepared ) )
                        return Failed( "The cooked model of the animation pack cannot be shown" );
                    return STEP_MORE;
                }
                if ( m_models.empty() || !m_shell.ShowModelLoad( first, &m_models[ 0 ].prepared ) )
                    return Failed( "The model of the animation pack cannot be shown" );
                return STEP_MORE;
            }
            m_shell.ShowModelFinish();
            m_shell.PlayFirstClip();
            return Done();
        }
        case CPackPresentation::KIND_MATERIAL_PREVIEW:
        {
            m_shell.UnloadViews();
            if ( first.IsNULLOrEmpty() || !m_shell.ShowMaterial( first ) )
                return Failed( "The material cannot be shown" );
            return Done();
        }
        case CPackPresentation::KIND_IMAGE_GRID:
        case CPackPresentation::KIND_ENVIRONMENT_PREVIEW:
        {
            m_shell.UnloadViews();
            if ( first.IsNULLOrEmpty() || !m_shell.ShowTexture( first ) )
                return Failed( "The picture cannot be shown" );
            m_shell.ShowTextureFinish();
            return Done();
        }
        default:
        {
            // the summary and the audio pack: nothing in 3D, the inspector holds the summary and the list
            m_shell.UnloadViews();
            return Done();
        }
    }
}

/*-------------------------------------------------------------------------*/

CIAssetLoadJob::EStepResult
CPackPresentationJob::Step( void )
{GUCEF_TRACE;

    m_shell.m_openStepFailed = false;
    switch ( m_presentation.kind )
    {
        case CPackPresentation::KIND_PORTRAIT:
        case CPackPresentation::KIND_SCENE_DEMO:
        {
            return m_useCooked || m_cookedResult ? StepCookedScene() : StepConvertedModel();
        }
        case CPackPresentation::KIND_EFFECT_FOUR_VIEW:
        {
            return StepConvertedScene();
        }
        case CPackPresentation::KIND_MODEL_GRID:
        {
            return StepGrid();
        }
        default:
        {
            return StepSingle();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPackPresentationJob::Discard( void )
{GUCEF_TRACE;

    CModelView::DiscardPrepared( m_mainPrepared );
    ReleaseCookMount();
    for ( size_t i=0; i<m_models.size(); ++i )
        CModelView::DiscardPrepared( m_models[ i ].prepared );
    // a request that was ended between its pieces leaves a half made presentation
    if ( m_step > 0 && m_shell.m_initialized )
        m_shell.UnloadViews();
    m_step = 0;
    ReleaseOwnMount();
}

/*-------------------------------------------------------------------------*/

void
CPackPresentationJob::ReleaseOwnMount( void )
{GUCEF_TRACE;

    // what was not handed to the shell goes: the folder of the conversion is unmounted again
    if ( m_converts && !m_kept && !m_request.id.IsNULLOrEmpty() )
    {
        m_shell.m_mounter.Forget( CPackAssetConverter::GetCacheMountPoint( m_request ) );
        CPackAssetConverter::Unmount( m_request );
        m_kept = true;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
