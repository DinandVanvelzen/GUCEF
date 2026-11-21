/*
 *  assetpluginUNREAL_TestApp: the triage of real packs: every package of the pack folders through the importer, results by failure class
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

#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

#ifndef GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_CORE_ASSETCONVERTERLIB_H
#include "AssetConverterLib.h"
#define GUCEF_CORE_ASSETCONVERTERLIB_H
#endif /* GUCEF_CORE_ASSETCONVERTERLIB_H ? */

#ifndef GUCEF_ASSETCONVERTER_CTEXTUREBAKER_H
#include "AssetConverterLib_CTextureBaker.h"
#define GUCEF_ASSETCONVERTER_CTEXTUREBAKER_H
#endif /* GUCEF_ASSETCONVERTER_CTEXTUREBAKER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_CORE_TESTUNREALSUPPORT_H
#include "TestUnrealSupport.h"
#define GUCEF_CORE_TESTUNREALSUPPORT_H
#endif /* GUCEF_CORE_TESTUNREALSUPPORT_H ? */

#include "TestUnrealPackTriage.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNREAL;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)

namespace {

/** The result of one package */
struct STriageResult
{
    CORE::CString pack;
    CORE::CString path;
    CORE::CString classes;
    CORE::CString versions;
    TAssetStatus status;
    CORE::CString reason;           /**< the first error, or the first skip reason */
    CORE::CString reasonClass;      /**< the reason with numbers and the names of the package taken out */
};

typedef GUCEF::map< CORE::CString, UInt32 > TCountMap;
typedef GUCEF::map< CORE::CString, CORE::CString > TExampleMap;

/** Counts of one pack */
struct SPackTally
{
    UInt32 statusCounts[ ASSETSTATUS_COUNT ];
    TCountMap failureClasses;
    TExampleMap failureExamples;
    TCountMap skipClasses;
    TCountMap classCounts;
    TCountMap versionCounts;

    SPackTally( void ) { for ( UInt32 s=0; s<ASSETSTATUS_COUNT; ++s ) statusCounts[ s ] = 0; }
};

}

/*-------------------------------------------------------------------------*/

/** The text with every run of digits replaced by N and the given names by <name> */
static CORE::CString
ReasonClassOf( const CORE::CString& reason, const CORE::CString::StringVector& names )
{
    CORE::CString text = reason;
    for ( size_t n=0; n<names.size(); ++n )
        if ( names[ n ].Length() >= 3 )
            text = text.ReplaceSubstr( names[ n ], "<name>" );
    CORE::CString result;
    bool inNumber = false;
    for ( UInt32 c=0; c<text.Length(); ++c )
    {
        if ( isdigit( (unsigned char) text[ c ] ) )
        {
            if ( !inNumber )
                result += 'N';
            inNumber = true;
            continue;
        }
        inNumber = false;
        result += text[ c ];
    }
    return result;
}

/*-------------------------------------------------------------------------*/

/** The status the pipeline gives the package, the way CAssetPipeline::ConvertAsset() and CConversionReportEntry decide it */
static TAssetStatus
StatusOf( bool imported, const CConversionReportEntry& entry, CORE::CString& reason )
{
    reason.Clear();
    for ( size_t i=0; i<entry.items.size() && reason.IsNULLOrEmpty(); ++i )
        if ( REPORTITEM_ERROR == entry.items[ i ].kind )
            reason = entry.items[ i ].message;
    if ( !reason.IsNULLOrEmpty() )
        return ASSETSTATUS_FAILED;
    if ( !imported )
    {
        if ( entry.skipped )
        {
            for ( size_t i=0; i<entry.items.size() && reason.IsNULLOrEmpty(); ++i )
                if ( REPORTITEM_SKIPPED == entry.items[ i ].kind )
                    reason = entry.items[ i ].message;
            return ASSETSTATUS_SKIPPED;
        }
        reason = "the import failed";
        return ASSETSTATUS_FAILED;
    }
    return entry.GetItemCount( REPORTITEM_APPROXIMATION ) > 0 ? ASSETSTATUS_APPROXIMATED : ASSETSTATUS_CONVERTED;
}

/*-------------------------------------------------------------------------*/

/** The value of an environment variable, empty when it is not set */
static CORE::CString
EnvironmentValue( const char* name )
{
    const char* value = getenv( name );
    return GUCEF_NULL != value ? CORE::CString( value ) : CORE::CString();
}

/*-------------------------------------------------------------------------*/

