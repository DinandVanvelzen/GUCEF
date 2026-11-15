/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestAssetFormSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

using namespace GUITEST;

typedef GUI::CITransferQueueModel TModel;

/** A queue that keeps its entries in a vector, writes down what it is asked to do and can refuse an action for every entry */
class CTestTransferModel : public TModel
{
    public:

    TEntryVector entries;
    GUCEF::vector< CORE::CString > performedIds;
    GUCEF::vector< ETransferAction > performedActions;
    Int32 refusedAction;        /**< an action that IsActionAvailable() says no to, -1 for none */
    UInt32 getEntriesCount;

    CTestTransferModel( void ) : TModel(), entries(), performedIds(), performedActions(), refusedAction( -1 ), getEntriesCount( 0 ) {}

    virtual void GetEntries( TEntryVector& result ) const GUCEF_VIRTUAL_OVERRIDE
    {
        result = entries;
        const_cast< CTestTransferModel* >( this )->getEntriesCount++;
    }

    virtual bool PerformAction( const CORE::CString& entryId, ETransferAction action ) GUCEF_VIRTUAL_OVERRIDE
    {
        performedIds.push_back( entryId );
        performedActions.push_back( action );
        return true;
    }

    virtual bool IsActionAvailable( const TEntry& entry, ETransferAction action ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( (Int32) action == refusedAction )
            return false;
        return TModel::IsActionAvailable( entry, action );
    }

    TEntry& Add( const CORE::CString& id, const CORE::CString& name, TModel::ETransferState state, UInt64 total = 0, UInt64 done = 0 )
    {
        TEntry entry;
        entry.id = id;
        entry.name = name;
        entry.state = state;
        entry.totalBytes = total;
        entry.doneBytes = done;
        entries.push_back( entry );
        return entries.back();
    }

    void Remove( const CORE::CString& id )
    {
        for ( size_t i=0; i<entries.size(); ++i )
        {
            if ( entries[ i ].id == id )
            {
                entries.erase( entries.begin() + i );
                return;
            }
        }
    }

    TEntry* Find( const CORE::CString& id )
    {
        for ( size_t i=0; i<entries.size(); ++i )
            if ( entries[ i ].id == id )
                return &entries[ i ];
        return GUCEF_NULL;
    }

    void Changed( void )
    {
        NotifyObservers( EntriesChangedEvent );
    }
};

const UInt64 MB = 1024 * 1024;

/** Entries in every state, with the sizes and texts the tests look at */
void
FillModel( CTestTransferModel& model )
{
    CTestTransferModel::TEntry& running = model.Add( "run", "ubuntu.iso", TModel::TRANSFERSTATE_RUNNING, 400 * MB, 100 * MB );
    running.bytesPerSecond = 2411724;
    running.secondsLeft = 130;
    running.kind = "Download";
    running.detail = "from the mirror";
    model.Add( "que", "queued.bin", TModel::TRANSFERSTATE_QUEUED, 200 * MB, 0 );
    CTestTransferModel::TEntry& paused = model.Add( "pau", "paused.zip", TModel::TRANSFERSTATE_PAUSED, 0, 0 );
    paused.fraction = 0.5f;
    CTestTransferModel::TEntry& finishing = model.Add( "fin", "verify.pak", TModel::TRANSFERSTATE_FINISHING, 1000, 1000 );
    finishing.stateText = "Verifying";
    CTestTransferModel::TEntry& done = model.Add( "don", "done.tar", TModel::TRANSFERSTATE_DONE, 2048, 2048 );
    done.folder = "/tmp/transfers";
    CTestTransferModel::TEntry& failed = model.Add( "fai", "failed.dat", TModel::TRANSFERSTATE_FAILED, 5000, 1200 );
    failed.message = "Disk full\nfree some space";
    model.Add( "can", "cancelled.dat", TModel::TRANSFERSTATE_CANCELLED, 5000, 100 );
}

/** The form in a screen of the context, with the layout done so that rows can be made */
bool
BuildInScreen( GUI::CGUIContext* context, GUI::CTransferQueueForm& form )
{
    GUI::CWidget* screen = MakeScreen( context );
    if ( !form.Build( screen ) )
        return false;
    context->FlushChanges();
    return true;
}

GUI::CWidget*
RowChild( GUI::CTransferQueueForm& form, const CORE::CString& id, const char* name )
{
    GUI::CWidget* row = form.GetRowWidget( id );
    return GUCEF_NULL == row ? GUCEF_NULL : row->GetChildWidget( name );
}

CORE::CString
RowText( GUI::CTransferQueueForm& form, const CORE::CString& id, const char* name )
{
    GUI::CWidget* widget = RowChild( form, id, name );
    return GUCEF_NULL == widget ? CORE::CString( "<none>" ) : widget->GetProperty( "Text" );
}

const char* const ActionButtons[] = { "Pause", "Resume", "Cancel", "Retry", "Remove", "OpenFolder" };

class CTransferScene : public CFormScene
{
    public:

    CTestTransferModel model;
    GUI::CTransferQueueForm form;

    explicit CTransferScene( GUI::CGUIContext* context ) : CFormScene(), model(), form( context )
    {
        GUI::CWidget* screen = MakeScreen( context );
        form.Build( screen );
        context->FlushChanges();
        FillModel( model );
        CTestTransferModel::TEntry& extra = model.Add( "fra", "indexing.db", TModel::TRANSFERSTATE_RUNNING, 0, 0 );
        extra.fraction = 0.8f;
        extra.kind = "Ingest";
        extra.detail = "the index of the catalogue";
        form.SetModel( &model );
        form.SelectEntry( "pau" );
        context->FlushChanges();
    }

