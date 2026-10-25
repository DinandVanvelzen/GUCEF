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

#ifndef GUCEF_GUI_CFORM_H
#define GUCEF_GUI_CFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef GUCEF_GUI_CFORMBACKEND_H
#include "gucefGUI_CFormBackend.h"
#define GUCEF_GUI_CFORMBACKEND_H
#endif /* GUCEF_GUI_CFORMBACKEND_H ? */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#include "gucefGUI_CIGUIContext.h"
#define GUCEF_GUI_CIGUICONTEXT_H
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

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

class CGUIContext;

/**
 *  A piece of user interface loaded from a layout: a root widget with everything below it. A gucefGUI
 *  layout (see CFormLayout) works with every driver and can be saved again after changes, which is what
 *  an editor does. A layout in the native format of the backend (RML for RmlUi ...) is handed to the
 *  driver's form backend instead, if it has one.
 *
 *  Applications derive their screens from CForm (or CFormEx) and wire up the widgets in OnPostLayoutLoad().
 *  A form without a parent is shown as a root of the context: Show() puts it on top, ShowModal() also
 *  keeps the input in it, Hide() takes it away.
 */
class GUCEF_GUI_PUBLIC_CPP CForm : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent LayoutLoadedEvent;
    static const CORE::CEvent LayoutSavedEvent;

    static void RegisterEvents( void );

    public:

    CForm( void );

    CForm( CIGUIContext* context );

    virtual ~CForm();

    /** Loads a layout file (from disk, else the VFS); the DStore codec is chosen by the extension */
    virtual bool LoadLayout( const CString& layoutStoragePath );

    virtual bool SaveLayout( const CString& layoutStoragePath );

    /** Loads a layout from an open resource, read with the XML codec */
    virtual bool LoadLayout( CORE::CIOAccess& layoutStorage );

    virtual bool SaveLayout( CORE::CIOAccess& layoutStorage );

    /** Loads a layout from an open resource, read with the given DStore codec (for example "xml" or "json") */
    bool LoadLayout( CORE::CIOAccess& layoutStorage, const CString& codecType );

    bool SaveLayout( CORE::CIOAccess& layoutStorage, const CString& codecType );

    /** Builds the form from a layout tree (the GUILayout node or a tree holding it) */
    bool LoadLayout( const CORE::CDataNode& layout );

    /** The layout of the form as it is now, ready to be written with any codec */
    bool SaveLayout( CORE::CDataNode& layout );

    /** The layout tree the form was built from, with nodes gucefGUI does not know left in */
    const CORE::CDataNode& GetDescription( void ) const;

    void SetContext( CIGUIContext* context );

    CIGUIContext* GetContext( void );

    /** The context, if it is a gucefGUI context (every current driver's is) */
    CGUIContext* GetGUIContext( void );

    virtual bool SetParent( CForm* parentForm );

    virtual bool SetParent( CWidget* parentWidget );

    virtual CWidget* GetParent( void );

    virtual const CWidget* GetRootWidget( void ) const;

    virtual CWidget* GetRootWidget( void );

    /** A widget of the form by name (at any depth) or by path ("Panel/OkButton"); the root by its own name */
    virtual CWidget* GetWidget( const CString& widgetName );

    /** GetWidget() as the class asked for, NULL when it is another widget type: T has to have GetRegisteredTypeName(), see CWidget::As() */
    template < class T >
    T* GetWidgetAs( const CString& widgetName );

    virtual bool IsVisible( void ) const;

    virtual bool SetVisibility( const bool isVisible );

    virtual bool Show( void );

    virtual bool ShowModal( void );

    virtual bool Hide( void );

    virtual const CString& GetClassTypeName( void ) const;

    virtual const CString& GetType( void ) const;

    protected:

    virtual void OnPreLayoutLoad( void );

    virtual void OnPostLayoutLoad( void );

    private:

    void DestroyContent( void );
    bool LoadNativeLayout( const CString& layoutStoragePath );
    bool LoadNativeLayout( CORE::CIOAccess& layoutStorage );

    CForm( const CForm& src );
    CForm& operator=( const CForm& src );

    private:

    CFormBackend* m_backend;
    CWidget* m_parentWidget;
    CIGUIContext* m_context;
    CWidget* m_root;
    CORE::CDataNode m_description;
    CFormLayout::TWidgetSourceMap m_sources;
};

/*-------------------------------------------------------------------------*/

template < class T >
T*
CForm::GetWidgetAs( const CString& widgetName )
{
    return CWidget::As< T >( GetWidget( widgetName ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CFORM_H ? */
