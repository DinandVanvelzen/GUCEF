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

#ifndef ANCESTRYBACKUP_CJSONSHAPE_H
#include "AncestryBackup_CJsonShape.h"
#define ANCESTRYBACKUP_CJSONSHAPE_H
#endif /* ANCESTRYBACKUP_CJSONSHAPE_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include <string>
#include <vector>
#include <cstdlib>

#include "AncestryBackup_CDiscoveryJournal.h"

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

CDiscoveryJournal::CDiscoveryJournal( void )
    : m_file()
    , m_nextSequence( 1 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDiscoveryJournal::~CDiscoveryJournal()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryJournal::GetNextSequenceOf( const CORE::CString& path )
{GUCEF_TRACE;

    if ( !CORE::FileExists( path ) )
        return 1;

    // read the end of the file only, the journal gets large
    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return 1;
    const UInt64 size = file.GetSize();
    const UInt32 tail = (UInt32) ( size < 262144 ? size : 262144 );
    if ( tail == 0 )
        return 1;
    file.Seek( -(Int64) tail, SEEK_END );
    std::vector< char > buffer( tail );
    if ( file.Read( &buffer[ 0 ], 1, tail ) != tail )
        return 1;

    UInt32 highest = 0;
    const std::string text( buffer.begin(), buffer.end() );
    size_t pos = 0;
    while ( ( pos = text.find( "{\"seq\":", pos ) ) != std::string::npos )
    {
        const UInt32 value = (UInt32) strtoul( text.c_str() + pos + 7, GUCEF_NULL, 10 );
        if ( value > highest )
            highest = value;
        pos += 7;
    }
    return highest + 1;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryJournal::Open( const CDataLayout& layout )
{GUCEF_TRACE;

    Close();

    // continue the numbering of earlier runs: the sequence only goes up so the last line has the highest number
    m_nextSequence = GetNextSequenceOf( layout.GetDiscoveryJournalPath() );

    if ( !CORE::CreateDirs( layout.GetDiscoveryDir() ) )
        return false;
    return m_file.Open( layout.GetDiscoveryJournalPath(), "ab" );
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryJournal::Close( void )
{GUCEF_TRACE;

    m_file.Close();
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryJournal::IsOpen( void ) const
{GUCEF_TRACE;

    return m_file.Opened();
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryJournal::GetNextSequence( void ) const
{GUCEF_TRACE;

    return m_nextSequence;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryJournal::Append( const SObservedRequest& observation )
{GUCEF_TRACE;

    if ( !m_file.Opened() )
        return false;

    const std::string line = std::string( CEndpointCatalog::ToJournalLine( observation ).STL_String().c_str() ) + "\n";
    const bool ok = m_file.Write( line.c_str(), (UInt32) line.size(), 1 ) == 1;
    m_file.Flush();
    if ( observation.sequence >= m_nextSequence )
        m_nextSequence = observation.sequence + 1;
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Reads a whole file into a string. The framework's text file loader appends line by line and gets slower with every line, a journal takes minutes */
static bool
ReadWholeFile( const CORE::CString& path, std::string& content )
{
    content.clear();
    if ( !CORE::FileExists( path ) )
        return false;
    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return false;
    const UInt64 size = file.GetSize();
    content.resize( (size_t) size );
    size_t done = 0;
    while ( done < content.size() )
    {
        const UInt32 chunk = (UInt32) ( content.size() - done > 16 * 1024 * 1024 ? 16 * 1024 * 1024 : content.size() - done );
        if ( file.Read( &content[ done ], 1, chunk ) != chunk )
            return false;
        done += chunk;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryJournal::Load( const CDataLayout& layout, CEndpointCatalog& catalog, UInt32* nextSequence )
{GUCEF_TRACE;

    UInt32 loaded = 0;
    UInt32 maxSequence = 0;

    std::string text;
    if ( ReadWholeFile( layout.GetDiscoveryJournalPath(), text ) )
    {
        size_t start = 0;
        while ( start < text.size() )
        {
            size_t end = text.find( '\n', start );
            if ( end == std::string::npos )
                end = text.size();
            const std::string line = text.substr( start, end - start );
            start = end + 1;
            if ( line.empty() )
                continue;

            CORE::CDataNode document;
            SObservedRequest observation;
            if ( !COM::CJsonRpcEndpoint::ParseJsonText( CORE::CString( line.c_str() ), document ) ||
                 !CEndpointCatalog::FromJournalDocument( document, observation ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AncestryBackup: Skipping a journal line that cannot be read" );
                continue;
            }

            SJsonShape shape;
            bool haveShape = false;
            if ( catalog.NeedsShape( observation ) )
            {
                std::string body;
                const CORE::CString bodyPath = CORE::CombinePath( layout.GetDiscoveryDir(), observation.bodyFile.c_str() );
                CORE::CDataNode bodyDocument;
                if ( ReadWholeFile( bodyPath, body ) && COM::CJsonRpcEndpoint::ParseJsonText( CORE::CString( body.c_str() ), bodyDocument ) )
                {
                    CJsonShape::Describe( bodyDocument, shape );
                    haveShape = true;
                }
            }
            catalog.Add( observation, haveShape ? &shape : GUCEF_NULL );
            if ( !observation.bodyFile.empty() )
                catalog.MarkShapeTried( observation );
            ++loaded;
            if ( observation.sequence > maxSequence )
                maxSequence = observation.sequence;
        }
    }

    if ( GUCEF_NULL != nextSequence )
        *nextSequence = maxSequence + 1;
    return loaded;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
