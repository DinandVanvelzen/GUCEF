/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H
#define GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H

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

#ifndef GUCEF_MULTIPLAY_CILOBBY_H
#include "gucefMULTIPLAY_CILobby.h"
#define GUCEF_MULTIPLAY_CILOBBY_H
#endif /* GUCEF_MULTIPLAY_CILOBBY_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#include "gucefMULTIPLAY_CLobbySettingsSchema.h"
#define GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSETTINGSSCHEMA_H ? */

#ifndef GUCEF_GAMEGUI_MACROS_H
#include "gucefGAMEGUI_macros.h"
#define GUCEF_GAMEGUI_MACROS_H
#endif /* GUCEF_GAMEGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The data of CLobbySettingsPanelForm::SettingChangedEvent */
class GUCEF_GAMEGUI_EXPORT_CPP CSettingChangedEventData : public CORE::CICloneable
{
    public:

    CORE::CString key;          /**< the key of the setting in the schema */
    CORE::CString value;
    bool proposed;              /**< the player may only propose the change: it became a vote, the control shows the setting as it is */

    CSettingChangedEventData( void );

    CSettingChangedEventData( const CORE::CString& key, const CORE::CString& value, bool proposed );

    virtual ~CSettingChangedEventData();

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

/**
 *  The game settings of a lobby, as an embeddable form: it fills a placeholder widget (as CPathFieldForm does) with a
 *  control per setting of the schema, a label next to it:
 *      BOOL   a check box              INT and FLOAT   a spinner (range, step and decimals from the definition)
 *      ENUM   a combo box              STRING          an edit box (the change is made on Enter or when the focus leaves)
 *  Settings with a group are placed in a group box with the name of the group. The groups stand below each other, each as high as its
 *  rows need (side by side in two or three columns when the panel is wide enough: 420 pixels for each), in a scroll panel that fills the placeholder: what does not fit is scrolled. A label that is wider than the room it has next to
 *  its control is broken into lines, its row grows with them (also when the width of the panel changes). A placeholder that is a scroll panel itself
 *  or is as high as its content (AutoHeight) is the room of the groups, there the form adds no scroll panel of its own. A setting with a unit (INT, FLOAT, ENUM and
 *  STRING) shows it after the control. A setting with enabledByKey is enabled only while the setting it names has the value
 *  enabledByValue (checked whenever any setting changes, whoever changed it, so a change that arrives from the network counts
 *  too); while it is disabled it keeps and shows its value, and a change of it is not sent. A key the schema does not have is no gate.
 *
 *  Bound to a lobby (SetLobby()) the schema and the values are the lobby's. A change of the player goes to
 *  CILobby::SetSetting(), which applies it or, for a setting with the VOTE policy and a member that is not the boss,
 *  makes it a proposal: the label of such a control says "(propose)" and after the change the control shows the setting
 *  as it still is. A control is enabled when the member may change the setting (BOSS: the boss; VOTE and ANY: every
 *  player; OWNER: every player for the value of its own) and the lobby is open. A per member (OWNER) setting shows and
 *  changes the value of the local member, which the lobby keys as "<memberId>/<key>".
 *
 *  Without a lobby the panel is a plain form for a schema (SetSchema(), SetValues(), GetValues()), as the form that
 *  creates a lobby uses it.
 *
 *  Every change of the player is notified with SettingChangedEvent.
 */
class GUCEF_GAMEGUI_EXPORT_CPP CLobbySettingsPanelForm : public GUI::CFormEx
{
    public:

    static const CORE::CEvent SettingChangedEvent;      /**< CSettingChangedEventData */

    static void RegisterEvents( void );

    /** Added to the label of a setting the player can only propose */
    static const CORE::CString ProposeSuffix;

    CLobbySettingsPanelForm( void );

    virtual ~CLobbySettingsPanelForm();

    /** Makes the controls in 'placeholder', which they fill */
    bool Build( GUI::CWidget* placeholder );

    /** The settings the panel shows, without a lobby; the values are the defaults of the schema */
    void SetSchema( const MULTIPLAY::CLobbySettingsSchema& schema );

