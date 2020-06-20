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

class GUCEF_CORE_PUBLIC_CPP CPulseGenerator : public CNotifier
{
    public:

    static const CEvent PulseEvent;

    static void RegisterEvents( void );

    typedef CPulseData TPulseEventData;

    public:

    CPulseGenerator( void );

    virtual ~CPulseGenerator();

    void RequestImmediatePulse( void );

    void RequestPulsesPerImmediatePulseRequest( const Int32 requestedPulsesPerImmediatePulseRequest );

    void RequestPeriodicPulses( void* requestor );

    void RequestPeriodicPulses( void* requestor                           ,
                                const UInt32 minimalPulseDeltaInMilliSecs );

    void RequestPeriodicPulses( void );

    void RequestStopOfPeriodicUpdates( void* requestor );

    void RequestPulseInterval( const UInt32 minimalPulseDeltaInMilliSecs );

    bool IsPulsingPeriodicly( void ) const;

    UInt32 GetRequiredPulseDeltaInMilliSecs( void ) const;

    Float64 GetActualPulseDeltaInMilliSecs( void ) const;

    UInt64 GetTickCountAtLastPulse( void ) const;

    void SetPulseGeneratorDriver( CIPulseGeneratorDriver* driver );

    CIPulseGeneratorDriver* GetPulseGeneratorDriver( void ) const;

    bool IsForcedStopOfPeriodicPulsesRequested( void ) const;

    void ForceStopOfPeriodicPulses( void );

    void AllowPeriodicPulses( void );

    /**
     *  Blocks the calling thread until at least the next pulse time window
     *  If there is a minimum time you at minimum want to wait regardless of the minimum need of pulse clients
     *  you can specify non-zero for "forcedMinimalDeltaInMilliSecs" 
     */
    void WaitTillNextPulseWindow( UInt32 forcedMinimalDeltaInMilliSecs = 25    ,
                                  UInt32 desiredMaximumDeltaInMilliSecs = 1000 ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool Lock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

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

    typedef std::map< void*, UInt32 > TPeriodicPulseRequestorMap;

    UInt64 m_lastCycleTickCount;
    UInt32 m_updateDeltaInMilliSecs;
    Float64 m_ticksPerMs;
    bool m_forcedStopOfPeriodPulses;
    TPeriodicPulseRequestorMap m_periodicUpdateRequestors;
    CIPulseGeneratorDriver* m_driver;
    MT::CMutex m_mutex;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-11-2004 :
        - Designed and implemented this class.

-----------------------------------------------------------------------------*/
