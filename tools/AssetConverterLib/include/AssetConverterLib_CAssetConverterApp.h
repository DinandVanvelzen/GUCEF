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

#ifndef ASSETCONVERTERLIB_CASSETCONVERTERAPP_H
#define ASSETCONVERTERLIB_CASSETCONVERTERAPP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#include "AssetConverterLib_CAssetPipelineOptions.h"
#define ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H
#endif /* ASSETCONVERTERLIB_CASSETPIPELINEOPTIONS_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKPLAN_H
#include "AssetConverterLib_CAssetPackPlan.h"
#define ASSETCONVERTERLIB_CASSETPACKPLAN_H
#endif /* ASSETCONVERTERLIB_CASSETPACKPLAN_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The command line converter, tools/AssetConverter, as a class so that tests run it in the process:
 *
 *  - "--manifest <pack.assetpack.json>" runs a pack. The folder of the manifest file is mounted (writeable) at "pack",
 *    so the relative paths of the manifest are folders next to it. A path that is no file on the disk is taken as a
 *    VFS path.
 *  - "--input <vfs path> --importer <name> --licence <record file>" runs one source as a pack of its own ("--pack <id>"
 *    names it, default the name of the source; "--intermediate <folder>" holds its intermediate, default
 *    <out>/intermediate). "--mount <folder>[=<mount point>]" (repeatable, read-only) makes the source reachable.
 *  - "--cook <target>" (repeatable, or "a,b") cooks for those targets instead of the manifest's; "--out <folder>" is the
 *    output folder (mounted at "out"); "--report <file>" the report (and its summary as .txt next to it).
 *  - "--force" cooks assets with validation errors, "--reimport" ignores the cache, "--threads <n>" sets the workers.
 *  - "--only <glob>" (repeatable) restricts the run to the assets whose path in the pack matches; "--validate-only"
 *    imports and validates without writing anything but the report of "--report"; "--cook-only" cooks what the
 *    intermediate holds; "--dry-run" writes nothing and tells per asset what the run would do and why (the plan, as
 *    JSON in the file of "--report" when given).
 *  - "--bootstrap <ini>", "--config <json>" and "--log <file>" are for the program (see AssetConverter_main.cpp).
 *
 *  Exit codes: 0 done, 1 bad command line, 2 the settings or the manifest cannot be read, 3 refused by the licence
 *  record, 4 assets failed, 5 the intermediate is in use by another run (its lock is held). The settings section "AssetConverter" gives "threads" and "threadPoolName" defaults.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetConverterApp : public CORE::CGloballyConfigurable
{
    public:

    static const CORE::CString ClassTypeName;
    static const CORE::CString AppName;         /**< "AssetConverter": the settings section and the names of the config files */

    static const Int32 ExitSuccess = 0;
    static const Int32 ExitUsage = 1;
    static const Int32 ExitConfig = 2;
    static const Int32 ExitRefused = 3;
    static const Int32 ExitFailures = 4;
    static const Int32 ExitBusy = 5;

    CAssetConverterApp( int argc, char** argv );

    virtual ~CAssetConverterApp() GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the command line; false when it cannot be read or help was asked for */
    bool ParseCommandLine( void );

    static void LogUsage( void );

    /** Mounts what the command line names, runs the pipeline, writes the report; the exit code */
    Int32 Run( void );

    /** Unmounts what Run() mounted (the destructor does it too) */
    void Unmount( void );

    const CORE::CString& GetBootstrapConfigFile( void ) const;
    const CORE::CString& GetConfigFile( void ) const;
    const CORE::CString& GetLogFile( void ) const;
    Int32 GetMinimalLogLevel( void ) const;

    /** The report of the last Run() */
    const CConversionReport& GetReport( void ) const;

    /** The plan of the last Run() with --dry-run */
    const CAssetPackPlan& GetPlan( void ) const;

    /** Whether the command line asked for a dry run */
    bool IsDryRun( void ) const;

    /** The options the command line and the settings gave */
    const CAssetPipelineOptions& GetOptions( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& globalConfig ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& globalConfig ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** Mounts a folder of the disk (made when 'create'); false when it cannot be */
    bool MountFolder( const CORE::CString& folder, const CORE::CString& mountPoint, bool writeable, bool create );

    /** The VFS path of a file of the disk, its folder mounted at the mount point; a path that is no file is taken as a VFS path */
    bool MountFileFolder( const CORE::CString& file, const CORE::CString& mountPoint, bool writeable, bool create, CORE::CString& vfsPath );

    CAssetConverterApp( const CAssetConverterApp& src );            /**< not implemented */
    CAssetConverterApp& operator=( const CAssetConverterApp& src ); /**< not implemented */

    private:

    int m_argc;
    char** m_argv;

    CORE::CString m_bootstrapConfigFile;
    CORE::CString m_configFile;
    CORE::CString m_logFile;
    Int32 m_minimalLogLevel;
    CORE::CString m_manifestFile;
    CORE::CString m_input;
    CORE::CString m_importer;
    CORE::CString m_licenceFile;
    CORE::CString m_packId;
    CORE::CString m_intermediateFolder;
    CORE::CString m_outFolder;
    CORE::CString m_reportFile;
    CORE::CString::StringVector m_mountSpecs;
    bool m_threadsGiven;
    bool m_dryRun;
    CAssetPackPlan m_plan;
    CAssetPipelineOptions m_options;
    CConversionReport m_report;
    CORE::CString::StringVector m_mounts;     /**< the archive names of the mounts of Run() */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETCONVERTERAPP_H ? */
