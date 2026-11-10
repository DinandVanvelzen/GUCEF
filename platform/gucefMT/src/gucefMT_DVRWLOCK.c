/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <assert.h>
#include <malloc.h>                   /* memory management */

#ifndef GUCEF_MT_MUTEX_H
#include "gucefMT_mutex.h"            /* O/S mutex wrapper */
#define GUCEF_MT_MUTEX_H
#endif /* GUCEF_MT_MUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"       /* wrappers for threading related O/S functionality */
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "gucefMT_DVRWLOCK.h"         /* rwlock function prototypes */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define RWLOCK_DEFAULT_THREAD_INFO_STACK_GROW_AMT         24

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SThreadSlot
{
    UInt32 threadId;
    Int32 reentrancyCount;

    struct SThreadSlot* nextSlot;
};
typedef struct SThreadSlot TThreadSlot;

/**
 *	Readers/Writers lock data storage.
 */
struct SRWLock
{
    UInt8  delflag;
    Int32 activeReaderCount;
    TThreadSlot* activeReaders;
    TThreadSlot* queuedWriters;
    TThreadSlot* freeThreadInfo;
    Int32 queuedReaderCount;
    UInt32 lastQueuedReaderThreadId;
    Int32 activeWriterCount;
    Int32 activeWriterReentrancyCount;
    Int32 queuedWriterCount;
    UInt8 writeLockAquisitionInProgress;
    UInt32 writeLockAquisitionThreadId;
    UInt32 wpriority;
    UInt32 lastWriterThreadId;
    struct SMutex* datalock;
    struct SMutex* writerlock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_MT_PRIVATE_C
TThreadSlot*
rwl_impl_get_free_slot( TRWLock* rwlock )
{
    GUCEF_BEGIN;
    
    if ( GUCEF_NULL == rwlock->freeThreadInfo )
    {
        for ( Int32 i=0; i<RWLOCK_DEFAULT_THREAD_INFO_STACK_GROW_AMT; ++i )
        {
            TThreadSlot* newSlot = (TThreadSlot*) calloc( 1, sizeof( TThreadSlot ) );
            if ( GUCEF_NULL == newSlot )
            {
                GUCEF_END;
                return GUCEF_NULL;
            }

            if ( GUCEF_NULL != rwlock->freeThreadInfo )
            {
                TThreadSlot* prevSlot = rwlock->freeThreadInfo;
                newSlot->nextSlot = prevSlot;
                rwlock->freeThreadInfo = newSlot;
            }
            else
            {
                rwlock->freeThreadInfo = newSlot;
            }
        }
    }

    if ( GUCEF_NULL != rwlock->freeThreadInfo )
    {
        TThreadSlot* slot = rwlock->freeThreadInfo;
        rwlock->freeThreadInfo = slot->nextSlot;
        slot->nextSlot = GUCEF_NULL;

        GUCEF_END;
        return slot;
    }

    GUCEF_END;
    return GUCEF_NULL;
}

/*--------------------------------------------------------------------------*/

GUCEF_MT_PRIVATE_C
TThreadSlot*
rwl_impl_get_reader_slot( TRWLock* rwlock, UInt32 threadId )
{
    GUCEF_BEGIN;

    TThreadSlot* slot = GUCEF_NULL;
    TThreadSlot* readerSlot = rwlock->activeReaders;

    /*
     *  Try to find an existing slot for this thread
     */
    while ( GUCEF_NULL != readerSlot )
    {
        if ( readerSlot->threadId == threadId )
            return readerSlot;
        readerSlot = readerSlot->nextSlot;
    }

    /*
     *  We dont have a reader slot for this thread.
     *  Its new to this lock. As such grab a new free slot and use that instead
     */
    slot = rwl_impl_get_free_slot( rwlock );
    if ( GUCEF_NULL != slot )
    {
        slot->threadId = threadId;
        readerSlot = rwlock->activeReaders;
        rwlock->activeReaders = slot;
        slot->nextSlot = readerSlot;

        /*
         *  We are adding to the total active reader threads
         */
        ++rwlock->activeReaderCount;
        
        GUCEF_END;
        return slot;
    }

    GUCEF_END;
    return slot;
}

/*--------------------------------------------------------------------------*/

GUCEF_MT_PRIVATE_C
TThreadSlot*
rwl_impl_get_queued_writer_slot( TRWLock* rwlock, UInt32 threadId )
{
    GUCEF_BEGIN;

    TThreadSlot* slot = GUCEF_NULL;
    TThreadSlot* queuedWriterSlot = rwlock->queuedWriters;

    /*
     *  Try to find an existing slot for this thread
     */
    while ( GUCEF_NULL != queuedWriterSlot )
    {
        if ( queuedWriterSlot->threadId == threadId )
            return queuedWriterSlot;
        queuedWriterSlot = queuedWriterSlot->nextSlot;
    }

    /*
     *  We dont have a reader slot for this thread.
     *  Its new to this lock. As such grab a new free slot and use that instead
     */
    slot = rwl_impl_get_free_slot( rwlock );
    if ( GUCEF_NULL != slot )
    {
        slot->threadId = threadId;
        queuedWriterSlot = rwlock->queuedWriters;
        rwlock->queuedWriters = slot;
        slot->nextSlot = queuedWriterSlot;

        /*
         *  We are adding to the total queued writer threads
         */
        ++rwlock->queuedWriterCount;
        
        GUCEF_END;
        return slot;
    }

    GUCEF_END;
    return slot;
}

/*--------------------------------------------------------------------------*/

GUCEF_MT_PRIVATE_C
void
rwl_impl_free_reader_slot( TRWLock* rwlock, UInt32 threadId )
{
    GUCEF_BEGIN;

    TThreadSlot* prevSlot = GUCEF_NULL;
    TThreadSlot* readerSlot = rwlock->activeReaders;

    /*
     *  Try to find the existing slot for this thread
     */
    while ( GUCEF_NULL != readerSlot )
    {
        if ( readerSlot->threadId == threadId )
        {
            /*
             *  Remove the thread's slot from the chain of active readers
             */
            if ( GUCEF_NULL != prevSlot )
                prevSlot->nextSlot = readerSlot->nextSlot;
            else
                rwlock->activeReaders = readerSlot->nextSlot;    

            /*
             *  Reset the slot for re-use and add to the free slot list
             */
            readerSlot->threadId = 0;
            readerSlot->reentrancyCount = 0;
            readerSlot->nextSlot = rwlock->freeThreadInfo;
            rwlock->freeThreadInfo = readerSlot;

            --rwlock->activeReaderCount;
            break;
        }
        prevSlot = readerSlot;
        readerSlot = readerSlot->nextSlot;
    }

    GUCEF_END;
}

/*--------------------------------------------------------------------------*/

GUCEF_MT_PRIVATE_C
void
rwl_impl_free_queued_writer_slot( TRWLock* rwlock, UInt32 threadId )
{
    GUCEF_BEGIN;

    TThreadSlot* prevSlot = GUCEF_NULL;
    TThreadSlot* queuedWriterSlot = rwlock->queuedWriters;

    /*
     *  Try to find the existing slot for this thread
     */
    while ( GUCEF_NULL != queuedWriterSlot )
    {
        if ( queuedWriterSlot->threadId == threadId )
        {
            /*
             *  Remove the thread's slot from the chain of queued writers
             */
            if ( GUCEF_NULL != prevSlot )
                prevSlot->nextSlot = queuedWriterSlot->nextSlot;
            else
                rwlock->queuedWriters = queuedWriterSlot->nextSlot;    

            /*
             *  Reset the slot for re-use and add to the free slot list
             */
            queuedWriterSlot->threadId = 0;
            queuedWriterSlot->reentrancyCount = 0;
            queuedWriterSlot->nextSlot = rwlock->freeThreadInfo;
            rwlock->freeThreadInfo = queuedWriterSlot;

            --rwlock->queuedWriterCount;
            break;
        }
        prevSlot = queuedWriterSlot;
        queuedWriterSlot = queuedWriterSlot->nextSlot;
    }

    GUCEF_END;
}

/*--------------------------------------------------------------------------*/

GUCEF_MT_PRIVATE_C
char
rwl_impl_push_active_reader( TRWLock* rwlock )
{
    GUCEF_BEGIN;
    
    UInt32 threadId = GetCurrentTaskID();

    TThreadSlot* threadSlot = rwl_impl_get_reader_slot( rwlock, threadId );
    if ( threadSlot == GUCEF_NULL )
        return 0;

    ++threadSlot->reentrancyCount;

    GUCEF_END;
    return 1;
}

/*--------------------------------------------------------------------------*/

GUCEF_MT_PRIVATE_C
char
rwl_impl_pop_active_reader( TRWLock* rwlock )
{
    GUCEF_BEGIN;
    
    if ( rwlock->activeReaderCount > 0 )
    {
        UInt32 threadId = GetCurrentTaskID();

        TThreadSlot* threadSlot = rwl_impl_get_reader_slot( rwlock, threadId );
        if ( threadSlot == GUCEF_NULL )
            return 0;
        
        --threadSlot->reentrancyCount;

        if ( 0 > threadSlot->reentrancyCount )
        {
            /* we should never get here if your push/pop counts match */
            GUCEF_ASSERT_ALWAYS;
        }

        if ( 0 == threadSlot->reentrancyCount )
            rwl_impl_free_reader_slot( rwlock, threadId );

        GUCEF_END;
        return 1;
    }
    else
    {
        /* we should never get here if your push/pop counts match */
        GUCEF_ASSERT_ALWAYS;
        GUCEF_END;
        return 0;
    }
}

/*--------------------------------------------------------------------------*/

GUCEF_MT_PRIVATE_C
char
rwl_impl_push_queued_writer( TRWLock* rwlock )
{
    GUCEF_BEGIN;
    
    UInt32 threadId = GetCurrentTaskID();

    TThreadSlot* threadSlot = rwl_impl_get_queued_writer_slot( rwlock, threadId );
    if ( threadSlot == GUCEF_NULL )
        return 0;

    ++threadSlot->reentrancyCount;

    GUCEF_END;
    return 1;
}

/*--------------------------------------------------------------------------*/

GUCEF_MT_PRIVATE_C
char
rwl_impl_pop_queued_writer( TRWLock* rwlock )
{
    GUCEF_BEGIN;
    
    if ( rwlock->queuedWriterCount > 0 )
    {
        UInt32 threadId = GetCurrentTaskID();

        TThreadSlot* threadSlot = rwl_impl_get_queued_writer_slot( rwlock, threadId );
        if ( threadSlot == GUCEF_NULL )
            return 0;
        
        --threadSlot->reentrancyCount;

        if ( 0 > threadSlot->reentrancyCount )
        {
            /* we should never get here if your push/pop counts match */
            GUCEF_ASSERT_ALWAYS;
        }

        if ( 0 == threadSlot->reentrancyCount )
            rwl_impl_free_queued_writer_slot( rwlock, threadId );

        GUCEF_END;
        return 1;
    }
    else
    {
        /* we should never get here if your push/pop counts match */
        GUCEF_ASSERT_ALWAYS;
        GUCEF_END;
        return 0;
    }
}

/*--------------------------------------------------------------------------*/

GUCEF_MT_PRIVATE_C
char
rwl_impl_has_foreign_reader( TRWLock* rwlock )
{
    GUCEF_BEGIN;

    UInt32 calledThreadId = GetCurrentTaskID();
    TThreadSlot* readerSlot = rwlock->activeReaders;

    while  ( GUCEF_NULL != readerSlot )
    {
        if ( readerSlot->threadId != calledThreadId )
        {
            GUCEF_END;
            return 1;
        }
        readerSlot = readerSlot->nextSlot;
    }
    
    GUCEF_END;
    return 0;
}

/*--------------------------------------------------------------------------*/

GUCEF_MT_PRIVATE_C
char
rwl_impl_has_special_access_for_multiple_readers_escalating_to_writers( TRWLock* rwlock )
{
    GUCEF_BEGIN;

    UInt32 calledThreadId = GetCurrentTaskID();

    /* 
     * We need to verify if ALL active readers are also queued writers
     * no exceptions allowed. If true then that tells you that all those readers are waiting for the other readers to finish
     * in order to grab the write lock. That will never happen since they are all waiting on eachohter. 
     * To eleviate this situation we will pick a winner which will be allowed to proceed, being granted 'special access' to 
     * escalate their read lock to a write lock
     */

    TThreadSlot* lastReaderSlot = GUCEF_NULL;
    TThreadSlot* readerSlot = rwlock->activeReaders;

    while  ( GUCEF_NULL != readerSlot )
    {        
        TThreadSlot* matchingQueuedWriter = GUCEF_NULL;
        TThreadSlot* queuedWriterSlot = rwlock->queuedWriters;
        while  ( GUCEF_NULL != queuedWriterSlot )
        {
            if ( readerSlot->threadId == queuedWriterSlot->threadId )
            {
                matchingQueuedWriter = queuedWriterSlot;
                break;
            }
            queuedWriterSlot = queuedWriterSlot->nextSlot;
        }

        if ( GUCEF_NULL == matchingQueuedWriter )
        {
            /*
             *  We have an active reader which does not have a matching queued writer
             *  This active reader should resolve their work normally and as such no special access will be granted at this time
             *  No point checking any of the other readers
             */
            GUCEF_END;
            return 0;
        }
        
        lastReaderSlot = readerSlot;
        readerSlot = readerSlot->nextSlot;
    }

    /*
     * If we get here we are in the scenario we are looking for
     * ALL currently active readers are also queued for writing
     * 
     * The way we resolve this and the purpose of this function is to pick a winner
     * In order to make it first-come first-serve we will use the first active reader as the winner
     * This is because the read attempt presumably is earlier in the workflow and the last reader in our linked list
     * is thus our first reader thread regardless of when that thread tried to escalate to being a writer
     */
    if ( GUCEF_NULL != lastReaderSlot )
    {
        if ( lastReaderSlot->threadId == calledThreadId )
        {
            /*
             *  The calling thread is the first active reader
             *  You are the winner!
             */
            GUCEF_END;
            return 1;
        }
    }
    
    GUCEF_END;
    return 0;
}

/*--------------------------------------------------------------------------*/

/**
 *	Function that creates a readers/writers lock data storage struct
 *      writer_overrules is a boolean. When non zero writers will be given
 *	a higher priority then readers. If 0 then readers will have priority
 *	over writers.
 */
TRWLock*
rwl_create( UInt32 writer_overrules )
{
	TRWLock* rwlock = malloc( sizeof( TRWLock ) );
    if ( GUCEF_NULL != rwlock )
    {
        UInt32 i=0;
        
        rwlock->delflag = 0;
        rwlock->activeReaderCount = 0;
        rwlock->activeReaders = GUCEF_NULL;
        rwlock->freeThreadInfo = GUCEF_NULL;
        rwlock->queuedReaderCount = 0;
        rwlock->lastQueuedReaderThreadId = 0;
        rwlock->activeWriterCount = 0;
        rwlock->activeWriterReentrancyCount = 0;
        rwlock->queuedWriterCount = 0;
        rwlock->queuedWriters = GUCEF_NULL;
        rwlock->writeLockAquisitionInProgress = 0;
        rwlock->writeLockAquisitionThreadId = 0;
        rwlock->wpriority = writer_overrules;
        rwlock->lastWriterThreadId = 0;
        rwlock->datalock = MutexCreate();
        rwlock->writerlock = MutexCreate();

        for ( i; i<RWLOCK_DEFAULT_THREAD_INFO_STACK_GROW_AMT; ++i  )
        {
            TThreadSlot* prevSlot = rwlock->freeThreadInfo;
            TThreadSlot* slot = (TThreadSlot*) calloc( 1, sizeof(TThreadSlot) );
            if ( GUCEF_NULL != slot )
            {
                rwlock->freeThreadInfo = slot;
                slot->nextSlot = prevSlot;
            }
            else
            {
                rwl_destroy( rwlock );
                return GUCEF_NULL;
            }
        }
    }
    return rwlock;
}

/*-------------------------------------------------------------------------*/

void
rwl_destroy( TRWLock* rwlock )
{
	if ( GUCEF_NULL != rwlock )
    {
        do 
        {
            if  ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
                /*
                 *	Set the delete flag.
                 */
                rwlock->delflag = 1;

                if ( MutexUnlock( rwlock->datalock ) > 0 )
                {                
                    do
                    {
                        /*
                         *	Wait data lock free for all readers and all writers to finish
                         */
                        while ( rwlock->activeReaderCount > 0              || 
                                rwlock->activeWriterCount > 0              ||
                                rwlock->queuedReaderCount > 0              ||
                                rwlock->queuedWriterCount > 0              ||
                                0 != rwlock->writeLockAquisitionInProgress  )
                        {
                            PrecisionDelay( 25 );
                        }

                        /*
                         *	Re-verify with the data lock that all readers and all writers are finished
                         *  If this comes to a different conclusion just go for another round
                         */
                        if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                        {
                            if ( 0 == rwlock->activeReaderCount             &&
                                 0 == rwlock->activeWriterCount             &&
                                 0 == rwlock->queuedReaderCount             &&
                                 0 == rwlock->queuedWriterCount             &&
                                 0 == rwlock->writeLockAquisitionInProgress )
                            {
                                TThreadSlot* slot = GUCEF_NULL;
                                
                                /*
                                 *	Cleanup allocated storage
                                 */
                                MutexDestroy( rwlock->writerlock );
                                rwlock->writerlock = GUCEF_NULL;
                                MutexUnlock( rwlock->datalock );    /* a mutex is never destroyed while locked */
                                MutexDestroy( rwlock->datalock );
                                rwlock->datalock = GUCEF_NULL;
                                
                                slot = rwlock->activeReaders;
                                while ( GUCEF_NULL != slot )
                                {
                                    TThreadSlot* nextSlot = slot->nextSlot;
                                    free( slot );
                                    slot = nextSlot;
                                }
                                rwlock->activeReaders = GUCEF_NULL;

                                slot = rwlock->queuedWriters;
                                while ( GUCEF_NULL != slot )
                                {
                                    TThreadSlot* nextSlot = slot->nextSlot;
                                    free( slot );
                                    slot = nextSlot;
                                }
                                rwlock->queuedWriters = GUCEF_NULL;

                                slot = rwlock->freeThreadInfo;
                                while ( GUCEF_NULL != slot )
                                {
                                    TThreadSlot* nextSlot = slot->nextSlot;
                                    free( slot );
                                    slot = nextSlot;
                                }
                                rwlock->freeThreadInfo = GUCEF_NULL;

                                free( rwlock );
                                return;
                            }
                            else
                            {
                                MutexUnlock( rwlock->datalock );
                            }
                        }
                    }
                    while ( 1 > 0 );
                }
            }            
        }
        while ( 1 > 0 );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
rwl_writer_overrules( const TRWLock* rwlock )
{
	if ( GUCEF_NULL != rwlock )
    {
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
                UInt32 wpriority = rwlock->wpriority;
                MutexUnlock( rwlock->datalock );
                return wpriority;
            }
        }
        while ( 0 == rwlock->delflag );
    }
    return 1;
}

