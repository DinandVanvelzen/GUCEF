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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTREAMEREVENTS_H
#include "CStreamerEvents.h"
#define GUCEF_CORE_CSTREAMEREVENTS_H
#endif /* GUCEF_CORE_CSTREAMEREVENTS_H ? */

#ifndef GUCEF_CORE_CNOTIFYINGMAPEVENTS_H
#include "CNotifyingMapEvents.h"
#define GUCEF_CORE_CNOTIFYINGMAPEVENTS_H
#endif /* GUCEF_CORE_CNOTIFYINGMAPEVENTS_H ? */

#ifndef GUCEF_CORE_CFILEURLHANDLER_H
#include "CFileURLHandler.h"
#define GUCEF_CORE_CFILEURLHANDLER_H
#endif /* GUCEF_CORE_CFILEURLHANDLER_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"               /* centralized configuration management */
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CTASKDELEGATOR_H
#include "gucefCORE_CTaskDelegator.h"
#define GUCEF_CORE_CTASKDELEGATOR_H
#endif /* GUCEF_CORE_CTASKDELEGATOR_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CITaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CINIDATASTORECODEC_H
#include "gucefCORE_CIniDataStoreCodec.h"
#define GUCEF_CORE_CINIDATASTORECODEC_H
#endif /* GUCEF_CORE_CINIDATASTORECODEC_H ? */

#ifndef GUCEF_CORE_CDSTOREBINARYCODEC_H
#include "gucefCORE_CDStoreBinaryCodec.h"
#define GUCEF_CORE_CDSTOREBINARYCODEC_H
#endif /* GUCEF_CORE_CDSTOREBINARYCODEC_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#ifndef GUCEF_CORE_CMETRICSCLIENTMANAGER_H
#include "gucefCORE_CMetricsClientManager.h"
#define GUCEF_CORE_CMETRICSCLIENTMANAGER_H
#endif /* GUCEF_CORE_CMETRICSCLIENTMANAGER_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CSYSCONSOLE_H
#include "CSysConsole.h"
#define GUCEF_CORE_CSYSCONSOLE_H
#endif /* GUCEF_CORE_CSYSCONSOLE_H ? */

#ifndef GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#include "CNotificationIDRegistry.h"
#define GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#endif /* GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H ? */

#ifndef GUCEF_CORE_CURLHANDLERREGISTRY_H
#include "CURLHandlerRegistry.h"
#define GUCEF_CORE_CURLHANDLERREGISTRY_H
#endif /* GUCEF_CORE_CURLHANDLERREGISTRY_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#include "CDStoreCodecPluginManager.h"
#define GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H
#endif /* GUCEF_CORE_CDSTORECODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CGENERICPLUGINMANAGER_H
#include "CGenericPluginManager.h"
#define GUCEF_CORE_CGENERICPLUGINMANAGER_H
#endif /* GUCEF_CORE_CGENERICPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CSTDCODECPLUGINMANAGER_H
#include "CStdCodecPluginManager.h"
#define GUCEF_CORE_CSTDCODECPLUGINMANAGER_H
#endif /* GUCEF_CORE_CSTDCODECPLUGINMANAGER_H ? */

#ifndef GUCEF_CORE_CEXCLUSIVEACTIVATIONMANAGER_H
#include "CExclusiveActivationManager.h"
#define GUCEF_CORE_CEXCLUSIVEACTIVATIONMANAGER_H
#endif /* GUCEF_CORE_CEXCLUSIVEACTIVATIONMANAGER_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H 
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CFUNCTIONREGISTRY_H
#include "gucefCORE_CFunctionRegistry.h"
#define GUCEF_CORE_CFUNCTIONREGISTRY_H
#endif /* GUCEF_CORE_CFUNCTIONREGISTRY_H ? */

#ifndef GUCEF_CORE_CLOGGINGGLOBAL_H
#include "gucefCORE_CLoggingGlobal.h"
#define GUCEF_CORE_CLOGGINGGLOBAL_H
#endif /* GUCEF_CORE_CLOGGINGGLOBAL_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CFILESYSTEMURIRESOURCEACCESSOR_H
#include "gucefCORE_CFileSystemUriResourceAccessor.h"
#define GUCEF_CORE_CFILESYSTEMURIRESOURCEACCESSOR_H
#endif /* GUCEF_CORE_CFILESYSTEMURIRESOURCEACCESSOR_H ? */

