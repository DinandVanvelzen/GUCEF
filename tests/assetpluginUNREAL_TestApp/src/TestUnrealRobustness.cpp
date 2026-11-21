/*
 *  assetpluginUNREAL_TestApp: Tests of the Unreal reader on truncated and damaged packages
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

#include <string.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_CORE_GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#include "assetpluginUNREAL_CUnrealPackage.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#include "assetpluginUNREAL_CUnrealPropertyReader.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPROPERTYREADER_H ? */

#ifndef GUCEF_CORE_TESTUNREALSUPPORT_H
#include "TestUnrealSupport.h"
#define GUCEF_CORE_TESTUNREALSUPPORT_H
#endif /* GUCEF_CORE_TESTUNREALSUPPORT_H ? */

#ifndef GUCEF_CORE_TESTUNREALWRITER_H
#include "TestUnrealWriter.h"
#define GUCEF_CORE_TESTUNREALWRITER_H
#endif /* GUCEF_CORE_TESTUNREALWRITER_H ? */

#ifndef GUCEF_CORE_TESTUNREALPROPERTIES_H
#include "TestUnrealProperties.h"
#define GUCEF_CORE_TESTUNREALPROPERTIES_H
#endif /* GUCEF_CORE_TESTUNREALPROPERTIES_H ? */

#include "TestUnrealRobustness.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETPLUGIN::UNREAL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)

/*-------------------------------------------------------------------------*/

/** Outcome of reading one (damaged) package completely: the package layer, then the properties of every export */
struct SOutcome
{
    bool opened;
    bool errorHasText;
    UInt32 exportsRead;
    double milliseconds;
};

static SOutcome
ReadEverything( const UInt8* bytes, UInt32 size )
{
    SOutcome outcome;
    outcome.opened = false;
    outcome.errorHasText = true;
    outcome.exportsRead = 0;
    const UInt64 start = MT::PrecisionTickCount();
    {
        CUnrealPackage package;
        outcome.opened = package.OpenMemory( bytes, size, "damaged", CUnrealReaderLimits() );
        if ( !outcome.opened )
            outcome.errorHasText = !package.GetError().IsNULLOrEmpty();
        else
        {
            for ( size_t i=0; i<package.GetExports().size(); ++i )
            {
                CUnrealArchiveReader reader;
                if ( !package.OpenPropertyRange( (UInt32) i, reader ) )
                    continue;
                CORE::CDataNode root( "root" );
                CUnrealPropertyReader properties( package );
                properties.ReadExport( reader, root, package.GetClassName( package.GetExports()[ i ] ) );
                ++outcome.exportsRead;
            }
            // the paths of every export, with the outer chains bounded
            for ( size_t i=0; i<package.GetExports().size(); ++i )
                package.GetExportPath( (UInt32) i );
            for ( size_t i=0; i<package.GetImports().size(); ++i )
                package.GetImportPath( (UInt32) i );
        }
    }
    outcome.milliseconds = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
    return outcome;
}

/*-------------------------------------------------------------------------*/

static UInt32 g_seed = 12345;

static UInt32
NextRandom( void )
{
    g_seed = g_seed * 1664525U + 1013904223U;
    return g_seed >> 8;
}

/*-------------------------------------------------------------------------*/

