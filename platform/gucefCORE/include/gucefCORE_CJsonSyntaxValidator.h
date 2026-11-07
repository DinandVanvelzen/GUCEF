/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CJSONSYNTAXVALIDATOR_H
#define GUCEF_CORE_CJSONSYNTAXVALIDATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Checks that bytes are one JSON text in the strict grammar of RFC 8259, before untrusted text reaches a JSON codec
 *  that is lenient (accepts trailing commas, reads past truncated input or recurses without bound).
 *
 *  Taken: exactly one value with optional white space (space, tab, line feed, carriage return) around it; objects with
 *  string member names; strings of valid UTF-8 without control characters, with the escapes \" \\ \/ \b \f \n \r \t
 *  and \uXXXX; numbers as -?(0|[1-9][0-9]*)(\.[0-9]+)?([eE][+-]?[0-9]+)?; true, false and null. Nesting of objects and
 *  arrays is bounded. Everything else, a byte order mark included, is refused.
 *
 *  The check reads every byte at most once, never past the given count, needs no terminating zero, does not
 *  recurse and does not allocate (the nesting is tracked in a fixed bit stack). Thread safe: no state.
 */
class GUCEF_CORE_PUBLIC_CPP CJsonSyntaxValidator
{
    public:

    /** The deepest nesting of objects and arrays the check can track; a larger maximum is taken as this one */
    static const UInt32 MaxTrackedNestingDepth = 4096;

    /**
     *  Whether the bytes are one strict JSON text whose objects and arrays nest at most maxNestingDepth deep (a
     *  scalar has depth 0, "[]" depth 1). On failure the error, when asked for, names the byte offset and the reason.
     */
    static bool IsValid( const void* text            ,
                         UInt32 byteCount            ,
                         UInt32 maxNestingDepth      ,
                         CString* error = GUCEF_NULL );

    private:

    CJsonSyntaxValidator( void );                                           /**< not implemented: static members only */
    CJsonSyntaxValidator( const CJsonSyntaxValidator& src );                /**< not implemented */
    CJsonSyntaxValidator& operator=( const CJsonSyntaxValidator& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJSONSYNTAXVALIDATOR_H ? */