/*--------------------------------------------------------------------------*/

UInt32
rwl_active_writers( const TRWLock* rwlock )
{
	if ( GUCEF_NULL != rwlock )
    {
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
                UInt32 activeWriterCount = rwlock->activeWriterCount;
                MutexUnlock( rwlock->datalock );
                return activeWriterCount;
            }
        }
        while ( 0 == rwlock->delflag );
    }
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt32
rwl_active_writer_reentrancy_depth( const TRWLock *rwlock )
{
	if ( GUCEF_NULL != rwlock )
    {
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
                UInt32 activeWriterReentrancyCount = rwlock->activeWriterReentrancyCount;
                MutexUnlock( rwlock->datalock );
                return activeWriterReentrancyCount;
            }
        }
        while ( 0 == rwlock->delflag );
    }
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt32
rwl_active_reader_reentrancy_depth( const TRWLock *rwlock, UInt32 threadId )
{
	GUCEF_BEGIN;

    if ( GUCEF_NULL != rwlock )
    {
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
                TThreadSlot* readerSlot = rwlock->activeReaders;

                /*
                 *  Try to find an existing slot for this thread
                 */
                while ( GUCEF_NULL != readerSlot )
                {
                    if ( readerSlot->threadId == threadId )
                    {
                        Int32 activeReaderReentrancyCount = readerSlot->reentrancyCount - 1;
                        MutexUnlock( rwlock->datalock );

                        GUCEF_END;
                        return activeReaderReentrancyCount;
                    }
                    readerSlot = readerSlot->nextSlot;
                }

                MutexUnlock( rwlock->datalock );
                GUCEF_END;
                return 0;
            }
        }
        while ( 0 == rwlock->delflag );
    }
    
    GUCEF_END;
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt32
rwl_queued_writers( const TRWLock* rwlock )
{
	GUCEF_BEGIN;

    if ( GUCEF_NULL != rwlock )
    {
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
                UInt32 queuedWriterCount = rwlock->queuedWriterCount;
                MutexUnlock( rwlock->datalock );

                GUCEF_END;
                return queuedWriterCount;
            }
        }
        while ( 0 == rwlock->delflag );
    }

    GUCEF_END;
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
rwl_active_readers( const TRWLock *rwlock )
{
	GUCEF_BEGIN;
    
    if ( GUCEF_NULL != rwlock )
    {
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
                UInt32 activeReaderCount = rwlock->activeReaderCount;
                MutexUnlock( rwlock->datalock );

                GUCEF_END;
                return activeReaderCount;
            }
        }
        while ( 0 == rwlock->delflag );
    }

    GUCEF_END;
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
rwl_queued_readers( const TRWLock *rwlock )
{
	GUCEF_BEGIN;

    if ( GUCEF_NULL != rwlock )
    {
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
                UInt32 queuedReaderCount = rwlock->queuedReaderCount;
                MutexUnlock( rwlock->datalock );
                
                GUCEF_END;
                return queuedReaderCount;
            }
        }
        while ( 0 == rwlock->delflag );
    }
    
    GUCEF_END;
    return 0;
}

