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

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "tablecodecpluginXLSX_CXlsxXmlCallbacks.h"

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

CXlsxXmlCallbacks::CXlsxXmlCallbacks( void )
    : CORE::CIDocumentParserCallbacks()
    , m_status( CORE::DATATABLE_STATUS_OK )
    , m_error()
    , m_syntaxError()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CXlsxXmlCallbacks::~CXlsxXmlCallbacks()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CXlsxXmlCallbacks::IsFailed( void ) const
{GUCEF_TRACE;

    return CORE::DATATABLE_STATUS_OK != m_status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxXmlCallbacks::GetStatus( void ) const
{GUCEF_TRACE;

    return m_status;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CXlsxXmlCallbacks::GetError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CXlsxXmlCallbacks::GetSyntaxError( void ) const
{GUCEF_TRACE;

    return m_syntaxError;
}

/*-------------------------------------------------------------------------*/

void
CXlsxXmlCallbacks::Fail( CORE::TDataTableStatus status, const CORE::CString& error )
{GUCEF_TRACE;

    if ( IsFailed() )
        return;
    m_status = status;
    m_error = error;
    OnFailed();
}

/*-------------------------------------------------------------------------*/

void
CXlsxXmlCallbacks::OnFailed( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CXlsxXmlCallbacks::OnDocumentBegin( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CXlsxXmlCallbacks::OnDocumentEnd( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CXlsxXmlCallbacks::OnNodeChildrenBegin( const CORE::CVariant& nodeId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CXlsxXmlCallbacks::OnNodeChildrenEnd( const CORE::CVariant& nodeId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CXlsxXmlCallbacks::OnParseError( CORE::Int32 errorCode, const CORE::CString& description )
{GUCEF_TRACE;

    if ( m_syntaxError.IsNULLOrEmpty() )
        m_syntaxError = "XML error " + CORE::Int32ToString( errorCode ) + ": " + description;
}

/*-------------------------------------------------------------------------*/

const char*
CXlsxXmlCallbacks::LocalName( const CORE::CVariant& name, UInt32& length )
{GUCEF_TRACE;

    // the text of a variant of the XML reader is not always terminated: its size is what counts
    const char* text = name.AsCharPtr( "" );
    length = name.ByteSize( false );
    for ( UInt32 i = length; i > 0; --i )
    {
        if ( ':' == text[ i - 1 ] )
        {
            length -= i;
            return text + i;
        }
    }
    return text;
}

/*-------------------------------------------------------------------------*/

void
CXlsxXmlCallbacks::TextOf( const CORE::CVariant& value, CORE::CString& holder, const char*& text, UInt32& length )
{GUCEF_TRACE;

    text = value.AsCharPtr( GUCEF_NULL );
    if ( GUCEF_NULL != text )
    {
        length = value.ByteSize( false );
        return;
    }
    holder = value.AsString();
    text = holder.C_String();
    length = holder.ByteSize() > 0 ? holder.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

bool
CXlsxXmlCallbacks::IsName( const char* name, UInt32 nameLength, const char* expected )
{GUCEF_TRACE;

    return strlen( expected ) == nameLength && 0 == memcmp( name, expected, nameLength );
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
