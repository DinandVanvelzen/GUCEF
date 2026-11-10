/*
 *  dbpluginVFSTABLES: GUCEF database backend over table files in the VFS
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "dbpluginVFSTABLES_CVfsTablesSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace VFSTABLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A decimal count without sign or blanks */
bool
ParseCount( const CORE::CString& text, UInt64& value )
{
    CORE::CUtf8String utf8 = CORE::ToUtf8String( text );
    const char* bytes = utf8.C_String();
    const UInt32 length = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
    if ( 0 == length || length > 20 )
        return false;
    UInt64 result = 0;
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( bytes[ i ] < '0' || bytes[ i ] > '9' )
            return false;
        const UInt64 digit = (UInt64) ( bytes[ i ] - '0' );
        if ( result > ( GUCEF_UINT64MAX - digit ) / 10 )
            return false;
        result = result * 10 + digit;
    }
    value = result;
    return true;
}

/** Takes a count option when it is given; false with the reason when it is not a count */
bool
TakeCountOption( const CORE::CValueList& options, const char* key, UInt64 maximum, UInt64& value, CORE::CString& error )
{
    CORE::CVariant text;
    if ( !options.TryGetValue( key, text ) )
        return true;
    UInt64 parsed = 0;
    if ( !ParseCount( text.AsString(), parsed ) || parsed > maximum )
    {
        error = CORE::CString( "Backend option \"" ) + key + "\" must be a count, not \"" + text.AsString() + "\"";
        return false;
    }
    value = parsed;
    return true;
}

/** Takes a boolean option when it is given */
bool
TakeBoolOption( const CORE::CValueList& options, const char* key, bool& value, CORE::CString& error )
{
    CORE::CVariant text;
    if ( !options.TryGetValue( key, text ) )
        return true;
    const CORE::CString lower = text.AsString().Lowercase();
    if ( "true" == lower || "1" == lower || "yes" == lower )
        value = true;
    else
    if ( "false" == lower || "0" == lower || "no" == lower )
        value = false;
    else
    {
        error = CORE::CString( "Backend option \"" ) + key + "\" must be true or false, not \"" + text.AsString() + "\"";
        return false;
    }
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsTablesSettings::CVfsTablesSettings( void )
    : writerId( GetDefaultWriterId() )
    , footerCacheBytes( DefaultFooterCacheBytes )
    , listingCacheTtlMs( 0 )
    , replaceRetryTimeoutMs( DefaultReplaceRetryTimeoutMs )
    , syncWrites( true )
    , skipIncompatibleFiles( false )
    , codecOptions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsTablesSettings::GetDefaultWriterId( void )
{GUCEF_TRACE;

    CORE::CString host = CORE::GetHostname();
    if ( host.IsNULLOrEmpty() )
        host = "host";
    return host + "-" + CORE::ToString( MT::GetProcessID() );
}

/*-------------------------------------------------------------------------*/

bool
CVfsTablesSettings::LoadFromBackendOptions( const CORE::CValueList& backendOptions, CORE::CString& error )
{GUCEF_TRACE;

    CVfsTablesSettings loaded;

    CORE::CVariant value;
    if ( backendOptions.TryGetValue( "writerId", value ) )
    {
        loaded.writerId = value.AsString();
        if ( loaded.writerId.IsNULLOrEmpty() )
        {
            error = "Backend option \"writerId\" must not be empty";
            return false;
        }
    }

    UInt64 count = loaded.footerCacheBytes;
    if ( !TakeCountOption( backendOptions, "footerCacheBytes", GUCEF_UINT64MAX, count, error ) )
        return false;
    loaded.footerCacheBytes = count;

    count = loaded.listingCacheTtlMs;
    if ( !TakeCountOption( backendOptions, "listingCacheTtlMs", GUCEF_UINT32MAX, count, error ) )
        return false;
    loaded.listingCacheTtlMs = (UInt32) count;

    count = loaded.replaceRetryTimeoutMs;
    if ( !TakeCountOption( backendOptions, "replaceRetryTimeoutMs", GUCEF_UINT32MAX, count, error ) )
        return false;
    loaded.replaceRetryTimeoutMs = (UInt32) count;

    if ( !TakeBoolOption( backendOptions, "syncWrites", loaded.syncWrites, error ) )
        return false;

    if ( backendOptions.TryGetValue( "onIncompatibleFile", value ) )
    {
        const CORE::CString mode = value.AsString();
        if ( "skip" == mode )
            loaded.skipIncompatibleFiles = true;
        else
        if ( "fail" == mode )
            loaded.skipIncompatibleFiles = false;
        else
        {
            error = "Backend option \"onIncompatibleFile\" must be fail or skip, not \"" + mode + "\"";
            return false;
        }
    }

    // the read options of the codecs
    static const char* const codecKeys[] = { "ioCoalesceGapBytes", "maxFooterBytes", "maxPageBytes" };
    loaded.codecOptions.SetAllowMultipleValues( false );
    for ( UInt32 i=0; i<3; ++i )
    {
        count = 0;
        CORE::CVariant given;
        if ( !backendOptions.TryGetValue( codecKeys[ i ], given ) )
            continue;
        if ( !TakeCountOption( backendOptions, codecKeys[ i ], GUCEF_UINT64MAX, count, error ) )
            return false;
        loaded.codecOptions.Set( codecKeys[ i ], CORE::CVariant( CORE::ToString( count ) ) );
    }

    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
