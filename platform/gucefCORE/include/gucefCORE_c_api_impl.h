/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* 
 *  This header provides a basic C interface to some basic CORE functionality
 *  The purpose of these functions is to provide a way to link against the
 *  C++ module and start a sequence of events (trigger use-cases) without being
 *  tied to C++ linking. Instead you can do dynamic linking against these C
 *  functions to provide a more resiliant bridge to C++ modules, isolating you
 *  from changes within the C++ code which would normally require re-linking.
 */
 
#ifndef GUCEF_CORE_C_API_IMP_H
#define GUCEF_CORE_C_API_IMP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_C_API_H
#include "gucefCORE_c_api.h"
#define GUCEF_CORE_C_API_H
#endif /* GUCEF_CORE_C_API_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
   extern "C" {
#endif   /* __cplusplus */

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C void
GUCEF_CORE_LinkCApi( TGucefCoreCApi* cApi );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C int
GUCEF_CORE_GucefMain( int argc    ,
                      char** argv , 
                      int runBool );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C int
GUCEF_CORE_GucefLoadConfig( const char* configPath ,
                            const char* dataCodec  );

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C int
GUCEF_CORE_GucefLoadPlugin( const char* pluginPath , 
                            const char* pluginType ,
                            const char* groupName  ,
                            int argc               , 
                            char** argv            );


/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C void
GUCEF_CORE_GucefAddPluginDir( const char* pluginDir );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C void
GUCEF_CORE_GucefSetDefaultPluginLoaderLogicType( const char* defaultLoaderLogicType );

/*--------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C void
GUCEF_CORE_GucefLog( int logType     , 
                     int logLevel    , 
                     const char* msg );

/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_C_API_IMP_H ? */
