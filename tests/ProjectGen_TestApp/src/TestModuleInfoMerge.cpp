/*
 *  ProjectGen_TestApp: GUCEF test application for the ProjectGen module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_PROJECTGEN_DATATYPES_H
#include "gucefProjectGen_DataTypes.h"
#define GUCEF_PROJECTGEN_DATATYPES_H
#endif /* GUCEF_PROJECTGEN_DATATYPES_H ? */

#ifndef GUCEF_PROJECTGEN_CMODULEINFOENTRY_H
#include "gucefProjectGen_CModuleInfoEntry.h"
#define GUCEF_PROJECTGEN_CMODULEINFOENTRY_H
#endif /* GUCEF_PROJECTGEN_CMODULEINFOENTRY_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestModuleInfoMerge.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PROJECTGEN;

/*-------------------------------------------------------------------------*/

/** a definition of a module for a platform, the build order is normally assigned when the project info is complete */
static CModuleInfoPtr
CreateModuleInfo( const CORE::CString& name, TModuleType moduleType, Int64 buildOrder )
{GUCEF_TRACE;

    CModuleInfoPtr info = CModuleInfo::CreateSharedObj();
    info->name = name;
    info->moduleType = moduleType;
    info->buildOrder = buildOrder;
    return info;
}

/*-------------------------------------------------------------------------*/

