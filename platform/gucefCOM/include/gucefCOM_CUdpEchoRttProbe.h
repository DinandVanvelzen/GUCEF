/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CUDPECHORTTPROBE_H
#define GUCEF_COM_CUDPECHORTTPROBE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CIRTTPROBE_H
#include "gucefCOMCORE_CIRttProbe.h"
#define GUCEF_COMCORE_CIRTTPROBE_H
#endif /* GUCEF_COMCORE_CIRTTPROBE_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

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

/**
 *  Round trip times with UDP echoes: a 16 byte ['G','D','E','C'][u32 seq][u64 send time] datagram goes to the
 *  target, where a CUdpEchoResponder sends it back. The send time is in microseconds of the clock of the prober,
 *  the answer carries it back, so probes can overlap and late answers still measure right.
 *  The target needs a port: that of the responder.
 */
class GUCEF_COM_PUBLIC_CPP CUdpEchoRttProbe : public COMCORE::CIRttProbe
{
    public:

    static const CORE::CString MethodName;     /**< "udp-echo" */

    CUdpEchoRttProbe( void );

    explicit CUdpEchoRttProbe( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CUdpEchoRttProbe();

    virtual bool Start( const COMCORE::CHostAddress& target ,
                        UInt32 count                        ,
                        UInt32 intervalMs                   ,
                        UInt32 timeoutMs                    ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetMethodName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Writes an echo probe datagram into the 16 byte buffer */
    static void WriteEchoPacket( UInt8* packet, UInt32 sequence, UInt64 sendTimeInMicroSecs );

    /** Reads an echo probe datagram, false if it is not one */
    static bool ReadEchoPacket( const void* data, UInt32 dataSize, UInt32& sequence, UInt64& sendTimeInMicroSecs );

    private:

    typedef GUCEF::map< UInt32, UInt64 > TOutstandingProbeMap;
    typedef CORE::CTEventHandlerFunctor< CUdpEchoRttProbe > TEventCallback;

    CUdpEchoRttProbe( const CUdpEchoRttProbe& src );
    CUdpEchoRttProbe& operator=( const CUdpEchoRttProbe& src );

    bool SendProbe( UInt64 nowInMicroSecs );
    bool CheckFinished( void );

    void RegisterEventHandlers( void );

    void OnSocketPackets( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );

    void OnProbeTimer( CORE::CNotifier* notifier    ,
                       const CORE::CEvent& eventId  ,
                       CORE::CICloneable* eventData );

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    COMCORE::CUDPSocket m_socket;
    CORE::CTimer m_timer;
    COMCORE::CHostAddress m_target;
    COMCORE::CIPv4Address m_targetAddress;
    UInt32 m_count;
    UInt64 m_intervalInMicroSecs;
    UInt64 m_timeoutInMicroSecs;
    UInt32 m_probesSent;
    UInt64 m_nextProbeInMicroSecs;
    bool m_isActive;
    TOutstandingProbeMap m_outstanding;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIRttProbe, CUdpEchoRttProbe, MT::CMutex > TUdpEchoRttProbeFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CUDPECHORTTPROBE_H ? */
