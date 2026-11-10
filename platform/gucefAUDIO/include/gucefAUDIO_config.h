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

#ifndef GUCEF_AUDIO_CONFIG_H
#define GUCEF_AUDIO_CONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"      /* GUCEF platform build configuration */
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      BUILD DEFINES                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      If we are building or using a DLL then it is sometimes desireable to
 *      export only C code or perhaps only C++ code. You can do this with the
 *      following defines
 */
#undef GUCEF_AUDIO_PUBLIC_CPP_CODE        /* do not change this line */
#undef GUCEF_AUDIO_PUBLIC_C_CODE          /* do not change this line */
#define GUCEF_AUDIO_PUBLIC_CPP_CODE /* do you want to enable the C++ exports ? */
#define GUCEF_AUDIO_PUBLIC_C_CODE   /* do you want to enable the C exports ? */

/*
 *      Same for plugins of this module (audio drivers and audio codecs)
 */
#undef GUCEF_AUDIO_PLUGIN_PUBLIC_CPP_CODE /* do not change this line */
#undef GUCEF_AUDIO_PLUGIN_PUBLIC_C_CODE   /* do not change this line */
#define GUCEF_AUDIO_PLUGIN_PUBLIC_CPP_CODE
#define GUCEF_AUDIO_PLUGIN_PUBLIC_C_CODE

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CONFIG_H ? */
