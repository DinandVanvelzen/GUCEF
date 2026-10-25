/*
 *  pluginglueWAYLAND: Library that owns the connection to the Wayland compositor for the plugins that share it
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

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#include "pluginglueWAYLAND_macros.h"
#define GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_MACROS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_ABI_H
#include "pluginglueWAYLAND_Abi.h"
#define GUCEF_PLUGINGLUE_WAYLAND_ABI_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_ABI_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace WAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The arithmetic of scales. A scale is kept in 120ths (120 is 1.0, 180 is 1.5, 240 is 2.0) as wp_fractional_scale_v1 sends it;
 *  a whole scale of wl_surface.set_buffer_scale is that times 120. A logical size is in surface units (what the compositor
 *  lays out and what pointer events are in), a buffer size in pixels.
 */
class GUCEF_PLUGINGLUE_WAYLAND_EXPORT_CPP CWaylandScale
{
    public:

    static const UInt32 Denominator = 120;

    /** The size in pixels of a logical size: rounded to the nearest, halves up, as the fractional scale protocol says */
    static UInt32 LogicalToBuffer( UInt32 logical, UInt32 scale120 );

    /** The logical size a buffer of that many pixels stands for (rounded up so a buffer is never larger than its surface) */
    static UInt32 BufferToLogical( UInt32 buffer, UInt32 scale120 );

    /** A surface local pointer coordinate (24.8 fixed point, logical units) in pixels of the buffer, rounded to the nearest */
    static Int32 PointerToBuffer( wl_fixed_t coordinate, UInt32 scale120 );

    /** The scale in 120ths of a whole scale factor of wl_output.scale and wl_surface.preferred_buffer_scale; below 1 counts as 1 */
    static UInt32 FromWholeScale( Int32 factor );

    static bool IsWholeScale( UInt32 scale120 );

    /** A scale as text for logs and properties: "1", "1.5", "1.25" */
    static CORE::CString ToString( UInt32 scale120 );
};

/*-------------------------------------------------------------------------*/

/**
 *  Decides the scale of a surface from what the compositor tells, in order of preference: the fractional scale of the
 *  surface (wp_fractional_scale_v1, used when a viewport can apply it), the preferred whole buffer scale of the surface
 *  (wl_surface v6), the largest whole scale of the outputs the surface is on. Without any of those 1.
 */
class GUCEF_PLUGINGLUE_WAYLAND_EXPORT_CPP CWaylandScaleTracker
{
    public:

    enum EScaleSource
    {
        SCALESOURCE_DEFAULT = 0 ,
        SCALESOURCE_OUTPUTS     ,
        SCALESOURCE_PREFERRED   ,
        SCALESOURCE_FRACTIONAL
    };

    CWaylandScaleTracker( void );

    /** Whether wp_fractional_scale_v1 and wp_viewporter are both there: a fractional scale is then applied with a viewport */
    void SetFractionalCapable( bool capable );

    bool IsFractionalCapable( void ) const;

    void OnFractionalScale( UInt32 scale120 );

    void OnPreferredBufferScale( Int32 factor );

    void OnOutputEntered( const void* output, Int32 wholeScale );

    void OnOutputLeft( const void* output );

    void OnOutputScaleChanged( const void* output, Int32 wholeScale );

    /** The scale to draw at, in 120ths */
    UInt32 GetScale120( void ) const;

    EScaleSource GetSource( void ) const;

    /** A viewport maps the buffer to the logical size: when the compositor sends fractional scales and a viewport is there */
    bool UsesViewport( void ) const;

    /** The factor for wl_surface.set_buffer_scale: 1 while a viewport does the scaling, else the whole scale */
    Int32 GetWholeBufferScale( void ) const;

    private:

    struct SOutputScale
    {
        const void* output;
        Int32 scale;
    };
    typedef GUCEF::vector< SOutputScale > TOutputScales;

    UInt32 m_fractional120;         /**< 0 until the compositor sent one */
    Int32 m_preferredWhole;         /**< 0 until the compositor sent one */
    TOutputScales m_outputs;
    bool m_fractionalCapable;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H ? */
