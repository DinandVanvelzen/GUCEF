/*
 *  AssetViewerLib: orbit, pan, zoom and frame controller of a 3D view
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

#include "AssetViewerLib_COrbitController.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

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
    Frame();
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

    m_target = m_boundsCenter;
    m_distance = GetFramingDistance( m_boundsRadius );
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

    return 20.0f * ( m_distance + m_boundsRadius );
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

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
