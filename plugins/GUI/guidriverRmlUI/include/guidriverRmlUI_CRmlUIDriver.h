/*
 *  guidriverRmlUI: GUI backend using RmlUi
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

#ifndef GUCEF_GUIDRIVERRMLUI_CRMLUIDRIVER_H
#define GUCEF_GUIDRIVERRMLUI_CRMLUIDRIVER_H

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

#ifndef GUCEF_guidriverRmlUI_MACROS_H
#include "guidriverRmlUI_macros.h"
#define GUCEF_guidriverRmlUI_MACROS_H
#endif /* GUCEF_guidriverRmlUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CRmlUIWidgetHandler;

/**
 *  What every RmlUi GUI driver shares, whatever draws it: RmlUi itself (started with the first
 *  driver, its files read through the VFS, its log in the GUCEF log), the widget handlers, the
 *  contexts and the capabilities. A renderer driver (guidriverRmlUIOpenGL ...) derives from it, names
 *  itself and makes contexts with its render interface.
 *
 *  Fonts: gucefGUI bitmap fonts work with every renderer; LoadFontFromAsset() adds a font file
 *  (TrueType, OpenType) for RML documents.
 */
class GUCEF_guidriverRmlUI_PUBLIC_CPP CRmlUIDriver : public GUI::CGUIDriver
{
    public:

    /** The name layouts use for hints meant for RmlUi ("BackendHints driver=...") */
    static const char* DriverName;

    CRmlUIDriver( void );

    virtual ~CRmlUIDriver();

    /** Whether RmlUi could be started */
    bool IsRmlUiReady( void ) const;

    virtual TGUIContextSet GetContextList( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetContextCount( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDriverName( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableFormTypes( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableWidgetTypes( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetAvailableFonts( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadFontFromAsset( const CString& assetPath ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDriverProperty( const CString& propertyName ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUI::CWidgetHandler* GetWidgetHandler( const CString& widgetType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TStringSet GetCapabilities( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetInterfaceVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** Lists the context while its owners keep it; the driver does not keep it alive */
    void AddContext( GUI::TGuiContextPtr& context );

    void RemoveContext( GUI::TGuiContextPtr& context );

    private:

    typedef GUCEF::map< CString, CRmlUIWidgetHandler* > THandlerMap;
    typedef CORE::CTWeakPtr< GUI::CIGUIContext, MT::CMutex > TGuiContextWeakPtr;
    typedef GUCEF::set< TGuiContextWeakPtr > TWeakContextSet;

    CRmlUIDriver( const CRmlUIDriver& src );
    CRmlUIDriver& operator=( const CRmlUIDriver& src );

    private:

    THandlerMap m_handlers;
    TWeakContextSet m_contexts;
    TStringSet m_fonts;
    bool m_rmlUiReady;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERRMLUI_CRMLUIDRIVER_H ? */
