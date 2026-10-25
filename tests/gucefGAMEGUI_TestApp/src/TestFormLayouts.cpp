/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#include "gucefGAMEGUI_TestCommon.h"
#include "TestLobbyFixture.h"
#include "TestListingProvider.h"
#include "TestChatService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef GUCEF::vector< GUI::CWidget* > TWidgetVector;

struct SScreenSize
{
    UInt32 width;
    UInt32 height;
};

/** The sizes the forms are made for: the smallest, common ones, the window of the developer, a large one, ultra wide (21:9, 32:9) and a portrait one */
const SScreenSize ScreenSizes[] = { { 800, 600 }, { 1024, 768 }, { 1242, 952 }, { 1920, 1080 }, { 3440, 1440 }, { 3840, 1080 }, { 5120, 1440 }, { 600, 900 } };
const UInt32 ScreenSizeCount = 8;

/** The most a scroll bar may be from what it scrolls: in a wide window it sits at the content, not at the far edge */
const Int32 ScrollBarGapLimit = 24;

CORE::CString
SizeName( UInt32 width, UInt32 height )
{
    return CORE::UInt32ToString( width ) + "x" + CORE::UInt32ToString( height );
}

void
Collect( GUI::CWidget* widget, TWidgetVector& all )
{
    all.push_back( widget );
    const GUI::CWidget::TWidgetVector& children = widget->GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
        Collect( children[ i ], all );
}

/** The widget as a scroll panel or as a window, NULL when it is another widget or there is none */
GUI::CScrollPanel*
AsScrollPanelOf( GUI::CWidget* widget )
{
    return GUCEF_NULL != widget ? widget->AsScrollPanel() : GUCEF_NULL;
}

const GUI::CScrollPanel*
AsScrollPanelOf( const GUI::CWidget* widget )
{
    return GUCEF_NULL != widget ? widget->AsScrollPanel() : GUCEF_NULL;
}

GUI::CWindow*
AsWindowOf( GUI::CWidget* widget )
{
    return GUCEF_NULL != widget ? widget->AsWindow() : GUCEF_NULL;
}

bool
IsContainerType( const CORE::CString& type )
{
    return type == "Panel" || type == "Form" || type == "VBox" || type == "HBox" || type == "Grid" || type == "Margin" ||
           type == "GroupBox" || type == "Window" || type == "ScrollPanel" || type == "TabControl" || type == "TabContentPane";
}

bool
IsDescendantOrSelf( const GUI::CWidget* widget, const GUI::CWidget* ancestor )
{
    for ( const GUI::CWidget* w=widget; GUCEF_NULL != w; w=w->GetParentWidget() )
        if ( w == ancestor )
            return true;
    return false;
}

bool
Inside( const GUI::CRect& inner, const GUI::CRect& outer )
{
    return inner.left >= outer.left && inner.top >= outer.top && inner.Right() <= outer.Right() && inner.Bottom() <= outer.Bottom();
}

bool
Intersects( const GUI::CRect& a, const GUI::CRect& b )
{
    return !a.IsEmpty() && !b.IsEmpty() && !a.Intersection( b ).IsEmpty();
}

/** The part of a widget that is seen: cut off at every ancestor that clips (a scroll panel, a window) */
GUI::CRect
VisibleRect( const GUI::CWidget* widget )
{
    GUI::CRect rect = widget->GetRect();
    for ( const GUI::CWidget* p=widget->GetParentWidget(); GUCEF_NULL != p; p=p->GetParentWidget() )
        if ( p->GetBoolProperty( "Clip" ) )
            rect = rect.Intersection( p->GetRect() );
    return rect;
}

/**
 *  The first thing that is wrong with the layout of a form, empty when it is fine:
 *  - a widget outside the rectangle of its parent (the width of a scroll panel less its scroll bar for what is in it; what
 *    is scrolled out vertically is cut off and is fine)
 *  - two controls that are seen and overlap
 *  - something that is seen overlapping the strip along the bottom edge, which has the buttons
 */
CORE::CString
FindLayoutProblem( GUI::CWidget* root )
{
    TWidgetVector all;
    Collect( root, all );

    for ( UInt32 i=1; i<(UInt32)all.size(); ++i )
    {
        GUI::CWidget* widget = all[ i ];
        GUI::CWidget* parent = widget->GetParentWidget();
        if ( !widget->IsVisible() || widget->GetRect().IsEmpty() )
            continue;
        GUI::CRect parentRect = parent->GetRect();
        const GUI::CScrollPanel* scrollParent = AsScrollPanelOf( parent );
        const bool scrolls = GUCEF_NULL != scrollParent;
        if ( scrolls )
        {
            // the part of the column left of the scroll bar
            const GUI::CRect viewport = scrollParent->GetViewportRect();
            parentRect = GUI::CRect( parentRect.left + viewport.left, parentRect.top, viewport.width, parentRect.height );
        }
        GUI::CRect rect = widget->GetRect();
        if ( scrolls )
        {
            if ( rect.left < parentRect.left || rect.Right() > parentRect.Right() )
                return "\"" + widget->GetName() + "\" is wider than the scroll panel \"" + parent->GetName() + "\"";
        }
        else
        if ( !Inside( rect, parentRect ) )
            return "\"" + widget->GetName() + "\" (" + widget->GetType() + ") is outside \"" + parent->GetName() + "\"";
    }

    // the scroll bar is next to what it scrolls, also in a wide window, and inside the panel
    for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
    {
        const GUI::CScrollPanel* scrollPanel = AsScrollPanelOf( all[ i ] );
        if ( GUCEF_NULL == scrollPanel || !scrollPanel->IsVisible() || scrollPanel->GetRect().IsEmpty() )
            continue;
        const GUI::CRect panelRect = scrollPanel->GetRect();
        const GUI::CRect track = scrollPanel->GetTrackRect();
        const GUI::CRect viewport = scrollPanel->GetViewportRect();
        if ( track.IsEmpty() || track.left < 0 || track.Right() > panelRect.width || viewport.Right() != track.left )
            return "the scroll bar of \"" + scrollPanel->GetName() + "\" is not at the right end of its column";
        Int32 contentRight = -1;
        const GUI::CWidget::TWidgetVector& children = scrollPanel->GetChildren();
        for ( UInt32 c=0; c<(UInt32)children.size(); ++c )
            if ( children[ c ]->IsVisible() && !children[ c ]->GetRect().IsEmpty() && children[ c ]->GetRect().Right() > contentRight )
                contentRight = children[ c ]->GetRect().Right();
        if ( contentRight >= 0 )
        {
            const Int32 gap = panelRect.left + track.left - contentRight;
            if ( gap < 0 || gap > ScrollBarGapLimit )
                return "the scroll bar of \"" + scrollPanel->GetName() + "\" is " + CORE::Int32ToString( gap ) + " pixels from what it scrolls";
        }
    }

    // the controls that are seen do not overlap
    TWidgetVector controls;
    for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
        if ( !IsContainerType( all[ i ]->GetType() ) && all[ i ]->IsVisible() && !VisibleRect( all[ i ] ).IsEmpty() )
            controls.push_back( all[ i ] );
    for ( UInt32 i=0; i<(UInt32)controls.size(); ++i )
        for ( UInt32 j=i+1; j<(UInt32)controls.size(); ++j )
            if ( Intersects( VisibleRect( controls[ i ] ), VisibleRect( controls[ j ] ) ) )
                return "\"" + controls[ i ]->GetName() + "\" overlaps \"" + controls[ j ]->GetName() + "\"";

    // the strip with the buttons
    GUI::CWidget* strip = GUCEF_NULL;
    for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
        if ( all[ i ]->GetName() == "ButtonRow" )
            strip = all[ i ];
    if ( GUCEF_NULL != strip )
    {
        for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
        {
            if ( all[ i ] == root || IsDescendantOrSelf( all[ i ], strip ) || !all[ i ]->IsVisible() )
                continue;
            if ( Intersects( VisibleRect( all[ i ] ), strip->GetRect() ) )
                return "\"" + all[ i ]->GetName() + "\" overlaps the strip of the buttons";
        }
    }
    return CORE::CString::Empty;
}

