/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#ifndef GUCEF_GEOMETRY_CTINTEGERRECT_H
#define GUCEF_GEOMETRY_CTINTEGERRECT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#include "gucefGEOMETRY_SShapeTextReader.h"
#define GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#endif /* GUCEF_GEOMETRY_SSHAPETEXTREADER_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A rectangle of integer cells for tiles, grids and pixels (section 7.2): position and size, half-open, so it holds
 *  the cells min <= p < min + size (the convention of Godot Rect2i and Unity RectInt). Not the closed CTRectangle.
 *  Converts to and from GUI::CRect at the call site.
 *
 *  - Kind: an integer kind of at most 32 bits (Int32 is exported); the arithmetic that could overflow is done in
 *    64 bits and saturates to the range of the kind, positions and sizes wrap as the kind does (no undefined behaviour).
 *  - Layout: position and size, 4 * sizeof( T ), no virtual functions. The default rectangle is empty at the origin.
 *  - Empty: a size of zero or less on some axis; an empty rectangle contains no cell and has no cells.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text: "IntegerRect(position (x, y), size (width, height))"; DataNode: child nodes position and size.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTIntegerRect
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef MATH::CTVector2D< T > TVector;
    typedef typename MATH::CTMathIntegerComponentTypes< T >::TUnsigned TUnsigned;

    GUCEF_GEOMETRY_STATIC_ASSERT( TTraits::IsInteger != 0, Integer_ranges_require_an_integer_kind );
    GUCEF_GEOMETRY_STATIC_ASSERT( sizeof( T ) <= 4, Integer_ranges_support_kinds_of_at_most_32_bits );

    TPoint position;
    TVector size;

    GUCEF_GEOMETRY_FORCE_INLINE CTIntegerRect( void ) : position(), size() {}

    GUCEF_GEOMETRY_FORCE_INLINE CTIntegerRect( const TPoint& minCorner, const TVector& cellSize ) : position( minCorner ), size( cellSize ) {}

    GUCEF_GEOMETRY_FORCE_INLINE CTIntegerRect( const T& x, const T& y, const T& width, const T& height ) : position( x, y ), size( width, height ) {}

    /** The first cell */
    GUCEF_GEOMETRY_FORCE_INLINE const TPoint& GetMin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return position; }

    /** position + size: the first cell that is not inside (wraps as the kind does for huge rectangles) */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetEnd( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TPoint( MATH::ComponentAdd( position.x, size.x ), MATH::ComponentAdd( position.y, size.y ) );
    }

    /** True for a size of zero or less on some axis */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !( TTraits::Zero() < size.x && TTraits::Zero() < size.y );
    }

    /** The number of cells; 0 for an empty rectangle */
    GUCEF_GEOMETRY_FORCE_INLINE UInt64 GetCellCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return IsEmpty() ? 0 : (UInt64) (Int64) size.x * (UInt64) (Int64) size.y;
    }

    /** width * height in the kind (wraps for huge rectangles); 0 for an empty rectangle */
    GUCEF_GEOMETRY_FORCE_INLINE T GetArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return IsEmpty() ? TTraits::Zero() : MATH::ComponentMultiply( size.x, size.y );
    }

    /** True when min <= p < min + size on both axes; false for an empty rectangle */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& cell ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !IsEmpty() &&
               !( cell.x < position.x ) && (TUnsigned) ( (TUnsigned) cell.x - (TUnsigned) position.x ) < (TUnsigned) size.x &&
               !( cell.y < position.y ) && (TUnsigned) ( (TUnsigned) cell.y - (TUnsigned) position.y ) < (TUnsigned) size.y;
    }

    /** True when every cell of 'other' is inside this rectangle; an empty 'other' is inside every rectangle, nothing is inside an empty one */
    bool Contains( const CTIntegerRect& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when the rectangles share a cell */
    bool Intersects( const CTIntegerRect& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The cells that both rectangles hold (an empty rectangle at the larger minimum corner when there are none) */
    CTIntegerRect Intersection( const CTIntegerRect& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The same as Intersection: this rectangle clipped to 'bounds' */
    GUCEF_GEOMETRY_FORCE_INLINE CTIntegerRect Clip( const CTIntegerRect& bounds ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return Intersection( bounds ); }

    /**
     *  The smallest rectangle that holds both (the other one when this is empty and vice versa). When the result is
     *  larger than the kind can size, the size saturates at the largest value of the kind and the result no longer
     *  holds both inputs: use TryUnion when the inputs can be that far apart.
     */
    CTIntegerRect Union( const CTIntegerRect& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        CTIntegerRect result;
        TryUnion( other, result );
        return result;
    }

    /** Union that reports the overflow: false when the size does not fit the kind ( 'out' then holds the saturated rectangle ) */
    bool TryUnion( const CTIntegerRect& other, CTIntegerRect& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Grown by 'margin' cells on every side (shrunk for a negative margin, the size does not go below 0); saturates to the range of the kind */
    CTIntegerRect Expanded( const T& margin ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Moved by 'offset' (wraps as the kind does) */
    GUCEF_GEOMETRY_FORCE_INLINE CTIntegerRect Translated( const TVector& offset ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return CTIntegerRect( TPoint( MATH::ComponentAdd( position.x, offset.x ), MATH::ComponentAdd( position.y, offset.y ) ), size );
    }

    /**
     *  The cell with the given index when iterating row by row (x fastest, then y), index below GetCellCount;
     *  the minimum corner for an index out of range.
     */
    TPoint GetCell( UInt64 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTIntegerRect& a, const CTIntegerRect& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a.position == b.position && a.size == b.size; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTIntegerRect& a, const CTIntegerRect& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    /** "IntegerRect(position (x, y), size (width, height))" */
    CORE::CString ToString( void ) const
    {
        return "IntegerRect(position " + position.ToString() + ", size " + size.ToString() + ")";
    }

    /** Parses the ToString form; false leaves this rectangle unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedPosition;
        TVector parsedSize;
        if ( !reader.TryLiteral( "IntegerRect" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "position" ) || !reader.TryValue( parsedPosition ) ||
             !reader.TryChar( ',' ) || !reader.TryLiteral( "size" ) || !reader.TryValue( parsedSize ) || !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        position = parsedPosition;
        size = parsedSize;
        return true;
    }

    /** Writes the child nodes position and size into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        return SerializeShapeChild( node, "position", position, settings ) && SerializeShapeChild( node, "size", size, settings );
    }

    /** Reads the child nodes position and size; a missing or invalid one fails and leaves this rectangle unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TPoint parsedPosition;
        TVector parsedSize;
        if ( !DeserializeShapeChild( node, "position", parsedPosition, settings ) || !DeserializeShapeChild( node, "size", parsedSize, settings ) )
            return false;
        position = parsedPosition;
        size = parsedSize;
        return true;
    }

    private:

    /** Clamps a 64 bit value to the range of the kind */
    static GUCEF_GEOMETRY_FORCE_INLINE T Saturate( Int64 value ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const Int64 lowest = (Int64) TTraits::Lowest();
        const Int64 highest = (Int64) TTraits::Max();
        return (T) ( value < lowest ? lowest : ( highest < value ? highest : value ) );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
bool
CTIntegerRect< T >::Contains( const CTIntegerRect& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( other.IsEmpty() )
        return true;
    if ( IsEmpty() )
        return false;
    return (Int64) position.x <= (Int64) other.position.x && (Int64) other.position.x + (Int64) other.size.x <= (Int64) position.x + (Int64) size.x &&
           (Int64) position.y <= (Int64) other.position.y && (Int64) other.position.y + (Int64) other.size.y <= (Int64) position.y + (Int64) size.y;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTIntegerRect< T >::Intersects( const CTIntegerRect& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( IsEmpty() || other.IsEmpty() )
        return false;
    return (Int64) position.x < (Int64) other.position.x + (Int64) other.size.x && (Int64) other.position.x < (Int64) position.x + (Int64) size.x &&
           (Int64) position.y < (Int64) other.position.y + (Int64) other.size.y && (Int64) other.position.y < (Int64) position.y + (Int64) size.y;
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTIntegerRect< T >
CTIntegerRect< T >::Intersection( const CTIntegerRect& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const Int64 lowX = (Int64) ( position.x < other.position.x ? other.position.x : position.x );
    const Int64 lowY = (Int64) ( position.y < other.position.y ? other.position.y : position.y );
    const Int64 highX = (Int64) position.x + (Int64) size.x < (Int64) other.position.x + (Int64) other.size.x ? (Int64) position.x + (Int64) size.x : (Int64) other.position.x + (Int64) other.size.x;
    const Int64 highY = (Int64) position.y + (Int64) size.y < (Int64) other.position.y + (Int64) other.size.y ? (Int64) position.y + (Int64) size.y : (Int64) other.position.y + (Int64) other.size.y;
    if ( IsEmpty() || other.IsEmpty() || highX <= lowX || highY <= lowY )
        return CTIntegerRect( TPoint( (T) lowX, (T) lowY ), TVector() );
    return CTIntegerRect( TPoint( (T) lowX, (T) lowY ), TVector( (T) ( highX - lowX ), (T) ( highY - lowY ) ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTIntegerRect< T >::TryUnion( const CTIntegerRect& other, CTIntegerRect& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( IsEmpty() )
    {
        out = other;
        return true;
    }
    if ( other.IsEmpty() )
    {
        out = *this;
        return true;
    }
    const Int64 lowX = (Int64) ( position.x < other.position.x ? position.x : other.position.x );
    const Int64 lowY = (Int64) ( position.y < other.position.y ? position.y : other.position.y );
    const Int64 highX = (Int64) position.x + (Int64) size.x < (Int64) other.position.x + (Int64) other.size.x ? (Int64) other.position.x + (Int64) other.size.x : (Int64) position.x + (Int64) size.x;
    const Int64 highY = (Int64) position.y + (Int64) size.y < (Int64) other.position.y + (Int64) other.size.y ? (Int64) other.position.y + (Int64) other.size.y : (Int64) position.y + (Int64) size.y;
    out = CTIntegerRect( TPoint( (T) lowX, (T) lowY ), TVector( Saturate( highX - lowX ), Saturate( highY - lowY ) ) );
    return highX - lowX <= (Int64) TTraits::Max() && highY - lowY <= (Int64) TTraits::Max();
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTIntegerRect< T >
CTIntegerRect< T >::Expanded( const T& margin ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const Int64 m = (Int64) margin;
    Int64 width = (Int64) size.x + 2 * m;
    Int64 height = (Int64) size.y + 2 * m;
    if ( width < 0 )
        width = 0;
    if ( height < 0 )
        height = 0;
    return CTIntegerRect( TPoint( Saturate( (Int64) position.x - m ), Saturate( (Int64) position.y - m ) ), TVector( Saturate( width ), Saturate( height ) ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTIntegerRect< T >::TPoint
CTIntegerRect< T >::GetCell( UInt64 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( index >= GetCellCount() )
        return position;
    const UInt64 width = (UInt64) (Int64) size.x;
    return TPoint( (T) ( (Int64) position.x + (Int64) ( index % width ) ), (T) ( (Int64) position.y + (Int64) ( index / width ) ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTIntegerRect< Int32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTINTEGERRECT_H ? */
