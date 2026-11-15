/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTMANUALCLOCKS_H
#define GUCEF_CORE_TESTAPP_TESTMANUALCLOCKS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CIUTCCLOCK_H
#include "gucefCORE_CIUtcClock.h"
#define GUCEF_CORE_CIUTCCLOCK_H
#endif /* GUCEF_CORE_CIUTCCLOCK_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace CORETEST {

/** A wall clock in UTC that only moves when the test moves it; read from any thread */
class CManualUtcClock : public GUCEF::CORE::CIUtcClock
{
    public:

    explicit CManualUtcClock( GUCEF::Int64 utcMs = 0 ) : GUCEF::CORE::CIUtcClock(), m_utcMs( utcMs ) {}
    virtual ~CManualUtcClock() GUCEF_VIRTUAL_OVERRIDE {}

    virtual GUCEF::Int64 GetUtcTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_utcMs.Load(); }

    void Set( GUCEF::Int64 utcMs ) { m_utcMs.Store( utcMs ); }
    void Advance( GUCEF::Int64 ms ) { m_utcMs.FetchAdd( ms ); }

    private:

    GUCEF::MT::CTAtomic< GUCEF::Int64 > m_utcMs;
};

/*-------------------------------------------------------------------------*/

/** A monotonic clock that only moves (forward) when the test moves it; read from any thread */
class CManualMonotonicClock : public GUCEF::CORE::CIMonotonicClock
{
    public:

    explicit CManualMonotonicClock( GUCEF::UInt64 ms = 1000 ) : GUCEF::CORE::CIMonotonicClock(), m_ms( ms ) {}
    virtual ~CManualMonotonicClock() GUCEF_VIRTUAL_OVERRIDE {}

    virtual GUCEF::UInt64 GetMonotonicTimeInMs( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_ms.Load(); }

    void Advance( GUCEF::UInt64 ms ) { m_ms.FetchAdd( ms ); }

    private:

    GUCEF::MT::CTAtomic< GUCEF::UInt64 > m_ms;
};

/*-------------------------------------------------------------------------*/

/** Both clocks moved together, as real time moves them; the wall clock alone can be set to jump */
class CManualClocks
{
    public:

    CManualClocks( GUCEF::Int64 utcMs ) : utc( utcMs ), monotonic( 1000 ) {}

    void Advance( GUCEF::Int64 ms ) { utc.Advance( ms ); monotonic.Advance( (GUCEF::UInt64) ms ); }

    CManualUtcClock utc;
    CManualMonotonicClock monotonic;
};

}; /* namespace CORETEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTMANUALCLOCKS_H ? */