/*--------------------------------------------------------------------------*/

UInt32
rwl_reader_start( TRWLock* rwlock, UInt32 lockWaitTimeoutInMs )
{
	if ( GUCEF_NULL != rwlock )
    {
        do
        {
            UInt8 done = 0;
            UInt32 lockResult = MutexLock( rwlock->datalock, lockWaitTimeoutInMs ); 
            switch ( lockResult )
            {
	            case GUCEF_MUTEX_OPERATION_SUCCESS :
                case GUCEF_MUTEX_ABANDONED :
                {
                
                    /*
                     *	First we check if destroy has not been called.
                     */
	                if ( 0 != rwlock->delflag )
                    {                
                        MutexUnlock( rwlock->datalock );
                        return GUCEF_RWLOCK_OPERATION_FAILED;
                    }

                    if ( 0 == rwlock->activeWriterCount && 0 == rwlock->queuedWriterCount && 0 == rwlock->writeLockAquisitionInProgress )
                    {
                        /*
                         *  There are no active writers so regardless who takes priority we can proceed
                         */
                        if ( rwl_impl_push_active_reader( rwlock ) != 0 )
                        {
                            MutexUnlock( rwlock->datalock );
                            return GUCEF_RWLOCK_OPERATION_SUCCESS;
                        }
                        else
                        {
                            MutexUnlock( rwlock->datalock );
                            return GUCEF_RWLOCK_OPERATION_FAILED;
                        }
                    }

                    if ( 0 == rwlock->wpriority && 0 == rwlock->activeWriterCount && 0 == rwlock->writeLockAquisitionInProgress )
                    {
                        /*
                         *  There are no active writers and readers are taking priority so any queued writers will have to wait
                         */
                        if ( rwl_impl_push_active_reader( rwlock ) != 0 )
                        {
                            MutexUnlock( rwlock->datalock );
                            return GUCEF_RWLOCK_OPERATION_SUCCESS;
                        }
                        else
                        {
                            MutexUnlock( rwlock->datalock );
                            return GUCEF_RWLOCK_OPERATION_FAILED;
                        }
                    }

                    if ( rwlock->activeWriterCount > 0 )
                    {
                        /*
                         *  The currently active writer could actually be the same thread
                         *  trying to obtain a read lock in another scope, acting as a reader.
                         *  We have to check for this or the thread can self-deadlock
                         */
                        if ( rwlock->lastWriterThreadId == GetCurrentTaskID() )
                        {
                            /*
                             *  The reader thread is also the writer thread
                             *  allow it to proceed since we already have write access anyway
                             */
                            if ( rwl_impl_push_active_reader( rwlock ) != 0 )
                            {
                                MutexUnlock( rwlock->datalock );
                                return GUCEF_RWLOCK_OPERATION_SUCCESS;
                            }
                            else
                            {
                                MutexUnlock( rwlock->datalock );
                                return GUCEF_RWLOCK_OPERATION_FAILED;
                            }
                        }
                    }                    
                    
                    /*
                     *	How we behave depends on wheter readers have priority or
                     *	wheter writers have priority.
                     */
                    if ( rwlock->wpriority > 0 )
                    {
                        rwlock->queuedReaderCount++;
                        rwlock->lastQueuedReaderThreadId = GetCurrentTaskID();
                        MutexUnlock( rwlock->datalock );

                        done = 0;
                        do
                        {
                            /*
                             *	If writers have priority we will wait for all writers
                             *	to finish this means the current writer and any writer
                             *	that may be queued. After that we increase the reader
                             *	counter.
                             */
                            Float64 elapsedTimeInMs = 0;
                            Float64 timerResolution = PrecisionTimerResolution() / 1000.0;
                            UInt64 startTicks = PrecisionTickCount();
                            while ( ( rwlock->activeWriterCount > 0 || rwlock->queuedWriterCount > 0 || 0 != rwlock->writeLockAquisitionInProgress ) && 
                                    ( 0 == rwlock->delflag )                                                                                         &&
                                    ( elapsedTimeInMs <= lockWaitTimeoutInMs )                                                                        )
                            {
                                PrecisionDelay( 25 );
                                elapsedTimeInMs = ( PrecisionTickCount() - startTicks ) / timerResolution;
                                if ( elapsedTimeInMs > lockWaitTimeoutInMs )
                                {
                                    /*
                                     *	We exceeded the max time we can wait for the writers to finish their work
                                     *  Unroll the administrative changes and return a timeout
                                     */  
                                    lockResult = MutexLock( rwlock->datalock, GUCEF_MT_SUPER_LONG_LOCK_TIMEOUT ); 
                                    switch ( lockResult )
                                    {
	                                    case GUCEF_MUTEX_OPERATION_SUCCESS :
                                        case GUCEF_MUTEX_ABANDONED :
                                        {
                                            rwlock->queuedReaderCount--;
                                            MutexUnlock( rwlock->datalock );
                                            return GUCEF_RWLOCK_WAIT_TIMEOUT;
                                        }
                                        default:
                                        case GUCEF_MUTEX_WAIT_TIMEOUT:
                                        case GUCEF_MUTEX_OPERATION_FAILED:
                                        {
                                            /*
                                             *	This should not happen
                                             *  Roll the dice on updating without a lock
                                             */
                                            GUCEF_ASSERT_ALWAYS;
                                            rwlock->queuedReaderCount--;
                                            return GUCEF_RWLOCK_WAIT_TIMEOUT;
                                        }
                                    }
                                }
                            }

                            /*
                             *	Re-verify with the data lock that all writers are finished
                             *  If this comes to a different conclusion just go for another round
                             */
                            lockResult = MutexLock( rwlock->datalock, lockWaitTimeoutInMs );
                            switch ( lockResult )
                            {
	                            case GUCEF_MUTEX_OPERATION_SUCCESS :
                                case GUCEF_MUTEX_ABANDONED :
                                {
                                    if ( 0 == rwlock->activeWriterCount && 0 == rwlock->queuedWriterCount && 0 == rwlock->writeLockAquisitionInProgress )
                                    {
                                        /*
                                         *  The writers are finished and readers can now proceed
                                         */
                                        if ( rwl_impl_push_active_reader( rwlock ) != 0 ) 
                                        {
                                            rwlock->queuedReaderCount--;
                                            MutexUnlock( rwlock->datalock );
                                            return GUCEF_RWLOCK_OPERATION_SUCCESS;
                                        }
                                        else
                                        {
                                            MutexUnlock( rwlock->datalock );
                                            return GUCEF_RWLOCK_OPERATION_FAILED;    
                                        }
                                    }
                                    else
                                    {
                                        MutexUnlock( rwlock->datalock );
                                        done = 1;
                                    }
                                    break;
                                }
                                default:
                                case GUCEF_MUTEX_WAIT_TIMEOUT:
                                case GUCEF_MUTEX_OPERATION_FAILED:
                                {
                                    break;
                                }
                            }
                        }
                        while ( 0 == rwlock->delflag && 0 == done );
                        if ( 0 != rwlock->delflag )
                        {
                            UInt8 done = 0;
                            do
                            {
                                /*
                                 *  Lock destruction started while we were waiting to become an active reader
                                 *  Unregister as queued and give up
                                 */
                                lockResult = MutexLock( rwlock->datalock, lockWaitTimeoutInMs );
                                switch ( lockResult )
                                {
	                                case GUCEF_MUTEX_OPERATION_SUCCESS :
                                    case GUCEF_MUTEX_ABANDONED :
                                    {                                                       
                                        --rwlock->queuedReaderCount;
                                        MutexUnlock( rwlock->datalock );
                                        done = 1;
                                        break;
                                    }
                                    default:
                                    case GUCEF_MUTEX_WAIT_TIMEOUT:
                                    case GUCEF_MUTEX_OPERATION_FAILED:
                                    {
                                        break;
                                    }
                                }
                            }
                            while ( 0 == done );
                        }
                        return GUCEF_RWLOCK_OPERATION_FAILED;
                    }
                    else
                    {
                        rwlock->queuedReaderCount++;
                        rwlock->lastQueuedReaderThreadId = GetCurrentTaskID();
                        MutexUnlock( rwlock->datalock );

                        do
                        {
                            /*
                             *	If readers have priority we will wait for all active writers
                             *	to finish but not queued writers
                             */
                            while ( ( rwlock->activeWriterCount > 0 || 0 != rwlock->writeLockAquisitionInProgress ) && 0 == rwlock->delflag )
                            {
                                PrecisionDelay( 25 );
                            }

                            /*
                             *	Re-verify with the data lock that all active writers are finished
                             *  If this comes to a different conclusion just go for another round
                             */
                            lockResult = MutexLock( rwlock->datalock, lockWaitTimeoutInMs );
                            switch ( lockResult )
                            {
	                            case GUCEF_MUTEX_OPERATION_SUCCESS :
                                case GUCEF_MUTEX_ABANDONED :
                                {
                                    if ( 0 == rwlock->activeWriterCount && 0 == rwlock->writeLockAquisitionInProgress )
                                    {
                                        /*
                                         *  The active writers are finished and readers can now proceed
                                         */
                                        if ( rwl_impl_push_active_reader( rwlock ) != 0 ) 
                                        {
                                            rwlock->queuedReaderCount--;
                                            MutexUnlock( rwlock->datalock );
                                            return GUCEF_RWLOCK_OPERATION_SUCCESS;
                                        }
                                        else
                                        {
                                            MutexUnlock( rwlock->datalock );
                                            return GUCEF_RWLOCK_OPERATION_FAILED;    
                                        }
                                    }
                                    else
                                    {
                                        MutexUnlock( rwlock->datalock );
                                    }
                                    break;
                                }
                                default:
                                case GUCEF_MUTEX_WAIT_TIMEOUT:
                                case GUCEF_MUTEX_OPERATION_FAILED:
                                {
                                    break;
                                }
                            }
                        }
                        while ( 0 == rwlock->delflag );
                        if ( 0 != rwlock->delflag )
                        {
                            do
                            {
                                /*
                                 *  Lock destruction started while we were waiting to become an active reader
                                 *  Unregister as queued and give up
                                 */
                                lockResult = MutexLock( rwlock->datalock, lockWaitTimeoutInMs );
                                switch ( lockResult )
                                {
	                                case GUCEF_MUTEX_OPERATION_SUCCESS :
                                    case GUCEF_MUTEX_ABANDONED :
                                    {                                                                                  
                                        --rwlock->queuedReaderCount;
                                        MutexUnlock( rwlock->datalock );
                                        break;
                                    }
                                    default:
                                    case GUCEF_MUTEX_WAIT_TIMEOUT:
                                    case GUCEF_MUTEX_OPERATION_FAILED:
                                    {
                                        break;
                                    }
                                }
                            }
                            while ( 1 > 0 );
                        }
                        return GUCEF_RWLOCK_OPERATION_FAILED;
                    }
                    break;
                }
                case GUCEF_MUTEX_WAIT_TIMEOUT :
                {
                    return GUCEF_RWLOCK_WAIT_TIMEOUT;    
                }
                default:
                case GUCEF_MUTEX_OPERATION_FAILED:
                {
                    break;
                }
            }
        }
        while ( 0 == rwlock->delflag );
    }
    return GUCEF_RWLOCK_OPERATION_FAILED;
}

