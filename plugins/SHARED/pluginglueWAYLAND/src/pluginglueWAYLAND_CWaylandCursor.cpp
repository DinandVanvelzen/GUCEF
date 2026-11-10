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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H
#include "pluginglueWAYLAND_CWaylandRuntime.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#include "pluginglueWAYLAND_CWaylandConnection.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H
#include "pluginglueWAYLAND_CWaylandScale.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDSCALE_H ? */

#include "pluginglueWAYLAND_CWaylandCursor.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CWaylandCursor::ToProtocolShape( ECursorShape shape )
{GUCEF_TRACE;

    switch ( shape )
    {
        case CURSORSHAPE_TEXT :         return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_TEXT;
        case CURSORSHAPE_POINTER :      return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_POINTER;
        case CURSORSHAPE_CROSSHAIR :    return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_CROSSHAIR;
        case CURSORSHAPE_WAIT :         return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_WAIT;
        case CURSORSHAPE_PROGRESS :     return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_PROGRESS;
        case CURSORSHAPE_MOVE :         return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_MOVE;
        case CURSORSHAPE_NOT_ALLOWED :  return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NOT_ALLOWED;
        case CURSORSHAPE_HELP :         return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_HELP;
        case CURSORSHAPE_EW_RESIZE :    return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_EW_RESIZE;
        case CURSORSHAPE_NS_RESIZE :    return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NS_RESIZE;
        case CURSORSHAPE_NESW_RESIZE :  return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NESW_RESIZE;
        case CURSORSHAPE_NWSE_RESIZE :  return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_NWSE_RESIZE;
        default :                       return GUCEF_WL_WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
    }
}

/*-------------------------------------------------------------------------*/

const char*
CWaylandCursor::ToThemeName( ECursorShape shape )
{GUCEF_TRACE;

    switch ( shape )
    {
        case CURSORSHAPE_TEXT :         return "text";
        case CURSORSHAPE_POINTER :      return "pointer";
        case CURSORSHAPE_CROSSHAIR :    return "crosshair";
        case CURSORSHAPE_WAIT :         return "wait";
        case CURSORSHAPE_PROGRESS :     return "progress";
        case CURSORSHAPE_MOVE :         return "move";
        case CURSORSHAPE_NOT_ALLOWED :  return "not-allowed";
        case CURSORSHAPE_HELP :         return "help";
        case CURSORSHAPE_EW_RESIZE :    return "ew-resize";
        case CURSORSHAPE_NS_RESIZE :    return "ns-resize";
        case CURSORSHAPE_NESW_RESIZE :  return "nesw-resize";
        case CURSORSHAPE_NWSE_RESIZE :  return "nwse-resize";
        default :                       return "default";
    }
}

/*-------------------------------------------------------------------------*/

const char*
CWaylandCursor::ToLegacyThemeName( ECursorShape shape )
{GUCEF_TRACE;

    switch ( shape )
    {
        case CURSORSHAPE_TEXT :         return "xterm";
        case CURSORSHAPE_POINTER :      return "hand2";
        case CURSORSHAPE_CROSSHAIR :    return "cross";
        case CURSORSHAPE_WAIT :         return "watch";
        case CURSORSHAPE_PROGRESS :     return "left_ptr_watch";
        case CURSORSHAPE_MOVE :         return "fleur";
        case CURSORSHAPE_NOT_ALLOWED :  return "crossed_circle";
        case CURSORSHAPE_HELP :         return "question_arrow";
        case CURSORSHAPE_EW_RESIZE :    return "sb_h_double_arrow";
        case CURSORSHAPE_NS_RESIZE :    return "sb_v_double_arrow";
        case CURSORSHAPE_NESW_RESIZE :  return "bottom_left_corner";
        case CURSORSHAPE_NWSE_RESIZE :  return "bottom_right_corner";
        default :                       return "left_ptr";
    }
}

/*-------------------------------------------------------------------------*/

CWaylandCursor::CWaylandCursor( CWaylandConnection& connection )
    : m_connection( connection )
    , m_shapeDevice( GUCEF_NULL )
    , m_pointer( GUCEF_NULL )
    , m_surface( GUCEF_NULL )
    , m_theme( GUCEF_NULL )
    , m_themeScale( 0 )
    , m_shape( CURSORSHAPE_DEFAULT )
    , m_visible( true )
    , m_pointerInside( false )
    , m_enterSerial( 0 )
    , m_scale120( 120 )
    , m_applyCount( 0 )
    , m_cursorApiTried( false )
    , m_cursorApi()
    , m_cursorApiLoaded( false )
{GUCEF_TRACE;

    memset( &m_cursorApi, 0, sizeof( m_cursorApi ) );
}

/*-------------------------------------------------------------------------*/

CWaylandCursor::~CWaylandCursor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWaylandCursor::SetCursorApi( const SWaylandCursorApi& api )
{GUCEF_TRACE;

    m_cursorApi = api;
    m_cursorApiTried = true;
    m_cursorApiLoaded = true;
}

/*-------------------------------------------------------------------------*/

void
CWaylandCursor::ReleaseTheme( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_theme && m_cursorApiLoaded )
        m_cursorApi.theme_destroy( m_theme );
    m_theme = GUCEF_NULL;
    m_themeScale = 0;
}

/*-------------------------------------------------------------------------*/

void
CWaylandCursor::Shutdown( void )
{GUCEF_TRACE;

    const SWaylandApi& api = m_connection.GetApi();
    ReleaseTheme();
    if ( GUCEF_NULL != m_surface )
    {
        gucef_wl_surface_destroy( &api, m_surface );
        m_surface = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_shapeDevice )
    {
        gucef_wp_cursor_shape_device_v1_destroy( &api, m_shapeDevice );
        m_shapeDevice = GUCEF_NULL;
    }
    m_pointer = GUCEF_NULL;
    m_pointerInside = false;
}

