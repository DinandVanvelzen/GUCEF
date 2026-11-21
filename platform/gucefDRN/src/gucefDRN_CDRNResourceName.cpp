/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#include "gucefDRN_CDRNResourceName.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CDRNResourceName::IsValidProviderName( const CORE::CString& providerName )
{GUCEF_TRACE;

    UInt32 size = providerName.ByteSize() > 0 ? providerName.ByteSize() - 1 : 0;
    if ( 0 == size || size > MaxProviderNameLength )
        return false;
    const char* text = providerName.C_String();
    for ( UInt32 i=0; i<size; ++i )
    {
        char c = text[ i ];
        bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '.' == c || '_' == c || '-' == c;
        if ( !ok )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNResourceName::IsValidResourceName( const CORE::CString& resourceName )
{GUCEF_TRACE;

    // checked byte by byte over the whole storage, so that a NUL inside the name is seen as well
    UInt32 size = resourceName.ByteSize() > 0 ? resourceName.ByteSize() - 1 : 0;
    if ( 0 == size || size > MaxResourceNameLength )
        return false;
    const UInt8* text = reinterpret_cast< const UInt8* >( resourceName.C_String() );

    UInt32 segmentStart = 0;
    for ( UInt32 i=0; i<=size; ++i )
    {
        if ( i == size || '/' == text[ i ] )
        {
            UInt32 segmentSize = i - segmentStart;
            if ( 0 == segmentSize )
                return false;   // a leading, trailing or double '/'
            if ( 1 == segmentSize && '.' == text[ segmentStart ] )
                return false;
            if ( 2 == segmentSize && '.' == text[ segmentStart ] && '.' == text[ segmentStart+1 ] )
                return false;
            segmentStart = i + 1;
            continue;
        }
        UInt8 c = text[ i ];
        if ( c < 0x20 || 0x7F == c || '\\' == c || ':' == c )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
