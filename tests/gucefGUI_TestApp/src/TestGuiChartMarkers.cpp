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

#include <math.h>

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

const Int64 Day = 86400000;

GUI::CColour
PixelAt( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 y )
{
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() ) + ( (size_t) y * pixels->GetWidthInPixels() + x ) * 4;
    return GUI::CColour( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
}

bool
SameRgb( const GUI::CColour& a, const GUI::CColour& b )
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

/** Pixels of a rectangle (clipped to the picture) in the colour */
Int32
CountInRect( const IMAGE::TPixelMapPtr& pixels, const GUI::CRect& rect, const GUI::CColour& colour )
{
    Int32 count = 0;
    for ( Int32 y=rect.top; y<rect.Bottom(); ++y )
        for ( Int32 x=rect.left; x<rect.Right(); ++x )
            if ( x >= 0 && y >= 0 && x < (Int32) pixels->GetWidthInPixels() && y < (Int32) pixels->GetHeightInPixels() && SameRgb( PixelAt( pixels, x, y ), colour ) )
                ++count;
    return count;
}

/** Records the marker events of one chart: the activations with their data, the selection changes */
class CMarkerEventLog : public CORE::CObserver
{
    public:

    UInt32 activations;
    UInt32 selectionChanges;
    Int32 lastIndex;
    CORE::CString lastId;

    explicit CMarkerEventLog( GUI::CChartView* chart )
        : CORE::CObserver(), activations( 0 ), selectionChanges( 0 ), lastIndex( -2 ), lastId()
    {
        TCallback activated( this, &CMarkerEventLog::OnActivated );
        SubscribeTo( chart, GUI::CChartView::MarkerActivatedEvent, activated );
        TCallback selection( this, &CMarkerEventLog::OnSelectionChanged );
        SubscribeTo( chart, GUI::CChartView::MarkerSelectionChangedEvent, selection );
    }

    virtual ~CMarkerEventLog() { UnsubscribeAllFromObserver(); }

    private:

    typedef CORE::CTEventHandlerFunctor< CMarkerEventLog > TCallback;

    void OnActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const GUI::CItemEventData* item = CORE::CICloneable::CloneableAs< GUI::CItemEventData >( eventData );
        if ( GUCEF_NULL == item || GUI::EVENTPHASE_TARGET != item->phase )
            return;
        ++activations;
        lastIndex = item->index;
        lastId = item->item;
    }

    void OnSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++selectionChanges;
    }
};

GUI::CGUIContext*
ContextOf( GUI::TGuiContextPtr& contextPtr )
{
    return contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
}

GUI::CWidget*
MakeScreen( GUI::CGUIContext* context )
{
    GUI::CWidget* root = context->CreateWidget( "Panel", "Screen", GUCEF_NULL );
    root->SetProperty( "Size", "1,0 1,0" );
    context->ShowRoot( root, false );
    return root;
}

GUI::CChartView*
MakeChart( GUI::CGUIContext* context, GUI::CWidget* parent, const char* name, Int32 left, Int32 top, Int32 width, Int32 height )
{
    GUI::CChartView* chart = GUI::CWidget::As< GUI::CChartView >( context->CreateWidget( "ChartView", name, parent ) );
    if ( GUCEF_NULL == chart )
        return GUCEF_NULL;
    chart->SetPosition( GUI::CUVector( GUI::CUDim( 0.0f, left ), GUI::CUDim( 0.0f, top ) ) );
    chart->SetSize( GUI::CUVector( GUI::CUDim( 0.0f, width ), GUI::CUDim( 0.0f, height ) ) );
    return chart;
}

bool
Capture( GUI::CGUIContext* context, IMAGE::TPixelMapPtr& pixels )
{
    context->RenderFrame( 16 );
    return context->CaptureFrame( pixels ) && !pixels.IsNULL();
}

/** Thirty daily points from 2024-06-01 rising by 1 a day from 100, as a line */
UInt32
AddPriceLine( GUI::CChartView& chart, Int64 start )
{
    GUCEF::vector< Int64 > x;
    GUCEF::vector< Float64 > values;
    for ( UInt32 i=0; i<30; ++i )
    {
        x.push_back( start + (Int64) i * Day );
        values.push_back( 100.0 + (Float64) i );
    }
    const UInt32 index = chart.AddSeries( "Price", GUI::CChartSeries::SERIESTYPE_LINE );
    chart.SetSeriesData( index, &x[ 0 ], &values[ 0 ], 30 );
    return index;
}

