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

#include "gucefCORE_COsCryptoCodecs.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Linux, Windows and macOS have their implementation in their platform directory.
 *  The other platforms offer no OS codecs.
 */
#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_LINUX ) && ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN ) && ( GUCEF_PLATFORM != GUCEF_PLATFORM_MACOS )

void
COsCryptoCodecs::CreateCodecs( CCodecRegistry::TICodecPtrVector& codecs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const char*
COsCryptoCodecs::GetProviderName( void )
{GUCEF_TRACE;

    return "";
}

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
