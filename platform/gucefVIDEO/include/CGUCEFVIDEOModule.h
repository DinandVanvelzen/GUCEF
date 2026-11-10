/*
 *  gucefVIDEO: GUCEF module providing video decoding and playback
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

#ifndef CGUCEFVIDEOMODULE_H
#define CGUCEFVIDEOMODULE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VIDEO_MACROS_H
#include "gucefVIDEO_macros.h"         /* often used gucefVIDEO macros */
#define GUCEF_VIDEO_MACROS_H
#endif /* GUCEF_VIDEO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VIDEO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CGUCEFVIDEOModule
{
    public:

    static bool Load( void );

    static bool Unload( void );

    private:

    CGUCEFVIDEOModule( void );
    CGUCEFVIDEOModule( const CGUCEFVIDEOModule& src );
    ~CGUCEFVIDEOModule();
    CGUCEFVIDEOModule& operator=( const CGUCEFVIDEOModule& src );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace VIDEO */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* CGUCEFVIDEOMODULE_H ? */
