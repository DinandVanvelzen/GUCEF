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

#ifndef GUCEF_MT_CTMAILBOX_H
#define GUCEF_MT_CTMAILBOX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <deque>
#include <set>
#include <vector>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CICLONEABLE_H
#include "gucefMT_CICloneable.h"
#define GUCEF_MT_CICLONEABLE_H
#endif /* GUCEF_MT_CICLONEABLE_H ? */

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#include "gucefMT_CObjectScopeReadOnlyLock.h"
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */

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
namespace MT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Basicly a tread-safe stack for passing data event data between
 *  multiple threads
 */
template < typename T >
class CTMailBox : public virtual MT::CILockable
{
    public:

    class Mail
    {
        public:

        T eventid;
        CICloneable* data;

        Mail( void )
            : eventid()
            , data( GUCEF_NULL )
        {GUCEF_TRACE;}
    };

    typedef T                                                   value_type;
    typedef GUCEF::set< T >                                     TMailIdSet;
    typedef GUCEF::vector< Mail >                               TMailVector;
    typedef std::deque< Mail, gucef_allocator< Mail > >         TMailQueue;
    typedef typename std::deque< Mail >::iterator               iterator;
    typedef typename std::deque< Mail >::const_iterator         const_iterator;

    CTMailBox( void );

    virtual ~CTMailBox();

    /**
     *  Adds the given eventid and data to the FIFO stack.
     *  if data is non-NULL then it will be cloned.
     *
     *  @param eventid the ID of the event you wish to add to the mailbox
     *  @param data cloneable data container for optional event data.
     */
    bool AddMail( const T& eventid                     ,
                  const CICloneable* data = GUCEF_NULL );

    /**
     *  Attempts to retrieve mail from the mailbox.
     *
     *  Note that if data is non-NULL then you should delete the
     *  object after handling the mail message or you will create
     *  a memory-leak.
     *
     *  @param eventid the ID of the event
     *  @param data cloneable data container for optional event data.
     *  @return whether mail was successfully retrieved from the mailbox.
     */
    bool GetMail( T& eventid                      ,
                  CICloneable** data = GUCEF_NULL );

    /**
     *  Attempts to retrieve mail from the mailbox for inspection without removing it from the mail stack
     *  You can use this in combination with PopMail() for operations which may fail at which point
     *  the mail needs to be retained within the mailbox in order to try again on a later time
     *
     *  @param lock mandatory lock param to make sure the access pattern includes thread safe access to data
     *  @param eventid the ID of the event
     *  @param data cloneable data container for optional event data.
     *  @return whether mail was successfully retrieved from the mailbox.
     */
    bool PeekMail( CScopeMutex& lock  ,
                   T& eventid         ,
                   CICloneable** data );

    /**
     *  Removes the first mail item in the mail stack
     *  Intended to be used in combination with PeekMail()
     */
    bool PopMail( CScopeMutex& lock );

    /**
     *  Attempts to retrieve mail from the mailbox.
     *
     *  Note that if data given as part of a mail entry is non-NULL
     *  then you should delete the object after handling the mail
     *  message or you will create a memory-leak.
     *
     *  @param mailList storage where mail items will be placed
     *  @param maxMailItems if greater then -1 will place a maximum on
     *  @param maxMailItems the number of mail items that can be placed in the mail list
     *  @return whether mail was successfully retrieved from the mailbox.
     */
    bool GetMailList( TMailVector& mailList   ,
                      Int32 maxMailItems = -1 );

    bool Clear( void );

    bool ClearAllExcept( const T& eventid );

    bool Delete( const T& eventid );

    bool Delete( const TMailIdSet& ids );

    bool HasMail( void ) const;

    UInt32 AmountOfMail( void ) const;

    void SetAcceptsNewMail( bool acceptNewMail );

    /**
     *  Utility function to help a given caller thread to wait until all mail in the mailbox
     *  has been consumed and the mailbox is empty or the timeout is reached, whichever comes first
     */
    TLockStatus WaitForZeroMailInMailbox( Int32 waitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS );

