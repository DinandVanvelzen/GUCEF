/*
 *  AssetConverterLib: the command line converter: options, mounts, a run of the pipeline and its exit code
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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#include "AssetConverterLib_CAssetPackManifest.h"
#define ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#endif /* ASSETCONVERTERLIB_CASSETPACKMANIFEST_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINE_H
#include "AssetConverterLib_CAssetPipeline.h"
#define ASSETCONVERTERLIB_CASSETPIPELINE_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINE_H ? */

#ifndef ASSETCONVERTERLIB_CLICENCERECORD_H
#include "AssetConverterLib_CLicenceRecord.h"
#define ASSETCONVERTERLIB_CLICENCERECORD_H
#endif /* ASSETCONVERTERLIB_CLICENCERECORD_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CAssetConverterApp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAssetConverterApp::ClassTypeName = "GUCEF::ASSETCONVERTER::CAssetConverterApp";
const CORE::CString CAssetConverterApp::AppName = "AssetConverter";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetConverterApp::CAssetConverterApp( int argc, char** argv )
    : CORE::CGloballyConfigurable()
    , m_argc( argc )
    , m_argv( argv )
    , m_bootstrapConfigFile()
    , m_configFile()
    , m_logFile()
    , m_minimalLogLevel( CORE::LOGLEVEL_NORMAL )
    , m_manifestFile()
    , m_input()
    , m_importer()
    , m_licenceFile()
    , m_packId()
    , m_intermediateFolder()
    , m_outFolder()
    , m_reportFile()
    , m_mountSpecs()
    , m_threadsGiven( false )
    , m_dryRun( false )
    , m_plan()
    , m_options()
    , m_report()
    , m_mounts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetConverterApp::~CAssetConverterApp()
{GUCEF_TRACE;

    Unmount();
}

/*-------------------------------------------------------------------------*/

void
CAssetConverterApp::LogUsage( void )
{GUCEF_TRACE;

    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << " converts asset packs into the engine-neutral intermediate and cooks it\n"
        "  --manifest <pack.assetpack.json>       runs the pack of the manifest\n"
        "  --input <vfs path> --importer <name> --licence <record.licence.json> [--pack <id>] [--intermediate <folder>]\n"
        "                                         runs one source as a pack of its own\n"
        "  --mount [<mount point>=]<folder or archive>  mounts a folder, or an archive the VFS can read, read-only (repeatable)\n"
        "  --cook <target>                        cooks for the target (repeatable, or a,b)\n"
        "  --out <folder>                         the output folder of the cooked files\n"
        "  --report <file>                        the report (JSON; the summary as .txt next to it)\n"
        "  --force                                cooks assets with validation errors\n"
        "  --reimport                             ignores the cache\n"
        "  --index-only                           builds the pack index (pack-index.json in the intermediate) and stops\n"
        "  --no-index                             does not write the pack index\n"
        "  --reindex                              builds the structure and grouping of the pack index again\n"
        "  --analyse                              adds the analysis level to the pack index: the sizes of the pictures from their headers (slow in an archive)\n"
        "  --reanalyse                            reads the files again for the statistics of the pack index\n"
        "  --rethumbnail                          renders the thumbnails again (carried in the options: the viewer renders them)\n"
        "  --retry-failed                         analyses again what failed before\n"
        "  --only <glob>                          converts only the assets whose path in the pack matches (repeatable)\n"
        "  --validate-only                        imports and validates; writes nothing but the report of --report\n"
        "  --cook-only                            cooks what the intermediate holds\n"
        "  --dry-run                              writes nothing: what the run would do per asset and why (to --report as JSON)\n"
        "  --threads <n>                          the number of workers (0: one per logical CPU)\n"
        "  --bootstrap <ini>, --config <json>, --log <file>" );
}

/*-------------------------------------------------------------------------*/

