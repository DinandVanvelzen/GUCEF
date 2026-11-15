/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMCPJOBWAITER_H
#define GUCEF_COM_CMCPJOBWAITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COM_CMCPREQUESTCONTEXT_H
#include "gucefCOM_CMcpRequestContext.h"
#define GUCEF_COM_CMCPREQUESTCONTEXT_H
#endif /* GUCEF_COM_CMCPREQUESTCONTEXT_H ? */

#ifndef GUCEF_COM_CIMCPJOBRESULTBUILDER_H
#include "gucefCOM_CIMcpJobResultBuilder.h"
#define GUCEF_COM_CIMCPJOBRESULTBUILDER_H
#endif /* GUCEF_COM_CIMCPJOBRESULTBUILDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CMcpJobBridge;

/**
 *  Engine internal of CMcpJobBridge: one call that waits for a job. A timer on the pulse generator of the call's
 *  request context asks the bridge to look at the run; the timer is switched off on its own thread once the call is
 *  completed, cancelled or abandoned, and the bridge deletes the waiter later. Lock order: the waiter, then the bridge.
 */
class GUCEF_COM_PRIVATE_CPP CMcpJobWaiter : public CORE::CObserver
{
    public:

    CMcpJobWaiter( CMcpJobBridge* bridge                      ,
                   const CMcpRequestContextPtr& context        ,
                   const CORE::CString& jobId                  ,
                   UInt64 deadlineMonotonicMs                  ,
                   const CIMcpJobResultBuilderPtr& builder     ,
                   const CORE::PulseGeneratorPtr& pulseGenerator ,
                   UInt32 pollIntervalMs                       );

    virtual ~CMcpJobWaiter() GUCEF_VIRTUAL_OVERRIDE;

    bool Start( void );

    bool IsDone( void ) const;

    /** The bridge goes: no calls into it from now on; a call that still waits is completed with 'result' first */
    void Abandon( const CMcpToolResult* result );

    const CMcpRequestContextPtr& GetContext( void ) const;
    const CORE::CString& GetJobId( void ) const;
    UInt64 GetDeadlineMonotonicMs( void ) const;
    const CIMcpJobResultBuilderPtr& GetBuilder( void ) const;

    /** The progress last forwarded to the client */
    UInt64 lastProgressDone;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CMcpJobWaiter > TEventCallback;

    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CMcpJobWaiter( const CMcpJobWaiter& src );              /**< not implemented */
    CMcpJobWaiter& operator=( const CMcpJobWaiter& src );   /**< not implemented */

    MT::CMutex m_lock;
    CMcpJobBridge* m_bridge;
    CMcpRequestContextPtr m_context;
    CORE::CString m_jobId;
    UInt64 m_deadlineMonotonicMs;
    CIMcpJobResultBuilderPtr m_builder;
    CORE::CTimer m_timer;
    MT::CTAtomic< UInt32 > m_done;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPJOBWAITER_H ? */