#ifndef GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H
#include "gucefCORE_CDataUriResourceAccessor.h"
#define GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H
#endif /* GUCEF_CORE_CDATAURIRESOURCEACCESSOR_H ? */

#ifndef GUCEF_CORE_CGLOBALDATADRIVENDSTORECODECFACTORY_H
#include "gucefCORE_CGlobalDataDrivenDStoreCodecFactory.h"
#define GUCEF_CORE_CGLOBALDATADRIVENDSTORECODECFACTORY_H
#endif /* GUCEF_CORE_CGLOBALDATADRIVENDSTORECODECFACTORY_H ? */

#if GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN

  #ifndef GUCEF_CORE_CWNDMSGHOOKNOTIFIER_H
  #include "CWndMsgHookNotifier.h"
  #define GUCEF_CORE_CWNDMSGHOOKNOTIFIER_H
  #endif /* GUCEF_CORE_CWNDMSGHOOKNOTIFIER_H ? */

  #ifndef GUCEF_CORE_CPPMSWINUTILS_H
  #include "gucefCORE_cppmswinutils.h"
  #define GUCEF_CORE_CPPMSWINUTILS_H
  #endif /* GUCEF_CORE_CPPMSWINUTILS_H ? */

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

  #ifndef GUCEF_CORE_CX11EVENTDISPATCHER_H
  #include "gucefCORE_CX11EventDispatcher.h"
  #define GUCEF_CORE_CX11EVENTDISPATCHER_H
  #endif /* GUCEF_CORE_CX11EVENTDISPATCHER_H ? */

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ? */

#include "gucefCORE_CCoreGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTFactory< CUriResourceAccessor, CFileSystemUriResourceAccessor, MT::CMutex >     TFileSystemUriResourceAccessorFactory;
typedef CTFactory< CUriResourceAccessor, CDataUriResourceAccessor, MT::CMutex >           TDataUriResourceAccessorFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CCoreGlobal::g_dataLock;
CCoreGlobal* CCoreGlobal::g_instance = NULL;

TFileSystemUriResourceAccessorFactory g_fileSystemUriResourceAccessorFactory;
TDataUriResourceAccessorFactory g_dataUriResourceAccessorFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void GUCEF_HIDDEN
AtExitApplicationStop( void )
{GUCEF_TRACE;

    CCoreGlobal::Instance()->GetApplication().Stop( true );
}

/*-------------------------------------------------------------------------*/

