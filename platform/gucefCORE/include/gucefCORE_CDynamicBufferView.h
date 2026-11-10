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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_VIEW_H
#define GUCEF_CORE_CDYNAMICBUFFER_VIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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
 *  Non-owning view of a CDynamicBuffer.
 *
 *  CDynamicBufferView wraps a CDynamicBuffer that is permanently kept in linked
 *  (non-owning) mode, enforcing the non-owning contract at the type level.
 *  It is impossible to have a CDynamicBufferView that owns memory.
 *
 *  Copying a view copies the link (rebinds to the same external buffer).
 *  The implicit conversion operator allows passing a view anywhere that
 *  a const CDynamicBuffer& is accepted without any allocation.
 *
 *  The caller is responsible for ensuring that the referenced buffer
 *  remains valid for the lifetime of the view.
 */
class GUCEF_CORE_PUBLIC_CPP CDynamicBufferView
{
    public:

    CDynamicBufferView( void );

    CDynamicBufferView( const void* externalBuffer ,
                        UInt32 bufferSize          );

    explicit CDynamicBufferView( const CDynamicBuffer& src );

    CDynamicBufferView( const CDynamicBufferView& src );

    CDynamicBufferView& operator=( const CDynamicBufferView& src );

    ~CDynamicBufferView();

    /* --- Rebind operations --- */

    CDynamicBufferView& LinkTo( const void* externalBuffer ,
                                UInt32 bufferSize          );

    CDynamicBufferView& LinkTo( const CDynamicBuffer& src );

    CDynamicBufferView& LinkTo( const CVariant& src );

    CDynamicBufferView& LinkTo( const CAsciiString& src, bool includeNullTerm = true );

    CDynamicBufferView& LinkTo( const CUtf8String& src, bool includeNullTerm = true );

    /* --- Const read operations — all delegate to the inner buffer --- */

    UInt32 GetBufferSize( void ) const;
    UInt32 GetDataSize( void ) const;
    UInt32 GetRemainingDataSize( UInt32 offset ) const;
    UInt32 GetUnusedBufferSize( void ) const;
    UInt32 GetRemainingBufferSize( UInt32 offset ) const;

    bool IsLinked( void ) const;   /**< always true for a view */

    const void* GetConstBufferPtr( UInt32 offset = 0 ) const;

    char operator[]( UInt32 index ) const;

    bool operator==( const CDynamicBuffer& other ) const;
    bool operator!=( const CDynamicBuffer& other ) const;
    bool operator<( const CDynamicBuffer& other ) const;
    bool operator>( const CDynamicBuffer& other ) const;

    UInt32 CopyTo( UInt32 offset ,
                   UInt32 size   ,
                   void* dest    ) const;

    UInt32 CopyTo( void* dest ) const;

    UInt32 CopyTo( CDynamicBuffer& dest ) const;

    Int32 Find( const void* data   ,
                UInt32 size        ,
                UInt32 offset = 0  ) const;

    Int32 FindPerElement( const void* searchData                ,
                          UInt32 searchDataSizeInElements       ,
                          UInt32 elementSize                    ,
                          UInt32 offsetInBytes = 0              ) const;

    CUtf8String::StringVector ParseUtf8StringElements( UInt32 seperatorCodePoint    ,
                                                       bool addEmptyElements = true ) const;

    CUtf8String::StringSet ParseUniqueUtf8StringElements( UInt32 seperatorCodePoint    ,
                                                          bool addEmptyElements = true ) const;

    CVariant AsVariant( UInt32 bufferOffset = 0, UInt8 varType = GUCEF_DATATYPE_BINARY_BLOB ) const;

    /* --- Templated const accessors — inline wrappers --- */

    template< typename T >
    const T& AsConstType( UInt32 byteOffset = 0 ) const;

    template< typename T >
    const T* AsConstTypePtr( UInt32 byteOffset = 0 ) const;

    template< typename T >
    const T* AsConstTypePtr( UInt32 byteOffset, UInt32 requiredSizeOfT ) const;

    /**
     *  Implicit conversion: allows a view to be passed wherever
     *  const CDynamicBuffer& is accepted without copying.
     */
    operator const CDynamicBuffer&( void ) const;

    /**
     *  Explicit accessor to the inner linked buffer.
     */
    const CDynamicBuffer& AsBuffer( void ) const;

    private:

    CDynamicBuffer m_buffer;  /**< invariant: m_buffer.IsLinked() == true at all times */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T >
inline const T&
CDynamicBufferView::AsConstType( UInt32 byteOffset ) const
{GUCEF_TRACE;
    return m_buffer.AsConstType< T >( byteOffset );
}

/*-------------------------------------------------------------------------*/

template< typename T >
inline const T*
CDynamicBufferView::AsConstTypePtr( UInt32 byteOffset ) const
{GUCEF_TRACE;
    return m_buffer.AsConstTypePtr< T >( byteOffset );
}

/*-------------------------------------------------------------------------*/

template< typename T >
inline const T*
CDynamicBufferView::AsConstTypePtr( UInt32 byteOffset, UInt32 requiredSizeOfT ) const
{GUCEF_TRACE;
    return m_buffer.AsConstTypePtr< T >( byteOffset, requiredSizeOfT );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDYNAMICBUFFER_VIEW_H ? */
