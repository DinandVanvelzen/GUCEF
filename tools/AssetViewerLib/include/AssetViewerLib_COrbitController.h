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

#ifndef ASSETVIEWERLIB_CORBITCONTROLLER_H
#define ASSETVIEWERLIB_CORBITCONTROLLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CIVIEWPORTCONTROLLER_H
#include "gucefGUI_CIViewportController.h"
#define GUCEF_GUI_CIVIEWPORTCONTROLLER_H
#endif /* GUCEF_GUI_CIVIEWPORTCONTROLLER_H ? */

#ifndef ASSETVIEWERLIB_MACROS_H
#include "AssetViewerLib_macros.h"
#define ASSETVIEWERLIB_MACROS_H
#endif /* ASSETVIEWERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The camera of a model view as a point looked at, a distance and two angles (y is up). It is the controller of a
 *  CViewportForm: the left button orbits, the middle or right button, or the left button with Shift, pans, the wheel moves
 *  closer and further away, F frames the bounds again and the keys 1 to 4 pick the fixed views. It does the arithmetic only and
 *  knows nothing of Ogre: the stage reads GetEye() and GetTarget() when ConsumeChanged() says that the view moved.
 */
class ASSETVIEWERLIB_PUBLIC_CPP COrbitController : public GUI::CIViewportController
{
    public:

    /** The fixed views of the capture */
    enum EViewPreset
    {
        VIEWPRESET_FRONT = 0        ,
        VIEWPRESET_SIDE             ,
        VIEWPRESET_TOP              ,
        VIEWPRESET_THREE_QUARTER    ,
        VIEWPRESET_COUNT
    };

    struct SVector3
    {
        Float32 x, y, z;
        SVector3( void ) : x( 0.0f ), y( 0.0f ), z( 0.0f ) {}
        SVector3( Float32 vx, Float32 vy, Float32 vz ) : x( vx ), y( vy ), z( vz ) {}
    };

    COrbitController( void );

    virtual ~COrbitController();

    /*
     *  The camera
     */

    /** The vertical field of view in degrees, 45 by default */
    void SetFieldOfView( Float32 degrees );
    Float32 GetFieldOfView( void ) const;

    void SetTarget( const SVector3& target );
    const SVector3& GetTarget( void ) const;

    /** The angles in radians: yaw turns about the vertical axis from +z towards +x, pitch lifts the eye above the horizon */
    void SetAngles( Float32 yaw, Float32 pitch );
    Float32 GetYaw( void ) const;
    Float32 GetPitch( void ) const;

    void SetDistance( Float32 distance );
    Float32 GetDistance( void ) const;

    /** Where the eye is: the target plus the distance along the direction the angles give */
    SVector3 GetEye( void ) const;

    /** The unit vectors of the view: forward (from the eye to the target), right and up */
    void GetBasis( SVector3& forward, SVector3& right, SVector3& up ) const;

    /** The width over the height of the surface, 1 until a size is known */
    void SetAspectRatio( Float32 aspect );
    Float32 GetAspectRatio( void ) const;

    /*
     *  Framing
     */

    /** Remembers the bounding sphere that Frame() goes back to, and frames it */
    void SetBounds( const SVector3& center, Float32 radius );
    const SVector3& GetBoundsCenter( void ) const;
    Float32 GetBoundsRadius( void ) const;

    /** Looks at the center of the bounds from a distance at which the whole sphere is in view with a little room left */
    void Frame( void );

    /** The distance at which a sphere of this radius fills the narrower of the two field of views, with the margin */
    Float32 GetFramingDistance( Float32 radius ) const;

    /** Whether a whole sphere is inside the view cone of the camera */
    bool IsSphereInView( const SVector3& center, Float32 radius ) const;

    /** Looks at the bounds from a fixed view and frames them */
    void ApplyPreset( EViewPreset preset );

    /** The clip planes that suit the bounds as they are framed now */
    Float32 GetNearClipDistance( void ) const;
    Float32 GetFarClipDistance( void ) const;

    /** Whether the view moved since this was called last */
    bool ConsumeChanged( void );

    /*
     *  CIViewportController
     */

    virtual void OnPointerMove( Int32 x, Int32 y, UInt32 buttonStates, UInt32 modifiers ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerButton( Int32 x, Int32 y, GUI::TMouseButton button, bool pressed, UInt32 clickCount, UInt32 modifiers ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnWheel( Int32 x, Int32 y, Int32 delta, UInt32 modifiers ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnKey( INPUT::KeyCode keyCode, bool pressed, UInt32 modifiers ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnPointerCaptureLost( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnSurfaceResized( UInt32 width, UInt32 height ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    COrbitController( const COrbitController& src );
    COrbitController& operator=( const COrbitController& src );

    void Orbit( Int32 deltaX, Int32 deltaY );
    void Pan( Int32 deltaX, Int32 deltaY );
    void ClampAngles( void );

    private:

    SVector3 m_target;
    SVector3 m_boundsCenter;
    Float32 m_boundsRadius;
    Float32 m_yaw;
    Float32 m_pitch;
    Float32 m_distance;
    Float32 m_fieldOfView;
    Float32 m_aspect;
    UInt32 m_surfaceHeight;
    Int32 m_lastX;
    Int32 m_lastY;
    bool m_dragging;
    bool m_changed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETVIEWERLIB_CORBITCONTROLLER_H ? */
