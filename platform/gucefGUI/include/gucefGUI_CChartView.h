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

#ifndef GUCEF_GUI_CCHARTVIEW_H
#define GUCEF_GUI_CCHARTVIEW_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CDRAWSURFACE_H
#include "gucefGUI_CDrawSurface.h"
#define GUCEF_GUI_CDRAWSURFACE_H
#endif /* GUCEF_GUI_CDRAWSURFACE_H ? */

#ifndef GUCEF_GUI_CCHARTSERIES_H
#include "gucefGUI_CChartSeries.h"
#define GUCEF_GUI_CCHARTSERIES_H
#endif /* GUCEF_GUI_CCHARTSERIES_H ? */

#ifndef GUCEF_GUI_CCHARTAXIS_H
#include "gucefGUI_CChartAxis.h"
#define GUCEF_GUI_CCHARTAXIS_H
#endif /* GUCEF_GUI_CCHARTAXIS_H ? */

#ifndef GUCEF_GUI_CCHARTDECIMATOR_H
#include "gucefGUI_CChartDecimator.h"
#define GUCEF_GUI_CCHARTDECIMATOR_H
#endif /* GUCEF_GUI_CCHARTDECIMATOR_H ? */

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
 *  A chart of series over a time axis (milliseconds since the Unix epoch, UTC) or a numeric x axis: lines,
 *  filled areas and bars (stacked per stack group), a value axis with chosen ticks (linear or log), a legend,
 *  and a crosshair with a tooltip that follows the pointer. The chart draws itself with the commands of a
 *  draw surface, so every driver that shows a draw surface shows it.
 *
 *  The drawing is rebuilt only when something changed (data, settings, properties, theme values, size,
 *  crosshair); large series are reduced to at most four points per pixel column (CChartDecimator), and the
 *  buffers of a rebuild are reused. Lookups (GetNearestPoints()) work on the data alone, without a window.
 *
 *  Properties (beyond those of DrawSurface): Title, Font, TextColour, AxisTextColour, BackgroundColour,
 *  AxisColour, GridColour, CrosshairColour, TooltipColour, SeriesColours, CrosshairVisible, LegendVisible,
 *  LegendPosition, XAxisType, LogScale, IncludeZero, ValuePrefix, ValueSuffix, Decimals.
 */
class GUCEF_GUI_PUBLIC_CPP CChartView : public CDrawSurface
{
    public:

    /** The crosshair moved to another x, or went away; not routed, no data: read GetCrosshairPoints() */
    static const CORE::CEvent CrosshairChangedEvent;

    static void RegisterEvents( void );

    /** What a lookup finds for one series */
    struct SSeriesPoint
    {
        UInt32 seriesIndex;
        UInt32 pointIndex;
        Int64 x;
        Float64 value;          /**< the value of the point */
        Float64 plotValue;      /**< where it is drawn: the top of its segment for a stacked bar, else the value */
    };
    typedef struct SSeriesPoint TSeriesPoint;
    typedef GUCEF::vector< TSeriesPoint > TSeriesPointVector;
    typedef GUCEF::vector< CChartSeries > TSeriesVector;

    CChartView( void );

    virtual ~CChartView();

    /** Adds a series at the end; returns its index, which is its place in the legend and in the palette */
    UInt32 AddSeries( const CString& name, CChartSeries::TSeriesType type );

    UInt32 GetSeriesCount( void ) const;

    /** NULL for a bad index; the pointer is good until the next AddSeries() or RemoveSeries() */
    CChartSeries* GetSeries( UInt32 index );

    const CChartSeries* GetSeries( UInt32 index ) const;

    /** -1 when there is none */
    Int32 FindSeries( const CString& name ) const;

    bool RemoveSeries( UInt32 index );

    void RemoveAllSeries( void );

    /** CChartSeries::SetData() of the series with that index */
    bool SetSeriesData( UInt32 index, const Int64* x, const Float64* values, UInt32 count );

    bool SetSeriesData( UInt32 index, const UInt64* x, const Float64* values, UInt32 count );

    /** The colour a series is drawn in: its own colour, else the palette (SeriesColours) colour of its index */
    CColour GetSeriesColour( UInt32 index ) const;

    CChartAxis& GetXAxis( void );

    const CChartAxis& GetXAxis( void ) const;

    CChartAxis& GetValueAxis( void );

    const CChartAxis& GetValueAxis( void ) const;

    /** The drawing is out of date: it is rebuilt before the next frame is drawn */
    void Invalidate( void );

    bool IsDirty( void ) const;

    /** Rebuilds the drawing now for the size of the widget */
    void Rebuild( void );

    /** Rebuilds the drawing now for a given size, for a chart that has no context or layout */
    void Rebuild( Int32 width, Int32 height );

    /** The plot area of the last rebuild, relative to the widget */
    const CRect& GetPlotRect( void ) const;

    /** The pixel column of x in the last rebuild, relative to the widget */
    Int32 XToPixel( Int64 x ) const;

    Int64 PixelToX( Int32 localX ) const;

