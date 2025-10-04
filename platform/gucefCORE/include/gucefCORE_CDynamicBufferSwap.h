/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CDYNAMICBUFFERSWAP_H
#define GUCEF_CORE_CDYNAMICBUFFERSWAP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"   
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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
 *  Class that implements a set of buffers that are switched in active
 *  ownership between a reader and a writer
 */
class GUCEF_CORE_PUBLIC_CPP CDynamicBufferSwap : public MT::CILockable
{
    public:

    CDynamicBufferSwap( UInt32 nrOfBuffers = 3 );

    virtual ~CDynamicBufferSwap();

    bool SetNrOfBuffers( UInt32 nrOfBuffers );

    UInt32 GetNrOfBuffers( void ) const;
    
    void SetMinimalBufferSize( UInt32 minimalBufferSize );

    void GetBufferSizeRange( UInt32& smallest, UInt32& largest ) const;

    Int32 GetCurrentReaderBufferIndex( void ) const;

    Int32 GetCurrentWriterBufferIndex( void ) const;

    UInt32 GetBuffersQueuedToRead( void ) const;

    /**
     *  Releases the lock on the current read buffer (if any) and attempts to aquire a lock 
     *  on the next read buffer if there is a next buffer ready for reading
     *  If the lock could not be obtainned within the specified timeout period GUCEF_NULL will be returned
     */
    CDynamicBuffer* GetNextReaderBuffer( CDateTime& associatedDt                       ,
                                         bool alwaysBlockForBufferAvailability = false ,
                                         UInt32 lockWaitTimeoutInMs = 1000             ,
                                         Int32* currentReaderBufferIndex = GUCEF_NULL  );

    /**
     *  Same as the other variant of GetNextReaderBuffer() except you dont have to pass along a
     *  associatedDt if you dont care about that information    
     */
    CDynamicBuffer* GetNextReaderBuffer( bool alwaysBlockForBufferAvailability = false ,
                                         UInt32 lockWaitTimeoutInMs = 1000             ,
                                         Int32* currentReaderBufferIndex = GUCEF_NULL  );

    /**
     *  Once a read buffer has been obtained using GetNextReaderBuffer() you can also 'peek' at the
     *  next available read buffer if any exists. This operation does not move the actual read cursor state
     *  Only the passed-in indeces are used as cursors and hence this does not advance the read positon 
     *  to the next buffer (if any) nor does it signal the end of reading. 
     *  The first call to PeekNextReaderBuffer() should use the GetNextReaderBuffer()'s 'currentReaderBufferIndex' value for 'peekReaderBufferIndex'
     *  
     *  Note that the multi-threading lock protection is linked to your lifecycle of GetNextReaderBuffer()
     *  calls and SignalEndOfReading() utilizing either of those invalidates the lock protection for the buffer 
     *  that was returned as part of a PeekNextReaderBuffer()
     *  As such the expectation is that you always combine your GetNextReaderBuffer, PeekNextReaderBuffer and SignalEndOfReading
     *  calls in the same thread as part of a sequenctial calling sequence
     */
    CDynamicBuffer* PeekNextReaderBuffer( CDateTime& associatedDt         ,
                                          Int32 peekReaderBufferIndex     ,
                                          Int32* newPeekReaderBufferIndex );

    /**
     *  Same as the other variant of PeekNextReaderBuffer() except you dont have to pass along a
     *  associatedDt if you dont care about that information    
     */
    CDynamicBuffer* PeekNextReaderBuffer( Int32 peekReaderBufferIndex     ,
                                          Int32* newPeekReaderBufferIndex );
    
    /**
     *  Re-obtains the currently assigned reader buffer, if any
     *  returns GUCEF_NULL if there is no reader buffer currently assigned
     *  This does not advance to the next buffer (if any) nor does it signal the end of reading
     */
    CDynamicBuffer* GetCurrenReaderBuffer( CDateTime& associatedDt           ,
                                           UInt32 lockWaitTimeoutInMs = 1000 );
        
    /**
     *  Provides the associated datetime of the current reader buffer if there is one
     *  if no reading is ongoing then CDateTime::Empty is returned
     *
     *  Note that this is really only useful from the same thread that does the reading for most use-cases
     *  From any other thread this is merely a mostly undefined snapshot in time
     */
    CDateTime GetCurrenReaderBufferAssociatedDt( void ) const;

    /**
     *  Provides the associated datetime of the current writer buffer if there is one
     *  if no writing is ongoing then CDateTime::Empty is returned
     *
     *  Note that this is really only useful from the same thread that does the writing for most use-cases
     *  From any other thread this is merely a mostly undefined snapshot in time
     */
    CDateTime GetCurrenWriterBufferAssociatedDt( void ) const;
    
    /**
     *  Releases the lock on the current read buffer
     *  This operation is also an implicit part of GetNextReaderBuffer()
     */
    bool SignalEndOfReading( void );

    /**
     *  Releases the lock on the current write buffer (if any) and attempts to aquire a lock 
     *  on the next write buffer. 
     *  If the lock could not be obtainned within the specified timeout period GUCEF_NULL will be returned
     *  This operation also sets the buffer associated dt to the dt the buffer was obtained upon successfully obtaining a write buffer
     */
    CDynamicBuffer* GetNextWriterBuffer( bool alwaysBlockForBufferAvailability ,
                                         UInt32 lockWaitTimeoutInMs            );

    /**
     *  Releases the lock on the current write buffer (if any) and attempts to aquire a lock 
     *  on the next write buffer. 
     *  If the lock could not be obtainned within the specified timeout period GUCEF_NULL will be returned
     *  The passed associated dt if non-Empty will be set as the buffer associated dt if a buffer is obtained
     */
    CDynamicBuffer* GetNextWriterBuffer( const CDateTime& associatedDt         ,
                                         bool alwaysBlockForBufferAvailability ,
                                         UInt32 lockWaitTimeoutInMs            );

    /**
     *  Releases the lock on the current write buffer if any
     *  This operation is also an implicit part of GetNextWriterBuffer()
     */
    bool SignalEndOfWriting( void );

    virtual const MT::CILockable * AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:
    
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CDynamicBufferSwap( const CDynamicBufferSwap& src ); /**< not implemented, not allowed */
    CDynamicBufferSwap& operator=( const CDynamicBufferSwap& src ); /**< not implemented, not allowed */

    class CBufferEntry
    {
        public:

        CDynamicBuffer buffer;
        MT::CMutex lock;
        bool hasUnreadData;
        CDateTime associatedDt;

        CBufferEntry( void );
        CBufferEntry( const CBufferEntry& src );
    };

    typedef GUCEF::vector< CBufferEntry > TBufferEntryVector;
    
    TBufferEntryVector m_buffers;
    Int32 m_currentReaderBufferIndex;
    bool m_readingIsOngoing;
    Int32 m_currentWriterBufferIndex;
    bool m_writingIsOngoing;
    UInt32 m_buffersQueuedToRead;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDYNAMICBUFFERSWAP_H ? */
