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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCORE_SecureMemory.h"

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

namespace {

typedef void* ( *TMemsetFunction )( void*, int, size_t );

/**
 *  The compiler cannot see which function is called through a volatile pointer, so it can neither
 *  drop the call nor replace it with nothing, whatever happens to the memory afterwards
 */
volatile TMemsetFunction g_memsetFunction = &::memset;

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
SecureWipe( void* memory, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == memory || 0 == size )
        return;
    TMemsetFunction wipe = g_memsetFunction;
    wipe( memory, 0, (size_t) size );
}

/*-------------------------------------------------------------------------*/

void
SecureWipeString( CString& text )
{GUCEF_TRACE;

    if ( !text.IsNULLOrEmpty() )
    {
        // the writable C string of a string that borrows a buffer is a copy of its own: the buffer of the owner stays as it is
        SecureWipe( text.C_String(), text.ByteSize() );
    }
    text.Clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