    const CMutex& GetLock( void ) const;

    typename TMailQueue::iterator begin( CScopeMutex& lock );

    typename TMailQueue::iterator end( void );

    typename TMailQueue::iterator erase( CScopeMutex& lock, typename TMailQueue::iterator& index );

    typename TMailQueue::const_iterator begin( CScopeMutex& lock ) const;

    typename TMailQueue::const_iterator end( void ) const;

    virtual const CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TLockStatus ReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TLockStatus ReadOnlyUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTMailBox( const CTMailBox& src ) {}                              /**< not supported */
    CTMailBox& operator=( const CTMailBox& src ) { return *this; }    /**< not supported */

    private:

    TMailQueue m_mailQueue;
    bool m_acceptsNewMail;
    CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T >
CTMailBox< T >::CTMailBox( void )
    : MT::CILockable()
    , m_mailQueue()
    , m_acceptsNewMail( true )
    , m_lock()
{GUCEF_TRACE;

}

/*--------------------------------------------------------------------------*/

template< typename T >
CTMailBox< T >::~CTMailBox()
{GUCEF_TRACE;

    CScopeMutex lock( m_lock );
    m_acceptsNewMail = false;
    Clear();
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CTMailBox< T >::AddMail( const T& eventid                     ,
                         const CICloneable* data /* = NULL */ )
{GUCEF_TRACE;

    CScopeMutex lock( m_lock );

    if ( m_acceptsNewMail )
    {
        Mail entry;
        entry.eventid = eventid;
        if ( GUCEF_NULL != data )
        {
            entry.data = data->Clone();
        }
        else
        {
            entry.data = GUCEF_NULL;
        }
        m_mailQueue.push_back( entry );
        
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

template< typename T >
void 
CTMailBox< T >::SetAcceptsNewMail( bool acceptNewMail )
{GUCEF_TRACE;

    CScopeMutex lock( m_lock );
    m_acceptsNewMail = acceptNewMail;
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CTMailBox< T >::GetMail( T& eventid         ,
                         CICloneable** data )
{GUCEF_TRACE;

    CScopeMutex lock( m_lock );

    if ( !m_mailQueue.empty() )
    {
        Mail& entry = m_mailQueue.front();
        eventid = entry.eventid;

        if ( GUCEF_NULL != data )
            *data = entry.data;
        else
            GUCEF_DELETE entry.data;

        m_mailQueue.pop_front();
        return true;
    }

    if ( GUCEF_NULL != data )
        *data = GUCEF_NULL;
    return false;
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CTMailBox< T >::PeekMail( CScopeMutex& lock  ,
                          T& eventid         ,
                          CICloneable** data )
{GUCEF_TRACE;

    GUCEF_ASSERT( GUCEF_NULL != &lock );
    GUCEF_ASSERT( &m_lock == lock.GetMutex() );
    GUCEF_ASSERT( lock.IsLocked() );
    
    if ( !m_mailQueue.empty() )
    {
        Mail& entry = m_mailQueue.front();
        eventid = entry.eventid;
        if ( GUCEF_NULL != data )
            *data = entry.data;

        return true;
    }
    if ( GUCEF_NULL != data )
        *data = GUCEF_NULL;
    return false;
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CTMailBox< T >::PopMail( CScopeMutex& lock )
{GUCEF_TRACE;

    GUCEF_ASSERT( GUCEF_NULL != &lock );
    GUCEF_ASSERT( &m_lock == lock.GetMutex() );
    GUCEF_ASSERT( lock.IsLocked() );

    if ( !m_mailQueue.empty() )
    {
        Mail& entry = m_mailQueue.front();
        GUCEF_DELETE entry.data;
        m_mailQueue.pop_front();
        return true;
    }
    return false;
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CTMailBox< T >::GetMailList( TMailVector& mailList ,
                             Int32 maxMailItems    )
{GUCEF_TRACE;

    CScopeMutex lock( m_lock );

    Int32 mailItemsRead = 0;
    while ( mailItemsRead < maxMailItems || maxMailItems < 0 )
    {
        if ( !m_mailQueue.empty() )
        {
            mailList.push_back( m_mailQueue.front() );
            m_mailQueue.pop_front();

            ++mailItemsRead;
        }
        else
        {
            break;
        }
    }
    return mailItemsRead > 0;
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CTMailBox< T >::Clear( void )
{GUCEF_TRACE;

    CScopeMutex lock( m_lock );
    
    typename TMailQueue::iterator i( m_mailQueue.begin() );
    while ( i != m_mailQueue.end() )
    {
        GUCEF_DELETE (*i).data;
        ++i;
    }
    m_mailQueue.clear();
    return true;
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CTMailBox< T >::ClearAllExcept( const T& eventid )
{GUCEF_TRACE;

    CScopeMutex lock( m_lock );
    
    #if __cplusplus >= 201103L

    // C++11 added the erase()
    typename TMailQueue::iterator i( m_mailQueue.begin() );
    while ( i != m_mailQueue.end() )
    {
        if ( (*i).eventid != eventid )
        {
            GUCEF_DELETE (*i).data;
            i = m_mailQueue.erase( i );
            continue;
        }
        ++i;
    }

    #else

    TMailQueue copyQueue;
    while ( !m_mailQueue.empty() )
    {
        if ( m_mailQueue.front().eventid == eventid )
            copyQueue.push_back( m_mailQueue.front() );
        else
            GUCEF_DELETE m_mailQueue.front().data;
        m_mailQueue.pop_front();
    }
    m_mailQueue = copyQueue;

    #endif
        
    return true;
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CTMailBox< T >::Delete( const T& eventid )
{GUCEF_TRACE;

    CScopeMutex lock( m_lock );

    #if __cplusplus >= 201103L

    // C++11 added the erase()
    typename TMailQueue::iterator i( m_mailQueue.begin() );
    while ( i != m_mailQueue.end() )
    {
        if ( (*i).eventid == eventid )
        {
            GUCEF_DELETE (*i).data;
            i = m_mailQueue.erase( i );
            continue;
        }
        ++i;
    }

    #else

    TMailQueue copyQueue;
    while ( !m_mailQueue.empty() )
    {
        if ( m_mailQueue.front().eventid != eventid )
            copyQueue.push_back( m_mailQueue.front() );
        else
            GUCEF_DELETE m_mailQueue.front().data;
        m_mailQueue.pop_front();
    }
    m_mailQueue = copyQueue;

    #endif
    
    return true;
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CTMailBox< T >::Delete( const TMailIdSet& ids )
{GUCEF_TRACE;

    CScopeMutex lock( m_lock );

    #if __cplusplus >= 201103L

    // C++11 added the erase()
    typename TMailQueue::iterator i( m_mailQueue.begin() );
    while ( i != m_mailQueue.end() )
    {
        typename TMailIdSet::const_iterator idIt = ids.find( (*i).eventid );
        if ( idIt != ids.end() )
        {
            GUCEF_DELETE (*i).data;
            i = m_mailQueue.erase( i );
            continue;
        }
        ++i;
    }

    #else

    TMailQueue copyQueue;
    while ( !m_mailQueue.empty() )
    {
        typename TMailIdSet::const_iterator idIt = ids.find( m_mailQueue.front().eventid );
        if ( idIt != ids.end() )
            copyQueue.push_back( m_mailQueue.front() );
        else
            GUCEF_DELETE m_mailQueue.front().data;
        m_mailQueue.pop_front();
    }
    m_mailQueue = copyQueue;

    #endif
    
    return true;

}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CTMailBox< T >::HasMail( void ) const
{GUCEF_TRACE;

    CScopeMutex lock( m_lock );
    return m_mailQueue.size() > 0;
}

/*--------------------------------------------------------------------------*/

template< typename T >
UInt32
CTMailBox< T >::AmountOfMail( void ) const
{GUCEF_TRACE;

    CScopeMutex lock( m_lock );
    return (UInt32) m_mailQueue.size();
}

/*--------------------------------------------------------------------------*/

template< typename T >
TLockStatus
CTMailBox< T >::WaitForZeroMailInMailbox( Int32 waitTimeoutInMs )
{GUCEF_TRACE;

    bool hasMail = false;
    UInt64 tickCount = PrecisionTickCount();
    do
    {
        CScopeMutex lock( m_lock );
        hasMail = !m_mailQueue.empty();
        lock.EarlyUnlock();

        if ( hasMail )
        {
            PrecisionDelay( 10 );
            UInt64 newTickCount = PrecisionTickCount();
            
            if ( waitTimeoutInMs > 0 && PrecisionTimerTicksToMs( newTickCount - tickCount ) >= waitTimeoutInMs )
                return TLockStatus::LOCKSTATUS_WAIT_TIMEOUT;
        }
    }
    while ( hasMail );

    return TLockStatus::LOCKSTATUS_OPERATION_SUCCESS;
}

/*-------------------------------------------------------------------------*/

template< typename T >
const MT::CILockable*
CTMailBox< T >::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*--------------------------------------------------------------------------*/

template< typename T >
TLockStatus
CTMailBox< T >::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*--------------------------------------------------------------------------*/

template< typename T >
TLockStatus
CTMailBox< T >::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*--------------------------------------------------------------------------*/

template< typename T >
TLockStatus
CTMailBox< T >::ReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*--------------------------------------------------------------------------*/

template< typename T >
TLockStatus
CTMailBox< T >::ReadOnlyUnlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*--------------------------------------------------------------------------*/

template< typename T >
const CMutex&
CTMailBox< T >::GetLock( void ) const
{GUCEF_TRACE;

    return m_lock;
}

/*--------------------------------------------------------------------------*/

template< typename T >
typename CTMailBox< T >::TMailQueue::iterator
CTMailBox< T >::erase( CScopeMutex& lock, typename TMailQueue::iterator& index )
{GUCEF_TRACE;

    GUCEF_ASSERT( GUCEF_NULL != &lock );
    GUCEF_ASSERT( &m_lock == lock.GetMutex() );
    GUCEF_ASSERT( lock.IsLocked() );

    return m_mailQueue.erase( index );
}

/*--------------------------------------------------------------------------*/

template< typename T >
typename CTMailBox< T >::TMailQueue::iterator
CTMailBox< T >::begin( CScopeMutex& lock )
{GUCEF_TRACE;

    GUCEF_ASSERT( GUCEF_NULL != &lock );
    GUCEF_ASSERT( &m_lock == lock.GetMutex() );
    GUCEF_ASSERT( lock.IsLocked() );

    return m_mailQueue.begin();
}

/*--------------------------------------------------------------------------*/

template< typename T >
typename CTMailBox< T >::TMailQueue::iterator
CTMailBox< T >::end( void )
{GUCEF_TRACE;

    return m_mailQueue.end();
}

/*--------------------------------------------------------------------------*/

template< typename T >
typename CTMailBox< T >::TMailQueue::const_iterator
CTMailBox< T >::begin( CScopeMutex& lock ) const
{GUCEF_TRACE;

    GUCEF_ASSERT( GUCEF_NULL != &lock );
    GUCEF_ASSERT( &m_lock == lock.GetMutex() );
    GUCEF_ASSERT( lock.IsLocked() );

    return m_mailQueue.begin();
}

/*--------------------------------------------------------------------------*/

template< typename T >
typename CTMailBox< T >::TMailQueue::const_iterator
CTMailBox< T >::end( void ) const
{GUCEF_TRACE;

    return m_mailQueue.end();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_CTMAILBOX_H  ? */
