/*
 *  guidriverRmlUIOpenGL: module providing a GUI backend using RmlUi and OpenGL
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_GUIDRIVERRMLUIGL_CRMLUIGUIDRIVER_H
#define GUCEF_GUIDRIVERRMLUIGL_CRMLUIGUIDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIDRIVER_H
#include "guidriverRmlUI_CRmlUIDriver.h"
#define GUCEF_GUIDRIVERRMLUI_CRMLUIDRIVER_H
#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIDRIVER_H ? */

#ifndef GUCEF_GUIDRIVERRMLUIGL_MACROS_H
#include "guidriverRmlUIOpenGL_macros.h"
#define GUCEF_GUIDRIVERRMLUIGL_MACROS_H
#endif /* GUCEF_GUIDRIVERRMLUIGL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERRMLUIGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The RmlUi GUI driver that draws with OpenGL 3.3 (desktop), OpenGL ES 3 (Android) or WebGL 2
 *  (browsers), registered as "RmlUi".
 *
 *  A GUI context needs a window whose GL context is current on the calling thread: the window
 *  drivers XWinGL, Win32GL, AndroidGLES, EGL (offscreen) and the browser canvas. GL functions are
 *  loaded with the window's GL_GETPROCADDRESS function when it names one.
 */
class GUIDRIVERRMLUIGL_PUBLIC_CPP CRmlUIGuiDriver : public guidriverRmlUI::CRmlUIDriver
{
    public:

    CRmlUIGuiDriver( void );

    virtual ~CRmlUIGuiDriver();

    virtual GUI::TGuiContextPtr CreateGUIContext( GUI::TWindowContextPtr windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetCapabilities( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDriverProperty( const CORE::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CRmlUIGuiDriver( const CRmlUIGuiDriver& src );
    CRmlUIGuiDriver& operator=( const CRmlUIGuiDriver& src );

    private:

    CORE::CString m_glDescription;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERRMLUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERRMLUIGL_CRMLUIGUIDRIVER_H ? */
