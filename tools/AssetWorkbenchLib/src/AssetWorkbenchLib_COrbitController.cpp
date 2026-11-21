/*
 *  AssetWorkbenchLib: orbit, pan, zoom and frame controller of a 3D view
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

#include <math.h>

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#include "AssetWorkbenchLib_COrbitController.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const Float32 Pi = 3.14159265358979f;
const Float32 MaxPitch = 1.5533430f;                 // 89 degrees: the view never flips over the pole
const Float32 OrbitRadiansPerPixel = 0.0087f;
const Float32 WheelZoomFactor = 0.88f;               // per notch
const Float32 FramingMargin = 1.12f;

COrbitController::SVector3
Cross( const COrbitController::SVector3& a, const COrbitController::SVector3& b )
{
    return COrbitController::SVector3( a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x );
}

Float32
Dot( const COrbitController::SVector3& a, const COrbitController::SVector3& b )
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

COrbitController::SVector3
Normalize( const COrbitController::SVector3& v )
{
    const Float32 length = sqrtf( Dot( v, v ) );
    if ( length < 1e-12f )
        return COrbitController::SVector3( 0.0f, 0.0f, -1.0f );
    return COrbitController::SVector3( v.x / length, v.y / length, v.z / length );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COrbitController::COrbitController( void )
    : GUI::CIViewportController()
    , m_target()
    , m_boundsCenter()
    , m_boundsRadius( 1.0f )
    , m_hasBox( false )
    , m_minimumFar( 0.0f )
    , m_centreCovered( false )
    , m_boundsMinimum()
    , m_boundsMaximum()
    , m_fillFraction( 0.9f )
    , m_yaw( 0.0f )
    , m_pitch( 0.0f )
    , m_distance( 3.0f )
    , m_fieldOfView( 45.0f )
    , m_aspect( 1.0f )
    , m_surfaceHeight( 1 )
    , m_lastX( 0 )
    , m_lastY( 0 )
    , m_dragging( false )
    , m_changed( true )
{GUCEF_TRACE;

    ApplyPreset( VIEWPRESET_THREE_QUARTER );
}

/*-------------------------------------------------------------------------*/

COrbitController::~COrbitController()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
COrbitController::SetFieldOfView( Float32 degrees )
{GUCEF_TRACE;

    if ( degrees < 5.0f )
        degrees = 5.0f;
    else
    if ( degrees > 120.0f )
        degrees = 120.0f;
    m_fieldOfView = degrees;
    m_changed = true;
}

/*-------------------------------------------------------------------------*/

