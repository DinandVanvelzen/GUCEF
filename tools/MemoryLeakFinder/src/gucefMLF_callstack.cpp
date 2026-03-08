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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <map>

#ifndef GUCEF_DYNNEWOFF_H
#include "gucef_dynnewoff.h"
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#undef GUCEF_USE_CALLSTACK_TRACING
#undef GUCEF_USE_CALLSTACK_PLATFORM_TRACING

#include "gucefMLF_callstack.h"
#include "gucefMLF_CCallStackTracker.h"

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

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
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define STACK_RESIZE_AMOUNT     256

#ifdef GUCEF_MSWIN_BUILD
  #define EOL "\r\n"
#else
  #define EOL "\n"
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MLF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex         CCallStackTracker::g_instanceLock;
CCallStackTracker* CCallStackTracker::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION: CCallStackTracker                                  //
//                                                                         //
//-------------------------------------------------------------------------*/

/* static */
CCallStackTracker*
CCallStackTracker::Instance( void )
{
    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_instanceLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = new CCallStackTracker();
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

/* static */
void
CCallStackTracker::Deinstance( void )
{
    MT::CScopeMutex lock( g_instanceLock );
    if ( GUCEF_NULL != g_instance )
    {
        delete g_instance;
        g_instance = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

/* static */
bool
CCallStackTracker::IsInitialized( void )
{
    return GUCEF_NULL != g_instance;
}

/*-------------------------------------------------------------------------*/

CCallStackTracker::CCallStackTracker( void )
    : m_inventory()
    , m_pushCallback( GUCEF_NULL )
    , m_popCallback( GUCEF_NULL )
    , m_logFilename( GUCEF_NULL )
    , m_logFile( GUCEF_NULL )
    , m_logInCsvFormat( false )
    , m_logStack( false )
    , m_datalock( true )
{
}

/*-------------------------------------------------------------------------*/

CCallStackTracker::~CCallStackTracker( void )
{
    MT::CScopeWriterLock writeLock( m_datalock );
    CloseLogFile();

    free( m_logFilename );
    m_logFilename = GUCEF_NULL;

    m_inventory.clear();
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::CloseLogFile( void )
{
    MT::CScopeWriterLock writeLock( m_datalock );
    if ( ( m_logFile != stdout ) && ( m_logFile != GUCEF_NULL ) )
    {
        fclose( m_logFile );
        m_logFile = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::FreeCallstackCopy( TCallStack* stackCopy )
{
    if ( GUCEF_NULL != stackCopy )
    {
        if ( 0 != stackCopy->staticsAreCopied )
        {
            for ( UInt32 s = 0; s < stackCopy->items; ++s )
            {
                free( (void*) stackCopy->file[ s ] );
            }
        }
        free( stackCopy->file );
        free( stackCopy->linenr );
        free( stackCopy->entryTickCount );
        free( stackCopy );
    }
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::PrintCallstackCopy( TCallStack& stackToPrint, FILE* dest )
{
    StackTraceInfo::PrintCallstack( stackToPrint, dest );
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::SetIsStackLoggingEnabled( bool isEnabled )
{
    MT::CScopeWriterLock writeLock( m_datalock );
    m_logStack = isEnabled;
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::SetIsStackLoggingInCsvFormat( bool isInCsv )
{
    MT::CScopeWriterLock writeLock( m_datalock );
    m_logInCsvFormat = isInCsv;
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::SetLogFilename( const char* filename )
{
    MT::CScopeWriterLock writeLock( m_datalock );

    bool wasOpen = GUCEF_NULL != m_logFile;
    CloseLogFile();

    free( m_logFilename );
    m_logFilename = GUCEF_NULL;

    if ( GUCEF_NULL != filename )
    {
        UInt32 strLen = (UInt32) strlen( filename ) + 1;
        m_logFilename = (char*) malloc( strLen );
        if ( GUCEF_NULL != m_logFilename )
            memcpy( m_logFilename, filename, strLen );
    }

    if ( wasOpen && GUCEF_NULL != m_logFilename )
    {
        m_logFile = fopen( m_logFilename, "ab" );
    }
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::SetStackLogOutputToStdOut( void )
{
    MT::CScopeWriterLock writeLock( m_datalock );
    CloseLogFile();
    free( m_logFilename );
    m_logFilename = GUCEF_NULL;
    m_logFile = stdout;
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::Log( const char* logtype ,
                        UInt32 threadID     ,
                        Int32 stackheight   ,
                        const char* file    ,
                        Int32 line          ,
                        UInt32 ticksSpent   )
{
    MT::CScopeReaderLock readLock( m_datalock );

    if ( m_logStack )
    {
        if ( GUCEF_NULL == m_logFile )
        {
            MT::CScopeWriterLock writeLock( readLock );

            if ( GUCEF_NULL == m_logFilename )
            {
                m_logFilename = (char*) malloc( 14 );
                memcpy( m_logFilename, "Callstack.txt", 14 );
                m_logFile = fopen( m_logFilename, "ab" );
            }
            else
            {
                m_logFile = fopen( m_logFilename, "ab" );
            }

            writeLock.TransitionToReader( readLock );
        }

        if ( GUCEF_NULL != m_logFile )
        {
            if ( !m_logInCsvFormat )
            {
                if ( ticksSpent > 0 )
                    fprintf( m_logFile, "Thread %d: %s: %d: %s(%d) (%d ms)%s", threadID, logtype, stackheight, file, line, ticksSpent, EOL );
                else
                    fprintf( m_logFile, "Thread %d: %s: %d: %s(%d)%s", threadID, logtype, stackheight, file, line, EOL );
            }
            else
            {
                if ( ticksSpent > 0 )
                    fprintf( m_logFile, "%d,%s,%d,%s,%d,%d%s", threadID, logtype, stackheight, file, line, ticksSpent, EOL );
                else
                    fprintf( m_logFile, "%d,%s,%d,%s,%d%s", threadID, logtype, stackheight, file, line, EOL );
            }
            fflush( m_logFile );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::SetStackPushCallback( TStackPushCallback cBack )
{
    MT::CScopeWriterLock writeLock( m_datalock );
    m_pushCallback = cBack;
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::SetStackPopCallback( TStackPopCallback cBack )
{
    MT::CScopeWriterLock writeLock( m_datalock );
    m_popCallback = cBack;
}

/*-------------------------------------------------------------------------*/

CCallStackTracker::StackTraceInfo*
CCallStackTracker::GetStackTraceInfoForCallingThread( MT::CScopeReaderLock& readLock )
{
    UInt32 threadid = MT::GetCurrentTaskID();

    TThreadIdToCallStackMap::iterator i = m_inventory.find( threadid );
    if ( i != m_inventory.end() )
        return &(*i).second;

    /* First time seeing this thread — escalate to write lock */
    MT::CScopeWriterLock writeLock( readLock );
    StackTraceInfo& newStack = m_inventory[ threadid ];
    newStack.m_callstack.threadid = threadid;

    assert( 0 == writeLock.GetWriterReentrancyDepth() );

    writeLock.TransitionToReader( readLock );
    return &newStack;
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::CallstackScopeBegin( const char* file, Int32 line )
{
    MT::CScopeReaderLock readLock( m_datalock );
    StackTraceInfo* info = GetStackTraceInfoForCallingThread( readLock );
    info->CallstackScopeBegin( file, line );
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::CallstackScopeEnd( void )
{
    MT::CScopeReaderLock readLock( m_datalock );
    StackTraceInfo* info = GetStackTraceInfoForCallingThread( readLock );
    info->CallstackScopeEnd();
}

/*-------------------------------------------------------------------------*/

Int32
CCallStackTracker::GetCallstackForCurrentThread( TCallStack** outStack )
{
    MT::CScopeReaderLock readLock( m_datalock );
    StackTraceInfo* info = GetStackTraceInfoForCallingThread( readLock );
    return info->GetCallstackForCurrentThread( outStack ) ? 0 : -1;
}

/*-------------------------------------------------------------------------*/

Int32
CCallStackTracker::GetCallstackCopyForCurrentThread( TCallStack** outStack, bool alsoCopyStatics )
{
    MT::CScopeReaderLock readLock( m_datalock );
    StackTraceInfo* info = GetStackTraceInfoForCallingThread( readLock );
    return info->GetCallstackCopyForCurrentThread( outStack, alsoCopyStatics );
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::PrintCallstack( FILE* dest )
{
    MT::CScopeWriterLock writeLock( m_datalock );
    TThreadIdToCallStackMap::iterator i = m_inventory.begin();
    while ( i != m_inventory.end() )
    {
        (*i).second.PrintCallstack( dest );
        ++i;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION: CCallStackTracker::StackTraceInfo                  //
//                                                                         //
//-------------------------------------------------------------------------*/

CCallStackTracker::StackTraceInfo::StackTraceInfo( void )
    : m_callstack()
{
    memset( &m_callstack, 0, sizeof( m_callstack ) );
}

/*-------------------------------------------------------------------------*/

CCallStackTracker::StackTraceInfo::~StackTraceInfo( void )
{
}

/*-------------------------------------------------------------------------*/

/* static */
void
CCallStackTracker::StackTraceInfo::PrintCallstack( TCallStack& stackToPrint, FILE* dest )
{
    fprintf( dest, "------------------------------%s", EOL );
    fprintf( dest, "Callstack for thread %d:%s%s", stackToPrint.threadid, EOL, EOL );
    fprintf( dest, "stack size = %d%s", stackToPrint.items, EOL );
    fprintf( dest, "------------------------------%s%s", EOL, EOL );
    if ( stackToPrint.items > 0 )
    {
        for ( UInt32 n = 0; n < stackToPrint.items; ++n )
        {
            fprintf( dest, "%d: %s(%d)%s", n + 1, stackToPrint.file[ n ], stackToPrint.linenr[ n ], EOL );
        }
        fprintf( dest, "%s%s------------------------------%s%s", EOL, EOL, EOL, EOL );
    }
    else
    {
        fprintf( dest, ">>> no items on stack <<<%s%s", EOL, EOL );
        fprintf( dest, "------------------------------%s%s", EOL, EOL );
    }
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::StackTraceInfo::PrintCallstack( FILE* dest )
{
    PrintCallstack( m_callstack, dest );
}

/*-------------------------------------------------------------------------*/

/* static */
void
CCallStackTracker::StackTraceInfo::Push( TCallStack* stack, const char* file, Int32 line )
{
    if ( stack->items == stack->reservedStacksize )
    {
        stack->reservedStacksize += STACK_RESIZE_AMOUNT;
        stack->file = (const char**) realloc( (char**) stack->file, stack->reservedStacksize * sizeof(const char*) );
        stack->linenr = (int*) realloc( stack->linenr, stack->reservedStacksize * sizeof(Int32) );
        stack->entryTickCount = (UInt64*) realloc( stack->entryTickCount, stack->reservedStacksize * sizeof(UInt64) );
        stack->staticsAreCopied = 0;
    }

    stack->file[ stack->items ]           = file;
    stack->linenr[ stack->items ]         = line;
    stack->entryTickCount[ stack->items ] = MT::PrecisionTickCount();
    ++stack->items;
}

/*-------------------------------------------------------------------------*/

/* static */
void
CCallStackTracker::StackTraceInfo::Pop( TCallStack* stack )
{
    if ( stack->items > 0 )
    {
        --stack->items;
    }
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::StackTraceInfo::CallstackScopeBegin( const char* file, Int32 line )
{
    Push( &m_callstack, file, line );
}

/*-------------------------------------------------------------------------*/

void
CCallStackTracker::StackTraceInfo::CallstackScopeEnd( void )
{
    Pop( &m_callstack );
}

/*-------------------------------------------------------------------------*/

bool
CCallStackTracker::StackTraceInfo::GetCallstackForCurrentThread( TCallStack** outStack )
{
    if ( GUCEF_NULL == outStack )
        return false;
    *outStack = &m_callstack;
    return true;
}

/*-------------------------------------------------------------------------*/

Int32
CCallStackTracker::StackTraceInfo::GetCallstackCopyForCurrentThread( TCallStack** outStack, bool alsoCopyStatics )
{
    TCallStack* scopeStack = GUCEF_NULL;
    if ( !GetCallstackForCurrentThread( &scopeStack ) )
        return -1;

    TCallStack* stackCopy = (TCallStack*) malloc( sizeof( TCallStack ) );
    if ( GUCEF_NULL == stackCopy )
    {
        *outStack = GUCEF_NULL;
        return -2;
    }

    stackCopy->threadid          = scopeStack->threadid;
    stackCopy->items             = scopeStack->items;
    stackCopy->reservedStacksize = stackCopy->items;

    UInt64 blockSize = (UInt64) stackCopy->items * sizeof(UInt64);
    stackCopy->entryTickCount = (UInt64*) malloc( (size_t) blockSize );
    if ( GUCEF_NULL == stackCopy->entryTickCount )
    {
        free( stackCopy );
        *outStack = GUCEF_NULL;
        return -2;
    }
    memcpy( stackCopy->entryTickCount, scopeStack->entryTickCount, (size_t) blockSize );

    blockSize = (UInt64) stackCopy->items * sizeof(const char*);
    stackCopy->file = (const char**) malloc( (size_t) blockSize );
    if ( GUCEF_NULL == stackCopy->file )
    {
        free( stackCopy->entryTickCount );
        free( stackCopy );
        *outStack = GUCEF_NULL;
        return -2;
    }

    if ( !alsoCopyStatics )
    {
        memcpy( stackCopy->file, scopeStack->file, (size_t) blockSize );
        stackCopy->staticsAreCopied = 0;
    }
    else
    {
        memset( stackCopy->file, 0, (size_t) blockSize );
        for ( UInt32 s = 0; s < stackCopy->items; ++s )
        {
            size_t filePathLength = strlen( scopeStack->file[ s ] );
            char* newBuf = (char*) malloc( filePathLength + 1 );
            if ( GUCEF_NULL != newBuf )
            {
                stackCopy->file[ s ] = newBuf;
                memcpy( (void*) stackCopy->file[ s ], scopeStack->file[ s ], filePathLength + 1 );
            }
            else
            {
                for ( UInt32 s2 = 0; s2 < s; ++s2 )
                    free( (void*) stackCopy->file[ s2 ] );
                free( stackCopy->entryTickCount );
                free( stackCopy->file );
                free( stackCopy );
                *outStack = GUCEF_NULL;
                return -2;
            }
        }
        stackCopy->staticsAreCopied = 1;
    }

    blockSize = (UInt64) stackCopy->items * sizeof(Int32);
    stackCopy->linenr = (Int32*) malloc( (size_t) blockSize );
    if ( GUCEF_NULL == stackCopy->linenr )
    {
        if ( stackCopy->staticsAreCopied )
        {
            for ( UInt32 s = 0; s < stackCopy->items; ++s )
                free( (void*) stackCopy->file[ s ] );
        }
        free( stackCopy->entryTickCount );
        free( stackCopy->file );
        free( stackCopy );
        *outStack = GUCEF_NULL;
        return -2;
    }
    memcpy( stackCopy->linenr, scopeStack->linenr, (size_t) blockSize );

    *outStack = stackCopy;
    return 1;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      C API — 1-line delegations to CCallStackTracker::Instance()        //
//                                                                         //
//-------------------------------------------------------------------------*/

void
MEMMAN_CallstackScopeBegin( const char* file, Int32 line )
{
    CCallStackTracker::Instance()->CallstackScopeBegin( file, line );
}

/*-------------------------------------------------------------------------*/

void
MEMMAN_CallstackScopeEnd( void )
{
    CCallStackTracker::Instance()->CallstackScopeEnd();
}

/*-------------------------------------------------------------------------*/

Int32
MEMMAN_GetCallstackForCurrentThread( TCallStack** outStack )
{
    return CCallStackTracker::Instance()->GetCallstackForCurrentThread( outStack );
}

/*-------------------------------------------------------------------------*/

Int32
MEMMAN_GetCallstackCopyForCurrentThread( TCallStack** outStack, UInt32 alsoCopyStatics )
{
    return CCallStackTracker::Instance()->GetCallstackCopyForCurrentThread( outStack, 0 != alsoCopyStatics );
}

/*-------------------------------------------------------------------------*/

void
MEMMAN_FreeCallstackCopy( TCallStack* stackCopy )
{
    CCallStackTracker::Instance()->FreeCallstackCopy( stackCopy );
}

/*-------------------------------------------------------------------------*/

void
GUCEF_PrintCallstack( void )
{
    CCallStackTracker::Instance()->PrintCallstack( stdout );
}

/*-------------------------------------------------------------------------*/

void
GUCEF_PrintCallstackCopy( TCallStack* stackToPrint )
{
    if ( GUCEF_NULL != stackToPrint )
        CCallStackTracker::Instance()->PrintCallstackCopy( *stackToPrint, stdout );
}

/*-------------------------------------------------------------------------*/

void
GUCEF_PrintCallstackCopyTo( TCallStack* stackToPrint, FILE* dest )
{
    if ( GUCEF_NULL != stackToPrint )
        CCallStackTracker::Instance()->PrintCallstackCopy( *stackToPrint, dest );
}

/*-------------------------------------------------------------------------*/

void
GUCEF_DumpCallstack( const char* filename )
{
    FILE* fptr = fopen( filename, "wb" );
    if ( GUCEF_NULL != fptr )
    {
        CCallStackTracker::Instance()->PrintCallstack( fptr );
        fclose( fptr );
    }
}

/*-------------------------------------------------------------------------*/

void
GUCEF_ShutdowntCallstackUtility( void )
{
    CCallStackTracker::Deinstance();
}

/*-------------------------------------------------------------------------*/

void
GUCEF_InitCallstackUtility( void )
{
    CCallStackTracker::Instance();
}

/*-------------------------------------------------------------------------*/

void
GUCEF_SetStackLogging( const UInt32 logStackBool )
{
    CCallStackTracker::Instance()->SetIsStackLoggingEnabled( 0 != logStackBool );
}

/*-------------------------------------------------------------------------*/

void
GUCEF_LogStackToStdOut( void )
{
    CCallStackTracker::Instance()->SetStackLogOutputToStdOut();
}

/*-------------------------------------------------------------------------*/

void
GUCEF_SetStackLoggingInCvsFormat( const UInt32 logAsCvsBool )
{
    CCallStackTracker::Instance()->SetIsStackLoggingInCsvFormat( 0 != logAsCvsBool );
}

/*-------------------------------------------------------------------------*/

void
GUCEF_SetStackPushCallback( TStackPushCallback cBack )
{
    CCallStackTracker::Instance()->SetStackPushCallback( cBack );
}

/*-------------------------------------------------------------------------*/

void
GUCEF_SetStackPopCallback( TStackPopCallback cBack )
{
    CCallStackTracker::Instance()->SetStackPopCallback( cBack );
}

/*-------------------------------------------------------------------------*/

void
GUCEF_LogStackTo( const char* filename )
{
    CCallStackTracker::Instance()->SetLogFilename( filename );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MLF */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