    /** The pixel row of a value in the last rebuild, relative to the widget; a value a log scale cannot show gives the bottom of the plot */
    Int32 ValueToPixel( Float64 value ) const;

    Float64 PixelToValue( Int32 localY ) const;

    /**
     *  The point of every visible, non empty series nearest to x, in series order; 'points' is cleared first.
     *  Points without a value (NaN) are skipped. Needs no window, no context and no rebuild. Returns the number of points.
     */
    UInt32 GetNearestPoints( Int64 x, TSeriesPointVector& points ) const;

    /** The same for a pixel column of the last rebuild */
    UInt32 GetNearestPointsAtPixel( Int32 localX, TSeriesPointVector& points ) const;

    /** Puts the crosshair at the x of the point nearest to 'x' over all visible series; false when there are no points */
    bool SetCrosshairX( Int64 x );

    /** The same for a pixel column of the last rebuild; false outside the plot or without points */
    bool SetCrosshairPixel( Int32 localX );

    void ClearCrosshair( void );

    bool HasCrosshair( void ) const;

    Int64 GetCrosshairX( void ) const;

    /** The nearest points at the crosshair */
    const TSeriesPointVector& GetCrosshairPoints( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name this widget type is registered under: what GetType() of the widgets of this class says */
    static const char* GetRegisteredTypeName( void ) { return "ChartView"; }

    static CWidget* Create( void );

    protected:

    virtual void OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool NeedsUpdate( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnUpdate( UInt32 deltaInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void AdjustPlacement( CRect& rect, const CRect& area, bool sizeFromArea ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** The values of the look a rebuild used, to see whether the theme changed them */
    struct SLook
    {
        CColour textColour;
        CColour axisTextColour;
        CColour axisColour;
        CColour gridColour;
        CColour crosshairColour;
        CColour tooltipColour;
        CString font;
        CString title;
        CString seriesColours;
        CString valuePrefix;
        CString valueSuffix;
        CString xAxisType;
        CString legendPosition;
        Int32 decimals;
        bool logScale;
        bool includeZero;
        bool legendVisible;
        bool crosshairVisible;
    };

    /** Totals of one stack group per x */
    struct SStackColumn
    {
        Int64 x;
        Float64 positive;
        Float64 negative;
    };
    typedef GUCEF::vector< SStackColumn > TStackColumnVector;
    typedef GUCEF::vector< Int32 > TInt32Vector;
    typedef GUCEF::vector< CColour > TColourVector;

    bool LookChanged( void ) const;
    void CaptureLook( void );
    UInt32 SeriesRevisionSum( void ) const;

    /** The properties onto the axes and the palette */
    void ApplyProperties( void );

    Int32 LineHeight( void ) const;
    Int32 TextWidth( const CString& text ) const;

    /** The x range of the visible series (with half a bar slot on each side when there are bars) */
    bool GetDataXRange( Float64& minX, Float64& maxX, Float64& barSlot ) const;

    /** The value range of the visible series in the x range, stacked totals included */
    bool GetDataValueRange( Float64 minX, Float64 maxX, Float64& minValue, Float64& maxValue );

    /** The index of the stack group of a bar series and the number of groups */
    UInt32 GetBarGroup( UInt32 seriesIndex, UInt32& groupCount ) const;

    /** Collects the x values of the bar series of a group into m_stack, totals zeroed */
    void PrepareStack( UInt32 group );

    /** Where a point is drawn: for a stacked bar the top of its segment */
    Float64 GetPlotValue( UInt32 seriesIndex, UInt32 pointIndex ) const;

    void DrawSeriesArea( UInt32 seriesIndex, const CColour& colour, Int32 baselineY );
    void DrawSeriesLine( UInt32 seriesIndex, const CColour& colour );
    void DrawBars( Float64 barSlot );
    void DrawLegend( Int32 top, Int32 width );
    void DrawCrosshair( void );

    /** Fills m_keep with the indexes of a series to draw: the visible ones, reduced when there are many */
    void CollectDrawnPoints( const CChartSeries& series );

    Int32 LegendHeight( Int32 width ) const;
    UInt32 VisibleSeriesCount( void ) const;

    void UpdateCrosshairPoints( void );

    CChartView( const CChartView& src );
    CChartView& operator=( const CChartView& src );

    private:

    TSeriesVector m_series;
    CChartAxis m_xAxis;
    CChartAxis m_valueAxis;
    bool m_dirty;
    Int32 m_builtWidth;
    Int32 m_builtHeight;
    UInt32 m_builtSeriesRevision;
    CRect m_plotRect;
    Float64 m_barSlot;
    bool m_hasCrosshair;
    Int64 m_crosshairX;
    TSeriesPointVector m_crosshairPoints;
    SLook m_look;
    TColourVector m_palette;
    CString m_paletteText;
    CChartDecimator::TIndexVector m_keep;
    TStackColumnVector m_stack;
    TInt32Vector m_columnTop;
    TInt32Vector m_columnBottom;
    Int32 m_lineHeight;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CCHARTVIEW_H ? */
