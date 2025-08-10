/*
 *  ProjectGen-VSImporter: Plugin for ProjectGen to import from Visual Studio files
 *
 *  Copyright (C) 1998 - 2025.  Dinand Vanvelzen
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

#ifndef PROJECTGENVSIMPORTER_MACROS_H
#define PROJECTGENVSIMPORTER_MACROS_H

/*
 *      Build configuration specific macros.
 *      Also includes the generic macros which are build config independant.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PROJECTGEN_ETYPES_H
#include "gucefProjectGen_ETypes.h"      /* simple types */
#define GUCEF_PROJECTGEN_ETYPES_H
#endif /* GUCEF_PROJECTGEN_ETYPES_H ? */

#ifndef GUCEF_PROJECTGEN_MACROS_H
#include "gucefProjectGen_macros.h"
#define GUCEF_PROJECTGEN_MACROS_H
#endif /* GUCEF_PROJECTGEN_MACROS_H ? */

#ifndef PROJECTGENVSIMPORTER_CONFIG_H
#include "ProjectGenVSImporter_config.h"
#define PROJECTGENVSIMPORTER_CONFIG_H
#endif /* PROJECTGENVSIMPORTER_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef PROJECTGENVSIMPORTER_EXPORT
#ifdef PROJECTGENVSIMPORTER_BUILD_MODULE
  #define PROJECTGENVSIMPORTER_EXPORT GUCEF_EXPORT
#else
  #define PROJECTGENVSIMPORTER_EXPORT GUCEF_IMPORT
#endif /* PROJECTGENVSIMPORTER_BUILD_MODULE ? */

#undef PROJECTGENVSIMPORTER_EXPORT_CPP
#ifdef PROJECTGENVSIMPORTER_EXPORT_CPP_CODE
  #define PROJECTGENVSIMPORTER_EXPORT_CPP PROJECTGENVSIMPORTER_EXPORT
#else
  #define PROJECTGENVSIMPORTER_EXPORT_CPP GUCEF_HIDDEN
#endif /* PROJECTGENVSIMPORTER_EXPORT_CPP_CODE */

#undef PROJECTGENVSIMPORTER_EXPORT_C
#ifdef PROJECTGENVSIMPORTER_EXPORT_C_CODE
  #define PROJECTGENVSIMPORTER_EXPORT_C PROJECTGENVSIMPORTER_EXPORT
#else
  #define PROJECTGENVSIMPORTER_EXPORT_C GUCEF_HIDDEN
#endif /* PROJECTGENVSIMPORTER_EXPORT_C_CODE */

/*-------------------------------------------------------------------------*/

#endif /* PROJECTGENVSIMPORTER_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 26-03-2005 :
       - Initial version of this file.

-----------------------------------------------------------------------------*/
