/*
 *  tablecodecpluginPARQUET: GUCEF table codec plugin for Apache Parquet files
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

#include "tablecodecpluginPARQUET_CParquetReadOptions.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace PARQUET {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
ParseCount( const CORE::CValueList& options, const char* key, UInt64 minimum, UInt64 maximum, UInt64& value, CORE::CString& error )
{
    if ( !options.HasKey( key ) )
        return true;
    const CORE::CString text = options.GetValueAlways( key ).AsString();
    const char* chars = text.C_String();
    const UInt32 length = text.Length();
    UInt64 result = 0;
    bool valid = length > 0 && length <= 20;
    for ( UInt32 i=0; valid && i<length; ++i )
    {
        if ( chars[ i ] < '0' || chars[ i ] > '9' )
        {
            valid = false;
            break;
        }
        const UInt64 digit = (UInt64) ( chars[ i ] - '0' );
        if ( result > ( GUCEF_UINT64MAX - digit ) / 10 )
        {
            valid = false;
            break;
        }
        result = result * 10 + digit;
    }
    if ( !valid || result < minimum || result > maximum )
    {
        error = CORE::CString( "Parquet option \"" ) + key + "\" is not a count within its bounds: \"" + text + "\"";
        return false;
    }
    value = result;
    return true;
}

bool
ParseFlag( const CORE::CValueList& options, const char* key, bool& value, CORE::CString& error )
{
    if ( !options.HasKey( key ) )
        return true;
    const CORE::CString text = options.GetValueAlways( key ).AsString().Lowercase();
    if ( text == "true" || text == "1" || text == "yes" ) { value = true; return true; }
    if ( text == "false" || text == "0" || text == "no" ) { value = false; return true; }
    error = CORE::CString( "Parquet option \"" ) + key + "\" is not a boolean: \"" + text + "\"";
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SParquetReadOptions::SParquetReadOptions( void )
    : maxFooterBytes( 64ULL * 1024 * 1024 )
    , maxMetaDataBytes( 256ULL * 1024 * 1024 )
    , maxPageBytes( 256ULL * 1024 * 1024 )
    , maxChunkBytes( 1024ULL * 1024 * 1024 )
    , ioCoalesceGapBytes( 1024ULL * 1024 )
    , validateUtf8( false )
{
}

/*-------------------------------------------------------------------------*/

bool
SParquetReadOptions::Parse( const CORE::CValueList& options, CORE::CString& error )
{GUCEF_TRACE;

    // the bounds stay below the largest pooled buffer (2 GiB) so every bounded size can be allocated in one buffer
    const UInt64 maxBuffer = 1536ULL * 1024 * 1024;
    SParquetReadOptions parsed( *this );
    if ( !ParseCount( options, "maxFooterBytes", 8, maxBuffer, parsed.maxFooterBytes, error ) ||
         !ParseCount( options, "maxMetaDataBytes", 1024, 16ULL * 1024 * 1024 * 1024, parsed.maxMetaDataBytes, error ) ||
         !ParseCount( options, "maxPageBytes", 1024, maxBuffer, parsed.maxPageBytes, error ) ||
         !ParseCount( options, "maxChunkBytes", 1024, maxBuffer, parsed.maxChunkBytes, error ) ||
         !ParseCount( options, "ioCoalesceGapBytes", 0, maxBuffer, parsed.ioCoalesceGapBytes, error ) ||
         !ParseFlag( options, "validateUtf8", parsed.validateUtf8, error ) )
        return false;
    *this = parsed;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