/*--------------------------------------------------------------------------*/

UInt32
rwl_reader_stop( TRWLock *rwlock )
{
	GUCEF_BEGIN;
    if ( GUCEF_NULL != rwlock )
    {
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
                if ( rwlock->activeWriterCount > 0 )
                {
                    if ( rwlock->lastWriterThreadId == GetCurrentTaskID() )
                    {
                        /*
                         *  The caller is the currently active writer thread who also grabbed a read lock
                         *  this is considered ok since the write lock is read-write access and already covered the read access
                         */
                        if ( rwl_impl_pop_active_reader( rwlock ) != 0 )
                        {
                            MutexUnlock( rwlock->datalock );
                            GUCEF_END;
                            return GUCEF_RWLOCK_OPERATION_SUCCESS;
                        }
                        else
                        {
                            MutexUnlock( rwlock->datalock );
                            GUCEF_END;
                            return GUCEF_RWLOCK_OPERATION_FAILED;
                        }
                    }
                    else
                    {
                        /*
                         *  Something is wrong here.
                         *  How could you have gotten a read lock in the first place if there is an active writer?
                         *  Placing a call to rwl_reader_stop() without a successfull call to rwl_reader_start() ?
                         */
                        MutexUnlock( rwlock->datalock );
                        GUCEF_ASSERT_ALWAYS;
                        GUCEF_END;
                        return GUCEF_RWLOCK_OPERATION_FAILED;
                    }
                }               

                if ( rwlock->activeReaderCount > 0 )
                {
                    if ( rwl_impl_pop_active_reader( rwlock ) != 0 )
                    {
                        MutexUnlock( rwlock->datalock );
                        GUCEF_END;
                        return GUCEF_RWLOCK_OPERATION_SUCCESS;
                    }
                    else
                    {
                        MutexUnlock( rwlock->datalock );
                        GUCEF_END;
                        return GUCEF_RWLOCK_OPERATION_FAILED;
                    }
                }
                else
                {
                    /*
                     *  Something is wrong here.
                     *  Most likely you have a mismatched calling pattern to lock vs unlock
                     *  It is required to call unlock exactly as many times as you call lock
                     */   
                    GUCEF_ASSERT_ALWAYS;
                    MutexUnlock( rwlock->datalock );
                    GUCEF_END;
                    return GUCEF_RWLOCK_OPERATION_FAILED;                     
                }
            }
        }
        while ( 0 == rwlock->delflag );
    }    
    GUCEF_END;
    return GUCEF_RWLOCK_OPERATION_FAILED;
}

/*--------------------------------------------------------------------------*/

