/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H
#define GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMAILBOXFORCLONEABLES_H
#include "gucefMT_CMailBoxForCloneables.h"
#define GUCEF_MT_CMAILBOXFORCLONEABLES_H
#endif /* GUCEF_MT_CMAILBOXFORCLONEABLES_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

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
 *  Expands upon CMailboxForCloneables functionality by constructing shared pointers
 *  Upon mailbox egress
 */
template< typename CloneableType, class PtrLockType >
class CTMailboxForSharedCloneables : public MT::CMailboxForCloneables
{
    public:

    typedef CloneableType                                           TCloneableType;
    typedef PtrLockType                                             TSharedPtrLockType;
    typedef CTSharedPtr< CloneableType, PtrLockType >               TMailSPtr;
    typedef GUCEF::vector< TMailSPtr >                              TMailSPtrList;

    CTMailboxForSharedCloneables( void );

    virtual ~CTMailboxForSharedCloneables();

    /**
     *  Attempts to retrieve mail from the mailbox.
     *
     *  Note that if data is non-NULL then you should delete the
     *  object after handling the mail message or you will create
     *  a memory-leak.
     *
     *  @param mail the shared pointer that will hold the mail
     *  @return whether mail was successfully retrieved from the mailbox.
     */
    bool GetMailSPtr( TMailSPtr& mail );

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
    bool GetSPtrBulkMail( TMailSPtrList& mailList ,
                          Int32 maxMailItems = -1 );

    /**
     *  Attempts to pop back the items removed from the mailbox due to a GetSPtrBulkMail() call
     *  Note that this requires the items to not have any other outstanding references via their shared pointers
     */
    bool ReInsertSPtrBulkMail( TMailSPtrList& mailList            ,
                               bool bestEffortSkipIfNeeded = true );

    private:

    CTMailboxForSharedCloneables( const CTMailboxForSharedCloneables& src );             /**< not implemented */
    CTMailboxForSharedCloneables& operator=( const CTMailboxForSharedCloneables& src );  /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename CloneableType, class PtrLockType >
CTMailboxForSharedCloneables< CloneableType, PtrLockType >::CTMailboxForSharedCloneables( void )
    : MT::CMailboxForCloneables()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename CloneableType, class PtrLockType >
CTMailboxForSharedCloneables< CloneableType, PtrLockType >::~CTMailboxForSharedCloneables()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename CloneableType, class PtrLockType >
bool
CTMailboxForSharedCloneables< CloneableType, PtrLockType >::GetMailSPtr( TMailSPtr& mail )
{GUCEF_TRACE;

    mail.Unlink();
    MT::CObjectScopeLock lock( this );

    if ( !m_mailQueue.empty() )
    {
        mail = TMailSPtr( static_cast< CloneableType* >( m_mailQueue.front() ) );
        m_mailQueue.pop_front();
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template< typename CloneableType, class PtrLockType >
bool
CTMailboxForSharedCloneables< CloneableType, PtrLockType >::GetSPtrBulkMail( TMailSPtrList& mailList ,
                                                                             Int32 maxMailItems      )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    if ( m_mailQueue.empty() )
        return false; // nothing to read, early out

    // We know how many mail items we will read so lets not perform unnessesary reallocs of the vector's underlying memory
    if ( maxMailItems > 0 )
    {
        size_t itemsToRead = GUCEF_SMALLEST( m_mailQueue.size(), (size_t) maxMailItems );
        mailList.reserve( itemsToRead );
    }
    else
    {
        mailList.reserve( m_mailQueue.size() );
    }

    Int32 mailItemsRead = 0;
    while ( mailItemsRead < maxMailItems || maxMailItems < 0 )
    {
        if ( !m_mailQueue.empty() )
        {
            #ifdef GUCEF_DEBUG_MODE

            CICloneable* clonable = m_mailQueue.front();
            CloneableType* derivedClonable = static_cast< CloneableType* >( clonable );
            TMailSPtr objPtr( derivedClonable );
            mailList.push_back( objPtr );
            GUCEF_ASSERT( objPtr.GetPointerAlways() == static_cast< CloneableType* >( m_mailQueue.front() ) );
            m_mailQueue.pop_front();

            #else

            mailList.push_back( TMailSPtr( static_cast< CloneableType* >( m_mailQueue.front() ) ) );
            m_mailQueue.pop_front();

            #endif

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

template< typename CloneableType, class PtrLockType >
bool
CTMailboxForSharedCloneables< CloneableType, PtrLockType >::ReInsertSPtrBulkMail( TMailSPtrList& mailList     ,
                                                                                  bool bestEffortSkipIfNeeded )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    bool errorOccured = false;
    typename TMailSPtrList::iterator i = mailList.begin();
    while ( i != mailList.end() )
    {
        // Note that this only works if the shared pointer holds the last remaining reference
        // otherwise relinquishing ownership will be refused
        CloneableType* rawPtr = GUCEF_NULL;
        if ( (*i).RelinquishOwnership( rawPtr ) )
        {
            m_mailQueue.push_front( rawPtr );
        }
        else
        {
            errorOccured = true;
            if ( !bestEffortSkipIfNeeded )
            {
                // we are not allowed to skip any items for the pop back
                // this may be because ordering garantuee is more important for the given use-case
                return false;
            }
        }
        ++i;
    }
    return !errorOccured;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H  ? */
