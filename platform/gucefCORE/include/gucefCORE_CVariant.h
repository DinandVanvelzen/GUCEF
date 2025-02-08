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

#ifndef GUCEF_CORE_CVARIANT_H
#define GUCEF_CORE_CVARIANT_H

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

#ifndef GUCEF_CORE_CITIME_H
#include "gucefCORE_CITime.h"
#define GUCEF_CORE_CITIME_H
#endif /* GUCEF_CORE_CITIME_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CTFRACTION_H
#include "gucefCORE_CTFraction.h"
#define GUCEF_CORE_CTFRACTION_H
#endif /* GUCEF_CORE_CTFRACTION_H ? */

#ifndef GUCEF_CORE_C_VARIANTDATA_H
#include "gucefCORE_c_variantdata.h"
#define GUCEF_CORE_C_VARIANTDATA_H
#endif /* GUCEF_CORE_C_VARIANTDATA_H ? */

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

/**
 *  Imlementation of a Variant class mainly used for interaction between 
 *  compile-time and run-time typed environments and/or between weakly and
 *  strongly typed environments.
 *
 *  Performance wise your gain or loss will depend on data used to store in the variant
 *  and whether you otherwise would have used a string to represent the same.
 *  Strings always use dynamic memory allocations whereas this is not always the case
 *  with a variant. Various operations such as comparison can potentially also be faster 
 *  depending on the type interactions involved.
 */
class GUCEF_CORE_PUBLIC_CPP CVariant
{
    public:

    typedef std::vector< CVariant, gucef_allocator< CVariant > >                        VariantVector;
    typedef std::set< CVariant, std::less< CVariant >, gucef_allocator< CVariant > >    VariantSet;

    static const CVariant           Empty;
    static const VariantVector      EmptyVector;
    
    CVariant( void );

    CVariant( const TVariantData* variantData, bool linkIfPossible );

    CVariant( const TVariantData& variantData, bool linkIfPossible );
    
    /**
     *  Note that if the variant uses linked heap data said heap data will be copied
     *  to a privately owned copy to avoid hard to trace invalid pointer issues
     */
    CVariant( const CVariant& src );

    /**
     *  Copies the variant like a copy constructor but via the 'linkIfPossible' param
     *  allows specifying if we can perform a LinkTo() operation in a single step.
     *  Please use with care as to not end up with invalid pointers, this is intended to be used in-stack
     */
    CVariant( const CVariant& src, bool linkIfPossible );

    #ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
    CVariant( CVariant&& src ) GUCEF_NOEXCEPT;
    #endif
    
    ~CVariant(); 
    
    CVariant( bool    data );
    CVariant( Int8    data );
    CVariant( UInt8   data ); 
    CVariant( Int16   data );
    CVariant( UInt16  data );
    CVariant( Int32   data );
    CVariant( UInt32  data );
    CVariant( Int64   data );
    CVariant( UInt64  data );
    CVariant( Float32 data );
    CVariant( Float64 data );
    
    CVariant( void* data_memory_address );
    CVariant( TDefaultFuncPtr function_memory_address );

    CVariant( TInt32Fraction data );
    CVariant( TUInt32Fraction data );
    CVariant( const TInt64Int32Fraction& data );
    CVariant( const TUInt64UInt32Fraction& data );
    CVariant( const TInt64Fraction& data );
    CVariant( const TUInt64Fraction& data );
    
    /**
     *  Creates a private copy of the ASCII string data and sets the type as such
     */
    CVariant( const CAsciiString& data );

    /**
     *  Creates a private copy of the UTF8 string data and sets the type as such
     */
    CVariant( const CUtf8String& data );

    /**
     *  Creates a private copy of the assumed to be null terminated string data and sets the type as UTF8
     */
    CVariant( const char* utf8Str );

    /**
     *  Creates a private copy of the string data and sets the type as UTF8
     */
    CVariant( const std::string& utf8Str );

    CVariant( const void* data, UInt32 dataSize, UInt8 varType = GUCEF_DATATYPE_BINARY_BLOB );

    /**
     *  Creates a private copy of any data contained within the buffer
     */
    CVariant( const CDynamicBuffer& data, UInt8 varType = GUCEF_DATATYPE_BINARY_BLOB );

    /**
     *  Creates a variant holding the datetime value
     */
    CVariant( const CDateTime& datetime );

    /**
     *  Interprets the string form data per the type ID given
     *  Creates a private copy
     */
    CVariant( UInt8 varType, const CString& stringFormData, const CVariant& defaultValue = CVariant::Empty );
    
