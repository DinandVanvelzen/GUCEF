/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CVARIANT_VIEW_H
#define GUCEF_CORE_CVARIANT_VIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

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
 *  Non-owning view of a CVariant value.
 *
 *  CVariantView wraps a CVariant that is kept in linked (non-owning) mode
 *  for heap-based types (strings, blobs), enforcing the non-owning contract
 *  at the type level for dynamic memory types.  Scalar types (int, float, etc.)
 *  are copied by value since they carry no heap allocation.
 *
 *  No mutable operations are exposed — the view is strictly read-only.
 *
 *  The implicit conversion operator allows passing a view anywhere that
 *  a const CVariant& is accepted without any allocation.
 *
 *  The caller is responsible for ensuring that any referenced heap buffer
 *  remains valid for the lifetime of the view.
 */
class GUCEF_CORE_PUBLIC_CPP CVariantView
{
    public:

    CVariantView( void );

    /**
     *  Link to an existing variant.
     *  For dynamic-memory types (strings, blobs) no copy is made.
     *  For scalar types the value is copied inline.
     */
    explicit CVariantView( const CVariant& src );

    /**
     *  Link to a raw buffer with the given variant type.
     */
    CVariantView( const void* externalBuffer, UInt32 bufferSize,
                  UInt8 varType = GUCEF_DATATYPE_BINARY_BLOB );

    /**
     *  Link to a null-terminated ASCII / UTF-8 string buffer.
     */
    CVariantView( const char* externalBuffer,
                  UInt8 varType = GUCEF_DATATYPE_ASCII_STRING );

    /**
     *  Link to a CDynamicBuffer.
     */
    explicit CVariantView( const CDynamicBuffer& src,
                           UInt8 varType = GUCEF_DATATYPE_BINARY_BLOB );

    /* --- Scalar value constructors (no heap allocation; value stored inline) --- */

    CVariantView( bool    data );
    CVariantView( Int8    data );
    CVariantView( UInt8   data );
    CVariantView( Int16   data );
    CVariantView( UInt16  data );
    CVariantView( Int32   data );
    CVariantView( UInt32  data );
    CVariantView( Int64   data );
    CVariantView( UInt64  data );
    CVariantView( Float32 data );
    CVariantView( Float64 data );

    CVariantView( const CVariantView& src );

    CVariantView& operator=( const CVariantView& src );

    ~CVariantView();

    /* --- Rebind operations --- */

    CVariantView& LinkTo( const CVariant& src );

    CVariantView& LinkTo( const void* externalBuffer, UInt32 bufferSize,
                          UInt8 varType = GUCEF_DATATYPE_BINARY_BLOB );

    CVariantView& LinkTo( const char* externalBuffer,
                          UInt8 varType = GUCEF_DATATYPE_ASCII_STRING );

    CVariantView& LinkTo( const CDynamicBuffer& src,
                          UInt32 bufferOffset = 0,
                          UInt8 varType = GUCEF_DATATYPE_BINARY_BLOB,
                          UInt32 bytesToLink = 0 );

    CVariantView& LinkTo( const CAsciiString& src );
    CVariantView& LinkTo( const CUtf8String& src );
    CVariantView& LinkTo( const CUtf16String& src );
    CVariantView& LinkTo( const CUtf32String& src );

    /* --- Type query --- */

    bool IsInteger( void ) const;
    bool IsSignedInteger( void ) const;
    bool IsUnsignedInteger( void ) const;
    bool IsFloat( void ) const;
    bool IsNumber( void ) const;
    bool IsFraction( void ) const;
    bool IsString( void ) const;
    bool IsBoolean( void ) const;
    bool IsBinary( void ) const;
    bool IsBlob( void ) const;
    bool IsNULLOrEmpty( void ) const;
    bool IsInitialized( void ) const;
    bool IsNIL( void ) const;
    bool IsNULL( void ) const;
    bool IsVoid( void ) const;
    bool IsMemoryAddress( void ) const;

    /**
     *  For dynamic-memory types this returns true when the inner variant
     *  was linked (not copied) to the external data.
     */
    bool IsDynamicMemoryLinked( void ) const;

    UInt8           GetTypeId( void ) const;
    const char*     GetTypeNameC( void ) const;
    CString         GetTypeName( void ) const;

