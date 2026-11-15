/*
 *  ProjectGen: GUCEF module providing the business logic to generate module/project files
 *
 *  Copyright (C) 1998 - 2025.  Dinand Vanvelzen
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

#include "gucefCORE_Logging.h"
#include "dvcppstringutils.h"
#include "gucefProjectGen_CProjectInfo.h"
#include "gucefProjectGen_DataTypes.h"

#include "gucefProjectGen_CCIMatrixGenerator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PROJECTGEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/*-------------------------------------------------------------------------*/

/**
 *  One entry of cihelpergen:Matrix:Toolchains: a compiler/platform/runner combination
 */
struct SMatrixToolchain
{
    CORE::CString id;
    CORE::CString platform;     // ProjectGen platform name the toolchain builds, for example linux64
    CORE::CString compiler;     // for example gcc, clang, msvc
    CORE::CString arch;         // for example x64, arm64
    CORE::CString runner;       // runs-on label of the CI runner that has this toolchain
    CORE::CString cmakeArgs;    // extra CMake configure arguments, shell expanded by the CI cell script
    UInt32 minStandardYear;     // lowest C++ standard year the toolchain is asked to build, 0: no limit
    UInt32 maxStandardYear;     // highest C++ standard year, 0: no limit
    bool enabled;               // cells of disabled toolchains are not generated
};

typedef std::vector< SMatrixToolchain > TMatrixToolchainVector;

/*-------------------------------------------------------------------------*/

CORE::CString
JsonString( const CORE::CString& value )
{GUCEF_TRACE;

    CORE::CString out = "\"";
    for ( UInt32 i=0; i<value.Length(); ++i )
    {
        char c = value[ i ];
        switch ( c )
        {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:   out += c; break;
        }
    }
    out += '"';
    return out;
}

/*-------------------------------------------------------------------------*/