void
PerformModuleInfoMergeTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING ProjectGen module info merge TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "ModuleInfoMerge" );

    const CORE::CString android = "android";

    GUCEF_TESTFW_TESTCASE( "Test: A module defined for all platforms only is merged for a platform" )
        try
        {
            CModuleInfoEntryPtr entry = CModuleInfoEntry::CreateSharedObj();
            CModuleInfoPtr allInfo = CreateModuleInfo( "GenericLib", MODULETYPE_STATIC_LIBRARY, 3 );
            allInfo->AddNameOfDependency( "gucefCORE" );
            entry->SetModuleInfo( allInfo, KnownPlatforms::AllPlatforms );

            CModuleInfoPtr merged;
            ASSERT_TRUE( MergeModuleInfo( entry, android, merged ) );
            ASSERT_FALSE( merged.IsNULL() );
            if ( !merged.IsNULL() )
            {
                ASSERT_TRUE( merged->name == "GenericLib" );
                ASSERT_TRUE( MODULETYPE_STATIC_LIBRARY == merged->moduleType );
                ASSERT_TRUE( 3 == merged->buildOrder );
                ASSERT_TRUE( merged->HasDependencyWithName( "gucefCORE" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A module defined for the target platform only is merged" )
        try
        {
            CModuleInfoEntryPtr entry = CModuleInfoEntry::CreateSharedObj();
            entry->SetModuleInfo( CreateModuleInfo( "AndroidOnlyLib", MODULETYPE_SHARED_LIBRARY, 2 ), android );

            CModuleInfoPtr merged;
            ASSERT_TRUE( MergeModuleInfo( entry, android, merged ) );
            ASSERT_FALSE( merged.IsNULL() );
            if ( !merged.IsNULL() )
            {
                ASSERT_TRUE( merged->name == "AndroidOnlyLib" );
                ASSERT_TRUE( MODULETYPE_SHARED_LIBRARY == merged->moduleType );
            }

            // for another platform there is nothing to merge
            CModuleInfoPtr mergedForOther;
            ASSERT_FALSE( MergeModuleInfo( entry, "linux", mergedForOther ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A module defined for all and for the target platform is layered into one definition" )
        try
        {
            CModuleInfoEntryPtr entry = CModuleInfoEntry::CreateSharedObj();
            CModuleInfoPtr allInfo = CreateModuleInfo( "LayeredLib", MODULETYPE_SHARED_LIBRARY, 4 );
            allInfo->AddNameOfDependency( "gucefCORE" );
            entry->SetModuleInfo( allInfo, KnownPlatforms::AllPlatforms );
            CModuleInfoPtr androidInfo = CModuleInfo::CreateSharedObj();
            androidInfo->AddNameOfDependency( "log" );
            entry->SetModuleInfo( androidInfo, android );

            CModuleInfoPtr merged;
            ASSERT_TRUE( MergeModuleInfo( entry, android, merged ) );
            ASSERT_FALSE( merged.IsNULL() );
            if ( !merged.IsNULL() )
            {
                ASSERT_TRUE( merged->name == "LayeredLib" );
                ASSERT_TRUE( MODULETYPE_SHARED_LIBRARY == merged->moduleType );
                ASSERT_TRUE( merged->HasDependencyWithName( "gucefCORE" ) );
                ASSERT_TRUE( merged->HasDependencyWithName( "log" ) );
            }

            // the definition for all platforms itself is not altered by the merge
            ASSERT_FALSE( allInfo->HasDependencyWithName( "log" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A platform definition without a name takes the name of the module's named definition" )
        try
        {
            // like a library defined as a shared library for linux and windows, and as a static library for the web
            // without repeating its name there
            CModuleInfoEntryPtr entry = CModuleInfoEntry::CreateSharedObj();
            entry->SetModuleInfo( CreateModuleInfo( "NamedLib", MODULETYPE_SHARED_LIBRARY, 5 ), "linux" );
            entry->SetModuleInfo( CreateModuleInfo( "NamedLib", MODULETYPE_SHARED_LIBRARY, 5 ), "mswin" );
            entry->SetModuleInfo( CreateModuleInfo( CORE::CString::Empty, MODULETYPE_STATIC_LIBRARY, 5 ), "emscripten" );

            CModuleInfoPtr merged;
            ASSERT_TRUE( MergeModuleInfo( entry, "emscripten", merged ) );
            ASSERT_FALSE( merged.IsNULL() );
            if ( !merged.IsNULL() )
            {
                ASSERT_TRUE( merged->name == "NamedLib" );
                ASSERT_TRUE( MODULETYPE_STATIC_LIBRARY == merged->moduleType );
            }

            // the names per platform, from which the build files name the module, have it for every platform
            TModuleInfoPtrMap names;
            GetModuleInfoWithUniqueModuleNames( entry, names );
            ASSERT_TRUE( 3 == names.size() );
            for ( TModuleInfoPtrMap::iterator i=names.begin(); i!=names.end(); ++i )
                ASSERT_TRUE( (*i).second->name == "NamedLib" );

            // the definition itself is not altered
            ASSERT_TRUE( entry->FindModuleInfoForPlatform( "emscripten" )->name.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A module built on some platforms, as different kinds of library, is built for the target of all platforms" )
        try
        {
            // like zlib: a shared library on linux and windows, a static library on the web, nothing on android
            CModuleInfoEntryPtr entry = CModuleInfoEntry::CreateSharedObj();
            entry->SetModuleInfo( CreateModuleInfo( "MixedLib", MODULETYPE_SHARED_LIBRARY, 5 ), "linux" );
            entry->SetModuleInfo( CreateModuleInfo( "MixedLib", MODULETYPE_SHARED_LIBRARY, 5 ), "mswin" );
            entry->SetModuleInfo( CreateModuleInfo( CORE::CString::Empty, MODULETYPE_STATIC_LIBRARY, 5 ), "emscripten" );

            // the kinds differ, so there is no single kind for all platforms
            ASSERT_TRUE( MODULETYPE_UNDEFINED == entry->GetModuleType( KnownPlatforms::AllPlatforms ) );

            // but it is built: a target for all platforms has to include it
            ASSERT_TRUE( IsModuleBuiltForPlatform( entry, KnownPlatforms::AllPlatforms ) );
            ASSERT_TRUE( IsModuleBuiltForPlatform( entry, "emscripten" ) );
            ASSERT_FALSE( IsModuleBuiltForPlatform( entry, "android" ) );

            // headers only are not built
            CModuleInfoEntryPtr headers = CModuleInfoEntry::CreateSharedObj();
            headers->SetModuleInfo( CreateModuleInfo( "Headers", MODULETYPE_HEADER_INCLUDE_LOCATION, 1 ), KnownPlatforms::AllPlatforms );
            ASSERT_FALSE( IsModuleBuiltForPlatform( headers, KnownPlatforms::AllPlatforms ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A module without a definition or without a build order is not merged" )
        try
        {
            CModuleInfoEntryPtr empty = CModuleInfoEntry::CreateSharedObj();
            CModuleInfoPtr merged;
            ASSERT_FALSE( MergeModuleInfo( empty, android, merged ) );

            // without build order the definition does not count as a definition of a module on its own
            CModuleInfoEntryPtr noBuildOrder = CModuleInfoEntry::CreateSharedObj();
            noBuildOrder->SetModuleInfo( CreateModuleInfo( "Unordered", MODULETYPE_STATIC_LIBRARY, -1 ), KnownPlatforms::AllPlatforms );
            ASSERT_FALSE( MergeModuleInfo( noBuildOrder, android, merged ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: All modules are merged for a platform, generic and platform specific ones alike" )
        try
        {
            TStringToModuleInfoEntryPtrMap allInfo;

            CModuleInfoEntryPtr generic = CModuleInfoEntry::CreateSharedObj();
            generic->SetModuleInfo( CreateModuleInfo( "GenericLib", MODULETYPE_STATIC_LIBRARY, 1 ), KnownPlatforms::AllPlatforms );
            allInfo[ "GenericLib" ] = generic;

            CModuleInfoEntryPtr specific = CModuleInfoEntry::CreateSharedObj();
            specific->SetModuleInfo( CreateModuleInfo( "AndroidLib", MODULETYPE_SHARED_LIBRARY, 2 ), android );
            allInfo[ "AndroidLib" ] = specific;

            CModuleInfoEntryPtr windowsOnly = CModuleInfoEntry::CreateSharedObj();
            windowsOnly->SetModuleInfo( CreateModuleInfo( "WindowsLib", MODULETYPE_SHARED_LIBRARY, 3 ), "mswin" );
            allInfo[ "WindowsLib" ] = windowsOnly;

            TModuleInfoPtrVector mergedInfo;
            TModuleInfoEntryPairVector mergeLinks;
            ASSERT_TRUE( MergeAllModuleInfoForPlatform( allInfo, android, mergedInfo, mergeLinks ) );
            ASSERT_TRUE( 2 == mergedInfo.size() );
            ASSERT_TRUE( 2 == mergeLinks.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL ProjectGen module info merge TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
