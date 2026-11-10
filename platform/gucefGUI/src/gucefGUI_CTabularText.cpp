/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include <string.h>

#include "gucefGUI_CTabularText.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CTabularText::EndsInEmptyCell( const TTable& rows )
{GUCEF_TRACE;

    return !rows.empty() && 1 == rows.back().size() && rows.back()[ 0 ].IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CString
CTabularText::QuoteCell( const CString& text )
{GUCEF_TRACE;

    const char* chars = text.C_String();
    bool quote = false;
    for ( const char* c = chars; '\0' != *c; ++c )
    {
        if ( '\t' == *c || '\n' == *c || '\r' == *c || '"' == *c )
        {
            quote = true;
            break;
        }
    }
    if ( !quote )
        return text;
    CString quoted;
    quoted += '"';
    for ( const char* c = chars; '\0' != *c; ++c )
    {
        if ( '"' == *c )
            quoted += '"';
        quoted += *c;
    }
    quoted += '"';
    return quoted;
}

/*-------------------------------------------------------------------------*/

CString
CTabularText::FormatRow( const TRow& row )
{GUCEF_TRACE;

    CString text;
    for ( size_t i=0; i<row.size(); ++i )
    {
        if ( i > 0 )
            text += '\t';
        text += QuoteCell( row[ i ] );
    }
    return text;
}

/*-------------------------------------------------------------------------*/

CString
CTabularText::JoinLines( const GUCEF::vector< CString >& lines )
{GUCEF_TRACE;

    CString result;
    if ( lines.empty() )
        return result;
    size_t total = 0;
    for ( size_t i=0; i<lines.size(); ++i )
        total += strlen( lines[ i ].C_String() ) + ( i > 0 ? 1 : 0 );
    const bool lastIsEmpty = '\0' == lines.back().C_String()[ 0 ];
    if ( lastIsEmpty )
        ++total;
    if ( 0 == total )
        return result;
    char* buffer = result.Reserve( (UInt32) total + 1 );
    if ( GUCEF_NULL == buffer )
        return result;
    size_t offset = 0;
    for ( size_t i=0; i<lines.size(); ++i )
    {
        if ( i > 0 )
            buffer[ offset++ ] = '\n';
        const size_t size = strlen( lines[ i ].C_String() );
        memcpy( buffer + offset, lines[ i ].C_String(), size );
        offset += size;
    }
    if ( lastIsEmpty )
        buffer[ offset++ ] = '\n';
    buffer[ offset ] = '\0';
    result.DetermineLength();
    return result;
}

/*-------------------------------------------------------------------------*/

CString
CTabularText::Format( const TTable& rows )
{GUCEF_TRACE;

    GUCEF::vector< CString > lines;
    lines.reserve( rows.size() );
    for ( size_t r=0; r<rows.size(); ++r )
        lines.push_back( FormatRow( rows[ r ] ) );
    return JoinLines( lines );
}

/*-------------------------------------------------------------------------*/

bool
CTabularText::Parse( const CString& text, TTable& rows )
{GUCEF_TRACE;

    rows.clear();
    const char* chars = text.C_String();
    const size_t size = strlen( chars );
    if ( 0 == size )
        return true;

    bool closed = true;
    size_t i = 0;
    while ( i < size )
    {
        // one row
        rows.push_back( TRow() );
        TRow& row = rows.back();
        for (;;)
        {
            CString cell;
            if ( i < size && '"' == chars[ i ] )
            {
                // a quoted cell: up to the closing quote, a doubled quote is a quote
                ++i;
                bool done = false;
                while ( i < size && !done )
                {
                    if ( '"' == chars[ i ] )
                    {
                        if ( i + 1 < size && '"' == chars[ i + 1 ] )
                        {
                            cell += '"';
                            i += 2;
                        }
                        else
                        {
                            ++i;
                            done = true;
                        }
                    }
                    else
                        cell += chars[ i++ ];
                }
                if ( !done )
                    closed = false;

                // what follows the closing quote up to the end of the cell belongs to the cell
                while ( i < size && '\t' != chars[ i ] && '\n' != chars[ i ] && '\r' != chars[ i ] )
                    cell += chars[ i++ ];
            }
            else
            {
                while ( i < size && '\t' != chars[ i ] && '\n' != chars[ i ] && '\r' != chars[ i ] )
                    cell += chars[ i++ ];
            }
            row.push_back( cell );
            if ( i < size && '\t' == chars[ i ] )
            {
                ++i;
                if ( i >= size )
                {
                    // a tab at the very end: an empty cell follows
                    row.push_back( CString::Empty );
                    break;
                }
                continue;
            }
            break;
        }

        // the end of the row
        if ( i < size && '\r' == chars[ i ] )
        {
            ++i;
            if ( i < size && '\n' == chars[ i ] )
                ++i;
        }
        else
        if ( i < size && '\n' == chars[ i ] )
            ++i;
    }
    return closed;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
