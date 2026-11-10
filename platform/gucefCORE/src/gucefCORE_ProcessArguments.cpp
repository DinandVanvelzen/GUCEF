/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
  #include <shellapi.h>         /* CommandLineToArgvW() */
  #undef min
  #undef max
  #ifdef _MSC_VER
    #pragma comment(lib, "shell32.lib")
  #endif
#endif

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

#include "gucefCORE_ProcessArguments.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static inline bool
IsArgumentSpace( char c )
{
    return ' ' == c || '\t' == c || '\r' == c || '\n' == c;
}

/*-------------------------------------------------------------------------*/

/** Reads one part [ begin, end ) of an argument as key=value into the list, see ParseApplicationArguments() */
static void
AddArgumentPart( const char* begin, const char* end, CValueList& keyValueList )
{
    for ( ;; )
    {
        while ( begin < end && IsArgumentSpace( *begin ) )
            ++begin;
        while ( end > begin && IsArgumentSpace( *( end - 1 ) ) )
            --end;
        if ( begin == end )
            return;

        // an option such as --console is read by the entry point macros, not a key / value pair: skip it
        if ( end - begin >= 2 && '-' == begin[ 0 ] && '-' == begin[ 1 ] )
        {
            while ( begin < end && !IsArgumentSpace( *begin ) )
                ++begin;
            continue;
        }
        break;
    }

    const char* equals = begin;
    while ( equals < end && '=' != *equals )
        ++equals;
    const CString key( begin, (UInt32) ( equals - begin ) );
    if ( key.IsNULLOrEmpty() )
        return;
    const CString value = equals < end ? CString( equals + 1, (UInt32) ( end - equals - 1 ) ) : CString();
    keyValueList.Set( key, CVariant( value ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
GetProcessArguments( int argc                         ,
                     char** argv                      ,
                     CString::StringVector& arguments )
{GUCEF_TRACE;

    arguments.clear();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // To verify on Windows: quoted arguments with spaces, an empty argument (""), a trailing backslash before a
    // quote, and a service started by the SCM (the image path's command line). The split is the one of
    // CommandLineToArgvW(), which is what the Microsoft C runtime does for argv except for the program name.
    int count = 0;
    LPWSTR* wideArguments = ::CommandLineToArgvW( ::GetCommandLineW(), &count );
    if ( NULL == wideArguments )
        return false;
    for ( int i=1; i<count; ++i )
        arguments.push_back( ToString( wideArguments[ i ] ) );
    ::LocalFree( wideArguments );
    return true;

    #else

    if ( GUCEF_NULL == argv )
        return true;
    for ( int i=1; i<argc; ++i )
    {
        if ( GUCEF_NULL != argv[ i ] )
            arguments.push_back( CString( argv[ i ] ) );
    }
    return true;

    #endif
}

/*-------------------------------------------------------------------------*/

void
ParseApplicationArguments( int argc                 ,
                           char** argv              ,
                           CValueList& keyValueList ,
                           char pairSeparator       )
{GUCEF_TRACE;

    if ( argc <= 0 || GUCEF_NULL == argv )
        return;

    // Windows: the entry point macros give argv[ 0 ] the argument string, there is no program path to skip
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const int first = 0;
    #else
    const int first = 1;
    #endif

    for ( int i=first; i<argc; ++i )
    {
        const char* argument = argv[ i ];
        if ( GUCEF_NULL == argument )
            continue;

        const char* partBegin = argument;
        const char* cursor = argument;
        for ( ;; )
        {
            if ( '\0' == *cursor || pairSeparator == *cursor )
            {
                AddArgumentPart( partBegin, cursor, keyValueList );
                if ( '\0' == *cursor )
                    break;
                partBegin = cursor + 1;
            }
            ++cursor;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
