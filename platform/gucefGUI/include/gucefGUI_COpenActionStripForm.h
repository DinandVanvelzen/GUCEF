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

#ifndef GUCEF_GUI_COPENACTIONSTRIPFORM_H
#define GUCEF_GUI_COPENACTIONSTRIPFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CRECENTLOCATIONS_H
#include "gucefGUI_CRecentLocations.h"
#define GUCEF_GUI_CRECENTLOCATIONS_H
#endif /* GUCEF_GUI_CRECENTLOCATIONS_H ? */

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

/**
 *  The way in for an application that opens things from the file system: a row of buttons, one per way to open ("Open
 *  folder...", "Open file...", ...), and below it a list of the places that were opened lately. The form says what the user
 *  asked for and leaves the doing to the application, which shows the file system dialogs of its choice:
 *    - ActionRequestedEvent (TCloneableString with the id of the action): a button was activated (RequestAction() does the same);
 *    - RecentChosenEvent (TCloneableString with the path): a place of the list was chosen. The list shows its first row
 *      ("Open recent...") again afterwards, so the same place can be chosen twice.
 *
 *  The recent places are those of a CRecentLocations (not owned, SetRecentLocations()); the list follows its ChangedEvent.
 *  The buttons share the width of the strip by the length of their text, so it fits any panel; the strip is GetPreferredHeight() pixels high. It takes the
 *  place of a widget of another form (Build( placeholder )).
 */
class GUCEF_GUI_PUBLIC_CPP COpenActionStripForm : public CFormEx
{
    public:

    static const CORE::CEvent ActionRequestedEvent;
    static const CORE::CEvent RecentChosenEvent;

    static void RegisterEvents( void );

    enum ELayout
    {
        ROW_HEIGHT = 30     ,
        ROW_GAP = 4         ,
        PREFERRED_HEIGHT = 2 * ROW_HEIGHT + ROW_GAP + 8
    };

    struct GUCEF_GUI_PUBLIC_CPP SAction
    {
        CString id;
        CString label;
        CString tooltip;
    };
    typedef struct SAction TAction;
    typedef GUCEF::vector< TAction > TActionVector;

    explicit COpenActionStripForm( CIGUIContext* context );

    virtual ~COpenActionStripForm();

    /** A way to open, as a button; in the order they are added, before Build() */
    void AddAction( const CString& id, const CString& label, const CString& tooltip );

    /** The first row of the list of recent places, which is what the list shows when nothing is chosen */
    void SetRecentLabel( const CString& label );

    /** Makes the strip in 'placeholder', which it fills */
    bool Build( CWidget* placeholder );

    /** The recent places the list shows (not owned, NULL for none); it has to be there as long as the form has it */
    void SetRecentLocations( CRecentLocations* recent );

    CRecentLocations* GetRecentLocations( void ) const;

    /** Notifies ActionRequestedEvent as the button of the action does; false for an id that is not an action */
    bool RequestAction( const CString& id );

    /** Notifies RecentChosenEvent as the list does for the place at the index of the recent places; false for an index outside of the list */
    bool ChooseRecent( UInt32 index );

    const TActionVector& GetActions( void ) const;

    CWidget* GetActionWidget( const CString& id );

    /** The rows of the list: the first row and a row for each recent place */
    UInt32 GetRecentRowCount( void );

    static UInt32 GetPreferredHeight( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< COpenActionStripForm > TEventCallback;

    void OnActionActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRecentValueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRecentChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void FillRecent( void );
    static CString MakeFraction( UInt32 numerator, UInt32 denominator );

    COpenActionStripForm( const COpenActionStripForm& src );
    COpenActionStripForm& operator=( const COpenActionStripForm& src );

    private:

    TActionVector m_actions;
    CString m_recentLabel;
    CRecentLocations* m_recent;
    bool m_updatingWidgets;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_COPENACTIONSTRIPFORM_H ? */
