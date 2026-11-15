/*
 *  dbpluginPOSTGRESQL: gucefCOMCORE database backend for PostgreSQL
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

#ifndef DBPLUGIN_POSTGRESQL_MACROS_H
#define DBPLUGIN_POSTGRESQL_MACROS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CONFIG_H
#include "dbpluginPOSTGRESQL_config.h"
#define DBPLUGIN_POSTGRESQL_CONFIG_H
#endif /* DBPLUGIN_POSTGRESQL_CONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      When compiling the plugin DBPLUGIN_POSTGRESQL_BUILD_MODULE is defined.
 */
#undef DBPLUGIN_POSTGRESQL_EXPORTSPEC
#ifdef DBPLUGIN_POSTGRESQL_BUILD_MODULE
  #define DBPLUGIN_POSTGRESQL_EXPORTSPEC GUCEF_EXPORT
#else
  #define DBPLUGIN_POSTGRESQL_EXPORTSPEC GUCEF_IMPORT
#endif /* DBPLUGIN_POSTGRESQL_BUILD_MODULE ? */

/*-------------------------------------------------------------------------*/

#undef DBPLUGIN_POSTGRESQL_PUBLIC_CPP
#ifdef DBPLUGIN_POSTGRESQL_PUBLIC_CPP_CODE
  #define DBPLUGIN_POSTGRESQL_PUBLIC_CPP DBPLUGIN_POSTGRESQL_EXPORTSPEC
#else
  #define DBPLUGIN_POSTGRESQL_PUBLIC_CPP GUCEF_HIDDEN
#endif /* DBPLUGIN_POSTGRESQL_PUBLIC_CPP_CODE */

#undef DBPLUGIN_POSTGRESQL_PUBLIC_C
#ifdef DBPLUGIN_POSTGRESQL_PUBLIC_C_CODE
  #define DBPLUGIN_POSTGRESQL_PUBLIC_C DBPLUGIN_POSTGRESQL_EXPORTSPEC
#else
  #define DBPLUGIN_POSTGRESQL_PUBLIC_C GUCEF_HIDDEN
#endif /* DBPLUGIN_POSTGRESQL_PUBLIC_C_CODE */

#define DBPLUGIN_POSTGRESQL_PRIVATE_CPP GUCEF_HIDDEN
#define DBPLUGIN_POSTGRESQL_PRIVATE_C GUCEF_HIDDEN

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_POSTGRESQL_MACROS_H ? */
