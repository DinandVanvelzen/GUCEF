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

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#define GUCEF_GUI_CROUTEDEVENTDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CICLONEABLE_H
#include "gucefMT_CICloneable.h"
#define GUCEF_MT_CICLONEABLE_H
#endif /* GUCEF_MT_CICLONEABLE_H ? */

#ifndef GUCEF_INPUT_KEYBOARD_H
#include "gucefINPUT_keyboard.h"
#define GUCEF_INPUT_KEYBOARD_H
#endif /* GUCEF_INPUT_KEYBOARD_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

#ifndef GUCEF_GUI_EWIDGETPROPERTY_H
#include "gucefGUI_EWidgetProperty.h"
#define GUCEF_GUI_EWIDGETPROPERTY_H
#endif /* GUCEF_GUI_EWIDGETPROPERTY_H ? */

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
 *  Base of the data that comes with every event that is routed through the widget tree.
 *  A routed event travels from the root down to the parent of the target (tunnel), reaches the
 *  target and then travels back up to the root (bubble). Every widget on the way notifies its own
 *  observers. An observer that sets 'handled' stops the event from travelling any further.
 */
class GUCEF_GUI_PUBLIC_CPP CRoutedEventData : public MT::CICloneable
{
    public:

    CWidget* source;        /**< the widget the event is about (the target) */
    TEventPhase phase;      /**< the phase in which the observer receives it */
    bool handled;           /**< set by an observer to stop the event */

    CRoutedEventData( void );
    virtual ~CRoutedEventData();

    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see MT::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );

    /**
     *  The data as routed event data when it is one of the classes of this file that are routed, else NULL
     *  (also for NULL); for the subclass itself use MT::CICloneable::CloneableAs()
     */
    static CRoutedEventData* FromCloneable( MT::CICloneable* data );
    static const CRoutedEventData* FromCloneable( const MT::CICloneable* data );

    /** Sets the position relative to the top left of the widget that is notified, for the data that has one; the others ignore it */
    virtual void SetLocalOrigin( Int32 widgetLeft, Int32 widgetTop );
};

/*-------------------------------------------------------------------------*/

/** Pointer (mouse, touch) events: move, down, up, click, enter and leave */
class GUCEF_GUI_PUBLIC_CPP CPointerEventData : public CRoutedEventData
{
    public:

    Int32 x;                /**< position in the logical coordinates of the GUI context */
    Int32 y;
    Int32 localX;           /**< position relative to the top left of the widget the observer belongs to */
    Int32 localY;
    TMouseButton button;    /**< the button that changed, MOUSEBUTTON_NONE for moves */
    UInt32 buttonStates;    /**< bit (1 << button) for every button that is held down */
    UInt32 modifiers;       /**< INPUT::KeyModifier flags */
    UInt32 clickCount;      /**< 1 for a click, 2 for a double click, 3 for a triple click */

    CPointerEventData( void );
    virtual ~CPointerEventData();

    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see MT::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );

    virtual void SetLocalOrigin( Int32 widgetLeft, Int32 widgetTop ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

class GUCEF_GUI_PUBLIC_CPP CWheelEventData : public CRoutedEventData
{
    public:

    Int32 x;
    Int32 y;
    Int32 localX;
    Int32 localY;
    Int32 delta;            /**< notches: positive is away from the user (up), negative towards the user */
    UInt32 modifiers;

    CWheelEventData( void );
    virtual ~CWheelEventData();

    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see MT::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );

    virtual void SetLocalOrigin( Int32 widgetLeft, Int32 widgetTop ) GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

class GUCEF_GUI_PUBLIC_CPP CKeyEventData : public CRoutedEventData
{
    public:

    INPUT::KeyCode keyCode;
    UInt32 modifiers;       /**< INPUT::KeyModifier flags */

    CKeyEventData( void );
    virtual ~CKeyEventData();

    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see MT::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/** Text typed by the user, after keyboard layout and input method: UTF-8 */
class GUCEF_GUI_PUBLIC_CPP CTextEventData : public CRoutedEventData
{
    public:

    CString text;

    CTextEventData( void );
    virtual ~CTextEventData();

    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see MT::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

class GUCEF_GUI_PUBLIC_CPP CNavigationEventData : public CRoutedEventData
{
    public:

    TNavigation direction;

    CNavigationEventData( void );
    virtual ~CNavigationEventData();

    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see MT::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

class GUCEF_GUI_PUBLIC_CPP CFocusEventData : public CRoutedEventData
{
    public:

    CWidget* other;         /**< the widget that lost (for a gain) or gained (for a loss) the focus, may be NULL */

    CFocusEventData( void );
    virtual ~CFocusEventData();

    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see MT::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/** A value of a widget changed because of the user (or a command), for example a slider or a selection */
class GUCEF_GUI_PUBLIC_CPP CValueChangedEventData : public CRoutedEventData
{
    public:

    EWidgetProperty property;       /**< the standard property that changed; EWidgetProperty_Unknown for another property, which customPropertyName names */
    CString customPropertyName;
    CString oldValue;
    CString newValue;

    CValueChangedEventData( void );
    virtual ~CValueChangedEventData();

    /** The name of the property that changed, the same for a standard property as the id turned into text */
    const CString& GetPropertyName( void ) const;

    /** Sets the property by name: a standard name becomes its id */
    void SetPropertyName( const CString& name );

    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see MT::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/** The data of CWidget::PropertyChangedEvent: which property of the widget changed. Not routed, so no CRoutedEventData. */
class GUCEF_GUI_PUBLIC_CPP CPropertyChangedEventData : public MT::CICloneable
{
    public:

    EWidgetProperty property;       /**< the standard property that changed; EWidgetProperty_Unknown for another property, which customPropertyName names */
    CString customPropertyName;

    CPropertyChangedEventData( void );
    explicit CPropertyChangedEventData( EWidgetProperty changedProperty );
    explicit CPropertyChangedEventData( const CString& changedPropertyName );
    virtual ~CPropertyChangedEventData();

    const CString& GetPropertyName( void ) const;

    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see MT::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------*/

/** An item of an item widget (list, menu, tree) was chosen */
class GUCEF_GUI_PUBLIC_CPP CItemEventData : public CRoutedEventData
{
    public:

    Int32 index;            /**< the index of the item, -1 when none */
    CString item;           /**< the text (for trees and menus: the path) of the item */

    CItemEventData( void );
    virtual ~CItemEventData();

    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see MT::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */
