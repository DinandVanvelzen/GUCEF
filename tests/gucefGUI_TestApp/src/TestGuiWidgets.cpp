/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGUI_TestCommon.h"

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

/** Counts events of one kind and remembers the data of the last */
class CCounter : public CORE::CObserver
{
    public:

    CORE::CEvent eventId;
    UInt32 count;
    CORE::CString lastValue;
    Int32 lastIndex;
    CORE::CString lastItem;

    CCounter( CORE::CNotifier* notifier, const CORE::CEvent& id )
        : CORE::CObserver(), eventId( id ), count( 0 ), lastValue(), lastIndex( -1 ), lastItem()
    {
        SubscribeTo( notifier, id );
    }

    virtual ~CCounter() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( id != eventId )
            return;
        GUI::CRoutedEventData* routed = GUI::CRoutedEventData::FromCloneable( eventData );
        if ( GUCEF_NULL != routed && routed->source != notifier )
            return;     // only the target phase
        ++count;
        GUI::CValueChangedEventData* changed = CORE::CICloneable::CloneableAs< GUI::CValueChangedEventData >( eventData );
        if ( GUCEF_NULL != changed )
            lastValue = changed->newValue;
        GUI::CItemEventData* item = CORE::CICloneable::CloneableAs< GUI::CItemEventData >( eventData );
        if ( GUCEF_NULL != item )
        {
            lastIndex = item->index;
            lastItem = item->item;
        }
    }
};

/** A root panel over the whole picture, shown */
GUI::CWidget*
MakeScreen( GUI::CGUIContext* context )
{
    GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
    root->SetProperty( "Size", "1,0 1,0" );
    context->ShowRoot( root, false );
    return root;
}

