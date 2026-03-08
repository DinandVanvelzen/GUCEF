/*
 *  MemoryLeakFinder: GUCEF module providing memory diagnostics
 *  Copyright (C) 2002 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_MLF_CCALLSTACKTRACKER_H
#define GUCEF_MLF_CCALLSTACKTRACKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <stdio.h>

#ifndef GUCEF_MLF_ETYPES_H
#include "gucefMLF_ETypes.h"
#define GUCEF_MLF_ETYPES_H
#endif /* GUCEF_MLF_ETYPES_H ? */

#ifndef GUCEF_CALLSTACK_H
#include "gucefMLF_callstack.h"
#define GUCEF_CALLSTACK_H
#endif /* GUCEF_CALLSTACK_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MLF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * Encapsulates the call stack tracking subsystem.
 * Replaces the anonymous StackInventory class that lived inside callstack.cpp.
 *
 * Thread-safe: one TCallStack per thread, keyed by thread ID.
 * The stack is stored inside a per-thread StackTraceInfo object; the master
 * map is protected by m_datalock (read lock for access, upgraded to write lock
 * on first encounter of a new thread ID).
 */
class GUCEF_HIDDEN CCallStackTracker
{
    public:

    static CCallStackTracker* Instance( void );
    static void               Deinstance( void );
    static bool               IsInitialized( void );

    void SetStackPushCallback( TStackPushCallback cBack );
    void SetStackPopCallback( TStackPopCallback cBack );

    void CallstackScopeBegin( const char* file, Int32 line );
    void CallstackScopeEnd( void );

    Int32 GetCallstackForCurrentThread( TCallStack** outStack );
    Int32 GetCallstackCopyForCurrentThread( TCallStack** outStack, bool alsoCopyStatics );
    void  FreeCallstackCopy( TCallStack* stackCopy );

    void PrintCallstack( FILE* dest );
    void PrintCallstackCopy( TCallStack& stackToPrint, FILE* dest );
    void SetIsStackLoggingEnabled( bool isEnabled );
    void SetIsStackLoggingInCsvFormat( bool isInCsv );
    void SetLogFilename( const char* filename );
    void SetStackLogOutputToStdOut( void );
    void CloseLogFile( void );
    void Log( const char* logtype, UInt32 threadID, Int32 stackheight,
              const char* file,    Int32 line,       UInt32 ticksSpent );

    /**
     * RAII scope guard — pushes on construction, pops on destruction.
     * Use instead of raw MEMMAN_CallstackScopeBegin/End pairs.
     */
    class CScopeGuard
    {
        public:
        CScopeGuard( const char* file, Int32 line )
        {
            CCallStackTracker* t = CCallStackTracker::Instance();
            if ( GUCEF_NULL != t )
                t->CallstackScopeBegin( file, line );
        }
        ~CScopeGuard( void )
        {
            CCallStackTracker* t = CCallStackTracker::Instance();
            if ( GUCEF_NULL != t )
                t->CallstackScopeEnd();
        }
        private:
        CScopeGuard( const CScopeGuard& );
        CScopeGuard& operator=( const CScopeGuard& );
    };

    private:

    CCallStackTracker( void );
    ~CCallStackTracker( void );
    CCallStackTracker( const CCallStackTracker& );
    CCallStackTracker& operator=( const CCallStackTracker& );

    /* ------------------------------------------------------------------ */
    /* Per-thread callstack                                                 */
    /* ------------------------------------------------------------------ */
    class GUCEF_HIDDEN StackTraceInfo
    {
        public:

        TCallStack m_callstack;

        static void Push( TCallStack* stack, const char* file, Int32 line );
        static void Pop( TCallStack* stack );

        void PrintCallstack( FILE* dest );
        static void PrintCallstack( TCallStack& stackToPrint, FILE* dest );

        void CallstackScopeBegin( const char* file, Int32 line );
        void CallstackScopeEnd( void );

        bool  GetCallstackForCurrentThread( TCallStack** outStack );
        Int32 GetCallstackCopyForCurrentThread( TCallStack** outStack, bool alsoCopyStatics );

        StackTraceInfo( void );
        ~StackTraceInfo( void );
    };

    typedef std::map< UInt32, StackTraceInfo > TThreadIdToCallStackMap;

    StackTraceInfo* GetStackTraceInfoForCallingThread( MT::CScopeReaderLock& readerLock );

    /* ------------------------------------------------------------------ */
    /* Members                                                              */
    /* ------------------------------------------------------------------ */
    TThreadIdToCallStackMap m_inventory;
    TStackPushCallback      m_pushCallback;
    TStackPopCallback       m_popCallback;
    char*                   m_logFilename;
    FILE*                   m_logFile;
    bool                    m_logInCsvFormat;
    bool                    m_logStack;

    MT::CReadWriteLock      m_datalock;

    static MT::CMutex        g_instanceLock;
    static CCallStackTracker* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MLF */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MLF_CCALLSTACKTRACKER_H ? */
