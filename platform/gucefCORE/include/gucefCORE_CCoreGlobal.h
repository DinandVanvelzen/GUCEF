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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#define GUCEF_CORE_CCOREGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_LOGTYPES_H
#include "gucefCORE_LogTypes.h"
#define GUCEF_CORE_LOGTYPES_H
#endif /* GUCEF_CORE_LOGTYPES_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDateTime;
class CUriResourceAccessorFactory;
class CTaskManager;
class CURLHandlerRegistry;
class CDStoreCodecRegistry;
class CDataDrivenDStoreCodecFactory;
class CExclusiveActivationManager;
class CGUCEFApplication;
class CLogManager;
class CMetricsClientManager;
class CDStoreCodecPluginManager;
class CGenericPluginManager;
class CPluginControl;
class CSysConsole;
class CNotificationIDRegistry;
class CStdCodecPluginManager;
class CConfigStore;
class CCodecRegistry;
class CFunctionRegistry;

//typedef CTBasicSharedPtr< CPulseGenerator, MT::CMutex > BasicPulseGeneratorPtr;

/*-------------------------------------------------------------------------*/

/**
 *  Singular singleton providing access to all global Core systems
 */
class GUCEF_CORE_PUBLIC_CPP CCoreGlobal
{
    public:

    static CCoreGlobal* Instance( void );

    CGUCEFApplication& GetApplication( void );

    PulseGeneratorPtr GetPulseGenerator( void );

    CTaskManager& GetTaskManager( void );

    CLogManager& GetLogManager( void );

    CMetricsClientManager& GetMetricsClientManager( void );

    CPluginControl& GetPluginControl( void );

    CSysConsole& GetSysConsole( void );

    CConfigStore& GetConfigStore( void );

    CNotificationIDRegistry& GetNotificationIDRegistry( void );

    CURLHandlerRegistry& GetUrlHandlerRegistry( void );

    CUriResourceAccessorFactory& GetUriResourceAccessorFactory( void );

    CDStoreCodecRegistry& GetDStoreCodecRegistry( void );

    CDataDrivenDStoreCodecFactory& GetDataDrivenDStoreCodecFactory( void );

    CDStoreCodecPluginManager& GetDStoreCodecPluginManager( void );

    CGenericPluginManager& GetGenericPluginManager( void );

    CStdCodecPluginManager& GetStdCodecPluginManager( void );

    CExclusiveActivationManager& GetExclusiveActivationManager( void );

    CCodecRegistry& GetCodecRegistry( void );

    CFunctionRegistry& GetFunctionRegistry( void );

    /**
     *  Logging proxy call provided here to avoid including the logging manager 
     *  with its more involved dependencies
     */
    void Log( const TLogMsgType logMsgType ,
              const Int32 logLevel         ,
              const CString& logMessage    );

    /**
     *  Logging proxy call provided here to avoid including the logging manager 
     *  with its more involved dependencies
     */
    void Log( const TLogMsgType logMsgType ,
              const Int32 logLevel         ,
              const CString& logMessage    ,
              const UInt32 threadId        ,
              const CDateTime& timestamp   );

    private:
    friend class CGUCEFCOREModule;

    static void Deinstance( void );

    private:

    CCoreGlobal( void );

    ~CCoreGlobal();

    void Initialize( void );

    private:

    PulseGeneratorPtr m_pulseGenerator;
    CTaskManager* m_taskManager;
    CURLHandlerRegistry* m_urlHandlerRegistry;
    CDStoreCodecRegistry* m_dstoreCodecRegistry;
    CDataDrivenDStoreCodecFactory* m_dataDrivenDStoreCodecFactory;
    CExclusiveActivationManager* m_exclusiveActivationManager;
    CGUCEFApplication* m_application;
    CLogManager* m_logManager;
    CMetricsClientManager* m_metricsClientManager;
    CDStoreCodecPluginManager* m_dstoreCodecPluginManager;
    CGenericPluginManager* m_genericPluginManager;
    CPluginControl* m_pluginControl;
    CSysConsole* m_sysConsole;
    CNotificationIDRegistry* m_notificationIdRegistry;
    CStdCodecPluginManager* m_stdCodecPluginManager;
    CConfigStore* m_configStore;
    CCodecRegistry* m_codecRegistry;
    CFunctionRegistry* m_functionRegistry;
    CUriResourceAccessorFactory* m_uriResourceAccessorFactory;

    static MT::CMutex g_dataLock;
    static CCoreGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
