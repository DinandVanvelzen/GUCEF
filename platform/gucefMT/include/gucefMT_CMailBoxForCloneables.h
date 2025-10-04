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

#ifndef GUCEF_MT_CMAILBOXFORCLONEABLES_H
#define GUCEF_MT_CMAILBOXFORCLONEABLES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <deque>
#include <vector>

#ifndef GUCEF_SFINAE_H
#include "gucef_SFINAE.h"
#define GUCEF_SFINAE_H
#endif /* GUCEF_SFINAE_H ? */

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
 *  Basicly a tread-safe queue for passing data event data between
 *  multiple threads
 */
class GUCEF_MT_PUBLIC_CPP CMailboxForCloneables : public MT::CILockable
{
    public:

    typedef GUCEF::vector< CICloneable* >   TMailVector;
    typedef std::deque< CICloneable* >      TMailQueue;

    CMailboxForCloneables( void );

    virtual ~CMailboxForCloneables();

    /**
     *  Adds the given eventid and data to the FIFO stack.
     *  if data is non-NULL then it will be cloned.
     *
     *  @param eventid the ID of the event you wish to add to the mailbox
     *  @param data cloneable data container for optional event data.
     */
    void AddMail( const CICloneable* mail = GUCEF_NULL );

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
    template< typename T >
    bool GetMail( T** mail );

    /**
     *  Attempts to retrieve mail from the mailbox for inspection without removing it from the mail stack
     *  You can use this in combination with PopMail() for operations which may fail at which point
     *  the mail needs to be retained within the mailbox in order to try again on a later time
     *
     *  @param eventid the ID of the event
     *  @param data cloneable data container for optional event data.
     *  @return whether mail was successfully retrieved from the mailbox.
     */
    template< typename T >
    bool PeekMail( T** mail );

    /**
     *  Removes the first mail item in the mail stack
     *  Intended to be used in combination with PeekMail()
     */
    bool PopMail( void );

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
    template< typename TContainer >
    bool GetPtrBulkMail( TContainer& mailList    ,
                         Int32 maxMailItems = -1 );

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
    template< typename TContainer >
    bool GetValBulkMail( TContainer& mailList    ,
                         Int32 maxMailItems = -1 );

    /**
     *  Attempts to add mail to the mailbox in bulk
     *
     *  @param mailList container from where the mail items will be read
     *  @return whether mail was successfully added to the mailbox.
     */
    template< typename TContainer >
    bool AddBulkMail( const TContainer& mailList, UInt32 itemsPerLockCycle = 1000 );

    void Clear( void );

    void Delete( const CICloneable* mail );

    bool HasMail( void ) const;

    UInt32 AmountOfMail( void ) const;

    bool DoLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const;

    bool DoUnlock( void ) const;

    virtual const CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    template< typename S >
    const CICloneable* AsCloneablePtr( typename EnableIf< TypeIsPointerType< S >::value, S >::type& thing );

    template< typename S >
    const CICloneable* AsCloneablePtr( typename EnableIfNot< TypeIsPointerType< S >::value, S >::type& thing );

    CMailboxForCloneables( const CMailboxForCloneables& src );             /**< not implemented */
    CMailboxForCloneables& operator=( const CMailboxForCloneables& src );  /**< not implemented */

    protected:

    TMailQueue m_mailQueue;
    CMutex m_datalock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename S >
const CICloneable*
CMailboxForCloneables::AsCloneablePtr( typename EnableIf< TypeIsPointerType< S >::value, S >::type& thing )
{GUCEF_TRACE;

    return static_cast< const CICloneable* >( thing );
}

/*--------------------------------------------------------------------------*/

template< typename S >
const CICloneable*
CMailboxForCloneables::AsCloneablePtr( typename EnableIfNot< TypeIsPointerType< S >::value, S >::type& thing )
{GUCEF_TRACE;

    return static_cast< const CICloneable* >( &thing );
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CMailboxForCloneables::GetMail( T** mail )
{GUCEF_TRACE;

    if ( GUCEF_NULL == mail )
        return false;

    CObjectScopeLock lock( this );

    if ( !m_mailQueue.empty() )
    {
        *mail = static_cast< T >( m_mailQueue.front() );
        m_mailQueue.pop_front();
        return true;
    }
    *mail = GUCEF_NULL;
    return false;
}

/*--------------------------------------------------------------------------*/

template< typename T >
bool
CMailboxForCloneables::PeekMail( T** mail )
{GUCEF_TRACE;

    if ( GUCEF_NULL == mail )
        return false;

    CObjectScopeLock lock( this );

    if ( !m_mailQueue.empty() )
    {
        *mail = static_cast< T* >( m_mailQueue.front() );
        return true;
    }
    *mail = GUCEF_NULL;
    return false;
}

/*-------------------------------------------------------------------------*/

template< typename TContainer >
bool
CMailboxForCloneables::GetPtrBulkMail( TContainer& mailList ,
                                       Int32 maxMailItems   )
{GUCEF_TRACE;

    CObjectScopeLock lock( this );

    Int32 mailItemsRead = 0;
    while ( mailItemsRead < maxMailItems || maxMailItems < 0 )
    {
        if ( !m_mailQueue.empty() )
        {
            mailList.push_back( static_cast< typename TContainer::value_type >( m_mailQueue.front() ) );
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

/*-------------------------------------------------------------------------*/

template< typename TContainer >
bool
CMailboxForCloneables::GetValBulkMail( TContainer& mailList ,
                                       Int32 maxMailItems   )
{GUCEF_TRACE;

    CObjectScopeLock lock( this );

    Int32 mailItemsRead = 0;
    while ( mailItemsRead < maxMailItems || maxMailItems < 0 )
    {
        if ( !m_mailQueue.empty() )
        {
            // We know we are dealing with clonables so fetching by value means we need to guard against memory leaks
            typename TContainer::value_type* mail = static_cast< typename TContainer::value_type* >( m_mailQueue.front() );
            mailList.push_back( *mail );
            GUCEF_DELETE mail;
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

/*-------------------------------------------------------------------------*/

template< typename TContainer >
bool
CMailboxForCloneables::AddBulkMail( const TContainer& mailList, UInt32 itemsPerLockCycle )
{GUCEF_TRACE;

    bool totalSuccess = true;

    typename TContainer::const_iterator i = mailList.begin();
    while ( i != mailList.end() )
    {
        CObjectScopeLock lock( this );

        UInt32 n=0;
        while ( i != mailList.end() && n < itemsPerLockCycle )
        {
            const CICloneable* origMail = AsCloneablePtr< const typename TContainer::const_iterator::value_type >( (*i) );
            if ( GUCEF_NULL != origMail )
            {
                CICloneable* clonedMail = origMail->Clone();
                if ( GUCEF_NULL != clonedMail )
                {
                    m_mailQueue.push_back( clonedMail );
                }
                else
                {
                    // Cloning failed
                    totalSuccess = false;
                }
            }
            ++i; ++n;
        }
    }
    
    return totalSuccess;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MT_CMAILBOXFORCLONEABLES_H  ? */
