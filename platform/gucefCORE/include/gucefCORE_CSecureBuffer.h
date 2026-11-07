/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CSECUREBUFFER_H
#define GUCEF_CORE_CSECUREBUFFER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  A byte buffer for a secret (a password, a token, a key) with a capacity that is fixed when it is made
 *  and that is never exceeded: content that does not fit is refused, nothing grows on the way.
 *  Every byte that stops being content is overwritten (SecureWipe()) before it is reused or given back:
 *  Clear(), Truncate(), a shorter Assign(), SetCapacity(), the destructor. The byte after the content is
 *  always zero, so GetData() is also a C string.
 *
 *  A copy is an explicit act (the copy constructor, CopyFrom(), CopyToString()), has storage of its own and is
 *  wiped the same way when it goes out of scope: a function that hands out a CSecureBuffer by value hands out
 *  a scoped secret. There is no stream operator and no implicit conversion to a string, so a buffer cannot
 *  end up in a log line by accident.
 *
 *  Not thread safe. Memory locking and protection from core dumps are not done here.
 */
class GUCEF_CORE_PUBLIC_CPP CSecureBuffer
{
    public:

    /**
     *  Called with the whole storage (content area and terminator) of a buffer after it was wiped and before it is
     *  given back to the allocator. For tests that check the wiping without reading memory that is already freed.
     */
    typedef void ( *TReleaseAuditor )( const void* storage, UInt32 storageSize );

    static void SetReleaseAuditor( TReleaseAuditor auditor );

    /** No capacity: nothing fits, until SetCapacity() */
    CSecureBuffer( void );

    /** Storage of its own for 'capacity' bytes of content */
    explicit CSecureBuffer( UInt32 capacity );

    /**
     *  Uses storage of the caller: it has to be at least capacity + 1 bytes, outlive this object and is not
     *  freed by it; it is cleared now and wiped when this object goes away. A buffer on the stack for example.
     */
    CSecureBuffer( void* storage, UInt32 capacity );

    /** A copy with storage of its own, as large as the content of the source needs */
    CSecureBuffer( const CSecureBuffer& src );

    ~CSecureBuffer();

    UInt32 GetCapacity( void ) const;

    UInt32 GetSize( void ) const;

    bool IsEmpty( void ) const;

    bool IsOwningStorage( void ) const;

    /** The content, followed by a zero byte; valid until the next change */
    const UInt8* GetData( void ) const;

    /** Replaces the content; false (and an empty buffer) when it does not fit */
    bool Assign( const void* data, UInt32 size );

    /** Adds to the content; false (and nothing added) when it does not fit */
    bool Append( const void* data, UInt32 size );

    /** Replaces the content with a copy of the content of the source; false (and an empty buffer) when it does not fit */
    bool CopyFrom( const CSecureBuffer& src );

    /** Keeps the first 'size' bytes, wipes the rest; does nothing when the content is not longer than that */
    void Truncate( UInt32 size );

    /** Wipes everything */
    void Clear( void );

    /**
     *  Moves to storage for 'capacity' bytes of content, keeping what fits, and wipes the old storage;
     *  false for storage that the caller supplied, and when the new storage cannot be had
     */
    bool SetCapacity( UInt32 capacity );

    /** Whether the content is exactly these bytes; the time taken does not depend on where they differ */
    bool Equals( const void* data, UInt32 size ) const;

    /**
     *  For code that fills the buffer in place (reading from the O/S for example): the whole capacity can be written,
     *  then SetSize() says how much of it is content. The buffer is empty before the writing starts.
     */
    UInt8* GetWritableData( void );

    /** Sets the size of the content to the given number of bytes written through GetWritableData(); false when above the capacity */
    bool SetSize( UInt32 size );

    /**
     *  Puts a copy of the content in a string. This ends the protection of that copy: the string is not wiped
     *  by its destructor, wipe it with SecureWipeString() when done.
     */
    void CopyToString( CString& destination ) const;

    /** The whole storage including the terminator, for tests that check that the memory was wiped */
    const UInt8* GetRawStorage( void ) const;

    UInt32 GetRawStorageSize( void ) const;

    private:

    CSecureBuffer& operator=( const CSecureBuffer& src );

    void Release( void );

    private:

    UInt8* m_storage;
    UInt32 m_capacity;
    UInt32 m_size;
    bool m_ownsStorage;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSECUREBUFFER_H ? */
