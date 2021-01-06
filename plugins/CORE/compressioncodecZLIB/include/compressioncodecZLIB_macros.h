/*
 *  compressioncodecZLIB: Codec plugin for zlib/gzip/deflate/crc32/adler32
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

#ifndef GUCEF_COMPRESSIONCODEC_ZLIB_MACROS_H
#define GUCEF_COMPRESSIONCODEC_ZLIB_MACROS_H

/*
 *      Build configuration specific macros.
 *      Also includes the generic macros which are build config independant.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"      /* simple types */
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"      /* VFS Module build configuration */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_COMPRESSIONCODEC_ZLIB_CONFIG_H
#include "compressioncodecZLIB_config.h"
#define GUCEF_COMPRESSIONCODEC_ZLIB_CONFIG_H
#endif /* GUCEF_COMPRESSIONCODEC_ZLIB_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMPRESSIONCODEC_ZLIB_MACROS_H ? */

