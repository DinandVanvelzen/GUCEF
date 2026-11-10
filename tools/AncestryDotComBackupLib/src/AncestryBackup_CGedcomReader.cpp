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

#include "AncestryBackup_CGedcomReader.h"

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

static bool
IsSpace( char c ) { return c == ' ' || c == '\t'; }

/*-------------------------------------------------------------------------*/

UInt32
CGedcomReader::Parse( const char* data, size_t size, CIGedcomHandler& handler )
{GUCEF_TRACE;

    UInt32 accepted = 0;
    UInt32 lineNumber = 0;
    size_t pos = 0;

    // UTF-8 byte order mark
    if ( size >= 3 && (unsigned char) data[ 0 ] == 0xEF && (unsigned char) data[ 1 ] == 0xBB && (unsigned char) data[ 2 ] == 0xBF )
        pos = 3;

    while ( pos < size )
    {
        // the line is [pos, end)
        size_t end = pos;
        while ( end < size && data[ end ] != '\n' && data[ end ] != '\r' )
            ++end;

        ++lineNumber;
        size_t p = pos;
        // the next line starts after this line end, CRLF is one line end
        size_t next = end;
        if ( next < size && data[ next ] == '\r' && next + 1 < size && data[ next + 1 ] == '\n' )
            next += 2;
        else
        if ( next < size )
            next += 1;

        while ( p < end && IsSpace( data[ p ] ) )
            ++p;
        if ( p == end )
        {
            // an empty line is not a problem
            pos = next;
            continue;
        }

        SGedcomLine line;
        line.lineNumber = lineNumber;

        // level
        size_t levelStart = p;
        UInt32 level = 0;
        while ( p < end && data[ p ] >= '0' && data[ p ] <= '9' && p - levelStart < 3 )
        {
            level = level * 10 + (UInt32) ( data[ p ] - '0' );
            ++p;
        }
        if ( p == levelStart || ( p < end && !IsSpace( data[ p ] ) ) )
        {
            handler.OnMalformedLine( lineNumber );
            pos = next;
            continue;
        }
        line.level = level;
        while ( p < end && IsSpace( data[ p ] ) )
            ++p;

        // optional id
        if ( p < end && data[ p ] == '@' )
        {
            size_t idStart = p + 1;
            size_t idEnd = idStart;
            while ( idEnd < end && data[ idEnd ] != '@' )
                ++idEnd;
            if ( idEnd >= end )
            {
                handler.OnMalformedLine( lineNumber );
                pos = next;
                continue;
            }
            line.xref = data + idStart;
            line.xrefLength = (UInt32) ( idEnd - idStart );
            p = idEnd + 1;
            while ( p < end && IsSpace( data[ p ] ) )
                ++p;
        }

        // tag
        size_t tagStart = p;
        while ( p < end && !IsSpace( data[ p ] ) )
            ++p;
        if ( p == tagStart )
        {
            handler.OnMalformedLine( lineNumber );
            pos = next;
            continue;
        }
        line.tag = data + tagStart;
        line.tagLength = (UInt32) ( p - tagStart );

        // value: what is left, without the single space that separates it from the tag
        if ( p < end )
            ++p;
        if ( p < end )
        {
            line.value = data + p;
            line.valueLength = (UInt32) ( end - p );
        }

        handler.OnLine( line );
        ++accepted;
        pos = next;
    }
    return accepted;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
