/*
 *  AssetStoreConverterHook: the converter pipeline as the conversion step of the primary asset store
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

#include "AssetStoreConverterHook_CTextLineDiff.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORECONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt32 MaxLinesForLcs = 2000;
const UInt32 Context = 3;

/** One line of the edit script: ' ' kept, '-' removed, '+' added, with the line numbers it has in both texts */
struct SEdit
{
    char kind;
    UInt32 before;
    UInt32 after;
};

typedef GUCEF::vector< SEdit > TEditVector;

void
AddEdit( TEditVector& edits, char kind, UInt32 before, UInt32 after )
{GUCEF_TRACE;

    SEdit edit;
    edit.kind = kind;
    edit.before = before;
    edit.after = after;
    edits.push_back( edit );
}

void
Emit( CORE::CString& out, const CORE::CString& line, UInt32 maxLines, UInt32& written, bool& truncated )
{GUCEF_TRACE;

    if ( written >= maxLines )
    {
        truncated = true;
        return;
    }
    out += line;
    out += '\n';
    ++written;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CTextLineDiff::SplitLines( const CORE::CString& text, CORE::CString::StringVector& lines )
{GUCEF_TRACE;

    lines.clear();
    CORE::CString current;
    for ( UInt32 i=0; i<text.Length(); ++i )
    {
        const char c = text[ i ];
        if ( '\n' == c )
        {
            lines.push_back( current );
            current.Clear();
            continue;
        }
        if ( '\r' == c )
            continue;
        current += c;
    }
    if ( !current.IsNULLOrEmpty() )
        lines.push_back( current );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTextLineDiff::Unified( const CORE::CString& before        ,
                        const CORE::CString& after         ,
                        const CORE::CString& beforeName    ,
                        const CORE::CString& afterName     ,
                        UInt32 maxLines                    ,
                        bool& truncated                    )
{GUCEF_TRACE;

    truncated = false;
    CORE::CString::StringVector a;
    CORE::CString::StringVector b;
    SplitLines( before, a );
    SplitLines( after, b );
    const UInt32 n = (UInt32) a.size();
    const UInt32 m = (UInt32) b.size();

    TEditVector edits;
    if ( n <= MaxLinesForLcs && m <= MaxLinesForLcs )
    {
        // the lengths of the longest common subsequences of the tails
        GUCEF::vector< UInt16 > table( ( n + 1 ) * ( m + 1 ), 0 );
        for ( UInt32 i=n; i>0; --i )
            for ( UInt32 j=m; j>0; --j )
                table[ ( i-1 ) * ( m+1 ) + ( j-1 ) ] = a[ i-1 ] == b[ j-1 ] ? (UInt16) ( table[ i * ( m+1 ) + j ] + 1 )
                                                                             : ( table[ i * ( m+1 ) + ( j-1 ) ] >= table[ ( i-1 ) * ( m+1 ) + j ] ? table[ i * ( m+1 ) + ( j-1 ) ] : table[ ( i-1 ) * ( m+1 ) + j ] );
        UInt32 i = 0;
        UInt32 j = 0;
        while ( i < n && j < m )
        {
            if ( a[ i ] == b[ j ] )
            {
                AddEdit( edits, ' ', i, j );
                ++i;
                ++j;
            }
            else
            if ( table[ ( i+1 ) * ( m+1 ) + j ] >= table[ i * ( m+1 ) + ( j+1 ) ] )
            {
                AddEdit( edits, '-', i, j );
                ++i;
            }
            else
            {
                AddEdit( edits, '+', i, j );
                ++j;
            }
        }
        for ( ; i<n; ++i )
            AddEdit( edits, '-', i, j );
        for ( ; j<m; ++j )
            AddEdit( edits, '+', i, j );
    }
    else
    {
        for ( UInt32 i=0; i<n; ++i )
            AddEdit( edits, '-', i, 0 );
        for ( UInt32 j=0; j<m; ++j )
            AddEdit( edits, '+', n, j );
    }

    CORE::CString out;
    UInt32 written = 0;
    bool any = false;
    for ( size_t e=0; e<edits.size(); ++e )
        if ( ' ' != edits[ e ].kind )
            any = true;
    if ( !any )
        return out;
    Emit( out, "--- " + beforeName, maxLines, written, truncated );
    Emit( out, "+++ " + afterName, maxLines, written, truncated );
    size_t e = 0;
    while ( e < edits.size() && !truncated )
    {
        // the next change, with its context
        while ( e < edits.size() && ' ' == edits[ e ].kind )
            ++e;
        if ( e >= edits.size() )
            break;
        size_t start = e >= Context ? e - Context : 0;
        size_t end = e;
        size_t keptRun = 0;
        while ( end < edits.size() )
        {
            if ( ' ' == edits[ end ].kind )
            {
                ++keptRun;
                if ( keptRun > 2 * Context )
                    break;
            }
            else
                keptRun = 0;
            ++end;
        }
        // the context after the last change of the hunk
        while ( end > start && ' ' == edits[ end-1 ].kind && keptRun > Context )
        {
            --end;
            --keptRun;
        }
        UInt32 beforeStart = edits[ start ].before;
        UInt32 afterStart = edits[ start ].after;
        UInt32 beforeCount = 0;
        UInt32 afterCount = 0;
        for ( size_t h=start; h<end; ++h )
        {
            if ( '+' != edits[ h ].kind )
                ++beforeCount;
            if ( '-' != edits[ h ].kind )
                ++afterCount;
        }
        Emit( out, "@@ -" + CORE::ToString( beforeCount > 0 ? beforeStart + 1 : beforeStart ) + "," + CORE::ToString( beforeCount ) +
                   " +" + CORE::ToString( afterCount > 0 ? afterStart + 1 : afterStart ) + "," + CORE::ToString( afterCount ) + " @@", maxLines, written, truncated );
        for ( size_t h=start; h<end && !truncated; ++h )
        {
            const CORE::CString& line = '+' == edits[ h ].kind ? b[ edits[ h ].after ] : a[ edits[ h ].before ];
            Emit( out, CORE::CString( &edits[ h ].kind, 1 ) + line, maxLines, written, truncated );
        }
        e = end;
    }
    return out;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORECONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
