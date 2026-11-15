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

#include "gucefGUI_CGridCsvOptions.h"

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

CGridCsvOptions::CGridCsvOptions( void )
    : delimiter( ',' )
    , lineEnding( LINEENDING_CRLF )
    , byteOrderMark( false )
    , includeHeader( true )
    , rawValues( false )
    , batchRowCount( 4096 )
    , immediateCellLimit( 200000 )
    , cellsPerFrame( 50000 )
    , useThreadPool( false )
    , threadPoolName()
    , showResultMessage( true )
    , suggestedFileName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CGridCsvOptions::Validate( CString& error ) const
{GUCEF_TRACE;

    const unsigned char d = (unsigned char) delimiter;
    if ( '\0' == d || '"' == d || '\r' == d || '\n' == d || d > 0x7F )
    {
        error = "The delimiter of a CSV file must be one ASCII character that is not a quote, a line break or NUL";
        return false;
    }
    if ( batchRowCount < 1 || batchRowCount > 1048576 )
    {
        error = "The rows of a batch must be between 1 and 1048576";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CString
CGridCsvOptions::GetLineEndingCodecText( void ) const
{GUCEF_TRACE;

    switch ( lineEnding )
    {
        case LINEENDING_LF: return "lf";
        case LINEENDING_CR: return "cr";
        case LINEENDING_CRLF:
        default: return "crlf";
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
