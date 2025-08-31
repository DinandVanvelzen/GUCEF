/*
 *  gucef common header: provides header based platform wide facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_IO_H
#define GUCEF_IO_H

/*
 *      Basic I/O macros for the GUCEF platform
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"        /* GUCEF configuration */
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"      /* GUCEF platform compilation targets */
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Macros that defines the correct character to be used for directory
 *  seperation in a path depending on the platform we are building for.
 */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define GUCEF_DIRSEPCHAROPPOSITE '/'
  #define GUCEF_DIRSEPCHAR '\\'
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )
  #define GUCEF_DIRSEPCHAROPPOSITE '\\'
  #define GUCEF_DIRSEPCHAR '/'
#endif

/*-------------------------------------------------------------------------*/

/*
 *  Macros that defines the correct end of line string to be used
 *  depending on the platform we are building for.
 */
#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define GUCEF_EOL "\r\n"
  #define GUCEF_EOL_LENGTH 2
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN ) )
  #define GUCEF_EOL "\n"
  #define GUCEF_EOL_LENGTH 1
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_IPHONEOS ) )
  #define GUCEF_EOL "\r"
  #define GUCEF_EOL_LENGTH 1
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_DOS_EOF_CHAR  (char) 26

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IO_H ? */