    UInt32          ByteSize( bool includeNullTerm = true ) const;

    /* --- Value accessors (all const) --- */

    bool            AsBool( bool defaultIfNeeded = false, bool resolveVars = false ) const;
    Int8            AsInt8( Int8 defaultIfNeeded = 0, bool resolveVars = false ) const;
    UInt8           AsUInt8( UInt8 defaultIfNeeded = 0, bool resolveVars = false ) const;
    Int16           AsInt16( Int16 defaultIfNeeded = 0, bool resolveVars = false ) const;
    UInt16          AsUInt16( UInt16 defaultIfNeeded = 0, bool resolveVars = false ) const;
    Int32           AsInt32( Int32 defaultIfNeeded = 0, bool resolveVars = false ) const;
    UInt32          AsUInt32( UInt32 defaultIfNeeded = 0, bool resolveVars = false ) const;
    Int64           AsInt64( Int64 defaultIfNeeded = 0, bool resolveVars = false ) const;
    UInt64          AsUInt64( UInt64 defaultIfNeeded = 0, bool resolveVars = false ) const;
    Float32         AsFloat32( Float32 defaultIfNeeded = 0.0f, bool resolveVars = false ) const;
    Float64         AsFloat64( Float64 defaultIfNeeded = 0.0, bool resolveVars = false ) const;

    CString         AsString( const CString& defaultIfNeeded = CString::Empty, bool resolveVars = false ) const;
    CAsciiString    AsAsciiString( const CAsciiString& defaultIfNeeded = CAsciiString::Empty, bool resolveVars = false ) const;
    CUtf8String     AsUtf8String( const CUtf8String& defaultIfNeeded = CUtf8String::Empty, bool resolveVars = false ) const;
    CUtf16String    AsUtf16String( const CUtf16String& defaultIfNeeded = CUtf16String::Empty, bool resolveVars = false ) const;
    CUtf32String    AsUtf32String( const CUtf32String& defaultIfNeeded = CUtf32String::Empty, bool resolveVars = false ) const;

    const void*     AsVoidPtr( const void* defaultIfNeeded = GUCEF_NULL ) const;
    const char*     AsCharPtr( const char* defaultIfNeeded = GUCEF_NULL ) const;

    CDynamicBuffer  AsBuffer( void ) const;

    CDateTime       AsDateTime( const CDateTime& defaultIfNeeded = CDateTime::Empty, bool resolveVars = false ) const;
    CTimestamp      AsTimestamp( const CTimestamp& defaultIfNeeded = CTimestamp::Empty, bool resolveVars = false ) const;

    const TVariantData* CStyleAccess( void ) const;

    /* --- Comparison --- */

    bool operator==( const CVariant& other ) const;
    bool operator!=( const CVariant& other ) const;
    bool operator<( const CVariant& other ) const;
    bool operator>( const CVariant& other ) const;

    /* --- Templated accessors --- */

    template< typename T >
    T AsTValue( const T defaultIfNeeded = T(), bool resolveVars = false ) const;

    template< typename TemplateBsobType >
    TemplateBsobType AsBsob( void ) const;

    template< typename TemplateBsobType >
    const TemplateBsobType* AsBsobPtr( void ) const;

    /**
     *  Implicit conversion: allows a view to be passed wherever
     *  const CVariant& is accepted without copying.
     */
    operator const CVariant&( void ) const;

    /**
     *  Explicit accessor to the inner variant.
     */
    const CVariant& AsVariant( void ) const;

    private:

    CVariant m_variant;  /**< holds a view (linked for heap types) of external data */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename T >
inline T
CVariantView::AsTValue( const T defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;
    return m_variant.AsTValue< T >( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

template< typename TemplateBsobType >
inline TemplateBsobType
CVariantView::AsBsob( void ) const
{GUCEF_TRACE;
    return m_variant.AsBsob< TemplateBsobType >();
}

/*-------------------------------------------------------------------------*/

template< typename TemplateBsobType >
inline const TemplateBsobType*
CVariantView::AsBsobPtr( void ) const
{GUCEF_TRACE;
    return m_variant.AsBsobPtr< TemplateBsobType >();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CVARIANT_VIEW_H ? */
