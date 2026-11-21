/*
 *  assetpluginUNITY_TestApp: Test application for the Unity plugin of the asset converter (assetpluginUNITY)
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

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "AssetConverterLib_CAssetPluginRegistry.h"
#include "AssetConverterLib_CAssetConverterGlobal.h"
#include "AssetConverterLib_CAssetPipeline.h"
#include "AssetConverterLib_CPackIndexReader.h"
#include "AssetConverterLib_CPackIndexLocator.h"

#include "TestUnitySupport.h"
#include "TestUnityPackIndexCorpus.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETCONVERTER;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

CORE::CString
EnvironmentValue( const char* name )
{
    return CORE::ResolveVars( "$ENVVAR:" + CORE::CString( name ) + "$" );
}

/** The counts of the index as lines, for the golden file */
CORE::CString
DescribeCounts( const CPackIndexDocument& doc )
{
    CORE::CString text = "resources " + CORE::UInt32ToString( doc.counts.resources ) + "\nnoise " + CORE::UInt32ToString( doc.counts.noise ) +
                         "\nmetadata " + CORE::UInt32ToString( doc.counts.metadata ) + "\n";
    for ( size_t g=0; g<doc.groups.size(); ++g )
        text += "group " + doc.groups[ g ].id + " " + CORE::UInt32ToString( doc.groups[ g ].count ) + " subgroups " + CORE::UInt32ToString( (UInt32) doc.groups[ g ].subGroups.size() ) + "\n";
    return text;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUnityPackIndexCorpusTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNITY.PackIndexCorpus" );

    const CORE::CString enabled = EnvironmentValue( "GUCEF_TEST_ASSET_CORPUS" );
    const CORE::CString package = EnvironmentValue( "GUCEF_TEST_PACKINDEX_UNITYPACKAGE" );
    const CORE::CString goldenFolder = EnvironmentValue( "GUCEF_TEST_PACKINDEX_GOLDEN" );
    if ( enabled != "1" || package.IsNULLOrEmpty() || goldenFolder.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Pack index corpus: the test is off (GUCEF_TEST_ASSET_CORPUS, GUCEF_TEST_PACKINDEX_UNITYPACKAGE and GUCEF_TEST_PACKINDEX_GOLDEN)" );
        return;
    }

    GUCEF_TESTFW_TESTCASE( "Corpus: a package named by the environment is indexed once, grouped to the golden counts, and a second run does no work" )
    try
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        ASSERT_TRUE( vfs.AddRoot( CORE::StripFilename( package ), "PackIndexCorpusDir", false, false, "packindexcorpus" ) );
        MakeWorkFolder( "pidx_corpus" );
        CAssetPackManifest manifest;
        manifest.id = "corpus-pack";
        manifest.source = "/packindexcorpus/" + CORE::ExtractFilename( package );
        manifest.importer = "unity";
        manifest.intermediate = "/work/pidx_corpus/intermediate";

        CAssetPipelineOptions options;
        options.indexOnly = true;
        options.reindex = true;
        CAssetPipeline pipeline;
        CConversionReport report;
        const UInt64 start = MT::PrecisionTickCount();
        ASSERT_TRUE( pipeline.RunPack( manifest, options, report ) );
        const UInt32 totalMs = (UInt32) MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
        const CPackIndexRunResult first = pipeline.GetLastIndexResult();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Pack index corpus: " << first.ToSummaryText() << "; the whole run took " << totalMs << " ms" );
        ASSERT_TRUE( first.structureBuilt && first.written );

        CPackIndexReader reader;
        CPackIndexDocument doc;
        ASSERT_TRUE( CPackIndexReader::RESULT_OK == reader.Read( "work/pidx_corpus/intermediate", doc ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Pack index corpus: type " << doc.profile.packType << ", shards " << (UInt32) doc.shards.size() << "\n" << DescribeCounts( doc ) );
        ASSERT_TRUE( doc.counts.resources > 1000 );

        // a second run: the index stands
        options.reindex = false;
        ASSERT_TRUE( pipeline.RunPack( manifest, options, report ) );
        ASSERT_TRUE( pipeline.GetLastIndexResult().reused && 0 == pipeline.GetLastIndexResult().resourcesGrouped );

        CORE::CreateDirs( goldenFolder );
        const CORE::CString goldenPath = CORE::CombinePath( goldenFolder, "unity-package-counts.txt" );
        const CORE::CString counts = DescribeCounts( doc );
        if ( !CORE::FileExists( goldenPath ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Pack index corpus: the golden file " << goldenPath << " is made" );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( goldenPath, counts ) );
        }
        CORE::CString golden;
        ASSERT_TRUE( CORE::LoadTextFileAsString( goldenPath, golden ) );
        ASSERT_TRUE( golden == counts );

        // a look at what is in the groups, for people (not compared): the sub-groups and a few paths of each group
        CORE::CString samples;
        for ( size_t g=0; g<doc.groups.size(); ++g )
        {
            const CPackIndexGroup& group = doc.groups[ g ];
            samples += group.id + " (" + CORE::UInt32ToString( group.count ) + ")\n";
            for ( size_t s=0; s<group.subGroups.size(); ++s )
                samples += "    sub-group " + group.subGroups[ s ].id + " " + CORE::UInt32ToString( group.subGroups[ s ].count ) + "\n";
            UInt32 shown = 0;
            GUCEF::map< CORE::CString, UInt32 > formats;
            for ( size_t r=0; r<doc.resources.size(); ++r )
            {
                if ( doc.resources[ r ].group != group.id )
                    continue;
                ++formats[ doc.resources[ r ].format ];
                if ( shown++ < 6 )
                    samples += "    " + doc.resources[ r ].path + " | " + doc.resources[ r ].subGroup + " | " + doc.resources[ r ].groupRule + "\n";
            }
            GUCEF::map< CORE::CString, UInt32 >::const_iterator f = formats.begin();
            samples += "    formats:";
            while ( f != formats.end() )
            {
                samples += " " + (*f).first + "=" + CORE::UInt32ToString( (*f).second );
                ++f;
            }
            samples += "\n";
        }
        CORE::WriteStringAsTextFile( CORE::CombinePath( goldenFolder, "unity-package-samples.txt" ), samples );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