    bool IsInteger( void ) const;
    bool IsSignedInteger( void ) const;
    bool IsUnsignedInteger( void ) const;
    bool IsFloat( void ) const;
    bool IsNumber( void ) const;
    bool IsFraction( void ) const;
    bool IsSignedFraction( void ) const;
    bool IsUnsignedFraction( void ) const;
    bool IsString( void ) const;
    bool IsBoolean( void ) const;
    bool IsBinary( void ) const;
    bool IsBlob( void ) const;
    bool IsBsob( void ) const;
    bool IsMemoryAddress( void ) const;
    
    static bool UsesDynamicMemory( UInt8 typeId );
    bool UsesDynamicMemory( void ) const;
    bool IsDynamicMemoryLinked( void ) const;
    bool OwnsDynamicMemory( void ) const;

    /**
     *  If the variant object is the future recipient of a value which is based on dynamic memory
     *  this member function can be used to pre-allocate the memory needed to store the value.
     *  This allows one to forgo the potential overhead of an intermediate transfer buffer.
     * 
     *  The memory allocated will be owned by the variant object and will be freed when the object is destroyed.
     *  (provided the memory is not explicitly transferred to another object)
     * 
     *  Note that this function will refuse to allocate memory if the type specified is not a dynamic memory type.
     *  Using this functionality within the context of a non-dynamic memory type is considered an error.
     */
    void* PreAllocateDynamicMemory( UInt32 byteSize, UInt8 varType = GUCEF_DATATYPE_BINARY_BLOB );

    bool IsInitialized( void ) const;
    bool IsNULLOrEmpty( void ) const; 

    bool IsLittleEndian( void ) const;
    bool IsBigEndian( void ) const;
    bool IsEndianAgnostic( void ) const;

    CVariant& ToLittleEndian( void );
    CVariant& ToBigEndian( void );
    CVariant& ToNativeEndian( void );

    /**
     *  This member function allows you to set the type ID
     *  In most cases you should not need this as setting/assigning/linking automatically sets the correct type id
     *  However there can be cases where more direct manipulation of the underlying variant data is needed or occurs
     *  In such use cases this member function can be used to 'correct' the type to make the class aware of such changes.
     */
    void OverrideTypeId( UInt8 typeId );

    UInt8 GetTypeId( void ) const;
    const char* GetTypeNameC( void ) const;
    CString GetTypeName( void ) const;

    static UInt8 TypeIdForTypeName( const char* typeName );
    static UInt8 TypeIdForTypeName( const CString& typeName );
    static const char* TypeNameForTypeIdC( UInt8 typeId );
    static CString TypeNameForTypeId( UInt8 typeId );

    bool            AsBool( bool defaultIfNeeded = false, bool resolveVarsIfApplicable = false ) const;
    char            AsChar( char defaultIfNeeded = '\0' ) const;
    Int8            AsInt8( Int8 defaultIfNeeded = 0, bool resolveVarsIfApplicable = false ) const;
    UInt8           AsUInt8( UInt8 defaultIfNeeded = 0, bool resolveVarsIfApplicable = false ) const;
    Int16           AsInt16( Int16 defaultIfNeeded = 0, bool resolveVarsIfApplicable = false ) const;
    UInt16          AsUInt16( UInt16 defaultIfNeeded = 0, bool resolveVarsIfApplicable = false ) const;
    Int32           AsInt32( Int32 defaultIfNeeded = 0, bool resolveVarsIfApplicable = false ) const;
    UInt32          AsUInt32( UInt32 defaultIfNeeded = 0, bool resolveVarsIfApplicable = false ) const;
    Int64           AsInt64( Int64 defaultIfNeeded = 0, bool resolveVarsIfApplicable = false ) const;
    UInt64          AsUInt64( UInt64 defaultIfNeeded = 0, bool resolveVarsIfApplicable = false ) const;
    Float32         AsFloat32( Float32 defaultIfNeeded = 0.0f, bool resolveVarsIfApplicable = false ) const;
    Float64         AsFloat64( Float64 defaultIfNeeded = 0.0, bool resolveVarsIfApplicable = false ) const;
    CString         AsString( const CString& defaultIfNeeded = CString::Empty, bool resolveVarsIfApplicable = false ) const;
    CAsciiString    AsAsciiString( const CAsciiString& defaultIfNeeded = CAsciiString::Empty, bool resolveVarsIfApplicable = false ) const;
    CUtf8String     AsUtf8String( const CUtf8String& defaultIfNeeded = CUtf8String::Empty, bool resolveVarsIfApplicable = false ) const;
    const void*     AsVoidPtr( const void* defaultIfNeeded = GUCEF_NULL ) const;
    const char*     AsCharPtr( const char* defaultIfNeeded = GUCEF_NULL ) const;    
    CDynamicBuffer  AsBuffer( void ) const;
    CDateTime       AsDateTime( const CDateTime& defaultIfNeeded = CDateTime::Empty, bool resolveVarsIfApplicable = false ) const;
    size_t          AsSizeT( size_t defaultIfNeeded = 0, bool resolveVarsIfApplicable = false ) const;
    TDefaultFuncPtr AsDefaultFuncPtr( TDefaultFuncPtr defaultIfNeeded = GUCEF_NULL ) const;

