/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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
class CTimestamp;
class CUriResourceAccessorFactory;
class CTaskManager;
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
class CBuiltInCodecs;
class CFunctionRegistry;
class CDataTableBufferPool;
class CTableCodecRegistry;
class CTableCodecPluginManager;

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


    CUriResourceAccessorFactory& GetUriResourceAccessorFactory( void );

    CDStoreCodecRegistry& GetDStoreCodecRegistry( void );

    CDataDrivenDStoreCodecFactory& GetDataDrivenDStoreCodecFactory( void );

    CDStoreCodecPluginManager& GetDStoreCodecPluginManager( void );

    CGenericPluginManager& GetGenericPluginManager( void );

    CStdCodecPluginManager& GetStdCodecPluginManager( void );

    CExclusiveActivationManager& GetExclusiveActivationManager( void );

    CCodecRegistry& GetCodecRegistry( void );

    /** The codecs gucefCORE brings itself: software and OS crypto, see CBuiltInCodecs */
    CBuiltInCodecs& GetBuiltInCodecs( void );

    CFunctionRegistry& GetFunctionRegistry( void );

    /** The default pool of data table buffers and batch objects; tables may be given their own */
    CDataTableBufferPool& GetDataTableBufferPool( void );

    /** The table codecs by type name: csv is built in, others come from table codec plugins */
    CTableCodecRegistry& GetTableCodecRegistry( void );

    /** Loads and unloads table codec plugins, plugin type "GucefTableCodecPlugin" */
    CTableCodecPluginManager& GetTableCodecPluginManager( void );

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
              const CTimestamp& timestamp  );

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
    CBuiltInCodecs* m_builtInCodecs;
    CFunctionRegistry* m_functionRegistry;
    CDataTableBufferPool* m_dataTableBufferPool;
    CTableCodecRegistry* m_tableCodecRegistry;
    CTableCodecPluginManager* m_tableCodecPluginManager;
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
