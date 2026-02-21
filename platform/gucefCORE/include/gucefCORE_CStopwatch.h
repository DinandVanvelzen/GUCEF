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

#ifndef GUCEF_CORE_CSTOPWATCH_H
#define GUCEF_CORE_CSTOPWATCH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMESTAMP_H
#include "gucefCORE_CTimestamp.h"
#define GUCEF_CORE_CTIMESTAMP_H
#endif /* GUCEF_CORE_CTIMESTAMP_H ? */

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
 *  High-resolution stopwatch for measuring execution time.
 *  
 *  Combines two CTimestamp objects to provide easy time delta determination.
 *  Useful for performance measurement and metrics collection.
 *  
 *  Basic usage:
 *  @code
 *  CStopwatch sw;
 *  sw.Start();
 *  // ... code to measure ...
 *  sw.Stop();
 *  UInt64 elapsedMs = sw.GetElapsedMilliseconds();
 *  @endcode
 *  
 *  The stopwatch uses the same platform-native resolution as CTimestamp:
 *  - Windows: 100 nanoseconds
 *  - Linux/Android/macOS: 1 nanosecond
 *  - Other platforms: 1 microsecond
 */
class GUCEF_CORE_PUBLIC_CPP CStopwatch
{
    public:

    CStopwatch( void );

    CStopwatch( const CStopwatch& src );

    /**
     *  Constructs and immediately starts the stopwatch
     */
    explicit CStopwatch( bool startImmediately );

    ~CStopwatch();

    CStopwatch& operator=( const CStopwatch& src );

    /**
     *  Starts the stopwatch by capturing the current timestamp.
     *  If already running, this resets the start time.
     */
    void Start( void );

    /**
     *  Stops the stopwatch by capturing the current timestamp.
     *  Has no effect if the stopwatch was never started.
     */
    void Stop( void );

    /**
     *  Resets the stopwatch, clearing both start and stop timestamps.
     */
    void Reset( void );

    /**
     *  Resets and immediately starts the stopwatch.
     *  Equivalent to calling Reset() followed by Start().
     */
    void Restart( void );

    /**
     *  Returns whether the stopwatch is currently running
     *  (started but not yet stopped).
     */
    bool IsRunning( void ) const;

    /**
     *  Returns whether the stopwatch has been started at least once.
     */
    bool HasStarted( void ) const;

    /**
     *  Returns whether the stopwatch has been stopped.
     */
    bool HasStopped( void ) const;

    /**
     *  Returns the elapsed time in milliseconds.
     *  If still running, returns elapsed time since start.
     *  If stopped, returns time between start and stop.
     *  Returns 0 if never started.
     */
    UInt64 GetElapsedMilliseconds( void ) const;

    /**
     *  Returns the elapsed time in microseconds.
     *  If still running, returns elapsed time since start.
     *  If stopped, returns time between start and stop.
     *  Returns 0 if never started.
     */
    UInt64 GetElapsedMicroseconds( void ) const;

    /**
     *  Returns the elapsed time in nanoseconds.
     *  If still running, returns elapsed time since start.
     *  If stopped, returns time between start and stop.
     *  Returns 0 if never started.
     */
    UInt64 GetElapsedNanoseconds( void ) const;

    /**
     *  Returns the elapsed time in seconds as a floating-point value.
     *  Provides sub-second precision.
     */
    Float64 GetElapsedSeconds( void ) const;

    /**
     *  Returns the elapsed time in platform-native ticks.
     *  See CTimestamp for tick resolution on each platform.
     */
    Int64 GetElapsedTicks( void ) const;

    /**
     *  Returns the start timestamp.
     *  Returns CTimestamp::Empty if not started.
     */
    const CTimestamp& GetStartTimestamp( void ) const;

    /**
     *  Returns the stop timestamp.
     *  Returns CTimestamp::Empty if not stopped.
     */
    const CTimestamp& GetStopTimestamp( void ) const;

    /**
     *  Returns the effective end timestamp for elapsed time calculation.
     *  If stopped, returns the stop timestamp.
     *  If running, returns the current time.
     *  If not started, returns CTimestamp::Empty.
     */
    CTimestamp GetEffectiveEndTimestamp( void ) const;

    /**
     *  Static convenience method: creates a started stopwatch.
     */
    static CStopwatch StartNew( void );

    private:

    CTimestamp m_startTimestamp;
    CTimestamp m_stopTimestamp;
    bool m_isRunning;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSTOPWATCH_H ? */
