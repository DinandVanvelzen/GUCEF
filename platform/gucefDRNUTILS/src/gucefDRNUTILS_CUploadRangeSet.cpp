/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CUploadRangeSet.h"

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUploadRangeSet::CUploadRangeSet( void )
    : m_ranges()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

void
CUploadRangeSet::Add( UInt64 offset, UInt64 length )
{GUCEF_TRACE;

    if ( 0 == length )
        return;
    UInt64 start = offset;
    UInt64 end = offset + length;      // exclusive
    TRangeVector merged;
    bool placed = false;
    for ( UInt32 i=0; i<m_ranges.size(); ++i )
    {
        const SRange& r = m_ranges[ i ];
        UInt64 rEnd = r.offset + r.length;
        if ( rEnd < start )
        {
            merged.push_back( r );          // entirely before, not touching
        }
        else
        if ( r.offset > end )
        {
            if ( !placed )
            {
                SRange n;
                n.offset = start;
                n.length = end - start;
                merged.push_back( n );
                placed = true;
            }
            merged.push_back( r );          // entirely after, not touching
        }
        else
        {
            // overlapping or touching: grow the new range
            if ( r.offset < start )
                start = r.offset;
            if ( rEnd > end )
                end = rEnd;
        }
    }
    if ( !placed )
    {
        SRange n;
        n.offset = start;
        n.length = end - start;
        merged.push_back( n );
    }
    m_ranges = merged;
}

/*-------------------------------------------------------------------------*/

bool
CUploadRangeSet::Contains( UInt64 offset, UInt64 length ) const
{GUCEF_TRACE;

    if ( 0 == length )
        return true;
    for ( UInt32 i=0; i<m_ranges.size(); ++i )
    {
        const SRange& r = m_ranges[ i ];
        if ( r.offset <= offset && r.offset + r.length >= offset + length )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

UInt64
CUploadRangeSet::GetTotalBytes( void ) const
{GUCEF_TRACE;

    UInt64 total = 0;
    for ( UInt32 i=0; i<m_ranges.size(); ++i )
        total += m_ranges[ i ].length;
    return total;
}

/*-------------------------------------------------------------------------*/

bool
CUploadRangeSet::IsComplete( UInt64 size ) const
{GUCEF_TRACE;

    return Contains( 0, size );
}

/*-------------------------------------------------------------------------*/

const CUploadRangeSet::TRangeVector&
CUploadRangeSet::GetRanges( void ) const
{GUCEF_TRACE;

    return m_ranges;
}

/*-------------------------------------------------------------------------*/

void
CUploadRangeSet::GetMissing( UInt64 size, TRangeVector& missing ) const
{GUCEF_TRACE;

    missing.clear();
    UInt64 position = 0;
    for ( UInt32 i=0; i<m_ranges.size() && position < size; ++i )
    {
        const SRange& r = m_ranges[ i ];
        if ( r.offset > position )
        {
            SRange gap;
            gap.offset = position;
            gap.length = ( r.offset < size ? r.offset : size ) - position;
            missing.push_back( gap );
        }
        UInt64 rEnd = r.offset + r.length;
        if ( rEnd > position )
            position = rEnd;
    }
    if ( position < size )
    {
        SRange gap;
        gap.offset = position;
        gap.length = size - position;
        missing.push_back( gap );
    }
}

/*-------------------------------------------------------------------------*/

void
CUploadRangeSet::Clear( void )
{GUCEF_TRACE;

    m_ranges.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUploadRangeSet::ToString( void ) const
{GUCEF_TRACE;

    CORE::CString text;
    for ( UInt32 i=0; i<m_ranges.size(); ++i )
    {
        if ( i > 0 )
            text += ',';
        text += CORE::ToString( m_ranges[ i ].offset ) + "-" + CORE::ToString( m_ranges[ i ].offset + m_ranges[ i ].length - 1 );
    }
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CUploadRangeSet::FromString( const CORE::CString& text )
{GUCEF_TRACE;

    CUploadRangeSet parsed;
    CORE::CString trimmed = text.Trim( true ).Trim( false );
    if ( !trimmed.IsNULLOrEmpty() )
    {
        CORE::CString::StringVector parts = trimmed.ParseElements( ',', false );
        for ( UInt32 i=0; i<parts.size(); ++i )
        {
            CORE::CString part = parts[ i ].Trim( true ).Trim( false );
            Int32 dash = part.HasChar( '-' );
            if ( dash <= 0 )
                return false;
            CORE::CString first = part.SubstrToIndex( (UInt32) dash, true );
            CORE::CString last = part.CutChars( (UInt32) dash + 1, true );
            if ( first.IsNULLOrEmpty() || last.IsNULLOrEmpty() )
                return false;
            UInt64 from = CORE::StringToUInt64( first, GUCEF_UINT64MAX );
            UInt64 to = CORE::StringToUInt64( last, GUCEF_UINT64MAX );
            if ( GUCEF_UINT64MAX == from || GUCEF_UINT64MAX == to || to < from )
                return false;
            parsed.Add( from, to - from + 1 );
        }
    }
    m_ranges = parsed.m_ranges;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
