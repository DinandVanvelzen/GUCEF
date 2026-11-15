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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */

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

/** The coordinate axis ( 0 = X, 1 = Y, 2 = Z ) of a signed axis */
static inline UInt32
SignedAxisIndex( TSignedAxis axis ) GUCEF_MATH_REALTIME_CONTEXT
{
    return ( (UInt32) axis / 2 ) % 3;
}

/*-------------------------------------------------------------------------*/

/** +1 for the positive axes, -1 for the negative ones */
static inline Int32
SignedAxisSign( TSignedAxis axis ) GUCEF_MATH_REALTIME_CONTEXT
{
    return 0 == ( (UInt32) axis % 2 ) ? 1 : -1;
}

/*-------------------------------------------------------------------------*/

/** The columns right, up, backward of a system as signed axes: backward is the opposite of forward */
static inline void
GetBasis( TSignedAxis right, TSignedAxis up, TSignedAxis forward, UInt32 ( &index )[ 3 ], Int32 ( &sign )[ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT
{
    index[ 0 ] = SignedAxisIndex( right );
    index[ 1 ] = SignedAxisIndex( up );
    index[ 2 ] = SignedAxisIndex( forward );
    sign[ 0 ] = SignedAxisSign( right );
    sign[ 1 ] = SignedAxisSign( up );
    sign[ 2 ] = -SignedAxisSign( forward );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCoordinateSystem::CCoordinateSystem( void )
    : m_right( SIGNEDAXIS_POSITIVE_X )
    , m_up( SIGNEDAXIS_POSITIVE_Y )
    , m_forward( SIGNEDAXIS_NEGATIVE_Z )
    , m_unitsPerMetre( 1.0 )
{
}

/*-------------------------------------------------------------------------*/

CCoordinateSystem::CCoordinateSystem( TSignedAxis right, TSignedAxis up, TSignedAxis forward, Float64 unitsPerMetre )
    : m_right( right )
    , m_up( up )
    , m_forward( forward )
    , m_unitsPerMetre( unitsPerMetre )
{
    GUCEF_MATH_ASSERT( IsValid() );
}

/*-------------------------------------------------------------------------*/

CCoordinateSystem
CCoordinateSystem::GucefDefault( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CCoordinateSystem( SIGNEDAXIS_POSITIVE_X, SIGNEDAXIS_POSITIVE_Y, SIGNEDAXIS_NEGATIVE_Z, 1.0 );
}

/*-------------------------------------------------------------------------*/

CCoordinateSystem
CCoordinateSystem::Unity( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CCoordinateSystem( SIGNEDAXIS_POSITIVE_X, SIGNEDAXIS_POSITIVE_Y, SIGNEDAXIS_POSITIVE_Z, 1.0 );
}

/*-------------------------------------------------------------------------*/

CCoordinateSystem
CCoordinateSystem::Unreal( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CCoordinateSystem( SIGNEDAXIS_POSITIVE_Y, SIGNEDAXIS_POSITIVE_Z, SIGNEDAXIS_POSITIVE_X, 100.0 );
}

/*-------------------------------------------------------------------------*/

CCoordinateSystem
CCoordinateSystem::Blender( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CCoordinateSystem( SIGNEDAXIS_POSITIVE_X, SIGNEDAXIS_POSITIVE_Z, SIGNEDAXIS_POSITIVE_Y, 1.0 );
}

/*-------------------------------------------------------------------------*/

CCoordinateSystem
CCoordinateSystem::Godot( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    return CCoordinateSystem( SIGNEDAXIS_POSITIVE_X, SIGNEDAXIS_POSITIVE_Y, SIGNEDAXIS_NEGATIVE_Z, 1.0 );
}

/*-------------------------------------------------------------------------*/

TSignedAxis
CCoordinateSystem::GetRightAxis( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_right;
}

/*-------------------------------------------------------------------------*/

TSignedAxis
CCoordinateSystem::GetUpAxis( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_up;
}

/*-------------------------------------------------------------------------*/

TSignedAxis
CCoordinateSystem::GetForwardAxis( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_forward;
}

/*-------------------------------------------------------------------------*/

Int32
CCoordinateSystem::GetBasisDeterminant( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 index[ 3 ];
    Int32 sign[ 3 ];
    GetBasis( m_right, m_up, m_forward, index, sign );
    if ( index[ 0 ] == index[ 1 ] || index[ 0 ] == index[ 2 ] || index[ 1 ] == index[ 2 ] )
        return 0;
    // the determinant of a signed permutation: the product of the signs times the parity of the permutation
    const bool cyclic = ( ( index[ 0 ] + 1 ) % 3 == index[ 1 ] );
    return sign[ 0 ] * sign[ 1 ] * sign[ 2 ] * ( cyclic ? 1 : -1 );
}

/*-------------------------------------------------------------------------*/

THandedness
CCoordinateSystem::GetHandedness( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    // right x up == backward exactly when [ right, up, backward ] has determinant +1
    return GetBasisDeterminant() < 0 ? HANDEDNESS_LEFT : HANDEDNESS_RIGHT;
}

/*-------------------------------------------------------------------------*/

Float64
CCoordinateSystem::GetUnitsPerMetre( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_unitsPerMetre;
}

/*-------------------------------------------------------------------------*/

bool
CCoordinateSystem::IsValid( void ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return (UInt32) m_right < 6 && (UInt32) m_up < 6 && (UInt32) m_forward < 6 && 0 != GetBasisDeterminant() &&
           m_unitsPerMetre > 0.0 && CTNumericTraits< Float64 >::IsFinite( m_unitsPerMetre );
}

/*-------------------------------------------------------------------------*/

void
CCoordinateSystem::GetConversionPermutation( const CCoordinateSystem& target, Int32 ( &columnMajor )[ 9 ] ) const GUCEF_MATH_REALTIME_CONTEXT
{
    UInt32 sourceIndex[ 3 ];
    Int32 sourceSign[ 3 ];
    UInt32 targetIndex[ 3 ];
    Int32 targetSign[ 3 ];
    GetBasis( m_right, m_up, m_forward, sourceIndex, sourceSign );
    GetBasis( target.m_right, target.m_up, target.m_forward, targetIndex, targetSign );
    for ( UInt32 i=0; i<9; ++i )
        columnMajor[ i ] = 0;
    // P = B_target B_source^T: P( r, c ) = sum over the basis vectors k of B_target( r, k ) B_source( c, k )
    for ( UInt32 k=0; k<3; ++k )
        columnMajor[ sourceIndex[ k ] * 3 + targetIndex[ k ] ] += targetSign[ k ] * sourceSign[ k ];
}

/*-------------------------------------------------------------------------*/

Float64
CCoordinateSystem::GetConversionScale( const CCoordinateSystem& target ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return target.m_unitsPerMetre / m_unitsPerMetre;
}

/*-------------------------------------------------------------------------*/

bool
CCoordinateSystem::ConversionFlipsWinding( const CCoordinateSystem& target ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return GetBasisDeterminant() * target.GetBasisDeterminant() < 0;
}

/*-------------------------------------------------------------------------*/

void
CCoordinateSystem::GetAxisVector( TSignedAxis axis, Int32 ( &vector )[ 3 ] ) GUCEF_MATH_REALTIME_CONTEXT
{
    vector[ 0 ] = 0;
    vector[ 1 ] = 0;
    vector[ 2 ] = 0;
    vector[ SignedAxisIndex( axis ) ] = SignedAxisSign( axis );
}

/*-------------------------------------------------------------------------*/

bool
CCoordinateSystem::operator==( const CCoordinateSystem& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return m_right == other.m_right && m_up == other.m_up && m_forward == other.m_forward && m_unitsPerMetre == other.m_unitsPerMetre;
}

/*-------------------------------------------------------------------------*/

bool
CCoordinateSystem::operator!=( const CCoordinateSystem& other ) const GUCEF_MATH_REALTIME_CONTEXT
{
    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

const char*
CCoordinateSystem::GetAxisName( TSignedAxis axis )
{
    static const char* const names[ 6 ] = { "+X", "-X", "+Y", "-Y", "+Z", "-Z" };
    return (UInt32) axis < 6 ? names[ axis ] : names[ 0 ];
}

/*-------------------------------------------------------------------------*/

bool
CCoordinateSystem::AxisFromName( const CORE::CString& name, TSignedAxis& axis )
{
    for ( UInt32 i=0; i<6; ++i )
    {
        if ( name == GetAxisName( (TSignedAxis) i ) )
        {
            axis = (TSignedAxis) i;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCoordinateSystem::ToString( void ) const
{
    CORE::CString text( "(" );
    text += GetAxisName( m_right );
    text += ", ";
    text += GetAxisName( m_up );
    text += ", ";
    text += GetAxisName( m_forward );
    text += ", ";
    text += CTNumericTraits< Float64 >::ToString( m_unitsPerMetre );
    text += ')';
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CCoordinateSystem::FromString( const CORE::CString& text )
{
    const char* data = text.C_String();
    UInt32 length = text.Length();
    if ( GUCEF_NULL == data )
        return false;
    // "(a, b, c, units)": split at the commas inside the parentheses
    UInt32 first = 0;
    UInt32 last = length;
    while ( first < last && IsComponentTextWhitespace( data[ first ] ) )
        ++first;
    while ( last > first && IsComponentTextWhitespace( data[ last - 1 ] ) )
        --last;
    if ( last - first < 2 || '(' != data[ first ] || ')' != data[ last - 1 ] )
        return false;
    CORE::CString parts[ 4 ];
    UInt32 count = 0;
    UInt32 start = first + 1;
    for ( UInt32 i=first + 1; i<=last - 1; ++i )
    {
        if ( i == last - 1 || ',' == data[ i ] )
        {
            UInt32 a = start;
            UInt32 b = i;
            while ( a < b && IsComponentTextWhitespace( data[ a ] ) )
                ++a;
            while ( b > a && IsComponentTextWhitespace( data[ b - 1 ] ) )
                --b;
            if ( a == b || count >= 4 )
                return false;
            parts[ count++ ] = CORE::CString( data + a, b - a );
            start = i + 1;
        }
    }
    TSignedAxis right = SIGNEDAXIS_POSITIVE_X;
    TSignedAxis up = SIGNEDAXIS_POSITIVE_Y;
    TSignedAxis forward = SIGNEDAXIS_NEGATIVE_Z;
    Float64 units = 1.0;
    if ( 4 != count || !AxisFromName( parts[ 0 ], right ) || !AxisFromName( parts[ 1 ], up ) || !AxisFromName( parts[ 2 ], forward ) ||
         !CTNumericTraits< Float64 >::FromString( parts[ 3 ], units ) )
        return false;
    CCoordinateSystem parsed;
    parsed.m_right = right;
    parsed.m_up = up;
    parsed.m_forward = forward;
    parsed.m_unitsPerMetre = units;
    if ( !parsed.IsValid() )
        return false;
    *this = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCoordinateSystem::Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ ) const
{
    return node.SetAttribute( "right", CORE::CVariant( GetAxisName( m_right ) ) ) &&
           node.SetAttribute( "up", CORE::CVariant( GetAxisName( m_up ) ) ) &&
           node.SetAttribute( "forward", CORE::CVariant( GetAxisName( m_forward ) ) ) &&
           node.SetAttribute( "unitsPerMetre", CORE::CVariant( m_unitsPerMetre ) );
}

/*-------------------------------------------------------------------------*/

bool
CCoordinateSystem::Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& /*settings*/ )
{
    const CORE::CDataNode::TKeyValuePair* right = node.GetAttribute( "right" );
    const CORE::CDataNode::TKeyValuePair* up = node.GetAttribute( "up" );
    const CORE::CDataNode::TKeyValuePair* forward = node.GetAttribute( "forward" );
    const CORE::CDataNode::TKeyValuePair* units = node.GetAttribute( "unitsPerMetre" );
    if ( GUCEF_NULL == right || GUCEF_NULL == up || GUCEF_NULL == forward || GUCEF_NULL == units )
        return false;
    CCoordinateSystem parsed;
    if ( !AxisFromName( right->second.AsString(), parsed.m_right ) || !AxisFromName( up->second.AsString(), parsed.m_up ) ||
         !AxisFromName( forward->second.AsString(), parsed.m_forward ) || !ComponentFromVariant( units->second, parsed.m_unitsPerMetre ) )
        return false;
    if ( !parsed.IsValid() )
        return false;
    *this = parsed;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
