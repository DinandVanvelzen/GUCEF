/*
 *  ProjectGen-DependsFilter: Plugin for ProjectGen to leverage depends tool output
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

#ifndef PROJECTGENDEPENDSFILTER_MACROS_H
#define PROJECTGENDEPENDSFILTER_MACROS_H

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

#ifndef PROJECTGENDEPENDSFILTER_CONFIG_H
#include "ProjectGenDependsFilter_config.h"
#define PROJECTGENDEPENDSFILTER_CONFIG_H
#endif /* PROJECTGENDEPENDSFILTER_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#undef PROJECTGENDEPFILTER_EXPORT
#ifdef PROJECTGENDEPFILTER_BUILD_MODULE
  #define PROJECTGENDEPFILTER_EXPORT GUCEF_EXPORT
#else
  #define PROJECTGENDEPFILTER_EXPORT GUCEF_IMPORT
#endif /* PROJECTGENDEPFILTER_BUILD_MODULE ? */

#undef PROJECTGENDEPFILTER_EXPORT_CPP
#ifdef PROJECTGENDEPFILTER_EXPORT_CPP_CODE
  #define PROJECTGENDEPFILTER_EXPORT_CPP PROJECTGENDEPFILTER_EXPORT
#else
  #define PROJECTGENDEPFILTER_EXPORT_CPP GUCEF_HIDDEN
#endif /* PROJECTGENDEPFILTER_EXPORT_CPP_CODE */

#undef PROJECTGENDEPFILTER_EXPORT_C
#ifdef PROJECTGENDEPFILTER_EXPORT_C_CODE
  #define PROJECTGENDEPFILTER_EXPORT_C PROJECTGENDEPFILTER_EXPORT
#else
  #define PROJECTGENDEPFILTER_EXPORT_C GUCEF_HIDDEN
#endif /* PROJECTGENDEPFILTER_EXPORT_C_CODE */

/*-------------------------------------------------------------------------*/

#endif /* PROJECTGENDEPENDSFILTER_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 26-03-2005 :
       - Initial version of this file.

-----------------------------------------------------------------------------*/
