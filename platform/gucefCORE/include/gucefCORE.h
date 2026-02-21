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
 *      This is the main include header for the gucefCORE library.
 *      It is part of a framework called GUCEF.
 *      All classes in this library have to do core framework functionality
 *      and many of the other modules will have a dependancy on this module.
 *      It houses the generic toolkits.
 */

#ifndef GUCEF_CORE_H
#define GUCEF_CORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_GUCEFCORE_CONFIG_H
#include "gucefCORE_config.h"
#define GUCEF_CORE_GUCEFCORE_CONFIG_H
#endif /* GUCEF_CORE_GUCEFCORE_CONFIG_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_ESTRUCTS_H
#include "EStructs.h"
#define GUCEF_CORE_ESTRUCTS_H
#endif /* GUCEF_CORE_ESTRUCTS_H ? */

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

#ifndef GUCEF_CORE_DVSTRUTILS_H
#include "dvstrutils.h"
#define GUCEF_CORE_DVSTRUTILS_H
#endif /* GUCEF_CORE_DVSTRUTILS_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_MFILE_H
#include "MFILE.h"
#define GUCEF_CORE_MFILE_H
#endif /* GUCEF_CORE_MFILE_H ? */

#ifndef GUCEF_CORE_PAFILE_H
#include "PAFILE.h"
#define GUCEF_CORE_PAFILE_H
#endif /* GUCEF_CORE_PAFILE_H ? */

#ifndef GUCEF_CORE_C_IOACCESS_H
#include "gucefCORE_c_ioaccess.h"
#define GUCEF_CORE_C_IOACCESS_H
#endif /* GUCEF_CORE_C_IOACCESS_H ? */

/*---------------------------------------------------------------------------/

        C++ includes

/---------------------------------------------------------------------------*/
#ifdef __cplusplus

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_STREAMERS_H
#include "streamers.h"
#define GUCEF_CORE_STREAMERS_H
#endif /* GUCEF_CORE_STREAMERS_H ? */

#ifndef GUCEF_CORE_STREAMABLES_H
#include "streamables.h"
#define GUCEF_CORE_STREAMABLES_H
#endif /* GUCEF_CORE_STREAMABLES_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTRANSACTIONEVENTCONTROL_H
#include "CTransactionEventControl.h"
#define GUCEF_CORE_CTRANSACTIONEVENTCONTROL_H
#endif /* GUCEF_CORE_CTRANSACTIONEVENTCONTROL_H ? */

#ifndef GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#include "CNotificationIDRegistry.h"
#define GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#endif /* GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H ? */

#ifndef GUCEF_CORE_CGENERICPLUGINMANAGER_H
#include "CGenericPluginManager.h"
#define GUCEF_CORE_CGENERICPLUGINMANAGER_H
#endif /* GUCEF_CORE_CGENERICPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CSTACK_H
#include "CStack.h"
#define GUCEF_CORE_CSTACK_H
#endif /* GUCEF_CORE_CSTACK_H ? */

#ifndef GUCEF_CORE_CDATAQUEUE_H
#include "CDataQueue.h"
#define GUCEF_CORE_CDATAQUEUE_H
#endif /* GUCEF_CORE_CDATAQUEUE_H ? */

#ifndef GUCEF_CORE_CBLOCKSTACK_H
#include "CBlockStack.h"
#define GUCEF_CORE_CBLOCKSTACK_H
#endif /* GUCEF_CORE_CBLOCKSTACK_H ? */

#ifndef GUCEF_CORE_CMEMORYBLOCKPOOL_H
#include "CMemoryBlockPool.h"
#define GUCEF_CORE_CMEMORYBLOCKPOOL_H
#endif /* GUCEF_CORE_CMEMORYBLOCKPOOL_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CNODE_H
#include "CNode.h"
#define GUCEF_CORE_CNODE_H
#endif /* GUCEF_CORE_CNODE_H ? */

#ifndef GUCEF_CORE_CLLNODE_H
#include "CLLNode.h"
#define GUCEF_CORE_CLLNODE_H
#endif /* GUCEF_CORE_CLLNODE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneAble.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CPLUGINMANAGER_H
#include "CPluginManager.h"
#define GUCEF_CORE_CPLUGINMANAGER_H
#endif /* GUCEF_CORE_CPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERSTRINGSTREAM_H
#include "gucefCORE_CDynamicBufferStringStream.h"
#define GUCEF_CORE_CDYNAMICBUFFERSTRINGSTREAM_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERSTRINGSTREAM_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERSWAP_H
#include "gucefCORE_CDynamicBufferSwap.h"
#define GUCEF_CORE_CDYNAMICBUFFERSWAP_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERSWAP_H ? */

