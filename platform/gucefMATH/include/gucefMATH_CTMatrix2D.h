/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_MATH_CTMATRIX2D_H
#define GUCEF_MATH_CTMATRIX2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_MATH_CTNUMERICCONVERTER_H
#include "gucefMATH_CTNumericConverter.h"
#define GUCEF_MATH_CTNUMERICCONVERTER_H
#endif /* GUCEF_MATH_CTNUMERICCONVERTER_H ? */

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

#ifndef GUCEF_MATH_COMPONENTFUNCTIONS_H
#include "gucefMATH_ComponentFunctions.h"
#define GUCEF_MATH_COMPONENTFUNCTIONS_H
#endif /* GUCEF_MATH_COMPONENTFUNCTIONS_H ? */

#ifndef GUCEF_MATH_CTVECTOR2D_H
#include "gucefMATH_CTVector2D.h"
#define GUCEF_MATH_CTVECTOR2D_H
#endif /* GUCEF_MATH_CTVECTOR2D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Exact scaling by powers of two, used by the matrix inverses to equilibrate rows (section 11.1.1 item 7): every row
 *  is scaled by the power of two that brings its largest magnitude into [0.5, 1), so determinants and cofactors stay in
 *  range and keep their precision (fixed point in particular), and the singularity test is relative.
 *  - NormalizingExponent( m ): k with m * 2^k in [0.5, 1); 0 for zero, non-finite values and the exact kinds.
 *  - Scale( x, k ): x * 2^k, exact unless it overflows or underflows (fixed point saturates, and rounds half away from
 *    zero when shifting right). The exact kinds (fractions, integers) are not scaled.
 *  Realtime-safe.
 */
template < typename T, int Kind >
class CTMathBinaryScaling
{
    public:

    static GUCEF_MATH_FORCE_INLINE Int32 NormalizingExponent( const T& /*magnitude*/ ) GUCEF_MATH_REALTIME_CONTEXT { return 0; }
    static GUCEF_MATH_FORCE_INLINE T Scale( const T& x, Int32 /*exponent*/ ) GUCEF_MATH_REALTIME_CONTEXT { return x; }
};

template < typename T >
class CTMathBinaryScaling< T, MATHCOMPONENTKIND_FLOAT >
{
    public:

    static GUCEF_MATH_FORCE_INLINE Int32 NormalizingExponent( const T& magnitude ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( !( T( 0 ) < magnitude ) || !CTNumericTraits< T >::IsFinite( magnitude ) )
            return 0;
        int exponent = 0;
        frexp( (Float64) magnitude, &exponent );
        return -(Int32) exponent;
    }

    static GUCEF_MATH_FORCE_INLINE T Scale( const T& x, Int32 exponent ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return 0 == exponent ? x : (T) ldexp( (Float64) x, (int) exponent );
    }
};

template < typename TStorage, UInt32 F >
class CTMathBinaryScaling< CTFixedPoint< TStorage, F >, MATHCOMPONENTKIND_FIXED >
{
    public:

    typedef CTFixedPoint< TStorage, F > T;
    enum { StorageBits = sizeof( TStorage ) * 8 };

    static Int32 NormalizingExponent( const T& magnitude ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt64 raw = MathMagnitude( (Int64) magnitude.GetRaw() );
        if ( 0 == raw )
            return 0;
        Int32 highestBit = 0;
        while ( 0 != ( raw >> 1 ) )
        {
            raw >>= 1;
            ++highestBit;
        }
        // raw * 2^k in [ 2^( F - 1 ), 2^F )
        return (Int32) F - 1 - highestBit;
    }

