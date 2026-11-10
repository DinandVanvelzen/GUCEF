/*
 *  vfspluginCUEBIN: Generic GUCEF VFS plugin for CD images described by a CUE sheet (BIN/CUE)
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

#include <stdio.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "vfspluginCUEBIN_CCueSheet.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace CUEBIN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< CORE::CString > TTokenVector;

/** The words of a line; a word in double quotes may hold spaces */
void
Tokenize( const char* line, UInt32 length, TTokenVector& tokens )
{
    tokens.clear();
    UInt32 i = 0;
    while ( i < length )
    {
        while ( i < length && ( ' ' == line[ i ] || '\t' == line[ i ] ) )
            ++i;
        if ( i >= length )
            break;

        if ( '"' == line[ i ] )
        {
            const UInt32 start = ++i;
            while ( i < length && '"' != line[ i ] )
                ++i;
            tokens.push_back( CORE::CString( line + start, i - start ) );
            if ( i < length )
                ++i;
        }
        else
        {
            const UInt32 start = i;
            while ( i < length && ' ' != line[ i ] && '\t' != line[ i ] )
                ++i;
            tokens.push_back( CORE::CString( line + start, i - start ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

CCueSheet::TTrackMode
ParseTrackMode( const CORE::CString& text )
{
    const CORE::CString mode = text.Uppercase();
    if ( "AUDIO" == mode )         return CCueSheet::TRACKMODE_AUDIO;
    if ( "MODE1/2048" == mode )    return CCueSheet::TRACKMODE_MODE1_2048;
    if ( "MODE1/2352" == mode )    return CCueSheet::TRACKMODE_MODE1_2352;
    if ( "MODE2/2336" == mode )    return CCueSheet::TRACKMODE_MODE2_2336;
    if ( "MODE2/2352" == mode )    return CCueSheet::TRACKMODE_MODE2_2352;
    return CCueSheet::TRACKMODE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCueSheet::STrack::STrack( void )
    : number( 0 )
    , mode( TRACKMODE_UNKNOWN )
    , fileIndex( 0 )
    , index0( -1 )
    , index1( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCueSheet::CCueSheet( void )
    : m_files()
    , m_tracks()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CCueSheet::ParseMsf( const CORE::CString& msf, UInt64& sectors )
{GUCEF_TRACE;

    unsigned int minutes = 0, seconds = 0, frames = 0;
    char end = 0;
    if ( 3 != sscanf( msf.C_String(), "%u:%u:%u%c", &minutes, &seconds, &frames, &end ) || seconds >= 60 || frames >= 75 )
        return false;
    sectors = ( (UInt64) minutes * 60 + seconds ) * 75 + frames;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCueSheet::Parse( const CORE::CString& text, CORE::CString& error )
{GUCEF_TRACE;

    m_files.clear();
    m_tracks.clear();
    error.Clear();

    const char* data = text.C_String();
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    UInt32 lineStart = 0;
    UInt32 lineNumber = 0;
    TTokenVector tokens;
    while ( lineStart < size )
    {
        UInt32 lineEnd = lineStart;
        while ( lineEnd < size && '\n' != data[ lineEnd ] && '\r' != data[ lineEnd ] )
            ++lineEnd;
        ++lineNumber;
        Tokenize( data + lineStart, lineEnd - lineStart, tokens );
        lineStart = lineEnd + 1;
        if ( tokens.empty() )
            continue;

        const CORE::CString command = tokens[ 0 ].Uppercase();
        if ( "FILE" == command )
        {
            if ( tokens.size() < 2 )
            {
                error = "line " + CORE::ToString( lineNumber ) + ": FILE without a name";
                return false;
            }
            SFile file;
            file.name = tokens[ 1 ];
            file.type = tokens.size() > 2 ? tokens[ 2 ].Uppercase() : CORE::CString( "BINARY" );
            m_files.push_back( file );
        }
        else
        if ( "TRACK" == command )
        {
            if ( m_files.empty() || tokens.size() < 3 )
            {
                error = "line " + CORE::ToString( lineNumber ) + ": TRACK before any FILE, or without a number and mode";
                return false;
            }
            STrack track;
            track.number = (UInt32) CORE::StringToInt32( tokens[ 1 ] );
            track.mode = ParseTrackMode( tokens[ 2 ] );
            track.fileIndex = (UInt32) m_files.size() - 1;
            if ( TRACKMODE_UNKNOWN == track.mode )
            {
                error = "line " + CORE::ToString( lineNumber ) + ": track mode " + tokens[ 2 ] + " is not supported";
                return false;
            }
            m_tracks.push_back( track );
        }
        else
        if ( "INDEX" == command )
        {
            UInt64 position = 0;
            if ( m_tracks.empty() || tokens.size() < 3 || !ParseMsf( tokens[ 2 ], position ) )
            {
                error = "line " + CORE::ToString( lineNumber ) + ": INDEX outside a track or with a bad position";
                return false;
            }
            const Int32 indexNumber = CORE::StringToInt32( tokens[ 1 ] );
            if ( 0 == indexNumber )
                m_tracks.back().index0 = (Int64) position;
            else
            if ( 1 == indexNumber )
                m_tracks.back().index1 = position;
            // higher indexes mark points inside a track, they do not change where it is
        }
        // REM, CATALOG, TITLE, PERFORMER, SONGWRITER, FLAGS, ISRC, PREGAP, POSTGAP, CDTEXTFILE: nothing to do with where the
        // sectors are; PREGAP and POSTGAP describe silence that is not in the files
    }

    if ( m_tracks.empty() )
    {
        error = "no tracks";
        return false;
    }
    for ( size_t f=0; f<m_files.size(); ++f )
    {
        if ( "BINARY" != m_files[ f ].type )
        {
            error = "file type " + m_files[ f ].type + " of " + m_files[ f ].name + " is not supported, only BINARY";
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CCueSheet::TFileVector&
CCueSheet::GetFiles( void ) const
{GUCEF_TRACE;

    return m_files;
}

/*-------------------------------------------------------------------------*/

const CCueSheet::TTrackVector&
CCueSheet::GetTracks( void ) const
{GUCEF_TRACE;

    return m_tracks;
}

/*-------------------------------------------------------------------------*/

UInt32
CCueSheet::GetSectorSize( TTrackMode mode )
{GUCEF_TRACE;

    switch ( mode )
    {
        case TRACKMODE_MODE1_2048: return 2048;
        case TRACKMODE_MODE2_2336: return 2336;
        case TRACKMODE_AUDIO:
        case TRACKMODE_MODE1_2352:
        case TRACKMODE_MODE2_2352: return 2352;
        default:                   return 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CCueSheet::GetPayloadOffset( TTrackMode mode )
{GUCEF_TRACE;

    switch ( mode )
    {
        case TRACKMODE_MODE1_2352: return 16;
        case TRACKMODE_MODE2_2336: return 8;
        case TRACKMODE_MODE2_2352: return 24;
        default:                   return 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CCueSheet::GetPayloadSize( TTrackMode mode )
{GUCEF_TRACE;

    return TRACKMODE_AUDIO == mode ? 2352 : ( TRACKMODE_UNKNOWN == mode ? 0 : 2048 );
}

/*-------------------------------------------------------------------------*/

bool
CCueSheet::IsDataTrack( TTrackMode mode )
{GUCEF_TRACE;

    return TRACKMODE_AUDIO != mode && TRACKMODE_UNKNOWN != mode;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CUEBIN */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
