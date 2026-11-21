/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiRegistry.h"
#include "gucefMcpCi_CCiDefinition.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiDefinition::CCiDefinition( void )
    : id()
    , description()
    , backend()
    , url()
    , owner()
    , repo()
    , scratchOwner()
    , scratchRepo()
    , scratchUrl()
    , workflow()
    , matrixDir( "projects/CI/matrix" )
    , reportsOwner()
    , retentionDays( 14 )
    , remotes()
    , normalizedRemotes()
    , defaultMode( "build" )
    , defaultToolchains()
    , defaultCppStandards()
    , defaultProducts()
    , optionToolchains()
    , optionCppStandards()
    , optionProducts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

static CORE::CString
Attribute( const CORE::CDataNode& node, const char* name )
{
    return CCiUtils::Trimmed( node.GetAttributeValue( name ).AsString() );
}

/*-------------------------------------------------------------------------*/

bool
CCiDefinition::LoadConfig( const CORE::CDataNode& node, CORE::CString& problem )
{GUCEF_TRACE;

    id = Attribute( node, "id" );
    description = Attribute( node, "description" );
    backend = Attribute( node, "backend" ).Lowercase();
    url = Attribute( node, "url" );
    owner = Attribute( node, "owner" );
    repo = Attribute( node, "repo" );
    scratchOwner = Attribute( node, "scratchOwner" );
    scratchRepo = Attribute( node, "scratchRepo" );
    scratchUrl = Attribute( node, "scratchUrl" );
    workflow = Attribute( node, "workflow" );
    const CORE::CString matrix = Attribute( node, "matrixDir" );
    if ( !matrix.IsNULLOrEmpty() )
        matrixDir = matrix;
    reportsOwner = Attribute( node, "reportsOwner" );
    retentionDays = node.GetAttributeValue( "retentionDays" ).AsUInt32( 14 );
    remotes = CCiUtils::GetStringArray( node, "remotes" );

    const CORE::CDataNode* defaults = node.FindChild( "defaults" );
    if ( GUCEF_NULL != defaults )
    {
        const CORE::CString mode = Attribute( *defaults, "mode" );
        if ( !mode.IsNULLOrEmpty() )
            defaultMode = mode.Lowercase();
        defaultToolchains = CCiUtils::GetStringArray( *defaults, "toolchains" );
        defaultCppStandards = CCiUtils::GetStringArray( *defaults, "cppStandards" );
        defaultProducts = CCiUtils::GetStringArray( *defaults, "products" );
    }
    const CORE::CDataNode* options = node.FindChild( "options" );
    if ( GUCEF_NULL != options )
    {
        optionToolchains = CCiUtils::GetStringArray( *options, "toolchains" );
        optionCppStandards = CCiUtils::GetStringArray( *options, "cppStandards" );
        optionProducts = CCiUtils::GetStringArray( *options, "products" );
    }

    if ( backend.IsNULLOrEmpty() )
        backend = "forgejo";
    if ( scratchOwner.IsNULLOrEmpty() )
        scratchOwner = owner;
    if ( scratchRepo.IsNULLOrEmpty() )
        scratchRepo = repo;

    const char* missing = GUCEF_NULL;
    if ( id.IsNULLOrEmpty() )
        missing = "id";
    else
    if ( url.IsNULLOrEmpty() )
        missing = "url";
    else
    if ( scratchOwner.IsNULLOrEmpty() )
        missing = "scratchOwner";
    else
    if ( scratchRepo.IsNULLOrEmpty() )
        missing = "scratchRepo";
    else
    if ( workflow.IsNULLOrEmpty() )
        missing = "workflow";
    else
    if ( remotes.empty() )
        missing = "remotes";
    if ( GUCEF_NULL != missing )
    {
        problem = "the entry " + ( id.IsNULLOrEmpty() ? CORE::CString( "without an id" ) : "\"" + id + "\"" ) + " has no " + missing;
        return false;
    }
    if ( defaultMode != "build" && defaultMode != "test" )
    {
        problem = "the entry \"" + id + "\" has the default mode \"" + defaultMode + "\"; build or test";
        return false;
    }

    normalizedRemotes.clear();
    for ( size_t i=0; i<remotes.size(); ++i )
    {
        const CORE::CString normalized = CCiRegistry::NormalizeRemoteUrl( remotes[ i ] );
        if ( !normalized.IsNULLOrEmpty() )
            normalizedRemotes.push_back( normalized );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiDefinition::GetBaseUrl( void ) const
{GUCEF_TRACE;

    CORE::CString base = url;
    while ( base.EndsWith( "/" ) )
        base = base.CutChars( 1, false );
    return base;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiDefinition::GetScratchGitUrl( void ) const
{GUCEF_TRACE;

    if ( !scratchUrl.IsNULLOrEmpty() )
        return scratchUrl;
    return GetBaseUrl() + "/" + scratchOwner + "/" + scratchRepo + ".git";
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
