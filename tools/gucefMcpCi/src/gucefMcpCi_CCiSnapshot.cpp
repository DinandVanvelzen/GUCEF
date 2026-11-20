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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiSnapshot.h"

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

CCiSnapshot::CCiSnapshot( void )
    : repoDir()
    , baseCommit()
    , commit()
    , branchName()
    , includesUncommitted( false )
    , filesInSnapshot( 0 )
    , modifiedFiles( 0 )
    , addedFiles( 0 )
    , untrackedFiles( 0 )
    , deletedFiles( 0 )
    , changedBytes( 0 )
    , hasSendEstimate( false )
    , sendBytesEstimate( 0 )
    , unpublishedCommits( 0 )
    , changedPaths()
    , changedPathsTruncated( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiSnapshot::Describe( void ) const
{GUCEF_TRACE;

    CORE::CString text = "Snapshot of " + repoDir + ": " + CORE::UInt32ToString( filesInSnapshot ) + " files, base " +
                         baseCommit.SubstrToIndex( 10, true ) + ", " + CORE::UInt32ToString( unpublishedCommits ) +
                         " unpublished commit(s)";
    if ( includesUncommitted )
    {
        text += "; uncommitted work: " + CORE::UInt32ToString( modifiedFiles ) + " modified, " + CORE::UInt32ToString( addedFiles ) +
                " added (" + CORE::UInt32ToString( untrackedFiles ) + " untracked), " + CORE::UInt32ToString( deletedFiles ) +
                " deleted, " + CCiUtils::FormatBytes( changedBytes ) + " in changed files";
    }
    else
    {
        text += "; uncommitted work left out";
    }
    if ( hasSendEstimate )
        text += "; about " + CCiUtils::FormatBytes( sendBytesEstimate ) + " to send at most";
    text += ".\n";
    if ( !changedPaths.empty() )
    {
        text += "Changes:\n";
        for ( size_t i=0; i<changedPaths.size(); ++i )
            text += "  " + changedPaths[ i ] + "\n";
        if ( changedPathsTruncated )
            text += "  ...\n";
    }
    return text;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
