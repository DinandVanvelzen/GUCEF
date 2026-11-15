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

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#define GUCEF_MATH_CCOORDINATESYSTEM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTNUMERICCONVERTER_H
#include "gucefMATH_CTNumericConverter.h"
#define GUCEF_MATH_CTNUMERICCONVERTER_H
#endif /* GUCEF_MATH_CTNUMERICCONVERTER_H ? */

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

template < typename T > class CTVector3D;
template < typename T > class CTMatrix3D;
template < typename T > class CTMatrix4D;
template < typename T > class CTQuaternion;
template < typename T, typename TPosition > class CTTransform3D;

/*-------------------------------------------------------------------------*/

/**
 *  The axis conventions of a coordinate system: which signed axes are right, up and forward, and how many units make a
 *  metre (section 4.4 of the design). Converts directions, positions, rotations and transforms between systems, for
 *  example between engines and asset formats.
 *
 *  - Forward is the view direction of an unrotated camera or listener; asset "front" conventions are the business of
 *    the asset converters.
 *  - Handedness is not stored: right-handed exactly when right x up == -forward.
 *  - The conversion maps each physical direction of this system to the same physical direction of the target:
 *    M = S * B_target * B_this^T, with B the matrix whose columns are the right, up and backward unit axes and S the unit
 *    scale target.unitsPerMetre / this.unitsPerMetre. The unit-free part is a signed permutation, so conversions are
 *    exact apart from the unit scale.
 *  - ConvertRotation re-expresses a rotation as P R P^T with P the unit-free part; for a reflection (determinant -1)
 *    this negates the rotation angle as seen in the target, which is the physically correct result.
 *  - Value type: copyable, no heap, realtime-safe except the text and DataNode members.
 *  - Text "(+X, +Y, -Z, 1)" (right, up, forward, units per metre); DataNode attributes right, up, forward, unitsPerMetre.
 */
class GUCEF_MATH_PUBLIC_CPP CCoordinateSystem
{
    public:

    /** GucefDefault() */
    CCoordinateSystem( void );

    /** The axes must lie on three different coordinate axes and unitsPerMetre must be positive (else IsValid() is false) */
    CCoordinateSystem( TSignedAxis right, TSignedAxis up, TSignedAxis forward, Float64 unitsPerMetre );

    /** +X right, +Y up, -Z forward, metres: right-handed (also Ogre, OpenAL, glTF cameras, gucefAUDIO) */
    static CCoordinateSystem GucefDefault( void ) GUCEF_MATH_REALTIME_CONTEXT;

    /** +X right, +Y up, +Z forward, metres: left-handed */
    static CCoordinateSystem Unity( void ) GUCEF_MATH_REALTIME_CONTEXT;

    /** +Y right, +Z up, +X forward, centimetres: left-handed */
    static CCoordinateSystem Unreal( void ) GUCEF_MATH_REALTIME_CONTEXT;

    /** +X right, +Z up, +Y forward, metres: right-handed */
    static CCoordinateSystem Blender( void ) GUCEF_MATH_REALTIME_CONTEXT;

    /** +X right, +Y up, -Z forward, metres: right-handed */
    static CCoordinateSystem Godot( void ) GUCEF_MATH_REALTIME_CONTEXT;

