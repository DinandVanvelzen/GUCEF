/*
 *  guidriverWAYLAND: GUCEF window driver for Wayland: native windows with OpenGL through EGL
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


#ifndef GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWMANAGERIMP_H
#define GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWMANAGERIMP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#include "pluginglueWAYLAND_CWaylandConnection.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDCONNECTION_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_MACROS_H
#include "guidriverWAYLAND_macros.h"
#define GUCEF_GUIDRIVERWAYLAND_MACROS_H
#endif /* GUCEF_GUIDRIVERWAYLAND_MACROS_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_RUNTIME_H
#include "guidriverWAYLAND_Runtime.h"
#define GUCEF_GUIDRIVERWAYLAND_RUNTIME_H
#endif /* GUCEF_GUIDRIVERWAYLAND_RUNTIME_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The window backend "Wayland". It is registered whenever the plugin loads; whether it can make windows is asked with
 *  IsAvailable(): there is a Wayland session, the compositor accepts the connection and offers the globals a window needs,
 *  and EGL is installed. Window parameters (CValueList): APP_ID (the desktop file name, default the name of the program),
 *  DECORATIONS (auto, server, libdecor, none), MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT.
 */
class GUIDRIVERWAYLAND_PUBLIC_CPP CWaylandWindowManagerImp : public GUI::CWindowManagerBackend
{
    public:

    CWaylandWindowManagerImp( void );

    /** For tests: windows are made on this connection and with these libraries instead of the ones of the process (neither is owned) */
    CWaylandWindowManagerImp( PLUGINGLUE::WAYLAND::CWaylandConnection* connection, const SWaylandGuiRuntime* runtime );

    virtual ~CWaylandWindowManagerImp();

    virtual bool IsAvailable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CString GetAvailabilityDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title ,
                                                        const GUI::UInt32 width   ,
                                                        const GUI::UInt32 height  ,
                                                        const bool fullscreen     ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title             ,
                                                        const GUI::CVideoSettings& settings   ,
                                                        const CORE::CValueList* params = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TWindowContextPtr CreateWindowContext( const GUI::CString& title      ,
                                                        const GUI::UInt32 width        ,
                                                        const GUI::UInt32 height       ,
                                                        const bool fullscreen          ,
                                                        const CORE::CValueList& params ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyWindowContext( GUI::TWindowContextPtr& windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                     const GUI::CVideoSettings& settings   ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CWaylandWindowManagerImp( const CWaylandWindowManagerImp& src );
    CWaylandWindowManagerImp& operator=( const CWaylandWindowManagerImp& src );

    const SWaylandGuiRuntime& GetRuntime( void ) const;

    void Evaluate( void ) const;

    private:

    PLUGINGLUE::WAYLAND::CWaylandConnection* m_connection;
    const SWaylandGuiRuntime* m_runtime;
    mutable bool m_evaluated;
    mutable PLUGINGLUE::WAYLAND::SWaylandAvailability m_availability;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CWaylandWindowManagerImp, MT::CMutex > TWaylandWindowManagerImpPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWMANAGERIMP_H ? */