    TInt8Fraction         AsInt8Fraction( const TInt8Fraction& defaultIfNeeded = TInt8Fraction(), bool resolveVarsIfApplicable = false ) const;
    TUInt8Fraction        AsUInt8Fraction( const TUInt8Fraction& defaultIfNeeded = TUInt8Fraction(), bool resolveVarsIfApplicable = false ) const;
    TInt16Fraction        AsInt16Fraction( const TInt16Fraction& defaultIfNeeded = TInt16Fraction(), bool resolveVarsIfApplicable = false ) const;
    TUInt16Fraction       AsUInt16Fraction( const TUInt16Fraction& defaultIfNeeded = TUInt16Fraction(), bool resolveVarsIfApplicable = false ) const;
    TInt32Fraction        AsInt32Fraction( const TInt32Fraction& defaultIfNeeded = TInt32Fraction(), bool resolveVarsIfApplicable = false ) const;
    TUInt32Fraction       AsUInt32Fraction( const TUInt32Fraction& defaultIfNeeded = TUInt32Fraction(), bool resolveVarsIfApplicable = false ) const;
    TInt64Fraction        AsInt64Fraction( const TInt64Fraction& defaultIfNeeded = TInt64Fraction(), bool resolveVarsIfApplicable = false ) const;
    TUInt64Fraction       AsUInt64Fraction( const TUInt64Fraction& defaultIfNeeded = TUInt64Fraction(), bool resolveVarsIfApplicable = false ) const;
    TInt64Int32Fraction   AsInt64Int32Fraction( const TInt64Int32Fraction& defaultIfNeeded = TInt64Int32Fraction(), bool resolveVarsIfApplicable = false ) const;
    TUInt64UInt32Fraction AsUInt64UInt32Fraction( const TUInt64UInt32Fraction& defaultIfNeeded = TUInt64UInt32Fraction(), bool resolveVarsIfApplicable = false ) const;

    template < typename TemplateBsobType >
    TemplateBsobType AsBsob( void ) const;

    template < typename TemplateBsobType >
    TemplateBsobType* AsBsobPtr( void );

    template < typename TemplateBsobType >
    const TemplateBsobType* AsBsobPtr( void ) const;

    template < typename T >
    T AsTValue( const T defaultIfNeeded = T(), bool resolveVarsIfApplicable = false ) const;

    /**
     *  Returns the size of the storage used in bytes by the stored type
     */
    UInt32 ByteSize( bool includeNullTerm = true ) const;
    static UInt32 ByteSizeOfFixedSizeType( UInt8 varType );

    bool operator==( const CVariant& other ) const;
    bool operator!=( const CVariant& other ) const;
    bool operator<( const CVariant& other ) const;
    bool operator>( const CVariant& other ) const;
    bool operator<=( const CVariant& other ) const;
    bool operator>=( const CVariant& other ) const;

    CVariant& operator=( bool    data );
    CVariant& operator=( Int8    data );
    CVariant& operator=( UInt8   data ); 
    CVariant& operator=( Int16   data );
    CVariant& operator=( UInt16  data );
    CVariant& operator=( Int32   data );
    CVariant& operator=( UInt32  data );
    CVariant& operator=( Int64   data );
    CVariant& operator=( UInt64  data );
    CVariant& operator=( Float32 data );
    CVariant& operator=( Float64 data );
    CVariant& operator=( TInt32T2Fraction data );
    CVariant& operator=( TUInt32T2Fraction data );
    CVariant& operator=( const TInt64WInt32Fraction& data );
    CVariant& operator=( const TUInt64WUInt32Fraction& data );
    CVariant& operator=( const TInt64T2Fraction& data );
    CVariant& operator=( const TUInt64T2Fraction& data );
    CVariant& operator=( const CDynamicBuffer& data );
    CVariant& operator=( const CAsciiString& data );
    CVariant& operator=( const CUtf8String& data );
    CVariant& operator=( const CDateTime& data );    
    CVariant& operator=( const std::string& data );
    CVariant& operator=( const std::wstring& data );
    CVariant& operator=( const CVariant& src );
    CVariant& operator=( const TVariantData& src );

