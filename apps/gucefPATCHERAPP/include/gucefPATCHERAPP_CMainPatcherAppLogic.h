/*
 *  gucefPATCHERAPP: Application module for the patcher application
 *  Copyright (C) 2002 - 2013.  Dinand Vanvelzen
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

#ifndef GUCEF_PATCHERAPP_CMAINPATCHERAPPLOGIC_H
#define GUCEF_PATCHERAPP_CMAINPATCHERAPPLOGIC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#include "gucefCORE_CPlatformNativeConsoleWindow.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLEWINDOW_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#include "gucefGUI_CIGUIContext.h"
#define GUCEF_GUI_CIGUICONTEXT_H
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

#ifndef GUCEF_PATCHER_CPATCHMANAGER_H
#include "gucefPATCHER_CPatchManager.h"
#define GUCEF_PATCHER_CPATCHMANAGER_H
#endif /* GUCEF_PATCHER_CPATCHMANAGER_H ? */

#ifndef GUCEF_PATCHERAPP_MACROS_H
#include "gucefPATCHERAPP_macros.h"
#define GUCEF_PATCHERAPP_MACROS_H
#endif /* GUCEF_PATCHERAPP_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PATCHERAPP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CMainPatcherAppLogic : public CORE::CObservingNotifier
{
    public:

    CMainPatcherAppLogic( void );

    virtual ~CMainPatcherAppLogic();
    
    Int32 Init( UInt32 argc, const char** argv );

    void Shutdown( void );

    private:

    typedef CORE::CTEventHandlerFunctor< CMainPatcherAppLogic > TEventCallback;

    CMainPatcherAppLogic( const CMainPatcherAppLogic& src );

    CMainPatcherAppLogic& operator=( const CMainPatcherAppLogic& src );

    bool SetupWindowContext( GUI::TWindowManagerBackendPtr windowMngrBackend ,
                             const GUI::CString& windowMngrBackendName       ,
                             const GUI::CString& guiDriverToUse              );

    virtual void OnFirstAppCycle( CORE::CNotifier* notifier           ,
                                  const CORE::CEvent& eventid         ,
                                  CORE::CICloneable* eventdata = GUCEF_NULL );

    virtual void OnPatchingProgress( CORE::CNotifier* notifier           ,
                                     const CORE::CEvent& eventid         ,
                                     CORE::CICloneable* eventdata = GUCEF_NULL );
    
    void ShutdownLogging( void );

    void SetupLogging( void );

    void LoadFontsFromAssets( GUI::TGuiContextPtr guiContext );

    bool StartPatchingWork( void );

    private:

    CORE::CFileAccess* m_logFile;
    CORE::CStdLogger* m_fileLogger;
    CORE::CPlatformNativeConsoleLogger* m_consoleLogger;
    CORE::CPlatformNativeConsoleWindow* m_consoleWindow;
    GUI::TWindowContextPtr m_windowContext;
    GUI::TGuiContextPtr m_guiContext;
    PATCHER::CPatchManager m_patchManager;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHERAPP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PATCHERAPP_CMAINPATCHERAPPLOGIC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 23-01-2013 :
        - Dinand: Initial version.

---------------------------------------------------------------------------*/