    TSignedAxis GetRightAxis( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    TSignedAxis GetUpAxis( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    TSignedAxis GetForwardAxis( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** Right-handed exactly when right x up == -forward */
    THandedness GetHandedness( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    Float64 GetUnitsPerMetre( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** True when the three axes lie on different coordinate axes and the units per metre are positive and finite */
    bool IsValid( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The right axis as a unit vector */
    template < typename T > CTVector3D< T > GetRight( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The up axis as a unit vector */
    template < typename T > CTVector3D< T > GetUp( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The forward axis as a unit vector */
    template < typename T > CTVector3D< T > GetForward( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** M = S * B_target * B_this^T: maps positions and displacements of this system to 'target', including unit scale */
    template < typename T > CTMatrix3D< T > GetConversionMatrixTo( const CCoordinateSystem& target ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** GetConversionMatrixTo as a 4x4 affine matrix */
    template < typename T > CTMatrix4D< T > GetConversionMatrix4To( const CCoordinateSystem& target ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The rotation q of this system expressed in 'target' (P R P^T; exact component moves and sign changes) */
    template < typename T > CTQuaternion< T > ConvertRotation( const CCoordinateSystem& target, const CTQuaternion< T >& q ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The transform t of this system expressed in 'target': position converted with unit scale, rotation as ConvertRotation, scale permuted */
    template < typename T, typename TPosition >
    CTTransform3D< T, TPosition > ConvertTransform( const CCoordinateSystem& target, const CTTransform3D< T, TPosition >& t ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** True when the conversion to 'target' mirrors (determinant -1): triangle winding must be reversed */
    bool ConversionFlipsWinding( const CCoordinateSystem& target ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The unit-free conversion P = B_target * B_this^T as a signed permutation, column-major (entries -1, 0, 1) */
    void GetConversionPermutation( const CCoordinateSystem& target, Int32 ( &columnMajor )[ 9 ] ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** target.unitsPerMetre / this.unitsPerMetre */
    Float64 GetConversionScale( const CCoordinateSystem& target ) const GUCEF_MATH_REALTIME_CONTEXT;

    /** The unit vector of a signed axis, as integers */
    static void GetAxisVector( TSignedAxis axis, Int32 ( &vector )[ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT;

    bool operator==( const CCoordinateSystem& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    bool operator!=( const CCoordinateSystem& other ) const GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Text and DataNode (not realtime-safe: they allocate)                */
    /*---------------------------------------------------------------------*/

    /** "+X", "-X", ... "-Z" */
    static const char* GetAxisName( TSignedAxis axis );

    /** The axis of a GetAxisName text; false leaves 'axis' unchanged */
    static bool AxisFromName( const CORE::CString& name, TSignedAxis& axis );

    /** "(+X, +Y, -Z, 1)": right, up, forward, units per metre */
    CORE::CString ToString( void ) const;

    /** Parses the ToString form; false (also for an invalid system) leaves this value unchanged */
    bool FromString( const CORE::CString& text );

    /** Writes the attributes right, up, forward and unitsPerMetre */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const;

    /** Reads the attributes of Serialize; a missing or invalid attribute, or an invalid system, leaves this value unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings );

    private:

    /** The determinant (+1 or -1) of the basis [ right, up, backward ]; 0 for an invalid system */
    Int32 GetBasisDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT;

    TSignedAxis m_right;
    TSignedAxis m_up;
    TSignedAxis m_forward;
    Float64 m_unitsPerMetre;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/* The value types of the template members, included after the class (any header of the family can be included first) */

#ifndef GUCEF_MATH_CTVECTOR3D_H
#include "gucefMATH_CTVector3D.h"
#define GUCEF_MATH_CTVECTOR3D_H
#endif /* GUCEF_MATH_CTVECTOR3D_H ? */

#ifndef GUCEF_MATH_CTMATRIX4D_H
#include "gucefMATH_CTMatrix4D.h"
#define GUCEF_MATH_CTMATRIX4D_H
#endif /* GUCEF_MATH_CTMATRIX4D_H ? */

#ifndef GUCEF_MATH_CTQUATERNION_H
#include "gucefMATH_CTQuaternion.h"
#define GUCEF_MATH_CTQUATERNION_H
#endif /* GUCEF_MATH_CTQUATERNION_H ? */

#ifndef GUCEF_MATH_CTTRANSFORM3D_H
#include "gucefMATH_CTTransform3D.h"
#define GUCEF_MATH_CTTRANSFORM3D_H
#endif /* GUCEF_MATH_CTTRANSFORM3D_H ? */

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

/* The conversions only move components, change signs and multiply by the unit scale once: exact and inline */

template < typename T >
inline CTVector3D< T >
CCoordinateSystem::GetRight( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    Int32 v[ 3 ];
    GetAxisVector( m_right, v );
    return CTVector3D< T >( CTNumericTraits< T >::FromInt32( v[ 0 ] ), CTNumericTraits< T >::FromInt32( v[ 1 ] ), CTNumericTraits< T >::FromInt32( v[ 2 ] ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline CTVector3D< T >
CCoordinateSystem::GetUp( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    Int32 v[ 3 ];
    GetAxisVector( m_up, v );
    return CTVector3D< T >( CTNumericTraits< T >::FromInt32( v[ 0 ] ), CTNumericTraits< T >::FromInt32( v[ 1 ] ), CTNumericTraits< T >::FromInt32( v[ 2 ] ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline CTVector3D< T >
CCoordinateSystem::GetForward( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    Int32 v[ 3 ];
    GetAxisVector( m_forward, v );
    return CTVector3D< T >( CTNumericTraits< T >::FromInt32( v[ 0 ] ), CTNumericTraits< T >::FromInt32( v[ 1 ] ), CTNumericTraits< T >::FromInt32( v[ 2 ] ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline CTMatrix3D< T >
CCoordinateSystem::GetConversionMatrixTo( const CCoordinateSystem& target ) const GUCEF_MATH_REALTIME_CONTEXT
{
    Int32 p[ 9 ];
    GetConversionPermutation( target, p );
    T scale = CTNumericTraits< T >::One();
    CTNumericConverter< Float64, T >::Convert( GetConversionScale( target ), scale, CORE::ROUNDINGMODE_HALF_EVEN );
    T values[ 9 ];
    for ( UInt32 i=0; i<9; ++i )
        values[ i ] = 0 == p[ i ] ? CTNumericTraits< T >::Zero() : ( p[ i ] > 0 ? scale : ComponentNegate( scale ) );
    return CTMatrix3D< T >::FromColumnMajor( values );
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline CTMatrix4D< T >
CCoordinateSystem::GetConversionMatrix4To( const CCoordinateSystem& target ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return CTMatrix4D< T >( GetConversionMatrixTo< T >( target ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
inline CTQuaternion< T >
CCoordinateSystem::ConvertRotation( const CCoordinateSystem& target, const CTQuaternion< T >& q ) const GUCEF_MATH_REALTIME_CONTEXT
{
    Int32 p[ 9 ];
    GetConversionPermutation( target, p );
    const bool mirror = ConversionFlipsWinding( target );
    // P R P^T of a proper P maps the axis with P; a mirror P = -P' (P' proper) gives the same as P', i.e. -P q.xyz
    const T in[ 3 ] = { q.x, q.y, q.z };
    T out[ 3 ];
    for ( UInt32 r=0; r<3; ++r )
    {
        out[ r ] = CTNumericTraits< T >::Zero();
        for ( UInt32 c=0; c<3; ++c )
        {
            const Int32 entry = mirror ? -p[ c * 3 + r ] : p[ c * 3 + r ];
            if ( entry > 0 )
                out[ r ] = in[ c ];
            else if ( entry < 0 )
                out[ r ] = ComponentNegate( in[ c ] );
        }
    }
    return CTQuaternion< T >::FromXYZW( out[ 0 ], out[ 1 ], out[ 2 ], q.w );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TPosition >
inline CTTransform3D< T, TPosition >
CCoordinateSystem::ConvertTransform( const CCoordinateSystem& target, const CTTransform3D< T, TPosition >& t ) const GUCEF_MATH_REALTIME_CONTEXT
{
    Int32 p[ 9 ];
    GetConversionPermutation( target, p );
    const CTMatrix3D< TPosition > positionMatrix = GetConversionMatrixTo< TPosition >( target );
    const T scaleIn[ 3 ] = { t.scale.x, t.scale.y, t.scale.z };
    T scaleOut[ 3 ];
    for ( UInt32 r=0; r<3; ++r )
    {
        scaleOut[ r ] = scaleIn[ r ];
        for ( UInt32 c=0; c<3; ++c )
            if ( 0 != p[ c * 3 + r ] )
                scaleOut[ r ] = scaleIn[ c ];
    }
    return CTTransform3D< T, TPosition >( positionMatrix * t.position, ConvertRotation( target, t.rotation ), CTVector3D< T >( scaleOut[ 0 ], scaleOut[ 1 ], scaleOut[ 2 ] ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */
