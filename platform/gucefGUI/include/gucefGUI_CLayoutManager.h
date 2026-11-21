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

#ifndef GUCEF_GUI_CLAYOUTMANAGER_H
#define GUCEF_GUI_CLAYOUTMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CLAYOUTSTATE_H
#include "gucefGUI_CLayoutState.h"
#define GUCEF_GUI_CLAYOUTSTATE_H
#endif /* GUCEF_GUI_CLAYOUTSTATE_H ? */

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
 *  Keeps the layout of a window as the user shaped it, and the named layouts (presets) an application offers ("Browse", "Review",
 *  "Preview"): it watches the splitters and foldable panels below a widget, applies and captures CLayoutState values, and writes
 *  and reads the current layout and the presets of the user in the settings of the application.
 *
 *  Use: build the window, Attach() the root, add the presets of the application (AddPreset(), the built in ones are not saved),
 *  then Load() what the settings hold: that puts the layout of the last session back. Observe LayoutChangedEvent to know that the
 *  user changed it (to save the settings at a quiet moment) and PresetAppliedEvent to show which preset is on.
 *
 *  Events: LayoutChangedEvent when a splitter or a foldable panel changed because of the user or other code, but not because of
 *  ApplyPreset() or ApplyState(); PresetAppliedEvent (TCloneableString: the name) after a preset was applied. The active preset
 *  name is cleared as soon as the layout changes by hand: GetActivePreset() is then empty (the layout is "Custom").
 *
 *  The widgets must have unique names (CLayoutState), and widgets that are made after Attach() are watched after Rescan().
 */
class GUCEF_GUI_PUBLIC_CPP CLayoutManager : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent LayoutChangedEvent;
    static const CORE::CEvent PresetAppliedEvent;

    static void RegisterEvents( void );

    typedef GUCEF::vector< CString > TStringVector;

    CLayoutManager( void );

    virtual ~CLayoutManager();

    /** Watches the splitters and foldable panels below the widget (and itself); the widget must outlive the manager or be detached */
    bool Attach( CWidget* root );

    void Detach( void );

    /** Watches the splitters and foldable panels that were added since Attach() */
    void Rescan( void );

    CWidget* GetRoot( void ) const;

    /** The number of widgets that are watched */
    UInt32 GetWatchedCount( void ) const;

    /** Reads the layout as it is now */
    void CaptureState( CLayoutState& state ) const;

    /** Puts the state on the widgets without LayoutChangedEvent; clears the active preset unless 'presetName' says which one it is */
    UInt32 ApplyState( const CLayoutState& state, const CString& presetName = CString::Empty );

    /** Adds or replaces a named layout; a built in preset is not written by Save() */
    void AddPreset( const CString& name, const CLayoutState& state, bool builtIn = true );

    /** Makes a preset of the layout as it is now (a preset of the user, which Save() writes) */
    bool AddPresetFromCurrent( const CString& name );

    bool RemovePreset( const CString& name );

    bool HasPreset( const CString& name ) const;

    bool GetPreset( const CString& name, CLayoutState& state ) const;

    TStringVector GetPresetNames( void ) const;

    /** Applies the preset; false when there is none with the name */
    bool ApplyPreset( const CString& name );

    /** The preset that was applied and not changed by hand since, empty for a layout that is custom */
    const CString& GetActivePreset( void ) const;

    /** Writes the layout as it is, the active preset and the presets of the user as the child 'name' of the section, replacing what it had */
    bool Save( CORE::CDataNode& section, const CString& name = "Layout" ) const;

    /** Reads what Save() wrote and applies it (when attached, else on Attach()); false when the section has no such child. Presets that Load() reads replace those of the same name */
    bool Load( const CORE::CDataNode& section, const CString& name = "Layout" );

    private:

    CLayoutManager( const CLayoutManager& src );
    CLayoutManager& operator=( const CLayoutManager& src );

    typedef CORE::CTEventHandlerFunctor< CLayoutManager > TEventCallback;

    struct SPreset
    {
        CLayoutState state;
        bool builtIn;

        SPreset( void ) : state(), builtIn( true ) {}
    };

    typedef GUCEF::map< CString, SPreset > TPresetMap;
    typedef GUCEF::set< CWidget* > TWidgetSet;

    void Watch( CWidget& widget );
    void OnLayoutWidgetChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    CWidget* m_root;
    TWidgetSet m_watched;
    TPresetMap m_presets;
    CString m_activePreset;
    CLayoutState m_loaded;
    CString m_loadedActive;
    bool m_hasLoaded;
    bool m_applying;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CLAYOUTMANAGER_H ? */
