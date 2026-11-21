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
    , publishedUrl()
    , registryHost()
    , registryUser( "ci-bot" )
    , deployImages()
    , deployProblem()
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

    publishedUrl = Attribute( node, "publishedUrl" );
    registryHost = Attribute( node, "registryHost" );
    const CORE::CString user = Attribute( node, "registryUser" );
    if ( !user.IsNULLOrEmpty() )
        registryUser = user;

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

    // what may be deployed: strict, an entry with a deploy block that is not right is refused as a whole
    deployImages.clear();
    deployProblem.Clear();
    const CORE::CDataNode* deploy = node.FindChild( "deploy" );
    if ( GUCEF_NULL != deploy )
    {
        const CORE::CDataNode* images = deploy->FindChild( "images" );
        if ( GUCEF_DATATYPE_OBJECT != deploy->GetNodeType() || GUCEF_NULL == images || GUCEF_DATATYPE_ARRAY != images->GetNodeType() )
            deployProblem = "the entry \"" + id + "\" has a deploy block without an images array";
        if ( deployProblem.IsNULLOrEmpty() )
        {
            for ( CORE::CDataNode::const_iterator i = images->ConstBegin(); i != images->ConstEnd(); ++i )
            {
                CCiDeployImage image;
                if ( GUCEF_DATATYPE_OBJECT != (*i)->GetNodeType() )
                    deployProblem = "the deploy block of \"" + id + "\" has an image that is not an object";
                else
                if ( image.LoadConfig( *(*i), id, deployProblem ) && GUCEF_NULL != FindDeployImage( image.name ) )
                    deployProblem = "the deploy block of \"" + id + "\" names the image \"" + image.name + "\" twice";
                if ( !deployProblem.IsNULLOrEmpty() )
                    break;
                deployImages.push_back( image );
            }
        }
        if ( deployProblem.IsNULLOrEmpty() && deployImages.empty() )
            deployProblem = "the entry \"" + id + "\" has a deploy block without images";
        if ( deployProblem.IsNULLOrEmpty() && !registryHost.IsNULLOrEmpty() && ( registryHost.HasChar( '@' ) >= 0 || registryHost.HasChar( ' ' ) >= 0 ) )
            deployProblem = "the entry \"" + id + "\" has a registryHost that is not host:port";
        if ( !deployProblem.IsNULLOrEmpty() )
        {
            deployImages.clear();
            problem = deployProblem;
            return false;
        }
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
CCiDefinition::GetPublishedGitUrl( void ) const
{GUCEF_TRACE;

    if ( !publishedUrl.IsNULLOrEmpty() )
        return publishedUrl;
    return GetBaseUrl() + "/" + owner + "/" + repo + ".git";
}

/*-------------------------------------------------------------------------*/

/** The scheme of a URL with "://", empty when it has none */
static CORE::CString
SchemeOf( const CORE::CString& url )
{
    const Int32 end = url.HasSubstr( "://", true );
    return end > 0 ? url.SubstrToIndex( (UInt32) end + 3, true ).Lowercase() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiDefinition::GetRegistryUrl( void ) const
{GUCEF_TRACE;

    const CORE::CString ciScheme = SchemeOf( url ).IsNULLOrEmpty() ? CORE::CString( "http://" ) : SchemeOf( url );
    CORE::CString host = registryHost;
    if ( host.IsNULLOrEmpty() )
    {
        // the host of the web root, with the port of the registry of the lab
        CORE::CString rest = url.CutChars( SchemeOf( url ).Length(), true );
        const Int32 slash = rest.HasChar( '/', true );
        if ( slash >= 0 )
            rest = rest.SubstrToIndex( (UInt32) slash, true );
        const Int32 at = rest.HasChar( '@', false );
        if ( at >= 0 )
            rest = rest.CutChars( (UInt32) at + 1, true );
        if ( rest.StartsWith( "[" ) )
        {
            const Int32 close = rest.HasChar( ']', true );
            if ( close > 0 )
                rest = rest.SubstrToIndex( (UInt32) close + 1, true );
        }
        else
        {
            const Int32 colon = rest.HasChar( ':', true );
            if ( colon >= 0 )
                rest = rest.SubstrToIndex( (UInt32) colon, true );
        }
        host = rest + ":32001";
    }
    if ( SchemeOf( host ).IsNULLOrEmpty() )
        host = ciScheme + host;
    while ( host.EndsWith( "/" ) )
        host = host.CutChars( 1, false );
    return host;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiDefinition::GetRegistryHostForImages( void ) const
{GUCEF_TRACE;

    const CORE::CString registryUrl = GetRegistryUrl();
    return registryUrl.CutChars( SchemeOf( registryUrl ).Length(), true );
}

/*-------------------------------------------------------------------------*/

const CCiDeployImage*
CCiDefinition::FindDeployImage( const CORE::CString& imageName ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<deployImages.size(); ++i )
        if ( deployImages[ i ].name == imageName )
            return &deployImages[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiDefinition::ListDeployables( void ) const
{GUCEF_TRACE;

    if ( deployImages.empty() )
        return "none";
    CORE::CString::StringVector entries;
    for ( size_t i=0; i<deployImages.size(); ++i )
        entries.push_back( deployImages[ i ].name + " (" + deployImages[ i ].ListTargets() + ")" );
    return CCiUtils::Join( entries, ", " );
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
