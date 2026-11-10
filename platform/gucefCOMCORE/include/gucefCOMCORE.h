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

/*
 *      This is the main include header for the gucefCOMCORE library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do with basic communication facilitys
 */

#ifndef GUCEF_COMCORE_H
#define GUCEF_COMCORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Library build configuration      
 */
#ifndef GUCEFCOMCORE_CONFIG_H
#include "gucefCOMCORE_config.h"
#define GUCEFCOMCORE_CONFIG_H
#endif /* GUCEFCOMCORE_CONFIG_H ? */

/*
 *      Library specific macros and generic macros
 */
#ifndef GUCEFCOMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEFCOMCORE_MACROS_H
#endif /* GUCEFCOMCORE_MACROS_H ? */

/*
 *      Simple types
 */
#ifndef GUCEF_COMCORE_ETYPES_H
#include "gucefCOMCORE_ETypes.h"
#define GUCEF_COMCORE_ETYPES_H
#endif /* GUCEF_COMCORE_ETYPES_H ? */

#ifndef SOCKETERRORS_H
#include "socketerrors.h" 
#define SOCKETERRORS_H
#endif /* SOCKETERRORS_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_COMCORE_CCOM_H
#include "CCom.h"
#define GUCEF_COMCORE_CCOM_H
#endif /* GUCEF_COMCORE_CCOM_H ? */

#ifndef GUCEF_COMCORE_CSOCKET_H
#include "CSocket.h"
#define GUCEF_COMCORE_CSOCKET_H
#endif /* GUCEF_COMCORE_CSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */ 

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CPING_H
#include "CPing.h"
#define GUCEF_COMCORE_CPING_H
#endif /* GUCEF_COMCORE_CPING_H ? */

#ifndef GUCEF_COMCORE_CUDPMASTERSOCKET_H
#include "CUDPMasterSocket.h"
#define GUCEF_COMCORE_CUDPMASTERSOCKET_H
#endif /* GUCEF_COMCORE_CUDPMASTERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CUDPCHANNEL_H
#include "CUDPChannel.h"
#define GUCEF_COMCORE_CUDPCHANNEL_H
#endif /* GUCEF_COMCORE_CUDPCHANNEL_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_COMCORE_CIPV4ADDRESS_H
#include "gucefCOMCORE_CIPv4Address.h"
#define GUCEF_COMCORE_CIPV4ADDRESS_H
#endif /* GUCEF_COMCORE_CIPV4ADDRESS_H ? */

#ifndef GUCEF_COMCORE_CIPV6ADDRESS_H
#include "gucefCOMCORE_CIPv6Address.h"
#define GUCEF_COMCORE_CIPV6ADDRESS_H
#endif /* GUCEF_COMCORE_CIPV6ADDRESS_H ? */

#ifndef GUCEF_COMCORE_CDNSRESOLVER_H
#include "gucefCOMCORE_CDnsResolver.h"
#define GUCEF_COMCORE_CDNSRESOLVER_H
#endif /* GUCEF_COMCORE_CDNSRESOLVER_H ? */

#ifndef GUCEF_COMCORE_CDNSCACHE_H
#include "gucefCOMCORE_CDnsCache.h"
#define GUCEF_COMCORE_CDNSCACHE_H
#endif /* GUCEF_COMCORE_CDNSCACHE_H ? */

#ifndef GUCEF_COMCORE_CDNSCACHEENTRY_H
#include "gucefCOMCORE_CDnsCacheEntry.h"
#define GUCEF_COMCORE_CDNSCACHEENTRY_H
#endif /* GUCEF_COMCORE_CDNSCACHEENTRY_H ? */

#ifndef GUCEF_COMCORE_TLSTYPES_H
#include "gucefCOMCORE_TlsTypes.h"
#define GUCEF_COMCORE_TLSTYPES_H
#endif /* GUCEF_COMCORE_TLSTYPES_H ? */

#ifndef GUCEF_COMCORE_CTLSSETTINGS_H
#include "gucefCOMCORE_CTlsSettings.h"
#define GUCEF_COMCORE_CTLSSETTINGS_H
#endif /* GUCEF_COMCORE_CTLSSETTINGS_H ? */

#ifndef GUCEF_COMCORE_ITLSSESSION_H
#include "gucefCOMCORE_ITlsSession.h"
#define GUCEF_COMCORE_ITLSSESSION_H
#endif /* GUCEF_COMCORE_ITLSSESSION_H ? */

#ifndef GUCEF_COMCORE_ITLSPROVIDER_H
#include "gucefCOMCORE_ITlsProvider.h"
#define GUCEF_COMCORE_ITLSPROVIDER_H
#endif /* GUCEF_COMCORE_ITLSPROVIDER_H ? */

#ifndef GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H
#include "gucefCOMCORE_CTlsProviderRegistry.h"
#define GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H
#endif /* GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H ? */

#ifndef GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#include "gucefCOMCORE_CTlsClientConnection.h"
#define GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#endif /* GUCEF_COMCORE_CTLSCLIENTCONNECTION_H ? */

#ifndef GUCEF_COMCORE_COPENSSLTLSPROVIDER_H
#include "gucefCOMCORE_COpenSslTlsProvider.h"
#define GUCEF_COMCORE_COPENSSLTLSPROVIDER_H
#endif /* GUCEF_COMCORE_COPENSSLTLSPROVIDER_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#endif /* __cplusplus ? */
/*--------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_H ? */
