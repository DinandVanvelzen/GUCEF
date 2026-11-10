/*
 *  guidriverNull: GUCEF GUI driver that keeps the widgets but draws nothing, for tests and tools
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

#ifndef GUCEF_GUIDRIVERNULL_CNULLGUIDRIVER_H
#define GUCEF_GUIDRIVERNULL_CNULLGUIDRIVER_H

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

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUIDRIVERNULL_CNULLWIDGETHANDLER_H
#include "guidriverNull_CNullWidgetHandler.h"
#define GUCEF_GUIDRIVERNULL_CNULLWIDGETHANDLER_H
#endif /* GUCEF_GUIDRIVERNULL_CNULLWIDGETHANDLER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERNULL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A context that shows nothing; it counts the frames it was asked to draw. A frame capture draws the
 *  context on the CPU with the reference software renderer of gucefGUI.
 */
class GUIDRIVERNULL_PUBLIC_CPP CNullGUIContext : public GUI::CGUIContext
{
    public:

    CNullGUIContext( GUI::CGUIDriver* driver, GUI::TWindowContextPtr windowContext );

    virtual ~CNullGUIContext();

    CORE::UInt32 GetRenderCount( void ) const;

    CORE::UInt32 GetRootOrderChangeCount( void ) const;

    virtual bool CaptureFrame( IMAGE::TPixelMapPtr& pixels ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnRender( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnRootOrderChanged( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::UInt32 m_renderCount;
    CORE::UInt32 m_rootOrderChanges;
};

/*-------------------------------------------------------------------------*/

/**
 *  The reference driver: it keeps the widget tree and the computed rectangles of gucefGUI and records
 *  what its handlers are given, but draws nothing and needs no window or graphics. Tests of gucefGUI
 *  itself and of application logic run on it anywhere. Registered as "Null".
 */
class GUIDRIVERNULL_PUBLIC_CPP CNullGUIDriver : public GUI::CGUIDriver
{
    public:

    CNullGUIDriver( void );

    virtual ~CNullGUIDriver();

    virtual GUI::TGuiContextPtr CreateGUIContext( GUI::TWindowContextPtr windowContext ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TGUIContextSet GetContextList( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::UInt32 GetContextCount( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDriverName( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableFormTypes( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableWidgetTypes( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableFonts( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadFontFromAsset( const CORE::CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDriverProperty( const CORE::CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CWidgetHandler* GetWidgetHandler( const CORE::CString& widgetType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetCapabilities( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::UInt32 GetInterfaceVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CNullGUIDriver( const CNullGUIDriver& src );
    CNullGUIDriver& operator=( const CNullGUIDriver& src );

    typedef GUCEF::map< CORE::CString, CNullWidgetHandler* > THandlerMap;

    THandlerMap m_handlers;
    typedef CORE::CTWeakPtr< GUI::CIGUIContext, MT::CMutex > TGuiContextWeakPtr;
    typedef GUCEF::set< TGuiContextWeakPtr > TWeakContextSet;

    TWeakContextSet m_contexts;     /**< the contexts made, kept alive by their owners and not by the driver */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERNULL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERNULL_CNULLGUIDRIVER_H ? */