/**
 *  A form in a window of a limited width has its scroll bars inside that window, not further than a margin from its right edge: in a wide
 *  screen they are with the content in the window and not at the edge of the screen. Empty when it is so
 */
CORE::CString
FindBarEdgeProblem( GUI::CWidget* window )
{
    TWidgetVector all;
    Collect( window, all );
    const GUI::CRect windowRect = window->GetRect();
    const GUI::CScrollPanel* outermost = GUCEF_NULL;
    Int32 outermostRight = 0;
    for ( UInt32 i=0; i<(UInt32)all.size(); ++i )
    {
        const GUI::CScrollPanel* scrollPanel = AsScrollPanelOf( all[ i ] );
        if ( GUCEF_NULL == scrollPanel || !scrollPanel->IsVisible() || scrollPanel->GetRect().IsEmpty() )
            continue;
        const Int32 trackRight = scrollPanel->GetRect().left + scrollPanel->GetTrackRect().Right();
        if ( trackRight > windowRect.Right() )
            return "the scroll bar of \"" + scrollPanel->GetName() + "\" is beyond the right edge of the window";
        if ( GUCEF_NULL == outermost || trackRight > outermostRight )
        {
            outermost = scrollPanel;
            outermostRight = trackRight;
        }
    }
    // the one furthest right: next to the edge of the window; the others are in columns and are checked against what they scroll
    if ( GUCEF_NULL != outermost && windowRect.Right() - outermostRight > 48 )
        return "the scroll bar of \"" + outermost->GetName() + "\" is " + CORE::Int32ToString( windowRect.Right() - outermostRight ) + " pixels from the right edge of the window";
    return CORE::CString::Empty;
}

/** What the wheel does with a pointer over a widget: the scroll panels it is in, and what takes the wheel for a value */
bool
UsesWheelForItself( const GUI::CWidget* widget )
{
    const CORE::CString& type = widget->GetType();
    return widget->IsEffectivelyEnabled() && ( type == "Listbox" || type == "Textbox" || type == "Editbox" || type == "Slider" || type == "Spinner" || type == "GridView" || type == "Treeview" || type == "Combobox" );
}

/**
 *  The wheel over every widget of the form that is in the view of a scroll panel scrolls a scroll panel it is in, the widgets that are
 *  disabled included, as long as one of them has room to scroll. Empty when it does, else the first widget where it does not
 */
CORE::CString
FindWheelProblem( GUI::CGUIContext* context, GUI::CWidget* root, UInt32& tried )
{
    TWidgetVector all;
    Collect( root, all );
    for ( UInt32 i=1; i<(UInt32)all.size(); ++i )
    {
        GUI::CWidget* widget = all[ i ];
        if ( !widget->IsVisible() || VisibleRect( widget ).IsEmpty() || UsesWheelForItself( widget ) )
            continue;
        const GUI::CRect seen = VisibleRect( widget );
        const Int32 x = seen.left + seen.width / 2;
        const Int32 y = seen.top + seen.height / 2;
        GUI::CWidget* hit = context->HitTest( x, y );
        if ( GUCEF_NULL == hit || UsesWheelForItself( hit ) )
            continue;

        // the scroll panels the pointer is over, from the inside out, and how far each can go down
        GUCEF::vector< GUI::CScrollPanel* > panels;
        for ( GUI::CWidget* w=hit; GUCEF_NULL != w; w=w->GetParentWidget() )
        {
            if ( w != hit && UsesWheelForItself( w ) )
            {
                panels.clear();
                break;
            }
            GUI::CScrollPanel* panel = AsScrollPanelOf( w );
            if ( GUCEF_NULL != panel && panel->IsScrollable() )
                panels.push_back( panel );
        }
        Int32 before = 0;
        bool canScroll = false;
        for ( UInt32 p=0; p<(UInt32)panels.size(); ++p )
        {
            before += panels[ p ]->GetScrollPosition();
            if ( panels[ p ]->GetScrollPosition() < panels[ p ]->GetMaxScrollPosition() )
                canScroll = true;
        }
        if ( !canScroll )
            continue;

        context->InjectMouseMove( x, y );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        Int32 after = 0;
        for ( UInt32 p=0; p<(UInt32)panels.size(); ++p )
            after += panels[ p ]->GetScrollPosition();
        if ( after <= before )
            return "the wheel over \"" + hit->GetName() + "\" (" + hit->GetType() + ( hit->IsEffectivelyEnabled() ? ", enabled" : ", disabled" ) + ") scrolls nothing";
        ++tried;
        for ( UInt32 p=0; p<(UInt32)panels.size(); ++p )
            panels[ p ]->SetScrollPosition( 0 );
        context->FlushChanges();
    }
    return CORE::CString::Empty;
}

/** Every pixel outside the rectangle is black: nothing is drawn beyond the window */
bool
OnlyBlackOutside( const IMAGE::TPixelMapPtr& frame, const GUI::CRect& rect )
{
    if ( frame.IsNULL() )
        return false;
    const UInt8* data = static_cast< const UInt8* >( frame->GetDataPtr() );
    const Int32 width = (Int32) frame->GetWidthInPixels();
    const Int32 height = (Int32) frame->GetHeightInPixels();
    for ( Int32 y=0; y<height; ++y )
    {
        for ( Int32 x=0; x<width; ++x )
        {
            if ( x >= rect.left && x < rect.Right() && y >= rect.top && y < rect.Bottom() )
                continue;
            const UInt8* pixel = data + ( (size_t) y * width + x ) * 4;
            if ( 0 != pixel[ 0 ] || 0 != pixel[ 1 ] || 0 != pixel[ 2 ] )
                return false;
        }
    }
    return true;
}

