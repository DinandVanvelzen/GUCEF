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

#ifndef CGUCEFAUDIOMODULE_H
#define CGUCEFAUDIOMODULE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"         /* often used gucefAUDIO macros */
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CGUCEFAUDIOModule
{
    public:

    static bool Load( void );

    static bool Unload( void );

    private:

    CGUCEFAUDIOModule( void );
    CGUCEFAUDIOModule( const CGUCEFAUDIOModule& src );
    ~CGUCEFAUDIOModule();
    CGUCEFAUDIOModule& operator=( const CGUCEFAUDIOModule& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace AUDIO */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* CGUCEFAUDIOMODULE_H ? */