UInt32
rwl_reader_transition_to_writer( TRWLock* rwlock, UInt32 lockWaitTimeoutInMs )
{
	GUCEF_BEGIN;
    if ( GUCEF_NULL != rwlock )
    {
        UInt32 callerThreadId = GetCurrentTaskID();
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
	            /*
                 *	First we check if destroy has not been called.
                 */
	            if ( 0 != rwlock->delflag )
                {                
                    /*
                     *  Refusing request because we have to prep for deletion
                     */
                    MutexUnlock( rwlock->datalock );
                    GUCEF_END;
                    return GUCEF_RWLOCK_OPERATION_FAILED;
                }

                if ( rwlock->activeWriterCount > 0               &&
                     rwlock->lastWriterThreadId == callerThreadId )
                {
                    /*
                     *  We are in the scenario of a writer which 
                     *  called code that took a reader lock as a reentrancy no-op since a write lock is implicitly read-write
                     *  Said reader code doesnt itself know this and perhaps thinks it has just a reader lock
                     *  We just move the reentrancy from reading to writing to update our administration
                     */
                    if ( rwl_impl_pop_active_reader( rwlock ) != 0 )
                    {
                        /*
                         *  Make sure the write lock itself is in-sync wrt its reentrancy count 
                         */
                        if ( MutexLock( rwlock->writerlock, lockWaitTimeoutInMs ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                        {
                            rwlock->activeWriterReentrancyCount = MutexReentrancy( rwlock->writerlock );
                            MutexUnlock( rwlock->datalock );
                            return GUCEF_RWLOCK_OPERATION_SUCCESS;
                        }
                        else
                        {
                            /*
                             *  You should not get here
                             *  We performed the sanity checks that should have allowed the writer lock grab to always work
                             */
                            rwl_impl_push_active_reader( rwlock );
                            MutexUnlock( rwlock->datalock );
                            return GUCEF_RWLOCK_OPERATION_FAILED;
                        }
                    }
                    else
                    {
                        MutexUnlock( rwlock->datalock );
                        return GUCEF_RWLOCK_OPERATION_FAILED;
                    }
                }

	            /*
                 *	Sanity check
                 */
                if ( ( ( rwlock->activeWriterCount > 0            ||
                         rwlock->activeWriterReentrancyCount > 0  ) && rwlock->lastWriterThreadId != callerThreadId ) ||
                     rwlock->activeReaderCount <= 0           )
                {
                    /*
                     *  You should not get here
                     *  We have not met the prerequisite conditions to transition a reader
                     *      - There must be no active writer because presumably you already have an active reader
                     *              Exception is the current is already both a reader and a writer.
                     *              In that case you are allowed to transition the reader part since you 
                     *              already have a write lock anyway
                     *      - There must be an active reader because how else can you transition it
                     */
                    GUCEF_ASSERT_ALWAYS;
                    MutexUnlock( rwlock->datalock );
                    GUCEF_END;
                    return GUCEF_RWLOCK_OPERATION_FAILED;
                }

                if ( rwl_impl_pop_active_reader( rwlock ) != 0 )
                {
                    if ( rwl_impl_push_queued_writer( rwlock ) != 0 ) 
                    {

                    }
                    else
                    {
   
                    }
                }
                else
                {
                    MutexUnlock( rwlock->datalock );
                    GUCEF_END;
                    return GUCEF_RWLOCK_OPERATION_FAILED;
                }
                
                if ( 0 == rwlock->activeWriterCount && 0 == rwl_impl_has_foreign_reader( rwlock ) )
                {
                    /*
                     *  There are no active writers or other thread readers so we can make this quick
                     */
                    while ( GUCEF_MUTEX_OPERATION_SUCCESS != MutexLock( rwlock->writerlock, GUCEF_MUTEX_INFINITE_TIMEOUT ) ) {};
                    rwlock->activeWriterCount++;        
                    if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                    {

                    }
                    else
                    {
   
                    }
                    rwlock->activeWriterReentrancyCount = MutexReentrancy( rwlock->writerlock );
                    rwlock->lastWriterThreadId = GetCurrentTaskID(); 
                    MutexUnlock( rwlock->datalock );
                    GUCEF_END;
                    return GUCEF_RWLOCK_OPERATION_SUCCESS;                    
                }

                if ( rwlock->wpriority > 0 )
                {
                    MutexUnlock( rwlock->datalock );
                    
                    do
                    {
                        if ( 1 >= rwlock->activeReaderCount             &&
                             0 == rwlock->writeLockAquisitionInProgress )
                        {
                            /*
                             *  Reverify with a data lock
                             *  We have to hurry and a new reader may slip in
                             */
                            UInt8 reverifyFailed = 0;
                            do
                            {
                                if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                {
                                    if ( 0 == rwl_impl_has_foreign_reader( rwlock ) &&
                                         0 == rwlock->writeLockAquisitionInProgress )
                                    {
                                        /*
                                         *  We found a moment without any active readers at all
                                         *  Now we just have to wait for other writers if any
                                         *
                                         *  The 'queuedWriterCount' already blocks new active readers but for consistency we also
                                         *  set the writeLockAquisitionInProgress flag 
                                         */
                                        rwlock->writeLockAquisitionInProgress = 1;
                                        rwlock->writeLockAquisitionThreadId = GetCurrentTaskID();
                                        MutexUnlock( rwlock->datalock );
                    
                                        do
                                        {
                                            if ( MutexLock( rwlock->writerlock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                            {
                                                /*
                                                 *  Now that we have the write lock we need to still go back and update the administration
                                                 *  our queued writer is now an active writer
                                                 */
                                                UInt32 dataLockResult = 0;
                                                do
                                                {
                                                    dataLockResult = MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT );
                                                    if ( GUCEF_MUTEX_OPERATION_SUCCESS == dataLockResult )
                                                    {                            
                                                        /*
                                                         *  We have successfully turned our active reader into a queued writer and 
                                                         *  after that turned the queued writer into the active writer
                                                         */
                                                        rwlock->activeWriterCount++;
                                                        rwlock->lastWriterThreadId = GetCurrentTaskID();
                                                        rwlock->writeLockAquisitionInProgress = 0;
                                                        if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                        {

                                                        }
                                                        else
                                                        {
   
                                                        }
                                                        rwlock->activeWriterReentrancyCount = MutexReentrancy( rwlock->writerlock );
                                                        MutexUnlock( rwlock->datalock );
                                                        GUCEF_END;
                                                        return GUCEF_RWLOCK_OPERATION_SUCCESS;
                                                    }
                                                }
                                                while ( GUCEF_MUTEX_OPERATION_SUCCESS != dataLockResult );

                                                /*
                                                 *  Logically we should never get here
                                                 */
                                                GUCEF_ASSERT_ALWAYS;
                                                rwlock->writeLockAquisitionInProgress = 0;
                                                if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                {

                                                }
                                                else
                                                {
   
                                                }
                                                MutexUnlock( rwlock->writerlock );
                                                GUCEF_END;
                                                return GUCEF_RWLOCK_OPERATION_FAILED;
                                            }
                                        }
                                        while ( 0 == rwlock->delflag );
                                        if ( 0 != rwlock->delflag )
                                        {
                                            do
                                            {
                                                /*
                                                 *  Lock destruction started while we were waiting to grab the lock
                                                 *  Unregister as queued and give up
                                                 */
                                                if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                                {                                                        
                                                    if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                    {

                                                    }
                                                    else
                                                    {
   
                                                    }
                                                    rwlock->writeLockAquisitionInProgress = 0;
                                                    MutexUnlock( rwlock->datalock );
                                                    break;
                                                }
                                            }
                                            while ( 1 > 0 );
                                        }
                                        GUCEF_END;
                                        return GUCEF_RWLOCK_OPERATION_FAILED;
                                    }
                                    else
                                    {
                                        reverifyFailed = 1;
                                        MutexUnlock( rwlock->datalock );
                                    }
                                }
                            }
                            while ( 0 == rwlock->delflag && 0 == reverifyFailed );
                        }
                        else
                        {
                            PrecisionDelay( 25 );
                        }
                    }
                    while ( 0 == rwlock->delflag );
                    if ( 0 != rwlock->delflag )
                    {
                        do
                        {
                            /*
                             *  Lock destruction started while we were waiting to grab the lock
                             *  Unregister as queued and give up
                             */
                            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                            {                                                        
                                if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                {

                                }
                                else
                                {
   
                                }
                                MutexUnlock( rwlock->datalock );
                                break;
                            }
                        }
                        while ( 1 > 0 );
                    }
                    GUCEF_END;
                    return GUCEF_RWLOCK_OPERATION_FAILED;
                }
                else
                {
                    /*
                     *  First wait for all the active and queued readers to finish
                     *  We need to grab the moment that happens via constant monitoring
                     */
                    MutexUnlock( rwlock->datalock );
                    
                    do
                    {
                        if ( 1 >= rwlock->activeReaderCount             && 
                             0 == rwlock->queuedReaderCount             &&
                             0 == rwlock->writeLockAquisitionInProgress )
                        {
                            /*
                             *  Reverify with a data lock
                             *  We have to hurry and a new reader may slip in
                             */
                            UInt8 reverifyFailed = 0;
                            do
                            {
                                if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                {
                                    if ( 0 == rwl_impl_has_foreign_reader( rwlock ) && 
                                         0 == rwlock->queuedReaderCount             &&
                                         0 == rwlock->writeLockAquisitionInProgress  )
                                    {
                                        /*
                                         *  We found a moment without any other thread readers at all
                                         *  Now we just have to wait for other writers if any
                                         *
                                         *  Here we cheat a little to have a chance:
                                         *  The writeLockAquisitionInProgress flag blocks new active readers
                                         *  It temporarly overrules the reader priority while we grab the other lock
                                         */
                                        rwlock->writeLockAquisitionInProgress = 1;
                                        rwlock->writeLockAquisitionThreadId = GetCurrentTaskID();
                                        MutexUnlock( rwlock->datalock );
                    
                                        do
                                        {
                                            if ( MutexLock( rwlock->writerlock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                            {
                                                /*
                                                 *  Now that we have the write lock we need to still go back and update the administration
                                                 *  our queued writer is now an active writer
                                                 */
                                                UInt32 dataLockResult = 0;
                                                do
                                                {
                                                    dataLockResult = MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT );
                                                    if ( GUCEF_MUTEX_OPERATION_SUCCESS == dataLockResult )
                                                    {                            
                                                        /*
                                                         *  We have successfully turned our active reader into a queued writer and 
                                                         *  after that turned the queued writer into the active writer
                                                         */
                                                        rwlock->activeWriterCount++;
                                                        rwlock->lastWriterThreadId = GetCurrentTaskID();
                                                        rwlock->writeLockAquisitionInProgress = 0;
                                                        rwlock->activeWriterReentrancyCount = MutexReentrancy( rwlock->writerlock );
                                                        if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                        {

                                                        }
                                                        else
                                                        {
   
                                                        }
                                                        MutexUnlock( rwlock->datalock );
                                                        GUCEF_END;
                                                        return GUCEF_RWLOCK_OPERATION_SUCCESS;                                    
                                                    }
                                                }
                                                while ( GUCEF_MUTEX_OPERATION_SUCCESS != dataLockResult );
                                                
                                                /*
                                                 *  Logically we should never get here
                                                 */
                                                GUCEF_ASSERT_ALWAYS;
                                                rwlock->writeLockAquisitionInProgress = 0;
                                                if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                {

                                                }
                                                else
                                                {
   
                                                }
                                                rwlock->activeWriterReentrancyCount = MutexReentrancy( rwlock->writerlock );
                                                MutexUnlock( rwlock->writerlock );
                                                GUCEF_END;
                                                return GUCEF_RWLOCK_OPERATION_FAILED;
                                            }
                                        }
                                        while ( 0 == rwlock->delflag );
                                        if ( 0 != rwlock->delflag )
                                        {
                                            do
                                            {
                                                /*
                                                 *  Lock destruction started while we were waiting to grab the lock
                                                 *  Unregister as queued and give up
                                                 */
                                                if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                                {                                                        
                                                    if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                    {

                                                    }
                                                    else
                                                    {
   
                                                    }
                                                    rwlock->writeLockAquisitionInProgress = 0;
                                                    MutexUnlock( rwlock->datalock );
                                                    break;
                                                }
                                            }
                                            while ( 1 > 0 );
                                        }
                                        GUCEF_END;
                                        return GUCEF_RWLOCK_OPERATION_FAILED;
                                    }
                                    else
                                    {
                                        reverifyFailed = 1;
                                        MutexUnlock( rwlock->datalock );
                                    }
                                }
                            }
                            while ( 0 == rwlock->delflag && 0 == reverifyFailed );
                        }
                        else
                        {
                            PrecisionDelay( 25 );
                        }
                    }
                    while ( 0 == rwlock->delflag );
                    if ( 0 != rwlock->delflag )
                    {
                        do
                        {
                            /*
                             *  Lock destruction started while we were waiting to grab the lock
                             *  Unregister as queued and give up
                             */
                            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                            {                                                        
                                if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                {

                                }
                                else
                                {
   
                                }
                                MutexUnlock( rwlock->datalock );
                                break;
                            }
                        }
                        while ( 1 > 0 );
                    }
                    GUCEF_END;
                    return GUCEF_RWLOCK_OPERATION_FAILED;
                }
            }
        }
        while ( 0 == rwlock->delflag );
    }    
    GUCEF_END;
    return GUCEF_RWLOCK_OPERATION_FAILED;
}

/*--------------------------------------------------------------------------*/

UInt32
rwl_writer_start( TRWLock* rwlock, UInt32 lockWaitTimeoutInMs )
{
	if ( GUCEF_NULL != rwlock )
    {
        do
        {
            UInt8 done = 0;
            UInt32 lockResult = MutexLock( rwlock->datalock, lockWaitTimeoutInMs ); 
            switch ( lockResult )
            {
	            case GUCEF_MUTEX_OPERATION_SUCCESS :
                case GUCEF_MUTEX_ABANDONED :
                {
	                /*
                     *	First we check if destroy has not been called.
                     */
	                if ( 0 != rwlock->delflag )
                    {                
                        MutexUnlock( rwlock->datalock );
                        return GUCEF_RWLOCK_OPERATION_FAILED;
                    }

                    if ( rwl_impl_push_queued_writer( rwlock ) != 0 ) 
                    {

                    }
                    else
                    {
   
                    }
                    if ( 0 < rwlock->activeWriterCount )
                    {
                        /*
                         *  The currently active writer could actually be the same thread
                         *  trying to obtain another write lock in another scope
                         *  We have to check for this or the thread can self-deadlock
                         */
                        if ( rwlock->lastWriterThreadId == GetCurrentTaskID() )
                        {
                            /*
                             *  The caller is already the currently active writer thread
                             *  allow it to proceed since we already have write access anyway
                             */
                            if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                            {

                            }
                            else
                            {
   
                            }
                            do
                            {
                                lockResult = MutexLock( rwlock->writerlock, GUCEF_MT_SHORT_LOCK_TIMEOUT );
                                switch ( lockResult )
                                {
	                                case GUCEF_MUTEX_OPERATION_SUCCESS :
                                    case GUCEF_MUTEX_ABANDONED :
                                    {
                                        rwlock->activeWriterReentrancyCount = MutexReentrancy( rwlock->writerlock );
                                        MutexUnlock( rwlock->datalock );
                                        return GUCEF_RWLOCK_OPERATION_SUCCESS;
                                    }
                                    case GUCEF_MUTEX_WAIT_TIMEOUT :
                                    {
                                        /*
                                         *  If we are correct about reentrancy there should not have been any lock contention
                                         *  as such logically we should never get here barring some bug/issue since we already have the lock supposedly
                                         */ 
                                        GUCEF_ASSERT_ALWAYS;
                                        --rwlock->activeWriterReentrancyCount;
                                        MutexUnlock( rwlock->datalock );
                                        return GUCEF_RWLOCK_WAIT_TIMEOUT;    
                                    }
                                    default:
                                    case GUCEF_MUTEX_OPERATION_FAILED:
                                    {
                                        break;
                                    }
                                }
                            }
                            while ( 0 == rwlock->delflag );

                            /*
                             *  We should not get here if you cleaned up your threads before cleaning up the read write lock itself
                             */
                            MutexUnlock( rwlock->datalock );
                            return GUCEF_RWLOCK_OPERATION_FAILED;
                        }                    
                    }
                    else
                    {
                        /*
                         *  Check for a cheaper early out 
                         */
                        if ( 0 == rwlock->activeReaderCount            && 
                             0 == rwlock->queuedReaderCount            && 
                             0 == rwlock->activeWriterCount            && 
                             0 == rwlock->writeLockAquisitionInProgress )
                        {
                            /*
                             *  There are no active writers or readers so we can make this quick
                             */
                            do
                            {
                                lockResult = MutexLock( rwlock->writerlock, lockWaitTimeoutInMs );
                                switch ( lockResult )
                                {
	                                case GUCEF_MUTEX_OPERATION_SUCCESS :
                                    case GUCEF_MUTEX_ABANDONED :
                                    {
                                        rwlock->activeWriterCount++;        
                                        if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                        {

                                        }
                                        else
                                        {
   
                                        }
                                        rwlock->lastWriterThreadId = GetCurrentTaskID();
                                        rwlock->activeWriterReentrancyCount = MutexReentrancy( rwlock->writerlock );
                                        MutexUnlock( rwlock->datalock );
                                        return GUCEF_RWLOCK_OPERATION_SUCCESS; 
                                    }
                                    case GUCEF_MUTEX_WAIT_TIMEOUT :
                                    {
                                        /*
                                         *  If we are correct about reentrancy there should not have been any lock contention
                                         *  as such logically we should never get here barring some bug/issue since we already have the lock supposedly
                                         */ 
                                        GUCEF_ASSERT_ALWAYS;
                                        --rwlock->activeWriterReentrancyCount;
                                        MutexUnlock( rwlock->datalock );
                                        return GUCEF_RWLOCK_WAIT_TIMEOUT;    
                                    }
                                    default:
                                    case GUCEF_MUTEX_OPERATION_FAILED:
                                    {
                                        break;
                                    }
                                }
                            }
                            while ( 0 == rwlock->delflag );

                            /*
                             *  We should not get here if you cleaned up your threads before cleaning up the read write lock itself
                             */
                            MutexUnlock( rwlock->datalock );
                            return GUCEF_RWLOCK_OPERATION_FAILED;
                        }
                    }

                    if ( 0 != rwlock->wpriority )
                    {
                        MutexUnlock( rwlock->datalock );
                    
                        /*
                         *  Wait for the right moment with high likelyhood of getting the lock
                         *  We dont care about queued readers since writers take priority
                         *  The queued writer counter/flag will prevent new readers from becoming active
                         */
                        do
                        {                                                     
                            if ( ( rwlock->queuedWriterCount > 0                          && 
                                   rwlock->activeReaderCount > 0                          &&
                                   rwlock->activeReaderCount <= rwlock->queuedWriterCount &&     /* first 5 checks are for the special secenario where all active readers are trying to get a write lock */
                                   0 == rwlock->activeWriterCount                         &&
                                   0 == rwlock->writeLockAquisitionInProgress              ) ||
                                 ( 1 >= rwlock->activeReaderCount             &&                 /* last 2 are for the more commen case where the reader is trying to escalate to a write lock implicitly */
                                   0 == rwlock->writeLockAquisitionInProgress )              )
                            {
                                /*
                                 *  Reverify with a data lock
                                 *  We have to hurry and a new reader may slip in
                                 */
                                UInt8 reverifyFailed = 0;
                                do
                                {
                                    lockResult = MutexLock( rwlock->datalock, GUCEF_MT_SHORT_LOCK_TIMEOUT );
                                    switch ( lockResult )
                                    {
	                                    case GUCEF_MUTEX_OPERATION_SUCCESS :
                                        case GUCEF_MUTEX_ABANDONED :
                                        {
                                            if ( 0 == rwlock->writeLockAquisitionInProgress )
                                            {
                                                UInt32 hasOtherReaderThreadsActive = 0;
                                                char specialAccessGranted = 0;
                                                if ( 1 == rwlock->activeReaderCount )
                                                {
                                                    /*
                                                     *  The reader count might be caused by the same calling thread
                                                     *  Only dig through the reader administration in this case as a minor
                                                     *  performance optimization
                                                     */
                                                    hasOtherReaderThreadsActive = rwl_impl_has_foreign_reader( rwlock );
                                                }
                                                else
                                                {
                                                    hasOtherReaderThreadsActive = 0 != rwlock->activeReaderCount;
                                                    
                                                    /* 
                                                     *   Check for possibility of the special secenario where all active readers are trying to 
                                                     *   get a write lock and they end up waiting on eachother to stop reading 
                                                     */
                                                    if ( rwlock->queuedWriterCount > 0                          && 
                                                         rwlock->activeReaderCount > 0                          &&
                                                         0 == rwlock->activeWriterCount                         &&
                                                         rwlock->activeReaderCount <= rwlock->queuedWriterCount  )    
                                                    {
                                                        /* its possible: do a more comprehensive check */
                                                        specialAccessGranted = rwl_impl_has_special_access_for_multiple_readers_escalating_to_writers( rwlock );
                                                    }
                                                }

                                                if ( 0 == hasOtherReaderThreadsActive || 0 != specialAccessGranted )
                                                {
                                                    /*
                                                     *  We found a moment without any active readers at all, excluding this thread
                                                     *  or special access was granted
                                                     *  Now we just have to wait for other writers if any
                                                     *
                                                     *  The 'queuedWriterCount' already blocks new active readers but for consistency we also
                                                     *  set the writeLockAquisitionInProgress flag 
                                                     */
                                                    rwlock->writeLockAquisitionInProgress = 1;
                                                    rwlock->writeLockAquisitionThreadId = GetCurrentTaskID(); 
                                                    MutexUnlock( rwlock->datalock );
                    
                                                    do
                                                    {
                                                        /*
                                                         *  This is the main call in this function, what its all about
                                                         *  Obtaining the write lock
                                                         *
                                                         */
                                                        lockResult = MutexLock( rwlock->writerlock, lockWaitTimeoutInMs );
                                                        switch ( lockResult )
                                                        {
	                                                        case GUCEF_MUTEX_OPERATION_SUCCESS :
                                                            case GUCEF_MUTEX_ABANDONED :
                                                            {
                                                                /*
                                                                 *  Now that we have the write lock we need to still go back and update the administration
                                                                 *  our queued writer is now an active writer
                                                                 */
                                                                UInt32 dataLockResult = 0;
                                                                do
                                                                {
                                                                    dataLockResult = MutexLock( rwlock->datalock, lockWaitTimeoutInMs );
                                                                    if ( dataLockResult == GUCEF_MUTEX_OPERATION_SUCCESS )
                                                                    {                            
                                                                        /*
                                                                         *  Perform one last check for the delete flag
                                                                         *  We dont want to wait for all the queued writers to slowly process which could be a lot of them
                                                                         */
                                                                        if ( 0 != rwlock->delflag )
                                                                        {
                                                                            /*
                                                                             *  Destruction started while we were waiting to get the lock
                                                                             *  Undo grabbing the writer lock
                                                                             */
                                                                            MutexUnlock( rwlock->writerlock );
                                                                            rwlock->writeLockAquisitionInProgress = 0;
                                                                            if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                                            {

                                                                            }
                                                                            else
                                                                            {
   
                                                                            }
                                                                            MutexUnlock( rwlock->datalock );
                                                                            return GUCEF_RWLOCK_OPERATION_FAILED;                                                                                                
                                                                        }
                                                                        else
                                                                        {
                                                                            /*
                                                                             *  We have successfully turned our queued writer into the active writer
                                                                             */
                                                                            rwlock->activeWriterCount++;
                                                                            rwlock->lastWriterThreadId = GetCurrentTaskID();
                                                                            rwlock->writeLockAquisitionInProgress = 0;
                                                                            if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                                            {

                                                                            }
                                                                            else
                                                                            {
   
                                                                            }
                                                                            rwlock->activeWriterReentrancyCount = MutexReentrancy( rwlock->writerlock );
                                                                            MutexUnlock( rwlock->datalock );
                                                                            return GUCEF_RWLOCK_OPERATION_SUCCESS;                                    
                                                                        }
                                                                    }
                                                                }
                                                                while ( GUCEF_MUTEX_OPERATION_SUCCESS != dataLockResult );

                                                                /*
                                                                 *  We should never get here logically
                                                                 */
                                                                GUCEF_ASSERT_ALWAYS;
                                                                rwlock->writeLockAquisitionInProgress = 0;
                                                                MutexUnlock( rwlock->writerlock );
                                                                return GUCEF_RWLOCK_OPERATION_FAILED;
                                                            }
                                                            case GUCEF_MUTEX_WAIT_TIMEOUT:
                                                            {
                                                                UInt32 dataLockResult = 0;
                                                                do
                                                                {
                                                                    dataLockResult = MutexLock( rwlock->datalock, lockWaitTimeoutInMs );
                                                                    if ( dataLockResult == GUCEF_MUTEX_OPERATION_SUCCESS )
                                                                    {                            
                                                                        /*
                                                                         *  Update the administration to undo our registration as a queued writer 
                                                                         *  before exiting and reporting a wait timeout
                                                                         */
                                                                        rwlock->writeLockAquisitionInProgress = 0;
                                                                        if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                                        {

                                                                        }
                                                                        else
                                                                        {
   
                                                                        }
                                                                        MutexUnlock( rwlock->datalock );
                                                                        return GUCEF_RWLOCK_WAIT_TIMEOUT;                                                                                                
                                                                    }
                                                                }
                                                                while ( GUCEF_MUTEX_OPERATION_SUCCESS != dataLockResult );

                                                                /*
                                                                 *  We should never get here logically
                                                                 */
                                                                GUCEF_ASSERT_ALWAYS;
                                                                rwlock->writeLockAquisitionInProgress = 0;
                                                                if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                                {

                                                                }
                                                                else
                                                                {
   
                                                                }
                                                                MutexUnlock( rwlock->writerlock );
                                                                return GUCEF_RWLOCK_OPERATION_FAILED;
                                                            }
                                                            default:
                                                            case GUCEF_MUTEX_OPERATION_FAILED:
                                                            {
                                                                UInt32 dataLockResult = 0;
                                                                do
                                                                {
                                                                    dataLockResult = MutexLock( rwlock->datalock, lockWaitTimeoutInMs );
                                                                    if ( dataLockResult == GUCEF_MUTEX_OPERATION_SUCCESS )
                                                                    {                            
                                                                        /*
                                                                         *  Update the administration to undo our registration as a queued writer 
                                                                         *  before exiting and reporting a wait timeout
                                                                         */
                                                                        rwlock->writeLockAquisitionInProgress = 0;
                                                                        if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                                        {

                                                                        }
                                                                        else
                                                                        {
   
                                                                        }
                                                                        MutexUnlock( rwlock->datalock );
                                                                        return GUCEF_RWLOCK_OPERATION_FAILED;                                                                                                
                                                                    }
                                                                }
                                                                while ( GUCEF_MUTEX_OPERATION_SUCCESS != dataLockResult );

                                                                /*
                                                                 *  We should never get here logically
                                                                 */
                                                                GUCEF_ASSERT_ALWAYS;
                                                                rwlock->writeLockAquisitionInProgress = 0;
                                                                if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                                {

                                                                }
                                                                else
                                                                {
   
                                                                }
                                                                return GUCEF_RWLOCK_OPERATION_FAILED;
                                                            }
                                                        }
                                                    }
                                                    while ( 0 == rwlock->delflag );
                                                    if ( 0 != rwlock->delflag )
                                                    {
                                                        do
                                                        {
                                                            /*
                                                             *  Lock destruction started while we were waiting to grab the lock
                                                             *  Unregister as queued and give up
                                                             */
                                                            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                                            {                                                        
                                                                if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                                {

                                                                }
                                                                else
                                                                {
   
                                                                }
                                                                rwlock->writeLockAquisitionInProgress = 0;
                                                                MutexUnlock( rwlock->datalock );
                                                                break;
                                                            }
                                                        }
                                                        while ( 1 > 0 );
                                                    }
                                                    return GUCEF_RWLOCK_OPERATION_FAILED;
                                                }
                                                else
                                                {
                                                    reverifyFailed = 1;
                                                    MutexUnlock( rwlock->datalock );
                                                }
                                            }
                                            else
                                            {
                                                reverifyFailed = 1;
                                                MutexUnlock( rwlock->datalock );
                                            }
                                            break;
                                        }
                                        default:
                                        case GUCEF_MUTEX_WAIT_TIMEOUT:
                                        case GUCEF_MUTEX_OPERATION_FAILED:
                                        {
                                            /*
                                             *  Could not get data lock to verify, loop again  
                                             */
                                            break;
                                        }
                                    }
                                }
                                while ( 0 == rwlock->delflag && 0 == reverifyFailed );
                            }
                            else
                            {
                                PrecisionDelay( 25 );
                            }
                        }
                        while ( 0 == rwlock->delflag );
                        if ( 0 != rwlock->delflag )
                        {
                            do
                            {
                                /*
                                 *  Lock destruction started while we were waiting to grab the lock
                                 *  Unregister as queued and give up
                                 */
                                if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                {                                                        
                                    if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                    {

                                    }
                                    else
                                    {
   
                                    }
                                    MutexUnlock( rwlock->datalock );
                                    break;
                                }                                                                 
                            }
                            while ( 1 > 0 );
                        }
                        return GUCEF_RWLOCK_OPERATION_FAILED;

                    }
                    else
                    {
                        /*
                         *  First wait for all the active and queued readers to finish
                         *  We need to grab the moment that happens via constant monitoring
                         */
                        MutexUnlock( rwlock->datalock );
                    
                        do
                        {
                            if ( 0 == rwlock->activeReaderCount            && 
                                 0 == rwlock->queuedReaderCount            &&
                                 0 == rwlock->writeLockAquisitionInProgress )
                            {
                                /*
                                 *  Reverify with a data lock
                                 *  We have to hurry and a new reader may slip in
                                 */
                                UInt8 reverifyFailed = 0;
                                do
                                {
                                    if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                    {
                                        if ( 0 == rwlock->activeReaderCount            && 
                                             0 == rwlock->queuedReaderCount            &&
                                             0 == rwlock->writeLockAquisitionInProgress )
                                        {
                                            /*
                                             *  We found a moment without any readers at all
                                             *  Now we just have to wait for other writers if any
                                             *
                                             *  Here we cheat a little to have a chance:
                                             *  The writeLockAquisitionInProgress flag blocks new active readers
                                             *  It temporarly overrules the reader priority while we grab the other lock
                                             */
                                            rwlock->writeLockAquisitionInProgress = 1;
                                            rwlock->writeLockAquisitionThreadId = GetCurrentTaskID();
                                            MutexUnlock( rwlock->datalock );
                    
                                            do
                                            {
                                                if ( MutexLock( rwlock->writerlock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                                {
                                                    /*
                                                     *  Now that we have the write lock we need to still go back and update the administration.
                                                     *  Our queued writer is now an active writer
                                                     */
                                                    do
                                                    {
                                                        if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                                        {
                                                            /*
                                                             *  Perform one last check for the delete flag
                                                             *  We dont want to wait for all the queued writers to slowly process which could be a lot of them
                                                             */
                                                            if ( 0 != rwlock->delflag )
                                                            {
                                                                /*
                                                                 *  Destruction started while we were waiting to get the writer lock
                                                                 *  Undo grabbing the writer lock
                                                                 */
                                                                MutexUnlock( rwlock->writerlock );
                                                                rwlock->writeLockAquisitionInProgress = 0;
                                                                if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                                {

                                                                }
                                                                else
                                                                {
   
                                                                }
                                                                MutexUnlock( rwlock->datalock );
                                                                return GUCEF_RWLOCK_OPERATION_FAILED;                                                                                                
                                                            }
                                                            else
                                                            {
                                                                /*
                                                                 *  We have successfully turned our queued writer into the active writer
                                                                 */
                                                                rwlock->activeWriterCount++;
                                                                rwlock->lastWriterThreadId = GetCurrentTaskID();
                                                                rwlock->writeLockAquisitionInProgress = 0;
                                                                if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                                {

                                                                }
                                                                else
                                                                {
   
                                                                }
                                                                MutexUnlock( rwlock->datalock );
                                                                return GUCEF_RWLOCK_OPERATION_SUCCESS;                                    
                                                            }
                                                        }
                                                    }
                                                    while ( 0 == rwlock->delflag );

                                                    /*
                                                     *  We should never get here logically
                                                     */
                                                    GUCEF_ASSERT_ALWAYS;
                                                    rwlock->writeLockAquisitionInProgress = 0;
                                                    MutexUnlock( rwlock->writerlock );
                                                    return GUCEF_RWLOCK_OPERATION_FAILED;
                                                }
                                            }
                                            while ( 0 == rwlock->delflag );
                                            if ( 0 != rwlock->delflag )
                                            {
                                                do
                                                {
                                                    /*
                                                     *  Lock destruction started while we were waiting to grab the lock
                                                     *  Unregister as queued and give up
                                                     */
                                                    if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                                    {                                                        
                                                        if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                                        {

                                                        }
                                                        else
                                                        {
   
                                                        }
                                                        rwlock->writeLockAquisitionInProgress = 0;
                                                        MutexUnlock( rwlock->datalock );
                                                        break;
                                                    }
                                                }
                                                while ( 1 > 0 );
                                                return GUCEF_RWLOCK_OPERATION_FAILED;
                                            }
                                        }
                                        else
                                        {
                                            reverifyFailed = 1;
                                            MutexUnlock( rwlock->datalock );
                                        }
                                    }
                                }
                                while ( 0 == rwlock->delflag && 0 == reverifyFailed );
                            }
                            else
                            {
                                PrecisionDelay( 25 );
                            }
                        }
                        while ( 0 == rwlock->delflag );
                        if ( 0 != rwlock->delflag )
                        {
                            do
                            {
                                /*
                                 *  Lock destruction started while we were waiting to grab the lock
                                 *  Unregister as queued and give up
                                 */
                                if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
                                {                                                        
                                    if ( rwl_impl_pop_queued_writer( rwlock ) != 0 ) 
                                    {

                                    }
                                    else
                                    {
   
                                    }
                                    MutexUnlock( rwlock->datalock );
                                    break;
                                }
                            }
                            while ( 1 > 0 );
                        }
                        return GUCEF_RWLOCK_OPERATION_FAILED;
                    }
                    break;
                }
                case GUCEF_MUTEX_WAIT_TIMEOUT :
                {
                    return GUCEF_RWLOCK_WAIT_TIMEOUT;    
                }
                default:
                case GUCEF_MUTEX_OPERATION_FAILED:
                {
                    break;
                }
            }
        }
        while ( 0 == rwlock->delflag );
    }
    return GUCEF_RWLOCK_OPERATION_FAILED;
}

/*--------------------------------------------------------------------------*/

UInt32
rwl_writer_stop( TRWLock *rwlock )
{
    GUCEF_BEGIN;
	if ( GUCEF_NULL != rwlock )
    {
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
                if ( rwlock->activeWriterCount > 0                    &&
                     rwlock->lastWriterThreadId == GetCurrentTaskID() )
                {
                    /*
                     *  The caller is the currently active writer thread
                     *  This should always be the case
                     *  allow it to proceed since we already have write access anyway
                     */
                    if ( rwlock->activeWriterReentrancyCount > 0 )
                    {
                        --rwlock->activeWriterReentrancyCount;
                    }
                    else
                    {
                        assert( rwlock->activeWriterCount == 1 );
                        --rwlock->activeWriterCount;
                    }

                    MutexUnlock( rwlock->writerlock );
                }               
                else
                {
                    assert( rwlock->activeWriterCount > 0 );
                    GUCEF_UNREACHABLE;
                }
                                
                MutexUnlock( rwlock->datalock );
                GUCEF_END;
                return GUCEF_RWLOCK_OPERATION_SUCCESS;
            }
        }
        while ( 0 == rwlock->delflag );
    }    
    GUCEF_END;
    return GUCEF_RWLOCK_OPERATION_FAILED;
}

/*--------------------------------------------------------------------------*/

UInt32
rwl_writer_transition_to_reader( TRWLock* rwlock, UInt32 lockWaitTimeoutInMs )
{
	GUCEF_BEGIN;
	if ( GUCEF_NULL != rwlock )
    {
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
	            UInt32 callerThreadId = GetCurrentTaskID();
                
                /*
                 *	Sanity check
                 *  If we claim to be transitioning a writer that means there should be a currently active writer
                 *  a writer that is the calling thread.
                 */
                if ( 1 != rwlock->activeWriterCount               ||
                     1 < rwlock->activeReaderCount                ||
                     rwlock->lastWriterThreadId != callerThreadId ||
                     0 != rwl_impl_has_foreign_reader( rwlock )    )
                {
                    /*
                     *  You should not get here
                     *  We have not met the prerequisite conditions to transition a writer
                     *      - There must be no other active reader because presumably you already have an active writer
                     *      - There must be an active writer, the caller, because how else can you transition it
                     */
                    GUCEF_ASSERT_ALWAYS;
                    MutexUnlock( rwlock->datalock );
                    GUCEF_END;
                    return GUCEF_RWLOCK_OPERATION_FAILED;
                }                
                
                /*
                 *  The caller is the currently active writer thread
                 *  This should always be the case
                 *  allow it to proceed since we already have write access anyway
                 */
                assert( rwlock->activeWriterCount == 1 );                    
                if ( rwlock->activeWriterReentrancyCount > 0 )
                {
                    /*
                     *  Since the current write lock has a reentrancy the way we transition to a reader 
                     *  is by reducing a level of reentrancy and replacing it with a read level.
                     *  Note that this does not release the write lock, it merely alters our internal accounting
                     *  at this stage.
                     */
                    if ( rwl_impl_push_active_reader( rwlock ) != 0 )
                    {
                        rwlock->activeWriterReentrancyCount = MutexReentrancy( rwlock->writerlock );
                        MutexUnlock( rwlock->writerlock );
                        --rwlock->activeWriterReentrancyCount;
                        MutexUnlock( rwlock->datalock );
                        GUCEF_END;
                        return GUCEF_RWLOCK_OPERATION_SUCCESS;
                    }
                    else
                    {
                        MutexUnlock( rwlock->datalock );
                        GUCEF_END;
                        return GUCEF_RWLOCK_OPERATION_FAILED;
                    }                       
                }

                /*
                 *  If we get here then the write lock we'd be releasing is the last one held by this thread
                 *  As such we need to release the write lock and from that point forward this thread will be purely a reader
                 */
                if ( rwl_impl_push_active_reader( rwlock ) != 0 )
                {
                    --rwlock->activeWriterCount;
                    MutexUnlock( rwlock->writerlock );
                    MutexUnlock( rwlock->datalock );
                    GUCEF_END;
                    return GUCEF_RWLOCK_OPERATION_SUCCESS;
                }
                else
                {
                    MutexUnlock( rwlock->datalock );
                    GUCEF_END;
                    return GUCEF_RWLOCK_OPERATION_FAILED;
                }
            }
        }
        while ( 0 == rwlock->delflag );
    }
    GUCEF_END;
    return GUCEF_RWLOCK_OPERATION_FAILED;
}

