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

#ifndef GUCEF_COM_CTCPCONNECTRTTPROBE_H
#define GUCEF_COM_CTCPCONNECTRTTPROBE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

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
 *  Round trip times from TCP connects: the time from starting a non-blocking connect to the target port until
 *  the connection is made, after which it is closed again. A refused connection also proves the host answered,
 *  it counts as a result too. Works without privileges anywhere TCP works, the target needs a port.
 *
 *  The measurement is as fine as the pulses of the pulse generator the socket runs on.
 */
class GUCEF_COM_PUBLIC_CPP CTcpConnectRttProbe : public COMCORE::CIRttProbe
{
    public:

    static const CORE::CString MethodName;     /**< "tcp-connect" */

    CTcpConnectRttProbe( void );

    explicit CTcpConnectRttProbe( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CTcpConnectRttProbe();

    virtual bool Start( const COMCORE::CHostAddress& target ,
                        UInt32 count                        ,
                        UInt32 intervalMs                   ,
                        UInt32 timeoutMs                    ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetMethodName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTcpConnectRttProbe( const CTcpConnectRttProbe& src );
    CTcpConnectRttProbe& operator=( const CTcpConnectRttProbe& src );

    bool StartProbe( UInt64 nowInMicroSecs );
    bool EndProbe( const CORE::CEvent& eventId, UInt64 rttInMicroSecs );
    bool Finish( void );

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    COMCORE::CTCPClientSocket m_socket;
    CORE::CTimer m_timer;
    COMCORE::CHostAddress m_target;
    UInt32 m_count;
    UInt32 m_intervalInMicroSecs;
    UInt32 m_timeoutInMicroSecs;
    UInt32 m_probesStarted;
    bool m_isActive;
    bool m_probeInProgress;
    UInt64 m_probeStartInMicroSecs;
    UInt64 m_nextProbeInMicroSecs;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIRttProbe, CTcpConnectRttProbe, MT::CMutex > TTcpConnectRttProbeFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CTCPCONNECTRTTPROBE_H ? */