    operator CAsciiString() const;
    operator CUtf8String() const;

    void Clear( void );

    /**
     *  Sets the variant data based on a raw pointer to the data type of type
     *  indicated by dataSize. Please take care that the buffer pointed to by data
     *  has enough bytes to hold the varType indicated or the operation will fail.
     */
    bool Set( const void* data, UInt32 dataSize, UInt8 varType = GUCEF_DATATYPE_BINARY_BLOB, bool linkOnlyForDynMem = false );

    bool Set( const TVariantData* src, bool linkOnlyForDynMem = false );

    bool SetString( UInt8 varType, const CString& data, const CVariant& defaultValue = CVariant::Empty );

    /**
     *  If you are setting a binary type using a string said string is assumed to be Base64 encoded
     */
    bool SetFromString( UInt8 varType, const CString& data, const CVariant& defaultValue = CVariant::Empty );

    const TVariantData* CStyleAccess( void ) const;

    CVariant& LinkTo( const CDynamicBuffer& src, UInt32 bufferOffset = 0, UInt8 varType = GUCEF_DATATYPE_BINARY_BLOB, UInt32 bytesToLink = 0 );
    CVariant& LinkTo( const void* externalBuffer, UInt32 bufferSize, UInt8 varType = GUCEF_DATATYPE_BINARY_BLOB );
    CVariant& LinkTo( const char* externalBuffer, UInt32 bufferSize, UInt8 varType = GUCEF_DATATYPE_ASCII_STRING );
    CVariant& LinkTo( const char* externalBuffer, UInt8 varType = GUCEF_DATATYPE_ASCII_STRING );
    CVariant& LinkTo( const CVariant& src );
    CVariant& LinkTo( const TVariantData* src );
    CVariant& LinkTo( const TVariantData& src );
    CVariant& LinkTo( const CAsciiString& src );
    CVariant& LinkTo( const CUtf8String& src );
    CVariant& LinkTo( const std::string& src );
    CVariant& LinkTo( const std::wstring& src );
        
    CVariant& TransferOwnershipTo( CVariant& newOwner );
    bool TransferOwnershipFrom( CDynamicBuffer& oldOwner );

    protected:

    void* HeapReserve( UInt32 byteSize, bool allowReduction = false );

    TVariantData m_variantData;
};

/*-------------------------------------------------------------------------*/

template < typename TemplateBsobType >
inline TemplateBsobType
CVariant::AsBsob( void ) const
{
    // Compile time check to ensure the type size does not exceed the size available for a BSOB
    // Failure to check for this would result in runtime access violations
    // If you get a compiler error here your type does not fit in a BSOD and you need to use 
    // alternate storage such as the BLOB type which uses the heap
    #if ( defined ( _MSC_VER ) && __cplusplus >= 199711L ) || ( __cplusplus >= 201103L )
    static_assert( sizeof( TemplateBsobType ) <= GUCEF_VARIANT_BSOB_SIZE, "Type size is invalid. Is not smaller or equal" );
    #else
    { char const CompileTimeTypeSizeSmallerEqualsCheck[ sizeof( TemplateBsobType ) <= GUCEF_VARIANT_BSOD_SIZE ? 1 : -1 ]; }
    #endif 
    
    return *( reinterpret_cast< const TemplateBsobType* >( m_variantData.union_data.bsob_data ) );
}

/*-------------------------------------------------------------------------*/

template < typename TemplateBsobType >
inline TemplateBsobType*
CVariant::AsBsobPtr( void )
{
    // Compile time check to ensure the type size does not exceed the size available for a BSOB
    // Failure to check for this would result in runtime access violations
    // If you get a compiler error here your type does not fit in a BSOD and you need to use 
    // alternate storage such as the BLOB type which uses the heap
    #if ( defined ( _MSC_VER ) && __cplusplus >= 199711L ) || ( __cplusplus >= 201103L )
    static_assert( sizeof( TemplateBsobType ) <= GUCEF_VARIANT_BSOB_SIZE, "Type size is invalid. Is not smaller or equal" );
    #else
    { char const CompileTimeTypeSizeSmallerEqualsCheck[ sizeof( TemplateBsobType ) <= GUCEF_VARIANT_BSOD_SIZE ? 1 : -1 ]; }
    #endif 

    return ( reinterpret_cast< TemplateBsobType* >( m_variantData.union_data.bsob_data ) );
}

