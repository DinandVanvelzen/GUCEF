/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#define GUCEF_CORE_CPULSEGENERATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_CORE_CPULSEDATA_H
#include "gucefCORE_CPulseData.h"
#define GUCEF_CORE_CPULSEDATA_H
#endif /* GUCEF_CORE_CPULSEDATA_H ? */

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

class CIPulseGeneratorDriver;

/*--------------------------------------------------------------------------*/

class GUCEF_CORE_PUBLIC_CPP CPulseGenerator : public CNotifier ,
                                              public CTSharedObjCreator< CPulseGenerator, MT::CMutex >
{
    public:

    static const CEvent PulseEvent;

    static void RegisterEvents( void );

    typedef CPulseData TPulseEventData;

    public:

    CPulseGenerator( void );

    virtual ~CPulseGenerator();

    void RequestImmediatePulse( void );

    /**
     *  Passes an immediate pulse request on to the driver (CIPulseGeneratorDriver::QueueImmediatePulse()), for a
     *  request made on any thread, for example one that waits for socket readiness on behalf of the pulse generator's
     *  users. Unlike RequestImmediatePulse() it never pulses on the calling thread: without a driver it does nothing,
     *  and a driver keeps the request for its own loop. The driver is called without any lock of the pulse generator
     *  held; meanwhile SetPulseGeneratorDriver() waits before it lets the driver go.
     *  A task delegator (and the gucefDRN node task) ends its wait between cycles on such a request; the busy wait and
     *  Emscripten drivers take it at the next pulse of their loops.
     *
     *  @return whether a driver took the request
     */
    bool RequestImmediatePulseThroughDriver( void );

    void RequestPulsesPerImmediatePulseRequest( const Int32 requestedPulsesPerImmediatePulseRequest );

    void RequestPeriodicPulses( void* requestor );

    void RequestPeriodicPulses( void* requestor                           ,
                                const UInt32 minimalPulseDeltaInMilliSecs );

    void RequestPeriodicPulses( void );

    void RequestStopOfPeriodicUpdates( void* requestor );

    void RequestPulseInterval( const UInt32 minimalPulseDeltaInMilliSecs );

    bool IsPulsingPeriodicly( void ) const;

    /** The number of requestors (timers, sockets) that have periodic pulses requested: what is still asking for pulses */
    UInt32 GetNrOfPeriodicPulseRequestors( void ) const;

    UInt32 GetRequiredPulseDeltaInMilliSecs( void ) const;

    Float64 GetActualPulseDeltaInMilliSecs( void ) const;

    UInt64 GetTickCountAtLastPulse( void ) const;

    static UInt64 GetTickCount( void );

    Float64 GetTimeSinceTickCountInMilliSecs( UInt64 sinceTickCount ) const;

    /**
     *  Sets the driver, GUCEF_NULL for none. When it replaces a driver it returns only once no call that is made without
     *  the data lock (RequestImmediatePulseThroughDriver(), GetPulseDriverThreadId(), RequestPeriodicPulses()) is inside
     *  the previous one, which may be destroyed then. Not called from within a call of the driver
     */
    void SetPulseGeneratorDriver( CIPulseGeneratorDriver* driver );

    CIPulseGeneratorDriver* GetPulseGeneratorDriver( void ) const;

    /**
     *  Records the thread that drives the pulses when no driver object is set. When the calling thread records itself
     *  its operating system id is recorded as well (see GetPulseDriverSystemThreadId()); for another thread it is not
     *  known here and reads 0.
     */
    void SetPulseDriverThreadId( TThreadId driverThreadId );
    
    /** MT::GetCurrentThreadID() of the thread that drives the pulses, 0 when no thread does */
    TThreadId GetPulseDriverThreadId( void ) const;

    /**
     *  The operating system's id (MT::GetCurrentThreadSystemID()) of the thread GetPulseDriverThreadId() reports, for
     *  display; 0 when no thread drives the pulses or the id is not known. Where GUCEF_SYSTEM_THREAD_ID_IS_THREAD_ID is
     *  1 it is GetPulseDriverThreadId(). Never compare it to decide which thread runs.
     */
    TSystemThreadId GetPulseDriverSystemThreadId( void ) const;

    bool IsForcedStopOfPeriodicPulsesRequested( void ) const;

    void ForceStopOfPeriodicPulses( void );

    void ForceDirectPulse( void );

    void AllowPeriodicPulses( void );

    /**
     *  Blocks the calling thread until at least the next pulse time window
     *  If there is a minimum time you at minimum want to wait regardless of the minimum need of pulse clients
     *  you can specify non-zero for "forcedMinimalDeltaInMilliSecs" 
     * 
     *  @param elapsedTimeInMilliSecs optional output param of actual ellapsed time post-wait
     */
    void WaitTillNextPulseWindow( UInt32 forcedMinimalDeltaInMilliSecs = 25    ,
                                  UInt32 desiredMaximumDeltaInMilliSecs = 1000 ,
                                  UInt32* elapsedTimeInMilliSecs = GUCEF_NULL  ) const;

    /**
     *  The time WaitTillNextPulseWindow() would block the calling thread with the same parameters, without blocking.
     *  For a driver that waits in a way of its own, for example on a signal that ends the wait early.
     *
     *  @return the time to wait in milliseconds, 0 if the next pulse time window has come already
     */
    UInt32 GetTimeTillNextPulseWindowInMilliSecs( UInt32 forcedMinimalDeltaInMilliSecs = 25    ,
                                                  UInt32 desiredMaximumDeltaInMilliSecs = 1000 ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus NotificationUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CIPulseGeneratorDriver;

    void OnDriverPulse( void );

    private:

    /**
     *  returns the minimal required resolution for the application
     *  update interval in milliseconds as requested by the application
     *  sub systems.
     *
     *  @return the minimal required update frequency in milliseconds
     */
    UInt32 DetermineRequiredPulseInterval( void ) const;

    private:

    typedef GUCEF::map< void*, UInt32 > TPeriodicPulseRequestorMap;

    UInt64 m_lastCycleTickCount;
    UInt32 m_updateDeltaInMilliSecs;
    Float64 m_ticksPerMs;
    bool m_forcedStopOfPeriodPulses;
    TPeriodicPulseRequestorMap m_periodicUpdateRequestors;
    CIPulseGeneratorDriver* m_driver;
    MT::CTAtomic< TThreadId > m_driverThreadId;            /**< written under m_dataLock, read without it (GetPulseDriverThreadId()) */
    GUCEF_SYSTEM_THREAD_ID_CODE( MT::CTAtomic< TSystemThreadId > m_driverSystemThreadId; )   /**< display only, written with m_driverThreadId */
    MT::CMutex m_notificationLock;
    MT::CMutex m_dataLock;
    mutable MT::CTAtomic< UInt32 > m_driverRequestsInFlight;    /**< calls made without m_dataLock inside the driver: RequestImmediatePulseThroughDriver(), GetPulseDriverThreadId(), RequestPeriodicPulses() */
};

/*-------------------------------------------------------------------------*/

typedef typename CTSharedPtrCreator< CPulseGenerator, MT::CMutex >::TSharedPtrType  PulseGeneratorPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */
