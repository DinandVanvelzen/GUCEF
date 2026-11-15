/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CSERVICESTOPSIGNALS_H
#define GUCEF_CORE_CSERVICESTOPSIGNALS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

/**
 *  The stop signal of a service process (SIGTERM of a container runtime or of systemd, SIGINT of a terminal; on
 *  Windows the console control events), handled so that the service can drain instead of stopping at once.
 *  - InstallHandlers() installs handlers that only note the signal number (one store of a sig_atomic_t, which is all a
 *    signal handler may do safely). Called after GUCEF_OSSERVICEMAIN_BEGIN installed its own handlers, these replace
 *    them, so CGUCEFApplication does not stop at its next pulse: the service decides when to stop.
 *  - An instance watches a pulse generator and raises StopRequestedEvent once, on that pulse generator's thread, at the
 *    first pulse after a signal was noted (also for a signal noted before the instance existed).
 *  The static members may be used without an instance (a service with its own loop polls GetPendingSignal()).
 */
class GUCEF_CORE_PUBLIC_CPP CServiceStopSignals : public CTSGNotifier
{
    public:

    static const CEvent StopRequestedEvent;     /**< TCloneableInt32 (cloneables.h): the signal number */

    static void RegisterEvents( void );

    /** Installs the noting handlers for SIGTERM and SIGINT (Windows: the console control handler); false on failure */
    static bool InstallHandlers( void );

    /** Puts back the handlers that were in place before InstallHandlers() (tests); false when nothing was installed */
    static bool RestoreHandlers( void );

    /** Async-signal-safe: one store of the number (0 is taken as SIGTERM) */
    static void NoteSignal( int signalNumber );

    /** The signal noted last, 0 when none came */
    static int GetPendingSignal( void );

    /** Forgets the signal; for tests and for a service that starts again in the same process */
    static void Reset( void );

    /** Watches the pulse generator (GUCEF_NULL: the global one) */
    explicit CServiceStopSignals( PulseGeneratorPtr pulseGenerator = PulseGeneratorPtr() );
    virtual ~CServiceStopSignals() GUCEF_VIRTUAL_OVERRIDE;

    /** Whether StopRequestedEvent was raised by this instance */
    bool IsStopRequested( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPumpedNotify( CNotifier* notifier, const CEvent& eventId, CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CServiceStopSignals( const CServiceStopSignals& src );              /**< not implemented */
    CServiceStopSignals& operator=( const CServiceStopSignals& src );   /**< not implemented */

    bool m_raised;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSERVICESTOPSIGNALS_H ? */
