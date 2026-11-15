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
#ifndef GUCEF_GUI_CTRANSFERQUEUEFORM_H
#define GUCEF_GUI_CTRANSFERQUEUEFORM_H

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

#ifndef GUCEF_GUI_CITRANSFERQUEUEMODEL_H
#include "gucefGUI_CITransferQueueModel.h"
#define GUCEF_GUI_CITRANSFERQUEUEMODEL_H
#endif /* GUCEF_GUI_CITRANSFERQUEUEMODEL_H ? */

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
 *  A list of transfers (downloads, uploads, copies, background jobs) with a progress bar, the state and the buttons that
 *  fit the state of each, and a summary line at the bottom. The transfers are those of a CITransferQueueModel: the form
 *  knows nothing of what a transfer is.
 *
 *  Each transfer is a row of two lines: the name at the left and the state, the percentage, the bytes, the rate and the time
 *  left (or the message of a failure) at the right; below that the progress bar and the buttons of the actions the model
 *  offers for the entry (Pause, Resume, Cancel, Retry, Remove, Open folder, from the right edge without gaps); a third small
 *  line has the kind and the detail of the entry. The rows are coloured by state and the selected row is highlighted. When
 *  there is no model or it has no entries, "No transfers" is shown in place of the rows.
 *
 *  The rows are pooled: a row widget exists for every row in view (and a few more) and is bound to another entry when the list
 *  scrolls, so a queue of thousands of entries costs the widgets of one screen. What the queries answer for an entry that is
 *  not in view comes from the entries, not from a widget.
 *
 *  The model tells that something changed with EntriesChangedEvent, which only marks the form as dirty. The form reads the
 *  entries again at most once every RefreshIntervalInMs of frame time while it is dirty (it updates on every FrameStartedEvent of
 *  its context, Update() does it with a manual clock), or at once with Refresh(). The first refresh after SetModel() is at once.
 *
 *  What the user asks for with a button is notified as an event of the form that has the id of the entry as
 *  CORE::TCloneableString (PauseRequestedEvent, ResumeRequestedEvent, CancelRequestedEvent, RetryRequestedEvent,
 *  RemoveRequestedEvent, OpenFolderRequestedEvent) and with SetPerformActions( true ), the default, also carried out by
 *  CITransferQueueModel::PerformAction(); an application either subscribes or lets the model do it. A click on a row selects its
 *  entry (SelectionChangedEvent with the id) and Delete removes the selected entry when the model offers that.
 *
 *  The form builds its layout in code, as a window (Build()) or in the place of a widget of another form (Build( placeholder )).
 *  The model is not owned and has to outlive the form or be taken away with SetModel( GUCEF_NULL ).
 */
class GUCEF_GUI_PUBLIC_CPP CTransferQueueForm : public CFormEx
{
    public:

    static const CORE::CEvent PauseRequestedEvent;          /**< TCloneableString: the id of the entry */
    static const CORE::CEvent ResumeRequestedEvent;         /**< TCloneableString: the id of the entry */
    static const CORE::CEvent CancelRequestedEvent;         /**< TCloneableString: the id of the entry */
    static const CORE::CEvent RetryRequestedEvent;          /**< TCloneableString: the id of the entry */
    static const CORE::CEvent RemoveRequestedEvent;         /**< TCloneableString: the id of the entry */
    static const CORE::CEvent OpenFolderRequestedEvent;     /**< TCloneableString: the id of the entry */
    static const CORE::CEvent SelectionChangedEvent;        /**< TCloneableString: the id of the selected entry, empty when the selection was cleared */

    static void RegisterEvents( void );

    typedef CITransferQueueModel::TEntry TEntry;
    typedef CITransferQueueModel::TEntryVector TEntryVector;
    typedef CITransferQueueModel::ETransferAction ETransferAction;

    enum EDefaults
    {
        RefreshIntervalInMs = 250   ,   /**< how long changes of the model may wait before the rows show them, in milliseconds of frame time */
        RowHeight           = 56        /**< the height of the space of a row in pixels with a small font; GetRowHeight() is the height in use */
    };

    CTransferQueueForm( void );

    explicit CTransferQueueForm( CIGUIContext* context );

    virtual ~CTransferQueueForm();

    /** Makes the form as a window of the context; false when it has no context */
    bool Build( const CString& title = "Transfers" );

    /** Makes the form in the place of 'placeholder', which it fills (as a settings page is built in its pane) */
    bool Build( CWidget* placeholder );

    /*
     *  The model
     */

    /** Shows the transfers of the model (not owned, NULL for none); the entries are read at once */
    void SetModel( CITransferQueueModel* model );

    CITransferQueueModel* GetModel( void ) const;

    /** Whether a press of a button also calls CITransferQueueModel::PerformAction(); the event is notified in any case */
    void SetPerformActions( bool perform );