    /** Values for settings by key, without a lobby; keys the schema does not have are kept for GetValues() only when it has them */
    void SetValues( const CORE::CValueList& values );

    /** The values the controls show, by schema key */
    void GetValues( CORE::CValueList& values );

    /** The lobby whose settings are shown and changed; none to go back to a plain form. Takes the schema of the lobby. */
    void SetLobby( MULTIPLAY::CILobbyPtr lobby );

    const MULTIPLAY::CILobbyPtr& GetLobby( void ) const;

    const MULTIPLAY::CLobbySettingsSchema& GetSchema( void ) const;

    /** The number of controls: one per setting */
    UInt32 GetSettingCount( void ) const;

    /** The control of the setting by its schema key, NULL when there is none */
    GUI::CWidget* GetControl( const CORE::CString& key );

    /** The text of the label of a setting (the text of the check box for a BOOL) */
    CORE::CString GetLabelText( const CORE::CString& key );

    /** The text of the unit label after the control of a setting, empty when it has no unit */
    CORE::CString GetUnitText( const CORE::CString& key );

    /** Whether the setting it depends on has the value it needs, true when it depends on none; whether the player may change it is IsSettingEditable() */
    bool IsGateOpen( const CORE::CString& key );

    /** The value the control of the setting shows, in the text form lobbies keep it */
    CORE::CString GetValue( const CORE::CString& key );

    /** Whether the player can change the setting now */
    bool IsSettingEditable( const CORE::CString& key );

    /** Whether the change of the player is only a proposal for a vote */
    bool IsProposalOnly( const CORE::CString& key );

    /** Reads the values and the rights from the lobby (or the values given) again */
    void Refresh( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CLobbySettingsPanelForm > TEventCallback;

    struct SControl
    {
        MULTIPLAY::CLobbySettingDef def;
        CORE::CString widgetName;
        CORE::CString labelName;                    /**< empty for a check box, which carries its own text */
        CORE::CString unitName;                     /**< empty without a unit */
    };
    typedef struct SControl TControl;
    typedef GUCEF::vector< TControl > TControlVector;
    typedef GUCEF::map< CORE::CString, CORE::CString > TValueMap;

    void BuildLayout( CORE::CDataNode& layout );
    void AddControlNodes( CORE::CDataNode& parent, UInt32 index );
    void AddBlockNodes( CORE::CDataNode& parent, UInt32 first, UInt32 count, bool grouped, UInt32& groupCount );
    UInt32 GetColumnsForWidth( Int32 width ) const;
    void UpdateColumns( void );
    void FitRows( void );
    bool Rebuild( void );
    Int32 FindControlIndex( const CORE::CString& key ) const;
    Int32 FindControlIndexOfWidget( CORE::CNotifier* widget );
    CORE::CString GetValueKey( const TControl& control ) const;
    CORE::CString GetKnownValue( const TControl& control ) const;
    CORE::CString ReadControl( const TControl& control );
    void ShowControl( const TControl& control, const CORE::CString& value );
    bool CanChange( const TControl& control ) const;
    bool IsProposal( const TControl& control ) const;
    bool IsGateOpen( const TControl& control ) const;
    void Commit( UInt32 index );

    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLobbyStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnControlChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CLobbySettingsPanelForm( const CLobbySettingsPanelForm& src );
    CLobbySettingsPanelForm& operator=( const CLobbySettingsPanelForm& src );

    private:

    GUI::CWidget* m_placeholder;
    MULTIPLAY::CLobbySettingsSchema m_schema;
    TValueMap m_values;
    MULTIPLAY::CILobbyPtr m_lobby;
    TControlVector m_controls;
    bool m_hostMakesRoom;                           /**< the placeholder scrolls or is as high as its content: the form does not scroll */
    UInt32 m_columns;                               /**< the columns of groups the layout has */
    UInt32 m_wantedColumns;                         /**< the columns the width of the panel has room for */
    Int32 m_fittedWidth;                            /**< the width of the panel the rows were fitted to */
    bool m_subscribedToFrames;
    bool m_updating;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GAMEGUI_CLOBBYSETTINGSPANELFORM_H ? */
