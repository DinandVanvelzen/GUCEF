/*
 *  pluginglueOGRECOMMON: Header only adapters between GUCEF and the Ogre 1.x API, compiled into the plugins that use them
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

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H
#define GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      GENERAL MACROS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The methods of the Ogre archive interface are const in the Ogre 1.x line and not const in Ogre-Next. The archive
 *  classes of this module are written once and take their constness from this macro, which a build of the other
 *  line defines as nothing before including the headers.
 */
#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST
  #define GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST const
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_ARCHIVE_CONST ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H ? */
