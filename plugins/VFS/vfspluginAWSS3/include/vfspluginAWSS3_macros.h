/*
 *  vfspluginAWSS3: Generic GUCEF VFS plugin for dealing with S3 storage in AWS
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

#ifndef GUCEF_VFSPLUGIN_AWSS3_MACROS_H
#define GUCEF_VFSPLUGIN_AWSS3_MACROS_H

/*
 *      Build configuration specific macros.
 *      Also includes the generic macros which are build config independant.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_ETYPES_H
#include "gucefVFS_ETypes.h"      /* simple types */
#define GUCEF_VFS_ETYPES_H
#endif /* GUCEF_VFS_ETYPES_H ? */

#ifndef GUCEF_VFS_MACROS_H
#include "gucefVFS_macros.h"      /* VFS Module build configuration */
#define GUCEF_VFS_MACROS_H
#endif /* GUCEF_VFS_MACROS_H ? */

#ifndef GUCEF_VFSPLUGIN_AWSS3_CONFIG_H
#include "vfspluginAWSS3_config.h"
#define GUCEF_VFSPLUGIN_AWSS3_CONFIG_H
#endif /* GUCEF_VFSPLUGIN_AWSS3_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef GUCEF_VFSPLUGIN_AWSS3_EXPORT
#ifdef GUCEF_VFSPLUGIN_AWSS3_BUILD_MODULE
  #define GUCEF_VFSPLUGIN_AWSS3_EXPORT GUCEF_EXPORT
#else
  #define GUCEF_VFSPLUGIN_AWSS3_EXPORT GUCEF_IMPORT
#endif /* GUCEF_VFSPLUGIN_AWSS3_BUILD_MODULE ? */

#undef GUCEF_VFSPLUGIN_AWSS3_EXPORT_CPP
#ifdef GUCEF_VFSPLUGIN_AWSS3_EXPORT_CPP_CODE
  #define GUCEF_VFSPLUGIN_AWSS3_EXPORT_CPP GUCEF_VFSPLUGIN_AWSS3_EXPORT
#else
  #define GUCEF_VFSPLUGIN_AWSS3_EXPORT_CPP
#endif /* GUCEF_VFSPLUGIN_AWSS3_EXPORT_CPP_CODE */

#undef GUCEF_VFSPLUGIN_AWSS3_EXPORT_C
#ifdef GUCEF_VFSPLUGIN_AWSS3_EXPORT_C_CODE
  #define GUCEF_VFSPLUGIN_AWSS3_EXPORT_C GUCEF_VFSPLUGIN_AWSS3_EXPORT
#else
  #define GUCEF_VFSPLUGIN_AWSS3_EXPORT_C
#endif /* GUCEF_VFSPLUGIN_AWSS3_EXPORT_C_CODE */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_AWSS3_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 26-03-2005 :
       - Initial version of this file.

-----------------------------------------------------------------------------*/