Float32
COrbitController::GetFieldOfView( void ) const
{GUCEF_TRACE;

    return m_fieldOfView;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::SetTarget( const SVector3& target )
{GUCEF_TRACE;

    m_target = target;
    m_changed = true;
}

/*-------------------------------------------------------------------------*/

const COrbitController::SVector3&
COrbitController::GetTarget( void ) const
{GUCEF_TRACE;

    return m_target;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::SetAngles( Float32 yaw, Float32 pitch )
{GUCEF_TRACE;

    m_yaw = yaw;
    m_pitch = pitch;
    ClampAngles();
    m_changed = true;
}

/*-------------------------------------------------------------------------*/

Float32
COrbitController::GetYaw( void ) const
{GUCEF_TRACE;

    return m_yaw;
}

/*-------------------------------------------------------------------------*/

Float32
COrbitController::GetPitch( void ) const
{GUCEF_TRACE;

    return m_pitch;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::SetDistance( Float32 distance )
{GUCEF_TRACE;

    const Float32 minimum = m_boundsRadius * 0.02f;
    m_distance = distance < minimum ? minimum : distance;
    m_changed = true;
}

/*-------------------------------------------------------------------------*/

Float32
COrbitController::GetDistance( void ) const
{GUCEF_TRACE;

    return m_distance;
}

/*-------------------------------------------------------------------------*/

COrbitController::SVector3
COrbitController::GetEye( void ) const
{GUCEF_TRACE;

    const Float32 horizontal = cosf( m_pitch );
    return SVector3( m_target.x + m_distance * horizontal * sinf( m_yaw ) ,
                     m_target.y + m_distance * sinf( m_pitch )            ,
                     m_target.z + m_distance * horizontal * cosf( m_yaw ) );
}

/*-------------------------------------------------------------------------*/

void
COrbitController::GetBasis( SVector3& forward, SVector3& right, SVector3& up ) const
{GUCEF_TRACE;

    const SVector3 eye = GetEye();
    forward = Normalize( SVector3( m_target.x - eye.x, m_target.y - eye.y, m_target.z - eye.z ) );
    right = Normalize( Cross( forward, SVector3( 0.0f, 1.0f, 0.0f ) ) );
    up = Cross( right, forward );
}

/*-------------------------------------------------------------------------*/

void
COrbitController::SetAspectRatio( Float32 aspect )
{GUCEF_TRACE;

    if ( aspect < 0.05f )
        aspect = 0.05f;
    if ( aspect != m_aspect )
    {
        m_aspect = aspect;
        m_changed = true;
    }
}

/*-------------------------------------------------------------------------*/

Float32
COrbitController::GetAspectRatio( void ) const
{GUCEF_TRACE;

    return m_aspect;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::SetBounds( const SVector3& center, Float32 radius )
{GUCEF_TRACE;

    m_boundsCenter = center;
    m_boundsRadius = radius < 1e-6f ? 1e-6f : radius;
    m_hasBox = false;
    m_centreCovered = false;
    m_minimumFar = 0.0f;
    Frame();
}

/*-------------------------------------------------------------------------*/

void
COrbitController::SetBoundsBox( const SVector3& minimum, const SVector3& maximum, Float32 fillFraction, bool centreCovered )
{GUCEF_TRACE;

    m_boundsMinimum = SVector3( minimum.x < maximum.x ? minimum.x : maximum.x, minimum.y < maximum.y ? minimum.y : maximum.y, minimum.z < maximum.z ? minimum.z : maximum.z );
    m_boundsMaximum = SVector3( minimum.x < maximum.x ? maximum.x : minimum.x, minimum.y < maximum.y ? maximum.y : minimum.y, minimum.z < maximum.z ? maximum.z : minimum.z );
    m_boundsCenter = SVector3( 0.5f * ( m_boundsMinimum.x + m_boundsMaximum.x ), 0.5f * ( m_boundsMinimum.y + m_boundsMaximum.y ), 0.5f * ( m_boundsMinimum.z + m_boundsMaximum.z ) );
    const Float32 dx = m_boundsMaximum.x - m_boundsMinimum.x, dy = m_boundsMaximum.y - m_boundsMinimum.y, dz = m_boundsMaximum.z - m_boundsMinimum.z;
    m_boundsRadius = 0.5f * sqrtf( dx*dx + dy*dy + dz*dz );
    if ( m_boundsRadius < 1e-6f )
        m_boundsRadius = 1e-6f;
    m_fillFraction = fillFraction > 0.0f ? ( fillFraction > 1.0f ? 1.0f : fillFraction ) : 0.9f;
    m_hasBox = true;
    m_centreCovered = centreCovered;
    Frame();
}

/*-------------------------------------------------------------------------*/

void COrbitController::SetMinimumFarClip( Float32 distance ) { m_minimumFar = distance > 0.0f ? distance : 0.0f; m_changed = true; }
bool COrbitController::HasBoundsBox( void ) const { return m_hasBox; }
bool COrbitController::IsBoxCentredOnCovered( void ) const { return m_hasBox && m_centreCovered; }
const COrbitController::SVector3& COrbitController::GetBoundsMinimum( void ) const { return m_boundsMinimum; }
const COrbitController::SVector3& COrbitController::GetBoundsMaximum( void ) const { return m_boundsMaximum; }
Float32 COrbitController::GetFillFraction( void ) const { return m_fillFraction; }

/*-------------------------------------------------------------------------*/

Float32
COrbitController::GetFramingDistanceFor( Float32 yaw, Float32 pitch ) const
{GUCEF_TRACE;

    if ( !m_hasBox )
        return GetFramingDistance( m_boundsRadius );

    // the basis of a camera at these angles, looking at the middle of the box
    const Float32 horizontal = cosf( pitch );
    const SVector3 forward = Normalize( SVector3( -horizontal * sinf( yaw ), -sinf( pitch ), -horizontal * cosf( yaw ) ) );
    const SVector3 right = Normalize( Cross( forward, SVector3( 0.0f, 1.0f, 0.0f ) ) );
    const SVector3 up = Cross( right, forward );

    // every corner has to be inside the view: |x| <= fill * tan(half horizontal) * (distance + depth), the same for y
    const Float32 tanVertical = tanf( 0.5f * m_fieldOfView * Pi / 180.0f );
    const Float32 tanHorizontal = tanVertical * m_aspect;
    Float32 distance = 0.0f;
    for ( UInt32 corner=0; corner<8; ++corner )
    {
        const SVector3 c( ( corner & 1 ? m_boundsMaximum.x : m_boundsMinimum.x ) - m_boundsCenter.x,
                          ( corner & 2 ? m_boundsMaximum.y : m_boundsMinimum.y ) - m_boundsCenter.y,
                          ( corner & 4 ? m_boundsMaximum.z : m_boundsMinimum.z ) - m_boundsCenter.z );
        const Float32 x = fabsf( Dot( c, right ) );
        const Float32 y = fabsf( Dot( c, up ) );
        const Float32 depth = Dot( c, forward );
        const Float32 forX = x / ( m_fillFraction * tanHorizontal ) - depth;
        const Float32 forY = y / ( m_fillFraction * tanVertical ) - depth;
        if ( forX > distance )
            distance = forX;
        if ( forY > distance )
            distance = forY;
    }
    // the nearest corner stays in front of the camera
    const Float32 minimum = 1.05f * m_boundsRadius;
    return distance < minimum ? minimum : distance;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::GetFramedView( Float32 yaw, Float32 pitch, SVector3& target, Float32& distance ) const
{GUCEF_TRACE;

    target = m_boundsCenter;
    distance = GetFramingDistanceFor( yaw, pitch );
    if ( !m_hasBox || !m_centreCovered )
        return;

    const Float32 horizontal = cosf( pitch );
    const SVector3 forward = Normalize( SVector3( -horizontal * sinf( yaw ), -sinf( pitch ), -horizontal * cosf( yaw ) ) );
    const SVector3 right = Normalize( Cross( forward, SVector3( 0.0f, 1.0f, 0.0f ) ) );
    const SVector3 up = Cross( right, forward );
    const Float32 tanVertical = tanf( 0.5f * m_fieldOfView * Pi / 180.0f );
    const Float32 tanHorizontal = tanVertical * m_aspect;

    // a few rounds: centre what the corners cover, then bring it to the size asked
    for ( UInt32 round=0; round<6; ++round )
    {
        const SVector3 eye( target.x - distance * forward.x, target.y - distance * forward.y, target.z - distance * forward.z );
        Float32 lowX = 1e30f, highX = -1e30f, lowY = 1e30f, highY = -1e30f;
        for ( UInt32 corner=0; corner<8; ++corner )
        {
            const SVector3 c( ( corner & 1 ? m_boundsMaximum.x : m_boundsMinimum.x ) - eye.x,
                              ( corner & 2 ? m_boundsMaximum.y : m_boundsMinimum.y ) - eye.y,
                              ( corner & 4 ? m_boundsMaximum.z : m_boundsMinimum.z ) - eye.z );
            const Float32 depth = Dot( c, forward );
            if ( depth < 1e-4f )
                return;         // a corner behind the camera: what the symmetric fit gave stays
            const Float32 x = Dot( c, right ) / ( depth * tanHorizontal );
            const Float32 y = Dot( c, up ) / ( depth * tanVertical );
            lowX = x < lowX ? x : lowX;
            highX = x > highX ? x : highX;
            lowY = y < lowY ? y : lowY;
            highY = y > highY ? y : highY;
        }
        const Float32 centreX = 0.5f * ( lowX + highX ), centreY = 0.5f * ( lowY + highY );
        const Float32 halfX = 0.5f * ( highX - lowX ), halfY = 0.5f * ( highY - lowY );
        // move the target by what the centre is off, as seen at the distance of the target
        target.x += ( centreX * tanHorizontal * right.x + centreY * tanVertical * up.x ) * distance;
        target.y += ( centreX * tanHorizontal * right.y + centreY * tanVertical * up.y ) * distance;
        target.z += ( centreX * tanHorizontal * right.z + centreY * tanVertical * up.z ) * distance;
        const Float32 largest = halfX > halfY ? halfX : halfY;
        if ( largest > 1e-6f )
            distance *= largest / m_fillFraction;
    }
}

/*-------------------------------------------------------------------------*/

Float32
COrbitController::GetFilledFraction( Float32 width, Float32 height, Float32 extent, Float32 aspect )
{GUCEF_TRACE;

    // an orthographic view is 'extent' high and 'extent * aspect' wide
    if ( extent <= 0.0f || aspect <= 0.0f )
        return 0.0f;
    const Float32 vertical = height / extent;
    const Float32 horizontal = width / ( extent * aspect );
    return vertical > horizontal ? vertical : horizontal;
}

/*-------------------------------------------------------------------------*/

const COrbitController::SVector3&
COrbitController::GetBoundsCenter( void ) const
{GUCEF_TRACE;

    return m_boundsCenter;
}

/*-------------------------------------------------------------------------*/

Float32
COrbitController::GetBoundsRadius( void ) const
{GUCEF_TRACE;

    return m_boundsRadius;
}

/*-------------------------------------------------------------------------*/

Float32
COrbitController::GetFramingDistance( Float32 radius ) const
{GUCEF_TRACE;

    // the sphere has to fit the narrower of the vertical and the horizontal field of view
    const Float32 halfVertical = 0.5f * m_fieldOfView * Pi / 180.0f;
    const Float32 halfHorizontal = atanf( tanf( halfVertical ) * m_aspect );
    const Float32 half = halfVertical < halfHorizontal ? halfVertical : halfHorizontal;
    return FramingMargin * radius / sinf( half );
}

/*-------------------------------------------------------------------------*/

void
COrbitController::Frame( void )
{GUCEF_TRACE;

    GetFramedView( m_yaw, m_pitch, m_target, m_distance );
    m_changed = true;
}

/*-------------------------------------------------------------------------*/

bool
COrbitController::IsSphereInView( const SVector3& center, Float32 radius ) const
{GUCEF_TRACE;

    const SVector3 eye = GetEye();
    SVector3 forward, right, up;
    GetBasis( forward, right, up );
    const SVector3 toCenter( center.x - eye.x, center.y - eye.y, center.z - eye.z );
    const Float32 distance = sqrtf( Dot( toCenter, toCenter ) );
    if ( distance <= radius )
        return false;

    const Float32 halfVertical = 0.5f * m_fieldOfView * Pi / 180.0f;
    const Float32 halfHorizontal = atanf( tanf( halfVertical ) * m_aspect );
    const Float32 half = halfVertical < halfHorizontal ? halfVertical : halfHorizontal;

    Float32 cosine = Dot( toCenter, forward ) / distance;
    if ( cosine > 1.0f )
        cosine = 1.0f;
    else
    if ( cosine < -1.0f )
        cosine = -1.0f;
    return acosf( cosine ) + asinf( radius / distance ) <= half;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::ApplyPreset( EViewPreset preset )
{GUCEF_TRACE;

    switch ( preset )
    {
        case VIEWPRESET_FRONT:          { m_yaw = 0.0f;          m_pitch = 0.0f;           break; }
        case VIEWPRESET_SIDE:           { m_yaw = 0.5f * Pi;     m_pitch = 0.0f;           break; }
        case VIEWPRESET_TOP:            { m_yaw = 0.0f;          m_pitch = MaxPitch;       break; }
        default:                        { m_yaw = 0.25f * Pi;    m_pitch = 0.4363323f;     break; }   // 25 degrees up
    }
    Frame();
}

/*-------------------------------------------------------------------------*/

COrbitController::SCameraState
COrbitController::GetState( void ) const
{GUCEF_TRACE;

    SCameraState state;
    state.target = m_target;
    state.yaw = m_yaw;
    state.pitch = m_pitch;
    state.distance = m_distance;
    return state;
}

/*-------------------------------------------------------------------------*/

COrbitController::SCameraState
COrbitController::GetFrameState( void ) const
{GUCEF_TRACE;

    SCameraState state = GetState();
    GetFramedView( state.yaw, state.pitch, state.target, state.distance );
    return state;
}

/*-------------------------------------------------------------------------*/

COrbitController::SCameraState
COrbitController::GetPresetState( EViewPreset preset ) const
{GUCEF_TRACE;

    SCameraState state = GetFrameState();
    switch ( preset )
    {
        case VIEWPRESET_FRONT:          { state.yaw = 0.0f;          state.pitch = 0.0f;           break; }
        case VIEWPRESET_SIDE:           { state.yaw = 0.5f * Pi;     state.pitch = 0.0f;           break; }
        case VIEWPRESET_TOP:            { state.yaw = 0.0f;          state.pitch = MaxPitch;       break; }
        default:                        { state.yaw = 0.25f * Pi;    state.pitch = 0.4363323f;     break; }
    }
    // the box is seen from the angles of the preset, not from those the camera has now
    GetFramedView( state.yaw, state.pitch, state.target, state.distance );
    return state;
}

/*-------------------------------------------------------------------------*/

bool
COrbitController::IsSameView( const SCameraState& a, const SCameraState& b, Float32 tolerance ) const
{GUCEF_TRACE;

    // the angle between two yaws, going the short way round the circle
    Float32 yaw = a.yaw - b.yaw;
    while ( yaw > Pi )
        yaw -= 2.0f * Pi;
    while ( yaw < -Pi )
        yaw += 2.0f * Pi;
    if ( yaw > tolerance || yaw < -tolerance )
        return false;
    // at the pole the yaw means nothing: the top view has one look whatever the yaw is
    const Float32 pitch = a.pitch - b.pitch;
    if ( pitch > tolerance || pitch < -tolerance )
        return false;
    const SVector3 offset( a.target.x - b.target.x, a.target.y - b.target.y, a.target.z - b.target.z );
    const Float32 allowed = tolerance * m_boundsRadius;
    if ( Dot( offset, offset ) > allowed * allowed )
        return false;
    const Float32 bigger = a.distance > b.distance ? a.distance : b.distance;
    const Float32 difference = a.distance > b.distance ? a.distance - b.distance : b.distance - a.distance;
    return difference <= tolerance * bigger;
}

/*-------------------------------------------------------------------------*/

Float32
COrbitController::GetNearClipDistance( void ) const
{GUCEF_TRACE;

    const Float32 nearClip = 0.01f * m_distance;
    const Float32 minimum = 0.001f * m_boundsRadius;
    return nearClip < minimum ? minimum : nearClip;
}

/*-------------------------------------------------------------------------*/

Float32
COrbitController::GetFarClipDistance( void ) const
{GUCEF_TRACE;

    const Float32 far = 20.0f * ( m_distance + m_boundsRadius );
    return far > m_minimumFar ? far : m_minimumFar;
}

/*-------------------------------------------------------------------------*/

bool
COrbitController::ConsumeChanged( void )
{GUCEF_TRACE;

    const bool changed = m_changed;
    m_changed = false;
    return changed;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::ClampAngles( void )
{GUCEF_TRACE;

    if ( m_pitch > MaxPitch )
        m_pitch = MaxPitch;
    else
    if ( m_pitch < -MaxPitch )
        m_pitch = -MaxPitch;
    while ( m_yaw > Pi )
        m_yaw -= 2.0f * Pi;
    while ( m_yaw < -Pi )
        m_yaw += 2.0f * Pi;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::Orbit( Int32 deltaX, Int32 deltaY )
{GUCEF_TRACE;

    m_yaw -= (Float32) deltaX * OrbitRadiansPerPixel;
    m_pitch += (Float32) deltaY * OrbitRadiansPerPixel;
    ClampAngles();
    m_changed = true;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::Pan( Int32 deltaX, Int32 deltaY )
{GUCEF_TRACE;

    SVector3 forward, right, up;
    GetBasis( forward, right, up );
    const Float32 halfVertical = 0.5f * m_fieldOfView * Pi / 180.0f;
    const Float32 perPixel = 2.0f * m_distance * tanf( halfVertical ) / (Float32) ( m_surfaceHeight > 0 ? m_surfaceHeight : 1 );
    m_target.x += ( -(Float32) deltaX * right.x + (Float32) deltaY * up.x ) * perPixel;
    m_target.y += ( -(Float32) deltaX * right.y + (Float32) deltaY * up.y ) * perPixel;
    m_target.z += ( -(Float32) deltaX * right.z + (Float32) deltaY * up.z ) * perPixel;
    m_changed = true;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::OnPointerMove( Int32 x, Int32 y, UInt32 buttonStates, UInt32 modifiers )
{GUCEF_TRACE;

    const bool left = 0 != ( buttonStates & ( 1U << GUI::MOUSEBUTTON_LEFT ) );
    const bool panButton = 0 != ( buttonStates & ( ( 1U << GUI::MOUSEBUTTON_MIDDLE ) | ( 1U << GUI::MOUSEBUTTON_RIGHT ) ) );
    if ( !m_dragging || ( !left && !panButton ) )
    {
        m_lastX = x;
        m_lastY = y;
        return;
    }
    const Int32 deltaX = x - m_lastX;
    const Int32 deltaY = y - m_lastY;
    m_lastX = x;
    m_lastY = y;
    if ( panButton || ( left && 0 != ( modifiers & INPUT::KEYMOD_SHIFT ) ) )
        Pan( deltaX, deltaY );
    else
        Orbit( deltaX, deltaY );
}

/*-------------------------------------------------------------------------*/

void
COrbitController::OnPointerButton( Int32 x, Int32 y, GUI::TMouseButton button, bool pressed, UInt32 clickCount, UInt32 modifiers )
{GUCEF_TRACE;

    m_lastX = x;
    m_lastY = y;
    m_dragging = pressed;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::OnWheel( Int32 x, Int32 y, Int32 delta, UInt32 modifiers )
{GUCEF_TRACE;

    Float32 distance = m_distance;
    for ( Int32 i=0; i<( delta < 0 ? -delta : delta ); ++i )
        distance = delta > 0 ? distance * WheelZoomFactor : distance / WheelZoomFactor;
    SetDistance( distance );
}

/*-------------------------------------------------------------------------*/

void
COrbitController::OnKey( INPUT::KeyCode keyCode, bool pressed, UInt32 modifiers )
{GUCEF_TRACE;

    if ( !pressed )
        return;
    switch ( keyCode )
    {
        case INPUT::KEYCODE_F:      { Frame(); break; }
        case INPUT::KEYCODE_1:      { ApplyPreset( VIEWPRESET_FRONT ); break; }
        case INPUT::KEYCODE_2:      { ApplyPreset( VIEWPRESET_SIDE ); break; }
        case INPUT::KEYCODE_3:      { ApplyPreset( VIEWPRESET_TOP ); break; }
        case INPUT::KEYCODE_4:      { ApplyPreset( VIEWPRESET_THREE_QUARTER ); break; }
        default:                    { break; }
    }
}

/*-------------------------------------------------------------------------*/

void
COrbitController::OnPointerCaptureLost( void )
{GUCEF_TRACE;

    m_dragging = false;
}

/*-------------------------------------------------------------------------*/

void
COrbitController::OnSurfaceResized( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    m_surfaceHeight = height;
    if ( width > 0 && height > 0 )
        SetAspectRatio( (Float32) width / (Float32) height );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
