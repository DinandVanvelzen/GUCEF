/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CGEDCOMREADER_H
#include "AncestryBackup_CGedcomReader.h"
#define ANCESTRYBACKUP_CGEDCOMREADER_H
#endif /* ANCESTRYBACKUP_CGEDCOMREADER_H ? */

#ifndef ANCESTRYBACKUP_CGEDCOMSUMMARY_H
#include "AncestryBackup_CGedcomSummary.h"
#define ANCESTRYBACKUP_CGEDCOMSUMMARY_H
#endif /* ANCESTRYBACKUP_CGEDCOMSUMMARY_H ? */

#ifndef ANCESTRYBACKUP_CZIPEXTRACTOR_H
#include "AncestryBackup_CZipExtractor.h"
#define ANCESTRYBACKUP_CZIPEXTRACTOR_H
#endif /* ANCESTRYBACKUP_CZIPEXTRACTOR_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include <algorithm>
#include <ctime>
#include <string>

#include "AncestryBackup_CExportFinder.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

std::vector< CORE::CString >
CExportFinder::GetDefaultDirs( void )
{GUCEF_TRACE;

    std::vector< CORE::CString > dirs;

    // the usual place first: the special directory lookup of the framework answers with the home directory on some platforms
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const CORE::CString home = CORE::GetEnv( "USERPROFILE" );
    #else
    const CORE::CString home = CORE::GetEnv( "HOME" );
    #endif
    if ( !home.IsNULLOrEmpty() && CORE::DirExists( CORE::CombinePath( home, "Downloads" ) ) )
    {
        dirs.push_back( CORE::CombinePath( home, "Downloads" ) );
        return dirs;
    }

    CORE::CString downloads;
    if ( CORE::TryResolveSpecialDir( CORE::SPECIALDIR_MY_DOWNLOADS_DIR, downloads ) && !downloads.IsNULLOrEmpty() )
        dirs.push_back( downloads );
    return dirs;
}

/*-------------------------------------------------------------------------*/

namespace {

/** Remembers only what the header says, and stops asking for more lines once the header is over */
class CHeaderReader : public CIGedcomHandler
{
    public:
    CHeaderReader( void ) : sourceSystem(), sawHeader( false ), headerOver( false ) {}
    virtual void OnLine( const SGedcomLine& line ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( line.level == 0 )
        {
            if ( line.tagLength == 4 && memcmp( line.tag, "HEAD", 4 ) == 0 )
                sawHeader = true;
            else
                headerOver = true;
            return;
        }
        if ( sawHeader && !headerOver && line.level == 1 && line.tagLength == 4 && memcmp( line.tag, "SOUR", 4 ) == 0 && line.value != GUCEF_NULL )
            sourceSystem.assign( line.value, line.valueLength );
    }
    virtual void OnMalformedLine( UInt32 lineNumber ) GUCEF_VIRTUAL_OVERRIDE {}
    std::string sourceSystem;
    bool sawHeader;
    bool headerOver;
};

}

/*-------------------------------------------------------------------------*/

bool
CExportFinder::LooksLikeAncestryExport( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return false;

    char head[ 16 * 1024 ];
    const UInt32 got = file.Read( head, 1, sizeof( head ) );
    if ( got == 0 )
        return false;

    CHeaderReader reader;
    CGedcomReader::Parse( head, got, reader );
    if ( !reader.sawHeader )
        return false;

    std::string system = reader.sourceSystem;
    for ( size_t i=0; i<system.size(); ++i )
        system[ i ] = (char) tolower( system[ i ] );
    return system.find( "ancestry" ) != std::string::npos;
}

/*-------------------------------------------------------------------------*/

