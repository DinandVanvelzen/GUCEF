/*
 *  gucefCOM: GUCEF module providing communication implementations 
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DLLINIT_H
#include "gucef_DLLInit.h"
#define GUCEF_DLLINIT_H
#endif /* GUCEF_DLLINIT_H ? */

#ifndef CGUCEFCOMMODULE_H
#include "CGUCEFCOMModule.h"
#define CGUCEFCOMMODULE_H
#endif /* CGUCEFCOMMODULE_H ? */

#include "gucefCOM.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_DLL_INIT_FUNC_BEG_END( GUCEF::COM::CGUCEFCOMModule );

/*-------------------------------------------------------------------------*/

const char*
GUCEF_COM_ModuleCompileTimeStamp( void )
{GUCEF_TRACE;

    return __TIMESTAMP__;
}

/*-------------------------------------------------------------------------*/
