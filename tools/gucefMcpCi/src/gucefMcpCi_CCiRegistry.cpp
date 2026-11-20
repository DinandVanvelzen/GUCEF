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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The part of 'text' before 'at' and after it */
static void
SplitAt( const CORE::CString& text, UInt32 at, UInt32 separatorLength, CORE::CString& before, CORE::CString& after )
{
    before = text.SubstrToIndex( at, true );
    after = text.CutChars( at + separatorLength, true );
}

/*-------------------------------------------------------------------------*/

/** The path of a remote without a leading or trailing / and without .git */
static CORE::CString
CleanRemotePath( const CORE::CString& path )
{
    CORE::CString clean = path.ReplaceChar( '\\', '/' );
    while ( clean.StartsWith( "/" ) )
        clean = clean.CutChars( 1, true );
    while ( clean.EndsWith( "/" ) )
        clean = clean.CutChars( 1, false );
    if ( clean.Lowercase().EndsWith( ".git" ) )
        clean = clean.CutChars( 4, false );
    while ( clean.EndsWith( "/" ) )
        clean = clean.CutChars( 1, false );
    return clean;
}

/*-------------------------------------------------------------------------*/

/** The host of an authority: without user info and port, lower case */
static CORE::CString
HostOfAuthority( const CORE::CString& authority )
{
    CORE::CString host = authority;
    const Int32 at = host.HasChar( '@', false );
    if ( at >= 0 )
        host = host.CutChars( (UInt32) at + 1, true );
    if ( host.StartsWith( "[" ) )
    {
        const Int32 close = host.HasChar( ']', true );
        if ( close > 0 )
            host = host.SubstrToIndex( (UInt32) close + 1, true );
    }
    else
    {
        const Int32 colon = host.HasChar( ':', true );
        if ( colon >= 0 )
            host = host.SubstrToIndex( (UInt32) colon, true );
    }
    return host.Lowercase();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiRegistry::CCiRegistry( void )
    : m_definitions()
    , m_gateway()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCiRegistry::LoadFromJson( const CORE::CString& jsonText, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CDataNode root;
    if ( !CCiUtils::ParseJson( jsonText, root ) )
    {
        problem = "it is not JSON";
        return false;
    }
    CCiGatewayEndpoint gateway;
    const CORE::CDataNode* gatewayNode = root.FindChild( "gateway" );
    if ( GUCEF_NULL != gatewayNode && GUCEF_DATATYPE_OBJECT == gatewayNode->GetNodeType() && !gateway.LoadConfig( *gatewayNode, problem ) )
        return false;

    const CORE::CDataNode* list = root.FindChild( "ci" );
    if ( GUCEF_NULL == list )
        list = &root;

    TDefinitionVector definitions;
    for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
    {
        const CORE::CDataNode& entry = *(*i);
        if ( GUCEF_DATATYPE_OBJECT != entry.GetNodeType() || &entry == gatewayNode )
            continue;
        CCiDefinition definition;
        if ( !definition.LoadConfig( entry, problem ) )
            return false;
        for ( size_t d=0; d<definitions.size(); ++d )
        {
            if ( definitions[ d ].id == definition.id )
            {
                problem = "the CI id \"" + definition.id + "\" is defined twice";
                return false;
            }
        }
        definitions.push_back( definition );
    }
    m_definitions = definitions;
    m_gateway = gateway;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiRegistry::LoadFromFile( const CORE::CString& path, bool ownerOnly, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CString text;
    if ( ownerOnly )
    {
        CORE::CDynamicBuffer content;
        CORE::CString readProblem;
        if ( !CORE::ReadOwnerOnlyFile( path, content, readProblem ) )
        {
            problem = "\"" + path + "\" cannot be read: " + readProblem;
            return false;
        }
        text = CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
    }
    else
    if ( !CCiUtils::ReadFile( path, text ) )
    {
        problem = "\"" + path + "\" cannot be read";
        return false;
    }

    CORE::CString parseProblem;
    if ( !LoadFromJson( text, parseProblem ) )
    {
        problem = "\"" + path + "\": " + parseProblem;
        return false;
    }
    m_gateway.ResolveRelativeTo( CORE::StripFilename( CCiUtils::MakeAbsolute( path ) ) );
    return true;
}

/*-------------------------------------------------------------------------*/

const CCiDefinition*
CCiRegistry::FindById( const CORE::CString& ciId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_definitions.size(); ++i )
        if ( m_definitions[ i ].id == ciId )
            return &m_definitions[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CCiDefinition*
CCiRegistry::FindByRemotes( const CORE::CString::StringVector& remotes, CORE::CString& matchedRemote ) const
{GUCEF_TRACE;

    for ( size_t d=0; d<m_definitions.size(); ++d )
    {
        const CCiDefinition& definition = m_definitions[ d ];
        for ( size_t r=0; r<remotes.size(); ++r )
        {
            const CORE::CString normalized = NormalizeRemoteUrl( remotes[ r ] );
            if ( normalized.IsNULLOrEmpty() )
                continue;
            for ( size_t k=0; k<definition.normalizedRemotes.size(); ++k )
            {
                if ( definition.normalizedRemotes[ k ] == normalized )
                {
                    matchedRemote = remotes[ r ];
                    return &definition;
                }
            }
        }
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CCiRegistry::TDefinitionVector&
CCiRegistry::GetDefinitions( void ) const
{GUCEF_TRACE;

    return m_definitions;
}

/*-------------------------------------------------------------------------*/

bool
CCiRegistry::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_definitions.empty();
}

/*-------------------------------------------------------------------------*/

const CCiGatewayEndpoint&
CCiRegistry::GetGateway( void ) const
{GUCEF_TRACE;

    return m_gateway;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiRegistry::ListIds( void ) const
{GUCEF_TRACE;

    CORE::CString::StringVector ids;
    for ( size_t i=0; i<m_definitions.size(); ++i )
        ids.push_back( m_definitions[ i ].id );
    return CCiUtils::Join( ids, ", " );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiRegistry::NormalizeRemoteUrl( const CORE::CString& url )
{GUCEF_TRACE;

    const CORE::CString text = CCiUtils::Trimmed( url );
    if ( text.IsNULLOrEmpty() )
        return CORE::CString::Empty;

    const Int32 schemeEnd = text.HasSubstr( "://", true );
    if ( schemeEnd > 0 )
    {
        CORE::CString scheme;
        CORE::CString rest;
        SplitAt( text, (UInt32) schemeEnd, 3, scheme, rest );
        scheme = scheme.Lowercase();
        if ( scheme == "file" )
            return "file:/" + CleanRemotePath( rest );

        const Int32 slash = rest.HasChar( '/', true );
        CORE::CString authority = rest;
        CORE::CString path;
        if ( slash >= 0 )
            SplitAt( rest, (UInt32) slash, 1, authority, path );
        const CORE::CString host = HostOfAuthority( authority );
        if ( host.IsNULLOrEmpty() )
            return CORE::CString::Empty;
        return host + "/" + CleanRemotePath( path );
    }

    // scp style: [user@]host:path, where the part before the ':' holds no '/' and is not a drive letter
    const Int32 colon = text.HasChar( ':', true );
    const Int32 slash = text.HasChar( '/', true );
    const Int32 backslash = text.HasChar( '\\', true );
    if ( colon > 1 && ( slash < 0 || colon < slash ) && ( backslash < 0 || colon < backslash ) )
    {
        CORE::CString authority;
        CORE::CString path;
        SplitAt( text, (UInt32) colon, 1, authority, path );
        const CORE::CString host = HostOfAuthority( authority );
        if ( host.IsNULLOrEmpty() )
            return CORE::CString::Empty;
        return host + "/" + CleanRemotePath( path );
    }

    // a local path
    return "file:/" + CleanRemotePath( text );
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
