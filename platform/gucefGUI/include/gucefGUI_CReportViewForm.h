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
#ifndef GUCEF_GUI_CREPORTVIEWFORM_H
#define GUCEF_GUI_CREPORTVIEWFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

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
 *  A list of the findings of some work, such as the problems of a conversion: a grid of entries (by default the columns
 *  severity, asset, message and details) with a colour per severity, a check box per severity that shows or hides its
 *  entries, a filter line, and a text box that shows the details of the selected entry.
 *
 *  The entries come as a list in a CORE::CDataNode (each child node is an entry, its attributes "severity" (info, warning or
 *  error, or 0 to 2), "asset", "message" and "details" and any other attributes the columns name) or one by one with
 *  AddEntry() while the work runs. The grid is rebuilt from the entries that pass the filters: at most once every
 *  RefreshIntervalInMs of frame time while entries keep arriving (the form updates on every FrameStartedEvent of its
 *  context, Update() does it with a manual clock), or at once with Refresh(); what the functions answer is up to date at
 *  once. The column set is the application's: SetColumns() names the key of the entries each column shows (severity,
 *  asset, message, details or any other attribute of the entries), its title and its width relative to the others.
 *
 *  The filter line keeps the entries that have its text in the value of any column, letters in any case. The form knows
 *  nothing of what an asset is. A double click or Enter on an entry notifies EntryActivatedEvent (TCloneableInt32 with the
 *  index of the entry, in the order they were added).
 *
 *  The form builds its layout in code, as a window (Build()) or in the place of a widget of another form (Build( placeholder )).
 */
class GUCEF_GUI_PUBLIC_CPP CReportViewForm : public CFormEx
{
    public:

    static const CORE::CEvent EntryActivatedEvent;

    static void RegisterEvents( void );

    enum ESeverity
    {
        SEVERITY_INFO = 0   ,
        SEVERITY_WARNING    ,
        SEVERITY_FAILURE    ,   /**< not named SEVERITY_ERROR as that is a macro of the Windows headers */

        SEVERITY_COUNT
    };

    /** How long entries may wait before the grid shows them, in milliseconds of frame time */
    enum EDefaults
    {
        RefreshIntervalInMs = 100
    };

    /** One finding */
    struct GUCEF_GUI_PUBLIC_CPP SEntry
    {
        ESeverity severity;
        CString asset;
        CString message;
        CString details;
        GUCEF::map< CString, CString > values;      /**< the other attributes, by key, for columns of the application */

        SEntry( void );
        SEntry( ESeverity entrySeverity, const CString& entryAsset, const CString& entryMessage, const CString& entryDetails = CString::Empty );

        /** The value a column with this key shows: "severity", "asset", "message" and "details", else a value of 'values' */
        CString GetValue( const CString& key ) const;
    };
    typedef struct SEntry TEntry;
    typedef GUCEF::vector< TEntry > TEntryVector;

    /** A column of the grid */
    struct GUCEF_GUI_PUBLIC_CPP SColumn
    {
        CString key;
        CString title;
        Int32 width;        /**< relative to the widths of the other columns */

        SColumn( void );
        SColumn( const CString& columnKey, const CString& columnTitle, Int32 columnWidth );
    };
    typedef struct SColumn TColumn;
    typedef GUCEF::vector< TColumn > TColumnVector;

    CReportViewForm( void );

    explicit CReportViewForm( CIGUIContext* context );

    virtual ~CReportViewForm();

    /** Makes the form as a window of the context; false when it has no context */
    bool Build( const CString& title = "Report" );

    /** Makes the form in the place of 'placeholder', which it fills (as a settings page is built in its pane) */
    bool Build( CWidget* placeholder );

    /*
     *  Columns
     */

    /** Replaces the columns; severity, asset, message and details with their titles when none are given */
    void SetColumns( const TColumnVector& columns );

    const TColumnVector& GetColumns( void ) const;

    /*
     *  Entries
     */

    /** Replaces the entries with the children of the node, one entry each */
    void SetEntries( const CORE::CDataNode& list );

    void SetEntries( const TEntryVector& entries );

    /** Adds an entry at the end (while work runs); the grid follows with the next refresh */
    void AddEntry( const TEntry& entry );

    void AddEntry( ESeverity severity, const CString& asset, const CString& message, const CString& details = CString::Empty );

    void ClearEntries( void );

    UInt32 GetEntryCount( void ) const;

    const TEntry& GetEntry( UInt32 index ) const;

    /** How many entries there are of a severity, shown or not */
    UInt32 GetEntryCount( ESeverity severity ) const;

    /*
     *  What is shown
     */

    void SetSeverityShown( ESeverity severity, bool shown );

    bool IsSeverityShown( ESeverity severity ) const;

    /** Keeps the entries that have the text in a column, letters in any case; empty for all */
    void SetFilter( const CString& text );

    const CString& GetFilter( void ) const;

    /** How many entries pass the severities and the filter */
    UInt32 GetShownCount( void ) const;

    /** The index of the entry in the grid row, -1 for a row that is not there (the grid is up to date after Refresh()) */
    Int32 GetShownEntryIndex( UInt32 row ) const;

    /** Selects an entry that is shown; false for one that is not */
    bool SelectEntry( Int32 index );

    /** The index of the selected entry, -1 when none */
    Int32 GetSelectedEntry( void ) const;

    /** Shows the entries that came and the filters changed since the last time in the grid now */
    void Refresh( void );

    /** Lets frame time pass, which brings the grid up to date when entries wait for longer than RefreshIntervalInMs */
    void Update( UInt32 deltaInMs );

    /** The colour of the rows of a severity */
    void SetSeverityColour( ESeverity severity, const CColour& colour );

    CColour GetSeverityColour( ESeverity severity ) const;

    static CString SeverityToString( ESeverity severity );

    /** Reads a name in any letter case ("info", "warning", "error") or a number; false for another text */
    static bool SeverityFromString( const CString& text, ESeverity& severity );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CReportViewForm > TEventCallback;

    void OnSeverityChecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSeverityUnchecked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFilterTextChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEntriesSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEntriesItemActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title );
    bool IsShown( const TEntry& entry ) const;
    void RebuildShown( void );
    void ShowColumns( void );
    void ShowRows( void );
    void ShowDetails( void );
    void ShowStatus( void );
    void ChangeSeverityShown( CORE::CNotifier* checkbox, bool shown );

    CReportViewForm( const CReportViewForm& src );
    CReportViewForm& operator=( const CReportViewForm& src );

    private:

    TColumnVector m_columns;
    TEntryVector m_entries;
    GUCEF::vector< UInt32 > m_shown;        /**< the entries the grid shows, in its order */
    bool m_severityShown[ SEVERITY_COUNT ];
    CColour m_severityColours[ SEVERITY_COUNT ];
    CString m_filter;
    CString m_filterLower;
    bool m_dirty;
    UInt32 m_waitedInMs;
    Int32 m_selectedEntry;
    bool m_updatingWidgets;
    bool m_frameSubscribed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CREPORTVIEWFORM_H ? */