/** How many pixels inside the rectangle are not black: the window has been drawn */
UInt32
CountLit( const IMAGE::TPixelMapPtr& frame, const GUI::CRect& rect )
{
    UInt32 lit = 0;
    if ( frame.IsNULL() )
        return 0;
    const UInt8* data = static_cast< const UInt8* >( frame->GetDataPtr() );
    const Int32 width = (Int32) frame->GetWidthInPixels();
    for ( Int32 y=rect.top; y<rect.Bottom(); y+=3 )
        for ( Int32 x=rect.left; x<rect.Right(); x+=3 )
            if ( 0 != data[ ( (size_t) y * width + x ) * 4 + 1 ] )
                ++lit;
    return lit;
}

/** The settings of a game with groups, a long label, a unit and a setting that follows another: what a game's setup screen has */
void
AddManySettings( MULTIPLAY::CLobbySettingsSchema& schema )
{
    MULTIPLAY::CLobbySettingDef def;
    schema.settings.clear();

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "scenario"; def.label = "Scenario"; def.group = "Setup"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
    def.options.push_back( "beginner" ); def.options.push_back( "intermediate" ); def.options.push_back( "custom" );
    def.defaultValue = "beginner"; def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
    schema.AddSetting( def );

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "size"; def.label = "Map size"; def.group = "Custom map"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
    def.options.push_back( "100" ); def.options.push_back( "125" ); def.options.push_back( "150" );
    def.defaultValue = "100"; def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
    schema.AddSetting( def );

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "density"; def.label = "Density"; def.group = "Custom map"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
    def.options.push_back( "low" ); def.options.push_back( "high" );
    def.defaultValue = "high"; def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
    schema.AddSetting( def );

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "computers"; def.label = "Computer players"; def.group = "Custom map"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
    def.hasRange = true; def.minValue = 0; def.maxValue = 7; def.step = 1; def.defaultValue = "1";
    def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
    schema.AddSetting( def );

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "limit"; def.label = "Turn limit (0: none)"; def.group = "Custom map"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
    def.hasRange = true; def.minValue = 0; def.maxValue = 999; def.step = 1; def.defaultValue = "0";
    def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
    schema.AddSetting( def );

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "difficulty"; def.label = "Computer players play as"; def.group = "Computer players"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
    def.options.push_back( "beginner" ); def.options.push_back( "intermediate" ); def.options.push_back( "veteran" );
    def.defaultValue = "intermediate"; def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
    schema.AddSetting( def );

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "seed"; def.label = "Seed of the map"; def.group = "Setup"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
    def.hasRange = true; def.minValue = 1; def.maxValue = 999999; def.step = 1; def.defaultValue = "417428";
    def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
    schema.AddSetting( def );

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "rules"; def.label = "Rules"; def.group = "Rules"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
    def.options.push_back( "1.0" ); def.options.push_back( "1.1" ); def.options.push_back( "match the assets" );
    def.defaultValue = "1.1"; def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
    schema.AddSetting( def );

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "expand"; def.label = "Computer players settle free planets"; def.group = "Computer players"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_BOOL;
    def.defaultValue = "false"; def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_BOSS;
    schema.AddSetting( def );

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "timer"; def.label = "Turn timer"; def.group = "Turn timer"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_BOOL;
    def.defaultValue = "true"; def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_VOTE;
    schema.AddSetting( def );

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "seconds"; def.label = "Turn time"; def.group = "Turn timer"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_INT;
    def.hasRange = true; def.minValue = 15; def.maxValue = 3600; def.step = 15; def.defaultValue = "180"; def.unit = "s";
    def.enabledByKey = "timer"; def.enabledByValue = "true"; def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_VOTE;
    schema.AddSetting( def );

    def = MULTIPLAY::CLobbySettingDef();
    def.key = "drop"; def.label = "A player that leaves is played by (ai: the computer, inert: nobody)"; def.group = "Turn timer"; def.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
    def.options.push_back( "ai" ); def.options.push_back( "inert" );
    def.defaultValue = "ai"; def.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_VOTE;
    schema.AddSetting( def );
}

/** What the game gives the form that creates a lobby */
MULTIPLAY::CLobbyCreateParams
MakeCreateTemplate( void )
{
    MULTIPLAY::CLobbyCreateParams params;
    params.name = "Test machine's game";
    params.gameId = "testgame";
    params.gameVersion = "1.0";
    params.maxPlayers = 4;
    params.allowSpectators = true;
    params.isPublic = false;
    params.topology = MULTIPLAY::GAMETOPOLOGY_CLIENT_SERVER;
    AddManySettings( params.schema );
    return params;
}

/** The first and last control of the settings, in the order of the schema, as widgets */
GUI::CWidget*
ControlOf( GAMEGUI::CLobbySettingsPanelForm* panel, const char* key )
{
    return GUCEF_NULL != panel ? panel->GetControl( key ) : GUCEF_NULL;
}