CORE::CString
Trimmed( const CORE::CString& value )
{GUCEF_TRACE;

    return value.Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

TStringVector
ParseList( const CORE::CString& list )
{GUCEF_TRACE;

    TStringVector result;
    TStringVector elements = list.ParseElements( ',', false );
    for ( size_t i=0; i<elements.size(); ++i )
    {
        CORE::CString element = Trimmed( elements[ i ] );
        if ( !element.IsNULLOrEmpty() )
            result.push_back( element );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
ToolchainParam( const CORE::CValueList& params ,
                const CORE::CString& id        ,
                const char* key                ,
                const CORE::CString& defaultValue )
{GUCEF_TRACE;

    return params.GetValueAlways( "cihelpergen:Matrix:Toolchain:" + id + ':' + key ).AsString( defaultValue, true );
}

/*-------------------------------------------------------------------------*/

void
LoadToolchains( const CORE::CValueList& params, TMatrixToolchainVector& toolchains )
{GUCEF_TRACE;

    TStringVector ids = ParseList( params.GetValueAlways( "cihelpergen:Matrix:Toolchains" ).AsString( "gcc-linux-x64,clang-linux-x64", true ) );
    for ( size_t i=0; i<ids.size(); ++i )
    {
        SMatrixToolchain toolchain;
        toolchain.id = ids[ i ];
        toolchain.platform = ToolchainParam( params, toolchain.id, "Platform", "linux64" );
        toolchain.compiler = ToolchainParam( params, toolchain.id, "Compiler", CORE::CString::Empty );
        toolchain.arch = ToolchainParam( params, toolchain.id, "Arch", "x64" );
        toolchain.runner = ToolchainParam( params, toolchain.id, "Runner", "gucef-linux" );
        toolchain.cmakeArgs = ToolchainParam( params, toolchain.id, "CMakeArgs", CORE::CString::Empty );
        toolchain.minStandardYear = CppStandardToYear( ToolchainParam( params, toolchain.id, "MinCppStandard", CORE::CString::Empty ) );
        toolchain.maxStandardYear = CppStandardToYear( ToolchainParam( params, toolchain.id, "MaxCppStandard", CORE::CString::Empty ) );
        toolchain.enabled = CORE::StringToBool( ToolchainParam( params, toolchain.id, "Enabled", "true" ), true );
        toolchains.push_back( toolchain );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CellToJson( const SMatrixToolchain& toolchain, UInt32 standardYear, const CORE::CString& buildType )
{GUCEF_TRACE;

    CORE::CString standardName = CppStandardYearToName( standardYear );
    return "{ \"id\": " + JsonString( toolchain.id + "-cxx" + standardName + '-' + buildType ) +
           ", \"toolchain\": " + JsonString( toolchain.id ) +
           ", \"platform\": " + JsonString( toolchain.platform ) +
           ", \"compiler\": " + JsonString( toolchain.compiler ) +
           ", \"arch\": " + JsonString( toolchain.arch ) +
           ", \"runner\": " + JsonString( toolchain.runner ) +
           ", \"cppStandard\": " + JsonString( standardName ) +
           ", \"cppStandardYear\": " + CORE::ToString( standardYear ) +
           ", \"buildType\": " + JsonString( buildType ) +
           ", \"cmakeArgs\": " + JsonString( toolchain.cmakeArgs ) + " }";
}

/*-------------------------------------------------------------------------*/

bool
WriteMatrixJson( const TMatrixToolchainVector& toolchains ,
                 const TStringVector& standards           ,
                 const TStringVector& buildTypes          ,
                 const CORE::CString& matrixDir           )
{GUCEF_TRACE;

    CORE::CString json = "{\n  \"cppStandards\": [";
    for ( size_t i=0; i<standards.size(); ++i )
        json += ( i > 0 ? ", " : "" ) + JsonString( CppStandardYearToName( CppStandardToYear( standards[ i ] ) ) );
    json += "],\n  \"buildTypes\": [";
    for ( size_t i=0; i<buildTypes.size(); ++i )
        json += ( i > 0 ? ", " : "" ) + JsonString( buildTypes[ i ] );
    json += "],\n  \"toolchains\": [\n";

    for ( size_t i=0; i<toolchains.size(); ++i )
    {
        const SMatrixToolchain& t = toolchains[ i ];
        json += "    { \"id\": " + JsonString( t.id ) + ", \"platform\": " + JsonString( t.platform ) +
                ", \"compiler\": " + JsonString( t.compiler ) + ", \"arch\": " + JsonString( t.arch ) +
                ", \"runner\": " + JsonString( t.runner ) + ", \"enabled\": " + ( t.enabled ? "true" : "false" ) + " }";
        json += ( i + 1 < toolchains.size() ) ? ",\n" : "\n";
    }
    json += "  ],\n  \"cells\": [\n";

    bool firstCell = true;
    for ( size_t i=0; i<toolchains.size(); ++i )
    {
        const SMatrixToolchain& toolchain = toolchains[ i ];
        if ( !toolchain.enabled )
            continue;

        for ( size_t s=0; s<standards.size(); ++s )
        {
            UInt32 year = CppStandardToYear( standards[ s ] );
            if ( 0 == year )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GenerateCIMatrix: Ignoring unknown C++ standard: " + standards[ s ] );
                continue;
            }
            if ( ( 0 != toolchain.minStandardYear && year < toolchain.minStandardYear ) ||
                 ( 0 != toolchain.maxStandardYear && year > toolchain.maxStandardYear )  )
                continue;

            for ( size_t b=0; b<buildTypes.size(); ++b )
            {
                json += ( firstCell ? "    " : ",\n    " ) + CellToJson( toolchain, year, buildTypes[ b ] );
                firstCell = false;
            }
        }
    }
    json += "\n  ]\n}\n";

    CORE::CString path = CORE::CombinePath( matrixDir, "matrix.json" );
    if ( !CORE::WriteStringAsTextFile( path, json ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GenerateCIMatrix: Failed to write " + path );
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GenerateCIMatrix: Wrote " + path );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
WriteProductsJson( const CProjectInfo& projectInfo                  ,
                   const CProjectTargetInfoBundle& targets          ,
                   const CORE::CString::StringSet& platformFilter   ,
                   const CORE::CString& pathToTargetsOutputDir      ,
                   const CORE::CString& pathToCMakeTargetsOutputDir ,
                   const CORE::CString& matrixDir                   ,
                   const TStringVector& productPrefixes             ,
                   const TStringVector& excludedProducts            )
{GUCEF_TRACE;

    CORE::CString targetsDir = pathToTargetsOutputDir.ReplaceChar( '\\', '/' );
    CORE::CString cmakeDir = pathToCMakeTargetsOutputDir.ReplaceChar( '\\', '/' );

    CORE::CString json = "{\n  \"products\": [\n";
    bool first = true;
    CProjectTargetInfoBundle::TProjectTargetInfoPtrMapMap::const_iterator i = targets.GetAllTargets().begin();
    while ( i != targets.GetAllTargets().end() )
    {
        const CORE::CString& targetName = (*i).first;
        const CProjectTargetInfoBundle::TProjectTargetInfoPtrMap& targetsPerPlatform = (*i).second;

        // Only the configured kinds of targets are products: building every executable separately recompiles the shared modules for each
        bool isProduct = productPrefixes.empty();
        for ( size_t p=0; p<productPrefixes.size() && !isProduct; ++p )
            isProduct = 0 == targetName.HasSubstr( productPrefixes[ p ], true );
        for ( size_t p=0; p<excludedProducts.size() && isProduct; ++p )
            isProduct = targetName != excludedProducts[ p ];
        if ( !isProduct )
        {
            ++i;
            continue;
        }

        CProjectTargetInfoBundle::TProjectTargetInfoPtrMap::const_iterator n = targetsPerPlatform.begin();
        while ( n != targetsPerPlatform.end() )
        {
            const CORE::CString& platformName = (*n).first;
            if ( platformFilter.find( platformName ) != platformFilter.end() )
            {
                const CProjectTargetInfoPtr& targetInfo = (*n).second;

                // The target needs the highest minimum standard of any module it links
                UInt32 minYear = 0;
                TModuleInfoEntryPtrSet::const_iterator m = targetInfo->modules.begin();
                while ( m != targetInfo->modules.end() )
                {
                    UInt32 year = projectInfo.GetEffectiveMinCppStandardYear( (*m), platformName );
                    if ( year > minYear )
                        minYear = year;
                    ++m;
                }

                json += first ? "    " : ",\n    ";
                first = false;
                json += "{ \"name\": " + JsonString( targetName ) +
                        ", \"platform\": " + JsonString( platformName ) +
                        ", \"minCppStandard\": " + JsonString( CppStandardYearToName( minYear ) ) +
                        ", \"minCppStandardYear\": " + CORE::ToString( minYear ) +
                        ", \"project\": " + JsonString( cmakeDir + '/' + targetName ) +
                        ", \"globpaths\": " + JsonString( targetsDir + '/' + platformName + '/' + targetName + "/globpaths.txt" ) + " }";
            }
            ++n;
        }
        ++i;
    }
    json += "\n  ]\n}\n";

    CORE::CString path = CORE::CombinePath( matrixDir, "products.json" );
    if ( !CORE::WriteStringAsTextFile( path, json ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GenerateCIMatrix: Failed to write " + path );
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GenerateCIMatrix: Wrote " + path );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
GenerateNightlyWorkflow( const CORE::CValueList& params, const CORE::CString& pathToMatrixDir )
{GUCEF_TRACE;

    CORE::CString cron = params.GetValueAlways( "cihelpergen:Matrix:NightlyCron" ).AsString( "0 1 * * *", true );
    CORE::CString planRunner = params.GetValueAlways( "cihelpergen:Matrix:PlanRunner" ).AsString( "gucef-linux", true );
    CORE::CString checkout = params.GetValueAlways( "cihelpergen:ForgejoActionsCheckoutAction" ).AsString( "actions/checkout@v4", true );
    CORE::CString windowHours = params.GetValueAlways( "cihelpergen:Matrix:ChangeWindowHours" ).AsString( "25", true );
    CORE::CString dir = pathToMatrixDir.ReplaceChar( '\\', '/' );

    // Pushes (including commits fetched by a mirror sync) build what the push changed
    CORE::CString pushTrigger;
    TStringVector branches = ParseList( params.GetValueAlways( "cihelpergen:Matrix:PushBranches" ).AsString( "master,main", true ) );
    if ( !branches.empty() )
    {
        pushTrigger = "  push:\n    branches:\n";
        for ( size_t i=0; i<branches.size(); ++i )
            pushTrigger += "      - " + branches[ i ] + "\n";
    }

    CORE::CString yml =
        "# AUTO GENERATED BY PROJECTGENERATOR CI HELPER\n"
        "#\n"
        "# Builds every enabled toolchain x C++ standard x build type cell for the products impacted by the changes:\n# on pushes (also commits fetched by a mirror sync) and nightly.\n"
        "# The cells come from " + dir + "/matrix.json, the targets from " + dir + "/products.json.\n"
        "name: gucef-ci\n"
        "\n"
        "on:\n" +
        pushTrigger +
        "  schedule:\n"
        "    - cron: '" + cron + "'\n"
        "  workflow_dispatch:\n"
        "    inputs:\n"
        "      force:\n"
        "        description: 'Build everything regardless of changes? (true/false)'\n"
        "        required: false\n"
        "        default: 'false'\n"
        "      cppStandards:\n"
        "        description: 'Only these C++ standards, comma separated (empty: all)'\n"
        "        required: false\n"
        "        default: ''\n"
        "      toolchains:\n"
        "        description: 'Only these toolchains, comma separated (empty: all enabled)'\n"
        "        required: false\n"
        "        default: ''\n"
        "\n"
        "jobs:\n"
        "  plan:\n"
        "    runs-on: " + planRunner + "\n"
        "    outputs:\n"
        "      cells: ${{ steps.plan.outputs.cells }}\n"
        "      any: ${{ steps.plan.outputs.any }}\n"
        "    steps:\n"
        "      - uses: " + checkout + "\n"
        "        with:\n"
        "          fetch-depth: 0\n"
        "      - id: plan\n"
        "        env:\n"
        "          CI_BEFORE: ${{ github.event.before }}\n"
        "          CI_FORCE: ${{ inputs.force }}\n"
        "          CI_CPP_STANDARDS: ${{ inputs.cppStandards }}\n"
        "          CI_TOOLCHAINS: ${{ inputs.toolchains }}\n"
        "          CI_SINCE_HOURS: '" + windowHours + "'\n"
        "          CI_MATRIX_DIR: " + dir + "\n"
        "        run: python3 projects/CI/scripts/ci_plan.py\n"
        "\n"
        "  build:\n"
        "    needs: plan\n"
        "    if: needs.plan.outputs.any == 'true'\n"
        "    name: ${{ matrix.cell.id }}\n"
        "    runs-on: ${{ matrix.cell.runner }}\n"
        "    strategy:\n"
        "      fail-fast: false\n"
        "      matrix:\n"
        "        cell: ${{ fromJSON(needs.plan.outputs.cells) }}\n"
        "    steps:\n"
        "      - uses: " + checkout + "\n"
        "      - name: Build ${{ matrix.cell.id }}\n"
        "        env:\n"
        "          CELL_ID: ${{ matrix.cell.id }}\n"
        "          CELL_PLATFORM: ${{ matrix.cell.platform }}\n"
        "          CELL_COMPILER: ${{ matrix.cell.compiler }}\n"
        "          CELL_CPP_STANDARD: ${{ matrix.cell.cppStandard }}\n"
        "          CELL_BUILD_TYPE: ${{ matrix.cell.buildType }}\n"
        "          CELL_CMAKE_ARGS: ${{ matrix.cell.cmakeArgs }}\n"
        "          CELL_PRODUCTS: ${{ matrix.cell.products }}\n"
        "          CI_TOOL: FORGEJO-ACTIONS\n"
        "        run: bash projects/CI/scripts/ci_cell.sh\n";

    return yml;
}

/*-------------------------------------------------------------------------*/

} /* anonymous namespace */

/*-------------------------------------------------------------------------*/

bool
GenerateCIMatrix( const CProjectInfo& projectInfo                  ,
                  const CProjectTargetInfoBundle& targets          ,
                  const CORE::CString::StringSet& platformFilter   ,
                  const CORE::CString& pathToTargetsOutputDir      ,
                  const CORE::CString& pathToCMakeTargetsOutputDir ,
                  const CORE::CValueList& params                   )
{GUCEF_TRACE;

    if ( projectInfo.rootDirs.empty() )
        return false;

    CORE::CString matrixRelDir = params.GetValueAlways( "cihelpergen:Matrix:Dir" ).AsString( "projects/CI/matrix", true ).ReplaceChar( '\\', '/' );
    CORE::CString matrixDir = CORE::CombinePath( projectInfo.rootDirs[ 0 ], matrixRelDir );
    if ( !CORE::CreateDirs( matrixDir ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GenerateCIMatrix: Failed to create directories. Dir path: " + matrixDir );
        return false;
    }

    TMatrixToolchainVector toolchains;
    LoadToolchains( params, toolchains );
    TStringVector standards = ParseList( params.GetValueAlways( "cihelpergen:Matrix:CppStandards" ).AsString( "98,11,14,17,20,23", true ) );
    TStringVector buildTypes = ParseList( params.GetValueAlways( "cihelpergen:Matrix:BuildTypes" ).AsString( "Release", true ) );

    // The matrix covers more platforms than the per target workflows do, so it has its own filter
    CORE::CString::StringSet matrixPlatformFilter = params.GetValueAlways( "cihelpergen:Matrix:PlatformFilter" ).AsString( CORE::CString::Empty, true ).ParseUniqueElements( ',', false );
    if ( matrixPlatformFilter.empty() )
        matrixPlatformFilter = platformFilter;

    bool totalSuccess = WriteMatrixJson( toolchains, standards, buildTypes, matrixDir );
    TStringVector productPrefixes = ParseList( params.GetValueAlways( "cihelpergen:Matrix:ProductPrefixes" ).AsString( "GUCEF_tag_", true ) );
    TStringVector excludedProducts = ParseList( params.GetValueAlways( "cihelpergen:Matrix:ExcludedProducts" ).AsString( "GUCEF_tag_obsolete", true ) );
    totalSuccess = WriteProductsJson( projectInfo, targets, matrixPlatformFilter, pathToTargetsOutputDir, pathToCMakeTargetsOutputDir, matrixDir, productPrefixes, excludedProducts ) && totalSuccess;

    // The workflow is written into every project root, same as the per target workflows
    CORE::CString workflowsDir = params.GetValueAlways( "cihelpergen:ForgejoActionsWorkflowsDir" ).AsString( ".forgejo/workflows", true );
    CORE::CString workflow = GenerateNightlyWorkflow( params, matrixRelDir );
    TStringVector::const_iterator m = projectInfo.rootDirs.begin();
    while ( m != projectInfo.rootDirs.end() )
    {
        CORE::CString dir = CORE::CombinePath( (*m), workflowsDir ).ReplaceChar( GUCEF_DIRSEPCHAROPPOSITE, GUCEF_DIRSEPCHAR );
        CORE::CString path = CORE::CombinePath( dir, "gucef-ci.yml" );
        if ( CORE::CreateDirs( dir ) && CORE::WriteStringAsTextFile( path, workflow ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GenerateCIMatrix: Wrote " + path );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GenerateCIMatrix: Failed to write " + path );
            totalSuccess = false;
        }
        ++m;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PROJECTGEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