#ifndef GUCEF_CORE_CVARIANTSTREAM_H
#include "gucefCORE_CVariantStream.h"
#define GUCEF_CORE_CVARIANTSTREAM_H
#endif /* GUCEF_CORE_CVARIANTSTREAM_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CDSTORECODEC_H
#include "CDStoreCodec.h"
#define GUCEF_CORE_CDSTORECODEC_H
#endif /* GUCEF_CORE_CDSTORECODEC_H */

#ifndef GUCEF_CORE_CDSTORECODECPLUGIN_H
#include "CDStoreCodecPlugin.h"
#define GUCEF_CORE_CDSTORECODECPLUGIN_H
#endif /* GUCEF_CORE_CDSTORECODECPLUGIN_H */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#include "CDStoreCodecPluginManager.h"
#define GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#endif /* GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H
#include "gucefCORE_CDataDrivenDStoreCodecFactory.h"         
#define GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H
#endif /* GUCEF_CORE_CDATADRIVENDSTORECODECFACTORY_H ? */

#ifndef GUCEF_CORE_CTREGISTRY_H
#include "CTRegistry.h"
#define GUCEF_CORE_CTREGISTRY_H
#endif /* GUCEF_CORE_CTREGISTRY_H ? */

#ifndef GUCEF_CORE_CURI_H
#include "gucefCORE_CUri.h"
#define GUCEF_CORE_CURI_H
#endif /* GUCEF_CORE_CURI_H ? */

#ifndef GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H
#include "gucefCORE_CDataUriResourceAccessor.h"
#define GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H
#endif /* GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H ? */

#ifndef GUCEF_CORE_CURL_H
#include "CURL.h"
#define GUCEF_CORE_CURL_H
#endif /* GUCEF_CORE_CURL_H ? */

#ifndef GUCEF_CORE_CURLHANDLER_H
#include "CURLHandler.h"
#define GUCEF_CORE_CURLHANDLER_H
#endif /* GUCEF_CORE_CURLHANDLER_H ? */

#ifndef GUCEF_CORE_CURLHANDLERREGISTRY_H
#include "CURLHandlerRegistry.h"
#define GUCEF_CORE_CURLHANDLERREGISTRY_H
#endif /* GUCEF_CORE_CURLHANDLERREGISTRY_H ? */

#ifndef GUCEF_CORE_CURLDATARETRIEVER_H
#include "CURLDataRetriever.h"
#define GUCEF_CORE_CURLDATARETRIEVER_H
#endif /* GUCEF_CORE_CURLDATARETRIEVER_H ? */

#ifndef GUCEF_CORE_CSTATEMACHINE_H
#include "CStateMachine.h"
#define GUCEF_CORE_CSTATEMACHINE_H
#endif /* GUCEF_CORE_CSTATEMACHINE_H ? */

#ifndef GUCEF_CORE_CSTATEHANDLER_H
#include "CStateHandler.h"
#define GUCEF_CORE_CSTATEHANDLER_H
#endif /* GUCEF_CORE_CSTATEHANDLER_H ? */

#ifndef GUCEF_CORE_CSTDOSTREAMADAPTER_H
#include "CSTDOStreamAdapter.h"
#define GUCEF_CORE_CSTDOSTREAMADAPTER_H
#endif /* GUCEF_CORE_CSTDOSTREAMADAPTER_H ? */

#ifndef GUCEF_CORE_CIPLUGIN_H
#include "CIPlugin.h"
#define GUCEF_CORE_CIPLUGIN_H
#endif /* GUCEF_CORE_CIPLUGIN_H ? */

#ifndef GUCEF_CORE_CTLINKEDCLONEABLEOBJ_H
#include "CTLinkedCloneableObj.h"
#define GUCEF_CORE_CTLINKEDCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTLINKEDCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_CORE_CTBASICSHAREDPTR_H
#include "CTBasicSharedPtr.h"
#define GUCEF_CORE_CTBASICSHAREDPTR_H
#endif /* GUCEF_CORE_CTBASICSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CILOGGER_H
#include "CILogger.h"
#define GUCEF_CORE_CILOGGER_H
#endif /* GUCEF_CORE_CILOGGER_H ? */

