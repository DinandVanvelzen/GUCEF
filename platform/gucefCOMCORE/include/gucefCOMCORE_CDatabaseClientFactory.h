/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CDATABASECLIENTFACTORY_H
#define GUCEF_COMCORE_CDATABASECLIENTFACTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CIDATABASECLIENT_H
#include "gucefCOMCORE_CIDatabaseClient.h"
#define GUCEF_COMCORE_CIDATABASECLIENT_H
#endif /* GUCEF_COMCORE_CIDATABASECLIENT_H ? */

/*-------------------------------------------------------------------------*/

/*
 *  CDatabaseClientFactory and CIDatabaseClientPtr are declared with CIDatabaseClient, in the way that CRttProbeFactory
 *  and CPortMapperFactory are declared with their interfaces. This header is the include for code that only creates clients:
 *
 *      typedef CORE::CTAbstractFactory< CORE::CString, CIDatabaseClient, MT::CMutex >  CDatabaseClientFactory;
 *      typedef CDatabaseClientFactory::TProductPtr                                      CIDatabaseClientPtr;
 */

#endif /* GUCEF_COMCORE_CDATABASECLIENTFACTORY_H ? */