CCoreGlobal*
CCoreGlobal::Instance()
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CCoreGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                
                // atexit() is FILO
                // We want to make sure we properly stop the app before we destroy things 
                // as such we hook up a shutdown via atexit() as well to help ordering
                atexit( CCoreGlobal::Deinstance );
                atexit( AtExitApplicationStop );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CCoreGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CCoreGlobal::Initialize( void )
{GUCEF_TRACE;

    /*
     *      Very important: Initialize the memory manager second, after the stack tracer !!!!!
     */
    #if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && !defined( GUCEF_DYNNEWON_DISABLED ) )
    DRGUP_Initialize();
    DRGUP_SetExhaustiveTesting( 0 );
    DRGUP_SetPaddingSize( 0 );    
    CString memLogFilePath = RelativePath( "$MODULEDIR$/GUCEFMemoryLog.txt" );
    DRGUP_SetLogFile( memLogFilePath.C_String() );
    #endif

    /*
     *  Instantiate all the singletons
     *
     *  We start with the log manager so that it is possible to log everything from that point on
     *  if a logger is registered at an early stage
     */
    m_logManager = &CLoggingGlobal::Instance()->GetLogManager();
    m_notificationIdRegistry = GUCEF_NEW CNotificationIDRegistry();
    m_pulseGenerator = CTSharedObjCreator< CPulseGenerator, MT::CMutex >::CreateSharedObj();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    // Windows components access has no other dependencies, so we can create it right away
    CWindowsComponentAccess::Instance(); 
    
    #endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

    /*
     *  Make sure all events are registered from the start
     */
    CConfigStore::RegisterEvents();
    CNotifier::RegisterEvents();
    CPulseGenerator::RegisterEvents();
    CGUCEFApplication::RegisterEvents();
    CPluginControl::RegisterEvents();
    CStreamerEvents::RegisterEvents();
    CTimer::RegisterEvents();    
    CIURLEvents::RegisterEvents();
    CNotifyingMapEvents::RegisterEvents();    
    CTaskDelegator::RegisterEvents();
    CTaskConsumer::RegisterEvents();
    CTaskManager::RegisterEvents();
    CTONRegistryEvents::RegisterEvents();

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    CX11EventDispatcher::RegisterEvents();
    
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    CWndMsgHookNotifier::RegisterEvents();
    
    #endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

    /*
     *  Next the Config Store because everything that used the global config will need it including perhaps
     *  other singletons
     */
    m_configStore = GUCEF_NEW CConfigStore();
    m_metricsClientManager = GUCEF_NEW CMetricsClientManager();

    /*
     *  The application is the main 'singleton'big game loop' orchestrater
     *  and it is important to have it up and running as soon as possible
     *  so that we can use it for the rest of the initialization process and also to make sure it is ready to
     *  receive events from the start of the app lifecycle
     */
    m_application = GUCEF_NEW CGUCEFApplication();
    m_application->SetPulseGenerator( m_pulseGenerator );
    m_logManager->SetPulseGenerator( m_pulseGenerator );

    /*
     *  Instantiate the rest of the singletons
     */    
    m_sysConsole = GUCEF_NEW CSysConsole();
    m_taskManager = GUCEF_NEW CTaskManager();
    m_dstoreCodecRegistry = GUCEF_NEW CDStoreCodecRegistry();
    m_dataDrivenDStoreCodecFactory = GUCEF_NEW CGlobalDataDrivenDStoreCodecFactory();
    m_codecRegistry = GUCEF_NEW CCodecRegistry();
    m_pluginControl = GUCEF_NEW CPluginControl();
    m_dstoreCodecPluginManager = GUCEF_NEW CDStoreCodecPluginManager();
    m_genericPluginManager = GUCEF_NEW CGenericPluginManager();
    m_stdCodecPluginManager = GUCEF_NEW CStdCodecPluginManager();        
    m_urlHandlerRegistry = GUCEF_NEW CURLHandlerRegistry();
    m_uriResourceAccessorFactory = GUCEF_NEW CUriResourceAccessorFactory();
    m_functionRegistry = GUCEF_NEW CFunctionRegistry();

    /*
     *      Register some default URI handlers
     */
    m_urlHandlerRegistry->Register( "file", CURLHandlerRegistry::TRegisteredObjTypedPtr( GUCEF_NEW CFileURLHandler() ) ); // <- legacy version. @deprecated
    m_uriResourceAccessorFactory->RegisterConcreteFactory( CFileSystemUriResourceAccessor::SchemeName, &g_fileSystemUriResourceAccessorFactory );
    m_uriResourceAccessorFactory->RegisterConcreteFactory( CDataUriResourceAccessor::SchemeName, &g_dataUriResourceAccessorFactory );

    /*
     *      Register some default codecs
     */    
    m_dstoreCodecRegistry->Register( "ini", CDStoreCodecRegistry::TRegisteredObjTypedPtr( GUCEF_NEW CIniDataStoreCodec() ) );
    m_dstoreCodecRegistry->Register( "dnc", CDStoreCodecRegistry::TRegisteredObjTypedPtr( GUCEF_NEW CDataStoreBinaryCodec() ) );

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "gucefCORE Global systems initialized" );
    m_pluginControl->ListModuleForLinkBack( "gucefCORE" );
}

/*-------------------------------------------------------------------------*/