/*-------------------------------------------------------------------------*/

template < typename TemplateBsobType >
inline const TemplateBsobType*
CVariant::AsBsobPtr( void ) const
{
    // Compile time check to ensure the type size does not exceed the size available for a BSOB
    // Failure to check for this would result in runtime access violations
    // If you get a compiler error here your type does not fit in a BSOD and you need to use 
    // alternate storage such as the BLOB type which uses the heap
    #if ( defined ( _MSC_VER ) && __cplusplus >= 199711L ) || ( __cplusplus >= 201103L )
    static_assert( sizeof( TemplateBsobType ) <= GUCEF_VARIANT_BSOB_SIZE, "Type size is invalid. Is not smaller or equal" );
    #else
    { char const CompileTimeTypeSizeSmallerEqualsCheck[ sizeof( TemplateBsobType ) <= GUCEF_VARIANT_BSOD_SIZE ? 1 : -1 ]; }
    #endif 

    return ( reinterpret_cast< const TemplateBsobType* >( m_variantData.union_data.bsob_data ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline T
CVariant::AsTValue( const T defaultIfNeeded, bool resolveVarsIfApplicable ) const 
{GUCEF_TRACE;

    // All the explicit template specializations should take care of making sure you never actually
    // end up here. If for some reason you do, the best we can offer is the 'defaultIfNeeded'
    return defaultIfNeeded;
}

/*-------------------------------------------------------------------------*/

template <> inline CVariant CVariant::AsTValue( const CVariant defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return *this; }
template <> inline bool CVariant::AsTValue( const bool defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsBool( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline Int8 CVariant::AsTValue( const Int8 defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsInt8( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline UInt8 CVariant::AsTValue( const UInt8 defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsUInt8( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline Int16 CVariant::AsTValue( const Int16 defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsInt16( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline UInt16 CVariant::AsTValue( const UInt16 defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsUInt16( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline Int32 CVariant::AsTValue( const Int32 defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsInt32( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline UInt32 CVariant::AsTValue( const UInt32 defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsUInt32( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline Int64 CVariant::AsTValue( const Int64 defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsInt64( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline UInt64 CVariant::AsTValue( const UInt64 defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsUInt64( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline Float32 CVariant::AsTValue( const Float32 defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsFloat32( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline Float64 CVariant::AsTValue( const Float64 defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsFloat64( defaultIfNeeded, resolveVarsIfApplicable ); }
//template <> inline size_t CVariant::AsTValue( const size_t defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsSizeT( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline CAsciiString CVariant::AsTValue( const CAsciiString defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsAsciiString( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline CUtf8String CVariant::AsTValue( const CUtf8String defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsUtf8String( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline CDateTime CVariant::AsTValue( const CDateTime defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsDateTime( defaultIfNeeded, resolveVarsIfApplicable ); }
template <> inline std::string CVariant::AsTValue( const std::string defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsString( defaultIfNeeded, resolveVarsIfApplicable ).STL_String(); }
template <> inline std::wstring CVariant::AsTValue( const std::wstring defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return ToWString( AsString( ToString( defaultIfNeeded ), resolveVarsIfApplicable ) ); }
//template <> inline void* CVariant::AsTValue( const void* defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsVoidPtr( defaultIfNeeded ), resolveVarsIfApplicable ) ); }
template <> inline TDefaultFuncPtr CVariant::AsTValue( const TDefaultFuncPtr defaultIfNeeded, bool resolveVarsIfApplicable ) const {GUCEF_TRACE; return AsDefaultFuncPtr( defaultIfNeeded ); }

/*-------------------------------------------------------------------------*/

inline CString ToString( const CVariant& var ) { return var.AsString(); }

/*-------------------------------------------------------------------------*/

GUCEF_CORE_PUBLIC_CPP CString::StringVector ToStringVector( const CVariant::VariantVector& var );
GUCEF_CORE_PUBLIC_CPP CString::StringVector ToStringVector( const CVariant::VariantSet& var );
GUCEF_CORE_PUBLIC_CPP CString::StringSet ToStringSet( const CVariant::VariantVector& var );
GUCEF_CORE_PUBLIC_CPP CString::StringSet ToStringSet( const CVariant::VariantSet& var );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CVARIANT_H ? */