/*--------------------------------------------------------------------------*/

void
rwl_signal_thread_killed( TRWLock *rwlock, UInt32 killedThreadId )
{
    if ( GUCEF_NULL == rwlock )
        return;

	GUCEF_BEGIN;
	if ( GUCEF_NULL != rwlock )
    {
        do
        {
            if ( MutexLock( rwlock->datalock, GUCEF_MUTEX_INFINITE_TIMEOUT ) == GUCEF_MUTEX_OPERATION_SUCCESS )
            {
                /*
                 *	Check if the killed thread was our active writer
                 */
                if ( rwlock->activeWriterCount > 0                ||
                     rwlock->lastWriterThreadId == killedThreadId  )
                {
                    /*
                     *  The killed thread was the active writer.
                     *  As such our writer lock is now abandoned and our administration incorrect.
                     *  Let's fix it.
                     */
                    rwlock->activeWriterCount = 0;
                    rwlock->activeWriterReentrancyCount = 0;
                    MutexUnlock( rwlock->writerlock );
                }

                /*
                 *	Check if the killed thread was an active reader
                 */
                if ( rwlock->activeReaderCount > 0 )
                {
                    TThreadSlot* prevSlot = GUCEF_NULL;
                    TThreadSlot* readerSlot = rwlock->activeReaders;
                    while ( GUCEF_NULL != readerSlot )
                    {
                        if ( readerSlot->threadId == killedThreadId )
                        {
                            readerSlot->reentrancyCount = 0;
                            readerSlot->threadId = 0;
                            if ( GUCEF_NULL != prevSlot )
                                prevSlot->nextSlot = readerSlot->nextSlot;    
                            else
                                rwlock->activeReaders = readerSlot->nextSlot;
                            readerSlot->nextSlot = rwlock->freeThreadInfo;
                            rwlock->freeThreadInfo = readerSlot;
                            break;
                        }
                        else
                        {
                            prevSlot = readerSlot;
                            readerSlot = readerSlot->nextSlot;
                        }
                    }
                }

                /*
                 *	Check if the killed thread was a queued writer
                 */
                if ( rwlock->queuedWriterCount > 0 )
                {
                    TThreadSlot* prevSlot = GUCEF_NULL;
                    TThreadSlot* queuedWriterSlot = rwlock->queuedWriters;
                    while ( GUCEF_NULL != queuedWriterSlot )
                    {
                        if ( queuedWriterSlot->threadId == killedThreadId )
                        {
                            queuedWriterSlot->reentrancyCount = 0;
                            queuedWriterSlot->threadId = 0;
                            if ( GUCEF_NULL != prevSlot )
                                prevSlot->nextSlot = queuedWriterSlot->nextSlot;    
                            else
                                rwlock->activeReaders = queuedWriterSlot->nextSlot;
                            queuedWriterSlot->nextSlot = rwlock->freeThreadInfo;
                            rwlock->freeThreadInfo = queuedWriterSlot;
                            break;
                        }
                        else
                        {
                            prevSlot = queuedWriterSlot;
                            queuedWriterSlot = queuedWriterSlot->nextSlot;
                        }
                    }
                }
                
                MutexUnlock( rwlock->datalock );
                GUCEF_END;
                return;
            }
        }
        while ( 0 == rwlock->delflag );
    }
    GUCEF_END;
}

/*--------------------------------------------------------------------------*/