static bool
EndsWithNoCase( const std::string& text, const char* suffix )
{
    const size_t n = strlen( suffix );
    if ( text.size() < n )
        return false;
    for ( size_t i=0; i<n; ++i )
        if ( tolower( text[ text.size() - n + i ] ) != tolower( suffix[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
NewerFirst( const SExportCandidate& a, const SExportCandidate& b )
{
    return a.modifiedAtInMs > b.modifiedAtInMs;
}

/*-------------------------------------------------------------------------*/

void
CExportFinder::FindRecent( const std::vector< CORE::CString >& dirs   ,
                           UInt64 nowInMs                              ,
                           UInt64 maxAgeInMs                           ,
                           std::vector< SExportCandidate >& candidates ,
                           std::vector< CORE::CString >* zipsSeen      )
{GUCEF_TRACE;

    candidates.clear();
    if ( GUCEF_NULL != zipsSeen )
        zipsSeen->clear();

    for ( size_t d=0; d<dirs.size(); ++d )
    {
        CORE::CFileSystemIterator iterator;
        if ( !iterator.FindFirst( dirs[ d ] ) )
            continue;
        do
        {
            if ( !iterator.IsAFile() )
                continue;

            const std::string name = iterator.GetResourceName().STL_String();
            const bool isGedcom = EndsWithNoCase( name, ".ged" ) || EndsWithNoCase( name, ".gedcom" );
            const bool isZip = EndsWithNoCase( name, ".zip" );
            if ( !isGedcom && !isZip )
                continue;

            const CORE::CString path = CORE::CombinePath( dirs[ d ], iterator.GetResourceName() );
            const UInt64 modified = CORE::GetFileModificationTime( path ).ToUnixEpochBasedTicksInMillisecs();
            if ( modified + maxAgeInMs < nowInMs )
                continue;

            if ( isZip )
            {
                std::string lower = name;
                for ( size_t i=0; i<lower.size(); ++i )
                    lower[ i ] = (char) tolower( lower[ i ] );
                if ( GUCEF_NULL != zipsSeen && ( lower.find( "ged" ) != std::string::npos || lower.find( "tree" ) != std::string::npos || lower.find( "ancestry" ) != std::string::npos ) )
                    zipsSeen->push_back( path );
                continue;
            }

            if ( !LooksLikeAncestryExport( path ) )
                continue;

            SExportCandidate candidate;
            candidate.path = path;
            candidate.modifiedAtInMs = modified;
            candidate.sizeInBytes = CORE::FileSize( path );
            candidates.push_back( candidate );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
    std::sort( candidates.begin(), candidates.end(), NewerFirst );
}

/*-------------------------------------------------------------------------*/

bool
CExportFinder::ZipHoldsAncestryExport( const CORE::CString& zipPath )
{GUCEF_TRACE;

    if ( !CZipExtractor::IsAvailable() )
        return false;

    const CORE::CString workDir = CORE::CombinePath( CORE::TempDir(), "AncestryDotComBackup_zipcheck" );
    CORE::CString extracted;
    if ( !CZipExtractor::ExtractFirst( zipPath, ".ged", workDir, extracted ) )
        return false;
    const bool yes = LooksLikeAncestryExport( extracted );
    CORE::DeleteFile( extracted );
    return yes;
}

/*-------------------------------------------------------------------------*/

void
CExportFinder::FindRecentZips( const std::vector< CORE::CString >& dirs, UInt64 nowInMs, UInt64 maxAgeInMs, std::vector< CORE::CString >& zips )
{GUCEF_TRACE;

    zips.clear();
    std::vector< SExportCandidate > found;
    for ( size_t d=0; d<dirs.size(); ++d )
    {
        CORE::CFileSystemIterator iterator;
        if ( !iterator.FindFirst( dirs[ d ] ) )
            continue;
        do
        {
            if ( !iterator.IsAFile() || !EndsWithNoCase( iterator.GetResourceName().STL_String(), ".zip" ) )
                continue;
            const CORE::CString path = CORE::CombinePath( dirs[ d ], iterator.GetResourceName() );
            const UInt64 modified = CORE::GetFileModificationTime( path ).ToUnixEpochBasedTicksInMillisecs();
            if ( modified + maxAgeInMs < nowInMs )
                continue;
            SExportCandidate candidate;
            candidate.path = path;
            candidate.modifiedAtInMs = modified;
            candidate.sizeInBytes = CORE::FileSize( path );
            found.push_back( candidate );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
    std::sort( found.begin(), found.end(), NewerFirst );

    // the first one that holds an export of Ancestry, which is told by its content
    for ( size_t i=0; i<found.size(); ++i )
        if ( ZipHoldsAncestryExport( found[ i ].path ) )
            zips.push_back( found[ i ].path );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CExportFinder::FormatTimestamp( UInt64 unixEpochInMs )
{GUCEF_TRACE;

    const time_t seconds = (time_t) ( unixEpochInMs / 1000 );
    struct tm parts;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    gmtime_s( &parts, &seconds );
    #else
    gmtime_r( &seconds, &parts );
    #endif
    char text[ 32 ];
    strftime( text, sizeof( text ), "%Y%m%d-%H%M%S", &parts );
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