#ifndef GUCEF_CORE_CSTDLOGGER_H
#include "CStdLogger.h"
#define GUCEF_CORE_CSTDLOGGER_H
#endif /* GUCEF_CORE_CSTDLOGGER_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CMSGEXCEPTION_H
#include "CMsgException.h"
#define GUCEF_CORE_CMSGEXCEPTION_H
#endif /* GUCEF_CORE_CMSGEXCEPTION_H ? */

#ifndef GUCEF_CORE_CEXCEPTION_H
#include "CException.h"
#define GUCEF_CORE_CEXCEPTION_H
#endif /* GUCEF_CORE_CEXCEPTION_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#include "gucefCORE_CPlatformNativeConsoleWindow.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H ? */

/*---------------------------------------------------------------------------/

        C++ MS Windows includes

/---------------------------------------------------------------------------*/
#if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN

/*
 *  Note that the following includes may require a include path to be added
 *  to the project since these are O/S specific and not common in the GUCEF
 *  code-base distribution.
 */

#ifndef GUCEF_CORE_CMSWINCONSOLELOGGER_H
#include "CMSWinConsoleLogger.h"
#define GUCEF_CORE_CMSWINCONSOLELOGGER_H
#endif /* GUCEF_CORE_CMSWINCONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CWINDOWMSGHOOK_H
#include "CWindowMsgHook.h"
#define GUCEF_CORE_CWINDOWMSGHOOK_H
#endif /* GUCEF_CORE_CWINDOWMSGHOOK_H ? */

#ifndef GUCEF_CORE_CMSWIN32WINDOW_H
#include "gucefCORE_CMsWin32Window.h"
#define GUCEF_CORE_CMSWIN32WINDOW_H
#endif /* GUCEF_CORE_CMSWIN32WINDOW_H ? */

#ifndef GUCEF_CORE_CMSWIN32EDITBOX_H
#include "gucefCORE_CMsWin32Editbox.h"
#define GUCEF_CORE_CMSWIN32EDITBOX_H
#endif /* GUCEF_CORE_CMSWIN32EDITBOX_H ? */

#ifndef GUCEF_CORE_CMSWIN32CONSOLEWINDOW_H
#include "gucefCORE_CMsWin32ConsoleWindow.h"
#define GUCEF_CORE_CMSWIN32CONSOLEWINDOW_H
#endif /* GUCEF_CORE_CMSWIN32CONSOLEWINDOW_H ? */

#ifndef GUCEF_CORE_MSWINUTILS_H
#include "gucefCORE_mswinutils.h"
#define GUCEF_CORE_MSWINUTILS_H
#endif /* GUCEF_CORE_MSWINUTILS_H ? */

#endif /* GUCEF_PLATFORM_MSWIN ? */

#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_C const char* 
GUCEF_CORE_ModuleCompileTimeStamp( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 07-06-2007 :
       - Dinand: Added cloneables
       - Dinand: Added streamers
       - Dinand: Added streamables
- 18-02-2007 :
       - Dinand: Added CLogManager
       - Dinand: Added CILogger
       - Dinand: Added CStdLogger
- 06-10-2006 :
       - Dinand: Added CTLinkedCloneableObj
       - Dinand: re-Added CTCloneableObj
- 30-09-2006 :
       - Dinand: re-Added CGenericPluginManager
       - Dinand: re-Added CIPlugin
       - Dinand: Removed CEventPump
       - Dinand: Removed CEventPumpClient
       - Dinand: Removed CStringStorage
       - Dinand: Removed CTRegistryMT
- 21-09-2005 :
       - Dinand: Added CValueList
- 15-05-2005 :
       - Dinand: Added CStateMachine
       - Dinand: Added CStateHandler
- 24-04-2005 :
       - Dinand: Added CTRegistryMT
       - Dinand: Added CTRegistry
       - Dinand: Added CURL
       - Dinand: Added CURLHandler
       - Dinand: Added CURLHandlerRegistry
- 08-04-2005 :
       - Dinand: Added CConfigStore
       - Dinand: Added CDStoreCodec
       - Dinand: Added CDStoreCodecPlugin
       - Dinand: Added CDStoreCodecPluginRef
       - Dinand: Added CDStoreCodecRegistry
       - Dinand: Added CDStoreCodecPluginManager
- 05-04-2005 :
       - Dinand: Added CDataNode
       - Dinand: Removed CXMLSegment
       - Dinand: Removed CXMLParser
       - Dinand: Removed CXMLComment
       - Dinand: Removed CXMLNode
- 11-03-2004 :
       - Dinand: Made the initial version of the library header. The classes here
         where originally part of one large GUCEF codebase without segmentation.

-----------------------------------------------------------------------------*/
