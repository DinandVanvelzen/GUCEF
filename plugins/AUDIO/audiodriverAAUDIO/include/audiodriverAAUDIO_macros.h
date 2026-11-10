/*
 *  audiodriverAAUDIO: gucefAUDIO driver plugin for AAudio on Android
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

#ifndef GUCEF_AUDIODRIVER_AAUDIO_MACROS_H
#define GUCEF_AUDIODRIVER_AAUDIO_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

#ifndef GUCEF_AUDIODRIVER_AAUDIO_CONFIG_H
#include "audiodriverAAUDIO_config.h"
#define GUCEF_AUDIODRIVER_AAUDIO_CONFIG_H
#endif /* GUCEF_AUDIODRIVER_AAUDIO_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef GUCEF_AUDIODRIVER_AAUDIO_EXPORT
#ifdef GUCEF_AUDIODRIVER_AAUDIO_BUILD_MODULE
  #define GUCEF_AUDIODRIVER_AAUDIO_EXPORT GUCEF_EXPORT
#else
  #define GUCEF_AUDIODRIVER_AAUDIO_EXPORT GUCEF_IMPORT
#endif /* GUCEF_AUDIODRIVER_AAUDIO_BUILD_MODULE ? */

#undef GUCEF_AUDIODRIVER_AAUDIO_EXPORT_CPP
#ifdef GUCEF_AUDIODRIVER_AAUDIO_EXPORT_CPP_CODE
  #define GUCEF_AUDIODRIVER_AAUDIO_EXPORT_CPP GUCEF_AUDIODRIVER_AAUDIO_EXPORT
#else
  #define GUCEF_AUDIODRIVER_AAUDIO_EXPORT_CPP
#endif /* GUCEF_AUDIODRIVER_AAUDIO_EXPORT_CPP_CODE */

#undef GUCEF_AUDIODRIVER_AAUDIO_EXPORT_C
#ifdef GUCEF_AUDIODRIVER_AAUDIO_EXPORT_C_CODE
  #define GUCEF_AUDIODRIVER_AAUDIO_EXPORT_C GUCEF_AUDIODRIVER_AAUDIO_EXPORT
#else
  #define GUCEF_AUDIODRIVER_AAUDIO_EXPORT_C
#endif /* GUCEF_AUDIODRIVER_AAUDIO_EXPORT_C_CODE */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIODRIVER_AAUDIO_MACROS_H ? */
