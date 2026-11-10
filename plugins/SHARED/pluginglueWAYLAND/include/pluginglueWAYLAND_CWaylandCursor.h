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


#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCURSOR_H
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCURSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#include "pluginglueWAYLAND_macros.h"
#define GUCEF_PLUGINGLUE_WAYLAND_MACROS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_MACROS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#include "pluginglueWAYLAND_Protocols.h"
#define GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_PROTOCOLS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

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

class CWaylandConnection;

/**
 *  The pointer image over the windows of the application. The compositor forgets it each time the pointer enters a surface,
 *  so the wish is kept (shape, hidden or not) and set again on each enter. A shape goes to the compositor by name
 *  (wp_cursor_shape_v1) when it offers that, else it is a picture of the cursor theme of the user (libwayland-cursor,
 *  at the whole scale of the window). Without either the compositor's own pointer stays.
 */
class GUCEF_PLUGINGLUE_WAYLAND_EXPORT_CPP CWaylandCursor
{
    public:

    enum ECursorShape
    {
        CURSORSHAPE_DEFAULT = 0 ,
        CURSORSHAPE_TEXT        ,
        CURSORSHAPE_POINTER     ,
        CURSORSHAPE_CROSSHAIR   ,
        CURSORSHAPE_WAIT        ,
        CURSORSHAPE_PROGRESS    ,
        CURSORSHAPE_MOVE        ,
        CURSORSHAPE_NOT_ALLOWED ,
        CURSORSHAPE_HELP        ,
        CURSORSHAPE_EW_RESIZE   ,
        CURSORSHAPE_NS_RESIZE   ,
        CURSORSHAPE_NESW_RESIZE ,
        CURSORSHAPE_NWSE_RESIZE ,

        CURSORSHAPE_COUNT
    };

    /** The value of wp_cursor_shape_device_v1.shape for the shape */
    static UInt32 ToProtocolShape( ECursorShape shape );

    /** The name of the shape in a cursor theme that follows the freedesktop naming, and the older name of it */
    static const char* ToThemeName( ECursorShape shape );

    static const char* ToLegacyThemeName( ECursorShape shape );

    explicit CWaylandCursor( CWaylandConnection& connection );

    ~CWaylandCursor();

    /** Lets go of everything made for the cursor; the connection does this before it closes */
    void Shutdown( void );

    /** The pointer exists or does not (seat capability): the device for shapes is made or let go */
    void OnPointerChanged( struct wl_pointer* pointer );

    /** The pointer entered a surface of the application: the cursor is set again, at the scale given */
    void OnPointerEnter( UInt32 serial, UInt32 scale120 );

    void OnPointerLeave( void );

    void SetShape( ECursorShape shape );

    ECursorShape GetShape( void ) const;

    void SetVisible( bool visible );

    bool IsVisible( void ) const;

    /** Takes the functions for cursor themes from the caller instead of libwayland-cursor (tests) */
    void SetCursorApi( const SWaylandCursorApi& api );

    /** The calls made to the compositor for the cursor since the start, for tests and logs */
    UInt32 GetApplyCount( void ) const;

    private:

    CWaylandCursor( const CWaylandCursor& src );
    CWaylandCursor& operator=( const CWaylandCursor& src );

    void Apply( void );

    void ReleaseTheme( void );

    private:

    CWaylandConnection& m_connection;
    struct wp_cursor_shape_device_v1* m_shapeDevice;
    struct wl_pointer* m_pointer;
    struct wl_surface* m_surface;
    struct wl_cursor_theme* m_theme;
    UInt32 m_themeScale;
    ECursorShape m_shape;
    bool m_visible;
    bool m_pointerInside;
    UInt32 m_enterSerial;
    UInt32 m_scale120;
    UInt32 m_applyCount;
    bool m_cursorApiTried;
    SWaylandCursorApi m_cursorApi;
    bool m_cursorApiLoaded;
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

#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCURSOR_H ? */