    bool GetPerformActions( void ) const;

    /** Reads the entries of the model and shows them now */
    void Refresh( void );

    /** Lets frame time pass, which brings the rows up to date when the model changed and RefreshIntervalInMs have passed */
    void Update( UInt32 deltaInMs );

    /** How often the entries were read from the model */
    UInt32 GetRefreshCount( void ) const;

    /*
     *  The entries as of the last refresh
     */

    UInt32 GetEntryCount( void ) const;

    const TEntry& GetEntry( UInt32 index ) const;

    /** The index of the entry with the id, -1 for an id that is not there */
    Int32 FindEntryIndex( const CString& id ) const;

    /*
     *  Selection
     */

    /** The id of the selected entry, empty when none */
    const CString& GetSelectedEntryId( void ) const;

    /** Selects the entry with the id and scrolls it into view; an empty id clears the selection. False for an id that is not there */
    bool SelectEntry( const CString& id );

    /*
     *  Actions
     */

    /**
     *  Does what a press of the button of the action does: notifies the event of the action and, with SetPerformActions( true ),
     *  has the model carry it out. False when the entry is not there or the model does not offer the action for it
     */
    bool RequestAction( const CString& id, ETransferAction action );

    /*
     *  What is shown
     */

    /** Whether "No transfers" is shown (no model, or a model without entries) */
    bool IsEmptyStateShown( void ) const;

    /** How many row widgets show an entry now: the entries in view, at most the entries */
    UInt32 GetRealizedRowCount( void ) const;

    /** Whether the button of the action is shown on the row of the entry (for an entry out of view: whether the model offers it) */
    bool IsActionShown( const CString& id, ETransferAction action ) const;

    /** The widget of the row that shows the entry, NULL for an entry that is not in view */
    CWidget* GetRowWidget( const CString& id ) const;

    /**
     *  The height of the space of a row in pixels: RowHeight, or more when the font of the form has taller lines; the three
     *  lines of a row, its buttons and the summary are as high as a line of the font needs
     */
    Int32 GetRowHeight( void ) const;

    /** The text at the right of the first line of a row for an entry */
    static CString FormatEntryInfo( const TEntry& entry );

    /** "0 B", "512 B", "1.5 KB", "12.4 MB", "1.20 GB": 1024 based, one decimal for KB and MB, two for GB and TB */
    static CString FormatSize( UInt64 bytes );

    /** The size per second: "3.1 MB/s" */
    static CString FormatRate( UInt64 bytesPerSecond );

    /** "5 s", "2 min 10 s", "1 h 05 min" */
    static CString FormatDuration( UInt64 seconds );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** The destruction of the model: the form lets go of it (the standard events reach only this) */
    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CTransferQueueForm > TEventCallback;

    /** A row widget with the widgets in it */
    struct SRow
    {
        CWidget* panel;
        CWidget* name;
        CWidget* info;
        CWidget* detail;
        CWidget* progress;
        CWidget* buttons[ CITransferQueueModel::TRANSFERACTION_COUNT ];
        Int32 entryIndex;               /**< the entry the row shows, -1 for a spare row */

        SRow( void );
    };
    typedef GUCEF::vector< SRow > TRowVector;

    void OnModelChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnButtonActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRowsMouseClick( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRowsKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title );
    void ChangeSelection( const CString& id );
    void ShowEmptyState( void );
    void ShowSummary( void );
    void RealizeRows( void );
    bool MakeRow( UInt32 slot );
    void BindRow( UInt32 slot, Int32 entryIndex, Int32 rowWidth );
    Int32 TextWidth( CWidget* widget, const CString& text ) const;

    static const CORE::CEvent& GetActionEvent( ETransferAction action );

    /** Makes the metrics of the rows from the line height of the font when it changed; true when they changed */
    bool UpdateRowMetrics( void );

    CTransferQueueForm( const CTransferQueueForm& src );
    CTransferQueueForm& operator=( const CTransferQueueForm& src );

    private:

    CITransferQueueModel* m_model;
    TEntryVector m_entries;
    TRowVector m_rows;
    CString m_selectedId;
    CString m_summary;
    bool m_performActions;
    bool m_dirty;
    bool m_rowsDirty;
    bool m_refreshed;
    UInt32 m_waitedInMs;
    UInt32 m_refreshCount;
    Int32 m_lastScroll;
    Int32 m_lastViewportWidth;
    Int32 m_lastViewportHeight;
    bool m_frameSubscribed;
    Int32 m_fontLineHeight;         /**< the line height the metrics below were made for, 0 before the first */
    Int32 m_rowHeight;
    Int32 m_lineHeight;
    Int32 m_buttonHeight;
    Int32 m_secondLineTop;
    Int32 m_thirdLineTop;
    Int32 m_thirdLineHeight;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CTRANSFERQUEUEFORM_H ? */
