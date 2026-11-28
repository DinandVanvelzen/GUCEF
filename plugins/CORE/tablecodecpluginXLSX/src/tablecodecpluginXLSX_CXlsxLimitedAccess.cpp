/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#include "tablecodecpluginXLSX_CXlsxLimitedAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace XLSX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CXlsxLimitedAccess::CXlsxLimitedAccess( const CORE::IOAccessPtr& inner, UInt64* budget )
    : CORE::CIOAccess()
    , m_inner( inner )
    , m_budget( budget )
    , m_bytesRead( 0 )
    , m_exceeded( false )
    , m_aborted( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CXlsxLimitedAccess::~CXlsxLimitedAccess()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CXlsxLimitedAccess::IsExceeded( void ) const
{GUCEF_TRACE;

    return m_exceeded;
}

/*-------------------------------------------------------------------------*/

void
CXlsxLimitedAccess::Abort( void )
{GUCEF_TRACE;

    m_aborted = true;
}

/*-------------------------------------------------------------------------*/

UInt64
CXlsxLimitedAccess::GetBytesRead( void ) const
{GUCEF_TRACE;

    return m_bytesRead;
}

/*-------------------------------------------------------------------------*/

void
CXlsxLimitedAccess::Open( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CXlsxLimitedAccess::Close( void )
{GUCEF_TRACE;

    if ( !m_inner.IsNULL() )
        m_inner->Close();
}

/*-------------------------------------------------------------------------*/

bool
CXlsxLimitedAccess::Opened( void ) const
{GUCEF_TRACE;

    return !m_inner.IsNULL() && m_inner->Opened();
}

/*-------------------------------------------------------------------------*/

UInt32
CXlsxLimitedAccess::Read( void* dest, UInt32 esize, UInt32 elements )
{GUCEF_TRACE;

    if ( m_aborted || m_exceeded || 0 == esize || 0 == elements || m_inner.IsNULL() )
        return 0;

    UInt64 wanted = (UInt64) esize * elements;
    bool cut = false;
    if ( wanted > *m_budget )
    {
        wanted = *m_budget;
        cut = true;
    }

    UInt64 got = 0;
    if ( wanted > 0 )
        got = m_inner->Read( dest, 1, (UInt32) wanted );
    *m_budget -= got;
    m_bytesRead += got;

    if ( cut && got == wanted )
    {
        // the budget is used up: more data behind it is a part that is larger than allowed
        char probe = 0;
        if ( 1 == m_inner->Read( &probe, 1, 1 ) )
            m_exceeded = true;
    }
    return (UInt32) ( got / esize );
}

/*-------------------------------------------------------------------------*/

UInt64
CXlsxLimitedAccess::Tell( void ) const
{GUCEF_TRACE;

    return m_bytesRead;
}

/*-------------------------------------------------------------------------*/

Int32
CXlsxLimitedAccess::Seek( Int64 offset, Int32 origin )
{GUCEF_TRACE;

    return -1;
}

/*-------------------------------------------------------------------------*/

char
CXlsxLimitedAccess::GetChar( void )
{GUCEF_TRACE;

    char value = 0;
    Read( &value, 1, 1 );
    return value;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxLimitedAccess::Eof( void ) const
{GUCEF_TRACE;

    return m_aborted || m_exceeded || m_inner.IsNULL() || m_inner->Eof();
}

/*-------------------------------------------------------------------------*/

bool
CXlsxLimitedAccess::IsReadable( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxLimitedAccess::IsWriteable( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxLimitedAccess::IsValid( void )
{GUCEF_TRACE;

    return Opened();
}

/*-------------------------------------------------------------------------*/

void
CXlsxLimitedAccess::Flush( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt64
CXlsxLimitedAccess::GetSize( void ) const
{GUCEF_TRACE;

    return m_inner.IsNULL() ? 0 : m_inner->GetSize();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CXlsxLimitedAccess::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSX */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
