/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_CAUDIOGLOBAL_H
#include "gucefAUDIO_CAudioGlobal.h"
#define GUCEF_AUDIO_CAUDIOGLOBAL_H
#endif /* GUCEF_AUDIO_CAUDIOGLOBAL_H ? */

#include "CGUCEFAUDIOModule.h"  /* definition of the class implemented here */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CGUCEFAUDIOModule::Load( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGUCEFAUDIOModule::Unload( void )
{GUCEF_TRACE;

    CAudioGlobal::Deinstance();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace AUDIO */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
