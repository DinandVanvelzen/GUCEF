/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_STRINGUTILS_H
#define GUCEF_AUTH_STRINGUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The number of bytes of the text, without the terminator. CString::Length() counts characters, which for text with
 *  multi byte characters is fewer than the bytes that hashing, encryption and signing work on.
 */
inline UInt32
GetByteLength( const CORE::CString& text )
{GUCEF_TRACE;

    UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

/*-------------------------------------------------------------------------*/

/** A string of exactly the given number of bytes from text that is not null terminated */
inline CORE::CString
MakeString( const char* text, UInt32 byteCount )
{GUCEF_TRACE;

    CORE::CString result;
    if ( GUCEF_NULL != text && byteCount > 0 )
        result.Append( text, byteCount );
    return result;
}

/** Whether the string is one of the values, compared as it is */
inline bool
ContainsString( const CORE::CString::StringVector& values, const CORE::CString& value )
{GUCEF_TRACE;

    for ( size_t i=0; i<values.size(); ++i )
        if ( values[ i ] == value )
            return true;
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_STRINGUTILS_H ? */