    virtual ~CTransferScene()
    {
        form.SetModel( GUCEF_NULL );
    }
};

CFormScene*
BuildTransferQueueScene( GUI::CGUIContext* context )
{
    return new CTransferScene( context );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiTransferQueueFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.TransferQueueForm" );

    GUCEF_TESTFW_TESTCASE( "TransferQueue: The form builds as a window and in the place of a widget, with the rows, the empty text and the summary" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        ASSERT_TRUE( GUCEF_NULL != context );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( form.Build( "Downloads" ) && form.IsVisible() );
        ASSERT_TRUE( "Downloads" == form.GetRootWidget()->GetProperty( "Title" ) );
        const char* names[] = { "Rows", "Empty", "Summary" };
        for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
            ASSERT_TRUE( HasWidget( form, names[ i ] ) );
        ASSERT_TRUE( GUCEF_NULL != GUI::CWidget::ToScrollPanel( form.GetWidget( "Rows" ) ) );
        ASSERT_TRUE( "No transfers" == form.GetWidget( "Empty" )->GetProperty( "Text" ) );
        ASSERT_TRUE( form.GetWidget( "Summary" )->GetProperty( "Text" ).IsNULLOrEmpty() );

        // in the place of a widget of another form
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* placeholder = context->CreateWidget( "Panel", "Place", screen );
        placeholder->SetProperty( "Size", "0,500 0,400" );
        GUI::CTransferQueueForm inPlace;
        ASSERT_TRUE( inPlace.Build( placeholder ) && placeholder == inPlace.GetRootWidget()->GetParentWidget() );
        ASSERT_TRUE( HasWidget( inPlace, "Rows" ) && inPlace.IsEmptyStateShown() );
        ASSERT_TRUE( 0 == inPlace.GetEntryCount() && 0 == inPlace.GetRealizedRowCount() );
        ASSERT_TRUE( GUCEF_NULL == inPlace.GetModel() && inPlace.GetPerformActions() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: The empty text is shown for no model and for a model without entries, and hidden when there are entries" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        ASSERT_TRUE( form.IsEmptyStateShown() );

        CTestTransferModel model;
        form.SetModel( &model );
        ASSERT_TRUE( form.IsEmptyStateShown() && 0 == form.GetRealizedRowCount() );
        ASSERT_TRUE( "0 transfers, 0 running" == form.GetWidget( "Summary" )->GetProperty( "Text" ) );

        model.Add( "a", "first", TModel::TRANSFERSTATE_QUEUED );
        form.Refresh();
        ASSERT_FALSE( form.IsEmptyStateShown() );
        ASSERT_TRUE( 1 == form.GetRealizedRowCount() );

        model.Remove( "a" );
        form.Refresh();
        ASSERT_TRUE( form.IsEmptyStateShown() && 0 == form.GetRealizedRowCount() );

        form.SetModel( GUCEF_NULL );
        ASSERT_TRUE( form.IsEmptyStateShown() );
        ASSERT_TRUE( form.GetWidget( "Summary" )->GetProperty( "Text" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: Every entry has a row with its name, its state text, and a progress from the bytes or from the fraction" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        CTestTransferModel model;
        FillModel( model );
        form.SetModel( &model );
        context->FlushChanges();
        ASSERT_TRUE( 7 == form.GetEntryCount() && 7 == form.GetRealizedRowCount() );
        ASSERT_TRUE( 0 == form.FindEntryIndex( "run" ) && 6 == form.FindEntryIndex( "can" ) && -1 == form.FindEntryIndex( "none" ) );
        ASSERT_TRUE( "ubuntu.iso" == form.GetEntry( 0 ).name && form.GetEntry( 99 ).id.IsNULLOrEmpty() );

        ASSERT_TRUE( "ubuntu.iso" == RowText( form, "run", "Name" ) && "paused.zip" == RowText( form, "pau", "Name" ) );
        ASSERT_TRUE( RowText( form, "run", "Info" ).HasSubstr( "Running" ) == 0 );
        ASSERT_TRUE( RowText( form, "que", "Info" ).HasSubstr( "Queued" ) == 0 );
        ASSERT_TRUE( RowText( form, "fin", "Info" ).HasSubstr( "Verifying" ) == 0 && RowText( form, "fin", "Info" ).HasSubstr( "Finishing" ) < 0 );
        ASSERT_TRUE( RowText( form, "fai", "Info" ).HasSubstr( "Failed: Disk full free some space" ) == 0 );
        ASSERT_TRUE( "Download: from the mirror" == RowText( form, "run", "Detail" ) );

        // from the bytes: 100 of 400, from the fraction: a half; done is all, nothing known is zero
        ASSERT_TRUE( 25.0f == RowChild( form, "run", "Progress" )->GetFloatProperty( "Value" ) );
        ASSERT_TRUE( 50.0f == RowChild( form, "pau", "Progress" )->GetFloatProperty( "Value" ) );
        ASSERT_TRUE( 100.0f == RowChild( form, "fin", "Progress" )->GetFloatProperty( "Value" ) );
        ASSERT_TRUE( 100.0f == RowChild( form, "don", "Progress" )->GetFloatProperty( "Value" ) );
        ASSERT_TRUE( 0.0f == RowChild( form, "que", "Progress" )->GetFloatProperty( "Value" ) );
        ASSERT_TRUE( 24.0f == (Int32) RowChild( form, "fai", "Progress" )->GetFloatProperty( "Value" ) );

        CTestTransferModel::TEntry& unknown = model.Add( "unk", "unknown", TModel::TRANSFERSTATE_RUNNING, 0, 4096 );
        unknown.fraction = -1.0f;
        form.Refresh();
        ASSERT_TRUE( 0.0f == RowChild( form, "unk", "Progress" )->GetFloatProperty( "Value" ) );
        ASSERT_TRUE( RowText( form, "unk", "Info" ).HasSubstr( "4.0 KB" ) >= 0 );

        // the colours of the rows follow the state
        const GUI::CColour neutral = form.GetRowWidget( "run" )->GetColourProperty( "BackgroundColour" );
        ASSERT_TRUE( neutral == form.GetRowWidget( "que" )->GetColourProperty( "BackgroundColour" ) );
        ASSERT_TRUE( neutral != form.GetRowWidget( "fai" )->GetColourProperty( "BackgroundColour" ) );
        ASSERT_TRUE( neutral != form.GetRowWidget( "don" )->GetColourProperty( "BackgroundColour" ) );
        ASSERT_TRUE( neutral != form.GetRowWidget( "can" )->GetColourProperty( "BackgroundColour" ) );
        ASSERT_TRUE( form.GetRowWidget( "fai" )->GetColourProperty( "BackgroundColour" ).r > form.GetRowWidget( "fai" )->GetColourProperty( "BackgroundColour" ).g );
        ASSERT_TRUE( form.GetRowWidget( "don" )->GetColourProperty( "BackgroundColour" ).g > form.GetRowWidget( "don" )->GetColourProperty( "BackgroundColour" ).r );
        form.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: The sizes, the rates and the durations are written as people read them" )
    try
    {
        typedef GUI::CTransferQueueForm TForm;
        ASSERT_TRUE( "0 B" == TForm::FormatSize( 0 ) && "512 B" == TForm::FormatSize( 512 ) && "1023 B" == TForm::FormatSize( 1023 ) );
        ASSERT_TRUE( "1.0 KB" == TForm::FormatSize( 1024 ) && "1.5 KB" == TForm::FormatSize( 1536 ) );
        ASSERT_TRUE( "12.4 MB" == TForm::FormatSize( 13002342 ) && "27.0 MB" == TForm::FormatSize( 27 * MB ) );
        ASSERT_TRUE( "1.20 GB" == TForm::FormatSize( 1288490189ULL ) && "5.00 TB" == TForm::FormatSize( 5ULL * 1024 * 1024 * 1024 * 1024 ) );
        ASSERT_TRUE( "1.00 GB" == TForm::FormatSize( 1024 * MB ) );
        ASSERT_TRUE( "1.0 MB" == TForm::FormatSize( 1048575 ) );       // rounds up to the next unit
        ASSERT_TRUE( "3.1 MB/s" == TForm::FormatRate( 3250586 ) && "0 B/s" == TForm::FormatRate( 0 ) );

        ASSERT_TRUE( "0 s" == TForm::FormatDuration( 0 ) && "5 s" == TForm::FormatDuration( 5 ) && "59 s" == TForm::FormatDuration( 59 ) );
        ASSERT_TRUE( "1 min 00 s" == TForm::FormatDuration( 60 ) && "2 min 10 s" == TForm::FormatDuration( 130 ) && "2 min 05 s" == TForm::FormatDuration( 125 ) );
        ASSERT_TRUE( "59 min 59 s" == TForm::FormatDuration( 3599 ) && "1 h 00 min" == TForm::FormatDuration( 3600 ) && "1 h 05 min" == TForm::FormatDuration( 3900 ) );
        ASSERT_TRUE( "25 h 00 min" == TForm::FormatDuration( 90000 ) );

        // the text of an entry
        TModel::TEntry entry;
        entry.state = TModel::TRANSFERSTATE_RUNNING;
        entry.totalBytes = 27 * MB;
        entry.doneBytes = 13002342;
        entry.bytesPerSecond = 3250586;
        entry.secondsLeft = 5;
        ASSERT_TRUE( "Running, 45%, 12.4 MB of 27.0 MB, 3.1 MB/s, 5 s left" == TForm::FormatEntryInfo( entry ) );
        entry.secondsLeft = 130;
        entry.stateText = "Downloading";
        ASSERT_TRUE( "Downloading, 45%, 12.4 MB of 27.0 MB, 3.1 MB/s, 2 min 10 s left" == TForm::FormatEntryInfo( entry ) );
        entry.state = TModel::TRANSFERSTATE_FAILED;
        entry.stateText.Clear();
        entry.message = "Connection reset\nby the server";
        ASSERT_TRUE( "Failed: Connection reset by the server" == TForm::FormatEntryInfo( entry ) );
        entry.state = TModel::TRANSFERSTATE_DONE;
        ASSERT_TRUE( "Done, 27.0 MB" == TForm::FormatEntryInfo( entry ) );
        entry.state = TModel::TRANSFERSTATE_QUEUED;
        entry.doneBytes = 0;
        ASSERT_TRUE( "Queued, 27.0 MB" == TForm::FormatEntryInfo( entry ) );
        entry.state = TModel::TRANSFERSTATE_PAUSED;
        entry.totalBytes = 0;
        entry.fraction = 0.5f;
        ASSERT_TRUE( "Paused, 50%" == TForm::FormatEntryInfo( entry ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: The summary counts the entries, the running ones, the bytes of the entries with a total and the rate of the running ones" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        ASSERT_TRUE( form.GetWidget( "Summary" )->GetProperty( "Text" ).IsNULLOrEmpty() );     // nothing before the first refresh

        CTestTransferModel model;
        CTestTransferModel::TEntry& a = model.Add( "a", "a", TModel::TRANSFERSTATE_RUNNING, 400 * MB, 100 * MB );
        a.bytesPerSecond = 2411724;
        model.Add( "b", "b", TModel::TRANSFERSTATE_QUEUED, 200 * MB, 0 );
        model.Add( "c", "c", TModel::TRANSFERSTATE_RUNNING, 0, 0 );
        form.SetModel( &model );
        ASSERT_TRUE( "3 transfers, 2 running, 100.0 MB of 600.0 MB, 2.3 MB/s" == form.GetWidget( "Summary" )->GetProperty( "Text" ) );

        model.entries.resize( 1 );
        model.entries[ 0 ].state = TModel::TRANSFERSTATE_DONE;
        model.entries[ 0 ].bytesPerSecond = 0;
        form.Refresh();
        ASSERT_TRUE( "1 transfer, 0 running, 400.0 MB of 400.0 MB" == form.GetWidget( "Summary" )->GetProperty( "Text" ) );
        form.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: The buttons of a row are those the state offers: queued, running, paused, finishing, done, failed and cancelled" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        CTestTransferModel model;
        FillModel( model );
        form.SetModel( &model );
        context->FlushChanges();

        // pause, resume, cancel, retry, remove, open folder
        const struct { const char* id; bool shown[ 6 ]; } expected[] = {
            { "que", { true , false, true , false, false, false } },
            { "run", { true , false, true , false, false, false } },
            { "pau", { false, true , true , false, false, false } },
            { "fin", { false, false, true , false, false, false } },
            { "don", { false, false, false, false, true , true  } },
            { "fai", { false, false, false, true , true , false } },
            { "can", { false, false, false, true , true , false } } };
        for ( UInt32 e=0; e<sizeof( expected ) / sizeof( expected[ 0 ] ); ++e )
        {
            for ( UInt32 a=0; a<6; ++a )
            {
                const bool shown = form.IsActionShown( expected[ e ].id, (TModel::ETransferAction) a );
                ASSERT_TRUE( shown == expected[ e ].shown[ a ] );
                // the widget agrees with the query
                ASSERT_TRUE( shown == RowChild( form, expected[ e ].id, ActionButtons[ a ] )->GetBoolProperty( "Visible" ) );
            }
        }
        ASSERT_FALSE( form.IsActionShown( "none", TModel::TRANSFERACTION_PAUSE ) );
        ASSERT_TRUE( "Open folder" == RowChild( form, "don", "OpenFolder" )->GetProperty( "Text" ) );

        // a state change moves the buttons along with it
        model.Find( "run" )->state = TModel::TRANSFERSTATE_PAUSED;
        form.Refresh();
        ASSERT_TRUE( form.IsActionShown( "run", TModel::TRANSFERACTION_RESUME ) && !form.IsActionShown( "run", TModel::TRANSFERACTION_PAUSE ) );
        form.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: A model that overrides IsActionAvailable decides which buttons a row has" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        CTestTransferModel model;
        FillModel( model );
        model.refusedAction = TModel::TRANSFERACTION_PAUSE;     // a job that cannot be paused
        form.SetModel( &model );
        context->FlushChanges();
        ASSERT_FALSE( form.IsActionShown( "run", TModel::TRANSFERACTION_PAUSE ) );
        ASSERT_TRUE( form.IsActionShown( "run", TModel::TRANSFERACTION_CANCEL ) );
        ASSERT_FALSE( RowChild( form, "que", "Pause" )->GetBoolProperty( "Visible" ) );

        // a request for what is not offered is not carried out
        CEventRecorder paused( &form, GUI::CTransferQueueForm::PauseRequestedEvent );
        ASSERT_FALSE( form.RequestAction( "run", TModel::TRANSFERACTION_PAUSE ) );
        ASSERT_TRUE( 0 == paused.count && model.performedIds.empty() );
        // and not for an entry that is not there, or for what the state does not allow
        ASSERT_FALSE( form.RequestAction( "none", TModel::TRANSFERACTION_CANCEL ) );
        ASSERT_FALSE( form.RequestAction( "don", TModel::TRANSFERACTION_RESUME ) );
        ASSERT_TRUE( model.performedIds.empty() );

        model.refusedAction = -1;
        form.Refresh();
        ASSERT_TRUE( form.IsActionShown( "run", TModel::TRANSFERACTION_PAUSE ) );
        form.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: Pressing a button notifies the event of the action with the id and has the model perform it; without SetPerformActions only the event" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        CTestTransferModel model;
        FillModel( model );
        form.SetModel( &model );
        context->FlushChanges();

        CEventRecorder pause( &form, GUI::CTransferQueueForm::PauseRequestedEvent );
        CEventRecorder resume( &form, GUI::CTransferQueueForm::ResumeRequestedEvent );
        CEventRecorder cancel( &form, GUI::CTransferQueueForm::CancelRequestedEvent );
        CEventRecorder retry( &form, GUI::CTransferQueueForm::RetryRequestedEvent );
        CEventRecorder remove( &form, GUI::CTransferQueueForm::RemoveRequestedEvent );
        CEventRecorder openFolder( &form, GUI::CTransferQueueForm::OpenFolderRequestedEvent );
        CEventRecorder selection( &form, GUI::CTransferQueueForm::SelectionChangedEvent );

        ClickWidget( context, RowChild( form, "run", "Pause" ) );
        ASSERT_TRUE( 1 == pause.count && "run" == pause.Last< CORE::TCloneableString >()->GetData() );
        ASSERT_TRUE( 1 == model.performedIds.size() && "run" == model.performedIds[ 0 ] && TModel::TRANSFERACTION_PAUSE == model.performedActions[ 0 ] );
        ClickWidget( context, RowChild( form, "pau", "Resume" ) );
        ASSERT_TRUE( 1 == resume.count && "pau" == resume.Last< CORE::TCloneableString >()->GetData() && TModel::TRANSFERACTION_RESUME == model.performedActions[ 1 ] );
        ClickWidget( context, RowChild( form, "que", "Cancel" ) );
        ASSERT_TRUE( 1 == cancel.count && "que" == cancel.Last< CORE::TCloneableString >()->GetData() && TModel::TRANSFERACTION_CANCEL == model.performedActions[ 2 ] );
        ClickWidget( context, RowChild( form, "fai", "Retry" ) );
        ASSERT_TRUE( 1 == retry.count && "fai" == retry.Last< CORE::TCloneableString >()->GetData() && TModel::TRANSFERACTION_RETRY == model.performedActions[ 3 ] );
        ClickWidget( context, RowChild( form, "can", "Remove" ) );
        ASSERT_TRUE( 1 == remove.count && "can" == remove.Last< CORE::TCloneableString >()->GetData() && TModel::TRANSFERACTION_REMOVE == model.performedActions[ 4 ] );
        ClickWidget( context, RowChild( form, "don", "OpenFolder" ) );
        ASSERT_TRUE( 1 == openFolder.count && "don" == openFolder.Last< CORE::TCloneableString >()->GetData() && TModel::TRANSFERACTION_OPEN_FOLDER == model.performedActions[ 5 ] );
        ASSERT_TRUE( 6 == model.performedIds.size() );
        ASSERT_TRUE( 0 == selection.count );        // pressing a button does not select the row

        // the application that does the work itself only gets the events
        form.SetPerformActions( false );
        ASSERT_FALSE( form.GetPerformActions() );
        ClickWidget( context, RowChild( form, "run", "Cancel" ) );
        ASSERT_TRUE( 2 == cancel.count && "run" == cancel.Last< CORE::TCloneableString >()->GetData() );
        ASSERT_TRUE( 6 == model.performedIds.size() );
        ASSERT_TRUE( form.RequestAction( "fai", TModel::TRANSFERACTION_REMOVE ) );
        ASSERT_TRUE( 2 == remove.count && 6 == model.performedIds.size() );

        // the programmatic request does what the button does
        form.SetPerformActions( true );
        ASSERT_TRUE( form.RequestAction( "fai", TModel::TRANSFERACTION_RETRY ) );
        ASSERT_TRUE( 2 == retry.count && 7 == model.performedIds.size() && "fai" == model.performedIds[ 6 ] );
        form.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: Changes of the model are shown at most once per refresh interval of frame time, Refresh shows them at once" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        CTestTransferModel model;
        model.Add( "a", "a", TModel::TRANSFERSTATE_RUNNING, 1000, 100 );
        model.Add( "b", "b", TModel::TRANSFERSTATE_RUNNING, 1000, 200 );
        model.Add( "c", "c", TModel::TRANSFERSTATE_QUEUED, 1000, 0 );

        // the first refresh after SetModel is at once
        const UInt32 start = form.GetRefreshCount();
        form.SetModel( &model );
        ASSERT_TRUE( start + 1 == form.GetRefreshCount() && 3 == form.GetEntryCount() );
        UInt32 count = form.GetRefreshCount();

        // nothing changed: no refresh however long it takes
        form.Update( 1000 );
        ASSERT_TRUE( count == form.GetRefreshCount() );

        // an entry comes: the model said so, the form waits
        model.Add( "d", "d", TModel::TRANSFERSTATE_QUEUED );
        model.Changed();
        ASSERT_TRUE( count == form.GetRefreshCount() && 3 == form.GetEntryCount() );
        form.Update( 100 );
        form.Update( 100 );
        ASSERT_TRUE( count == form.GetRefreshCount() && 3 == form.GetEntryCount() );
        form.Update( 60 );
        ASSERT_TRUE( count + 1 == form.GetRefreshCount() && 4 == form.GetEntryCount() );
        count = form.GetRefreshCount();

        // progress reports for every byte: one refresh for the interval
        for ( UInt32 i=0; i<50; ++i )
        {
            model.Find( "a" )->doneBytes += 1;
            model.Changed();
        }
        form.Update( 100 );
        form.Update( 100 );
        ASSERT_TRUE( count == form.GetRefreshCount() );
        form.Update( 50 );
        ASSERT_TRUE( count + 1 == form.GetRefreshCount() );
        ASSERT_TRUE( 150 == form.GetEntry( 0 ).doneBytes );
        ASSERT_TRUE( "Running, 15%, 150 B of 1000 B" == RowText( form, "a", "Info" ) );
        count = form.GetRefreshCount();
        form.Update( 1000 );
        ASSERT_TRUE( count == form.GetRefreshCount() );

        // the wait starts again after a refresh
        model.Find( "b" )->doneBytes = 900;
        model.Changed();
        form.Update( 249 );
        ASSERT_TRUE( count == form.GetRefreshCount() );
        form.Update( 1 );
        ASSERT_TRUE( count + 1 == form.GetRefreshCount() && 900 == form.GetEntry( 1 ).doneBytes );
        count = form.GetRefreshCount();

        // Refresh() does not wait
        model.Find( "b" )->doneBytes = 950;
        model.Changed();
        form.Refresh();
        ASSERT_TRUE( count + 1 == form.GetRefreshCount() && 950 == form.GetEntry( 1 ).doneBytes );
        ASSERT_TRUE( "Running, 95%, 950 B of 1000 B" == RowText( form, "b", "Info" ) );
        form.Update( 1000 );
        ASSERT_TRUE( count + 1 == form.GetRefreshCount() );        // the refresh took the change along

        // the frame time of the context does the same
        count = form.GetRefreshCount();
        model.Find( "c" )->state = TModel::TRANSFERSTATE_RUNNING;
        model.Changed();
        context->RenderFrame( 100 );
        ASSERT_TRUE( count == form.GetRefreshCount() );
        context->RenderFrame( 200 );
        ASSERT_TRUE( count + 1 == form.GetRefreshCount() && TModel::TRANSFERSTATE_RUNNING == form.GetEntry( 2 ).state );
        form.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: A click on a row selects its entry and notifies SelectionChangedEvent, and the selection is cleared when the entry is gone" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        CTestTransferModel model;
        FillModel( model );
        form.SetModel( &model );
        context->FlushChanges();
        CEventRecorder selection( &form, GUI::CTransferQueueForm::SelectionChangedEvent );
        ASSERT_TRUE( form.GetSelectedEntryId().IsNULLOrEmpty() );

        // a click in the empty part of the first line of the row
        GUI::CWidget* row = form.GetRowWidget( "pau" );
        const GUI::CRect rect = row->GetRect();
        ClickAt( context, rect.left + 3, rect.top + 30 );
        ASSERT_TRUE( "pau" == form.GetSelectedEntryId() );
        ASSERT_TRUE( 1 == selection.count && "pau" == selection.Last< CORE::TCloneableString >()->GetData() );
        context->FlushChanges();
        const GUI::CColour selectedColour = form.GetRowWidget( "pau" )->GetColourProperty( "BorderColour" );
        ASSERT_TRUE( selectedColour != form.GetRowWidget( "que" )->GetColourProperty( "BorderColour" ) );

        // the same entry again is no change
        context->RenderFrame( 500 );
        row = form.GetRowWidget( "pau" );
        ClickAt( context, row->GetRect().left + 3, row->GetRect().top + 30 );
        ASSERT_TRUE( 1 == selection.count );

        // by program
        ASSERT_TRUE( form.SelectEntry( "don" ) && "don" == form.GetSelectedEntryId() );
        ASSERT_TRUE( 2 == selection.count && "don" == selection.Last< CORE::TCloneableString >()->GetData() );
        ASSERT_FALSE( form.SelectEntry( "nonsense" ) );
        ASSERT_TRUE( "don" == form.GetSelectedEntryId() && 2 == selection.count );
        ASSERT_TRUE( form.GetRowWidget( "don" )->GetColourProperty( "BorderColour" ) == selectedColour );
        ASSERT_TRUE( form.GetRowWidget( "pau" )->GetColourProperty( "BorderColour" ) != selectedColour );

        // the entry goes away: the selection with it
        model.Remove( "don" );
        model.Changed();
        form.Refresh();
        ASSERT_TRUE( form.GetSelectedEntryId().IsNULLOrEmpty() );
        ASSERT_TRUE( 3 == selection.count && selection.Last< CORE::TCloneableString >()->GetData().IsNULLOrEmpty() );

        // an entry that stays keeps the selection through refreshes
        ASSERT_TRUE( form.SelectEntry( "fai" ) );
        form.Refresh();
        form.Refresh();
        ASSERT_TRUE( "fai" == form.GetSelectedEntryId() && 4 == selection.count );

        // clearing it by program
        ASSERT_TRUE( form.SelectEntry( CORE::CString::Empty ) && form.GetSelectedEntryId().IsNULLOrEmpty() && 5 == selection.count );
        form.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: Delete removes the selected entry when it can be removed" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        CTestTransferModel model;
        FillModel( model );
        form.SetModel( &model );
        context->FlushChanges();
        CEventRecorder remove( &form, GUI::CTransferQueueForm::RemoveRequestedEvent );

        // a click gives the list the keyboard
        GUI::CWidget* row = form.GetRowWidget( "run" );
        ClickAt( context, row->GetRect().left + 3, row->GetRect().top + 30 );
        ASSERT_TRUE( "run" == form.GetSelectedEntryId() );
        PressKey( context, INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( 0 == remove.count && model.performedIds.empty() );        // a running transfer is not removed

        context->RenderFrame( 500 );
        row = form.GetRowWidget( "can" );
        ClickAt( context, row->GetRect().left + 3, row->GetRect().top + 30 );
        ASSERT_TRUE( "can" == form.GetSelectedEntryId() );
        PressKey( context, INPUT::KEYCODE_DELETE );
        ASSERT_TRUE( 1 == remove.count && "can" == remove.Last< CORE::TCloneableString >()->GetData() );
        ASSERT_TRUE( 1 == model.performedIds.size() && TModel::TRANSFERACTION_REMOVE == model.performedActions[ 0 ] );
        form.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: The rows are pooled to what is in view: 300 entries refreshed over and over make no new widgets, and scrolling rebinds the rows" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        CTestTransferModel model;
        for ( UInt32 i=0; i<300; ++i )
        {
            CTestTransferModel::TEntry& entry = model.Add( "t" + CORE::UInt32ToString( i ), "transfer " + CORE::UInt32ToString( i ), TModel::TRANSFERSTATE_RUNNING, 1000, i );
            entry.bytesPerSecond = 100;
        }
        form.SetModel( &model );
        context->FlushChanges();
        GUI::CScrollPanel* rows = GUI::CWidget::ToScrollPanel( form.GetWidget( "Rows" ) );
        const UInt32 realized = form.GetRealizedRowCount();
        ASSERT_TRUE( 300 == form.GetEntryCount() && realized > 4 && realized < 30 );

        GUI::CWidget::TWidgetSet children;
        rows->GetChildWidgetSet( children );
        const size_t widgetCount = children.size();
        ASSERT_TRUE( widgetCount <= realized + 1 );

        for ( UInt32 round=0; round<20; ++round )
        {
            for ( UInt32 i=0; i<300; ++i )
                model.entries[ i ].doneBytes = ( i + round * 7 ) % 1000;
            model.Changed();
            form.Refresh();
            context->RenderFrame( 16 );
        }
        children.clear();
        rows->GetChildWidgetSet( children );
        ASSERT_TRUE( children.size() == widgetCount && realized == form.GetRealizedRowCount() );
        ASSERT_TRUE( GUCEF_NULL != form.GetRowWidget( "t0" ) && GUCEF_NULL == form.GetRowWidget( "t250" ) );

        // the content is as high as all the rows, and scrolling brings the end of the list into the rows
        context->FlushChanges();
        ASSERT_TRUE( rows->IsScrollable() && rows->GetContentHeight() >= 300 * (Int32) GUI::CTransferQueueForm::RowHeight - 2 );
        rows->SetScrollPosition( rows->GetMaxScrollPosition() );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( GUCEF_NULL != form.GetRowWidget( "t299" ) && GUCEF_NULL == form.GetRowWidget( "t0" ) );
        ASSERT_TRUE( "transfer 299" == RowText( form, "t299", "Name" ) );
        children.clear();
        rows->GetChildWidgetSet( children );
        ASSERT_TRUE( children.size() == widgetCount && form.GetRealizedRowCount() > 0 && form.GetRealizedRowCount() <= realized );

        // an entry out of view is still selected, and answers the queries from the model
        ASSERT_TRUE( form.SelectEntry( "t10" ) );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( GUCEF_NULL != form.GetRowWidget( "t10" ) );     // scrolled into view
        ASSERT_TRUE( form.IsActionShown( "t200", TModel::TRANSFERACTION_PAUSE ) && !form.IsActionShown( "t200", TModel::TRANSFERACTION_REMOVE ) );
        form.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: The model is replaced by another or taken away, and a model that is destroyed first is let go of" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        CTestTransferModel first;
        first.Add( "a", "from the first", TModel::TRANSFERSTATE_RUNNING, 100, 10 );
        CTestTransferModel second;
        second.Add( "x", "from the second", TModel::TRANSFERSTATE_QUEUED );
        second.Add( "y", "also second", TModel::TRANSFERSTATE_QUEUED );

        form.SetModel( &first );
        ASSERT_TRUE( &first == form.GetModel() && 1 == form.GetEntryCount() );
        form.SetModel( &second );
        ASSERT_TRUE( &second == form.GetModel() && 2 == form.GetEntryCount() && 2 == form.GetRealizedRowCount() );
        ASSERT_TRUE( GUCEF_NULL == form.GetRowWidget( "a" ) && "from the second" == RowText( form, "x", "Name" ) );

        // the old model is not listened to any more
        UInt32 count = form.GetRefreshCount();
        first.Changed();
        form.Update( 1000 );
        ASSERT_TRUE( count == form.GetRefreshCount() );
        second.Changed();
        form.Update( 1000 );
        ASSERT_TRUE( count + 1 == form.GetRefreshCount() );

        // the selection of an entry the other model does not have is gone
        form.SelectEntry( "x" );
        form.SetModel( &first );
        ASSERT_TRUE( form.GetSelectedEntryId().IsNULLOrEmpty() );

        // none
        form.SetModel( GUCEF_NULL );
        ASSERT_TRUE( GUCEF_NULL == form.GetModel() && 0 == form.GetEntryCount() && form.IsEmptyStateShown() && 0 == form.GetRealizedRowCount() );
        count = form.GetRefreshCount();
        second.Changed();
        form.Update( 1000 );
        ASSERT_TRUE( count == form.GetRefreshCount() );

        // a model that goes before the form
        {
            CTestTransferModel temporary;
            temporary.Add( "t", "temporary", TModel::TRANSFERSTATE_RUNNING );
            form.SetModel( &temporary );
            ASSERT_TRUE( 1 == form.GetEntryCount() );
        }
        ASSERT_TRUE( GUCEF_NULL == form.GetModel() );
        form.Update( 1000 );
        ASSERT_TRUE( 0 == form.GetEntryCount() && form.IsEmptyStateShown() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: The rows lie inside the list, the name stays left of the state text and the buttons are right of the progress bar and do not overlap" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        CTestTransferModel model;
        FillModel( model );
        model.Find( "run" )->name = "a very long name of a transfer that does not fit in the half of a row at all, whatever the font is";
        form.SetModel( &model );
        context->FlushChanges();
        context->RenderFrame( 16 );

        GUI::CWidget* rowsWidget = form.GetWidget( "Rows" );
        const GUI::CRect rowsRect = rowsWidget->GetRect();
        GUI::CRect previousRow;
        for ( UInt32 e=0; e<form.GetEntryCount(); ++e )
        {
            const CORE::CString id = form.GetEntry( e ).id;
            GUI::CWidget* row = form.GetRowWidget( id );
            ASSERT_TRUE( GUCEF_NULL != row );
            const GUI::CRect rowRect = row->GetRect();
            // inside the list and under the row above
            ASSERT_TRUE( rowRect.left >= rowsRect.left && rowRect.Right() <= rowsRect.Right() );
            ASSERT_TRUE( rowRect.top >= rowsRect.top && rowRect.Bottom() <= rowsRect.Bottom() );
            if ( e > 0 )
                ASSERT_TRUE( rowRect.top >= previousRow.Bottom() );
            previousRow = rowRect;

            const GUI::CRect name = RowChild( form, id, "Name" )->GetRect();
            const GUI::CRect info = RowChild( form, id, "Info" )->GetRect();
            const GUI::CRect progress = RowChild( form, id, "Progress" )->GetRect();
            ASSERT_TRUE( name.left >= rowRect.left && name.Right() <= info.left && info.Right() <= rowRect.Right() );
            ASSERT_TRUE( name.Bottom() <= progress.top && info.Bottom() <= progress.top );
            ASSERT_TRUE( progress.width > 50 && progress.Right() <= rowRect.Right() );

            // the buttons: right of the progress bar, in order, without overlapping, the last at the right edge
            Int32 previousRight = progress.Right();
            Int32 shown = 0;
            for ( UInt32 a=0; a<6; ++a )
            {
                GUI::CWidget* button = RowChild( form, id, ActionButtons[ a ] );
                if ( !button->GetBoolProperty( "Visible" ) )
                    continue;
                const GUI::CRect buttonRect = button->GetRect();
                ASSERT_TRUE( buttonRect.left >= previousRight && buttonRect.width >= 40 );
                ASSERT_TRUE( buttonRect.top >= rowRect.top && buttonRect.Bottom() <= rowRect.Bottom() );
                previousRight = buttonRect.Right();
                ++shown;
            }
            ASSERT_TRUE( shown > 0 && previousRight <= rowRect.Right() && previousRight >= rowRect.Right() - 12 );
        }

        // the name that does not fit is cut with dots and the row has it whole in the tooltip
        const CORE::CString longName = RowText( form, "run", "Name" );
        ASSERT_TRUE( longName.Length() < model.Find( "run" )->name.Length() && longName.HasSubstr( "..." ) >= 0 );
        ASSERT_TRUE( form.GetRowWidget( "run" )->GetProperty( "Tooltip" ).HasSubstr( "whatever the font is" ) >= 0 );
        form.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: A layout of the form that was saved holds no rows, and loads into a form that makes its own" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CTransferQueueForm first( context );
        ASSERT_TRUE( BuildInScreen( context, first ) );
        CTestTransferModel model;
        FillModel( model );
        first.SetModel( &model );
        context->FlushChanges();
        CORE::CDataNode layout;
        ASSERT_TRUE( first.SaveLayout( layout ) );

        GUI::CTransferQueueForm second( context );
        ASSERT_TRUE( second.LoadLayout( layout ) && second.Show() );
        context->FlushChanges();
        second.SetModel( &model );
        context->FlushChanges();
        ASSERT_TRUE( 7 == second.GetRealizedRowCount() && 7 + 1 == second.GetWidget( "Rows" )->GetChildren().size() );        // the rows and the spacer
        first.SetModel( GUCEF_NULL );
        second.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TransferQueue: The rows are laid out from the line of the font, so a font of 22 px lines fits a row; the summary is one line high" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateLayoutLintContext();
        GUI::CGUIContext* context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        context->SetLogicalResolution( 608, 234 );
        GUI::CTransferQueueForm form( context );
        ASSERT_TRUE( BuildInScreen( context, form ) );
        CTestTransferModel model;
        CTestTransferModel::TEntry& entry = model.Add( "one", "Medieval Weapons", TModel::TRANSFERSTATE_DONE, 100, 100 );
        entry.kind = "Ingest";
        entry.detail = "into the primary store";
        form.SetModel( &model );
        for ( UInt32 f=0; f<3; ++f )
            context->RenderFrame( 16 );

        // three lines of 22 px, the buttons and the margins: more than the 56 px of a small font
        const Int32 line = GUITEST::LayoutLintLineHeight;
        ASSERT_TRUE( form.GetRowHeight() >= 3 * line + 8 && (Int32) GUI::CTransferQueueForm::RowHeight <= form.GetRowHeight() );
        GUI::CWidget* row = form.GetRowWidget( "one" );
        GUI::CWidget* detail = RowChild( form, "one", "Detail" );
        ASSERT_TRUE( GUCEF_NULL != row && GUCEF_NULL != detail );
        if ( GUCEF_NULL != row && GUCEF_NULL != detail )
            ASSERT_TRUE( detail->GetRect().height >= line && detail->GetRect().Bottom() <= row->GetRect().Bottom() && detail->GetRect().top >= row->GetRect().top );
        GUI::CWidget* summary = form.GetWidget( "Summary" );
        ASSERT_TRUE( summary->GetRect().height >= line && summary->GetRect().Bottom() <= 234 );

        // the lint finds no cut text and nothing outside its row
        GUI::TLayoutLintFindingVector findings;
        GUI::CLayoutLint lint;
        lint.Run( *context, GUI::CLayoutLintOptions(), findings );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "TransferQueue at 608x234: " << GUI::CLayoutLint::Report( findings, 20 ) );
        UInt32 counts[ GUI::LAYOUTLINTRULE_COUNT ] = { 0 };
        GUI::CLayoutLint::CountByRule( findings, GUI::LAYOUTLINTSEVERITY_INFO, counts );
        ASSERT_TRUE( 0 == counts[ GUI::LAYOUTLINTRULE_TEXTCLIPPED ] && 0 == counts[ GUI::LAYOUTLINTRULE_OUTSIDEPARENT ] && 0 == counts[ GUI::LAYOUTLINTRULE_BELOWMINIMUM ] );

        // with the small font of the other tests the row keeps its old height
        context->RegisterFont( "Test", MakeTestFont( 6, 9 ) );
        context->RenderFrame( 16 );
        ASSERT_TRUE( (Int32) GUI::CTransferQueueForm::RowHeight == form.GetRowHeight() );
        form.SetModel( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RunFormRenderTests( "transfer queue", &BuildTransferQueueScene );
    RunFormLintBaseline( "TransferQueue", &BuildTransferQueueScene );
}

/*-------------------------------------------------------------------------*/
