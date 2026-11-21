/*
 *  AssetWorkbenchLib: the name of the view preset of a camera, which says Custom as soon as the camera left the preset
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

#include "AssetWorkbenchLib_CViewPresetTracker.h"

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

// a pixel of the orbit is 0.0087 radians: one pixel of orbit is already Custom
const Float32 PresetTolerance = 0.004f;

const char* const PresetNames[ CViewPresetTracker::PRESET_COUNT ] =
{
    "Frame", "Front", "Side", "Top", "Three quarter", "Four views"
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

Float32
CViewPresetTracker::GetTolerance( void )
{GUCEF_TRACE;

    return PresetTolerance;
}

/*-------------------------------------------------------------------------*/

const char*
CViewPresetTracker::GetPresetName( EPreset preset )
{GUCEF_TRACE;

    if ( (Int32) preset < 0 || (Int32) preset >= (Int32) PRESET_COUNT )
        return "Custom";
    return PresetNames[ preset ];
}

/*-------------------------------------------------------------------------*/

bool
CViewPresetTracker::TryParsePreset( const CORE::CString& name, EPreset& preset )
{GUCEF_TRACE;

    for ( Int32 i=0; i<(Int32) PRESET_COUNT; ++i )
    {
        if ( name.Equals( PresetNames[ i ], false ) )
        {
            preset = static_cast< EPreset >( i );
            return true;
        }
    }
    if ( name.Equals( "quad", false ) || name.Equals( "four", false ) )
    {
        preset = PRESET_FOUR_VIEWS;
        return true;
    }
    if ( name.Equals( "threequarter", false ) || name.Equals( "three-quarter", false ) )
    {
        preset = PRESET_THREE_QUARTER;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CViewPresetTracker::CViewPresetTracker( void )
    : m_preset( PRESET_FRAME )
    , m_base( PRESET_FRAME )
    , m_fourViews( false )
    , m_frameReference()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CViewPresetTracker::Apply( EPreset preset, COrbitController& controller )
{GUCEF_TRACE;

    switch ( preset )
    {
        case PRESET_FRONT:          { controller.ApplyPreset( COrbitController::VIEWPRESET_FRONT ); break; }
        case PRESET_SIDE:           { controller.ApplyPreset( COrbitController::VIEWPRESET_SIDE ); break; }
        case PRESET_TOP:            { controller.ApplyPreset( COrbitController::VIEWPRESET_TOP ); break; }
        case PRESET_THREE_QUARTER:  { controller.ApplyPreset( COrbitController::VIEWPRESET_THREE_QUARTER ); break; }
        case PRESET_FOUR_VIEWS:     { controller.ApplyPreset( COrbitController::VIEWPRESET_THREE_QUARTER ); break; }
        default:
        {
            // the bounds from the angles the camera has now: those are the angles Frame is at
            controller.Frame();
            preset = PRESET_FRAME;
            break;
        }
    }
    m_base = preset;
    m_preset = preset;
    m_fourViews = PRESET_FOUR_VIEWS == preset;
    m_frameReference = controller.GetState();
}

/*-------------------------------------------------------------------------*/

bool
CViewPresetTracker::IsAt( EPreset preset, const COrbitController& controller ) const
{GUCEF_TRACE;

    const COrbitController::SCameraState now = controller.GetState();
    COrbitController::SCameraState wanted;
    switch ( preset )
    {
        case PRESET_FRONT:          { wanted = controller.GetPresetState( COrbitController::VIEWPRESET_FRONT ); break; }
        case PRESET_SIDE:           { wanted = controller.GetPresetState( COrbitController::VIEWPRESET_SIDE ); break; }
        case PRESET_TOP:            { wanted = controller.GetPresetState( COrbitController::VIEWPRESET_TOP ); break; }
        case PRESET_THREE_QUARTER:
        case PRESET_FOUR_VIEWS:     { wanted = controller.GetPresetState( COrbitController::VIEWPRESET_THREE_QUARTER ); break; }
        default:
        {
            wanted = controller.GetFrameState();
            wanted.yaw = m_frameReference.yaw;
            wanted.pitch = m_frameReference.pitch;
            break;
        }
    }
    return controller.IsSameView( now, wanted, PresetTolerance );
}

/*-------------------------------------------------------------------------*/

bool
CViewPresetTracker::Update( const COrbitController& controller )
{GUCEF_TRACE;

    const EPreset before = m_preset;
    if ( m_fourViews )
    {
        // the layout stays; only the camera of the perspective view is judged
        m_preset = IsAt( PRESET_FOUR_VIEWS, controller ) ? PRESET_FOUR_VIEWS : PRESET_CUSTOM;
        return before != m_preset;
    }

    // the preset the camera is at: the one it was put at if it still is there, else the first of the list that matches
    if ( PRESET_CUSTOM != m_base && IsAt( m_base, controller ) )
    {
        m_preset = m_base;
        return before != m_preset;
    }
    m_preset = PRESET_CUSTOM;
    for ( Int32 i=(Int32) PRESET_FRONT; i<(Int32) PRESET_THREE_QUARTER + 1; ++i )
    {
        if ( IsAt( static_cast< EPreset >( i ), controller ) )
        {
            m_preset = static_cast< EPreset >( i );
            break;
        }
    }
    if ( PRESET_CUSTOM == m_preset && IsAt( PRESET_FRAME, controller ) )
        m_preset = PRESET_FRAME;
    return before != m_preset;
}

/*-------------------------------------------------------------------------*/

CViewPresetTracker::EPreset
CViewPresetTracker::GetPreset( void ) const
{GUCEF_TRACE;

    return m_preset;
}

/*-------------------------------------------------------------------------*/

CViewPresetTracker::EPreset
CViewPresetTracker::GetBasePreset( void ) const
{GUCEF_TRACE;

    return m_base;
}

/*-------------------------------------------------------------------------*/

bool
CViewPresetTracker::IsCustom( void ) const
{GUCEF_TRACE;

    return PRESET_CUSTOM == m_preset;
}

/*-------------------------------------------------------------------------*/

bool
CViewPresetTracker::IsFourViews( void ) const
{GUCEF_TRACE;

    return m_fourViews;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CViewPresetTracker::GetLabel( void ) const
{GUCEF_TRACE;

    if ( PRESET_CUSTOM == m_preset )
        return m_fourViews ? "Custom 1/4" : "Custom";
    return GetPresetName( m_preset );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
