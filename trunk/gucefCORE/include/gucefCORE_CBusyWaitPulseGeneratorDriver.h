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

#ifndef GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
#define GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H
 
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#include "gucefCORE_CIPulseGeneratorDriver.h"
#define GUCEF_CORE_CIPULSEGENERATORDRIVER_H
#endif /* GUCEF_CORE_CIPULSEGENERATORDRIVER_H ? */

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

class GUCEF_CORE_EXPORT_CPP CBusyWaitPulseGeneratorDriver : public CIPulseGeneratorDriver
{
    public:
    
    CBusyWaitPulseGeneratorDriver( void );
    
    CBusyWaitPulseGeneratorDriver( const CBusyWaitPulseGeneratorDriver& src );
    
    virtual ~CBusyWaitPulseGeneratorDriver();
    
    CBusyWaitPulseGeneratorDriver& operator=( const CIPulseGeneratorDriver& src );
    
    virtual void RequestPulse( CPulseGenerator& pulseGenerator );
    
    virtual void RequestPeriodicPulses( CPulseGenerator& pulseGenerator    ,
                                        const UInt32 pulseDeltaInMilliSecs );

    virtual void RequestPulseInterval( CPulseGenerator& pulseGenerator    ,
                                       const UInt32 pulseDeltaInMilliSecs );
    
    virtual void RequestStopOfPeriodicUpdates( CPulseGenerator& pulseGenerator );
    
    void Run( CPulseGenerator& pulseGenerator );

    private:
    
    bool m_loop;
    Float64 m_desiredPulseDelta;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CBUSYWAITPULSEGENERATORDRIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 12-11-2004 :
        - Designed and implemented this class.

-----------------------------------------------------------------------------*/
