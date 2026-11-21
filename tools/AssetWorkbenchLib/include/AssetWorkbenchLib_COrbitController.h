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

#ifndef ASSETWORKBENCHLIB_CORBITCONTROLLER_H
#define ASSETWORKBENCHLIB_CORBITCONTROLLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CIVIEWPORTCONTROLLER_H
#include "gucefGUI_CIViewportController.h"
#define GUCEF_GUI_CIVIEWPORTCONTROLLER_H
#endif /* GUCEF_GUI_CIVIEWPORTCONTROLLER_H ? */

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
 *  The camera of a model view as a point looked at, a distance and two angles (y is up). It is the controller of a
 *  CViewportForm: the left button orbits, the middle or right button, or the left button with Shift, pans, the wheel moves
 *  closer and further away, F frames the bounds again and the keys 1 to 4 pick the fixed views. It does the arithmetic only and
 *  knows nothing of Ogre: the stage reads GetEye() and GetTarget() when ConsumeChanged() says that the view moved.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP COrbitController : public GUI::CIViewportController
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

    /** Where the camera is, as the four things the user changes by orbiting, panning and zooming */
    struct SCameraState
    {
        SVector3 target;
        Float32 yaw;
        Float32 pitch;
        Float32 distance;

        SCameraState( void ) : target(), yaw( 0.0f ), pitch( 0.0f ), distance( 1.0f ) {}
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

    /**
     *  Remembers the bounds as a box, for what is long, tall or flat (a sphere around it leaves most of the view empty), and frames
     *  it. The camera then looks at the middle of the box from the distance at which its corners, seen from the angles of the
     *  camera, fill 'fillFraction' of the narrower side of the view: 0.9 when it is 0 or less. The sphere (center and radius) is
     *  the one around the box, so everything that works with a sphere keeps working. With 'centreCovered' the target is moved so that
     *  what the box covers in the view is in the middle of it (see GetFramedView(): the orthographic views then look at the middle of the
     *  box instead of the target); without, the camera looks at the middle of the box.
     */
    void SetBoundsBox( const SVector3& minimum, const SVector3& maximum, Float32 fillFraction = 0.0f, bool centreCovered = false );
    bool IsBoxCentredOnCovered( void ) const;
    bool HasBoundsBox( void ) const;
    const SVector3& GetBoundsMinimum( void ) const;
    const SVector3& GetBoundsMaximum( void ) const;
    Float32 GetFillFraction( void ) const;

    /** The distance at which the bounds fill the view as asked, seen from these angles (and from the middle of the bounds) */
    Float32 GetFramingDistanceFor( Float32 yaw, Float32 pitch ) const;

    /**
     *  The target and distance that put what the box covers in the middle of the view, filling it as asked, seen from these angles. A box seen
     *  from above and aside is not symmetric in the view (the near corners are bigger and lower): the target is moved so that the covered part
     *  is, which shows the box bigger than from its middle. Without a box: the middle of the bounds and the distance of the sphere
     */
    void GetFramedView( Float32 yaw, Float32 pitch, SVector3& target, Float32& distance ) const;

    /** The size of the part of the view that the bounds fill: the side of the view, as a part of that side, that a box of this width and height takes at this extent */
    static Float32 GetFilledFraction( Float32 width, Float32 height, Float32 extent, Float32 aspect );

    /** Looks at the center of the bounds from a distance at which the whole sphere is in view with a little room left */
    void Frame( void );

    /** The distance at which a sphere of this radius fills the narrower of the two field of views, with the margin */
    Float32 GetFramingDistance( Float32 radius ) const;

    /** Whether a whole sphere is inside the view cone of the camera */
    bool IsSphereInView( const SVector3& center, Float32 radius ) const;

    /** Looks at the bounds from a fixed view and frames them */
    void ApplyPreset( EViewPreset preset );

    /** The camera as it is now */
    SCameraState GetState( void ) const;

    /** The camera of a fixed view of the bounds as ApplyPreset() makes it, without touching the camera */
    SCameraState GetPresetState( EViewPreset preset ) const;

    /** The camera framing the bounds from the angles it has now, as Frame() makes it, without touching the camera */
    SCameraState GetFrameState( void ) const;

    /**
     *  Whether two cameras are the same view within a tolerance: the angles within 'tolerance' radians, the target within
     *  'tolerance' of the radius of the bounds from one another and the distance within a part 'tolerance' of the bigger one
     */
    bool IsSameView( const SCameraState& a, const SCameraState& b, Float32 tolerance ) const;

    /** The far clip plane is at least this far from the camera (0 for no minimum): background that is left out of the bounds, a sky, stays in view */
    void SetMinimumFarClip( Float32 distance );

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
    bool m_hasBox;
    Float32 m_minimumFar;
    bool m_centreCovered;
    SVector3 m_boundsMinimum;
    SVector3 m_boundsMaximum;
    Float32 m_fillFraction;
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

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CORBITCONTROLLER_H ? */
