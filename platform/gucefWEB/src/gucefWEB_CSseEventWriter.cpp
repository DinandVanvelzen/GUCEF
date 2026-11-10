/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#include "gucefWEB_CSseEventWriter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CSseEventWriter::AppendField( CORE::CDynamicBuffer& out   ,
                              const char* fieldPrefix     ,
                              UInt32 prefixSize           ,
                              const char* data            ,
                              UInt32 dataSize             )
{GUCEF_TRACE;

    // every line of the text becomes a line of the field: CR LF, LF and a lone CR all end a line in SSE
    UInt32 lineStart = 0;
    for ( UInt32 i=0; i<=dataSize; ++i )
    {
        const bool atEnd = ( i == dataSize );
        const char c = atEnd ? '\0' : data[ i ];
        if ( !atEnd && '\r' != c && '\n' != c )
            continue;

        out.Append( fieldPrefix, prefixSize );
        if ( i > lineStart )
            out.Append( data + lineStart, i - lineStart );
        out.Append( "\n", 1 );

        if ( '\r' == c && i+1 < dataSize && '\n' == data[ i+1 ] )
            ++i;
        lineStart = i+1;
    }
}

/*-------------------------------------------------------------------------*/

bool
CSseEventWriter::AppendEvent( CORE::CDynamicBuffer& out         ,
                              const char* data                  ,
                              UInt32 dataSize                   ,
                              const CORE::CString& eventName    )
{GUCEF_TRACE;

    if ( !eventName.IsNULLOrEmpty() )
    {
        if ( eventName.HasChar( '\r' ) >= 0 || eventName.HasChar( '\n' ) >= 0 || eventName.HasChar( ':' ) >= 0 )
            return false;
        out.Append( "event: ", 7 );
        out.Append( eventName.C_String(), eventName.ByteSize() - 1 );
        out.Append( "\n", 1 );
    }
    AppendField( out, "data: ", 6, GUCEF_NULL == data ? "" : data, GUCEF_NULL == data ? 0 : dataSize );
    out.Append( "\n", 1 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSseEventWriter::AppendEvent( CORE::CDynamicBuffer& out         ,
                              const CORE::CString& data         ,
                              const CORE::CString& eventName    )
{GUCEF_TRACE;

    return AppendEvent( out, data.C_String(), data.ByteSize() > 0 ? data.ByteSize() - 1 : 0, eventName );
}

/*-------------------------------------------------------------------------*/

void
CSseEventWriter::AppendComment( CORE::CDynamicBuffer& out, const CORE::CString& text )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
    {
        out.Append( ":\n", 2 );
        return;
    }
    AppendField( out, ": ", 2, text.C_String(), text.ByteSize() - 1 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
