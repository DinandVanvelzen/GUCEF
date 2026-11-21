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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiCaller.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* CCiCaller::ScopeBuild = "build";
const char* CCiCaller::ScopeDeploy = "deploy";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiCaller::CCiCaller( void )
    : machineId()
    , scopes()
{GUCEF_TRACE;

    scopes.insert( ScopeBuild );
}

/*-------------------------------------------------------------------------*/

CCiCaller::CCiCaller( const CORE::CString& machine, const CORE::CString& scopeList )
    : machineId( machine )
    , scopes()
{GUCEF_TRACE;

    CORE::CString unknown;
    scopes = ParseScopes( scopeList, unknown );
}

/*-------------------------------------------------------------------------*/

bool
CCiCaller::HasScope( const char* scope ) const
{GUCEF_TRACE;

    return scopes.find( scope ) != scopes.end();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiCaller::ListScopes( void ) const
{GUCEF_TRACE;

    CORE::CString::StringVector names;
    for ( CORE::CString::StringSet::const_iterator i = scopes.begin(); i != scopes.end(); ++i )
        names.push_back( *i );
    return CCiUtils::Join( names, "," );
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringSet
CCiCaller::ParseScopes( const CORE::CString& scopeList, CORE::CString& unknown )
{GUCEF_TRACE;

    CORE::CString::StringSet result;
    unknown.Clear();
    const CORE::CString::StringVector names = scopeList.ParseElements( ',', false );
    for ( size_t i=0; i<names.size(); ++i )
    {
        const CORE::CString name = CCiUtils::Trimmed( names[ i ] ).Lowercase();
        if ( name.IsNULLOrEmpty() )
            continue;
        if ( name == ScopeBuild || name == ScopeDeploy )
            result.insert( name );
        else
            unknown += ( unknown.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::CString( "," ) ) + name;
    }
    // every machine of the tokens file may build; deploy only when it is named
    result.insert( ScopeBuild );
    return result;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
