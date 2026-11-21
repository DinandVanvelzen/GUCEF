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
#ifndef GUCEF_GUI_CVIEWPORTFORM_H
#define GUCEF_GUI_CVIEWPORTFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CIVIEWPORTCONTROLLER_H
#include "gucefGUI_CIViewportController.h"
#define GUCEF_GUI_CIVIEWPORTCONTROLLER_H
#endif /* GUCEF_GUI_CIVIEWPORTCONTROLLER_H ? */

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

class CDrawSurface;

/**
 *  A view the application draws or renders: a DrawSurface that fills the form with a tool bar of toggles above it. The
 *  toggles are the application's (an id, a text, a tooltip and a check state, or a list of choices): wireframe, shadows, a
 *  camera, whatever the view has. The form notifies ToggleChangedEvent when the user changes one.
 *
 *  A CIViewportController gets what the user does on the surface in the coordinates of the surface: the pointer (also
 *  outside the surface while a button that was pressed on it is held), the wheel, the keys while the surface has the
 *  keyboard focus (a click gives it), and the size of the surface when it changes, so orbit, pan, zoom and pick are the
 *  application's. What the surface shows is the application's as well: it draws on GetSurface(), or a driver shows a render
 *  texture in it (the form itself knows nothing of a renderer). SurfaceResizedEvent (TSurfaceSizeEventData) is notified when
 *  the size of the surface changes, after the frame that laid it out, and once when it is first known.
 *
 *  The form builds its layout in code, as a window (Build()) or in the place of a widget of another form (Build( placeholder )).
 */
class GUCEF_GUI_PUBLIC_CPP CViewportForm : public CFormEx
{
    public:

    static const CORE::CEvent ToggleChangedEvent;
    static const CORE::CEvent SurfaceResizedEvent;

    static void RegisterEvents( void );

    /** A toggle of the tool bar: a check box, or with 'choices' a list to pick from */
    struct GUCEF_GUI_PUBLIC_CPP SToggle
    {
        CString id;
        CString text;
        CString tooltip;
        bool checked;                       /**< a check box: its state */
        CString::StringVector choices;      /**< not empty: a list of choices instead of a check box */
        Int32 choice;                       /**< the choice picked, an index into 'choices' */

        SToggle( void );
        SToggle( const CString& toggleId, const CString& toggleText, const CString& toggleTooltip = CString::Empty, bool isChecked = false );
    };
    typedef struct SToggle TToggle;
    typedef GUCEF::vector< TToggle > TToggleVector;

    /** The data of ToggleChangedEvent */
    struct GUCEF_GUI_PUBLIC_CPP SToggleChange
    {
        CString id;
        bool checked;
        Int32 choice;       /**< -1 for a check box */

        SToggleChange( void );
    };
    typedef struct SToggleChange TToggleChange;
    typedef CORE::CTCloneableObj< TToggleChange > TToggleChangedEventData;

    /** The data of SurfaceResizedEvent */
    struct SSurfaceSize
    {
        UInt32 width;
        UInt32 height;
    };
    typedef struct SSurfaceSize TSurfaceSize;
    typedef CORE::CTCloneableObj< TSurfaceSize > TSurfaceSizeEventData;

    CViewportForm( void );

    explicit CViewportForm( CIGUIContext* context );

    virtual ~CViewportForm();

    /** Makes the form as a window of the context; false when it has no context */
    bool Build( const CString& title = "View" );

    /** Makes the form in the place of 'placeholder', which it fills (as a settings page is built in its pane) */
    bool Build( CWidget* placeholder );

    /*
     *  The surface
     */

    /** The surface to draw on or to show a render texture in; NULL before the form is built */
    CDrawSurface* GetSurface( void );

    /** Where the surface is in the coordinates of the context; empty before the first frame laid it out */
    CRect GetSurfaceRect( void );

    /** The size of the surface in pixels, as of the last frame */
    void GetSurfaceSize( UInt32& width, UInt32& height );

    /** Sees whether the size of the surface changed and tells so; the form does it after every frame of its context */
    void CheckSurfaceSize( void );

    /** The controller that gets the input of the surface (not owned, NULL for none); it is told the size of the surface at once when that is known */
    void SetController( CIViewportController* controller );

    CIViewportController* GetController( void ) const;

    /*
     *  Toggles
     */

    /** Replaces the toggles of the tool bar; it is hidden when there are none */
    void SetToggles( const TToggleVector& toggles );

    const TToggleVector& GetToggles( void ) const;

    bool SetToggleChecked( const CString& id, bool checked );

    bool GetToggleChecked( const CString& id ) const;

    bool SetToggleChoice( const CString& id, Int32 choice );

    Int32 GetToggleChoice( const CString& id ) const;

    /** As SetToggleChoice() but without ToggleChangedEvent: for the application that follows its own state (the choice is what the application did, not what the user did) */
    bool SetToggleChoiceSilently( const CString& id, Int32 choice );

    /**
     *  Shows a text in the list of a toggle that is none of its choices ("Custom": the state the choices cannot say); the choice
     *  is -1 then. No ToggleChangedEvent. Picking a choice of the list, or SetToggleChoice() and SetToggleChoiceSilently(), puts the choices back.
     */
    bool SetToggleDisplayText( const CString& id, const CString& text );

    /** The widget of a toggle (a check box or a combo box), NULL for an id that is none */
    CWidget* GetToggleWidget( const CString& id );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CViewportForm > TEventCallback;

    void OnMouseMove( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnMouseDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnMouseUp( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnMouseWheel( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyUp( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCaptureLost( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnToggleChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnToggleUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnToggleValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameRendered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title );
    void BuildToggleWidgets( void );
    void LayoutToolbar( void );
    void SubscribeToggleWidgets( void );
    Int32 FindToggle( const CString& id ) const;
    Int32 FindToggleOfWidget( CORE::CNotifier* widget ) const;
    void ChangeToggle( UInt32 index, bool checked, Int32 choice );

    CViewportForm( const CViewportForm& src );
    CViewportForm& operator=( const CViewportForm& src );

    private:

    TToggleVector m_toggles;
    CIViewportController* m_controller;
    UInt32 m_lastWidth;
    UInt32 m_lastHeight;
    bool m_sizeKnown;
    bool m_updatingWidgets;
    bool m_frameSubscribed;
    GUCEF::vector< Int32 > m_labelWidths;     /**< the room a toggle keeps for its label, 0 for a check box */
    GUCEF::vector< Int32 > m_widgetWidths;    /**< the width of the check box or combo box of a toggle */
    Int32 m_toolbarWidth;                     /**< the width the tool bar was laid out for, -1 for not yet */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CVIEWPORTFORM_H ? */