    static T Scale( const T& x, Int32 exponent ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( 0 == exponent )
            return x;
        const bool negative = x.GetRaw() < 0;
        const UInt64 magnitude = MathMagnitude( (Int64) x.GetRaw() );
        const UInt64 limit = ( ( (UInt64) 1 << ( StorageBits - 1 ) ) - 1 ) + ( negative ? 1 : 0 );
        UInt64 result = 0;
        if ( exponent > 0 )
        {
            if ( 0 != magnitude && ( exponent >= 63 || magnitude > ( limit >> exponent ) ) )
                return negative ? T::Lowest() : T::Highest();
            result = magnitude << exponent;
        }
        else
        {
            const Int32 shift = -exponent;
            if ( shift > 64 )
                return T::FromRaw( 0 );
            const UInt64 half = shift >= 64 ? ( (UInt64) 1 << 63 ) : ( (UInt64) 1 << ( shift - 1 ) );
            result = shift >= 64 ? ( magnitude >= half ? 1 : 0 ) : ( magnitude >> shift ) + ( 0 != ( magnitude & half ) ? 1 : 0 );
        }
        return T::FromRaw( (TStorage) MathApplySign( negative, result ) );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Text and DataNode forms of the matrix family (sections 9.1 and 9.2 of the design), over the elements in row order
 *  so that neither form depends on the storage order:
 *  - text "[[m00, m01, ...], [m10, ...], ...]", each row parsed with ComponentsFromString (white space allowed);
 *  - DataNode attributes m<row><column> (m00 ... m33) through SerializeComponents / DeserializeComponents.
 *  Reads go into a temporary: false leaves the output unchanged. Not realtime-safe (they allocate).
 */
template < typename T, UInt32 Rows, UInt32 Columns >
class CTMatrixComponentsCodec
{
    public:

    enum { Count = Rows * Columns };

    static CORE::CString ToString( const T ( &rowMajor )[ Count ] )
    {
        CORE::CString text( "[" );
        for ( UInt32 r=0; r<Rows; ++r )
        {
            if ( r > 0 )
                text += ", ";
            text += '[';
            for ( UInt32 c=0; c<Columns; ++c )
            {
                if ( c > 0 )
                    text += ", ";
                text += CTNumericTraits< T >::ToString( rowMajor[ r * Columns + c ] );
            }
            text += ']';
        }
        text += ']';
        return text;
    }

    static bool FromString( const CORE::CString& text, T ( &rowMajor )[ Count ] )
    {
        const char* data = text.C_String();
        UInt32 length = text.Length();
        if ( GUCEF_NULL == data )
            return false;
        UInt32 position = SkipWhitespace( data, length, 0 );
        if ( position >= length || '[' != data[ position ] )
            return false;
        position = SkipWhitespace( data, length, position + 1 );

        T parsed[ Count ];
        for ( UInt32 r=0; r<Rows; ++r )
        {
            if ( r > 0 )
            {
                if ( position >= length || ',' != data[ position ] )
                    return false;
                position = SkipWhitespace( data, length, position + 1 );
            }
            if ( position >= length || '[' != data[ position ] )
                return false;
            UInt32 end = position + 1;
            while ( end < length && ']' != data[ end ] && '[' != data[ end ] )
                ++end;
            if ( end >= length || ']' != data[ end ] )
                return false;
            // the row "[a, b, ...]" in the component form "(a, b, ...)"
            CORE::CString row( "(" );
            row += CORE::CString( data + position + 1, end - position - 1 );
            row += ')';
            T values[ Columns ];
            if ( !ComponentsFromString( row, values ) )
                return false;
            for ( UInt32 c=0; c<Columns; ++c )
                parsed[ r * Columns + c ] = values[ c ];
            position = SkipWhitespace( data, length, end + 1 );
        }
        if ( position >= length || ']' != data[ position ] )
            return false;
        if ( SkipWhitespace( data, length, position + 1 ) != length )
            return false;
        for ( UInt32 i=0; i<Count; ++i )
            rowMajor[ i ] = parsed[ i ];
        return true;
    }

    static bool Serialize( CORE::CDataNode& node, const T ( &rowMajor )[ Count ] )
    {
        char storage[ Count ][ 4 ];
        const char* names[ Count ];
        MakeNames( storage, names );
        return SerializeComponents( node, names, rowMajor );
    }

    static bool Deserialize( const CORE::CDataNode& node, T ( &rowMajor )[ Count ] )
    {
        char storage[ Count ][ 4 ];
        const char* names[ Count ];
        MakeNames( storage, names );
        return DeserializeComponents( node, names, rowMajor );
    }

    private:

    static UInt32 SkipWhitespace( const char* data, UInt32 length, UInt32 position )
    {
        while ( position < length && IsComponentTextWhitespace( data[ position ] ) )
            ++position;
        return position;
    }

    /** "m<row><column>" for every element in row order */
    static void MakeNames( char ( &storage )[ Count ][ 4 ], const char* ( &names )[ Count ] )
    {
        for ( UInt32 r=0; r<Rows; ++r )
        {
            for ( UInt32 c=0; c<Columns; ++c )
            {
                char* name = storage[ r * Columns + c ];
                name[ 0 ] = 'm';
                name[ 1 ] = (char) ( '0' + r );
                name[ 2 ] = (char) ( '0' + c );
                name[ 3 ] = '\0';
                names[ r * Columns + c ] = name;
            }
        }
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The text form of an angle of the rotation family: "<radians> rad" (section 9.1 of the design). Not realtime-safe.
 */
template < typename T >
CORE::CString
MathAngleToString( const CTRadian< T >& angle )
{
    CORE::CString text( CTNumericTraits< T >::ToString( angle.GetValue() ) );
    text += " rad";
    return text;
}

/*-------------------------------------------------------------------------*/

/**
 *  Parses "<value> rad" or "<value> deg" (white space allowed around both parts); false leaves 'angle' unchanged.
 *  Not realtime-safe.
 */
template < typename T >
bool
MathAngleFromString( const CORE::CString& text, CTRadian< T >& angle )
{
    const char* data = text.C_String();
    UInt32 length = text.Length();
    if ( GUCEF_NULL == data )
        return false;
    UInt32 first = 0;
    UInt32 last = length;
    while ( first < last && IsComponentTextWhitespace( data[ first ] ) )
        ++first;
    while ( last > first && IsComponentTextWhitespace( data[ last - 1 ] ) )
        --last;
    if ( last - first < 4 )
        return false;
    CORE::CString unit( data + last - 3, 3 );
    bool degrees = false;
    if ( unit == "deg" )
        degrees = true;
    else if ( !( unit == "rad" ) )
        return false;
    UInt32 numberEnd = last - 3;
    while ( numberEnd > first && IsComponentTextWhitespace( data[ numberEnd - 1 ] ) )
        --numberEnd;
    if ( numberEnd == first )
        return false;
    T value = CTNumericTraits< T >::Zero();
    if ( !CTNumericTraits< T >::FromString( CORE::CString( data + first, numberEnd - first ), value ) )
        return false;
    angle = degrees ? CTRadian< T >( CTDegree< T >( value ) ) : CTRadian< T >( value );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Writes an angle as the attribute 'name' in radians. Not realtime-safe */
template < typename T >
bool
MathAngleSerialize( CORE::CDataNode& node, const char* name, const CTRadian< T >& angle )
{
    return node.SetAttribute( CORE::CString( name ), ComponentToVariant( angle.GetValue() ), false );
}

/*-------------------------------------------------------------------------*/

/**
 *  Reads an angle from the attribute 'name' (radians) or, when that is missing, from 'degreesName' (degrees, may be
 *  null); false leaves 'angle' unchanged. Not realtime-safe.
 */
template < typename T >
bool
MathAngleDeserialize( const CORE::CDataNode& node, const char* name, const char* degreesName, CTRadian< T >& angle )
{
    T value = CTNumericTraits< T >::Zero();
    const CORE::CDataNode::TKeyValuePair* attribute = node.GetAttribute( name );
    if ( GUCEF_NULL != attribute )
    {
        if ( !ComponentFromVariant( attribute->second, value ) )
            return false;
        angle = CTRadian< T >( value );
        return true;
    }
    if ( GUCEF_NULL == degreesName )
        return false;
    attribute = node.GetAttribute( degreesName );
    if ( GUCEF_NULL == attribute || !ComponentFromVariant( attribute->second, value ) )
        return false;
    angle = CTRadian< T >( CTDegree< T >( value ) );
    return true;
}

/**
 *  Splits the inside of "( part, part, ... )" at the commas that are not nested in parentheses or brackets; each part
 *  is trimmed of ASCII white space. False when the text is not one parenthesised group, a part is empty, or there are
 *  not exactly 'expected' parts. Used by the composite text forms of the rotation and transform family. Not
 *  realtime-safe.
 */
inline bool
MathSplitParenthesisedList( const CORE::CString& text, CORE::CString* parts, UInt32 expected )
{
    const char* data = text.C_String();
    UInt32 length = text.Length();
    if ( GUCEF_NULL == data )
        return false;
    UInt32 first = 0;
    UInt32 last = length;
    while ( first < last && IsComponentTextWhitespace( data[ first ] ) )
        ++first;
    while ( last > first && IsComponentTextWhitespace( data[ last - 1 ] ) )
        --last;
    if ( last - first < 2 || '(' != data[ first ] || ')' != data[ last - 1 ] )
        return false;
    UInt32 count = 0;
    Int32 depth = 0;
    UInt32 partStart = first + 1;
    for ( UInt32 i=first + 1; i<=last - 1; ++i )
    {
        const char c = data[ i ];
        const bool end = ( i == last - 1 );
        if ( !end && ( '(' == c || '[' == c ) )
            ++depth;
        else if ( !end && ( ')' == c || ']' == c ) )
        {
            if ( --depth < 0 )
                return false;
        }
        else if ( end || ( ',' == c && 0 == depth ) )
        {
            if ( end && 0 != depth )
                return false;
            UInt32 a = partStart;
            UInt32 b = i;
            while ( a < b && IsComponentTextWhitespace( data[ a ] ) )
                ++a;
            while ( b > a && IsComponentTextWhitespace( data[ b - 1 ] ) )
                --b;
            if ( a == b || count >= expected )
                return false;
            parts[ count++ ] = CORE::CString( data + a, b - a );
            partStart = i + 1;
        }
    }
    return count == expected;
}

/*-------------------------------------------------------------------------*/

/**
 *  Reads "<label>( ... )" at 'position' (after optional white space): 'group' receives the parenthesised text including
 *  the parentheses and 'position' moves past it. False, with 'position' unchanged, when the label or a balanced group
 *  does not follow. Not realtime-safe.
 */
inline bool
MathTakeLabeledGroup( const CORE::CString& text, UInt32& position, const char* label, CORE::CString& group )
{
    const char* data = text.C_String();
    UInt32 length = text.Length();
    if ( GUCEF_NULL == data || GUCEF_NULL == label )
        return false;
    UInt32 i = position;
    while ( i < length && IsComponentTextWhitespace( data[ i ] ) )
        ++i;
    for ( const char* l = label; '\0' != *l; ++l, ++i )
        if ( i >= length || data[ i ] != *l )
            return false;
    while ( i < length && IsComponentTextWhitespace( data[ i ] ) )
        ++i;
    if ( i >= length || '(' != data[ i ] )
        return false;
    UInt32 start = i;
    Int32 depth = 0;
    for ( ; i<length; ++i )
    {
        if ( '(' == data[ i ] )
            ++depth;
        else if ( ')' == data[ i ] && 0 == --depth )
            break;
    }
    if ( i >= length )
        return false;
    group = CORE::CString( data + start, i + 1 - start );
    position = i + 1;
    return true;
}

/*-------------------------------------------------------------------------*/

/** True when only ASCII white space follows 'position' */
inline bool
MathIsTextEnd( const CORE::CString& text, UInt32 position )
{
    const char* data = text.C_String();
    UInt32 length = text.Length();
    while ( position < length && IsComponentTextWhitespace( data[ position ] ) )
        ++position;
    return position >= length;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A 2x2 matrix: the linear maps of the plane (rotation about +Z counter-clockwise positive, scale), column vectors.
 *
 *  - Kind: T is any numeric kind of CTNumericTraits. Inversion needs division; Rotation needs transcendentals.
 *  - Layout: column-major T m_data[ 4 ] (element (r, c) at c * 2 + r), private because the storage order must not leak
 *    into user code; access through operator()( row, column ), GetColumn, GetRow, GetData (column-major), ToRowMajor.
 *    Size 4 * sizeof( T ), alignment that of T, no virtual functions.
 *  - Products: every element of a product is a dot product of the component kernels (integers wrap, fixed point
 *    rounds once from the exact sum, floats in component order).
 *  - Equality: == exact; Equals with tolerances per element.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "[[m00, m01], [m10, m11]]" (row order); DataNode attributes m00, m01, m10, m11.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTMatrix2D
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;
    typedef typename CTMathComponentTraits< T >::TKernels TKernels;
    typedef CTVector2D< T > TVector;

    /** The identity */
    GUCEF_MATH_FORCE_INLINE CTMatrix2D( void )
    {
        m_data[ 0 ] = TTraits::One();  m_data[ 1 ] = TTraits::Zero();
        m_data[ 2 ] = TTraits::Zero(); m_data[ 3 ] = TTraits::One();
    }

    /** Converts every element with CTNumericConverter (half to even, saturating; NaN into an exact kind gives 0) */
    template < typename U >
    explicit CTMatrix2D( const CTMatrix2D< U >& other )
    {
        TMathStatus status = MATHSTATUS_OK;
        for ( UInt32 i=0; i<4; ++i )
        {
            m_data[ i ] = TTraits::Zero();
            status |= CTNumericConverter< U, T >::Convert( other.GetData()[ i ], m_data[ i ], CORE::ROUNDINGMODE_HALF_EVEN );
        }
        GUCEF_MATH_ASSERT( 0 == ( status & MATHSTATUS_INVALID_OPERAND ) );
        (void) status;
    }

    static GUCEF_MATH_FORCE_INLINE CTMatrix2D Identity( void ) GUCEF_MATH_REALTIME_CONTEXT { return CTMatrix2D(); }

    static GUCEF_MATH_FORCE_INLINE CTMatrix2D Zero( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix2D m;
        m.m_data[ 0 ] = TTraits::Zero();
        m.m_data[ 3 ] = TTraits::Zero();
        return m;
    }

    static GUCEF_MATH_FORCE_INLINE CTMatrix2D FromColumns( const TVector& c0, const TVector& c1 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix2D m;
        m.m_data[ 0 ] = c0.x; m.m_data[ 1 ] = c0.y;
        m.m_data[ 2 ] = c1.x; m.m_data[ 3 ] = c1.y;
        return m;
    }

    /** From 4 values in column-major order (the GetData order) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix2D FromColumnMajor( const T* values4 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix2D m;
        for ( UInt32 i=0; i<4; ++i )
            m.m_data[ i ] = values4[ i ];
        return m;
    }

    /** From 4 values in row-major order (m00, m01, m10, m11) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix2D FromRowMajor( const T* values4 ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix2D m;
        m.m_data[ 0 ] = values4[ 0 ]; m.m_data[ 2 ] = values4[ 1 ];
        m.m_data[ 1 ] = values4[ 2 ]; m.m_data[ 3 ] = values4[ 3 ];
        return m;
    }

    /** Counter-clockwise rotation by 'angle' (HasTranscendentals): Rotation( 90 deg ) * UnitX == UnitY */
    static CTMatrix2D Rotation( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** diag( s.x, s.y ) */
    static GUCEF_MATH_FORCE_INLINE CTMatrix2D Scale( const TVector& s ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix2D m;
        m.m_data[ 0 ] = s.x;
        m.m_data[ 3 ] = s.y;
        return m;
    }

    /*---------------------------------------------------------------------*/
    /*  Access                                                              */
    /*---------------------------------------------------------------------*/

    /** Element (row, column); an index out of range asserts in debug builds and is clamped */
    GUCEF_MATH_FORCE_INLINE T operator()( UInt32 row, UInt32 column ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( row < 2 && column < 2 );
        return m_data[ Index( row, column ) ];
    }

    GUCEF_MATH_FORCE_INLINE T& operator()( UInt32 row, UInt32 column ) GUCEF_MATH_REALTIME_CONTEXT
    {
        GUCEF_MATH_ASSERT( row < 2 && column < 2 );
        return m_data[ Index( row, column ) ];
    }

    GUCEF_MATH_FORCE_INLINE TVector GetColumn( UInt32 c ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector( ( *this )( 0, c ), ( *this )( 1, c ) );
    }

    GUCEF_MATH_FORCE_INLINE void SetColumn( UInt32 c, const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        ( *this )( 0, c ) = v.x;
        ( *this )( 1, c ) = v.y;
    }

    GUCEF_MATH_FORCE_INLINE TVector GetRow( UInt32 r ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TVector( ( *this )( r, 0 ), ( *this )( r, 1 ) );
    }

    /** The 4 elements in column-major order */
    GUCEF_MATH_FORCE_INLINE const T* GetData( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_data; }

    /** Writes the 4 elements in row-major order */
    GUCEF_MATH_FORCE_INLINE void ToRowMajor( T* out4 ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        out4[ 0 ] = m_data[ 0 ]; out4[ 1 ] = m_data[ 2 ];
        out4[ 2 ] = m_data[ 1 ]; out4[ 3 ] = m_data[ 3 ];
    }

    /*---------------------------------------------------------------------*/
    /*  Algebra                                                             */
    /*---------------------------------------------------------------------*/

    friend GUCEF_MATH_FORCE_INLINE CTMatrix2D operator+( const CTMatrix2D& a, const CTMatrix2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix2D m;
        for ( UInt32 i=0; i<4; ++i )
            m.m_data[ i ] = TKernels::Add( a.m_data[ i ], b.m_data[ i ] );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix2D operator-( const CTMatrix2D& a, const CTMatrix2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix2D m;
        for ( UInt32 i=0; i<4; ++i )
            m.m_data[ i ] = TKernels::Subtract( a.m_data[ i ], b.m_data[ i ] );
        return m;
    }

    GUCEF_MATH_FORCE_INLINE CTMatrix2D operator-( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix2D m;
        for ( UInt32 i=0; i<4; ++i )
            m.m_data[ i ] = TKernels::Negate( m_data[ i ] );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix2D operator*( const CTMatrix2D& a, const T& factor ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix2D m;
        for ( UInt32 i=0; i<4; ++i )
            m.m_data[ i ] = TKernels::Multiply( a.m_data[ i ], factor );
        return m;
    }

    friend GUCEF_MATH_FORCE_INLINE CTMatrix2D operator*( const T& factor, const CTMatrix2D& a ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a * factor;
    }

    /** a * b: applies b first */
    friend GUCEF_MATH_FORCE_INLINE CTMatrix2D operator*( const CTMatrix2D& a, const CTMatrix2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.Multiply( b );
    }

    friend GUCEF_MATH_FORCE_INLINE TVector operator*( const CTMatrix2D& m, const TVector& v ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return m.Transform( v );
    }

    GUCEF_MATH_FORCE_INLINE CTMatrix2D& operator*=( const CTMatrix2D& other ) GUCEF_MATH_REALTIME_CONTEXT { *this = Multiply( other ); return *this; }

    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTMatrix2D& a, const CTMatrix2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<4; ++i )
            if ( !( a.m_data[ i ] == b.m_data[ i ] ) )
                return false;
        return true;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTMatrix2D& a, const CTMatrix2D& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /** this * other (other applied first); every element a 2 term dot product of the kind */
    CTMatrix2D Multiply( const CTMatrix2D& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** this * v */
    TVector Transform( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT;

    GUCEF_MATH_FORCE_INLINE CTMatrix2D Transposed( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        CTMatrix2D m( *this );
        m.m_data[ 1 ] = m_data[ 2 ];
        m.m_data[ 2 ] = m_data[ 1 ];
        return m;
    }

    /** m00 m11 - m01 m10 (one rounding in fixed point; integers wrap) */
    T GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /**
     *  The inverse (HasDivision) of the equilibrated matrix E = Dr M Dc: each row, then each column, is scaled by the
     *  power of two that brings its largest magnitude into [0.5, 1) (exact; it keeps fixed point precise at small and
     *  large scales). False, 'out' unchanged, for a zero row or column, or when det( E ) is not finite or
     *  |det( E )| <= singularTolerance (>= 0): a relative test, so scaling a row or a column does not change the outcome.
     */
    bool TryInverse( CTMatrix2D& out, const T& singularTolerance ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The inverse (HasDivision); the zero matrix and a debug assertion when the determinant is zero */
    CTMatrix2D Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Queries and tolerant comparison                                     */
    /*---------------------------------------------------------------------*/

    GUCEF_MATH_FORCE_INLINE bool IsIdentity( void ) const GUCEF_MATH_REALTIME_CONTEXT { return *this == CTMatrix2D(); }

    GUCEF_MATH_FORCE_INLINE bool IsFinite( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<4; ++i )
            if ( !TTraits::IsFinite( m_data[ i ] ) )
                return false;
        return true;
    }

    /** Every element equal within the default tolerances of the kind */
    GUCEF_MATH_FORCE_INLINE bool Equals( const CTMatrix2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return Equals( other, TTraits::DefaultAbsoluteTolerance(), TTraits::DefaultRelativeTolerance() );
    }

    /** Every element: |a - b| <= max( absolute, relative * max( |a|, |b| ) ) */
    bool Equals( const CTMatrix2D& other, const T& absolute, const T& relative ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        for ( UInt32 i=0; i<4; ++i )
            if ( !TTraits::EqualsWithin( m_data[ i ], other.m_data[ i ], absolute, relative ) )
                return false;
        return true;
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "[[m00, m01], [m10, m11]]" */
    CORE::CString ToString( void ) const
    {
        T rows[ 4 ];
        ToRowMajor( rows );
        return CTMatrixComponentsCodec< T, 2, 2 >::ToString( rows );
    }

    /** Parses the ToString form; false leaves this matrix unchanged */
    bool FromString( const CORE::CString& text )
    {
        T rows[ 4 ];
        if ( !CTMatrixComponentsCodec< T, 2, 2 >::FromString( text, rows ) )
            return false;
        *this = FromRowMajor( rows );
        return true;
    }

    /** Writes the attributes m00, m01, m10, m11 */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
    {
        T rows[ 4 ];
        ToRowMajor( rows );
        return CTMatrixComponentsCodec< T, 2, 2 >::Serialize( node, rows );
    }

    /** Reads the attributes m00 ... m11; a missing or invalid attribute fails and leaves this matrix unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
    {
        T rows[ 4 ];
        if ( !CTMatrixComponentsCodec< T, 2, 2 >::Deserialize( node, rows ) )
            return false;
        *this = FromRowMajor( rows );
        return true;
    }

    private:

    static GUCEF_MATH_FORCE_INLINE UInt32 Index( UInt32 row, UInt32 column ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return ( column < 2 ? column : 1 ) * 2 + ( row < 2 ? row : 1 );
    }

    T m_data[ 4 ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members, out of the class: with GUCEF_MATH_PRECISE_INLINE (empty by default) they are not
 *  inline, so for the predefined kinds the copy compiled in the module (without floating-point contraction) runs.
 */

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix2D< T >
CTMatrix2D< T >::Rotation( const CTRadian< T >& angle ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasTranscendentals, Rotation_requires_transcendentals );
    T c = TTraits::Cos( angle.GetValue() );
    T s = TTraits::Sin( angle.GetValue() );
    CTMatrix2D m;
    m.m_data[ 0 ] = c; m.m_data[ 2 ] = ComponentNegate( s );
    m.m_data[ 1 ] = s; m.m_data[ 3 ] = c;
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix2D< T >
CTMatrix2D< T >::Multiply( const CTMatrix2D& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTMatrix2D m;
    for ( UInt32 c=0; c<2; ++c )
    {
        const T column[ 2 ] = { other.m_data[ c * 2 ], other.m_data[ c * 2 + 1 ] };
        for ( UInt32 r=0; r<2; ++r )
        {
            const T row[ 2 ] = { m_data[ r ], m_data[ 2 + r ] };
            m.m_data[ c * 2 + r ] = TKernels::Dot( row, column );
        }
    }
    return m;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE typename CTMatrix2D< T >::TVector
CTMatrix2D< T >::Transform( const TVector& v ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T b[ 2 ] = { v.x, v.y };
    const T row0[ 2 ] = { m_data[ 0 ], m_data[ 2 ] };
    const T row1[ 2 ] = { m_data[ 1 ], m_data[ 3 ] };
    return TVector( TKernels::Dot( row0, b ), TKernels::Dot( row1, b ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTMatrix2D< T >::GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    const T a[ 2 ] = { m_data[ 0 ], ComponentNegate( m_data[ 2 ] ) };
    const T b[ 2 ] = { m_data[ 3 ], m_data[ 1 ] };
    return TKernels::Dot( a, b );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE bool
CTMatrix2D< T >::TryInverse( CTMatrix2D& out, const T& singularTolerance ) const GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_STATIC_ASSERT( TTraits::HasDivision, Inverse_requires_a_numeric_kind_with_division );
    typedef CTMathBinaryScaling< T, CTMathComponentKind< T >::Value > TScaling;
    // rows equilibrated by powers of two
    Int32 exponent[ 2 ];
    T e[ 4 ];
    for ( UInt32 r=0; r<2; ++r )
    {
        const T largest = TTraits::Max( TTraits::Abs( m_data[ r ] ), TTraits::Abs( m_data[ 2 + r ] ) );
        if ( !( TTraits::Zero() < largest ) )
            return false;
        exponent[ r ] = TScaling::NormalizingExponent( largest );
        e[ r ] = TScaling::Scale( m_data[ r ], exponent[ r ] );
        e[ 2 + r ] = TScaling::Scale( m_data[ 2 + r ], exponent[ r ] );
    }
    // then the columns: E = Dr M Dc, M^-1 = Dc E^-1 Dr
    Int32 columnExponent[ 2 ];
    for ( UInt32 c=0; c<2; ++c )
    {
        const T largest = TTraits::Max( TTraits::Abs( e[ c * 2 ] ), TTraits::Abs( e[ c * 2 + 1 ] ) );
        if ( !( TTraits::Zero() < largest ) )
            return false;
        columnExponent[ c ] = TScaling::NormalizingExponent( largest );
        e[ c * 2 ] = TScaling::Scale( e[ c * 2 ], columnExponent[ c ] );
        e[ c * 2 + 1 ] = TScaling::Scale( e[ c * 2 + 1 ], columnExponent[ c ] );
    }
    const T a[ 2 ] = { e[ 0 ], ComponentNegate( e[ 2 ] ) };
    const T b[ 2 ] = { e[ 3 ], e[ 1 ] };
    const T determinant = TKernels::Dot( a, b );
    if ( !TTraits::IsFinite( determinant ) || !( singularTolerance < TTraits::Abs( determinant ) ) )
        return false;
    CTMatrix2D m;
    // element (r, c) of M^-1 is E^-1( r, c ) scaled by 2^( column exponent r + row exponent c )
    m.m_data[ 0 ] = TScaling::Scale( e[ 3 ] / determinant, columnExponent[ 0 ] + exponent[ 0 ] );
    m.m_data[ 1 ] = TScaling::Scale( ComponentNegate( e[ 1 ] ) / determinant, columnExponent[ 1 ] + exponent[ 0 ] );
    m.m_data[ 2 ] = TScaling::Scale( ComponentNegate( e[ 2 ] ) / determinant, columnExponent[ 0 ] + exponent[ 1 ] );
    m.m_data[ 3 ] = TScaling::Scale( e[ 0 ] / determinant, columnExponent[ 1 ] + exponent[ 1 ] );
    out = m;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix2D< T >
CTMatrix2D< T >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    CTMatrix2D m = Zero();
    bool invertible = TryInverse( m, TTraits::Zero() );
    GUCEF_MATH_ASSERT( invertible );
    (void) invertible;
    return m;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive members of the predefined kinds, member by member (section 5.10 of the design; see
 *  gucefMATH_CTVector3D.h for the scheme): ALL_KINDS, DIVISION and TRANSCENDENTAL groups, a list macro per kind family.
 */

#define GUCEF_MATH_CTMATRIX2D_ALL_KINDS_MEMBERS( prefix, kind ) \
    prefix CTMatrix2D< kind > CTMatrix2D< kind >::Multiply( const CTMatrix2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTVector2D< kind > CTMatrix2D< kind >::Transform( const CTVector2D< kind >& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix kind CTMatrix2D< kind >::GetDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIX2D_DIVISION_MEMBERS( prefix, kind ) \
    prefix bool CTMatrix2D< kind >::TryInverse( CTMatrix2D< kind >&, const kind& ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix2D< kind > CTMatrix2D< kind >::Inverse( void ) const GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIX2D_TRANSCENDENTAL_MEMBERS( prefix, kind ) \
    prefix CTMatrix2D< kind > CTMatrix2D< kind >::Rotation( const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTMATRIX2D_REAL_KIND( prefix, kind ) GUCEF_MATH_CTMATRIX2D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTMATRIX2D_DIVISION_MEMBERS( prefix, kind ) GUCEF_MATH_CTMATRIX2D_TRANSCENDENTAL_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTMATRIX2D_INTEGER_KIND( prefix, kind ) GUCEF_MATH_CTMATRIX2D_ALL_KINDS_MEMBERS( prefix, kind )
#define GUCEF_MATH_CTMATRIX2D_EXACT_KIND( prefix, kind ) GUCEF_MATH_CTMATRIX2D_ALL_KINDS_MEMBERS( prefix, kind ) GUCEF_MATH_CTMATRIX2D_DIVISION_MEMBERS( prefix, kind )

/** Every predefined kind except the fractions (those are in gucefMATH_fractiontypes.h, which includes CBigInteger) */
#define GUCEF_MATH_CTMATRIX2D_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTMATRIX2D_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTMATRIX2D_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTMATRIX2D_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTMATRIX2D_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTMATRIX2D_REAL_KIND( prefix, TFixed48_16 ) \
    GUCEF_MATH_CTMATRIX2D_INTEGER_KIND( prefix, Int8 ) \
    GUCEF_MATH_CTMATRIX2D_INTEGER_KIND( prefix, UInt8 ) \
    GUCEF_MATH_CTMATRIX2D_INTEGER_KIND( prefix, Int16 ) \
    GUCEF_MATH_CTMATRIX2D_INTEGER_KIND( prefix, UInt16 ) \
    GUCEF_MATH_CTMATRIX2D_INTEGER_KIND( prefix, Int32 ) \
    GUCEF_MATH_CTMATRIX2D_INTEGER_KIND( prefix, UInt32 ) \
    GUCEF_MATH_CTMATRIX2D_INTEGER_KIND( prefix, Int64 ) \
    GUCEF_MATH_CTMATRIX2D_INTEGER_KIND( prefix, UInt64 )

#define GUCEF_MATH_CTMATRIX2D_FRACTION_KINDS( prefix ) \
    GUCEF_MATH_CTMATRIX2D_EXACT_KIND( prefix, CORE::TInt64Fraction ) \
    GUCEF_MATH_CTMATRIX2D_EXACT_KIND( prefix, TMathBigFraction )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTMATRIX2D_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTMATRIX2D_H ? */
