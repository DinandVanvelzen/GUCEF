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

#ifndef GUCEF_GEOMETRY_CTINTEGERBOX3D_H
#define GUCEF_GEOMETRY_CTINTEGERBOX3D_H

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
 *  A box of integer cells for voxels and 3D grids (section 7.2): position and size, half-open, the 3D form of
 *  CTIntegerRect. Distinct from the closed CTAxisAlignedBox3D< Int32 >.
 *
 *  - Kind: an integer kind of at most 32 bits (Int32 is exported); the arithmetic that could overflow is done in
 *    64 bits and saturates to the range of the kind, positions and sizes wrap as the kind does.
 *  - Layout: position and size, 6 * sizeof( T ), no virtual functions. The default box is empty at the origin.
 *  - Empty: a size of zero or less on some axis.
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text: "IntegerBox3D(position (x, y, z), size (width, height, depth))"; DataNode: child nodes position and size.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTIntegerBox3D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef typename MATH::CTMathIntegerComponentTypes< T >::TUnsigned TUnsigned;

    GUCEF_GEOMETRY_STATIC_ASSERT( TTraits::IsInteger != 0, Integer_ranges_require_an_integer_kind );
    GUCEF_GEOMETRY_STATIC_ASSERT( sizeof( T ) <= 4, Integer_ranges_support_kinds_of_at_most_32_bits );

    TPoint position;
    TVector size;

    GUCEF_GEOMETRY_FORCE_INLINE CTIntegerBox3D( void ) : position(), size() {}

    GUCEF_GEOMETRY_FORCE_INLINE CTIntegerBox3D( const TPoint& minCorner, const TVector& cellSize ) : position( minCorner ), size( cellSize ) {}

    /** The first cell */
    GUCEF_GEOMETRY_FORCE_INLINE const TPoint& GetMin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return position; }

    /** position + size: the first cell that is not inside (wraps as the kind does for huge boxes) */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetEnd( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TPoint( MATH::ComponentAdd( position.x, size.x ), MATH::ComponentAdd( position.y, size.y ), MATH::ComponentAdd( position.z, size.z ) );
    }

    /** True for a size of zero or less on some axis */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !( TTraits::Zero() < size.x && TTraits::Zero() < size.y && TTraits::Zero() < size.z );
    }

    /** The number of cells; 0 for an empty box; saturates at the largest UInt64 (only reachable with 32 bit kinds at their extremes) */
    UInt64 GetCellCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** width * height * depth in the kind (wraps for huge boxes); 0 for an empty box */
    GUCEF_GEOMETRY_FORCE_INLINE T GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return IsEmpty() ? TTraits::Zero() : MATH::ComponentMultiply( MATH::ComponentMultiply( size.x, size.y ), size.z );
    }

    /** True when min <= p < min + size on all axes; false for an empty box */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& cell ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return !IsEmpty() &&
               !( cell.x < position.x ) && (TUnsigned) ( (TUnsigned) cell.x - (TUnsigned) position.x ) < (TUnsigned) size.x &&
               !( cell.y < position.y ) && (TUnsigned) ( (TUnsigned) cell.y - (TUnsigned) position.y ) < (TUnsigned) size.y &&
               !( cell.z < position.z ) && (TUnsigned) ( (TUnsigned) cell.z - (TUnsigned) position.z ) < (TUnsigned) size.z;
    }

    /** True when every cell of 'other' is inside this box; an empty 'other' is inside every box, nothing is inside an empty one */
    bool Contains( const CTIntegerBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when the boxes share a cell */
    bool Intersects( const CTIntegerBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The cells that both boxes hold (an empty box at the larger minimum corner when there are none) */
    CTIntegerBox3D Intersection( const CTIntegerBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The same as Intersection: this box clipped to 'bounds' */
    GUCEF_GEOMETRY_FORCE_INLINE CTIntegerBox3D Clip( const CTIntegerBox3D& bounds ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return Intersection( bounds ); }

    /**
     *  The smallest box that holds both (the other one when this is empty and vice versa). When the result is larger
     *  than the kind can size, the size saturates at the largest value of the kind and the result no longer holds both
     *  inputs: use TryUnion when the inputs can be that far apart.
     */
    CTIntegerBox3D Union( const CTIntegerBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        CTIntegerBox3D result;
        TryUnion( other, result );
        return result;
    }

    /** Union that reports the overflow: false when the size does not fit the kind ( 'out' then holds the saturated box ) */
    bool TryUnion( const CTIntegerBox3D& other, CTIntegerBox3D& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Grown by 'margin' cells on every side (shrunk for a negative margin, the size does not go below 0); saturates to the range of the kind */
    CTIntegerBox3D Expanded( const T& margin ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Moved by 'offset' (wraps as the kind does) */
    GUCEF_GEOMETRY_FORCE_INLINE CTIntegerBox3D Translated( const TVector& offset ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return CTIntegerBox3D( TPoint( MATH::ComponentAdd( position.x, offset.x ), MATH::ComponentAdd( position.y, offset.y ), MATH::ComponentAdd( position.z, offset.z ) ), size );
    }

    /**
     *  The cell with the given index when iterating row by row, slice by slice (x fastest, then y, then z), index
     *  below GetCellCount; the minimum corner for an index out of range.
     */
    TPoint GetCell( UInt64 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator==( const CTIntegerBox3D& a, const CTIntegerBox3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a.position == b.position && a.size == b.size; }
    friend GUCEF_GEOMETRY_FORCE_INLINE bool operator!=( const CTIntegerBox3D& a, const CTIntegerBox3D& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return !( a == b ); }

    /** "IntegerBox3D(position (x, y, z), size (width, height, depth))" */
    CORE::CString ToString( void ) const
    {
        return "IntegerBox3D(position " + position.ToString() + ", size " + size.ToString() + ")";
    }

    /** Parses the ToString form; false leaves this box unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TPoint parsedPosition;
        TVector parsedSize;
        if ( !reader.TryLiteral( "IntegerBox3D" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "position" ) || !reader.TryValue( parsedPosition ) ||
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

    /** Reads the child nodes position and size; a missing or invalid one fails and leaves this box unchanged */
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

    static GUCEF_GEOMETRY_FORCE_INLINE T Saturate( Int64 value ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const Int64 lowest = (Int64) TTraits::Lowest();
        const Int64 highest = (Int64) TTraits::Max();
        return (T) ( value < lowest ? lowest : ( highest < value ? highest : value ) );
    }

    static GUCEF_GEOMETRY_FORCE_INLINE Int64 Low( Int64 a, Int64 b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a < b ? a : b; }
    static GUCEF_GEOMETRY_FORCE_INLINE Int64 High( Int64 a, Int64 b ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return a < b ? b : a; }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
UInt64
CTIntegerBox3D< T >::GetCellCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( IsEmpty() )
        return 0;
    const UInt64 plane = (UInt64) (Int64) size.x * (UInt64) (Int64) size.y;
    const UInt64 depth = (UInt64) (Int64) size.z;
    if ( depth > 0 && plane > ( ~(UInt64) 0 ) / depth )
        return ~(UInt64) 0;
    return plane * depth;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTIntegerBox3D< T >::Contains( const CTIntegerBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( other.IsEmpty() )
        return true;
    if ( IsEmpty() )
        return false;
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const Int64 lo = (Int64) position.GetComponent( axis );
        const Int64 otherLo = (Int64) other.position.GetComponent( axis );
        if ( otherLo < lo || lo + (Int64) size.GetComponent( axis ) < otherLo + (Int64) other.size.GetComponent( axis ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTIntegerBox3D< T >::Intersects( const CTIntegerBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( IsEmpty() || other.IsEmpty() )
        return false;
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const Int64 lo = (Int64) position.GetComponent( axis );
        const Int64 otherLo = (Int64) other.position.GetComponent( axis );
        if ( !( lo < otherLo + (Int64) other.size.GetComponent( axis ) && otherLo < lo + (Int64) size.GetComponent( axis ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTIntegerBox3D< T >
CTIntegerBox3D< T >::Intersection( const CTIntegerBox3D& other ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TPoint low;
    TVector extent;
    bool empty = IsEmpty() || other.IsEmpty();
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const Int64 lo = High( (Int64) position.GetComponent( axis ), (Int64) other.position.GetComponent( axis ) );
        const Int64 hi = Low( (Int64) position.GetComponent( axis ) + (Int64) size.GetComponent( axis ), (Int64) other.position.GetComponent( axis ) + (Int64) other.size.GetComponent( axis ) );
        low.SetComponent( axis, (T) lo );
        if ( hi <= lo )
            empty = true;
        else
            extent.SetComponent( axis, (T) ( hi - lo ) );
    }
    return CTIntegerBox3D( low, empty ? TVector() : extent );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTIntegerBox3D< T >::TryUnion( const CTIntegerBox3D& other, CTIntegerBox3D& out ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
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
    bool fits = true;
    TPoint low;
    TVector extent;
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const Int64 lo = Low( (Int64) position.GetComponent( axis ), (Int64) other.position.GetComponent( axis ) );
        const Int64 hi = High( (Int64) position.GetComponent( axis ) + (Int64) size.GetComponent( axis ), (Int64) other.position.GetComponent( axis ) + (Int64) other.size.GetComponent( axis ) );
        low.SetComponent( axis, (T) lo );
        extent.SetComponent( axis, Saturate( hi - lo ) );
        fits = fits && hi - lo <= (Int64) TTraits::Max();
    }
    out = CTIntegerBox3D( low, extent );
    return fits;
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTIntegerBox3D< T >
CTIntegerBox3D< T >::Expanded( const T& margin ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const Int64 m = (Int64) margin;
    TPoint low;
    TVector extent;
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        Int64 length = (Int64) size.GetComponent( axis ) + 2 * m;
        if ( length < 0 )
            length = 0;
        low.SetComponent( axis, Saturate( (Int64) position.GetComponent( axis ) - m ) );
        extent.SetComponent( axis, Saturate( length ) );
    }
    return CTIntegerBox3D( low, extent );
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTIntegerBox3D< T >::TPoint
CTIntegerBox3D< T >::GetCell( UInt64 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( index >= GetCellCount() )
        return position;
    const UInt64 width = (UInt64) (Int64) size.x;
    const UInt64 height = (UInt64) (Int64) size.y;
    const UInt64 row = index / width;
    return TPoint( (T) ( (Int64) position.x + (Int64) ( index % width ) ),
                   (T) ( (Int64) position.y + (Int64) ( row % height ) ),
                   (T) ( (Int64) position.z + (Int64) ( row / height ) ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTIntegerBox3D< Int32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTINTEGERBOX3D_H ? */