void
Click( GUI::CGUIContext* context, Int32 x, Int32 y, GUI::TMouseButton button = GUI::MOUSEBUTTON_LEFT )
{
    context->InjectMouseMove( x, y );
    context->InjectMouseButton( button, true );
    context->InjectMouseButton( button, false );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiWidgetTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Widgets" );

    GUCEF_TESTFW_TESTCASE( "Widgets: An edit box takes typed text, Backspace, a length limit, select-all on focus and Enter" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CEditbox* edit = static_cast< GUI::CEditbox* >( context->CreateWidget( "Editbox", "Name", screen ) );
        edit->SetProperty( "Size", "0,200 0,20" );
        edit->SetIntProperty( "MaxLength", 5 );
        edit->SetBoolProperty( "SelectAllOnFocus", true );
        edit->SetText( "Old" );
        CCounter entered( edit, GUI::CEditbox::TextEnteredEvent );
        CCounter changed( edit, GUI::CWidget::ValueChangedEvent );

        Click( context, 10, 10 );
        ASSERT_TRUE( edit->HasFocus() && edit->GetBoolProperty( "Selected" ) && edit->GetBoolProperty( "CaretVisible" ) );
        // the first key replaces the selected text
        context->InjectText( "Ab" );
        ASSERT_TRUE( "Ab" == edit->GetText() && !edit->GetBoolProperty( "Selected" ) );
        context->InjectText( "cdefg" );
        ASSERT_TRUE( "Abcde" == edit->GetText() );
        context->InjectKey( INPUT::KEYCODE_BACK, true, 0 );
        ASSERT_TRUE( "Abcd" == edit->GetText() );
        // control characters are not text
        context->InjectText( "\t" );
        ASSERT_TRUE( "Abcd" == edit->GetText() );
        // multi byte UTF-8 counts as one character
        context->InjectText( "\xC3\xA9" );
        ASSERT_TRUE( "Abcd\xC3\xA9" == edit->GetText() );
        context->InjectKey( INPUT::KEYCODE_BACK, true, 0 );
        ASSERT_TRUE( "Abcd" == edit->GetText() );
        ASSERT_TRUE( 5 == changed.count && "Abcd" == changed.lastValue );
        context->InjectKey( INPUT::KEYCODE_RETURN, true, 0 );
        ASSERT_TRUE( 1 == entered.count );

        // the caret blinks while focused
        bool caret = edit->GetBoolProperty( "CaretVisible" );
        context->RenderFrame( 600 );
        ASSERT_TRUE( caret != edit->GetBoolProperty( "CaretVisible" ) );

        // read only takes nothing
        edit->SetBoolProperty( "ReadOnly", true );
        context->InjectText( "x" );
        ASSERT_TRUE( "Abcd" == edit->GetText() );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Widgets: A list box selects, skips disabled rows, scrolls, hovers and activates" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CListbox* list = static_cast< GUI::CListbox* >( context->CreateWidget( "Listbox", "List", screen ) );
        list->SetProperty( "Position", "0,100 0,100" );
        list->SetProperty( "Size", "0,100 0,40" );
        list->SetIntProperty( "RowHeight", 10 );
        GUI::CListbox::TStringVector items;
        for ( UInt32 i=0; i<10; ++i )
            items.push_back( CORE::CString( "Item" ) + CORE::UInt32ToString( i ) );
        list->SetItems( items );
        list->SetProperty( "DisabledItems", "1" );
        CCounter changed( list, GUI::CWidget::ValueChangedEvent );
        CCounter activated( list, GUI::CWidget::ItemActivatedEvent );
        context->FlushChanges();
        ASSERT_TRUE( 4 == list->GetVisibleRowCount() );

        Click( context, 110, 125 );
        ASSERT_TRUE( 2 == list->GetSelectedIndex() && 1 == changed.count && "2" == changed.lastValue );
        ASSERT_TRUE( 2 == list->GetIntProperty( "HoverIndex" ) );
        // a disabled row cannot be chosen
        context->RenderFrame( 1000 );
        Click( context, 110, 115 );
        ASSERT_TRUE( 2 == list->GetSelectedIndex() );
        // down skips the disabled row upwards, Enter activates
        context->InjectNavigation( GUI::NAVIGATION_UP );
        ASSERT_TRUE( 0 == list->GetSelectedIndex() );
        context->InjectKey( INPUT::KEYCODE_RETURN, true, 0 );
        ASSERT_TRUE( 1 == activated.count && 0 == activated.lastIndex && "Item0" == activated.lastItem );
        // the wheel scrolls, never past the end
        context->InjectMouseWheel( -3 );
        ASSERT_TRUE( 3 == list->GetIntProperty( "ScrollPosition" ) );
        context->InjectMouseWheel( -30 );
        ASSERT_TRUE( 6 == list->GetIntProperty( "ScrollPosition" ) );
        ASSERT_TRUE( 6 == list->GetRowAt( 0 ) && -1 == list->GetRowAt( 40 ) );
        // a double click activates the row under the pointer
        context->RenderFrame( 1000 );
        Click( context, 110, 135 );
        Click( context, 110, 135 );
        ASSERT_TRUE( 2 == activated.count && 9 == activated.lastIndex );
        // leaving drops the hover
        context->InjectMouseMove( 400, 400 );
        ASSERT_TRUE( -1 == list->GetIntProperty( "HoverIndex" ) );

        // the classic interface
        UInt32 lines = list->GetLineCount();
        ASSERT_TRUE( list->AppendLine( "Last" ) && lines + 1 == list->GetLineCount() );
        CORE::CString line;
        ASSERT_TRUE( list->GetLineAtIndex( lines, line ) && "Last" == line );
        ASSERT_TRUE( list->SetAllowMultiSelect( true ) && list->GetAllowMultiSelect() );
        ASSERT_TRUE( list->SetLineSelection( 3, true ) && list->SetLineSelection( 5, true ) );
        GUI::CListbox::TStringVector selected;
        ASSERT_TRUE( list->GetSelectedLines( selected ) && 2 == selected.size() && "Item3" == selected[ 0 ] );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Widgets: Sliders drag and snap, spinners step, check boxes toggle, progress bars report" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = MakeScreen( context );

        GUI::CSlider* slider = static_cast< GUI::CSlider* >( context->CreateWidget( "Slider", "Volume", screen ) );
        slider->SetProperty( "Size", "0,110 0,10" );
        slider->SetProperty( "ThumbSize", "10" );
        slider->SetProperty( "Max", "6" );
        context->FlushChanges();
        CCounter sliderChanged( slider, GUI::CWidget::ValueChangedEvent );
        // the track is 100 pixels: the middle is 3
        Click( context, 55, 5 );
        ASSERT_TRUE( 3.0f == slider->GetValue() && 1 == sliderChanged.count );
        context->InjectMouseMove( 10, 5 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseMove( 200, 5 );
        ASSERT_TRUE( 6.0f == slider->GetValue() );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( GUI::CRect( 100, 0, 10, 10 ) == slider->GetThumbRect() );
        context->InjectMouseMove( 50, 5 );
        context->InjectMouseWheel( -2 );
        ASSERT_TRUE( 4.0f == slider->GetValue() );

        GUI::CSpinner* spinner = static_cast< GUI::CSpinner* >( context->CreateWidget( "Spinner", "Amount", screen ) );
        spinner->SetProperty( "Position", "0,0 0,100" );
        spinner->SetProperty( "Size", "0,80 0,20" );
        spinner->SetProperty( "Max", "15" );
        context->FlushChanges();
        Click( context, 75, 110 );
        ASSERT_TRUE( 1 == spinner->GetValueInt32() );
        Click( context, 75, 110, GUI::MOUSEBUTTON_RIGHT );
        ASSERT_TRUE( 11 == spinner->GetValueInt32() );
        Click( context, 75, 110, GUI::MOUSEBUTTON_RIGHT );
        ASSERT_TRUE( 15 == spinner->GetValueInt32() );
        Click( context, 5, 110 );
        ASSERT_TRUE( 14 == spinner->GetValueInt32() && "14" == spinner->GetValueText() );
        spinner->SetProperty( "Decimals", "1" );
        ASSERT_TRUE( "14.0" == spinner->GetValueText() );

        GUI::CCheckbox* check = static_cast< GUI::CCheckbox* >( context->CreateWidget( "Checkbox", "Sound", screen ) );
        check->SetProperty( "Position", "0,0 0,200" );
        check->SetProperty( "Size", "0,20 0,20" );
        check->SetProperty( "NormalImage", "Off" );
        check->SetProperty( "CheckedImage", "On" );
        CCounter checked( check, GUI::CCheckbox::BoxCheckedEvent );
        context->FlushChanges();
        Click( context, 5, 205 );
        ASSERT_TRUE( check->GetCheckedState() && 1 == checked.count );
        context->InjectMouseMove( 400, 400 );
        ASSERT_TRUE( "On" == check->GetCurrentImage() );
        Click( context, 5, 205 );
        ASSERT_FALSE( check->GetCheckedState() );

        GUI::CProgressBar* progress = static_cast< GUI::CProgressBar* >( context->CreateWidget( "ProgressBar", "Load", screen ) );
        progress->SetValue( 25.0f );
        ASSERT_TRUE( 0.25f == progress->GetFraction() );
        progress->SetValue( 250.0f );
        ASSERT_TRUE( 1.0f == progress->GetFraction() );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Widgets: A group box places its children below its caption and is as high as they need; an option follows the check box it depends on" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = MakeScreen( context );

        GUI::CGroupBox* group = GUI::CWidget::As< GUI::CGroupBox >( context->CreateWidget( "GroupBox", "Options", screen ) );
        ASSERT_TRUE( GUCEF_NULL != group );
        if ( GUCEF_NULL == group )
            return;
        group->SetProperty( "Position", "0,10 0,10" );
        group->SetProperty( "Size", "0,200 0,0" );
        ASSERT_TRUE( group->SetCaption( "Options" ) && "Options" == group->GetCaption() );
        GUI::CWidget* parent = context->CreateWidget( "Checkbox", "Parent", group );
        parent->SetProperty( "Size", "1,0 0,20" );
        GUI::CWidget* child = context->CreateWidget( "Checkbox", "Child", group );
        child->SetProperty( "Size", "1,-20 0,20" );
        child->SetProperty( "HAlign", "Right" );
        child->SetProperty( "EnabledBy", "Parent" );
        context->FlushChanges();

        // below the caption, inside the padding (10), 4 apart; the group is as high as that, as wide as its Size
        const Int32 caption = group->GetCaptionHeight();
        ASSERT_TRUE( caption > 0 );
        const GUI::CRect& area = group->GetRect();
        ASSERT_TRUE( 200 == area.width && caption + 10 + 20 + 4 + 20 + 10 == area.height );
        ASSERT_TRUE( parent->GetRect().left == area.left + 10 && parent->GetRect().top == area.top + 10 + caption && 180 == parent->GetRect().width );
        // the dependent option is indented: as wide as the content less 20, on the right
        ASSERT_TRUE( child->GetRect().left == area.left + 10 + 20 && child->GetRect().top == parent->GetRect().top + 24 );

        // the option is enabled while its check box is checked, also when the box is clicked
        ASSERT_FALSE( child->IsEffectivelyEnabled() );
        parent->SetBoolProperty( "Checked", true );
        ASSERT_TRUE( child->IsEffectivelyEnabled() );
        context->FlushChanges();
        Click( context, parent->GetRect().left + 5, parent->GetRect().top + 5 );
        ASSERT_FALSE( parent->GetBoolProperty( "Checked" ) || child->IsEffectivelyEnabled() );

        // an option made later finds its check box; a check box made later finds the options that wait for it
        GUI::CWidget* late = context->CreateWidget( "Checkbox", "Late", group );
        late->SetProperty( "Size", "1,0 0,20" );
        late->SetProperty( "EnabledBy", "Parent" );
        ASSERT_FALSE( late->IsEffectivelyEnabled() );
        GUI::CWidget* waiting = context->CreateWidget( "Label", "Waiting", group );
        waiting->SetProperty( "Size", "1,0 0,20" );
        waiting->SetProperty( "EnabledBy", "Switch" );
        ASSERT_TRUE( waiting->IsEffectivelyEnabled() );     // nothing called Switch yet
        GUI::CWidget* switchBox = context->CreateWidget( "Checkbox", "Switch", GUCEF_NULL );
        switchBox->SetProperty( "Size", "1,0 0,20" );
        switchBox->SetParentWidget( group );
        ASSERT_FALSE( waiting->IsEffectivelyEnabled() );
        switchBox->SetBoolProperty( "Checked", true );
        ASSERT_TRUE( waiting->IsEffectivelyEnabled() );
        context->FlushChanges();
        ASSERT_TRUE( caption + 10 + 5 * 20 + 4 * 4 + 10 == group->GetRect().height );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Widgets: Picture buttons show the picture of their state" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CButton* button = static_cast< GUI::CButton* >( context->CreateWidget( "Button", "Pic", screen ) );
        button->SetProperty( "Size", "0,20 0,20" );
        button->SetProperty( "NormalImage", "N" );
        button->SetProperty( "HoverImage", "H" );
        button->SetProperty( "PushedImage", "P" );
        button->SetProperty( "DisabledImage", "D" );
        context->FlushChanges();
        ASSERT_TRUE( "N" == button->GetCurrentImage() );
        context->InjectMouseMove( 5, 5 );
        ASSERT_TRUE( "H" == button->GetCurrentImage() );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        ASSERT_TRUE( "P" == button->GetCurrentImage() );
        // dragged off while pressed: normal, not pushed
        context->InjectMouseMove( 100, 100 );
        ASSERT_TRUE( "N" == button->GetCurrentImage() );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        button->SetEnabled( false );
        ASSERT_TRUE( "D" == button->GetCurrentImage() );
        button->ResetProperty( "DisabledImage" );
        ASSERT_TRUE( "N" == button->GetCurrentImage() );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Widgets: A combo box drops its list as a popup, a click outside closes it" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        context->RegisterFont( "Test", MakeTestFont( 8, 10 ) );
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CCombobox* combo = static_cast< GUI::CCombobox* >( context->CreateWidget( "Combobox", "Size", screen ) );
        combo->SetProperty( "Position", "0,50 0,50" );
        combo->SetProperty( "Size", "0,100 0,20" );
        combo->SetProperty( "Font", "Test" );
        GUI::CCombobox::TStringVector items;
        items.push_back( "Small" );
        items.push_back( "Medium" );
        items.push_back( "Large" );
        combo->SetListItems( items );
        CCounter changed( combo, GUI::CWidget::ValueChangedEvent );
        context->FlushChanges();

        Click( context, 60, 60 );
        ASSERT_TRUE( combo->GetBoolProperty( "Expanded" ) && GUCEF_NULL != context->GetPopup() );
        context->FlushChanges();
        GUI::CWidget* list = context->GetPopup();
        ASSERT_TRUE( GUI::CRect( 50, 70, 100, 30 ) == list->GetRect() );
        Click( context, 60, 92 );
        ASSERT_TRUE( 2 == combo->GetSelectedIndex() && "Large" == combo->GetText() && 1 == changed.count );
        ASSERT_FALSE( combo->GetBoolProperty( "Expanded" ) );
        ASSERT_TRUE( GUCEF_NULL == context->GetPopup() );

        // open, then a click somewhere else: closed, nothing chosen, the click is used up
        Click( context, 60, 60 );
        ASSERT_TRUE( combo->GetBoolProperty( "Expanded" ) );
        Click( context, 400, 400 );
        ASSERT_FALSE( combo->GetBoolProperty( "Expanded" ) );
        ASSERT_TRUE( 2 == combo->GetSelectedIndex() );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Widgets: The wheel scrolls the list of a combo box without closing it or picking from it" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        context->RegisterFont( "Test", MakeTestFont( 8, 10 ) );
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CCombobox* combo = static_cast< GUI::CCombobox* >( context->CreateWidget( "Combobox", "Size", screen ) );
        combo->SetProperty( "Position", "0,50 0,50" );
        combo->SetProperty( "Size", "0,100 0,20" );
        combo->SetProperty( "Font", "Test" );
        combo->SetIntProperty( "DropDownRows", 4 );
        GUI::CCombobox::TStringVector items;
        for ( UInt32 i=0; i<20; ++i )
            items.push_back( "Item " + CORE::UInt32ToString( i ) );
        combo->SetListItems( items );
        combo->SetSelectedIndex( 1 );
        CCounter changed( combo, GUI::CWidget::ValueChangedEvent );
        context->FlushChanges();

        Click( context, 60, 60 );
        ASSERT_TRUE( combo->GetBoolProperty( "Expanded" ) && GUCEF_NULL != context->GetPopup() );
        context->FlushChanges();
        GUI::CWidget* list = context->GetPopup();
        context->InjectMouseMove( list->GetRect().left + 10, list->GetRect().top + 10 );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( 1 == list->GetIntProperty( "ScrollPosition" ) );
        ASSERT_TRUE( combo->GetBoolProperty( "Expanded" ) && context->GetPopup() == list );
        ASSERT_TRUE( 1 == combo->GetSelectedIndex() && 0 == changed.count );
        context->DestroyWidget( screen );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Widgets: Menu bars, popup menus, trees, tabs, grids and text boxes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        context->RegisterFont( "Test", MakeTestFont( 8, 10 ) );
        GUI::CWidget* screen = MakeScreen( context );

        // menu bar: "File" is 32 pixels of text plus 16
        GUI::CMenuBar* bar = static_cast< GUI::CMenuBar* >( context->CreateWidget( "MenuBar", "Menu", screen ) );
        bar->SetProperty( "Font", "Test" );
        bar->SetProperty( "Size", "1,0 0,14" );
        GUI::CMenuBar::TStringVector menuItems;
        menuItems.push_back( "File/Open" );
        menuItems.push_back( "File/Save" );
        menuItems.push_back( "Help/About" );
        bar->SetProperty( "Items", GUI::JoinStringList( menuItems ) );
        CCounter menuChosen( bar, GUI::CWidget::ItemActivatedEvent );
        context->FlushChanges();
        ASSERT_TRUE( 2 == bar->GetMenus().size() && 2 == bar->GetMenuItems( "File" ).size() );
        ASSERT_TRUE( GUI::CRect( 48, 0, 48, 14 ) == bar->GetMenuRect( 1 ) );
        Click( context, 10, 5 );
        ASSERT_TRUE( 0 == bar->GetIntProperty( "OpenMenu" ) && GUCEF_NULL != context->GetPopup() );
        context->FlushChanges();
        Click( context, 10, 14 + 15 );
        ASSERT_TRUE( 1 == menuChosen.count && "File/Save" == menuChosen.lastItem );
        ASSERT_TRUE( -1 == bar->GetIntProperty( "OpenMenu" ) && GUCEF_NULL == context->GetPopup() );

        // popup menu: separators and disabled items cannot be chosen
        GUI::CPopupMenu* popup = static_cast< GUI::CPopupMenu* >( context->CreateWidget( "PopupMenu", "Context", GUCEF_NULL ) );
        popup->SetProperty( "Font", "Test" );
        popup->SetProperty( "Items", "Cut\n-\nPaste" );
        CCounter popupChosen( popup, GUI::CWidget::ItemActivatedEvent );
        ASSERT_TRUE( popup->ShowAt( 300, 300 ) );
        context->FlushChanges();
        Click( context, 305, 315 );
        ASSERT_TRUE( 0 == popupChosen.count && GUCEF_NULL != context->GetPopup() );
        context->InjectNavigation( GUI::NAVIGATION_DOWN );
        ASSERT_TRUE( 0 == popup->GetIntProperty( "HoverIndex" ) );
        context->InjectNavigation( GUI::NAVIGATION_DOWN );
        ASSERT_TRUE( 2 == popup->GetIntProperty( "HoverIndex" ) );
        context->InjectNavigation( GUI::NAVIGATION_ACCEPT );
        ASSERT_TRUE( 1 == popupChosen.count && "Paste" == popupChosen.lastItem && 2 == popupChosen.lastIndex );
        ASSERT_TRUE( GUCEF_NULL == context->GetPopup() );

        // tree: only open levels show, a click in the indent opens a level
        GUI::CTreeview* tree = static_cast< GUI::CTreeview* >( context->CreateWidget( "Treeview", "Tree", screen ) );
        tree->SetProperty( "Font", "Test" );
        tree->SetProperty( "Position", "0,0 0,100" );
        tree->SetProperty( "Size", "0,200 0,100" );
        tree->SetProperty( "Items", "Ships/Hulls/Scout\nShips/Weapons\nPlanets" );
        context->FlushChanges();
        GUI::CTreeview::TRowVector rows = tree->GetVisibleRows();
        ASSERT_TRUE( 2 == rows.size() && "Ships" == rows[ 0 ].path && rows[ 0 ].hasChildren && "Planets" == rows[ 1 ].path && !rows[ 1 ].hasChildren );
        Click( context, 5, 105 );
        rows = tree->GetVisibleRows();
        ASSERT_TRUE( 4 == rows.size() && "Ships/Hulls" == rows[ 1 ].path && 1 == rows[ 1 ].depth && "Ships/Weapons" == rows[ 2 ].path );
        context->RenderFrame( 1000 );
        Click( context, 50, 125 );
        ASSERT_TRUE( "Ships/Weapons" == tree->GetSelectedItem() );
        context->InjectNavigation( GUI::NAVIGATION_UP );
        ASSERT_TRUE( "Ships/Hulls" == tree->GetSelectedItem() );
        context->InjectNavigation( GUI::NAVIGATION_RIGHT );
        ASSERT_TRUE( tree->IsExpanded( "Ships/Hulls" ) && 5 == tree->GetVisibleRows().size() );

        // tabs: one page shown at a time, below the tab headers
        GUI::CTabControl* tabs = static_cast< GUI::CTabControl* >( context->CreateWidget( "TabControl", "Tabs", screen ) );
        tabs->SetProperty( "Position", "0,300 0,0" );
        tabs->SetProperty( "Size", "0,200 0,100" );
        tabs->SetProperty( "Tabs", "One\nTwo" );
        GUI::CWidget* pageOne = context->CreateWidget( "TabContentPane", "PageOne", tabs );
        GUI::CWidget* pageTwo = context->CreateWidget( "TabContentPane", "PageTwo", tabs );

        // pages added (as a layout does) show the active one only, also before the active tab is ever set
        ASSERT_TRUE( pageOne->IsVisible() && !pageTwo->IsVisible() );
        GUI::CWidget* pageThree = context->CreateWidget( "TabContentPane", "PageThree", screen );
        ASSERT_TRUE( pageThree->IsVisible() && pageThree->SetParentWidget( tabs ) );
        ASSERT_FALSE( pageThree->IsVisible() );
        ASSERT_TRUE( pageThree->SetParentWidget( screen ) );
        context->DestroyWidget( pageThree );

        tabs->SetActiveTab( 1 );
        tabs->SetActiveTab( 0 );
        context->FlushChanges();
        ASSERT_TRUE( pageOne->IsVisible() && !pageTwo->IsVisible() );
        ASSERT_TRUE( GUI::CRect( 300, 20, 200, 80 ) == pageOne->GetRect() );
        ASSERT_TRUE( pageTwo == tabs->GetTabContentPane( 1 ) );
        Click( context, 450, 10 );
        ASSERT_TRUE( 1 == tabs->GetActiveTab() && !pageOne->IsVisible() && pageTwo->IsVisible() );

        // grid: the header row, then the data rows
        GUI::CGridView* grid = static_cast< GUI::CGridView* >( context->CreateWidget( "GridView", "Grid", screen ) );
        grid->SetProperty( "Position", "0,300 0,200" );
        grid->SetProperty( "Size", "0,200 0,100" );
        grid->SetProperty( "RowHeight", "10" );
        grid->SetProperty( "Columns", "Name\nAge" );
        grid->SetText( 0, 0, "Ann" );
        grid->SetText( 1, 0, "31" );
        grid->AppendText( "Bob" );
        ASSERT_TRUE( 2 == grid->GetRowCount() && 2 == grid->GetColumnCount() && "31" == grid->GetText( 1, 0 ) && "Bob" == grid->GetText( 0, 1 ) );
        context->FlushChanges();
        Click( context, 410, 225 );
        ASSERT_TRUE( 1 == grid->GetIntProperty( "SelectedRow" ) );
        GUI::CGridView::TGridIndexVector cells;
        grid->GetSelectedItems( cells );
        ASSERT_TRUE( 2 == cells.size() && 1 == cells[ 1 ].column && 1 == cells[ 1 ].row );

        // text box: lines, wrapping and scrolling
        GUI::CTextbox* text = static_cast< GUI::CTextbox* >( context->CreateWidget( "Textbox", "News", screen ) );
        text->SetProperty( "Font", "Test" );
        text->SetProperty( "Position", "0,0 0,300" );
        text->SetProperty( "Size", "0,72 0,20" );
        ASSERT_TRUE( text->AppendLine( "first line" ) && text->AppendLine( "second" ) && text->AppendLine( "third" ) );
        ASSERT_TRUE( 3 == text->GetLineCount() );
        context->FlushChanges();
        ASSERT_TRUE( 4 == text->GetDisplayLines().size() && 2 == text->GetVisibleLineCount() );
        context->InjectMouseMove( 10, 305 );
        context->InjectMouseWheel( -5 );
        ASSERT_TRUE( 2 == text->GetIntProperty( "ScrollPosition" ) );

        context->DestroyWidget( screen );
        context->DestroyWidget( popup );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Widgets: Image frames animate and take pixels, draw surfaces keep a committed drawing" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = static_cast< GUI::CGUIContext* >( contextPtr.GetPointerAlways() );
        GUI::CWidget* screen = MakeScreen( context );
        context->RegisterImage( "F0", MakeSolidImage( 4, 4, 1, 0, 0, 255 ) );
        context->RegisterImage( "F1", MakeSolidImage( 4, 4, 2, 0, 0, 255 ) );
        context->RegisterImage( "F2", MakeSolidImage( 4, 4, 3, 0, 0, 255 ) );

        GUI::CImageFrame* frame = static_cast< GUI::CImageFrame* >( context->CreateWidget( "ImageFrame", "Anim", screen ) );
        GUI::CImageFrame::TStringVector images;
        images.push_back( "F0" );
        images.push_back( "F1" );
        images.push_back( "F2" );
        GUI::CImageFrame::TUInt32Vector durations( 3, 100 );
        CCounter ended( frame, GUI::CImageFrame::AnimationEndedEvent );
        ASSERT_TRUE( frame->SetAnimation( images, durations, false ) );
        ASSERT_TRUE( "F0" == frame->GetImageName() );
        context->RenderFrame( 150 );
        ASSERT_TRUE( "F1" == frame->GetImageName() && 1 == frame->GetAnimationFrame() );
        context->RenderFrame( 100 );
        ASSERT_TRUE( "F2" == frame->GetImageName() );
        context->RenderFrame( 500 );
        ASSERT_TRUE( !frame->IsAnimationPlaying() && 1 == ended.count && "F2" == frame->GetImageName() );
        // looping goes round
        frame->SetAnimation( images, durations, true );
        context->RenderFrame( 350 );
        ASSERT_TRUE( "F0" == frame->GetImageName() && frame->IsAnimationPlaying() );
        frame->StopAnimation();

        // own pixels: registered under the frame's own name, changes show through UpdateImage()
        IMAGE::TPixelMapPtr pixels = MakeSolidImage( 8, 6, 0, 0, 255, 255 );
        ASSERT_TRUE( frame->SetImage( pixels ) );
        context->FlushChanges();
        ASSERT_TRUE( 8 == frame->GetRect().width && 6 == frame->GetRect().height );
        IMAGE::TPixelMapPtr resolved;
        UInt32 revision = 0;
        ASSERT_TRUE( context->ResolveImage( frame->GetImageName(), resolved, revision ) && resolved == pixels );
        ASSERT_TRUE( frame->UpdateImage() );
        UInt32 newRevision = 0;
        context->ResolveImage( frame->GetImageName(), resolved, newRevision );
        ASSERT_TRUE( newRevision == revision + 1 );
        CORE::CString ownName = frame->GetImageName();
        context->DestroyWidget( screen );
        ASSERT_FALSE( context->HasImage( ownName ) );

        GUI::CDrawSurface* surface = static_cast< GUI::CDrawSurface* >( context->CreateWidget( "DrawSurface", "Map", GUCEF_NULL ) );
        surface->FillRect( GUI::CRect( 0, 0, 10, 10 ), GUI::CColour( 255, 0, 0 ) );
        surface->DrawLine( 0, 0, 5, 5, GUI::CColour( 0, 255, 0 ) );
        GUI::CDrawSurface::TPointVector points( 2 );
        points[ 0 ].x = 1; points[ 0 ].y = 2;
        points[ 1 ].x = 3; points[ 1 ].y = 4;
        surface->DrawPoints( points, GUI::CColour( 0, 0, 255 ) );
        ASSERT_TRUE( surface->GetCommands().empty() && 0 == surface->GetRevision() );
        surface->Commit();
        ASSERT_TRUE( 3 == surface->GetCommands().size() && 1 == surface->GetRevision() && "1" == surface->GetProperty( "Revision" ) );
        ASSERT_TRUE( GUI::CDrawSurface::COMMAND_POINTS == surface->GetCommands()[ 2 ].type && 2 == surface->GetCommands()[ 2 ].points.size() );
        // drawing goes on in the next list, the committed one stays until the next commit
        surface->Clear();
        surface->DrawString( "Font", "text", 1, 2, GUI::CColour() );
        ASSERT_TRUE( 3 == surface->GetCommands().size() );
        surface->Commit();
        ASSERT_TRUE( 1 == surface->GetCommands().size() && "text" == surface->GetCommands()[ 0 ].text );
        context->DestroyWidget( surface );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
