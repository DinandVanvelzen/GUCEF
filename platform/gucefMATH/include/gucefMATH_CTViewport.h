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

#ifndef GUCEF_MATH_CTVIEWPORT_H
#define GUCEF_MATH_CTVIEWPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_MATH_SPROJECTIONCONVENTION_H
#include "gucefMATH_SProjectionConvention.h"
#define GUCEF_MATH_SPROJECTIONCONVENTION_H
#endif /* GUCEF_MATH_SPROJECTIONCONVENTION_H ? */

#ifndef GUCEF_MATH_COMPONENTFUNCTIONS_H
#include "gucefMATH_ComponentFunctions.h"
#define GUCEF_MATH_COMPONENTFUNCTIONS_H
#endif /* GUCEF_MATH_COMPONENTFUNCTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The rectangle of a render target that normalised device coordinates are mapped to (section 4.5.7 of the design),
 *  used by Project, Unproject and GetPickRay of gucefMATH_ProjectionFunctions.h.
 *
 *  - Screen space: the origin is the top left corner of the target, x grows to the right and y grows downward (the
 *    coordinates of windowing systems and mouse positions); ( x, y ) is the corner of the viewport and ( width, height )
 *    its size in the same units.
 *  - ndcYDown: false when the normalised device Y of the projection points up (OpenGL, Direct3D, Metal), true when it
 *    points down (a projection built with SProjectionConvention::flipY, Vulkan without a negative viewport). Build the
 *    viewport with FromConvention so that it always agrees with the projection matrix.
 *  - Layout: four values of T and a flag, no virtual functions; a plain parameter struct, realtime-safe except the text and
 *    DataNode members.
 *  - Text "(x, y, width, height)" with ", ndcYDown" before the closing bracket when the flag is set; DataNode: attributes
 *    x, y, width, height and ndcYDown (boolean).
 *  T is a floating point kind.
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTViewport
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;

    GUCEF_MATH_STATIC_ASSERT( TTraits::IsFloatingPoint, Viewports_require_a_floating_point_kind );

    T x;
    T y;
    T width;
    T height;
    bool ndcYDown;

    /** The unit viewport ( 0, 0, 1, 1 ) with the normalised device Y pointing up */
    GUCEF_MATH_FORCE_INLINE CTViewport( void )
        : x( TTraits::Zero() ), y( TTraits::Zero() ), width( TTraits::One() ), height( TTraits::One() ), ndcYDown( false )
    {
    }

    /** The viewport at ( x, y ) of the given size; the normalised device Y points up unless ndcYDownFlag is true */
    GUCEF_MATH_FORCE_INLINE CTViewport( const T& xValue, const T& yValue, const T& widthValue, const T& heightValue, bool ndcYDownFlag )
        : x( xValue ), y( yValue ), width( widthValue ), height( heightValue ), ndcYDown( ndcYDownFlag )
    {
    }

    /** The viewport at ( x, y ) of the given size for a projection built with 'convention' (ndcYDown = convention.flipY) */
    static GUCEF_MATH_FORCE_INLINE CTViewport FromConvention( const T& xValue, const T& yValue, const T& widthValue, const T& heightValue, const SProjectionConvention& convention ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return CTViewport( xValue, yValue, widthValue, heightValue, convention.flipY );
    }

    /** True when the width and height are finite and greater than zero and the corner is finite */
    GUCEF_MATH_FORCE_INLINE bool IsValid( void ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        return TTraits::IsFinite( x ) && TTraits::IsFinite( y ) && TTraits::IsFinite( width ) && TTraits::IsFinite( height ) &&
               TTraits::Zero() < width && TTraits::Zero() < height;
    }

    /** Exact comparison of every field */
    friend GUCEF_MATH_FORCE_INLINE bool operator==( const CTViewport& a, const CTViewport& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height && a.ndcYDown == b.ndcYDown;
    }

    friend GUCEF_MATH_FORCE_INLINE bool operator!=( const CTViewport& a, const CTViewport& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "(x, y, width, height)" or "(x, y, width, height, ndcYDown)" */
    CORE::CString ToString( void ) const
    {
        const T components[ 4 ] = { x, y, width, height };
        CORE::CString text = ComponentsToString( components );
        if ( ndcYDown && text.Length() > 0 )
        {
            CORE::CString opened( text.C_String(), text.Length() - 1 );
            opened += ", ndcYDown)";
            text = opened;
        }
        return text;
    }

    /** Parses the ToString form (white space allowed); false leaves this viewport unchanged */
    bool FromString( const CORE::CString& text )
    {
        const char* data = text.C_String();
        UInt32 length = text.Length();
        if ( GUCEF_NULL == data )
            return false;
        while ( length > 0 && IsComponentTextWhitespace( data[ length - 1 ] ) )
            --length;
        static const char suffix[] = ", ndcYDown)";
        const UInt32 suffixLength = (UInt32) ( sizeof( suffix ) - 1 );
        bool flag = false;
        CORE::CString numbers;
        if ( length > suffixLength && 0 == memcmp( data + length - suffixLength, suffix, suffixLength ) )
        {
            flag = true;
            numbers = CORE::CString( data, length - suffixLength );
            numbers += ')';
        }
        else
        {
            numbers = CORE::CString( data, length );
        }
        T components[ 4 ];
        if ( !ComponentsFromString( numbers, components ) )
            return false;
        x = components[ 0 ];
        y = components[ 1 ];
        width = components[ 2 ];
        height = components[ 3 ];
        ndcYDown = flag;
        return true;
    }

    /** Writes the attributes x, y, width, height and ndcYDown into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        const char* const names[ 4 ] = { "x", "y", "width", "height" };
        const T components[ 4 ] = { x, y, width, height };
        (void) settings;
        return SerializeComponents( node, names, components ) && node.SetAttribute( "ndcYDown", CORE::CVariant( ndcYDown ) );
    }

    /** Reads the attributes of Serialize; a missing or invalid attribute fails and leaves this viewport unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        const char* const names[ 4 ] = { "x", "y", "width", "height" };
        T components[ 4 ];
        (void) settings;
        const CORE::CDataNode::TKeyValuePair* flag = node.GetAttribute( "ndcYDown" );
        if ( GUCEF_NULL == flag || !DeserializeComponents( node, names, components ) )
            return false;
        bool flagValue = false;
        if ( flag->second.IsBoolean() || flag->second.IsInteger() )
        {
            flagValue = flag->second.AsBool();
        }
        else
        {
            CORE::CString flagText = flag->second.AsString();
            if ( flagText == "true" || flagText == "1" ) flagValue = true;
            else if ( flagText == "false" || flagText == "0" ) flagValue = false;
            else return false;
        }
        x = components[ 0 ];
        y = components[ 1 ];
        width = components[ 2 ];
        height = components[ 3 ];
        ndcYDown = flagValue;
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTViewport< Float32 > TFloat32Viewport;
typedef CTViewport< Float64 > TFloat64Viewport;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTVIEWPORT_H ? */