/** A game in the list of the browser */
MULTIPLAY::CGameListingEntry
MakeListing( const char* id, const char* name, MULTIPLAY::CGameListingEntry::TKind kind, UInt32 players, UInt32 maxPlayers, const char* map, bool passworded, Int32 pingMs )
{
    MULTIPLAY::CGameListingEntry listing;
    listing.listingId = id;
    listing.name = name;
    listing.kind = kind;
    listing.gameId = "testgame";
    listing.gameVersion = "1.0";
    listing.mapName = map;
    listing.modeName = "ffa";
    listing.players = players;
    listing.maxPlayers = maxPlayers;
    listing.passworded = passworded;
    listing.hostName = "Host";
    listing.endpoints.push_back( CORE::CString( "udp://127.0.0.1:" ) + id );
    listing.backendName = "lan";
    listing.pingMs = pingMs;
    return listing;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFormLayoutTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.Layouts" );

    GUCEF_TESTFW_TESTCASE( "Layouts: The form that creates a lobby has two columns and a button row that overlaps nothing, from 800 x 600 up" )
    try
    {
        for ( UInt32 s=0; s<ScreenSizeCount; ++s )
        {
            const UInt32 screenWidth = ScreenSizes[ s ].width;
            const UInt32 screenHeight = ScreenSizes[ s ].height;
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( screenWidth, screenHeight, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;

            GAMEGUI::CCreateLobbyForm form( context );
            CORE::CString::StringVector services;
            services.push_back( "gucef" );
            form.SetServiceNames( services );
            form.SetTemplate( MakeCreateTemplate() );
            ASSERT_TRUE( form.Open() );
            context->RenderFrame( 16 );
            context->FlushChanges();

            GUI::CWidget* window = form.GetRootWidget();
            const GUI::CRect rect = window->GetRect();
            const CORE::CString where = " at " + SizeName( screenWidth, screenHeight );

            // nine tenths of the screen, within the limits, in the middle
            Int32 wantedWidth = (Int32) ( ( screenWidth * 9 + 5 ) / 10 );
            Int32 wantedHeight = (Int32) ( ( screenHeight * 9 + 5 ) / 10 );
            wantedWidth = wantedWidth < 560 ? 560 : ( wantedWidth > 2000 ? 2000 : wantedWidth );
            wantedHeight = wantedHeight < 460 ? 460 : ( wantedHeight > 1000 ? 1000 : wantedHeight );
            ASSERT_TRUE( rect.width == wantedWidth && rect.height == wantedHeight );
            ASSERT_TRUE( Inside( rect, GUI::CRect( 0, 0, (Int32) screenWidth, (Int32) screenHeight ) ) );
            Int32 centerX = rect.left + rect.width / 2, centerY = rect.top + rect.height / 2;
            ASSERT_TRUE( centerX >= (Int32) screenWidth / 2 - 1 && centerX <= (Int32) screenWidth / 2 + 1 && centerY >= (Int32) screenHeight / 2 - 1 && centerY <= (Int32) screenHeight / 2 + 1 );

            // two columns: the general fields on the left, the settings in a scroll panel on the right, the buttons below both; one that scrolls when narrow
            const bool narrow = wantedWidth < 700;
            GUI::CWidget* general = form.GetWidget( "GeneralColumn" );
            GUI::CWidget* holder = form.GetWidget( "SettingsHolder" );
            GUI::CWidget* strip = form.GetWidget( "ButtonRow" );
            GUI::CWidget* body = form.GetWidget( "Body" );
            ASSERT_TRUE( GUCEF_NULL != holder && GUCEF_NULL != strip );
            if ( GUCEF_NULL == holder || GUCEF_NULL == strip )
                return;
            GUI::CScrollPanel* scroll = AsScrollPanelOf( narrow ? body : holder );
            ASSERT_TRUE( GUCEF_NULL != scroll && 16 == scroll->GetIntProperty( "ScrollBarWidth" ) );
            if ( narrow )
            {
                ASSERT_TRUE( GUCEF_NULL == general && "VBox" == holder->GetType() );
                ASSERT_TRUE( body->GetRect().Bottom() <= strip->GetRect().top );
            }
            else
            {
                ASSERT_TRUE( GUCEF_NULL != general && "ScrollPanel" == holder->GetType() && GUCEF_NULL == body );
                if ( GUCEF_NULL == general )
                    return;
                ASSERT_TRUE( general->GetRect().Right() <= holder->GetRect().left );
                ASSERT_TRUE( holder->GetRect().Bottom() <= strip->GetRect().top && general->GetRect().Bottom() <= strip->GetRect().top );
                ASSERT_TRUE( holder->GetRect().height >= 200 && holder->GetRect().width >= 360 );
                // the general fields keep a sensible width, a wide window gives the rest to the settings
                ASSERT_TRUE( general->GetRect().width <= 560 );
            }
            ASSERT_TRUE( strip->GetRect().Bottom() <= rect.Bottom() && strip->GetRect().left >= rect.left && strip->GetRect().Right() <= rect.Right() );
            ASSERT_TRUE( Inside( form.GetWidget( "Create" )->GetRect(), strip->GetRect() ) && Inside( form.GetWidget( "Cancel" )->GetRect(), strip->GetRect() ) );
            ASSERT_TRUE( Inside( form.GetWidget( "Error" )->GetRect(), strip->GetRect() ) );
            ASSERT_FALSE( Intersects( form.GetWidget( "Create" )->GetRect(), form.GetWidget( "Cancel" )->GetRect() ) );
            ASSERT_FALSE( Intersects( form.GetWidget( "Error" )->GetRect(), form.GetWidget( "Create" )->GetRect() ) );
            ASSERT_TRUE( form.GetWidget( "Create" )->GetRect().Right() < form.GetWidget( "Cancel" )->GetRect().left );

            // the settings fill the scroll panel: more than fits scrolls; a wide panel puts the groups in columns, side by side
            GAMEGUI::CLobbySettingsPanelForm* panel = form.GetSettingsPanel();
            ASSERT_TRUE( GUCEF_NULL != panel );
            if ( GUCEF_NULL == panel )
                return;
            ASSERT_TRUE( 12 == panel->GetSettingCount() && panel->GetRootWidget()->GetParentWidget() == holder );
            const Int32 roomForSettings = ( narrow ? rect.width : holder->GetRect().width ) - 16;
            const UInt32 wantedColumns = roomForSettings / 420 < 1 ? 1 : ( roomForSettings / 420 > 3 ? 3 : (UInt32) ( roomForSettings / 420 ) );
            ASSERT_TRUE( ( wantedColumns >= 2 ) == ( GUCEF_NULL != panel->GetWidget( "Column1" ) ) && ( 3 == wantedColumns ) == ( GUCEF_NULL != panel->GetWidget( "Column2" ) ) );
            if ( GUCEF_NULL != scroll )
                ASSERT_TRUE( ( scroll->GetContentHeight() > scroll->GetViewportHeight() ) == scroll->IsScrollable() );

            CORE::CString problem = FindLayoutProblem( window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the form that creates a lobby" << where.C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );
            problem = FindBarEdgeProblem( window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );

            // scrolled to the end, still in order, and the last setting is in view
            if ( GUCEF_NULL != scroll )
            {
                scroll->SetScrollPosition( scroll->GetMaxScrollPosition() );
                context->FlushChanges();
                problem = FindLayoutProblem( window );
                ASSERT_TRUE( problem.IsNULLOrEmpty() );
                GUI::CWidget* last = ControlOf( panel, "drop" );
                ASSERT_TRUE( GUCEF_NULL != last && Inside( last->GetRect(), scroll->GetRect() ) );
                scroll->SetScrollPosition( 0 );
                context->FlushChanges();
            }

            // nothing is drawn outside of the window, the window is drawn
            IMAGE::TPixelMapPtr frame = CaptureFrame( context );
            ASSERT_TRUE( !frame.IsNULL() && OnlyBlackOutside( frame, rect ) );
            ASSERT_TRUE( CountLit( frame, rect ) > 500 );
            WriteShot( frame, "create-lobby-" + SizeName( screenWidth, screenHeight ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layouts: The form that creates a lobby scrolls as one column on a narrow screen and goes back to two columns, keeping what was filled in" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 640, 480, true );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CCreateLobbyForm form( context );
        CORE::CString::StringVector services;
        services.push_back( "gucef" );
        services.push_back( "other" );
        form.SetServiceNames( services );
        form.SetTemplate( MakeCreateTemplate() );
        ASSERT_TRUE( form.Open() );
        context->RenderFrame( 16 );
        context->FlushChanges();

        // the handlers of the buttons, the text fields, the navigation and the frames of the context
        const UInt32 subscriptions = GetEventSubscriptionCount( form );
        ASSERT_TRUE( 6 == subscriptions );

        // one column: the body scrolls as a whole, the settings are in it below the general fields
        GUI::CScrollPanel* body = AsScrollPanelOf( form.GetWidget( "Body" ) );
        ASSERT_TRUE( GUCEF_NULL != body && GUCEF_NULL == form.GetWidget( "GeneralColumn" ) );
        if ( GUCEF_NULL == body )
            return;
        GUI::CWidget* window = form.GetRootWidget();
        GUI::CWidget* strip = form.GetWidget( "ButtonRow" );
        ASSERT_TRUE( 576 == window->GetRect().width && body->GetRect().Bottom() <= strip->GetRect().top );
        ASSERT_TRUE( body->IsScrollable() && body->GetContentHeight() > body->GetViewportHeight() + 300 );
        ASSERT_TRUE( form.GetWidget( "Name" )->GetRect().Bottom() <= form.GetWidget( "SettingsLabel" )->GetRect().top );
        ASSERT_TRUE( form.GetWidget( "SettingsLabel" )->GetRect().Bottom() <= form.GetSettingsPanel()->GetRootWidget()->GetRect().top );
        ASSERT_TRUE( form.GetSettingsPanel()->GetRootWidget()->GetParentWidget() == form.GetWidget( "SettingsHolder" ) );
        ASSERT_TRUE( "VBox" == form.GetWidget( "SettingsHolder" )->GetType() );
        CORE::CString problem = FindLayoutProblem( window );
        if ( !problem.IsNULLOrEmpty() )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the narrow form that creates a lobby: " << problem.C_String() );
        ASSERT_TRUE( problem.IsNULLOrEmpty() );
        WriteShot( CaptureFrame( context ), "create-lobby-640x480-top" );

        // the whole content scrolls: the last setting comes into view, the buttons stay
        body->SetScrollPosition( body->GetMaxScrollPosition() );
        context->FlushChanges();
        GUI::CWidget* last = ControlOf( form.GetSettingsPanel(), "drop" );
        ASSERT_TRUE( GUCEF_NULL != last && Inside( last->GetRect(), body->GetRect() ) );
        ASSERT_TRUE( strip->GetRect().Bottom() <= window->GetRect().Bottom() && Inside( form.GetWidget( "Create" )->GetRect(), strip->GetRect() ) );
        problem = FindLayoutProblem( window );
        ASSERT_TRUE( problem.IsNULLOrEmpty() );
        WriteShot( CaptureFrame( context ), "create-lobby-640x480-end" );
        ASSERT_TRUE( OnlyBlackOutside( CaptureFrame( context ), window->GetRect() ) );

        // what the player fills in goes along when the screen grows and the form has two columns
        form.GetWidget( "Name" )->SetProperty( "Text", "Kept name" );
        form.GetWidget( "Password" )->SetProperty( "Text", "kept" );
        form.GetWidget( "MaxPlayers" )->SetProperty( "Value", "5" );
        form.GetWidget( "Spectators" )->SetBoolProperty( "Checked", false );
        ASSERT_TRUE( form.SelectService( "other" ) );
        form.GetSettingsPanel()->GetControl( "seed" )->ChangeValueByUser( "Value", "4242" );
        context->SetLogicalResolution( 1242, 952 );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "GeneralColumn" ) && GUCEF_NULL == form.GetWidget( "Body" ) );
        ASSERT_TRUE( form.IsOpen() && form.IsVisible() && context->IsModalRoot( form.GetRootWidget() ) );
        ASSERT_TRUE( "Kept name" == form.GetWidget( "Name" )->GetProperty( "Text" ) && "kept" == form.GetWidget( "Password" )->GetProperty( "Text" ) );
        ASSERT_TRUE( 5 == form.GetWidget( "MaxPlayers" )->GetIntProperty( "Value" ) && !form.GetWidget( "Spectators" )->GetBoolProperty( "Checked" ) );
        ASSERT_TRUE( "other" == form.GetServiceName() && "4242" == form.GetSettingsPanel()->GetValue( "seed" ) );
        ASSERT_TRUE( 1118 == form.GetRootWidget()->GetRect().width );
        problem = FindLayoutProblem( form.GetRootWidget() );
        ASSERT_TRUE( problem.IsNULLOrEmpty() );

        // and the other way
        context->SetLogicalResolution( 640, 480 );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "Body" ) && "Kept name" == form.GetWidget( "Name" )->GetProperty( "Text" ) && "4242" == form.GetSettingsPanel()->GetValue( "seed" ) );

        // the layouts that came and went left no subscriptions behind and the new widgets have theirs
        ASSERT_TRUE( subscriptions == GetEventSubscriptionCount( form ) );

        // a create from the layout that came out of all this
        MULTIPLAY::CLobbyCreateParams params;
        ASSERT_TRUE( form.GetCreateParams( params ) && "Kept name" == params.name && 5 == params.maxPlayers && !params.allowSpectators );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layouts: The settings panel scrolls itself in a plain placeholder and lets the scroll panel it is in do it, a long label gets lines and a row that is high enough" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 800, 600, true );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        MULTIPLAY::CLobbySettingsSchema schema;
        AddManySettings( schema );

        // a plain panel: the form brings a scroll panel of its own that fills it
        GUI::CWidget* plain = context->CreateWidget( "Panel", "Plain", GUCEF_NULL );
        plain->SetProperty( "Position", "0,10 0,10" );
        plain->SetProperty( "Size", "0,420 0,300" );
        context->ShowRoot( plain, false );
        GAMEGUI::CLobbySettingsPanelForm ownScroll;
        ownScroll.SetSchema( schema );
        ASSERT_TRUE( ownScroll.Build( plain ) );
        context->FlushChanges();
        GUI::CScrollPanel* scroll = AsScrollPanelOf( ownScroll.GetRootWidget() );
        ASSERT_TRUE( GUCEF_NULL != scroll && scroll->GetParentWidget() == plain && scroll->GetRect() == plain->GetRect() );
        if ( GUCEF_NULL == scroll )
            return;
        ASSERT_TRUE( scroll->IsScrollable() && scroll->GetContentHeight() > 2 * plain->GetRect().height );
        ASSERT_TRUE( FindLayoutProblem( plain ).IsNULLOrEmpty() );

        // the label that is too long: in lines, its row as high as they need, the control as high as always
        GUI::CWidget* drop = ownScroll.GetControl( "drop" );
        GUI::CWidget* dropLabel = ownScroll.GetWidget( "Label11" );
        ASSERT_TRUE( GUCEF_NULL != drop && GUCEF_NULL != dropLabel );
        if ( GUCEF_NULL == drop || GUCEF_NULL == dropLabel )
            return;
        ASSERT_TRUE( dropLabel->GetBoolProperty( "WordWrap" ) && dropLabel->GetParentWidget()->GetRect().height > 60 );
        ASSERT_TRUE( 32 == drop->GetRect().height && Inside( drop->GetRect(), dropLabel->GetParentWidget()->GetRect() ) );
        ASSERT_TRUE( dropLabel->GetRect().height == dropLabel->GetParentWidget()->GetRect().height );
        GUI::CWidget* scenarioLabel = ownScroll.GetWidget( "Label0" );
        ASSERT_TRUE( !scenarioLabel->GetBoolProperty( "WordWrap" ) && 32 == scenarioLabel->GetParentWidget()->GetRect().height );

        // wide enough, the label needs one line: the row follows the width of the panel
        plain->SetProperty( "Size", "0,780 0,300" );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( !dropLabel->GetBoolProperty( "WordWrap" ) || dropLabel->GetParentWidget()->GetRect().height <= 60 );
        ASSERT_TRUE( FindLayoutProblem( plain ).IsNULLOrEmpty() );
        plain->SetProperty( "Size", "0,420 0,300" );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        ASSERT_TRUE( dropLabel->GetBoolProperty( "WordWrap" ) && dropLabel->GetParentWidget()->GetRect().height > 60 );

        // a scroll panel as the placeholder: the form adds none, the groups are in that one
        GUI::CScrollPanel* host = static_cast< GUI::CScrollPanel* >( context->CreateWidget( "ScrollPanel", "Host", GUCEF_NULL ) );
        host->SetProperty( "Position", "0,450 0,10" );
        host->SetProperty( "Size", "0,330 0,300" );
        context->ShowRoot( host, false );
        GAMEGUI::CLobbySettingsPanelForm hosted;
        hosted.SetSchema( schema );
        ASSERT_TRUE( hosted.Build( host ) );
        context->FlushChanges();
        ASSERT_TRUE( hosted.GetRootWidget()->GetParentWidget() == host && "VBox" == hosted.GetRootWidget()->GetType() );
        ASSERT_TRUE( host->IsScrollable() && host->GetContentHeight() == hosted.GetRootWidget()->GetRect().height );
        ASSERT_TRUE( hosted.GetRootWidget()->GetRect().width == 330 - 16 );

        // a placeholder that is as high as its content: nothing scrolls, the placeholder is as high as the settings
        GUI::CWidget* flexible = context->CreateWidget( "VBox", "Flexible", GUCEF_NULL );
        flexible->SetProperty( "Position", "0,10 0,330" );
        flexible->SetProperty( "Size", "0,420 0,0" );
        flexible->SetBoolProperty( "AutoHeight", true );
        context->ShowRoot( flexible, false );
        GAMEGUI::CLobbySettingsPanelForm inFlexible;
        inFlexible.SetSchema( schema );
        ASSERT_TRUE( inFlexible.Build( flexible ) );
        context->FlushChanges();
        ASSERT_TRUE( "VBox" == inFlexible.GetRootWidget()->GetType() && flexible->GetRect().height >= inFlexible.GetRootWidget()->GetRect().height );
        ASSERT_TRUE( flexible->GetRect().height > 500 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layouts: The windows of the forms move by their title bar and fold up, the overlay of the turn timer does neither" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 1242, 952, true );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CCreateLobbyForm form( context );
        CORE::CString::StringVector services;
        services.push_back( "gucef" );
        form.SetServiceNames( services );
        form.SetTemplate( MakeCreateTemplate() );
        ASSERT_TRUE( form.Open() );
        context->FlushChanges();

        GUI::CWindow* window = AsWindowOf( form.GetRootWidget() );
        ASSERT_TRUE( GUCEF_NULL != window && window->IsMovable() && window->IsCollapsible() && !window->IsCollapsed() );
        if ( GUCEF_NULL == window )
            return;
        const GUI::CRect start = window->GetRect();
        GUI::CWidget* name = form.GetWidget( "Name" );
        const GUI::CRect nameStart = name->GetRect();

        // by the title bar
        context->InjectMouseMove( start.left + 200, start.top + 10 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseMove( start.left + 200 - 60, start.top + 10 + 25 );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect().left == start.left - 60 && window->GetRect().top == start.top + 25 );
        ASSERT_TRUE( name->GetRect().left == nameStart.left - 60 && name->GetRect().top == nameStart.top + 25 );
        ASSERT_TRUE( FindLayoutProblem( window ).IsNULLOrEmpty() );
        WriteShot( CaptureFrame( context ), "create-lobby-moved-1242x952" );

        // folded up: the title bar only, the buttons gone with the rest; unfolded: everything as it was
        window->ToggleCollapsed();
        context->FlushChanges();
        WriteShot( CaptureFrame( context ), "create-lobby-folded-1242x952" );
        ASSERT_TRUE( window->IsCollapsed() && window->GetRect().height == window->GetTitleBarHeight() && window->GetRect().top == start.top + 25 );
        ASSERT_TRUE( GUCEF_NULL == context->HitTest( window->GetRect().left + 30, window->GetRect().top + 100 ) );
        GUI::CWidget* hitCreate = context->HitTest( form.GetWidget( "Create" )->GetRect().left + 5, form.GetWidget( "Create" )->GetRect().top + 5 );
        ASSERT_TRUE( GUCEF_NULL == hitCreate );
        window->ToggleCollapsed();
        context->FlushChanges();
        ASSERT_TRUE( window->GetRect() == GUI::CRect( start.left - 60, start.top + 25, start.width, start.height ) );
        ASSERT_TRUE( context->HitTest( form.GetWidget( "Create" )->GetRect().left + 5, form.GetWidget( "Create" )->GetRect().top + 5 ) == form.GetWidget( "Create" ) );

        // the overlay that says whose turn it is is a box that stays where it is
        GAMEGUI::CTurnWaitOverlayForm overlay( context );
        ASSERT_TRUE( overlay.Open( "Waiting on player Bob to complete their turn", 12000, true ) );
        context->FlushChanges();
        GUI::CWindow* box = AsWindowOf( overlay.GetRootWidget() );
        ASSERT_TRUE( GUCEF_NULL != box && !box->IsMovable() && !box->IsCollapsible() && !box->GetBoolProperty( "TitleVisible" ) );
        if ( GUCEF_NULL != box )
        {
            GUI::CRect boxStart = box->GetRect();
            ASSERT_TRUE( boxStart.width == 480 && boxStart.height == 140 );
            context->InjectMouseMove( boxStart.left + 100, boxStart.top + 5 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->InjectMouseMove( boxStart.left + 20, boxStart.top + 60 );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->FlushChanges();
            ASSERT_TRUE( box->GetRect() == boxStart && !box->IsCollapsed() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layouts: The lobby, its settings panel with the vote and the buttons stay apart at every size" )
    try
    {
        for ( UInt32 s=0; s<ScreenSizeCount; ++s )
        {
            const UInt32 screenWidth = ScreenSizes[ s ].width;
            const UInt32 screenHeight = ScreenSizes[ s ].height;
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( screenWidth, screenHeight, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;

            CLobbyFixture fixture;
            AddManySettings( fixture.params.schema );
            ASSERT_TRUE( fixture.Setup() );
            // with a chat, as the lobby of a game has
            CTestChatService aliceChat;
            CTestChatService bobChat;
            MULTIPLAY::CPlayerIdentity aliceIdentity;
            aliceIdentity.userId = "alice-id";
            aliceIdentity.displayName = "Alice";
            MULTIPLAY::CPlayerIdentity bobIdentity;
            bobIdentity.userId = "bob-id";
            bobIdentity.displayName = "Bob";
            CORE::CFutureResult connected( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( aliceChat.Connect( aliceIdentity, connected ) && bobChat.Connect( bobIdentity, connected ) );

            GAMEGUI::CLobbyForm form( context );
            ASSERT_TRUE( form.Open() );
            form.SetChatService( &bobChat );
            form.SetLobby( fixture.bob );
            fixture.GetHost()->SetChat( "test-chat", "lobby-1" );
            ASSERT_TRUE( aliceChat.JoinChannel( "lobby-1" ) );
            ASSERT_TRUE( aliceChat.SendToChannel( "lobby-1", "Welcome to the lobby" ) && bobChat.SendToChannel( "lobby-1", "Thanks, who starts?" ) );
            context->RenderFrame( 16 );
            context->FlushChanges();
            ASSERT_TRUE( form.GetWidget( "ChatHolder" )->IsVisible() );

            GUI::CWidget* window = form.GetRootWidget();
            const GUI::CRect rect = window->GetRect();
            const CORE::CString where = SizeName( screenWidth, screenHeight );
            ASSERT_TRUE( Inside( rect, GUI::CRect( 0, 0, (Int32) screenWidth, (Int32) screenHeight ) ) && rect.width >= (Int32) ( screenWidth < 640 ? screenWidth : 640 ) );

            // one column that scrolls as a whole when narrow, else the settings in a scroll panel on the right
            const bool narrow = rect.width < 760;
            GUI::CWidget* holderWidget = form.GetWidget( "SettingsHolder" );
            GUI::CScrollPanel* holder = AsScrollPanelOf( narrow ? form.GetWidget( "Body" ) : holderWidget );
            ASSERT_TRUE( GUCEF_NULL != holder && GUCEF_NULL != holderWidget && form.GetSettingsPanel()->GetRootWidget()->GetParentWidget() == holderWidget );
            if ( GUCEF_NULL == holder )
                return;
            GUI::CWidget* strip = form.GetWidget( "ButtonRow" );
            ASSERT_TRUE( holder->GetRect().height >= 150 && ( holder->GetContentHeight() > holder->GetViewportHeight() ) == holder->IsScrollable() );
            ASSERT_TRUE( holder->GetRect().Bottom() <= strip->GetRect().top );
            if ( !narrow )
            {
                ASSERT_FALSE( Intersects( holder->GetRect(), form.GetWidget( "Slots" )->GetRect() ) );
                ASSERT_FALSE( Intersects( holder->GetRect(), form.GetWidget( "Spectators" )->GetRect() ) );
                ASSERT_FALSE( Intersects( form.GetWidget( "ChatHolder" )->GetRect(), form.GetWidget( "Faction" )->GetRect() ) );
                // a window that is very wide has room for columns of groups
                if ( rect.width >= 1700 )
                    ASSERT_TRUE( GUCEF_NULL != form.GetSettingsPanel()->GetWidget( "Column1" ) );
            }

            CORE::CString problem = FindLayoutProblem( window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the lobby at " << where.C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );
            problem = FindBarEdgeProblem( window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );
            holder->SetScrollPosition( holder->GetMaxScrollPosition() );
            context->FlushChanges();
            ASSERT_TRUE( FindLayoutProblem( window ).IsNULLOrEmpty() );
            holder->SetScrollPosition( 0 );
            context->FlushChanges();

            IMAGE::TPixelMapPtr frame = CaptureFrame( context );
            ASSERT_TRUE( OnlyBlackOutside( frame, rect ) && CountLit( frame, rect ) > 500 );
            WriteShot( frame, "lobby-" + where );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layouts: The wheel scrolls the form that creates a lobby and the lobby over every widget in view, the disabled ones too" )
    try
    {
        UInt32 triedTotal = 0;

        // the form that creates a lobby: one column when narrow, a column of settings on the right when wide; the password is not filled in for now
        for ( UInt32 s=0; s<2; ++s )
        {
            const UInt32 screenWidth = 0 == s ? 640 : 1242;
            const UInt32 screenHeight = 0 == s ? 480 : 700;
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( screenWidth, screenHeight, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;
            GAMEGUI::CCreateLobbyForm form( context );
            CORE::CString::StringVector services;
            services.push_back( "gucef" );
            form.SetServiceNames( services );
            form.SetTemplate( MakeCreateTemplate() );
            ASSERT_TRUE( form.Open() );
            context->RenderFrame( 16 );
            context->FlushChanges();
            form.GetWidget( "Password" )->SetEnabled( false );
            form.GetWidget( "Spectators" )->SetEnabled( false );
            context->FlushChanges();
            UInt32 tried = 0;
            CORE::CString problem = FindWheelProblem( context, form.GetRootWidget(), tried );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the form that creates a lobby at " << SizeName( screenWidth, screenHeight ).C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() && tried > 3 );
            triedTotal += tried;
        }

        // the lobby as a player who is not the boss has it: most settings are disabled for him
        for ( UInt32 s=0; s<2; ++s )
        {
            const UInt32 screenWidth = 0 == s ? 800 : 1242;
            const UInt32 screenHeight = 0 == s ? 600 : 700;
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( screenWidth, screenHeight, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;
            CLobbyFixture fixture;
            AddManySettings( fixture.params.schema );
            ASSERT_TRUE( fixture.Setup() );
            GAMEGUI::CLobbyForm form( context );
            ASSERT_TRUE( form.Open() );
            form.SetLobby( fixture.bob );
            context->RenderFrame( 16 );
            context->FlushChanges();
            UInt32 tried = 0;
            CORE::CString problem = FindWheelProblem( context, form.GetRootWidget(), tried );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the lobby at " << SizeName( screenWidth, screenHeight ).C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() && tried > 3 );
            triedTotal += tried;
        }
        ASSERT_TRUE( triedTotal > 20 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layouts: The profile dialog with the line for the accounts and its button keeps its controls apart from 600 x 900 to 5120 x 1440" )
    try
    {
        const SScreenSize profileSizes[] = { { 640, 480 }, { 800, 600 }, { 1024, 768 }, { 1242, 952 }, { 1920, 1080 }, { 3440, 1440 }, { 3840, 1080 }, { 5120, 1440 }, { 600, 900 } };
        for ( UInt32 s=0; s<9; ++s )
        {
            const UInt32 screenWidth = profileSizes[ s ].width;
            const UInt32 screenHeight = profileSizes[ s ].height;
            const CORE::CString where = SizeName( screenWidth, screenHeight );
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( screenWidth, screenHeight, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;
            // the profile dialog with its line for the accounts and the button that leads here
            {
                GAMEGUI::CPlayerProfileForm profile( context );
                profile.SetAccountsEnabled( true );
                MULTIPLAY::CPlayerIdentity identity;
                identity.userId = "5f2b9a3e-1111-4222-8333-444455556666";
                identity.displayName = "Alice";
                identity.accounts.SetAllowMultipleValues( true );
                identity.accounts.Set( "steam", "76561198000000000" );
                identity.accounts.Set( "google", "109876543210123456789" );
                profile.SetIdentity( identity );
                ASSERT_TRUE( profile.Open() );
                context->RenderFrame( 16 );
                context->FlushChanges();
                GUI::CWidget* profileWindow = profile.GetRootWidget();
                ASSERT_TRUE( Inside( profileWindow->GetRect(), GUI::CRect( 0, 0, (Int32) screenWidth, (Int32) screenHeight ) ) );
                ASSERT_TRUE( profileWindow->GetRect().width <= 480 && profileWindow->GetRect().height <= 340 );
                CORE::CString profileProblem = FindLayoutProblem( profileWindow );
                if ( !profileProblem.IsNULLOrEmpty() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the profile dialog with accounts at " << where.C_String() << ": " << profileProblem.C_String() );
                ASSERT_TRUE( profileProblem.IsNULLOrEmpty() );
                ASSERT_FALSE( Intersects( profile.GetWidget( "Accounts" )->GetRect(), profile.GetWidget( "AccountsText" )->GetRect() ) );
                IMAGE::TPixelMapPtr profileFrame = CaptureFrame( context );
                ASSERT_TRUE( OnlyBlackOutside( profileFrame, profileWindow->GetRect() ) && CountLit( profileFrame, profileWindow->GetRect() ) > 100 );
                WriteShot( profileFrame, "profile-accounts-" + where );
                profile.Close();
                context->FlushChanges();
            }

        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Layouts: The game browser and the dialogs keep their controls inside their window and the buttons apart" )
    try
    {
        for ( UInt32 s=0; s<ScreenSizeCount; ++s )
        {
            const UInt32 screenWidth = ScreenSizes[ s ].width;
            const UInt32 screenHeight = ScreenSizes[ s ].height;
            const CORE::CString where = SizeName( screenWidth, screenHeight );

            // the browser
            {
                GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( screenWidth, screenHeight, true );
                GUI::CGUIContext* context = ContextOf( contextPtr );
                ASSERT_TRUE( GUCEF_NULL != context );
                if ( GUCEF_NULL == context )
                    return;
                TTestListingProviderFactory factory;
                MULTIPLAY::CGameBrowser browser;
                browser.SetPingEnabled( false );
                MULTIPLAY::CIGameListingProviderPtr providerPtr = factory.Create();
                CTestListingProvider* provider = static_cast< CTestListingProvider* >( providerPtr.GetPointerAlways() );
                browser.AddProvider( providerPtr );
                browser.Start();
                provider->Put( MakeListing( "1", "Alpha", MULTIPLAY::CGameListingEntry::KIND_LOBBY, 2, 4, "dust", false, 30 ) );
                provider->Put( MakeListing( "2", "bravo", MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS, 6, 8, "ice", true, -1 ) );
                provider->Put( MakeListing( "3", "Charlie", MULTIPLAY::CGameListingEntry::KIND_LOBBY, 4, 4, "dust", false, 120 ) );

                GAMEGUI::CServerBrowserForm form( context );
                ASSERT_TRUE( form.Open() );
                form.SetBrowser( &browser );
                context->RenderFrame( 16 );
                context->FlushChanges();
                GUI::CWidget* window = form.GetRootWidget();
                ASSERT_TRUE( Inside( window->GetRect(), GUI::CRect( 0, 0, (Int32) screenWidth, (Int32) screenHeight ) ) );
                CORE::CString problem = FindLayoutProblem( window );
                if ( !problem.IsNULLOrEmpty() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the browser at " << where.C_String() << ": " << problem.C_String() );
                ASSERT_TRUE( problem.IsNULLOrEmpty() );
                problem = FindBarEdgeProblem( window );
                if ( !problem.IsNULLOrEmpty() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: " << problem.C_String() );
                ASSERT_TRUE( problem.IsNULLOrEmpty() );
                ASSERT_TRUE( form.GetWidget( "Listings" )->GetRect().height >= 100 );
                ASSERT_TRUE( form.GetWidget( "Listings" )->GetRect().Bottom() <= form.GetWidget( "ButtonRow" )->GetRect().top );
                IMAGE::TPixelMapPtr frame = CaptureFrame( context );
                ASSERT_TRUE( OnlyBlackOutside( frame, window->GetRect() ) && CountLit( frame, window->GetRect() ) > 500 );
                WriteShot( frame, "browser-" + where );
                browser.Stop();
            }

            // the dialogs
            {
                GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( screenWidth, screenHeight, true );
                GUI::CGUIContext* context = ContextOf( contextPtr );
                ASSERT_TRUE( GUCEF_NULL != context );
                if ( GUCEF_NULL == context )
                    return;
                GAMEGUI::CDirectConnectForm direct( context );
                GAMEGUI::CPlayerProfileForm profile( context );
                GAMEGUI::CVotePromptForm votePrompt( context );
                GAMEGUI::CConnectingForm connecting( context );
                GUI::CForm* forms[ 4 ] = { &direct, &profile, &votePrompt, &connecting };
                ASSERT_TRUE( direct.Open() && profile.Open() );
                MULTIPLAY::CLobbyVote vote;
                vote.id = "vote-7";
                vote.what = "setting";
                vote.proposer = "bob-id";
                vote.key = "speed";
                vote.value = "4";
                vote.yes = 1;
                vote.needed = 2;
                ASSERT_TRUE( votePrompt.Open( vote, "Bob" ) );
                MULTIPLAY::CLaunchDescription launch;
                launch.gameId = "testgame";
                launch.topology = MULTIPLAY::GAMETOPOLOGY_CLIENT_SERVER;
                launch.serverMemberId = "alice-id";
                launch.localMemberId = "bob-id";
                MULTIPLAY::CLaunchMember server;
                server.id = "alice-id";
                server.name = "Alice";
                server.endpoints.push_back( "udp://192.168.1.5:7777" );
                launch.members.push_back( server );
                ASSERT_TRUE( connecting.Begin( launch ) );
                context->RenderFrame( 16 );
                context->FlushChanges();
                const char* const names[ 4 ] = { "direct-connect", "player-profile", "vote", "connecting" };
                for ( UInt32 f=0; f<4; ++f )
                {
                    GUI::CWidget* window = forms[ f ]->GetRootWidget();
                    ASSERT_TRUE( Inside( window->GetRect(), GUI::CRect( 0, 0, (Int32) screenWidth, (Int32) screenHeight ) ) );
                    ASSERT_TRUE( window->GetRect().width <= 520 && window->GetRect().height <= 300 );
                    CORE::CString problem = FindLayoutProblem( window );
                    if ( !problem.IsNULLOrEmpty() )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the dialog " << names[ f ] << " at " << where.C_String() << ": " << problem.C_String() );
                    ASSERT_TRUE( problem.IsNULLOrEmpty() );
                }

                // each alone on the screen
                direct.Close();
                profile.Close();
                votePrompt.Close();
                connecting.Cancel();
                ASSERT_TRUE( direct.Open() );
                context->RenderFrame( 16 );
                IMAGE::TPixelMapPtr frame = CaptureFrame( context );
                ASSERT_TRUE( OnlyBlackOutside( frame, direct.GetRootWidget()->GetRect() ) && CountLit( frame, direct.GetRootWidget()->GetRect() ) > 100 );
                WriteShot( frame, "direct-connect-" + where );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