bool
CAssetConverterApp::ParseCommandLine( void )
{GUCEF_TRACE;

    for ( int i=1; i<m_argc; ++i )
    {
        const CORE::CString arg( m_argv[ i ] );
        const bool hasValue = i+1 < m_argc;
        if ( "--help" == arg || "-h" == arg )
            return false;
        else if ( "--force" == arg )
            m_options.force = true;
        else if ( "--index-only" == arg )
            m_options.indexOnly = true;
        else if ( "--no-index" == arg )
            m_options.noIndex = true;
        else if ( "--reindex" == arg )
            m_options.reindex = true;
        else if ( "--analyse" == arg )
            m_options.analyse = true;
        else if ( "--reanalyse" == arg )
            m_options.reanalyse = true;
        else if ( "--rethumbnail" == arg )
            m_options.rethumbnail = true;
        else if ( "--retry-failed" == arg )
            m_options.retryFailed = true;
        else if ( "--reimport" == arg )
            m_options.reimport = true;
        else if ( "--validate-only" == arg )
            m_options.mode = PIPELINEMODE_VALIDATE_ONLY;
        else if ( "--cook-only" == arg )
            m_options.mode = PIPELINEMODE_COOK_ONLY;
        else if ( "--dry-run" == arg )
            m_dryRun = true;
        else if ( "--only" == arg && hasValue )
            m_options.assetGlobs.push_back( m_argv[ ++i ] );
        else if ( "--manifest" == arg && hasValue )
            m_manifestFile = m_argv[ ++i ];
        else if ( "--input" == arg && hasValue )
            m_input = m_argv[ ++i ];
        else if ( "--importer" == arg && hasValue )
            m_importer = m_argv[ ++i ];
        else if ( "--licence" == arg && hasValue )
            m_licenceFile = m_argv[ ++i ];
        else if ( "--pack" == arg && hasValue )
            m_packId = m_argv[ ++i ];
        else if ( "--intermediate" == arg && hasValue )
            m_intermediateFolder = m_argv[ ++i ];
        else if ( "--mount" == arg && hasValue )
            m_mountSpecs.push_back( m_argv[ ++i ] );
        else if ( "--cook" == arg && hasValue )
        {
            const CORE::CString::StringVector targets = CORE::CString( m_argv[ ++i ] ).ParseElements( ',', false );
            for ( size_t t=0; t<targets.size(); ++t )
                m_options.cookTargets.push_back( targets[ t ].Trim( true ).Trim( false ) );
        }
        else if ( "--out" == arg && hasValue )
            m_outFolder = m_argv[ ++i ];
        else if ( "--report" == arg && hasValue )
            m_reportFile = m_argv[ ++i ];
        else if ( "--threads" == arg && hasValue )
        {
            m_options.threads = CORE::StringToUInt32( m_argv[ ++i ], 0 );
            m_threadsGiven = true;
        }
        else if ( "--bootstrap" == arg && hasValue )
            m_bootstrapConfigFile = m_argv[ ++i ];
        else if ( "--config" == arg && hasValue )
            m_configFile = m_argv[ ++i ];
        else if ( "--log" == arg && hasValue )
            m_logFile = m_argv[ ++i ];
        else
        {
            GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": Unknown option or option without its value: " << arg );
            return false;
        }
    }
    for ( int i=1; i<m_argc; ++i )
    {
        if ( ( "--validate-only" == CORE::CString( m_argv[ i ] ) && PIPELINEMODE_VALIDATE_ONLY != m_options.mode ) ||
             ( "--cook-only" == CORE::CString( m_argv[ i ] ) && PIPELINEMODE_COOK_ONLY != m_options.mode ) )
        {
            GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": give --validate-only or --cook-only, not both" );
            return false;
        }
    }
    if ( m_manifestFile.IsNULLOrEmpty() == m_input.IsNULLOrEmpty() )
    {
        GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": give either --manifest or --input" );
        return false;
    }
    if ( !m_input.IsNULLOrEmpty() && ( m_importer.IsNULLOrEmpty() || m_licenceFile.IsNULLOrEmpty() ) )
    {
        GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": --input needs --importer and --licence" );
        return false;
    }
    if ( !m_input.IsNULLOrEmpty() && m_intermediateFolder.IsNULLOrEmpty() && m_outFolder.IsNULLOrEmpty() )
    {
        GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": --input needs --intermediate or --out" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetConverterApp::MountFolder( const CORE::CString& folder, const CORE::CString& mountPoint, bool writeable, bool create )
{GUCEF_TRACE;

    const CORE::CString nativePath = CORE::RelativePath( folder );
    if ( !create && !CORE::DirExists( nativePath ) && CORE::FileExists( nativePath ) )
    {
        // an archive (zip, tar, ...) the VFS can read: mounted as it is, read only
        CORE::CString archiveType;
        VFS::CVFS& archiveVfs = VFS::CVfsGlobal::Instance()->GetVfs();
        const CORE::CString archiveMountName = AppName + "/" + mountPoint;
        if ( !archiveVfs.TryGetArchiveTypeForFile( nativePath, archiveType ) || archiveType.IsNULLOrEmpty() ||
             !archiveVfs.MountArchive( archiveMountName, nativePath, archiveType, mountPoint, false, false, nativePath ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": the file " << folder << " is no archive that can be mounted at " << mountPoint << " (is the plugin of its format loaded?)" );
            return false;
        }
        m_mounts.push_back( archiveMountName );
        return true;
    }
    if ( create && !CORE::DirExists( nativePath ) )
        CORE::CreateDirs( nativePath );
    if ( !CORE::DirExists( nativePath ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": the folder " << folder << " is not there" );
        return false;
    }
    const CORE::CString archiveName = AppName + "/" + mountPoint;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.MountArchive( archiveName, nativePath, VFS::CVFS::FileSystemArchiveTypeName, mountPoint, writeable, false, nativePath ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": the folder " << folder << " cannot be mounted at " << mountPoint );
        return false;
    }
    m_mounts.push_back( archiveName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetConverterApp::MountFileFolder( const CORE::CString& file, const CORE::CString& mountPoint, bool writeable, bool create, CORE::CString& vfsPath )
{GUCEF_TRACE;

    const CORE::CString nativePath = CORE::RelativePath( file );
    const CORE::CString nativeFolder = CORE::StripFilename( nativePath );
    if ( !create && !CORE::FileExists( nativePath ) )
    {
        // not on the disk: a VFS path
        vfsPath = CVfsPaths::Normalize( file );
        return true;
    }
    if ( !MountFolder( nativeFolder.IsNULLOrEmpty() ? CORE::CString( "." ) : nativeFolder, mountPoint, writeable, create ) )
        return false;
    vfsPath = CVfsPaths::Join( mountPoint, CORE::ExtractFilename( nativePath ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetConverterApp::Unmount( void )
{GUCEF_TRACE;

    if ( m_mounts.empty() )
        return;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    for ( size_t i=m_mounts.size(); i>0; --i )
        vfs.UnmountArchiveByName( m_mounts[ i-1 ] );
    m_mounts.clear();
}

/*-------------------------------------------------------------------------*/

Int32
CAssetConverterApp::Run( void )
{GUCEF_TRACE;

    Unmount();
    m_report.Clear();

    // the folders the command line mounts
    for ( size_t i=0; i<m_mountSpecs.size(); ++i )
    {
        // [<mount point>=]<folder>, as the viewer takes it: the text before the first '=' is a mount point only when it holds no
        // path separator, so a folder whose path contains '=' can be given without a mount point
        CORE::CString folder = m_mountSpecs[ i ];
        CORE::CString mountPoint;
        const Int32 equals = folder.HasChar( '=', true );
        if ( equals > 0 )
        {
            const CORE::CString name = folder.SubstrFromRange( 0, (UInt32) equals );
            if ( name.HasChar( '/', true ) < 0 && name.HasChar( '\\', true ) < 0 )
            {
                mountPoint = CVfsPaths::Normalize( name );
                folder = folder.SubstrFromRange( (UInt32) equals+1, folder.Length() );
            }
        }
        if ( mountPoint.IsNULLOrEmpty() )
            mountPoint = CORE::LastSubDir( CORE::RelativePath( folder ) );
        if ( !MountFolder( folder, mountPoint, false, false ) )
            return ExitConfig;
    }

    CAssetPipelineOptions options = m_options;
    if ( !m_outFolder.IsNULLOrEmpty() )
    {
        if ( !MountFolder( m_outFolder, "out", true, true ) )
            return ExitConfig;
        options.outputDir = "/out";
    }
    if ( !m_reportFile.IsNULLOrEmpty() )
    {
        CORE::CString reportPath;
        if ( !MountFileFolder( m_reportFile, "report", true, true, reportPath ) )
            return ExitConfig;
        options.reportPath = reportPath;
    }

    CAssetPackManifest manifest;
    if ( !m_manifestFile.IsNULLOrEmpty() )
    {
        CORE::CString manifestPath;
        if ( !MountFileFolder( m_manifestFile, "pack", true, false, manifestPath ) || !manifest.Load( manifestPath ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": the manifest " << m_manifestFile << " cannot be read (is the JSON codec plugin loaded?)" );
            return ExitConfig;
        }
    }
    else
    {
        // one source as a pack of its own
        CORE::CString licencePath;
        CLicenceRecord record;
        if ( !MountFileFolder( m_licenceFile, "licence", false, false, licencePath ) || !record.Load( licencePath ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << ": the licence record " << m_licenceFile << " cannot be read" );
            return ExitConfig;
        }
        const CORE::CString input = CVfsPaths::Normalize( m_input );
        manifest.id = m_packId.IsNULLOrEmpty() ? CVfsPaths::StemOf( input ) : m_packId;
        manifest.title = manifest.id;
        manifest.licenceRecord = record.id;
        manifest.licenceFolder = "/" + CVfsPaths::FolderOf( licencePath );
        manifest.source = "/" + CVfsPaths::FolderOf( input );
        manifest.include.push_back( CVfsPaths::NameOf( input ) );
        manifest.importer = m_importer;
        if ( !m_intermediateFolder.IsNULLOrEmpty() )
        {
            if ( !MountFolder( m_intermediateFolder, "intermediate", true, true ) )
                return ExitConfig;
            manifest.intermediate = "/intermediate";
        }
        else
        {
            manifest.intermediate = "/out/intermediate";
        }
        manifest.output = "/out";
    }

    CAssetPipeline pipeline;
    if ( m_dryRun )
    {
        // nothing is written but the plan, to the report file when one is given
        m_plan.Clear();
        const bool planned = pipeline.PlanPack( manifest, options, m_plan );
        GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, m_plan.ToSummaryText() );
        if ( !options.reportPath.IsNULLOrEmpty() )
            m_plan.SaveJson( options.reportPath );
        if ( !planned )
            return ExitConfig;
        if ( !m_plan.conversionAllowed )
            return ExitRefused;
        return m_plan.IsRunnable() ? ExitSuccess : ExitConfig;
    }
    const bool success = pipeline.RunPack( manifest, options, m_report );
    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, m_report.ToSummaryText() );
    if ( m_report.IsRefused() )
        return ExitRefused;
    if ( m_report.IsLocked() )
        return ExitBusy;
    return success ? ExitSuccess : ExitFailures;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& CAssetConverterApp::GetBootstrapConfigFile( void ) const { return m_bootstrapConfigFile; }
const CORE::CString& CAssetConverterApp::GetConfigFile( void ) const { return m_configFile; }
const CORE::CString& CAssetConverterApp::GetLogFile( void ) const { return m_logFile; }
Int32 CAssetConverterApp::GetMinimalLogLevel( void ) const { return m_minimalLogLevel; }
const CConversionReport& CAssetConverterApp::GetReport( void ) const { return m_report; }
const CAssetPipelineOptions& CAssetConverterApp::GetOptions( void ) const { return m_options; }
const CAssetPackPlan& CAssetConverterApp::GetPlan( void ) const { return m_plan; }
bool CAssetConverterApp::IsDryRun( void ) const { return m_dryRun; }

/*-------------------------------------------------------------------------*/

bool
CAssetConverterApp::SaveConfig( CORE::CDataNode& globalConfig ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = globalConfig.Structure( AppName, '/' );
    section->SetAttribute( "threads", m_options.threads );
    section->SetAttribute( "threadPoolName", m_options.threadPoolName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetConverterApp::LoadConfig( const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    // the command line goes over the settings
    const CORE::CDataNode* section = globalConfig.FindChild( AppName );
    if ( GUCEF_NULL != section )
    {
        if ( !m_threadsGiven )
            m_options.threads = section->GetAttributeValueOrChildValueByName( "threads" ).AsUInt32( m_options.threads, true );
        m_options.threadPoolName = section->GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( m_options.threadPoolName, true );
    }
    const CORE::CDataNode* appArgs = globalConfig.Search( "Main/AppArgs", '/', true, true );
    if ( GUCEF_NULL != appArgs )
    {
        m_minimalLogLevel = appArgs->GetAttributeValueOrChildValueByName( "minimalLogLevel" ).AsInt32( m_minimalLogLevel, true );
        if ( m_logFile.IsNULLOrEmpty() )
            m_logFile = appArgs->GetAttributeValueOrChildValueByName( "logFile" ).AsString( CORE::CString::Empty, true );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetConverterApp::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
