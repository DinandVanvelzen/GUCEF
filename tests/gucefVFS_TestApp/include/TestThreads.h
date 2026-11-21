/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_VFS_TESTAPP_TESTTHREADS_H
#define GUCEF_VFS_TESTAPP_TESTTHREADS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a test thread does; 'index' tells the threads of a group apart */
class CTestThreadBody
{
    public:

    virtual ~CTestThreadBody() {}

    virtual void Run( GUCEF::UInt32 index ) = 0;
};

/**
 *  A group of threads that each run a body once, for the cases that call from several threads. The destructor joins what was
 *  started.
 */
class CTestThreads
{
    public:

    CTestThreads( CTestThreadBody& body, GUCEF::UInt32 count );

    ~CTestThreads();

    /** Starts the threads; false when one could not be started */
    bool Start( void );

    /** Waits for all of them; false when they did not all end in time (a deadlock) */
    bool Join( GUCEF::UInt32 timeoutMs );

    /** The thread id (GUCEF::MT::GetCurrentThreadID() on that thread) of the thread; 0 when it was not started */
    GUCEF::TThreadId GetThreadId( GUCEF::UInt32 index ) const;

    private:

    CTestThreads( const CTestThreads& src );
    CTestThreads& operator=( const CTestThreads& src );

    struct SSlot;
    static GUCEF::UInt32 GUCEF_CALLSPEC_PREFIX Main( void* data ) GUCEF_CALLSPEC_SUFFIX;

    CTestThreadBody& m_body;
    GUCEF::UInt32 m_count;
    SSlot* m_slots;
    bool m_started;
    bool m_joined;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTTHREADS_H ? */
