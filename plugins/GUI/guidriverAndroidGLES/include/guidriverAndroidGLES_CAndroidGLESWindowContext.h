/*
 *  guidriverAndroidGLES: module implementing GLES based window management for Android
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUIDRIVERANDROIDGLES_CANDROIDGLESWINDOWCONTEXT_H
#define GUIDRIVERANDROIDGLES_CANDROIDGLESWINDOWCONTEXT_H 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <EGL/egl.h>

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#include "gucefGUI_CVideoSettings.h"
#define GUCEF_GUI_CVIDEOSETTINGS_H
#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

#ifndef GUIDRIVERANDROIDGLES_MACROS_H
#include "guidriverAndroidGLES_macros.h"
#define GUIDRIVERANDROIDGLES_MACROS_H
#endif /* GUIDRIVERANDROIDGLES_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERANDROIDGLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The window of an Android app with an OpenGL ES 3 context on it. The native window comes from the
 *  android_app of android_native_app_glue, given as the "android_app" parameter (a pointer as text, the
 *  platform parameter the GUCEF Android glue passes on). Frames are drawn on the pulses of the
 *  application (the frequency of the video settings) and swapped to the screen.
 *
 *  Properties: WINDOW (the ANativeWindow), GL_GETPROCADDRESS, GL_FRAMEBUFFER ("0"), GL_API ("GLES"), GL_VERSION ("3.0").
 */
class GUIDRIVERANDROIDGLES_PUBLIC_CPP CAndroidGLESWindowContext : public GUI::CWindowContext
{
    public:

    CAndroidGLESWindowContext( void );

    virtual ~CAndroidGLESWindowContext();

    virtual GUI::UInt32 GetWidth( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetHeight( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CString GetProperty( const GUI::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetGuiContext( GUI::TGuiContextPtr& context ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::TGuiContextPtr GetGuiContext( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::UInt32 GetID( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool Initialize( const GUI::CString& title, const GUI::CVideoSettings& videoSettings, const CORE::CValueList* params );

    /**
     *  Waiting for the vertical blank or not. The window always covers the screen: the system decides its size,
     *  resolution and full screen in the settings are not used
     */
    bool ApplyVideoSettings( const GUI::CVideoSettings& videoSettings );

    void Shutdown( void );

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAndroidGLESWindowContext( const CAndroidGLESWindowContext& src );            /**< private because: must be unique */
    CAndroidGLESWindowContext& operator=( const CAndroidGLESWindowContext& src ); /**< private because: must be unique */

    private:

    GUI::TGuiContextPtr m_guiContext;
    GUI::UInt32 m_id;
    GUI::CString m_name;
    EGLDisplay m_display;
    EGLSurface m_surface;
    EGLContext m_context;
    void* m_nativeWindow;
    bool m_drivesRedraws;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CAndroidGLESWindowContext, MT::CMutex >  TAndroidGLESWindowContextPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERANDROIDGLES */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERANDROIDGLES_CANDROIDGLESWINDOWCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 08-10-2005 :
        - Initial version

---------------------------------------------------------------------------*/