void
PerformUnrealRobustnessTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "assetpluginUNREAL.Robustness" );

    struct SVersion { Int32 legacy; Int32 ue4; Int32 ue5; const char* name; };
    const SVersion versions[] = { { -7, 508, 0, "4.14" }, { -8, 522, 1009, "5.3" }, { -9, 522, 1018, "5.8" } };

    GUCEF_TESTFW_TESTCASE( "Truncation: a package cut at every length and at every boundary is read without a crash and with a clear error" )
    try
    {
        for ( UInt32 v=0; v<3; ++v )
        {
            GUCEF::vector< UInt8 > bytes;
            BuildRichPackage( versions[ v ].legacy, versions[ v ].ue4, versions[ v ].ue5, bytes );
            CUnrealPackage whole;
            ASSERT_TRUE( whole.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "whole", CUnrealReaderLimits() ) );
            const UInt32 headerSize = (UInt32) whole.GetSummary().totalHeaderSize;
            UInt32 cases = 0;
            UInt32 opened = 0;
            double slowest = 0;
            UInt32 silentFailures = 0;
            // every length inside the header, then every length near the boundaries of the exports and the trailer, then a stride
            for ( UInt32 length=0; length<bytes.size(); ++length )
            {
                const bool inHeader = length <= headerSize + 4;
                bool boundary = false;
                for ( size_t e=0; e<whole.GetExports().size() && !boundary; ++e )
                {
                    const UInt32 start = (UInt32) whole.GetExports()[ e ].serialOffset;
                    const UInt32 end = start + (UInt32) whole.GetExports()[ e ].serialSize;
                    boundary = ( length + 8 >= start && length <= start + 8 ) || ( length + 8 >= end && length <= end + 8 );
                }
                if ( whole.HasTrailer() )
                {
                    const UInt32 trailer = (UInt32) whole.GetTrailer().GetTrailerOffset();
                    boundary = boundary || ( length + 8 >= trailer && length <= trailer + (UInt32) whole.GetTrailer().GetHeaderLength() + 8 ) || length + 24 >= bytes.size();
                }
                if ( !inHeader && !boundary && 0 != length % 97 )
                    continue;
                const SOutcome outcome = ReadEverything( length > 0 ? &bytes[ 0 ] : GUCEF_NULL, length );
                ++cases;
                if ( outcome.opened ) ++opened;
                if ( !outcome.errorHasText ) ++silentFailures;
                if ( outcome.milliseconds > slowest ) slowest = outcome.milliseconds;
            }
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Truncation " << versions[ v ].name << ": " << cases << " cases, " << opened << " opened, slowest " << slowest << " ms" );
            ASSERT_TRUE( cases > 200 );
            ASSERT_TRUE( 0 == silentFailures );
            ASSERT_TRUE( slowest < 100.0 );
            // a package cut inside its header cannot be opened
            const SOutcome cut = ReadEverything( &bytes[ 0 ], headerSize - 1 );
            ASSERT_TRUE( !cut.opened && cut.errorHasText );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Corruption: every byte of the header replaced, every 32 bit value set to an extreme, random bytes anywhere" )
    try
    {
        const UInt8 replacements[] = { 0x00, 0xFF, 0x7F, 0x80 };
        const UInt32 extremes[] = { 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x40000000, 0x00FFFFFF, 0x0000FFFF };
        for ( UInt32 v=0; v<3; ++v )
        {
            GUCEF::vector< UInt8 > good;
            BuildRichPackage( versions[ v ].legacy, versions[ v ].ue4, versions[ v ].ue5, good );
            CUnrealPackage whole;
            ASSERT_TRUE( whole.OpenMemory( &good[ 0 ], (UInt32) good.size(), "whole", CUnrealReaderLimits() ) );
            const UInt32 headerSize = (UInt32) whole.GetSummary().totalHeaderSize;
            UInt32 cases = 0;
            double slowest = 0;
            UInt32 silentFailures = 0;
            for ( UInt32 at=0; at<headerSize; ++at )
            {
                for ( UInt32 r=0; r<4; ++r )
                {
                    GUCEF::vector< UInt8 > bytes = good;
                    bytes[ at ] = replacements[ r ];
                    const SOutcome outcome = ReadEverything( &bytes[ 0 ], (UInt32) bytes.size() );
                    ++cases;
                    if ( !outcome.errorHasText ) ++silentFailures;
                    if ( outcome.milliseconds > slowest ) slowest = outcome.milliseconds;
                }
                if ( at + 4 <= headerSize )
                {
                    for ( UInt32 x=0; x<6; ++x )
                    {
                        GUCEF::vector< UInt8 > bytes = good;
                        for ( UInt32 k=0; k<4; ++k )
                            bytes[ at + k ] = (UInt8) ( ( extremes[ x ] >> ( 8 * k ) ) & 0xFF );
                        const SOutcome outcome = ReadEverything( &bytes[ 0 ], (UInt32) bytes.size() );
                        ++cases;
                        if ( !outcome.errorHasText ) ++silentFailures;
                        if ( outcome.milliseconds > slowest ) slowest = outcome.milliseconds;
                    }
                }
            }
            // random damage anywhere, the property data and the tables of the exports included
            for ( UInt32 i=0; i<3000; ++i )
            {
                GUCEF::vector< UInt8 > bytes = good;
                const UInt32 hits = 1 + NextRandom() % 4;
                for ( UInt32 h=0; h<hits; ++h )
                    bytes[ NextRandom() % bytes.size() ] = (UInt8) NextRandom();
                const SOutcome outcome = ReadEverything( &bytes[ 0 ], (UInt32) bytes.size() );
                ++cases;
                if ( !outcome.errorHasText ) ++silentFailures;
                if ( outcome.milliseconds > slowest ) slowest = outcome.milliseconds;
            }
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Corruption " << versions[ v ].name << ": " << cases << " cases, slowest " << slowest << " ms" );
            ASSERT_TRUE( cases > 5000 );
            ASSERT_TRUE( 0 == silentFailures );
            ASSERT_TRUE( slowest < 100.0 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Counts: tables with absurd counts are refused before any memory is reserved" )
    try
    {
        GUCEF::vector< UInt8 > good;
        BuildRichPackage( -9, 522, 1018, good );
        CUnrealPackage whole;
        ASSERT_TRUE( whole.OpenMemory( &good[ 0 ], (UInt32) good.size(), "whole", CUnrealReaderLimits() ) );
        const CUnrealPackageSummary& summary = whole.GetSummary();
        // find the counts in the summary: they are consecutive with the offsets, search the 32 bit values
        UInt32 patched = 0;
        const Int32 counts[] = { summary.nameCount, summary.exportCount, summary.importCount, summary.softObjectPathsCount };
        const Int32 offsets[] = { summary.nameOffset, summary.exportOffset, summary.importOffset, summary.softObjectPathsOffset };
        for ( UInt32 c=0; c<4; ++c )
        {
            for ( UInt32 at=0; at+8<=summary.summarySize; at+=1 )
            {
                Int32 value = 0;
                memcpy( &value, &good[ at ], 4 );
                Int32 offset = 0;
                memcpy( &offset, &good[ at + 4 ], 4 );
                if ( value != counts[ c ] || offset != offsets[ c ] )
                    continue;
                GUCEF::vector< UInt8 > bytes = good;
                const Int32 huge = 0x7FFFFFF0;
                memcpy( &bytes[ at ], &huge, 4 );
                const UInt64 start = MT::PrecisionTickCount();
                CUnrealPackage package;
                const bool opened = package.OpenMemory( &bytes[ 0 ], (UInt32) bytes.size(), "huge", CUnrealReaderLimits() );
                const double ms = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
                ASSERT_TRUE( !opened && !package.GetError().IsNULLOrEmpty() && ms < 100.0 );
                ++patched;
                break;
            }
        }
        ASSERT_TRUE( 4 == patched );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

