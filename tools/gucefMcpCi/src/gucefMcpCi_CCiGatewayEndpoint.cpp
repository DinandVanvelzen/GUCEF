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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiGatewayEndpoint.h"

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

CCiGatewayEndpoint::CCiGatewayEndpoint( void )
    : url()
    , caFile()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCiGatewayEndpoint::LoadConfig( const CORE::CDataNode& node, CORE::CString& problem )
{GUCEF_TRACE;

    url = CCiUtils::Trimmed( node.GetAttributeValue( "url" ).AsString() );
    while ( url.EndsWith( "/" ) )
        url = url.CutChars( 1, false );
    caFile = CCiUtils::Trimmed( node.GetAttributeValue( "caFile" ).AsString() );
    if ( url.IsNULLOrEmpty() )
    {
        problem = "the gateway object has no url";
        return false;
    }
    CORE::CString urlProblem;
    if ( !IsAcceptableUrl( url, urlProblem ) )
    {
        problem = "the url of the gateway object cannot be used: " + urlProblem;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCiGatewayEndpoint::ResolveRelativeTo( const CORE::CString& baseDir )
{GUCEF_TRACE;

    if ( !caFile.IsNULLOrEmpty() && !CCiUtils::IsAbsolute( caFile ) && !baseDir.IsNULLOrEmpty() )
        caFile = CORE::CombinePath( baseDir, caFile );
}

/*-------------------------------------------------------------------------*/

bool
CCiGatewayEndpoint::IsSet( void ) const
{GUCEF_TRACE;

    return !url.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CCiGatewayEndpoint::IsAcceptableUrl( const CORE::CString& url, CORE::CString& problem )
{GUCEF_TRACE;

    const CORE::CString lower = url.Lowercase();
    if ( lower.StartsWith( "https://" ) && lower.Length() > 8 )
        return true;
    if ( lower.StartsWith( "http://" ) )
    {
        CORE::CString host = lower.CutChars( 7, true );
        const Int32 slash = host.HasChar( '/', true );
        if ( slash >= 0 )
            host = host.SubstrToIndex( (UInt32) slash, true );
        if ( host.StartsWith( "[::1]" ) )
            return true;
        const Int32 colon = host.HasChar( ':', true );
        if ( colon >= 0 )
            host = host.SubstrToIndex( (UInt32) colon, true );
        if ( host == "localhost" || host == "127.0.0.1" )
            return true;
        problem = "plain http is only used for a gateway on this machine (127.0.0.1, localhost); use https";
        return false;
    }
    problem = "\"" + url + "\" is not an https:// URL";
    return false;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
