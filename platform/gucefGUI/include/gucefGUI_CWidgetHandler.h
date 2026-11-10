/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CWIDGETHANDLER_H
#define GUCEF_GUI_CWIDGETHANDLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

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

class CWidget;

/**
 *  What a GUI driver supplies for one widget type: it makes the native object behind a gucefGUI
 *  widget and keeps it in step with the widget.
 *
 *  The widget (the facade) holds the property values; the handler only applies them. A property
 *  change reaches the handler once per frame (changes are batched) through ApplyProperty(), which
 *  looks the property up in the property map. A driver fills the map with MapProperty(); extra
 *  appliers can be put before or after the mapped one, or replace it, without subclassing. A widget
 *  can also override an applier for itself only (CWidget::SetPropertyApplierOverride()).
 *
 *  Handlers are shared by all widgets of a type and keep no per-widget state; per-widget native state
 *  lives behind CWidget::GetImplementationPtr().
 */
class GUCEF_GUI_PUBLIC_CPP CWidgetHandler
{
    public:

    /** Applies one property value to the native object; returns false when it could not */
    typedef bool (*TPropertyApplier)( CWidgetHandler& handler, CWidget& widget, const CString& value );

    /** Runs an imperative command (scroll to, select all ...) on the native object */
    typedef bool (*TCommandRunner)( CWidgetHandler& handler, CWidget& widget, const CORE::CValueList& arguments );

    typedef GUCEF::set< CString > TStringSet;

    CWidgetHandler( void );
    virtual ~CWidgetHandler();

    /** Makes the native object for the widget, below the native object of its parent (NULL: the root of the context) */
    virtual bool CreateNative( CWidget& widget ) = 0;

    /** Destroys the native object, the widget is going away */
    virtual void DestroyNative( CWidget& widget ) = 0;

    /** The widget got another parent (NULL for a root) */
    virtual void ApplyParent( CWidget& widget );

    /** The rectangle gucefGUI laid out for the widget, in logical pixels of the context (absolute) */
    virtual void ApplyLayout( CWidget& widget, const CRect& rect );

    /** The order of the children of the widget changed (z-order: last is on top) */
    virtual void ApplyChildOrder( CWidget& widget );

    /** The interaction state (hovered, pressed, focused) of the widget changed */
    virtual void ApplyState( CWidget& widget );

    /** Applies a property through the property map; unknown properties are ignored (return false) */
    virtual bool ApplyProperty( CWidget& widget, const CString& name, const CString& value );

    /** Runs a mapped command */
    virtual bool RunCommand( CWidget& widget, const CString& command, const CORE::CValueList& arguments );

    /** The native object, for an application that needs a feature gucefGUI does not offer */
    virtual void* GetNativeHandle( const CWidget& widget ) const;

    void MapProperty( const CString& name, TPropertyApplier applier );
    void PrependPropertyApplier( const CString& name, TPropertyApplier applier );
    void AppendPropertyApplier( const CString& name, TPropertyApplier applier );
    void ModifyPropertyApplier( const CString& name, TPropertyApplier applier );
    void MapCommand( const CString& name, TCommandRunner runner );

    /** The properties this handler maps; what is missing is reported as unsupported */
    TStringSet GetMappedProperties( void ) const;
    TStringSet GetMappedCommands( void ) const;
    bool IsPropertyMapped( const CString& name ) const;

    private:

    CWidgetHandler( const CWidgetHandler& src );
    CWidgetHandler& operator=( const CWidgetHandler& src );

    struct SApplierChain
    {
        GUCEF::vector< TPropertyApplier > before;
        TPropertyApplier main;
        GUCEF::vector< TPropertyApplier > after;

        SApplierChain( void );
    };
    typedef GUCEF::map< CString, SApplierChain > TPropertyMap;
    typedef GUCEF::map< CString, TCommandRunner > TCommandMap;

    TPropertyMap m_propertyMap;
    TCommandMap m_commandMap;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CWIDGETHANDLER_H ? */
