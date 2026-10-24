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

#ifndef GUCEF_COM_CICMPRTTPROBE_H
#define GUCEF_COM_CICMPRTTPROBE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CPING_H
#include "CPing.h"
#define GUCEF_COMCORE_CPING_H
#endif /* GUCEF_COMCORE_CPING_H ? */

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
 *  Round trip times with ICMP echo, through COMCORE::CPing. Not available where CPing cannot work: on Linux
 *  without CAP_NET_RAW or a ping_group_range that covers the process, on the web and on macOS.
 *  The port of the target is not used.
 */
class GUCEF_COM_PUBLIC_CPP CIcmpRttProbe : public COMCORE::CIRttProbe
{
    public:

    static const CORE::CString MethodName;     /**< "icmp" */

    CIcmpRttProbe( void );

    explicit CIcmpRttProbe( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CIcmpRttProbe();

    virtual bool Start( const COMCORE::CHostAddress& target ,
                        UInt32 count                        ,
                        UInt32 intervalMs                   ,
                        UInt32 timeoutMs                    ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetMethodName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Whether ICMP echo can be used by this process on this platform */
    static bool IsIcmpAvailable( void );

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CIcmpRttProbe( const CIcmpRttProbe& src );
    CIcmpRttProbe& operator=( const CIcmpRttProbe& src );

    private:

    COMCORE::CPing m_ping;
    COMCORE::CHostAddress m_target;
    UInt32 m_sequence;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIRttProbe, CIcmpRttProbe, MT::CMutex > TIcmpRttProbeFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CICMPRTTPROBE_H ? */
