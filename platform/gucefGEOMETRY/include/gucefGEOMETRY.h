/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

/*
 *      This is the main include header for the gucefGEOMETRY library.
 *      It is part of a framework called GUCEF.
 */

#ifndef GUCEF_GEOMETRY_H
#define GUCEF_GEOMETRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration
 */
#ifndef GUCEF_GEOMETRY_CONFIG_H
#include "gucefGEOMETRY_config.h"
#define GUCEF_GEOMETRY_CONFIG_H
#endif /* GUCEF_GEOMETRY_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_GEOMETRY_ETYPES_H
#include "gucefGEOMETRY_ETypes.h"
#define GUCEF_GEOMETRY_ETYPES_H
#endif /* GUCEF_GEOMETRY_ETYPES_H ? */

#ifdef __cplusplus

#ifndef GUCEF_GEOMETRY_CMODULE_H
#include "gucefGEOMETRY_CModule.h"
#define GUCEF_GEOMETRY_CMODULE_H
#endif /* GUCEF_GEOMETRY_CMODULE_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_H ? */