CCoreGlobal::CCoreGlobal( void )
    : m_pulseGenerator()
    , m_taskManager( GUCEF_NULL )                
    , m_urlHandlerRegistry( GUCEF_NULL )         
    , m_dstoreCodecRegistry( GUCEF_NULL )        
    , m_dataDrivenDStoreCodecFactory( GUCEF_NULL )
    , m_exclusiveActivationManager( GUCEF_NULL ) 
    , m_application( GUCEF_NULL )                
    , m_logManager( GUCEF_NULL )                
    , m_metricsClientManager( GUCEF_NULL )       
    , m_dstoreCodecPluginManager( GUCEF_NULL )   
    , m_genericPluginManager( GUCEF_NULL )       
    , m_pluginControl( GUCEF_NULL )              
    , m_sysConsole( GUCEF_NULL )                 
    , m_notificationIdRegistry( GUCEF_NULL )     
    , m_stdCodecPluginManager( GUCEF_NULL )      
    , m_configStore( GUCEF_NULL )                
    , m_codecRegistry( GUCEF_NULL )
    , m_functionRegistry( GUCEF_NULL )
    , m_uriResourceAccessorFactory( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCoreGlobal::~CCoreGlobal()
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "Shutting down gucefCORE global systems" );

    m_application->Stop( true );
    m_taskManager->RequestAllThreadsToStop( true, false, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
   
    m_uriResourceAccessorFactory->UnregisterAllConcreteFactories();
    m_urlHandlerRegistry->UnregisterAll();
    m_codecRegistry->UnregisterAll();
    m_dstoreCodecRegistry->UnregisterAll();
    m_dataDrivenDStoreCodecFactory->UnregisterAllConcreteFactories();
    m_pluginControl->UnlistModuleForLinkBack( "gucefCORE" );
    m_pluginControl->UnloadAll();
   

    m_logManager->FlushLogs();

    /*
     *      cleanup all singletons
     *      Take care to deinstance them in the correct order !!!
     */
    GUCEF_DELETE m_taskManager;
    m_taskManager = GUCEF_NULL;
    GUCEF_DELETE m_urlHandlerRegistry;
    m_urlHandlerRegistry = GUCEF_NULL;
    GUCEF_DELETE m_uriResourceAccessorFactory;
    m_uriResourceAccessorFactory = GUCEF_NULL;
    GUCEF_DELETE m_dstoreCodecRegistry;
    m_dstoreCodecRegistry = GUCEF_NULL;
    GUCEF_DELETE static_cast< CGlobalDataDrivenDStoreCodecFactory* >( m_dataDrivenDStoreCodecFactory );
    m_dataDrivenDStoreCodecFactory = GUCEF_NULL;
    GUCEF_DELETE m_exclusiveActivationManager;
    m_exclusiveActivationManager = GUCEF_NULL;
    GUCEF_DELETE m_application;
    m_application = GUCEF_NULL;
    CLoggingGlobal::Deinstance();
    m_logManager = GUCEF_NULL;
    GUCEF_DELETE m_metricsClientManager;
    m_metricsClientManager = GUCEF_NULL;
    GUCEF_DELETE m_dstoreCodecPluginManager;
    m_dstoreCodecPluginManager = GUCEF_NULL;
    GUCEF_DELETE m_genericPluginManager;
    m_genericPluginManager = GUCEF_NULL;
    GUCEF_DELETE m_stdCodecPluginManager;
    m_stdCodecPluginManager = GUCEF_NULL;
    GUCEF_DELETE m_pluginControl;
    m_pluginControl = GUCEF_NULL;
    GUCEF_DELETE m_sysConsole;
    m_sysConsole = GUCEF_NULL;
    GUCEF_DELETE m_notificationIdRegistry;
    m_notificationIdRegistry = GUCEF_NULL;
    GUCEF_DELETE m_configStore;
    m_configStore = GUCEF_NULL;
    GUCEF_DELETE m_codecRegistry;
    m_codecRegistry = GUCEF_NULL;
    GUCEF_DELETE m_functionRegistry;
    m_functionRegistry = GUCEF_NULL;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    
    CWindowsComponentAccess::Deinstance();
    
    #endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */
}

/*-------------------------------------------------------------------------*/

void
CCoreGlobal::Log( const TLogMsgType logMsgType ,
                  const Int32 logLevel         ,
                  const CString& logMessage    )
{
    if ( GUCEF_NULL != m_logManager  )
        m_logManager->Log( logMsgType, logLevel, logMessage ); 
}