/** Converts the packages of one pack folder; the results are added to 'results' and 'tally' */
static bool
TriagePack( const CORE::CString& folder, UInt32 packNumber, TAssetImporterPtr& importer, const CORE::CString& only, bool checkPictures,
            GUCEF::vector< STriageResult >& results, SPackTally& tally )
{
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString packName = CORE::LastSubDir( folder );
    const CORE::CString mountName = "unreal_triage_" + CORE::UInt32ToString( packNumber );
    if ( !vfs.MountArchive( mountName, folder, VFS::CVFS::FileSystemArchiveTypeName, mountName, false, false, folder ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Triage: the folder " << folder << " cannot be mounted" );
        return false;
    }

    CAssetPackManifest manifest;
    manifest.id = mountName;
    manifest.source = mountName;
    manifest.importer = "unreal";
    manifest.intermediate = WorkVfsPath( "triage_intermediate" );
    CAssetPackContext context;
    if ( !context.Open( manifest ) )
    {
        vfs.UnmountArchiveByName( mountName );
        return false;
    }
    importer->IndexPack( context );

    CORE::CString::StringVector files = context.GetFiles();
    std::sort( files.begin(), files.end() );
    const UInt64 start = MT::PrecisionTickCount();
    for ( size_t f=0; f<files.size(); ++f )
    {
        const CORE::CString extension = CVfsPaths::ExtensionOf( files[ f ] ).Lowercase();
        if ( "uasset" != extension && "umap" != extension && "mhpkg" != extension )
            continue;
        if ( !only.IsNULLOrEmpty() && files[ f ].HasSubstr( only ) < 0 )
            continue;

        STriageResult result;
        result.pack = packName;
        result.path = files[ f ];
        const CORE::CString vfsPath = context.ToVfsPath( files[ f ] );
        CORE::CString::StringVector names;
        names.push_back( CVfsPaths::StemOf( files[ f ] ) );
        {
            CUnrealPackage package;
            if ( package.Open( vfsPath ) )
            {
                const CUnrealVersionContext& versions = package.GetVersions();
                result.versions = "UE4 " + CORE::Int32ToString( versions.GetUe4Version() ) + " UE5 " + CORE::Int32ToString( versions.GetUe5Version() );
                CORE::CString::StringSet seen;
                for ( size_t e=0; e<package.GetExports().size(); ++e )
                {
                    if ( !package.GetExports()[ e ].outerIndex.IsNull() )
                        continue;
                    const CORE::CString className = package.GetClassName( package.GetExports()[ e ] );
                    names.push_back( package.GetExports()[ e ].objectName );
                    if ( "MetaData" == className || !seen.insert( className ).second )
                        continue;
                    result.classes += ( result.classes.IsNULLOrEmpty() ? "" : "," ) + className;
                    ++tally.classCounts[ className ];
                }
            }
            else
            {
                result.versions = "unreadable";
            }
        }
        ++tally.versionCounts[ result.versions ];

        CConversionReport report;
        CConversionReportEntry& selected = report.SelectAsset( files[ f ] );
        selected.importer = importer->GetName();
        CAssetScene scene;
        bool imported = false;
        try
        {
            imported = importer->Import( vfsPath, context, scene, report );
        }
        catch ( ... )
        {
            report.AddError( "triage", files[ f ], "the import threw an exception" );
        }
        const CConversionReportEntry* entry = report.FindEntry( files[ f ] );
        if ( GUCEF_NULL == entry )
            entry = &selected;
        result.status = StatusOf( imported, *entry, result.reason );

        // the pictures the importer wrote, decoded as the texture baker of the pipeline decodes them
        if ( checkPictures && ( ASSETSTATUS_CONVERTED == result.status || ASSETSTATUS_APPROXIMATED == result.status ) )
        {
            for ( size_t t=0; t<scene.textures.size(); ++t )
            {
                if ( scene.textures[ t ].sources.empty() || !vfs.FileExists( scene.textures[ t ].sources[ 0 ] ) )
                    continue;
                IMAGE::TPixelMapPtr picture;
                CORE::CString problem;
                if ( !CTextureBaker::LoadPicture( scene.textures[ t ].sources[ 0 ], picture, problem ) )
                {
                    names.push_back( scene.textures[ t ].name );
                    result.status = ASSETSTATUS_FAILED;
                    result.reason = "picture " + scene.textures[ t ].name + ": " + problem;
                    break;
                }
            }
        }
        result.reasonClass = ReasonClassOf( result.reason, names );
        ++tally.statusCounts[ result.status ];
        if ( ASSETSTATUS_FAILED == result.status )
        {
            if ( 1 == ++tally.failureClasses[ result.reasonClass ] )
                tally.failureExamples[ result.reasonClass ] = files[ f ];
        }
        else if ( ASSETSTATUS_SKIPPED == result.status )
        {
            ++tally.skipClasses[ result.reasonClass ];
        }
        results.push_back( result );

        // the files of the intermediate are not kept
        const CORE::CString intermediateDisk = WorkDiskPath( "triage_intermediate" );
        if ( CORE::DirExists( intermediateDisk ) )
            CORE::DeleteDir( intermediateDisk, true );
    }
    const Float64 seconds = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) / 1000.0;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Triage of " << packName << ": " << results.size() << " packages so far, " << seconds << " s for this pack" );
    context.Close();
    vfs.UnmountArchiveByName( mountName );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The lines of the counts of a map, the largest first */
static CORE::CString
SortedCounts( const TCountMap& counts, const TExampleMap* examples, const CORE::CString& indent )
{
    typedef GUCEF::vector< std::pair< UInt32, CORE::CString > > TOrder;
    TOrder order;
    for ( TCountMap::const_iterator i=counts.begin(); i!=counts.end(); ++i )
        order.push_back( std::pair< UInt32, CORE::CString >( (*i).second, (*i).first ) );
    std::sort( order.begin(), order.end() );
    std::reverse( order.begin(), order.end() );
    CORE::CString text;
    for ( size_t i=0; i<order.size(); ++i )
    {
        text += indent + CORE::UInt32ToString( order[ i ].first ) + "  " + order[ i ].second;
        if ( GUCEF_NULL != examples )
        {
            TExampleMap::const_iterator example = examples->find( order[ i ].second );
            if ( example != examples->end() )
                text += "   (e.g. " + (*example).second + ")";
        }
        text += "\n";
    }
    return text;
}

/*-------------------------------------------------------------------------*/

void
PerformUnrealPackTriage( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.Triage" );

    GUCEF_TESTFW_TESTCASE( "Triage: every package of the real packs of GUCEF_TEST_UNREAL_DIR through the importer, the results by status and failure class" )
    const CORE::CString folders = EnvironmentValue( "GUCEF_TEST_UNREAL_DIR" );
    if ( folders.IsNULLOrEmpty() )
        GUCEF_TESTFW_SKIP( "GUCEF_TEST_UNREAL_DIR is not set: no pack folders to triage" );
    try
    {
        TAssetImporterPtr importer;
        ASSERT_TRUE( CAssetConverterGlobal::Instance()->GetPluginRegistry().TryGetImporter( "unreal", importer ) );
        const CORE::CString only = EnvironmentValue( "GUCEF_TEST_UNREAL_TRIAGE_ONLY" );
        const CORE::CString imagePlugin = EnvironmentValue( "GUCEF_TEST_UNREAL_IMAGE_PLUGIN" );
        bool checkPictures = false;
        if ( !imagePlugin.IsNULLOrEmpty() )
        {
            checkPictures = CORE::CCoreGlobal::Instance()->GetPluginControl().AddPluginFromDir( imagePlugin, "assetpluginUNREAL_TestApp", true );
            if ( !checkPictures )
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Triage: the image plugin " << imagePlugin << " cannot be loaded: the pictures are not decoded" );
        }

        GUCEF::vector< STriageResult > results;
        GUCEF::vector< SPackTally > tallies;
        CORE::CString::StringVector packNames;
        const CORE::CString::StringVector list = folders.ParseElements( ';', false );
        for ( size_t p=0; p<list.size(); ++p )
        {
            SPackTally tally;
            const size_t before = results.size();
            ASSERT_TRUE( TriagePack( list[ p ], (UInt32) p, importer, only, checkPictures, results, tally ) );
            tallies.push_back( tally );
            packNames.push_back( CORE::LastSubDir( list[ p ] ) + " (" + CORE::UInt64ToString( results.size() - before ) + " packages)" );
        }

        CORE::CString table;
        for ( size_t r=0; r<results.size(); ++r )
            table += results[ r ].pack + "\t" + ToString( results[ r ].status ) + "\t" + results[ r ].versions + "\t" + results[ r ].classes + "\t" + results[ r ].path + "\t" + results[ r ].reason + "\n";
        CORE::CString summary;
        for ( size_t p=0; p<tallies.size(); ++p )
        {
            const SPackTally& tally = tallies[ p ];
            summary += "== " + packNames[ p ] + "\n  ";
            for ( UInt32 s=0; s<ASSETSTATUS_COUNT; ++s )
                summary += ToString( (TAssetStatus) s ) + " " + CORE::UInt32ToString( tally.statusCounts[ s ] ) + ( s + 1 < ASSETSTATUS_COUNT ? ", " : "\n" );
            summary += "  versions:\n" + SortedCounts( tally.versionCounts, GUCEF_NULL, "    " );
            summary += "  failure classes:\n" + SortedCounts( tally.failureClasses, &tally.failureExamples, "    " );
            summary += "  skip classes:\n" + SortedCounts( tally.skipClasses, GUCEF_NULL, "    " );
            summary += "  classes of the top level exports:\n" + SortedCounts( tally.classCounts, GUCEF_NULL, "    " );
        }
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Triage of the real packs:\n" << summary );
        const CORE::CString outFile = EnvironmentValue( "GUCEF_TEST_UNREAL_TRIAGE_OUT" );
        if ( !outFile.IsNULLOrEmpty() )
        {
            ASSERT_TRUE( CORE::WriteStringAsTextFile( outFile, summary + "\n" + table ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