GUI::CChartMarker
Marker( const char* id, Int64 x, Int32 series, const char* styleClass, const char* layer, const char* label, const char* tooltip )
{
    GUI::CChartMarker marker( id, x );
    marker.SetSeriesIndex( series );
    marker.SetStyleClass( styleClass );
    marker.SetLayer( layer );
    marker.SetLabel( label );
    marker.SetTooltip( tooltip );
    return marker;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiChartMarkerTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.ChartMarkers" );

    GUCEF_TESTFW_TESTCASE( "Chart markers: a marker sits above its point of the series or in the lane at the bottom; outside the range, of a hidden layer or a hidden series it is not shown" )
    try
    {
        GUI::CChartView chart;
        const Int64 start = GUI::CChartAxis::MakeTime( 2024, 6, 1 );
        const UInt32 price = AddPriceLine( chart, start );
        ASSERT_TRUE( 0 == chart.AddMarker( Marker( "split", start + 10 * Day, (Int32) price, "split", "", "2:1", "Split 2 for 1" ) ) );
        ASSERT_TRUE( 1 == chart.AddMarker( Marker( "lane", start + 20 * Day, -1, "", "", "", "In the lane" ) ) );
        ASSERT_TRUE( 2 == chart.AddMarker( Marker( "before", start - 5 * Day, (Int32) price, "", "", "", "" ) ) );
        ASSERT_TRUE( 3 == chart.AddMarker( Marker( "dividend", start + 15 * Day, (Int32) price, "", "dividends", "", "Dividend" ) ) );
        ASSERT_TRUE( 4 == chart.AddMarker( Marker( "top", start + 29 * Day, (Int32) price, "", "", "", "At the highest point" ) ) );
        ASSERT_TRUE( 5 == chart.GetMarkerCount() && 3 == chart.FindMarker( "dividend" ) && -1 == chart.FindMarker( "none" ) );
        // a second line keeps the x range when the price line is hidden
        GUCEF::vector< Int64 > otherX;
        GUCEF::vector< Float64 > otherValues;
        otherX.push_back( start );
        otherX.push_back( start + 29 * Day );
        otherValues.push_back( 100.0 );
        otherValues.push_back( 101.0 );
        const UInt32 other = chart.AddSeries( "Other", GUI::CChartSeries::SERIESTYPE_LINE );
        chart.SetSeriesData( other, &otherX[ 0 ], &otherValues[ 0 ], 2 );
        // a fixed value range: 129 is at the top of the plot
        chart.GetValueAxis().SetRange( 100.0, 129.0 );
        chart.SetMarkerLayerVisible( "dividends", false );
        ASSERT_FALSE( chart.IsMarkerLayerVisible( "dividends" ) );
        ASSERT_TRUE( chart.IsMarkerLayerVisible( "" ) );
        chart.Rebuild( 600, 300 );

        // above the point: the centre of the shape MarkerSize pixels above it
        GUI::CRect rect;
        ASSERT_TRUE( chart.GetMarkerRect( 0, rect ) );
        ASSERT_TRUE( GUI::CChartView::MarkerSize == rect.width && GUI::CChartView::MarkerSize == rect.height );
        ASSERT_TRUE( chart.XToPixel( start + 10 * Day ) == rect.left + rect.width / 2 );
        ASSERT_TRUE( chart.ValueToPixel( 110.0 ) - GUI::CChartView::MarkerSize == rect.top + rect.height / 2 );
        // the lane: above the bottom axis
        ASSERT_TRUE( chart.GetMarkerRect( 1, rect ) );
        ASSERT_TRUE( chart.GetPlotRect().Bottom() - 1 - GUI::CChartView::MarkerSize == rect.top + rect.height / 2 );
        // before the first point, a hidden layer: not shown
        ASSERT_FALSE( chart.IsMarkerShown( 2 ) );
        ASSERT_FALSE( chart.IsMarkerShown( 3 ) );
        // at the top of the plot there is no room above: below the point
        ASSERT_TRUE( chart.GetMarkerRect( 4, rect ) );
        ASSERT_TRUE( chart.ValueToPixel( 129.0 ) + GUI::CChartView::MarkerSize == rect.top + rect.height / 2 );

        // the layer shown again, the series hidden: the markers of the series go, the lane stays
        chart.SetMarkerLayerVisible( "dividends", true );
        chart.GetSeries( price )->SetVisible( false );
        chart.Rebuild( 600, 300 );
        ASSERT_FALSE( chart.IsMarkerShown( 0 ) );
        ASSERT_FALSE( chart.IsMarkerShown( 3 ) );
        ASSERT_TRUE( chart.IsMarkerShown( 1 ) );
        chart.GetSeries( price )->SetVisible( true );
        chart.Rebuild( 600, 300 );
        ASSERT_TRUE( chart.IsMarkerShown( 0 ) && chart.IsMarkerShown( 3 ) );

        // removing a marker keeps the others with their index moved down
        ASSERT_TRUE( chart.RemoveMarker( 2 ) );
        ASSERT_TRUE( 4 == chart.GetMarkerCount() && 2 == chart.FindMarker( "dividend" ) );
        ASSERT_FALSE( chart.RemoveMarker( 10 ) );
        chart.RemoveAllMarkers();
        ASSERT_TRUE( 0 == chart.GetMarkerCount() && GUCEF_NULL == chart.GetMarker( 0 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart markers: hit testing takes the shape with its slop and the nearest of overlapping markers; shape and colour come from the marker, its style class or EmphasisColour" )
    try
    {
        GUI::CChartView chart;
        const Int64 start = GUI::CChartAxis::MakeTime( 2024, 6, 1 );
        const UInt32 price = AddPriceLine( chart, start );
        chart.AddMarker( Marker( "a", start + 10 * Day, -1, "split", "", "", "" ) );
        chart.AddMarker( Marker( "b", start + 10 * Day + Day / 4, -1, "", "", "", "" ) );     // overlaps a, its centre 5 pixels to the right at most
        chart.AddMarker( Marker( "c", start + 20 * Day, (Int32) price, "unknown style", "", "", "" ) );
        GUI::CChartMarker own = Marker( "d", start + 25 * Day, (Int32) price, "split", "", "", "" );
        own.SetShape( GUI::CChartMarker::SHAPE_CIRCLE );
        own.SetColour( GUI::CColour( 10, 200, 10, 255 ) );
        chart.AddMarker( own );

        // nothing is hit before a rebuild
        ASSERT_TRUE( -1 == chart.HitTestMarker( 100, 100 ) );
        chart.SetMarkerStyle( "split", GUI::CChartMarker::SHAPE_TRIANGLE_UP, GUI::CColour( 200, 60, 60, 255 ) );
        chart.Rebuild( 600, 300 );

        GUI::CRect a, b, c;
        ASSERT_TRUE( chart.GetMarkerRect( 0, a ) && chart.GetMarkerRect( 1, b ) && chart.GetMarkerRect( 2, c ) );
        ASSERT_TRUE( b.left > a.left && b.left - a.left < GUI::CChartView::MarkerSize );
        // the centre of each overlapping marker hits that marker; between them the nearer one
        ASSERT_TRUE( 0 == chart.HitTestMarker( a.left + a.width / 2 - 2, a.top + a.height / 2 ) );
        ASSERT_TRUE( 1 == chart.HitTestMarker( b.left + b.width / 2 + 2, b.top + b.height / 2 ) );
        // the slop around a shape hits, beyond it nothing
        ASSERT_TRUE( 2 == chart.HitTestMarker( c.left - GUI::CChartView::MarkerHitSlop, c.top ) );
        ASSERT_TRUE( 2 == chart.HitTestMarker( c.Right() + GUI::CChartView::MarkerHitSlop - 1, c.Bottom() + GUI::CChartView::MarkerHitSlop - 1 ) );
        ASSERT_TRUE( -1 == chart.HitTestMarker( c.left - GUI::CChartView::MarkerHitSlop - 1, c.top ) );
        ASSERT_TRUE( -1 == chart.HitTestMarker( c.left, c.Bottom() + GUI::CChartView::MarkerHitSlop ) );

        // the look: the style class, the marker's own, the default
        GUI::CChartMarker::TShape shape = GUI::CChartMarker::SHAPE_DEFAULT;
        GUI::CColour colour;
        ASSERT_TRUE( chart.GetMarkerStyle( "split", shape, colour ) && GUI::CChartMarker::SHAPE_TRIANGLE_UP == shape );
        ASSERT_TRUE( GUI::CChartMarker::SHAPE_TRIANGLE_UP == chart.GetMarkerShape( 0 ) && SameRgb( GUI::CColour( 200, 60, 60, 255 ), chart.GetMarkerColour( 0 ) ) );
        ASSERT_TRUE( GUI::CChartMarker::SHAPE_DIAMOND == chart.GetMarkerShape( 1 ) && SameRgb( chart.GetColourProperty( GUI::EWidgetProperty_EmphasisColour ), chart.GetMarkerColour( 1 ) ) );
        ASSERT_TRUE( GUI::CChartMarker::SHAPE_DIAMOND == chart.GetMarkerShape( 2 ) );
        ASSERT_TRUE( GUI::CChartMarker::SHAPE_CIRCLE == chart.GetMarkerShape( 3 ) && SameRgb( GUI::CColour( 10, 200, 10, 255 ), chart.GetMarkerColour( 3 ) ) );
        chart.RemoveMarkerStyle( "split" );
        ASSERT_FALSE( chart.GetMarkerStyle( "split", shape, colour ) );
        ASSERT_TRUE( GUI::CChartMarker::SHAPE_DIAMOND == chart.GetMarkerShape( 0 ) );
        ASSERT_TRUE( CORE::CString( "triangle-up" ) == GUI::CChartMarker::ShapeToString( GUI::CChartMarker::SHAPE_TRIANGLE_UP ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart markers: the pointer over a marker shows its tooltip in place of the crosshair's, a click selects it and routes MarkerActivatedEvent with its index and id" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            context->RegisterFont( "Test", MakeTestFont( 6, 10 ) );
            GUI::CWidget* screen = MakeScreen( context );
            GUI::CChartView* chart = MakeChart( context, screen, "Chart", 20, 20, 600, 320 );
            ASSERT_TRUE( GUCEF_NULL != chart );
            if ( GUCEF_NULL != chart )
            {
                chart->SetProperty( GUI::EWidgetProperty_Font, "Test" );
                chart->SetColourProperty( GUI::EWidgetProperty_TooltipColour, GUI::CColour( 38, 38, 36, 255 ) );
                const Int64 start = GUI::CChartAxis::MakeTime( 2024, 6, 1 );
                const UInt32 price = AddPriceLine( *chart, start );
                chart->AddMarker( Marker( "dividend:2024-06-06", start + 5 * Day, (Int32) price, "", "", "", "Dividend $0.25" ) );
                chart->AddMarker( Marker( "split:2024-06-11", start + 10 * Day, (Int32) price, "", "", "2:1", "Split 2 for 1\nSources: a, b" ) );
                CMarkerEventLog log( chart );

                IMAGE::TPixelMapPtr pixels;
                ASSERT_TRUE( Capture( context, pixels ) );
                const Int32 ox = chart->GetRect().left;
                const Int32 oy = chart->GetRect().top;
                GUI::CRect split;
                ASSERT_TRUE( chart->GetMarkerRect( 1, split ) );
                const GUI::CColour tooltip = chart->GetColourProperty( GUI::EWidgetProperty_TooltipColour );
                // the left margin of the box of a tooltip to the right of the marker, where no text is
                const GUI::CRect beside( ox + split.Right() + 9, oy + split.top - 3, 4, 20 );
                ASSERT_TRUE( 0 == CountInRect( pixels, beside, tooltip ) );

                // over the marker: hovered, its tooltip to its right, no box of the crosshair at the top of the plot
                context->InjectMouseMove( ox + split.left + split.width / 2, oy + split.top + split.height / 2 );
                ASSERT_TRUE( Capture( context, pixels ) );
                ASSERT_TRUE( 1 == chart->GetHoveredMarker() && chart->HasCrosshair() );
                ASSERT_TRUE( CountInRect( pixels, beside, tooltip ) >= 70 );
                const Int32 crosshairBoxX = ox + chart->XToPixel( chart->GetCrosshairX() ) + 14;
                const GUI::CRect plot = chart->GetPlotRect();
                ASSERT_TRUE( split.top - 4 > plot.top + 10 );
                ASSERT_FALSE( SameRgb( tooltip, PixelAt( pixels, crosshairBoxX, oy + plot.top + 6 ) ) );

                // the pointer on the line away from the markers: the crosshair's tooltip is back
                context->InjectMouseMove( ox + chart->XToPixel( start + 20 * Day ), oy + plot.top + plot.height / 2 );
                ASSERT_TRUE( Capture( context, pixels ) );
                ASSERT_TRUE( -1 == chart->GetHoveredMarker() );
                ASSERT_TRUE( SameRgb( tooltip, PixelAt( pixels, ox + chart->XToPixel( chart->GetCrosshairX() ) + 14, oy + plot.top + 6 ) ) );
                ASSERT_TRUE( 0 == log.activations );

                // a click on the marker: selected, activated with its index and id
                context->InjectMouseMove( ox + split.left + 1, oy + split.top + split.height / 2 );
                context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
                context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
                ASSERT_TRUE( 1 == log.activations && 1 == log.lastIndex && "split:2024-06-11" == log.lastId );
                ASSERT_TRUE( 1 == chart->GetSelectedMarker() && 1 == log.selectionChanges );
                // a click beside every marker activates nothing
                context->InjectMouseMove( ox + chart->XToPixel( start + 25 * Day ), oy + plot.Bottom() - 4 );
                context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
                context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
                ASSERT_TRUE( 1 == log.activations && 1 == chart->GetSelectedMarker() );
                // leaving the chart: no marker hovered
                context->InjectMouseMove( 700, 500 );
                ASSERT_TRUE( -1 == chart->GetHoveredMarker() );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart markers: Tab gives a focusable chart the keys; Right, Left, Home and End select the shown markers in the order of x, Enter activates, the ring and the tooltip of the selection are drawn" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            context->RegisterFont( "Test", MakeTestFont( 6, 10 ) );
            GUI::CWidget* screen = MakeScreen( context );
            GUI::CChartView* chart = MakeChart( context, screen, "Chart", 20, 20, 600, 320 );
            ASSERT_TRUE( GUCEF_NULL != chart );
            if ( GUCEF_NULL != chart )
            {
                chart->SetProperty( GUI::EWidgetProperty_Font, "Test" );
                chart->SetColourProperty( GUI::EWidgetProperty_TooltipColour, GUI::CColour( 38, 38, 36, 255 ) );
                chart->SetBoolProperty( GUI::EWidgetProperty_Focusable, true );
                const Int64 start = GUI::CChartAxis::MakeTime( 2024, 6, 1 );
                const UInt32 price = AddPriceLine( *chart, start );
                // added out of the order of time: the keys follow the time
                chart->AddMarker( Marker( "late", start + 20 * Day, (Int32) price, "", "", "", "Late" ) );
                chart->AddMarker( Marker( "early", start + 4 * Day, (Int32) price, "", "", "", "Early" ) );
                chart->AddMarker( Marker( "middle", start + 12 * Day, -1, "", "", "", "Middle" ) );
                chart->AddMarker( Marker( "hidden", start + 16 * Day, (Int32) price, "", "dividends", "", "Hidden" ) );
                chart->SetMarkerLayerVisible( "dividends", false );
                CMarkerEventLog log( chart );
                IMAGE::TPixelMapPtr pixels;
                ASSERT_TRUE( Capture( context, pixels ) );
                const Int32 ox = chart->GetRect().left;
                const Int32 oy = chart->GetRect().top;
                const GUI::CColour selection = chart->GetColourProperty( GUI::EWidgetProperty_SelectionColour );

                // Tab: the chart has the focus, its plot a frame
                ASSERT_FALSE( chart->HasFocus() );
                context->InjectKey( INPUT::KEYCODE_TAB, true, 0 );
                context->InjectKey( INPUT::KEYCODE_TAB, false, 0 );
                ASSERT_TRUE( chart->HasFocus() );
                ASSERT_TRUE( Capture( context, pixels ) );
                const GUI::CRect plot = chart->GetPlotRect();
                ASSERT_TRUE( SameRgb( selection, PixelAt( pixels, ox + plot.left - 1, oy + plot.top + plot.height / 2 ) ) );

                // Right: the earliest; Right again in the order of time; the hidden layer is passed over
                context->InjectKey( INPUT::KEYCODE_RIGHT, true, 0 );
                ASSERT_TRUE( 1 == chart->GetSelectedMarker() );
                context->InjectKey( INPUT::KEYCODE_RIGHT, true, 0 );
                ASSERT_TRUE( 2 == chart->GetSelectedMarker() );
                context->InjectKey( INPUT::KEYCODE_RIGHT, true, 0 );
                ASSERT_TRUE( 0 == chart->GetSelectedMarker() );
                // at the last one Right stays, and the key does not move the focus
                context->InjectKey( INPUT::KEYCODE_RIGHT, true, 0 );
                ASSERT_TRUE( 0 == chart->GetSelectedMarker() && chart->HasFocus() );
                context->InjectKey( INPUT::KEYCODE_LEFT, true, 0 );
                ASSERT_TRUE( 2 == chart->GetSelectedMarker() );
                context->InjectKey( INPUT::KEYCODE_HOME, true, 0 );
                ASSERT_TRUE( 1 == chart->GetSelectedMarker() );
                context->InjectKey( INPUT::KEYCODE_END, true, 0 );
                ASSERT_TRUE( 0 == chart->GetSelectedMarker() );
                ASSERT_TRUE( 6 == log.selectionChanges && 0 == log.activations );

                // the ring around the selection and its tooltip, while the chart has the focus
                ASSERT_TRUE( Capture( context, pixels ) );
                GUI::CRect late;
                ASSERT_TRUE( chart->GetMarkerRect( 0, late ) );
                ASSERT_TRUE( SameRgb( selection, PixelAt( pixels, ox + late.left - 3, oy + late.top + late.height / 2 ) ) );
                ASSERT_TRUE( CountInRect( pixels, GUI::CRect( ox + late.Right() + 9, oy + late.top - 3, 4, 20 ), chart->GetColourProperty( GUI::EWidgetProperty_TooltipColour ) ) >= 70 );

                // Enter activates the selected marker
                context->InjectKey( INPUT::KEYCODE_RETURN, true, 0 );
                ASSERT_TRUE( 1 == log.activations && 0 == log.lastIndex && "late" == log.lastId );
                // a gamepad accept does the same
                context->InjectNavigation( GUI::NAVIGATION_ACCEPT );
                ASSERT_TRUE( 2 == log.activations );

                // the layer of the selection hidden: no selection
                ASSERT_TRUE( chart->SelectMarkerById( "middle" ) && 2 == chart->GetSelectedMarker() );
                ASSERT_FALSE( chart->SelectMarkerById( "hidden" ) );
                chart->SetMarkerLayerVisible( "dividends", true );
                ASSERT_TRUE( chart->SelectMarkerById( "hidden" ) && 3 == chart->GetSelectedMarker() );
                chart->SetMarkerLayerVisible( "dividends", false );
                ASSERT_TRUE( -1 == chart->GetSelectedMarker() );

                // replacing the markers keeps the selection on the same id, else drops it
                ASSERT_TRUE( chart->SelectMarkerById( "middle" ) );
                GUI::CChartView::TMarkerVector markers;
                markers.push_back( Marker( "new", start + 2 * Day, (Int32) price, "", "", "", "New" ) );
                markers.push_back( Marker( "middle", start + 12 * Day, -1, "", "", "", "Middle" ) );
                chart->SetMarkers( markers );
                ASSERT_TRUE( 1 == chart->GetSelectedMarker() && 2 == chart->GetMarkerCount() );
                markers.pop_back();
                chart->SetMarkers( markers );
                ASSERT_TRUE( -1 == chart->GetSelectedMarker() );

                // Right after a replacement starts at the first marker again
                context->InjectKey( INPUT::KEYCODE_RIGHT, true, 0 );
                ASSERT_TRUE( 0 == chart->GetSelectedMarker() );
                chart->RemoveAllMarkers();
                ASSERT_TRUE( -1 == chart->GetSelectedMarker() );
                // without markers the arrows are not the chart's
                ASSERT_FALSE( chart->SelectNextMarker( true ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart markers: the software reference renderer draws the shapes in their colours, a label above and nothing of a hidden layer" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            context->RegisterFont( "Test", MakeTestFont( 6, 10 ) );
            GUI::CWidget* screen = MakeScreen( context );
            GUI::CChartView* chart = MakeChart( context, screen, "Chart", 10, 10, 640, 360 );
            ASSERT_TRUE( GUCEF_NULL != chart );
            if ( GUCEF_NULL != chart )
            {
                chart->SetProperty( GUI::EWidgetProperty_Font, "Test" );
                chart->SetBoolProperty( GUI::EWidgetProperty_CrosshairVisible, false );
                const Int64 start = GUI::CChartAxis::MakeTime( 2024, 6, 1 );
                const UInt32 price = AddPriceLine( *chart, start );
                const GUI::CColour red( 220, 40, 40, 255 );
                const GUI::CColour green( 40, 200, 80, 255 );
                chart->SetMarkerStyle( "split", GUI::CChartMarker::SHAPE_TRIANGLE_UP, red );
                chart->AddMarker( Marker( "split", start + 8 * Day, (Int32) price, "split", "", "3:2", "Split 3 for 2" ) );
                GUI::CChartMarker square = Marker( "square", start + 16 * Day, -1, "", "", "", "" );
                square.SetShape( GUI::CChartMarker::SHAPE_SQUARE );
                square.SetColour( green );
                chart->AddMarker( square );
                chart->AddMarker( Marker( "diamond", start + 22 * Day, (Int32) price, "", "", "", "" ) );
                GUI::CChartMarker hidden = Marker( "hidden", start + 12 * Day, -1, "", "dividends", "", "" );
                hidden.SetShape( GUI::CChartMarker::SHAPE_SQUARE );
                hidden.SetColour( green );
                chart->AddMarker( hidden );
                chart->SetMarkerLayerVisible( "dividends", false );

                IMAGE::TPixelMapPtr pixels;
                ASSERT_TRUE( Capture( context, pixels ) );
                const Int32 ox = chart->GetRect().left;
                const Int32 oy = chart->GetRect().top;
                GUI::CRect triangle, box, diamond;
                ASSERT_TRUE( chart->GetMarkerRect( 0, triangle ) && chart->GetMarkerRect( 1, box ) && chart->GetMarkerRect( 2, diamond ) );
                const Int32 size = GUI::CChartView::MarkerSize;
                // the triangle: its tip one pixel wide at the top, its base the whole width at the bottom
                const Int32 tcx = ox + triangle.left + size / 2;
                ASSERT_TRUE( SameRgb( red, PixelAt( pixels, tcx, oy + triangle.top ) ) );
                ASSERT_FALSE( SameRgb( red, PixelAt( pixels, tcx - 2, oy + triangle.top ) ) );
                ASSERT_TRUE( SameRgb( red, PixelAt( pixels, ox + triangle.left, oy + triangle.Bottom() - 1 ) ) );
                ASSERT_TRUE( SameRgb( red, PixelAt( pixels, ox + triangle.Right() - 1, oy + triangle.Bottom() - 1 ) ) );
                // the square fills its rectangle
                ASSERT_TRUE( size * size == CountInRect( pixels, GUI::CRect( ox + box.left, oy + box.top, size, size ), green ) );
                // the diamond in EmphasisColour: the middle row whole, the corners empty
                const GUI::CColour emphasis = chart->GetColourProperty( GUI::EWidgetProperty_EmphasisColour );
                ASSERT_TRUE( size == CountInRect( pixels, GUI::CRect( ox + diamond.left, oy + diamond.top + size / 2, size, 1 ), emphasis ) );
                ASSERT_FALSE( SameRgb( emphasis, PixelAt( pixels, ox + diamond.left, oy + diamond.top ) ) );
                // the label above the triangle: ink of the text colour
                const GUI::CColour text = chart->GetColourProperty( GUI::EWidgetProperty_TextColour );
                ASSERT_TRUE( CountInRect( pixels, GUI::CRect( tcx - 12, oy + triangle.top - 14, 24, 13 ), text ) > 10 );
                // the hidden layer: no green beside the square
                ASSERT_FALSE( chart->IsMarkerShown( 3 ) );
                const Int32 hiddenX = ox + chart->XToPixel( start + 12 * Day );
                ASSERT_TRUE( 0 == CountInRect( pixels, GUI::CRect( hiddenX - size, oy + box.top, 2 * size, size ), green ) );
                chart->SetMarkerLayerVisible( "dividends", true );
                ASSERT_TRUE( Capture( context, pixels ) );
                ASSERT_TRUE( CountInRect( pixels, GUI::CRect( hiddenX - size, oy + box.top, 2 * size, size ), green ) >= size * size );
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