/*-------------------------------------------------------------------------*/

void
CWaylandCursor::OnPointerChanged( struct wl_pointer* pointer )
{GUCEF_TRACE;

    const SWaylandApi& api = m_connection.GetApi();
    if ( GUCEF_NULL != m_shapeDevice )
    {
        gucef_wp_cursor_shape_device_v1_destroy( &api, m_shapeDevice );
        m_shapeDevice = GUCEF_NULL;
    }
    m_pointer = pointer;
    m_pointerInside = false;
    if ( GUCEF_NULL != pointer && GUCEF_NULL != m_connection.GetCursorShapeManager() )
        m_shapeDevice = gucef_wp_cursor_shape_manager_v1_get_pointer( &api, m_connection.GetCursorShapeManager(), pointer );
}

/*-------------------------------------------------------------------------*/

void
CWaylandCursor::OnPointerEnter( UInt32 serial, UInt32 scale120 )
{GUCEF_TRACE;

    m_enterSerial = serial;
    m_scale120 = scale120;
    m_pointerInside = true;
    Apply();
}

/*-------------------------------------------------------------------------*/

void
CWaylandCursor::OnPointerLeave( void )
{GUCEF_TRACE;

    m_pointerInside = false;
}

/*-------------------------------------------------------------------------*/

void
CWaylandCursor::SetShape( ECursorShape shape )
{GUCEF_TRACE;

    if ( shape == m_shape )
        return;
    m_shape = shape;
    Apply();
}

/*-------------------------------------------------------------------------*/

CWaylandCursor::ECursorShape
CWaylandCursor::GetShape( void ) const
{GUCEF_TRACE;

    return m_shape;
}

/*-------------------------------------------------------------------------*/

void
CWaylandCursor::SetVisible( bool visible )
{GUCEF_TRACE;

    if ( visible == m_visible )
        return;
    m_visible = visible;
    Apply();
}

/*-------------------------------------------------------------------------*/

bool
CWaylandCursor::IsVisible( void ) const
{GUCEF_TRACE;

    return m_visible;
}

/*-------------------------------------------------------------------------*/

UInt32
CWaylandCursor::GetApplyCount( void ) const
{GUCEF_TRACE;

    return m_applyCount;
}

/*-------------------------------------------------------------------------*/

void
CWaylandCursor::Apply( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_pointer || !m_pointerInside )
        return;
    const SWaylandApi& api = m_connection.GetApi();
    ++m_applyCount;

    if ( !m_visible )
    {
        gucef_wl_pointer_set_cursor( &api, m_pointer, m_enterSerial, GUCEF_NULL, 0, 0 );
        return;
    }
    if ( GUCEF_NULL != m_shapeDevice )
    {
        gucef_wp_cursor_shape_device_v1_set_shape( &api, m_shapeDevice, m_enterSerial, ToProtocolShape( m_shape ) );
        return;
    }

    // a picture of the cursor theme of the user
    if ( GUCEF_NULL == m_connection.GetShm() || GUCEF_NULL == m_connection.GetCompositor() )
        return;
    if ( !m_cursorApiTried )
    {
        m_cursorApiTried = true;
        m_cursorApiLoaded = CWaylandRuntime::LoadCursorApi( m_cursorApi );
    }
    if ( !m_cursorApiLoaded )
        return;

    const UInt32 wholeScale = ( m_scale120 + CWaylandScale::Denominator - 1 ) / CWaylandScale::Denominator;
    if ( GUCEF_NULL == m_theme || m_themeScale != wholeScale )
    {
        ReleaseTheme();
        m_theme = m_cursorApi.theme_load( GUCEF_NULL, (int) ( 24 * wholeScale ), m_connection.GetShm() );
        m_themeScale = wholeScale;
        if ( GUCEF_NULL == m_theme )
            return;
    }
    struct wl_cursor* cursor = m_cursorApi.theme_get_cursor( m_theme, ToThemeName( m_shape ) );
    if ( GUCEF_NULL == cursor )
        cursor = m_cursorApi.theme_get_cursor( m_theme, ToLegacyThemeName( m_shape ) );
    if ( GUCEF_NULL == cursor )
        cursor = m_cursorApi.theme_get_cursor( m_theme, "left_ptr" );
    if ( GUCEF_NULL == cursor || 0 == cursor->image_count )
        return;

    struct wl_cursor_image* image = cursor->images[ 0 ];
    struct wl_buffer* buffer = m_cursorApi.image_get_buffer( image );
    if ( GUCEF_NULL == buffer )
        return;
    if ( GUCEF_NULL == m_surface )
        m_surface = gucef_wl_compositor_create_surface( &api, m_connection.GetCompositor() );
    if ( GUCEF_NULL == m_surface )
        return;

    if ( m_connection.GetCompositorVersion() >= GUCEF_WL_WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION )
        gucef_wl_surface_set_buffer_scale( &api, m_surface, (int32_t) wholeScale );
    gucef_wl_surface_attach( &api, m_surface, buffer, 0, 0 );
    if ( m_connection.GetCompositorVersion() >= GUCEF_WL_WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION )
        gucef_wl_surface_damage_buffer( &api, m_surface, 0, 0, (int32_t) image->width, (int32_t) image->height );
    else
        gucef_wl_surface_damage( &api, m_surface, 0, 0, (int32_t) image->width, (int32_t) image->height );
    gucef_wl_surface_commit( &api, m_surface );
    gucef_wl_pointer_set_cursor( &api, m_pointer, m_enterSerial, m_surface, (int32_t) ( image->hotspot_x / wholeScale ), (int32_t) ( image->hotspot_y / wholeScale ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
