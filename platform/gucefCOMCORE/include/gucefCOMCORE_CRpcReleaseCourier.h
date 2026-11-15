/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCRELEASECOURIER_H
#define GUCEF_COMCORE_CRPCRELEASECOURIER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#ifndef GUCEF_COMCORE_CIRPCSTREAM_H
#include "gucefCOMCORE_CIRpcStream.h"
#define GUCEF_COMCORE_CIRPCSTREAM_H
#endif /* GUCEF_COMCORE_CIRPCSTREAM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Lets go of the last references of server calls and streams linked to one pulse generator, on that pulse
 *  generator's thread, once each object has no mail of its own waiting or in delivery (CORE::CTSGNotifier::HasMail()):
 *  a thread safe notifier may not go inside the delivery of its own mail, where a handler of its events (a CancelledEvent
 *  handler that lets go of its kept reference) would otherwise hold the last reference.
 *
 *  One courier per pulse generator, kept by CRpcReleaseCourierRegistry (CComCoreGlobal) beyond the endpoints that use
 *  it: an endpoint that goes while references it handed over still wait for their objects' mail does not take them
 *  along (finding B-7 of the gucefDRNUTILS freeze). Hold() may be called from any thread: it only mails the courier.
 *  A reference whose object keeps getting mail is let go of after at most MaxRounds pulses.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcReleaseCourier : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent ReleaseEvent;         /**< internal: mailed to this courier's own thread */

    static const UInt32 MaxRounds = 1000;

    static void RegisterEvents( void );

    explicit CRpcReleaseCourier( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CRpcReleaseCourier() GUCEF_VIRTUAL_OVERRIDE;

    /** Takes the references (the vectors are emptied); they go on the pulse generator's thread */
    void Hold( GUCEF::vector< CIRpcStreamPtr >& streams, GUCEF::vector< CRpcServerCallPtr >& calls );

    /** Nothing held and no mail of the courier itself */
    bool IsIdle( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CRpcReleaseCourier( const CRpcReleaseCourier& src );                /**< not implemented */
    CRpcReleaseCourier& operator=( const CRpcReleaseCourier& src );     /**< not implemented */

    void OnRelease( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    mutable MT::CMutex m_lock;                      /**< leaf lock: held to take and hand over the references only */
    GUCEF::vector< CIRpcStreamPtr > m_streams;
    GUCEF::vector< CRpcServerCallPtr > m_calls;
    UInt32 m_rounds;                                /**< the pulses the current references waited for their objects' mail */
    bool m_mailed;                                  /**< a ReleaseEvent is on its way */
    CORE::CObserver m_selfObserver;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CRpcReleaseCourier, MT::CMutex > CRpcReleaseCourierPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCRELEASECOURIER_H ? */
