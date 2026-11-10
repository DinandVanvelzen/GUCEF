/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
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

/*
 *      This is the main include header for the gucefWEB library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do with implementations for
 *      standardized communication facilities
 */

#ifndef GUCEF_WEB_H
#define GUCEF_WEB_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration      
 */
#ifndef GUCEF_WEB_CONFIG_H
#include "gucefWEB_config.h"
#define GUCEF_WEB_CONFIG_H
#endif /* GUCEF_WEB_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */


#ifndef GUCEF_WEB_CASYNCHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CAsyncHttpServerRequestHandler.h"
#define GUCEF_WEB_CASYNCHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CASYNCHTTPSERVERREQUESTHANDLER_H ? */

#ifndef GUCEF_WEB_CASYNCHTTPSERVERRESPONSEHANDLER_H
#include "gucefWEB_CAsyncHttpServerResponseHandler.h"
#define GUCEF_WEB_CASYNCHTTPSERVERRESPONSEHANDLER_H
#endif /* GUCEF_WEB_CASYNCHTTPSERVERRESPONSEHANDLER_H ? */

#ifndef GUCEF_WEB_CGLOBALHTTPCODECLINKS_H
#include "gucefWEB_CGlobalHttpCodecLinks.h"
#define GUCEF_WEB_CGLOBALHTTPCODECLINKS_H
#endif /* GUCEF_WEB_CGLOBALHTTPCODECLINKS_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangeling
 */
#ifdef __cplusplus
   extern "C" {
#endif   /* __cplusplus */

/*-------------------------------------------------------------------------*/

GUCEF_WEB_PUBLIC_C const char* GUCEF_WEB_ModuleCompileTimeStamp( void );

/*-------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_H ? */

 
