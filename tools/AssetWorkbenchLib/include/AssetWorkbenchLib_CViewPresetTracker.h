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

#ifndef ASSETWORKBENCHLIB_CVIEWPRESETTRACKER_H
#define ASSETWORKBENCHLIB_CVIEWPRESETTRACKER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETWORKBENCHLIB_CORBITCONTROLLER_H
#include "AssetWorkbenchLib_COrbitController.h"
#define ASSETWORKBENCHLIB_CORBITCONTROLLER_H
#endif /* ASSETWORKBENCHLIB_CORBITCONTROLLER_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Follows the camera of a view and says which preset of the view list it is at: a preset is the label only while the camera
 *  equals the preset within a small tolerance, any orbit, pan, zoom or numeric change makes the label "Custom", and going back to
 *  the preset makes it the name of the preset again. The presets are Frame (the bounds framed from the angles the camera had when
 *  Frame was chosen), the fixed views Front, Side, Top and Three quarter, and Four views.
 *
 *  Four views is a layout of the stage, one perspective view with three orthographic ones, whose perspective camera is the
 *  three quarter view of the bounds; while that camera is not at it the label is "Custom 1/4" (a custom camera in the first view
 *  of four) and the layout stays. Choosing another preset leaves the four views and goes to the single view.
 *
 *  The tracker reads the controller and never changes it except in Apply(), so the label changes without a preset being applied
 *  again: the application must not apply a preset because of a label it set itself.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CViewPresetTracker
{
    public:

    /** The presets in the order of the view list; the index of a preset is its choice in the list */
    enum EPreset
    {
        PRESET_CUSTOM = -1      ,
        PRESET_FRAME = 0        ,
        PRESET_FRONT            ,
        PRESET_SIDE             ,
        PRESET_TOP              ,
        PRESET_THREE_QUARTER    ,
        PRESET_FOUR_VIEWS       ,
        PRESET_COUNT
    };

    /** How far a camera may be from a preset to be at it: radians for the angles, a part of the radius of the bounds and of the distance for the rest */
    static Float32 GetTolerance( void );

    static const char* GetPresetName( EPreset preset );

    /** The preset of a name, without regard for the case; "quad" is Four views and "custom" is none */
    static bool TryParsePreset( const CORE::CString& name, EPreset& preset );

    CViewPresetTracker( void );

    /** Puts the camera at the preset (Frame, a fixed view, or the three quarter view of the four views) and takes it as the preset */
    void Apply( EPreset preset, COrbitController& controller );

    /** Reads the camera and finds out the preset it is at; true when the label or the layout changed */
    bool Update( const COrbitController& controller );

    /** The preset the camera is at, PRESET_CUSTOM when it is at none */
    EPreset GetPreset( void ) const;

    bool IsCustom( void ) const;

    /** Whether the stage shows four views: Four views and Custom 1/4 */
    bool IsFourViews( void ) const;

    /** The preset the camera left, or is at: what Apply() was given last */
    EPreset GetBasePreset( void ) const;

    /** The text of the view list: the name of the preset, "Custom", or "Custom 1/4" in the four views */
    CORE::CString GetLabel( void ) const;

    private:

    bool IsAt( EPreset preset, const COrbitController& controller ) const;

    private:

    EPreset m_preset;
    EPreset m_base;
    bool m_fourViews;
    COrbitController::SCameraState m_frameReference;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CVIEWPRESETTRACKER_H ? */
