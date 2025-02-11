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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefCORE_CIPulseGeneratorDriver.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CIPulseGeneratorDriver::CIPulseGeneratorDriver( void )
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

CIPulseGeneratorDriver::CIPulseGeneratorDriver( const CIPulseGeneratorDriver& src )
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

CIPulseGeneratorDriver::~CIPulseGeneratorDriver()
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

CIPulseGeneratorDriver&
CIPulseGeneratorDriver::operator=( const CIPulseGeneratorDriver& src )
{GUCEF_TRACE;

    return *this;
}

/*--------------------------------------------------------------------------*/

void
CIPulseGeneratorDriver::SendDriverPulse( CPulseGenerator& pulseGenerator )
{GUCEF_TRACE;

    // disable Clang warining: reference cannot be bound to dereferenced null pointer in well-defined C++ code; comparison may be assumed to always evaluate to true [-Wtautological-undefined-compare]
    // while technically the null reference should never occur this is intended to catch bad memory access a bit earlier in the call stack
    #ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wtautological-undefined-compare"
    #endif
    if ( GUCEF_NULL != &pulseGenerator )
        pulseGenerator.OnDriverPulse();
    else
    {
        GUCEF_ERROR_LOG( LOGLEVEL_IMPORTANT, "CIPulseGeneratorDriver:SendDriverPulse: Attempt to access NULL pulseGenerator" );
    }
    #ifdef __clang__
    #pragma clang diagnostic pop
    #endif
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
