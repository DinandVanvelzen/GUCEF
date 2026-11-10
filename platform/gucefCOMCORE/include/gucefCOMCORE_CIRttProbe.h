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

#ifndef GUCEF_COMCORE_CIRTTPROBE_H
#define GUCEF_COMCORE_CIRTTPROBE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"      /* often used gucefCOMCORE macros */
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

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
 *  The outcome of one probe
 */
class GUCEF_COMCORE_EXPORT_CPP CRttProbeEventData : public CORE::CICloneable
{
    public:

    CRttProbeEventData( void );

    CRttProbeEventData( const CHostAddress& target   ,
                        UInt64 rttInMicroSecs        ,
                        UInt32 sequence              );

    CRttProbeEventData( const CRttProbeEventData& src );

    virtual ~CRttProbeEventData();

    CRttProbeEventData& operator=( const CRttProbeEventData& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    const CHostAddress& GetTarget( void ) const;

    /** The round trip time, 0 for a timeout or failure */
    UInt64 GetRttInMicroSecs( void ) const;

    /** The number of the probe, counting from 0 per Start() */
    UInt32 GetSequence( void ) const;

    private:

    CHostAddress m_target;
    UInt64 m_rttInMicroSecs;
    UInt32 m_sequence;
};

/*-------------------------------------------------------------------------*/

/**
 *  Measures round trip times to a host with one particular method
 *
 *  The probes run on a pulse generator, the results come as events: one ProbeResultEvent, ProbeTimeoutEvent or
 *  ProbeFailedEvent per probe and a ProbeFinishedEvent after the last one or after Stop().
 */
class GUCEF_COMCORE_EXPORT_CPP CIRttProbe : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ProbeResultEvent;      /**< CRttProbeEventData: an answer came back */
    static const CORE::CEvent ProbeTimeoutEvent;     /**< CRttProbeEventData: no answer within the timeout */
    static const CORE::CEvent ProbeFailedEvent;      /**< CRttProbeEventData: the probe could not be done */
    static const CORE::CEvent ProbeFinishedEvent;    /**< CRttProbeEventData of the last probe: all probes are done */

    static void RegisterEvents( void );

    CIRttProbe( void );

    virtual ~CIRttProbe();

    /**
     *  Starts probing the target
     *
     *  @param target the host to probe, with the port where the method needs one
     *  @param count the number of probes, 0 to keep probing until Stop()
     *  @param intervalMs the time between the start of two probes
     *  @param timeoutMs how long to wait for an answer
     */
    virtual bool Start( const CHostAddress& target ,
                        UInt32 count               ,
                        UInt32 intervalMs          ,
                        UInt32 timeoutMs           ) = 0;

    virtual void Stop( void ) = 0;

    virtual bool IsActive( void ) const = 0;

    /** False when the method cannot run here, for example ICMP without the privileges for it */
    virtual bool IsAvailable( void ) const = 0;

    /** The name the method is registered with in the factory, for example "udp-echo" */
    virtual const CORE::CString& GetMethodName( void ) const = 0;

    /** A monotonic clock in microseconds */
    static UInt64 GetTimeInMicroSecs( void );

    private:

    CIRttProbe( const CIRttProbe& src );
    CIRttProbe& operator=( const CIRttProbe& src );
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTAbstractFactory< CORE::CString, CIRttProbe, MT::CMutex > CRttProbeFactory;
typedef CRttProbeFactory::TProductPtr                                     CIRttProbePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CIRTTPROBE_H ? */
