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

#ifndef GUCEF_CORE_CIOACCESS_H
#define GUCEF_CORE_CIOACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>

#ifndef GUCEF_MT_CNOLOCK_H
#include "gucefMT_CNoLock.h"
#define GUCEF_MT_CNOLOCK_H
#endif /* GUCEF_MT_CNOLOCK_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"          /* GUCEF string implementation */
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_C_IOACCESS_H
#include "gucefCORE_c_ioaccess.h"           /* C implementation wrapped by this class */
#define GUCEF_CORE_C_IOACCESS_H
#endif /* GUCEF_CORE_C_IOACCESS_H ? */

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

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

class CDynamicBuffer;
class CVariant;

/*-------------------------------------------------------------------------*/

/**
 *      Abstract base class for media independent resource access
 */
class GUCEF_CORE_PUBLIC_CPP CIOAccess : public CICloneable
{
    public:

    typedef CTSharedPtr< CIOAccess, MT::CNoLock >   IOAccessNoLockPtr;     
    typedef GUCEF::set< IOAccessNoLockPtr >         IOAccessNoLockPtrSet;
    typedef CTSharedPtr< CIOAccess, MT::CMutex >    IOAccessPtr;          /**< threadsafe pointer to a resource accessor */

    CIOAccess( void );

    CIOAccess( const CIOAccess& src );

    virtual ~CIOAccess();

    CIOAccess& operator=( const CIOAccess& src );

    /**
     *      open the resource for I/O
     */
    virtual void Open( void ) = 0;

    /**
     *      close the resource
     */
    virtual void Close( void ) = 0;

    /**
     *      is the resource opened for reading ?
     */
    virtual bool Opened( void ) const = 0;

    /**
     *      reads data until the specified delimiter is reached.
     *      The data is written into the destination buffer until the
     *      delimiter is reached or the end of the file is reached.
     *      The delimiter itself is not written to the destination buffer
     *
     *      @param dest the destination buffer for the data
     *      @param bsize size of the destination buffer
     *      @param delimiter data segment delimiter
     *      @param size of the data segment delimiter
     *      @return number of bytes written into dest
     */
    UInt32 ReadUntill( void *dest            ,
                       UInt32 bsize          ,
                       const void* delimiter ,
                       UInt32 delimsize      );

    /**
     *      Skips bytesuntil the delimiter is reached or
     *      until the end of the file is reached.
     *      the actual number of bytes skipped is returned.
     *
     *      @param delimiter the delimiter bytes
     *      @param delimsize size of the delimiter segment
     *      @return the actual number of bytes skipped.
     */
    UInt32 SkipUntill( const void* delimiter ,
                       UInt32 delimsize      );

    /**
     *      read a line of text
     *      The delimiter is the platform text file line
     *      delimiter.
     */
    virtual CString ReadLine( void );

    /**
     *      Reads a string from the resource
     */
    virtual CString ReadString( void );

    /**
     *      Attempts to read the specified number of bytes from the resource
     */
    virtual UInt32 Read( void *dest      ,
                         UInt32 esize    ,
                         UInt32 elements ) = 0;

    UInt32 Read( CDynamicBuffer& dest ,
                 UInt32 esize         ,
                 UInt32 elements      );

    UInt32 Read( CDynamicBuffer& dest ,
                 UInt32 esize         );

    template < typename T >
    bool ReadValue( T& value ) { return 1 == Read( &value, sizeof( T ), 1 ); }

    /**
     *  Attempts to read a binary safe string that has a UInt32 byteSize field followed by the actual bytes
     *  This is a common way of reading strings that is both binary safe and doesnt rely on seeking for a null terminator    
     */
    bool ReadByteSizePrefixedString( CString& str );

    /**
     *  Attempts to write a binary safe string that has a UInt32 byteSize field followed by the actual bytes
     *  This is a common way of reading strings that is both binary safe and doesnt rely on seeking for a null terminator    
     */
    bool WriteByteSizePrefixedString( const CString& str );

    /**
     *      Attempts to write the specified number of bytes to the resource
     *      using srcdata as the data source.
     */
    virtual UInt32 Write( const void* srcdata ,
                          UInt32 esize        ,
                          UInt32 elements     );

    /**
     *  Attempts to write all the elements from the source data buffer
     *  returns the number of elements written
     */
    virtual UInt32 Write( CDynamicBuffer& sourceData ,
                          UInt32 esize               ,
                          UInt32 elements            );

    /**
     *  Attempts to write all the bytes in the source data buffer
     *  returns the number of bytes written
     */
    virtual UInt32 Write( CDynamicBuffer& sourceData );

    virtual UInt64 Write( CIOAccess& sourceData   ,
                          Int64 bytesToWrite = -1 );

    /**
     *  Attempts to write all the bytes of the string without the null terminator
     *  Optionally adds an EOL character to the end of the string if addEol is true
     *  returns the number of bytes written
     */
    virtual UInt32 Write( const CString& string ,
                          bool addEol = true    );

    /**
     *  Attempts to write all the raw bytes of the variant value
     *  returns the number of bytes written
     */
    virtual UInt32 Write( const CVariant& var               , 
                          bool includeNullTermIfAny = false );

    template < typename T >
    bool WriteValue( const T value ) { return 1 == Write( &value, sizeof( T ), 1 ); }

    /**
     *      Get the current offset in bytes
     */
    virtual UInt64 Tell( void ) const = 0;

    /**
     *      jump to a different part of the resource
     */
    virtual Int32 Seek( Int64 offset ,
                        Int32 origin ) = 0;

    /**
     *      jump to the given offset in the resource
     */
    virtual UInt32 Setpos( UInt64 position );

    /**
     *      Read a single character
     */
    virtual char GetChar( void ) = 0;

    /**
        *      are we at the end of the resource ?
        */
    virtual bool Eof( void ) const = 0;

    /**
     *      Can we read from this resource?
     */
    virtual bool IsReadable( void ) const = 0;

    /**
     *      Can we write to this resource?
     */
    virtual bool IsWriteable( void ) const = 0;

    /**
     *      Is the access to the resource a valid one or
     *      has something gone wrong ?
     */
    virtual bool IsValid( void ) = 0;

    /**
     *  @return returns the size of the resource if possible.
     */
    virtual UInt64 GetSize( void ) const;

    /**
     *  Flushes all outstanding mutations on the I/O device
     */
    virtual void Flush( void ) = 0;

    virtual TIOAccess* CStyleAccess( void );

    private:

    void LinkCStyleAccess( void );

    private:

    TIOAccess m_cStyleAccess;
};

/*-------------------------------------------------------------------------*/

typedef CIOAccess::IOAccessPtr IOAccessPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CIOACCESS_H ? */
