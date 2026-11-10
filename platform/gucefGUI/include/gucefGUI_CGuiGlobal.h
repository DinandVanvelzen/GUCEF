/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CGUIGLOBAL_H
#define GUCEF_GUI_CGUIGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CWindowManager;
class CGUIManager;
class CWidgetTypeRegistry;
class CFontRasterizerRegistry;

/*-------------------------------------------------------------------------*/

/**
 *  Singular singleton providing access to all global Vfs systems
 */
class GUCEF_GUI_PUBLIC_CPP CGuiGlobal
{
    public:

    static CGuiGlobal* Instance( void );

    CWindowManager& GetWindowManager( void );

    CGUIManager& GetGuiManager( void );

    /** The widget types gucefGUI knows, with their properties */
    CWidgetTypeRegistry& GetWidgetTypeRegistry( void );

    /** The font rasterizers the loaded plugins provide */
    CFontRasterizerRegistry& GetFontRasterizerRegistry( void );

    /** Where MountData() mounts the data files gucefGUI comes with in the VFS */
    static const CString DataMountPoint;

    /** The user interface font gucefGUI comes with (Noto Sans, SIL Open Font License), a VFS path */
    static const CString DefaultUiFontPath;

    /**
     *  Mounts the data files gucefGUI comes with, read only, at DataMountPoint in the VFS. They are in
     *  $MODULEDIR$/../data/gucefGUI, on the web at /data/gucefGUI.
     *
     *  @return true when they are mounted, also when they already were
     */
    bool MountData( void );

    /** Mounts the data files of a module in the VFS, see VFS::CVFS::MountModuleData() */
    static bool MountModuleData( const CString& moduleName );

    private:
    friend class CModule;

    static void Deinstance( void );

    private:

    CGuiGlobal( void );

    ~CGuiGlobal();

    void Initialize( void );

    private:

    CWindowManager* m_windowManager;
    CGUIManager* m_guiManager;
    CWidgetTypeRegistry* m_widgetTypeRegistry;
    CFontRasterizerRegistry* m_fontRasterizerRegistry;

    static CGuiGlobal* g_instance;
    static MT::CMutex g_datalock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGUIGLOBAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 16-02-2007 :
        - Dinand: Added this class

---------------------------------------------------------------------------*/
