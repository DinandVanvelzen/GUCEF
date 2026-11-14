/*
 *  guidriverCEGUIOpenGL: glue module for the MyGUI GUI backend using OpenGL
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUIDRIVERCEGUIGL_CGUIDRIVERGL_H
#include "guidriverCEGUIOpenGL_CGUIDriverGL.h"
#define GUCEF_GUIDRIVERCEGUIGL_CGUIDRIVERGL_H
#endif /* GUCEF_GUIDRIVERCEGUIGL_CGUIDRIVERGL_H ? */

#include "guidriverCEGUIOpenGL_CGUIContextGL.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERCEGUIGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CGUIContextGL::CGUIContextGL( CGUIDriverGL& guiDriver                   ,
                              GUI::TWindowContextPtr windowContext      ,
                              INPUT::CInputContext* inputContext        ,
                              CEGUI::GUIContext* ceGuiContext           )
    : GUIDRIVERCEGUI::CGUIContext( guiDriver    ,
                                   inputContext ,
                                   ceGuiContext )  ,
      m_windowContext( windowContext ),
      m_lastTimeInjection( clock() ) 
{GUCEF_TRACE;

    SubscribeTo( m_windowContext.GetPointer() );
}

/*-------------------------------------------------------------------------*/

CGUIContextGL::~CGUIContextGL()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGUIContextGL::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::CEGUIGL::CGUIContextGL";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CGUIContextGL::OnNotify( CORE::CNotifier* notifier   ,
                         const CORE::CEvent& eventID ,
                         CORE::CICloneable* evenData )
{GUCEF_TRACE;

    if ( eventID == GUI::CWindowContext::WindowContextRedrawEvent   ||
         eventID == GUI::CWindowContext::WindowContextActivateEvent  )
    {
        CEGUI::System* guiSystem = m_driver->GetCEGui();
        if ( GUCEF_NULL != guiSystem && GUCEF_NULL != m_ceGuiContext ) 
        {       
            clock_t now = clock();
            float elapsedGuiTime = ( now - m_lastTimeInjection ) * 0.001f;        
            guiSystem->injectTimePulse( elapsedGuiTime );
            m_ceGuiContext->injectTimePulse( elapsedGuiTime );
            m_lastTimeInjection = now;
        
            CEGUI::Renderer* guiRenderer = guiSystem->getRenderer();
            guiRenderer->beginRendering();
        
            m_ceGuiContext->markAsDirty();
            m_ceGuiContext->draw();

            guiRenderer->endRendering();
        }
    }
    else
    if ( eventID == GUI::CWindowContext::WindowContextSizeEvent )
    {        
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "GUIContext: Resizing CEGUI GUI context to " + CORE::UInt32ToString( m_windowContext->GetWidth() ) + "x" +
                                                                                                CORE::UInt32ToString( m_windowContext->GetHeight() ) );

		CEGUI::System::getSingleton().notifyDisplaySizeChanged( CEGUI::Sizef( (float) m_windowContext->GetWidth(), (float) m_windowContext->GetHeight() ) );
        //m_ceGuiContext->getRenderTarget().setArea( m_windowContext->GetWidth(), m_windowContext->GetHeight() );

        //CEGUI::Renderer* guiRenderer = m_driver->GetCEGui()->getRenderer();
        //guiRenderer->beginRendering();

        //m_ceGuiContext->markAsDirty();
        //m_ceGuiContext->draw();

        //guiRenderer->endRendering();
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERCEGUIGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
