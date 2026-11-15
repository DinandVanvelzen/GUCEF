/*
 *  guidriverMyGUI: GUI backend using MyGUI
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

#ifndef GUCEF_guidriverMyGUI_CMYGUIDRIVER_H
#define GUCEF_guidriverMyGUI_CMYGUIDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTWEAKPTR_H
#include "gucefCORE_CTWeakPtr.h"
#define GUCEF_CORE_CTWEAKPTR_H
#endif /* GUCEF_CORE_CTWEAKPTR_H ? */

#ifndef GUCEF_GUI_CGUIDRIVER_H
#include "gucefGUI_CGUIDriver.h"
#define GUCEF_GUI_CGUIDRIVER_H
#endif /* GUCEF_GUI_CGUIDRIVER_H ? */

#ifndef GUCEF_guidriverMyGUI_MACROS_H
#include "guidriverMyGUI_macros.h"
#define GUCEF_guidriverMyGUI_MACROS_H
#endif /* GUCEF_guidriverMyGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MyGUI { class LogManager; class LogSource; }

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CMyGUIDataManager;
class CMyGUILogListener;
class CMyGUIContext;
class CMyGUIWidgetHandler;

/**
 *  The part of a MyGUI GUI driver that does not depend on a renderer: the MyGUI log manager, the data manager
 *  over the virtual file system (with the data of the MyGUI.Engine module, the skins, layouts and fonts, as the
 *  first search path), the widget handlers, the fonts and the list of contexts. A renderer driver
 *  derives from it and makes the contexts.
 *
 *  MyGUI has one Gui, one render manager and one data manager per process: a driver has one context at a time.
 */
class GUCEF_guidriverMyGUI_PUBLIC_CPP CMyGUIDriver : public GUI::CGUIDriver
{
    public:

    /** The name of the data of MyGUI.Engine in the VFS, where the standard skins are */
    static const char* EngineDataName;

    /**
     *  @param ownDataManager  true: MyGUI reads its resources from the VFS through the data manager of this module;
     *                         false: the derived driver makes the data manager (the Ogre platform of MyGUI reads
     *                         through the resource system of Ogre, which GUCEF routes into the VFS)
     */
    explicit CMyGUIDriver( bool ownDataManager = true );

    virtual ~CMyGUIDriver();

    /** The data manager over the VFS; NULL when the derived driver has its own */
    CMyGUIDataManager* GetDataManager( void );

    /** Whether there is a context that is still in use */
    bool HasContext( void );

    virtual TGUIContextSet GetContextList( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetContextCount( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableFormTypes( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableWidgetTypes( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableFonts( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Loads a TrueType font from the VFS; its MyGUI name is the file name without the extension */
    virtual bool LoadFontFromAsset( const CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDriverProperty( const CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CWidgetHandler* GetWidgetHandler( const CString& widgetType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetCapabilities( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetInterfaceVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    typedef CORE::CTWeakPtr< GUI::CIGUIContext, MT::CMutex > TGuiContextWeakPtr;
    typedef GUCEF::set< TGuiContextWeakPtr > TWeakContextSet;
    typedef GUCEF::map< CString, CMyGUIWidgetHandler* > THandlerMap;
    typedef GUCEF::map< CString, CString > TFontMap;

    void AddContext( GUI::TGuiContextPtr& context );

    /** Loads the fonts that were asked for before the context of MyGUI existed, and makes them known to it */
    void LoadPendingFonts( CMyGUIContext& context );

    private:

    CMyGUIDriver( const CMyGUIDriver& src );
    CMyGUIDriver& operator=( const CMyGUIDriver& src );

    private:

    MyGUI::LogManager* m_logManager;
    MyGUI::LogSource* m_logSource;
    CMyGUILogListener* m_logListener;
    CMyGUIDataManager* m_dataManager;
    THandlerMap m_handlers;
    TWeakContextSet m_contexts;
    TStringSet m_fonts;
    TFontMap m_fontPaths;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUI_CMYGUIDRIVER_H ? */