/*-------------------------------------------------------------------------*/

void 
CCoreGlobal::Log( const TLogMsgType logMsgType ,
                  const Int32 logLevel         ,
                  const CString& logMessage    ,
                  const UInt32 threadId        ,
                  const CTimestamp& timestamp  )
{
    if ( GUCEF_NULL != m_logManager  )
        m_logManager->Log( logMsgType, logLevel, logMessage, threadId, timestamp );
}

/*-------------------------------------------------------------------------*/

CConfigStore&
CCoreGlobal::GetConfigStore( void )
{GUCEF_TRACE;

    return *m_configStore;
}

/*-------------------------------------------------------------------------*/

CGUCEFApplication&
CCoreGlobal::GetApplication( void )
{GUCEF_TRACE;

    return *m_application;
}

/*-------------------------------------------------------------------------*/

PulseGeneratorPtr
CCoreGlobal::GetPulseGenerator( void )
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CTaskManager&
CCoreGlobal::GetTaskManager( void )
{GUCEF_TRACE;

    return *m_taskManager;
}

/*-------------------------------------------------------------------------*/

CLogManager&
CCoreGlobal::GetLogManager( void )
{GUCEF_TRACE;

    return *m_logManager;
}

/*-------------------------------------------------------------------------*/

CMetricsClientManager&
CCoreGlobal::GetMetricsClientManager( void )
{
    return *m_metricsClientManager;
}

/*-------------------------------------------------------------------------*/

CPluginControl&
CCoreGlobal::GetPluginControl( void )
{GUCEF_TRACE;

    return *m_pluginControl;
}

/*-------------------------------------------------------------------------*/

CSysConsole&
CCoreGlobal::GetSysConsole( void )
{GUCEF_TRACE;

    return *m_sysConsole;
}

/*-------------------------------------------------------------------------*/

CNotificationIDRegistry&
CCoreGlobal::GetNotificationIDRegistry( void )
{GUCEF_TRACE;

    return *m_notificationIdRegistry;
}

/*-------------------------------------------------------------------------*/

CURLHandlerRegistry&
CCoreGlobal::GetUrlHandlerRegistry( void )
{GUCEF_TRACE;

    return *m_urlHandlerRegistry;
}

/*-------------------------------------------------------------------------*/

CUriResourceAccessorFactory& 
CCoreGlobal::GetUriResourceAccessorFactory( void )
{GUCEF_TRACE;

    return *m_uriResourceAccessorFactory;
}

/*-------------------------------------------------------------------------*/

CDStoreCodecRegistry&
CCoreGlobal::GetDStoreCodecRegistry( void )
{GUCEF_TRACE;

    return *m_dstoreCodecRegistry;
}

/*-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecFactory&
CCoreGlobal::GetDataDrivenDStoreCodecFactory( void )
{GUCEF_TRACE;

    return *m_dataDrivenDStoreCodecFactory;
}

/*-------------------------------------------------------------------------*/

CDStoreCodecPluginManager&
CCoreGlobal::GetDStoreCodecPluginManager( void )
{GUCEF_TRACE;

    return *m_dstoreCodecPluginManager;
}

/*-------------------------------------------------------------------------*/

CGenericPluginManager&
CCoreGlobal::GetGenericPluginManager( void )
{GUCEF_TRACE;

    return *m_genericPluginManager;
}

/*-------------------------------------------------------------------------*/

CStdCodecPluginManager&
CCoreGlobal::GetStdCodecPluginManager( void )
{GUCEF_TRACE;

    return *m_stdCodecPluginManager;
}

/*-------------------------------------------------------------------------*/

CExclusiveActivationManager&
CCoreGlobal::GetExclusiveActivationManager( void )
{GUCEF_TRACE;

    return *m_exclusiveActivationManager;
}

/*-------------------------------------------------------------------------*/

CCodecRegistry&
CCoreGlobal::GetCodecRegistry( void )
{GUCEF_TRACE;

    return *m_codecRegistry;
}

/*-------------------------------------------------------------------------*/

CFunctionRegistry&
CCoreGlobal::GetFunctionRegistry( void )
{GUCEF_TRACE;

    return *m_functionRegistry;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
