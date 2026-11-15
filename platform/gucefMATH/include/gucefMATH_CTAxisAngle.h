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

#ifndef GUCEF_MATH_CTAXISANGLE_H
#define GUCEF_MATH_CTAXISANGLE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTMATRIX2D_H
#include "gucefMATH_CTMatrix2D.h"
#define GUCEF_MATH_CTMATRIX2D_H
#endif /* GUCEF_MATH_CTMATRIX2D_H ? */

#ifndef GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#include "gucefMATH_CTDirectionVector3D.h"
#define GUCEF_MATH_CTDIRECTIONVECTOR3D_H
#endif /* GUCEF_MATH_CTDIRECTIONVECTOR3D_H ? */

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

template < typename T > class CTQuaternion;
template < typename T > class CTMatrix3D;

/*-------------------------------------------------------------------------*/

/**
 *  A rotation as a unit axis and an angle about it by the right-hand rule (section 4.5.4 of the design).
 *
 *  - Kind: T needs a square root and transcendentals (floats and fixed point).
 *  - Layout: the public members axis (a CTDirectionVector3D, so always of unit length) and angle (CTRadian).
 *  - The default value is the identity: axis +X, angle 0 (the axis of a zero rotation is arbitrary; +X is the fixed
 *    choice of CTQuaternion::ToAxisAngle as well).
 *  - Realtime: every member except the text and DataNode members is realtime-safe.
 *  - Text "((x, y, z), <angle> rad)" ("deg" accepted on input); DataNode: child node axis (attributes x, y, z) and the
 *    attribute radians (degrees accepted on input).
 */
template < typename T >
class GUCEF_MATH_TEMPLATE_PUBLIC CTAxisAngle
{
    public:

    typedef T TValue;
    typedef CTNumericTraits< T > TTraits;

    GUCEF_MATH_STATIC_ASSERT( TTraits::HasSqrt && TTraits::HasTranscendentals, Axis_angle_requires_a_numeric_kind_with_sqrt_and_transcendentals );

    CTDirectionVector3D< T > axis;
    CTRadian< T > angle;

    /** Axis +X, angle 0 */
    CTAxisAngle( void )
        : axis( CTDirectionVector3D< T >::UnitX() ), angle()
    {
    }

    CTAxisAngle( const CTDirectionVector3D< T >& rotationAxis, const CTRadian< T >& rotationAngle )
        : axis( rotationAxis ), angle( rotationAngle )
    {
    }

    /** ( axis sin( angle / 2 ), cos( angle / 2 ) ) */
    CTQuaternion< T > ToQuaternion( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The rotation matrix by Rodrigues' formula (CTMatrix3D::RotationAxisAngle) */
    CTMatrix3D< T > ToRotationMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Axis and angle in [0, pi] of the unit quaternion q; the identity gives axis +X and angle 0 */
    static CTAxisAngle FromQuaternion( const CTQuaternion< T >& q ) GUCEF_MATH_REALTIME_CONTEXT;

    friend bool operator==( const CTAxisAngle& a, const CTAxisAngle& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return a.axis == b.axis && a.angle == b.angle;
    }

    friend bool operator!=( const CTAxisAngle& a, const CTAxisAngle& b ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return !( a == b );
    }

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "((x, y, z), <angle> rad)" */
    CORE::CString ToString( void ) const
    {
        CORE::CString text( "(" );
        text += axis.ToString();
        text += ", ";
        text += MathAngleToString( angle );
        text += ')';
        return text;
    }

    /** Parses the ToString form (the axis must be of unit length, as for CTDirectionVector3D); false leaves this value unchanged */
    bool FromString( const CORE::CString& text )
    {
        CORE::CString parts[ 2 ];
        if ( !MathSplitParenthesisedList( text, parts, 2 ) )
            return false;
        CTAxisAngle parsed;
        if ( !parsed.axis.FromString( parts[ 0 ] ) || !MathAngleFromString( parts[ 1 ], parsed.angle ) )
            return false;
        *this = parsed;
        return true;
    }

    /** Writes the child node axis and the attribute radians */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        CORE::CDataNode* axisNode = node.FindOrAddChild( "axis" );
        return GUCEF_NULL != axisNode && axis.Serialize( *axisNode, settings ) && MathAngleSerialize( node, "radians", angle );
    }

    /** Reads the child node axis and the attribute radians (or degrees); failure leaves this value unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        const CORE::CDataNode* axisNode = node.FindChild( "axis" );
        if ( GUCEF_NULL == axisNode )
            return false;
        CTAxisAngle parsed;
        if ( !parsed.axis.Deserialize( *axisNode, settings ) || !MathAngleDeserialize( node, "radians", "degrees", parsed.angle ) )
            return false;
        *this = parsed;
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/* The related classes, included after the class (any header of the family can be included first) */

#ifndef GUCEF_MATH_CTQUATERNION_H
#include "gucefMATH_CTQuaternion.h"
#define GUCEF_MATH_CTQUATERNION_H
#endif /* GUCEF_MATH_CTQUATERNION_H ? */

#ifndef GUCEF_MATH_CTMATRIX3D_H
#include "gucefMATH_CTMatrix3D.h"
#define GUCEF_MATH_CTMATRIX3D_H
#endif /* GUCEF_MATH_CTMATRIX3D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTAxisAngle< T >::ToQuaternion( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTQuaternion< T >::FromAxisAngle( axis, angle );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTMatrix3D< T >
CTAxisAngle< T >::ToRotationMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix3D< T >::RotationAxisAngle( axis, angle );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_MATH_PRECISE_INLINE CTAxisAngle< T >
CTAxisAngle< T >::FromQuaternion( const CTQuaternion< T >& q ) GUCEF_MATH_REALTIME_CONTEXT
{
    CTAxisAngle result;
    q.ToAxisAngle( result.axis, result.angle );
    return result;
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

/* The precision-sensitive members of the predefined kinds (section 5.10 of the design; see gucefMATH_CTVector3D.h) */

#define GUCEF_MATH_CTAXISANGLE_REAL_KIND( prefix, kind ) \
    prefix CTQuaternion< kind > CTAxisAngle< kind >::ToQuaternion( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTMatrix3D< kind > CTAxisAngle< kind >::ToRotationMatrix3D( void ) const GUCEF_MATH_REALTIME_CONTEXT; \
    prefix CTAxisAngle< kind > CTAxisAngle< kind >::FromQuaternion( const CTQuaternion< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTAXISANGLE_PREDEFINED_KINDS( prefix ) \
    GUCEF_MATH_CTAXISANGLE_REAL_KIND( prefix, Float32 ) \
    GUCEF_MATH_CTAXISANGLE_REAL_KIND( prefix, Float64 ) \
    GUCEF_MATH_CTAXISANGLE_REAL_KIND( prefix, TFixed16_16 ) \
    GUCEF_MATH_CTAXISANGLE_REAL_KIND( prefix, TFixed32_32 ) \
    GUCEF_MATH_CTAXISANGLE_REAL_KIND( prefix, TFixed48_16 )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
namespace GUCEF {
namespace MATH {
GUCEF_MATH_CTAXISANGLE_PREDEFINED_KINDS( GUCEF_MATH_EXTERN_TEMPLATE )
}  /* namespace MATH */
}  /* namespace GUCEF */
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTAXISANGLE_H ? */
