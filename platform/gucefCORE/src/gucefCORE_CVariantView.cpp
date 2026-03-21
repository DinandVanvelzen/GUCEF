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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVARIANT_VIEW_H
#include "gucefCORE_CVariantView.h"
#define GUCEF_CORE_CVARIANT_VIEW_H
#endif /* GUCEF_CORE_CVARIANT_VIEW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVariantView::CVariantView( void )
    : m_variant()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( const CVariant& src )
    : m_variant()
{GUCEF_TRACE;

    m_variant.LinkTo( src );
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( const void* externalBuffer ,
                             UInt32 bufferSize          ,
                             UInt8 varType              )
    : m_variant()
{GUCEF_TRACE;

    m_variant.LinkTo( externalBuffer, bufferSize, varType );
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( const char* externalBuffer ,
                             UInt8 varType              )
    : m_variant()
{GUCEF_TRACE;

    m_variant.LinkTo( externalBuffer, varType );
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( const CDynamicBuffer& src ,
                             UInt8 varType             )
    : m_variant()
{GUCEF_TRACE;

    m_variant.LinkTo( src, 0, varType, 0 );
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( bool data )
    : m_variant( data )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( Int8 data )
    : m_variant( data )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( UInt8 data )
    : m_variant( data )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( Int16 data )
    : m_variant( data )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( UInt16 data )
    : m_variant( data )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( Int32 data )
    : m_variant( data )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( UInt32 data )
    : m_variant( data )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( Int64 data )
    : m_variant( data )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( UInt64 data )
    : m_variant( data )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( Float32 data )
    : m_variant( data )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( Float64 data )
    : m_variant( data )
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CVariantView::CVariantView( const CVariantView& src )
    : m_variant()
{GUCEF_TRACE;

    m_variant.LinkTo( src.m_variant );
}

/*-------------------------------------------------------------------------*/

CVariantView&
CVariantView::operator=( const CVariantView& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_variant.LinkTo( src.m_variant );
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantView::~CVariantView()
{GUCEF_TRACE;

    /* m_variant destructor will not free linked heap memory */
}

/*-------------------------------------------------------------------------*/

CVariantView&
CVariantView::LinkTo( const CVariant& src )
{GUCEF_TRACE;

    m_variant.LinkTo( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantView&
CVariantView::LinkTo( const void* externalBuffer ,
                       UInt32 bufferSize          ,
                       UInt8 varType              )
{GUCEF_TRACE;

    m_variant.LinkTo( externalBuffer, bufferSize, varType );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantView&
CVariantView::LinkTo( const char* externalBuffer ,
                       UInt8 varType              )
{GUCEF_TRACE;

    m_variant.LinkTo( externalBuffer, varType );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantView&
CVariantView::LinkTo( const CDynamicBuffer& src ,
                       UInt32 bufferOffset       ,
                       UInt8 varType             ,
                       UInt32 bytesToLink        )
{GUCEF_TRACE;

    m_variant.LinkTo( src, bufferOffset, varType, bytesToLink );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantView&
CVariantView::LinkTo( const CAsciiString& src )
{GUCEF_TRACE;

    m_variant.LinkTo( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantView&
CVariantView::LinkTo( const CUtf8String& src )
{GUCEF_TRACE;

    m_variant.LinkTo( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantView&
CVariantView::LinkTo( const CUtf16String& src )
{GUCEF_TRACE;

    m_variant.LinkTo( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

CVariantView&
CVariantView::LinkTo( const CUtf32String& src )
{GUCEF_TRACE;

    m_variant.LinkTo( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsInteger( void ) const
{GUCEF_TRACE;

    return m_variant.IsInteger();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsSignedInteger( void ) const
{GUCEF_TRACE;

    return m_variant.IsSignedInteger();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsUnsignedInteger( void ) const
{GUCEF_TRACE;

    return m_variant.IsUnsignedInteger();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsFloat( void ) const
{GUCEF_TRACE;

    return m_variant.IsFloat();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsNumber( void ) const
{GUCEF_TRACE;

    return m_variant.IsNumber();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsFraction( void ) const
{GUCEF_TRACE;

    return m_variant.IsFraction();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsString( void ) const
{GUCEF_TRACE;

    return m_variant.IsString();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsBoolean( void ) const
{GUCEF_TRACE;

    return m_variant.IsBoolean();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsBinary( void ) const
{GUCEF_TRACE;

    return m_variant.IsBinary();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsBlob( void ) const
{GUCEF_TRACE;

    return m_variant.IsBlob();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsNULLOrEmpty( void ) const
{GUCEF_TRACE;

    return m_variant.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_variant.IsInitialized();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsNIL( void ) const
{GUCEF_TRACE;

    return m_variant.IsNIL();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsNULL( void ) const
{GUCEF_TRACE;

    return m_variant.IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsVoid( void ) const
{GUCEF_TRACE;

    return m_variant.IsVoid();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsMemoryAddress( void ) const
{GUCEF_TRACE;

    return m_variant.IsMemoryAddress();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::IsDynamicMemoryLinked( void ) const
{GUCEF_TRACE;

    return m_variant.IsDynamicMemoryLinked();
}

/*-------------------------------------------------------------------------*/

UInt8
CVariantView::GetTypeId( void ) const
{GUCEF_TRACE;

    return m_variant.GetTypeId();
}

/*-------------------------------------------------------------------------*/

const char*
CVariantView::GetTypeNameC( void ) const
{GUCEF_TRACE;

    return m_variant.GetTypeNameC();
}

/*-------------------------------------------------------------------------*/

CString
CVariantView::GetTypeName( void ) const
{GUCEF_TRACE;

    return m_variant.GetTypeName();
}

/*-------------------------------------------------------------------------*/

UInt32
CVariantView::ByteSize( bool includeNullTerm ) const
{GUCEF_TRACE;

    return m_variant.ByteSize( includeNullTerm );
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::AsBool( bool defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsBool( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

Int8
CVariantView::AsInt8( Int8 defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsInt8( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

UInt8
CVariantView::AsUInt8( UInt8 defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsUInt8( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

Int16
CVariantView::AsInt16( Int16 defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsInt16( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

UInt16
CVariantView::AsUInt16( UInt16 defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsUInt16( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

Int32
CVariantView::AsInt32( Int32 defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsInt32( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

UInt32
CVariantView::AsUInt32( UInt32 defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsUInt32( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

Int64
CVariantView::AsInt64( Int64 defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsInt64( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

UInt64
CVariantView::AsUInt64( UInt64 defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsUInt64( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

Float32
CVariantView::AsFloat32( Float32 defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsFloat32( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

Float64
CVariantView::AsFloat64( Float64 defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsFloat64( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

CString
CVariantView::AsString( const CString& defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsString( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

CAsciiString
CVariantView::AsAsciiString( const CAsciiString& defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsAsciiString( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

CUtf8String
CVariantView::AsUtf8String( const CUtf8String& defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsUtf8String( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

CUtf16String
CVariantView::AsUtf16String( const CUtf16String& defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsUtf16String( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

CUtf32String
CVariantView::AsUtf32String( const CUtf32String& defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsUtf32String( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

const void*
CVariantView::AsVoidPtr( const void* defaultIfNeeded ) const
{GUCEF_TRACE;

    return m_variant.AsVoidPtr( defaultIfNeeded );
}

/*-------------------------------------------------------------------------*/

const char*
CVariantView::AsCharPtr( const char* defaultIfNeeded ) const
{GUCEF_TRACE;

    return m_variant.AsCharPtr( defaultIfNeeded );
}

/*-------------------------------------------------------------------------*/

CDynamicBuffer
CVariantView::AsBuffer( void ) const
{GUCEF_TRACE;

    return m_variant.AsBuffer();
}

/*-------------------------------------------------------------------------*/

CDateTime
CVariantView::AsDateTime( const CDateTime& defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsDateTime( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

CTimestamp
CVariantView::AsTimestamp( const CTimestamp& defaultIfNeeded, bool resolveVars ) const
{GUCEF_TRACE;

    return m_variant.AsTimestamp( defaultIfNeeded, resolveVars );
}

/*-------------------------------------------------------------------------*/

const TVariantData*
CVariantView::CStyleAccess( void ) const
{GUCEF_TRACE;

    return m_variant.CStyleAccess();
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::operator==( const CVariant& other ) const
{GUCEF_TRACE;

    return m_variant == other;
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::operator!=( const CVariant& other ) const
{GUCEF_TRACE;

    return m_variant != other;
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::operator<( const CVariant& other ) const
{GUCEF_TRACE;

    return m_variant < other;
}

/*-------------------------------------------------------------------------*/

bool
CVariantView::operator>( const CVariant& other ) const
{GUCEF_TRACE;

    return m_variant > other;
}

/*-------------------------------------------------------------------------*/

CVariantView::operator const CVariant&( void ) const
{GUCEF_TRACE;

    return m_variant;
}

/*-------------------------------------------------------------------------*/

const CVariant&
CVariantView::AsVariant( void ) const
{GUCEF_TRACE;

    return m_variant;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
