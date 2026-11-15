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

#ifndef GUCEF_GUIDRIVERCEGUIOGRE_CGUICONTEXTOGRE_H
#define GUCEF_GUIDRIVERCEGUIOGRE_CGUICONTEXTOGRE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <time.h>

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUIDRIVERCEGUI_CGUICONTEXT_H
#include "guidriverCEGUI_CGUIContext.h"
#define GUCEF_GUIDRIVERCEGUI_CGUICONTEXT_H
#endif /* GUCEF_GUIDRIVERCEGUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUIDRIVERCEGUIOGRE_MACROS_H
#include "guidriverCEGUIOgre_macros.h"
#define GUCEF_GUIDRIVERCEGUIOGRE_MACROS_H
#endif /* GUCEF_GUIDRIVERCEGUIOGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERCEGUIOGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CGUIDriverOgre;

/**
 *  Implementation of the GUI context for the CEGUI & Ogre combo
 */
class GUCEF_GUIDRIVERCEGUIOGRE_EXPORT_CPP CGUIContextOgre : public GUIDRIVERCEGUI::CGUIContext ,
                                                            public CORE::CObserver
{
    public:

    CGUIContextOgre( CGUIDriverOgre& guiDriver            ,
                     GUI::TWindowContextPtr windowContext ,
                     INPUT::CInputContext* inputContext   ,
                     CEGUI::GUIContext* ceGuiContext      );

    virtual ~CGUIContextOgre();

    virtual const CORE::CString& GetClassTypeName( void ) const;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier          ,
                           const CORE::CEvent& eventID        ,
                           CORE::CICloneable* evenData = GUCEF_NULL );

    private:

    CGUIContextOgre( const CGUIContextOgre& src );
    CGUIContextOgre& operator=( const CGUIContextOgre& src );

    private:

    GUI::TWindowContextPtr m_windowContext;
    clock_t m_lastTimeInjection;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERCEGUIOGRE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERCEGUIOGRE_CGUICONTEXTOGRE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 08-04-2007 :
        - Initial implementation

---------------------------------------------------------------------------*/
