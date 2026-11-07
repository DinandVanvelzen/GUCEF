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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <algorithm>

#include "gucefGUI_CGUIContext.h"
#include "gucefGUI_CChartPalette.h"

#include "gucefGUI_CChartView.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline bool IsNaN( Float64 value ) { return value != value; }

inline bool IsFinite( Float64 value ) { return value == value && ( value - value ) == 0.0; }

/** Space around everything */
const Int32 Padding = 8;

/** Space between the value labels and the plot, and between the plot and the x labels */
const Int32 LabelGap = 6;

/** Assumed text metrics when there is no context or font to measure with */
const Int32 FallbackCharWidth = 7;
const Int32 FallbackLineHeight = 14;

/** Legend: the swatch, the gap after it and the gap after an entry */
const Int32 SwatchSize = 10;
const Int32 SwatchGap = 4;
const Int32 LegendEntryGap = 16;

/** Alpha of the fill of an area series */
const UInt8 AreaAlpha = 72;

/** Part of its share of the bar slot a bar fills */
const Float64 BarFill = 0.8;

const Int64 MsPerDay = 86400000;

/** Rounds to the nearest integer, halves away from zero */
inline Int32
RoundToInt( Float64 value )
{
    if ( value > 1e9 )
        return 1000000000;
    if ( value < -1e9 )
        return -1000000000;
    return (Int32) floor( value + 0.5 );
}

/**
 *  Cuts a line to a rectangle (Liang-Barsky); false when nothing of it is inside. Only lines that leave the rectangle
 *  are changed, so the lines inside keep their exact end points.
 */
bool
ClipLine( Float64& x0, Float64& y0, Float64& x1, Float64& y1, Float64 left, Float64 top, Float64 right, Float64 bottom )
{
    Float64 t0 = 0.0, t1 = 1.0;
    const Float64 dx = x1 - x0, dy = y1 - y0;
    const Float64 p[ 4 ] = { -dx, dx, -dy, dy };
    const Float64 q[ 4 ] = { x0 - left, right - x0, y0 - top, bottom - y0 };
    for ( UInt32 i=0; i<4; ++i )
    {
        if ( 0.0 == p[ i ] )
        {
            if ( q[ i ] < 0.0 )
                return false;
            continue;
        }
        Float64 r = q[ i ] / p[ i ];
        if ( p[ i ] < 0.0 )
        {
            if ( r > t1 )
                return false;
            if ( r > t0 )
                t0 = r;
        }
        else
        {
            if ( r < t0 )
                return false;
            if ( r < t1 )
                t1 = r;
        }
    }
    const Float64 nx0 = x0 + t0 * dx, ny0 = y0 + t0 * dy;
    const Float64 nx1 = x0 + t1 * dx, ny1 = y0 + t1 * dy;
    x0 = nx0; y0 = ny0; x1 = nx1; y1 = ny1;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CChartView::CrosshairChangedEvent = "GUCEF::GUI::CChartView::CrosshairChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CChartView::RegisterEvents( void )
{GUCEF_TRACE;

    CrosshairChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CChartView::CChartView( void )
    : CDrawSurface( "ChartView" )
    , m_series()
    , m_xAxis()
    , m_valueAxis()
    , m_dirty( true )
    , m_builtWidth( -1 )
    , m_builtHeight( -1 )
    , m_builtSeriesRevision( 0 )
    , m_plotRect()
    , m_barSlot( 0.0 )
    , m_hasCrosshair( false )
    , m_crosshairX( 0 )
    , m_crosshairPoints()
    , m_look()
    , m_palette()
    , m_paletteText()
    , m_keep()
    , m_stack()
    , m_columnTop()
    , m_columnBottom()
    , m_lineHeight( FallbackLineHeight )
{GUCEF_TRACE;

    RegisterEvents();
    m_xAxis.SetKind( CChartAxis::AXISKIND_TIME );
    m_xAxis.SetIncludeZero( false );
    m_xAxis.SetExtendToTicks( false );
    m_look.decimals = -1;
    m_look.logScale = false;
    m_look.includeZero = true;
    m_look.legendVisible = true;
    m_look.crosshairVisible = true;
}

/*-------------------------------------------------------------------------*/

CChartView::~CChartView()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWidget*
CChartView::Create( void )
{GUCEF_TRACE;

    return GUCEF_NEW CChartView();
}

/*-------------------------------------------------------------------------*/

const CString&
CChartView::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CChartView";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt32
CChartView::AddSeries( const CString& name, CChartSeries::TSeriesType type )
{GUCEF_TRACE;

    m_series.push_back( CChartSeries( name, type ) );
    Invalidate();
    return (UInt32) m_series.size() - 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CChartView::GetSeriesCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_series.size();
}

/*-------------------------------------------------------------------------*/

CChartSeries*
CChartView::GetSeries( UInt32 index )
{GUCEF_TRACE;

    return index < (UInt32) m_series.size() ? &m_series[ index ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CChartSeries*
CChartView::GetSeries( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_series.size() ? &m_series[ index ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

Int32
CChartView::FindSeries( const CString& name ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
        if ( m_series[ i ].GetName() == name )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CChartView::RemoveSeries( UInt32 index )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_series.size() )
        return false;
    m_series.erase( m_series.begin() + index );
    UpdateCrosshairPoints();
    Invalidate();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CChartView::RemoveAllSeries( void )
{GUCEF_TRACE;

    m_series.clear();
    if ( m_hasCrosshair )
        ClearCrosshair();
    Invalidate();
}

/*-------------------------------------------------------------------------*/

bool
CChartView::SetSeriesData( UInt32 index, const Int64* x, const Float64* values, UInt32 count )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_series.size() || !m_series[ index ].SetData( x, values, count ) )
        return false;
    UpdateCrosshairPoints();
    Invalidate();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChartView::SetSeriesData( UInt32 index, const UInt64* x, const Float64* values, UInt32 count )
{GUCEF_TRACE;

    if ( index >= (UInt32) m_series.size() || !m_series[ index ].SetData( x, values, count ) )
        return false;
    UpdateCrosshairPoints();
    Invalidate();
    return true;
}

/*-------------------------------------------------------------------------*/

CColour
CChartView::GetSeriesColour( UInt32 index ) const
{GUCEF_TRACE;

    if ( index < (UInt32) m_series.size() && m_series[ index ].HasColour() )
        return m_series[ index ].GetColour();

    // the palette of the property; parsed again only when its text changed
    const CString& text = GetPropertyRef( EWidgetProperty_SeriesColours );
    if ( text != m_paletteText || m_palette.empty() )
    {
        CChartView* self = const_cast< CChartView* >( this );
        self->m_palette.clear();
        CChartPalette::FromStringList( GetStringListProperty( EWidgetProperty_SeriesColours ), self->m_palette );
        self->m_paletteText = text;
    }
    const TColourVector& palette = m_palette.empty() ? CChartPalette::GetDarkPalette() : m_palette;
    return palette[ index % (UInt32) palette.size() ];
}

/*-------------------------------------------------------------------------*/

CChartAxis&
CChartView::GetXAxis( void )
{GUCEF_TRACE;

    return m_xAxis;
}

/*-------------------------------------------------------------------------*/

const CChartAxis&
CChartView::GetXAxis( void ) const
{GUCEF_TRACE;

    return m_xAxis;
}

/*-------------------------------------------------------------------------*/

CChartAxis&
CChartView::GetValueAxis( void )
{GUCEF_TRACE;

    return m_valueAxis;
}

/*-------------------------------------------------------------------------*/

const CChartAxis&
CChartView::GetValueAxis( void ) const
{GUCEF_TRACE;

    return m_valueAxis;
}

/*-------------------------------------------------------------------------*/

void
CChartView::Invalidate( void )
{GUCEF_TRACE;

    m_dirty = true;
}

/*-------------------------------------------------------------------------*/

bool
CChartView::IsDirty( void ) const
{GUCEF_TRACE;

    return m_dirty || SeriesRevisionSum() != m_builtSeriesRevision;
}

/*-------------------------------------------------------------------------*/

UInt32
CChartView::SeriesRevisionSum( void ) const
{GUCEF_TRACE;

    // the count is in it, so taking a series away is a change too
    UInt32 sum = (UInt32) m_series.size() * 2654435761u;
    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
        sum += m_series[ i ].GetRevision() * ( 2 * i + 1 );
    return sum;
}

/*-------------------------------------------------------------------------*/

bool
CChartView::LookChanged( void ) const
{GUCEF_TRACE;

    // compares in place: no text is copied
    return GetColourProperty( EWidgetProperty_TextColour ) != m_look.textColour ||
           GetColourProperty( EWidgetProperty_AxisTextColour ) != m_look.axisTextColour ||
           GetColourProperty( EWidgetProperty_AxisColour ) != m_look.axisColour ||
           GetColourProperty( EWidgetProperty_GridColour ) != m_look.gridColour ||
           GetColourProperty( EWidgetProperty_CrosshairColour ) != m_look.crosshairColour ||
           GetColourProperty( EWidgetProperty_TooltipColour ) != m_look.tooltipColour ||
           GetPropertyRef( EWidgetProperty_Font ) != m_look.font ||
           GetPropertyRef( EWidgetProperty_Title ) != m_look.title ||
           GetPropertyRef( EWidgetProperty_SeriesColours ) != m_look.seriesColours ||
           GetPropertyRef( EWidgetProperty_ValuePrefix ) != m_look.valuePrefix ||
           GetPropertyRef( EWidgetProperty_ValueSuffix ) != m_look.valueSuffix ||
           GetPropertyRef( EWidgetProperty_XAxisType ) != m_look.xAxisType ||
           GetPropertyRef( EWidgetProperty_LegendPosition ) != m_look.legendPosition ||
           GetIntProperty( EWidgetProperty_Decimals ) != m_look.decimals ||
           GetBoolProperty( EWidgetProperty_LogScale ) != m_look.logScale ||
           GetBoolProperty( EWidgetProperty_IncludeZero ) != m_look.includeZero ||
           GetBoolProperty( EWidgetProperty_LegendVisible ) != m_look.legendVisible ||
           GetBoolProperty( EWidgetProperty_CrosshairVisible ) != m_look.crosshairVisible;
}

/*-------------------------------------------------------------------------*/

void
CChartView::CaptureLook( void )
{GUCEF_TRACE;

    m_look.textColour = GetColourProperty( EWidgetProperty_TextColour );
    m_look.axisTextColour = GetColourProperty( EWidgetProperty_AxisTextColour );
    m_look.axisColour = GetColourProperty( EWidgetProperty_AxisColour );
    m_look.gridColour = GetColourProperty( EWidgetProperty_GridColour );
    m_look.crosshairColour = GetColourProperty( EWidgetProperty_CrosshairColour );
    m_look.tooltipColour = GetColourProperty( EWidgetProperty_TooltipColour );
    // assigned only when different, so a steady rebuild copies no text
    if ( GetPropertyRef( EWidgetProperty_Font ) != m_look.font )
        m_look.font = GetPropertyRef( EWidgetProperty_Font );
    if ( GetPropertyRef( EWidgetProperty_Title ) != m_look.title )
        m_look.title = GetPropertyRef( EWidgetProperty_Title );
    if ( GetPropertyRef( EWidgetProperty_SeriesColours ) != m_look.seriesColours )
        m_look.seriesColours = GetPropertyRef( EWidgetProperty_SeriesColours );
    if ( GetPropertyRef( EWidgetProperty_ValuePrefix ) != m_look.valuePrefix )
        m_look.valuePrefix = GetPropertyRef( EWidgetProperty_ValuePrefix );
    if ( GetPropertyRef( EWidgetProperty_ValueSuffix ) != m_look.valueSuffix )
        m_look.valueSuffix = GetPropertyRef( EWidgetProperty_ValueSuffix );
    if ( GetPropertyRef( EWidgetProperty_XAxisType ) != m_look.xAxisType )
        m_look.xAxisType = GetPropertyRef( EWidgetProperty_XAxisType );
    if ( GetPropertyRef( EWidgetProperty_LegendPosition ) != m_look.legendPosition )
        m_look.legendPosition = GetPropertyRef( EWidgetProperty_LegendPosition );
    m_look.decimals = GetIntProperty( EWidgetProperty_Decimals );
    m_look.logScale = GetBoolProperty( EWidgetProperty_LogScale );
    m_look.includeZero = GetBoolProperty( EWidgetProperty_IncludeZero );
    m_look.legendVisible = GetBoolProperty( EWidgetProperty_LegendVisible );
    m_look.crosshairVisible = GetBoolProperty( EWidgetProperty_CrosshairVisible );
}

/*-------------------------------------------------------------------------*/

void
CChartView::ApplyProperties( void )
{GUCEF_TRACE;

    m_xAxis.SetKind( GetPropertyRef( EWidgetProperty_XAxisType ) == "Numeric" ? CChartAxis::AXISKIND_NUMERIC : CChartAxis::AXISKIND_TIME );
    m_xAxis.SetScale( CChartAxis::SCALE_LINEAR );
    m_valueAxis.SetKind( CChartAxis::AXISKIND_NUMERIC );
    m_valueAxis.SetScale( GetBoolProperty( EWidgetProperty_LogScale ) ? CChartAxis::SCALE_LOG : CChartAxis::SCALE_LINEAR );
    m_valueAxis.SetIncludeZero( GetBoolProperty( EWidgetProperty_IncludeZero ) );
    m_valueAxis.SetValuePrefix( GetPropertyRef( EWidgetProperty_ValuePrefix ) );
    m_valueAxis.SetValueSuffix( GetPropertyRef( EWidgetProperty_ValueSuffix ) );
    m_valueAxis.SetDecimals( GetIntProperty( EWidgetProperty_Decimals ) );
}

/*-------------------------------------------------------------------------*/

Int32
CChartView::LineHeight( void ) const
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    if ( GUCEF_NULL == context )
        return FallbackLineHeight;
    const CString& font = GetPropertyRef( EWidgetProperty_Font );
    if ( context->GetBitmapFont( font ).IsNULL() )
        return FallbackLineHeight;
    Int32 height = context->GetLineHeight( font );
    return height > 0 ? height : FallbackLineHeight;
}

/*-------------------------------------------------------------------------*/

Int32
CChartView::TextWidth( const CString& text ) const
{GUCEF_TRACE;

    CGUIContext* context = GetContext();
    Int32 width = 0, height = 0;
    if ( GUCEF_NULL != context && context->MeasureText( GetPropertyRef( EWidgetProperty_Font ), text, 0, width, height ) )
        return width;
    return (Int32) text.Length() * FallbackCharWidth;
}

/*-------------------------------------------------------------------------*/

UInt32
CChartView::VisibleSeriesCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
        if ( m_series[ i ].IsVisible() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

UInt32
CChartView::GetBarGroup( UInt32 seriesIndex, UInt32& groupCount ) const
{GUCEF_TRACE;

    // groups in the order of their first visible bar series; a series without a stack group is a group of its own
    UInt32 group = 0;
    groupCount = 0;
    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
    {
        const CChartSeries& series = m_series[ i ];
        if ( !series.IsVisible() || CChartSeries::SERIESTYPE_BAR != series.GetType() )
            continue;
        bool firstOfGroup = true;
        if ( !series.GetStackGroup().IsNULLOrEmpty() )
        {
            for ( UInt32 j=0; j<i; ++j )
            {
                const CChartSeries& earlier = m_series[ j ];
                if ( earlier.IsVisible() && CChartSeries::SERIESTYPE_BAR == earlier.GetType() && earlier.GetStackGroup() == series.GetStackGroup() )
                {
                    firstOfGroup = false;
                    break;
                }
            }
        }
        if ( firstOfGroup )
            ++groupCount;
        if ( i == seriesIndex )
            group = groupCount - 1;
        if ( i == seriesIndex && !firstOfGroup )
        {
            // the group of the first series with the same stack group
            for ( UInt32 j=0; j<i; ++j )
            {
                const CChartSeries& earlier = m_series[ j ];
                if ( earlier.IsVisible() && CChartSeries::SERIESTYPE_BAR == earlier.GetType() && earlier.GetStackGroup() == series.GetStackGroup() )
                {
                    UInt32 ignored = 0;
                    group = GetBarGroup( j, ignored );
                    break;
                }
            }
        }
    }
    return group;
}

/*-------------------------------------------------------------------------*/

void
CChartView::PrepareStack( UInt32 group )
{GUCEF_TRACE;

    // m_stack holds every x of the group once, sorted, with zero totals; the vector keeps its capacity
    m_stack.clear();
    UInt32 groupCount = 0;
    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
    {
        const CChartSeries& series = m_series[ i ];
        if ( !series.IsVisible() || CChartSeries::SERIESTYPE_BAR != series.GetType() || GetBarGroup( i, groupCount ) != group )
            continue;
        for ( UInt32 p=0; p<series.GetCount(); ++p )
        {
            SStackColumn column;
            column.x = series.GetX( p );
            column.positive = 0.0;
            column.negative = 0.0;
            m_stack.push_back( column );
        }
    }
    struct SLess
    {
        static bool Less( const SStackColumn& a, const SStackColumn& b ) { return a.x < b.x; }
        static bool Equal( const SStackColumn& a, const SStackColumn& b ) { return a.x == b.x; }
    };
    std::sort( m_stack.begin(), m_stack.end(), &SLess::Less );
    m_stack.erase( std::unique( m_stack.begin(), m_stack.end(), &SLess::Equal ), m_stack.end() );
}

/*-------------------------------------------------------------------------*/

Float64
CChartView::GetPlotValue( UInt32 seriesIndex, UInt32 pointIndex ) const
{GUCEF_TRACE;

    const CChartSeries& series = m_series[ seriesIndex ];
    const Float64 value = series.GetValue( pointIndex );
    if ( CChartSeries::SERIESTYPE_BAR != series.GetType() || series.GetStackGroup().IsNULLOrEmpty() || !IsFinite( value ) )
        return value;

    // the values of the earlier series of the stack with the same sign at the same x are below this one
    const Int64 x = series.GetX( pointIndex );
    Float64 base = 0.0;
    for ( UInt32 i=0; i<seriesIndex; ++i )
    {
        const CChartSeries& earlier = m_series[ i ];
        if ( !earlier.IsVisible() || CChartSeries::SERIESTYPE_BAR != earlier.GetType() || earlier.GetStackGroup() != series.GetStackGroup() )
            continue;
        for ( UInt32 p=earlier.LowerBound( x ); p<earlier.GetCount() && earlier.GetX( p ) == x; ++p )
        {
            Float64 other = earlier.GetValue( p );
            if ( IsFinite( other ) && ( other >= 0.0 ) == ( value >= 0.0 ) )
                base += other;
        }
    }
    // equal x values earlier in the same series stack too
    for ( UInt32 p=series.LowerBound( x ); p<pointIndex; ++p )
    {
        Float64 other = series.GetValue( p );
        if ( IsFinite( other ) && ( other >= 0.0 ) == ( value >= 0.0 ) )
            base += other;
    }
    return base + value;
}

/*-------------------------------------------------------------------------*/

bool
CChartView::GetDataXRange( Float64& minX, Float64& maxX, Float64& barSlot ) const
{GUCEF_TRACE;

    bool found = false;
    bool bars = false;
    Int64 smallestStep = 0;
    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
    {
        const CChartSeries& series = m_series[ i ];
        Int64 low = 0, high = 0;
        if ( !series.IsVisible() || !series.GetXRange( low, high ) )
            continue;
        if ( !found || (Float64) low < minX )
            minX = (Float64) low;
        if ( !found || (Float64) high > maxX )
            maxX = (Float64) high;
        found = true;
        if ( CChartSeries::SERIESTYPE_BAR == series.GetType() )
        {
            bars = true;
            const Int64* x = series.GetXData();
            for ( UInt32 p=1; p<series.GetCount(); ++p )
            {
                Int64 step = x[ p ] - x[ p-1 ];
                if ( step > 0 && ( 0 == smallestStep || step < smallestStep ) )
                    smallestStep = step;
            }
        }
    }
    barSlot = 0.0;
    if ( bars )
    {
        if ( 0 == smallestStep )
            smallestStep = CChartAxis::AXISKIND_TIME == m_xAxis.GetKind() ? MsPerDay : 1;
        barSlot = (Float64) smallestStep;
        if ( m_xAxis.IsAutoRange() )
        {
            minX -= barSlot / 2.0;
            maxX += barSlot / 2.0;
        }
    }
    return found;
}

/*-------------------------------------------------------------------------*/

bool
CChartView::GetDataValueRange( Float64 minX, Float64 maxX, Float64& minValue, Float64& maxValue )
{GUCEF_TRACE;

    const bool logScale = CChartAxis::SCALE_LOG == m_valueAxis.GetScale();
    const Int64 lowX = (Int64) floor( minX );
    const Int64 highX = (Int64) ceil( maxX );
    bool found = false;
    bool bars = false;

    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
    {
        const CChartSeries& series = m_series[ i ];
        if ( !series.IsVisible() || CChartSeries::SERIESTYPE_BAR == series.GetType() )
        {
            if ( series.IsVisible() )
                bars = true;
            continue;
        }
        const Float64* values = series.GetValueData();
        UInt32 first = series.LowerBound( lowX );
        UInt32 last = series.UpperBound( highX );
        for ( UInt32 p=first; p<last; ++p )
        {
            Float64 value = values[ p ];
            if ( !IsFinite( value ) || ( logScale && value <= 0.0 ) )
                continue;
            if ( !found || value < minValue )
                minValue = value;
            if ( !found || value > maxValue )
                maxValue = value;
            found = true;
        }
    }

    if ( bars )
    {
        UInt32 groupCount = 0;
        GetBarGroup( 0, groupCount );
        for ( UInt32 group=0; group<groupCount; ++group )
        {
            PrepareStack( group );
            for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
            {
                const CChartSeries& series = m_series[ i ];
                UInt32 ignored = 0;
                if ( !series.IsVisible() || CChartSeries::SERIESTYPE_BAR != series.GetType() || GetBarGroup( i, ignored ) != group )
                    continue;
                UInt32 first = series.LowerBound( lowX );
                UInt32 last = series.UpperBound( highX );
                for ( UInt32 p=first; p<last; ++p )
                {
                    Float64 value = series.GetValue( p );
                    if ( !IsFinite( value ) )
                        continue;
                    SStackColumn key;
                    key.x = series.GetX( p );
                    struct SLess { static bool Less( const SStackColumn& a, const SStackColumn& b ) { return a.x < b.x; } };
                    TStackColumnVector::iterator column = std::lower_bound( m_stack.begin(), m_stack.end(), key, &SLess::Less );
                    if ( column == m_stack.end() )
                        continue;
                    if ( value >= 0.0 )
                        (*column).positive += value;
                    else
                        (*column).negative += value;
                    Float64 top = value >= 0.0 ? (*column).positive : (*column).negative;
                    if ( logScale && top <= 0.0 )
                        continue;
                    if ( !found || top < minValue )
                        minValue = top;
                    if ( !found || top > maxValue )
                        maxValue = top;
                    found = true;
                }
            }
        }
        // bars stand on 0
        if ( !logScale )
        {
            if ( !found || minValue > 0.0 )
                minValue = 0.0;
            if ( !found || maxValue < 0.0 )
                maxValue = 0.0;
            found = true;
        }
    }
    return found;
}

/*-------------------------------------------------------------------------*/

Int32
CChartView::LegendHeight( Int32 width ) const
{GUCEF_TRACE;

    if ( !GetBoolProperty( EWidgetProperty_LegendVisible ) || VisibleSeriesCount() < 2 )
        return 0;
    const Int32 lineHeight = m_lineHeight;
    Int32 rows = 1;
    Int32 x = Padding;
    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
    {
        if ( !m_series[ i ].IsVisible() )
            continue;
        Int32 entryWidth = SwatchSize + SwatchGap + TextWidth( m_series[ i ].GetName() );
        if ( x > Padding && x + entryWidth > width - Padding )
        {
            ++rows;
            x = Padding;
        }
        x += entryWidth + LegendEntryGap;
    }
    return rows * ( lineHeight + 2 );
}

/*-------------------------------------------------------------------------*/

void
CChartView::DrawLegend( Int32 top, Int32 width )
{GUCEF_TRACE;

    const Int32 lineHeight = m_lineHeight;
    const CColour textColour = GetColourProperty( EWidgetProperty_TextColour );
    const CString& font = GetPropertyRef( EWidgetProperty_Font );
    Int32 x = Padding;
    Int32 y = top;
    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
    {
        if ( !m_series[ i ].IsVisible() )
            continue;
        const CString& name = m_series[ i ].GetName();
        Int32 entryWidth = SwatchSize + SwatchGap + TextWidth( name );
        if ( x > Padding && x + entryWidth > width - Padding )
        {
            y += lineHeight + 2;
            x = Padding;
        }
        FillRect( CRect( x, y + ( lineHeight - SwatchSize ) / 2, SwatchSize, SwatchSize ), GetSeriesColour( i ) );
        DrawString( font, name, x + SwatchSize + SwatchGap, y, textColour );
        x += entryWidth + LegendEntryGap;
    }
}

/*-------------------------------------------------------------------------*/

const CRect&
CChartView::GetPlotRect( void ) const
{GUCEF_TRACE;

    return m_plotRect;
}

/*-------------------------------------------------------------------------*/

Int32
CChartView::XToPixel( Int64 x ) const
{GUCEF_TRACE;

    return m_plotRect.left + CChartDecimator::ColumnOf( x, m_xAxis.GetMin(), m_xAxis.GetMax(), m_plotRect.width );
}

/*-------------------------------------------------------------------------*/

Int64
CChartView::PixelToX( Int32 localX ) const
{GUCEF_TRACE;

    if ( m_plotRect.width < 2 )
        return (Int64) m_xAxis.GetMin();
    Float64 unit = (Float64) ( localX - m_plotRect.left ) / (Float64) ( m_plotRect.width - 1 );
    return (Int64) floor( m_xAxis.GetMin() + unit * ( m_xAxis.GetMax() - m_xAxis.GetMin() ) + 0.5 );
}

/*-------------------------------------------------------------------------*/

Int32
CChartView::ValueToPixel( Float64 value ) const
{GUCEF_TRACE;

    const Int32 bottom = m_plotRect.top + m_plotRect.height - 1;
    Float64 unit = m_valueAxis.ToUnit( value );
    if ( IsNaN( unit ) )
        return bottom;
    if ( unit < -1000.0 )
        unit = -1000.0;
    if ( unit > 1000.0 )
        unit = 1000.0;
    return bottom - RoundToInt( unit * (Float64) ( m_plotRect.height - 1 ) );
}

/*-------------------------------------------------------------------------*/

Float64
CChartView::PixelToValue( Int32 localY ) const
{GUCEF_TRACE;

    if ( m_plotRect.height < 2 )
        return m_valueAxis.GetMin();
    const Int32 bottom = m_plotRect.top + m_plotRect.height - 1;
    return m_valueAxis.FromUnit( (Float64) ( bottom - localY ) / (Float64) ( m_plotRect.height - 1 ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CChartView::GetNearestPoints( Int64 x, TSeriesPointVector& points ) const
{GUCEF_TRACE;

    points.clear();
    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
    {
        const CChartSeries& series = m_series[ i ];
        if ( !series.IsVisible() )
            continue;
        UInt32 index = series.FindNearestValidIndex( x );
        if ( index >= series.GetCount() )
            continue;
        TSeriesPoint point;
        point.seriesIndex = i;
        point.pointIndex = index;
        point.x = series.GetX( index );
        point.value = series.GetValue( index );
        point.plotValue = GetPlotValue( i, index );
        points.push_back( point );
    }
    return (UInt32) points.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CChartView::GetNearestPointsAtPixel( Int32 localX, TSeriesPointVector& points ) const
{GUCEF_TRACE;

    return GetNearestPoints( PixelToX( localX ), points );
}

/*-------------------------------------------------------------------------*/

void
CChartView::UpdateCrosshairPoints( void )
{GUCEF_TRACE;

    if ( !m_hasCrosshair )
        return;
    GetNearestPoints( m_crosshairX, m_crosshairPoints );
    if ( m_crosshairPoints.empty() )
    {
        m_hasCrosshair = false;
        NotifyObservers( CrosshairChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

bool
CChartView::SetCrosshairX( Int64 x )
{GUCEF_TRACE;

    // snap to the x of the nearest point over all series
    bool found = false;
    Int64 snapped = 0;
    UInt64 bestDistance = 0;
    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
    {
        const CChartSeries& series = m_series[ i ];
        if ( !series.IsVisible() )
            continue;
        UInt32 index = series.FindNearestValidIndex( x );
        if ( index >= series.GetCount() )
            continue;
        Int64 candidate = series.GetX( index );
        UInt64 distance = candidate >= x ? (UInt64) candidate - (UInt64) x : (UInt64) x - (UInt64) candidate;
        if ( !found || distance < bestDistance || ( distance == bestDistance && candidate < snapped ) )
        {
            found = true;
            snapped = candidate;
            bestDistance = distance;
        }
    }
    if ( !found )
    {
        ClearCrosshair();
        return false;
    }
    if ( m_hasCrosshair && snapped == m_crosshairX )
        return true;

    m_hasCrosshair = true;
    m_crosshairX = snapped;
    GetNearestPoints( snapped, m_crosshairPoints );
    Invalidate();
    NotifyObservers( CrosshairChangedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CChartView::SetCrosshairPixel( Int32 localX )
{GUCEF_TRACE;

    if ( localX < m_plotRect.left || localX >= m_plotRect.Right() )
    {
        ClearCrosshair();
        return false;
    }
    return SetCrosshairX( PixelToX( localX ) );
}

/*-------------------------------------------------------------------------*/

void
CChartView::ClearCrosshair( void )
{GUCEF_TRACE;

    if ( !m_hasCrosshair )
        return;
    m_hasCrosshair = false;
    m_crosshairPoints.clear();
    Invalidate();
    NotifyObservers( CrosshairChangedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CChartView::HasCrosshair( void ) const
{GUCEF_TRACE;

    return m_hasCrosshair;
}

/*-------------------------------------------------------------------------*/

Int64
CChartView::GetCrosshairX( void ) const
{GUCEF_TRACE;

    return m_crosshairX;
}

/*-------------------------------------------------------------------------*/

const CChartView::TSeriesPointVector&
CChartView::GetCrosshairPoints( void ) const
{GUCEF_TRACE;

    return m_crosshairPoints;
}

/*-------------------------------------------------------------------------*/

void
CChartView::OnPropertyChanged( EWidgetProperty property, const CString& oldValue, const CString& newValue )
{GUCEF_TRACE;

    CDrawSurface::OnPropertyChanged( property, oldValue, newValue );
    switch ( property )
    {
        case EWidgetProperty_CrosshairVisible:
        {
            if ( !GetBoolProperty( EWidgetProperty_CrosshairVisible ) )
                ClearCrosshair();
            Invalidate();
            break;
        }
        case EWidgetProperty_Title:
        case EWidgetProperty_Font:
        case EWidgetProperty_TextColour:
        case EWidgetProperty_AxisTextColour:
        case EWidgetProperty_AxisColour:
        case EWidgetProperty_GridColour:
        case EWidgetProperty_CrosshairColour:
        case EWidgetProperty_TooltipColour:
        case EWidgetProperty_SeriesColours:
        case EWidgetProperty_LegendVisible:
        case EWidgetProperty_LegendPosition:
        case EWidgetProperty_XAxisType:
        case EWidgetProperty_LogScale:
        case EWidgetProperty_IncludeZero:
        case EWidgetProperty_ValuePrefix:
        case EWidgetProperty_ValueSuffix:
        case EWidgetProperty_Decimals:
        {
            Invalidate();
            break;
        }
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

bool
CChartView::OnDefaultAction( const CORE::CEvent& eventId, CRoutedEventData& eventData )
{GUCEF_TRACE;

    if ( MouseMoveEvent == eventId )
    {
        if ( !GetBoolProperty( EWidgetProperty_CrosshairVisible ) )
            return false;
        const CPointerEventData* pointer = static_cast< const CPointerEventData* >( &eventData );
        if ( m_plotRect.Contains( pointer->localX, pointer->localY ) )
            SetCrosshairPixel( pointer->localX );
        else
            ClearCrosshair();
        return true;
    }
    if ( MouseFocusLostEvent == eventId )
    {
        ClearCrosshair();
        return true;
    }
    return CDrawSurface::OnDefaultAction( eventId, eventData );
}

/*-------------------------------------------------------------------------*/

bool
CChartView::NeedsUpdate( void ) const
{GUCEF_TRACE;

    // a hidden chart does no work; it catches up when it is shown
    return IsEffectivelyVisible();
}

/*-------------------------------------------------------------------------*/

void
CChartView::OnUpdate( UInt32 deltaInMs )
{GUCEF_TRACE;

    CDrawSurface::OnUpdate( deltaInMs );
    const CRect& rect = GetRect();
    if ( rect.IsEmpty() )
        return;
    if ( !IsDirty() && !LookChanged() && rect.width == m_builtWidth && rect.height == m_builtHeight )
        return;
    Rebuild( rect.width, rect.height );
}

/*-------------------------------------------------------------------------*/

void
CChartView::AdjustPlacement( CRect& rect, const CRect& area, bool sizeFromArea )
{GUCEF_TRACE;

    CDrawSurface::AdjustPlacement( rect, area, sizeFromArea );
    // a new size is drawn in the same layout pass, before the frame shows the old drawing stretched
    if ( !rect.IsEmpty() && ( rect.width != m_builtWidth || rect.height != m_builtHeight ) )
        Rebuild( rect.width, rect.height );
}

/*-------------------------------------------------------------------------*/

void
CChartView::Rebuild( void )
{GUCEF_TRACE;

    const CRect& rect = GetRect();
    Rebuild( rect.width, rect.height );
}

/*-------------------------------------------------------------------------*/

void
CChartView::CollectDrawnPoints( const CChartSeries& series )
{GUCEF_TRACE;

    m_keep.clear();
    const UInt32 count = series.GetCount();
    if ( 0 == count )
        return;

    // the points in the x range and one more on each side, so the line runs to the edge of the plot
    UInt32 first = series.LowerBound( (Int64) floor( m_xAxis.GetMin() ) );
    UInt32 last = series.UpperBound( (Int64) ceil( m_xAxis.GetMax() ) );
    if ( first > 0 )
        --first;
    if ( last < count )
        ++last;

    if ( last - first > (UInt32) ( 2 * m_plotRect.width ) )
    {
        CChartDecimator::Decimate( series.GetXData(), series.GetValueData(), first, last, m_xAxis.GetMin(), m_xAxis.GetMax(),
                                   m_plotRect.width, m_keep );
    }
    else
    {
        for ( UInt32 i=first; i<last; ++i )
            m_keep.push_back( i );
    }
}

/*-------------------------------------------------------------------------*/

void
CChartView::DrawSeriesArea( UInt32 seriesIndex, const CColour& colour, Int32 baselineY )
{GUCEF_TRACE;

    const CChartSeries& series = m_series[ seriesIndex ];
    const Int32 width = m_plotRect.width;
    if ( width <= 0 )
        return;
    const bool logScale = CChartAxis::SCALE_LOG == m_valueAxis.GetScale();
    const Int32 none = 0x7fffffff;
    m_columnTop.assign( (size_t) width, none );
    m_columnBottom.assign( (size_t) width, -none );

    // per column the extent of the line, the segments between columns interpolated
    Int32 previousColumn = 0, previousY = 0;
    bool havePrevious = false;
    for ( UInt32 k=0; k<(UInt32)m_keep.size(); ++k )
    {
        const UInt32 p = m_keep[ k ];
        const Float64 value = series.GetValue( p );
        if ( !IsFinite( value ) || ( logScale && value <= 0.0 ) )
        {
            havePrevious = false;
            continue;
        }
        const Int32 column = XToPixel( series.GetX( p ) ) - m_plotRect.left;
        const Int32 y = ValueToPixel( value );
        Int32 from = havePrevious ? previousColumn : column;
        Int32 to = column;
        Int32 c0 = from < 0 ? 0 : from;
        Int32 c1 = to >= width ? width - 1 : to;
        for ( Int32 c=c0; c<=c1; ++c )
        {
            Int32 cy = y;
            if ( havePrevious && to != from )
                cy = previousY + RoundToInt( (Float64) ( y - previousY ) * (Float64) ( c - from ) / (Float64) ( to - from ) );
            if ( cy < m_columnTop[ c ] )
                m_columnTop[ c ] = cy;
            if ( cy > m_columnBottom[ c ] )
                m_columnBottom[ c ] = cy;
            if ( havePrevious && c == from && previousY < m_columnTop[ c ] )
                m_columnTop[ c ] = previousY;
            if ( havePrevious && c == from && previousY > m_columnBottom[ c ] )
                m_columnBottom[ c ] = previousY;
        }
        previousColumn = column;
        previousY = y;
        havePrevious = true;
    }

    // one rectangle per run of columns with the same extent
    CColour fill( colour.r, colour.g, colour.b, AreaAlpha );
    Int32 runStart = -1, runTop = 0, runBottom = 0;
    for ( Int32 c=0; c<=width; ++c )
    {
        bool has = c < width && m_columnTop[ c ] != none;
        Int32 top = 0, bottom = 0;
        if ( has )
        {
            top = m_columnTop[ c ] < baselineY ? m_columnTop[ c ] : baselineY;
            bottom = m_columnBottom[ c ] > baselineY ? m_columnBottom[ c ] : baselineY;
        }
        if ( runStart >= 0 && ( !has || top != runTop || bottom != runBottom ) )
        {
            FillRect( CRect( m_plotRect.left + runStart, runTop, c - runStart, runBottom - runTop + 1 ), fill );
            runStart = -1;
        }
        if ( has && runStart < 0 )
        {
            runStart = c;
            runTop = top;
            runBottom = bottom;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CChartView::DrawSeriesLine( UInt32 seriesIndex, const CColour& colour )
{GUCEF_TRACE;

    const CChartSeries& series = m_series[ seriesIndex ];
    const bool logScale = CChartAxis::SCALE_LOG == m_valueAxis.GetScale();
    const Int32 lineWidth = series.GetLineWidth();
    // lines that end far outside the plot are cut, so no driver draws long invisible lines
    const Float64 margin = (Float64) ( lineWidth + 2 );
    const Float64 left = (Float64) m_plotRect.left - margin;
    const Float64 top = (Float64) m_plotRect.top - margin;
    const Float64 right = (Float64) m_plotRect.Right() + margin;
    const Float64 bottom = (Float64) m_plotRect.Bottom() + margin;

    Int32 previousX = 0, previousY = 0;
    bool havePrevious = false;
    bool previousAlone = false;
    for ( UInt32 k=0; k<(UInt32)m_keep.size(); ++k )
    {
        const UInt32 p = m_keep[ k ];
        const Float64 value = series.GetValue( p );
        if ( !IsFinite( value ) || ( logScale && value <= 0.0 ) )
        {
            if ( havePrevious && previousAlone )
                FillRect( CRect( previousX - lineWidth / 2 - 1, previousY - lineWidth / 2 - 1, lineWidth + 2, lineWidth + 2 ), colour );
            havePrevious = false;
            continue;
        }
        const Int32 x = XToPixel( series.GetX( p ) );
        const Int32 y = ValueToPixel( value );
        if ( havePrevious )
        {
            if ( previousX >= left && previousX <= right && previousY >= top && previousY <= bottom &&
                 x >= left && x <= right && y >= top && y <= bottom )
            {
                DrawLine( previousX, previousY, x, y, colour, lineWidth );
            }
            else
            {
                Float64 x0 = previousX, y0 = previousY, x1 = x, y1 = y;
                if ( ClipLine( x0, y0, x1, y1, left, top, right, bottom ) )
                    DrawLine( RoundToInt( x0 ), RoundToInt( y0 ), RoundToInt( x1 ), RoundToInt( y1 ), colour, lineWidth );
            }
            previousAlone = false;
        }
        else
        {
            previousAlone = true;
        }
        previousX = x;
        previousY = y;
        havePrevious = true;
    }
    // a single point between gaps is shown as a dot
    if ( havePrevious && previousAlone )
        FillRect( CRect( previousX - lineWidth / 2 - 1, previousY - lineWidth / 2 - 1, lineWidth + 2, lineWidth + 2 ), colour );
}

/*-------------------------------------------------------------------------*/

void
CChartView::DrawBars( Float64 barSlot )
{GUCEF_TRACE;

    UInt32 groupCount = 0;
    GetBarGroup( 0, groupCount );
    if ( 0 == groupCount || !( barSlot > 0.0 ) )
        return;

    const Float64 span = m_xAxis.GetMax() - m_xAxis.GetMin();
    const Float64 slotPixels = span > 0.0 ? barSlot * (Float64) ( m_plotRect.width - 1 ) / span : (Float64) m_plotRect.width;
    const Float64 share = slotPixels / (Float64) groupCount;
    Int32 barWidth = (Int32) floor( share * BarFill );
    if ( barWidth < 1 )
        barWidth = 1;
    const Int64 lowX = (Int64) floor( m_xAxis.GetMin() );
    const Int64 highX = (Int64) ceil( m_xAxis.GetMax() );
    const bool logScale = CChartAxis::SCALE_LOG == m_valueAxis.GetScale();

    for ( UInt32 group=0; group<groupCount; ++group )
    {
        PrepareStack( group );
        // the left edge of this group's share within the slot, relative to the centre of the slot
        const Float64 shareLeft = -slotPixels / 2.0 + share * (Float64) group + ( share - (Float64) barWidth ) / 2.0;
        for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
        {
            const CChartSeries& series = m_series[ i ];
            UInt32 ignored = 0;
            if ( !series.IsVisible() || CChartSeries::SERIESTYPE_BAR != series.GetType() || GetBarGroup( i, ignored ) != group )
                continue;
            const CColour colour = GetSeriesColour( i );
            UInt32 first = series.LowerBound( lowX );
            UInt32 last = series.UpperBound( highX );
            for ( UInt32 p=first; p<last; ++p )
            {
                const Float64 value = series.GetValue( p );
                if ( !IsFinite( value ) )
                    continue;
                SStackColumn key;
                key.x = series.GetX( p );
                struct SLess { static bool Less( const SStackColumn& a, const SStackColumn& b ) { return a.x < b.x; } };
                TStackColumnVector::iterator column = std::lower_bound( m_stack.begin(), m_stack.end(), key, &SLess::Less );
                if ( column == m_stack.end() )
                    continue;
                Float64& total = value >= 0.0 ? (*column).positive : (*column).negative;
                const Float64 base = total;
                total += value;

                // on a log scale a bar without a positive base starts below the plot
                const Int32 baseY = ( logScale && base <= 0.0 ) ? m_plotRect.Bottom() : ValueToPixel( base );
                if ( logScale && total <= 0.0 )
                    continue;
                const Int32 topY = ValueToPixel( total );
                // a bar covers the rows from its value up to its base, the row of the base excluded (it belongs to the axis or the segment below)
                Int32 top = topY < baseY ? topY : baseY + 1;
                Int32 height = topY < baseY ? baseY - topY : topY - baseY;
                // a segment on another one keeps a line of the surface between them
                if ( 0.0 != base && height > 2 )
                {
                    if ( value >= 0.0 )
                        height -= 1;
                    else
                    {
                        top += 1;
                        height -= 1;
                    }
                }
                if ( height <= 0 )
                    continue;
                const Int32 centre = XToPixel( key.x );
                const Int32 left = centre + RoundToInt( shareLeft );
                FillRect( CRect( left, top, barWidth, height ), colour );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CChartView::DrawCrosshair( void )
{GUCEF_TRACE;

    if ( !m_hasCrosshair || m_crosshairPoints.empty() )
        return;
    const Int32 x = XToPixel( m_crosshairX );
    if ( x < m_plotRect.left || x >= m_plotRect.Right() )
        return;

    DrawLine( x, m_plotRect.top, x, m_plotRect.Bottom() - 1, GetColourProperty( EWidgetProperty_CrosshairColour ), 1 );
    for ( UInt32 i=0; i<(UInt32)m_crosshairPoints.size(); ++i )
    {
        const TSeriesPoint& point = m_crosshairPoints[ i ];
        if ( CChartSeries::SERIESTYPE_BAR == m_series[ point.seriesIndex ].GetType() )
            continue;
        const Int32 y = ValueToPixel( point.plotValue );
        FillRect( CRect( XToPixel( point.x ) - 2, y - 2, 5, 5 ), GetSeriesColour( point.seriesIndex ) );
    }

    // the tooltip: the x, then a line per series
    const CString& font = GetPropertyRef( EWidgetProperty_Font );
    const Int32 lineHeight = m_lineHeight;
    const Int32 decimals = GetIntProperty( EWidgetProperty_Decimals ) >= 0 ? GetIntProperty( EWidgetProperty_Decimals ) : 2;
    CString header = CChartAxis::AXISKIND_TIME == m_xAxis.GetKind() ? CChartAxis::FormatTime( m_crosshairX ) : m_xAxis.FormatValue( (Float64) m_crosshairX, 0 );
    Int32 textWidth = TextWidth( header );
    const UInt32 lines = (UInt32) m_crosshairPoints.size();
    GUCEF::vector< CString > texts;
    texts.reserve( lines );
    for ( UInt32 i=0; i<lines; ++i )
    {
        const TSeriesPoint& point = m_crosshairPoints[ i ];
        CString text = m_series[ point.seriesIndex ].GetName();
        text += ": ";
        text += m_valueAxis.FormatValue( point.value, decimals );
        Int32 width = SwatchSize + SwatchGap + TextWidth( text );
        if ( width > textWidth )
            textWidth = width;
        texts.push_back( text );
    }
    const Int32 boxWidth = textWidth + 12;
    const Int32 boxHeight = (Int32) ( lines + 1 ) * ( lineHeight + 2 ) + 8;
    Int32 boxLeft = x + 12;
    if ( boxLeft + boxWidth > m_plotRect.Right() )
        boxLeft = x - 12 - boxWidth;
    if ( boxLeft < 0 )
        boxLeft = 0;
    Int32 boxTop = m_plotRect.top + 4;
    CRect box( boxLeft, boxTop, boxWidth, boxHeight );
    FillRect( box, GetColourProperty( EWidgetProperty_TooltipColour ) );
    StrokeRect( box, GetColourProperty( EWidgetProperty_AxisColour ), 1 );
    const CColour textColour = GetColourProperty( EWidgetProperty_TextColour );
    Int32 y = boxTop + 4;
    DrawString( font, header, boxLeft + 6, y, textColour );
    for ( UInt32 i=0; i<lines; ++i )
    {
        y += lineHeight + 2;
        FillRect( CRect( boxLeft + 6, y + ( lineHeight - SwatchSize ) / 2, SwatchSize, SwatchSize ), GetSeriesColour( m_crosshairPoints[ i ].seriesIndex ) );
        DrawString( font, texts[ i ], boxLeft + 6 + SwatchSize + SwatchGap, y, textColour );
    }
}

/*-------------------------------------------------------------------------*/

void
CChartView::Rebuild( Int32 width, Int32 height )
{GUCEF_TRACE;

    m_dirty = false;
    m_builtWidth = width;
    m_builtHeight = height;
    m_builtSeriesRevision = SeriesRevisionSum();
    CaptureLook();
    ApplyProperties();
    m_lineHeight = LineHeight();

    Clear();
    if ( width <= 0 || height <= 0 )
    {
        m_plotRect = CRect();
        Commit();
        return;
    }

    const Int32 lineHeight = m_lineHeight;
    const CString& font = GetPropertyRef( EWidgetProperty_Font );
    const CColour textColour = GetColourProperty( EWidgetProperty_TextColour );
    const CColour axisTextColour = GetColourProperty( EWidgetProperty_AxisTextColour );
    const CColour axisColour = GetColourProperty( EWidgetProperty_AxisColour );
    const CColour gridColour = GetColourProperty( EWidgetProperty_GridColour );
    const CString& title = GetPropertyRef( EWidgetProperty_Title );

    // the bands from the outside in: padding, title, legend, x labels, value labels
    Int32 top = Padding;
    const Int32 titleTop = top;
    if ( !title.IsNULLOrEmpty() )
        top += lineHeight + 4;
    const Int32 legendHeight = LegendHeight( width );
    const bool legendAtBottom = GetPropertyRef( EWidgetProperty_LegendPosition ) == "Bottom";
    const Int32 legendTop = legendAtBottom ? height - Padding - legendHeight : top;
    if ( legendHeight > 0 && !legendAtBottom )
        top += legendHeight + 4;
    Int32 bottom = height - Padding - lineHeight - LabelGap;
    if ( legendHeight > 0 && legendAtBottom )
        bottom -= legendHeight + 4;

    // the x range first: the value range depends on the points in it
    Float64 minX = 0.0, maxX = 1.0;
    Float64 barSlot = 0.0;
    if ( !GetDataXRange( minX, maxX, barSlot ) )
    {
        minX = 0.0;
        maxX = CChartAxis::AXISKIND_TIME == m_xAxis.GetKind() ? (Float64) MsPerDay : 1.0;
    }
    m_barSlot = barSlot;
    Float64 rangeLowX = m_xAxis.IsAutoRange() ? minX : m_xAxis.GetMin();
    Float64 rangeHighX = m_xAxis.IsAutoRange() ? maxX : m_xAxis.GetMax();
    if ( !m_xAxis.IsAutoRange() )
    {
        // the fixed range: compute once so GetMin()/GetMax() hold it
        m_xAxis.Compute( minX, maxX, 2 );
        rangeLowX = m_xAxis.GetMin();
        rangeHighX = m_xAxis.GetMax();
    }

    Float64 minValue = 0.0, maxValue = 1.0;
    if ( !GetDataValueRange( rangeLowX, rangeHighX, minValue, maxValue ) )
    {
        minValue = CChartAxis::SCALE_LOG == m_valueAxis.GetScale() ? 1.0 : 0.0;
        maxValue = CChartAxis::SCALE_LOG == m_valueAxis.GetScale() ? 10.0 : 1.0;
    }

    // value ticks at least 2.5 lines apart
    Int32 plotHeight = bottom - top;
    if ( plotHeight < 1 )
        plotHeight = 1;
    UInt32 valueTicks = (UInt32) ( (Float64) plotHeight / ( (Float64) lineHeight * 2.5 ) );
    if ( valueTicks < 2 )
        valueTicks = 2;
    if ( valueTicks > 12 )
        valueTicks = 12;
    m_valueAxis.Compute( minValue, maxValue, valueTicks );
    Int32 labelWidth = 0;
    const CChartAxis::TTickVector& valueTickList = m_valueAxis.GetTicks();
    for ( UInt32 i=0; i<(UInt32)valueTickList.size(); ++i )
    {
        Int32 w = TextWidth( valueTickList[ i ].label );
        if ( w > labelWidth )
            labelWidth = w;
    }
    const Int32 left = Padding + labelWidth + LabelGap;
    const Int32 right = width - Padding;
    m_plotRect = CRect( left, top, right - left > 1 ? right - left : 1, plotHeight );

    // x ticks at least the widest label plus 16 pixels apart: estimate, then measure and fit
    Int32 xLabelWidth = TextWidth( CChartAxis::AXISKIND_TIME == m_xAxis.GetKind() ? "Mar 2024" : "0000" );
    for ( UInt32 attempt=0; attempt<3; ++attempt )
    {
        UInt32 xTicks = (UInt32) ( m_plotRect.width / ( xLabelWidth + 16 ) );
        if ( xTicks < 2 )
            xTicks = 2;
        m_xAxis.Compute( minX, maxX, xTicks );
        Int32 widest = 0;
        const CChartAxis::TTickVector& ticks = m_xAxis.GetTicks();
        for ( UInt32 i=0; i<(UInt32)ticks.size(); ++i )
        {
            Int32 w = TextWidth( ticks[ i ].label );
            if ( w > widest )
                widest = w;
        }
        if ( widest <= xLabelWidth || ticks.size() <= 2 )
            break;
        xLabelWidth = widest;
    }

    // grid lines at the value ticks
    for ( UInt32 i=0; i<(UInt32)valueTickList.size(); ++i )
    {
        Int32 y = ValueToPixel( valueTickList[ i ].value );
        if ( y >= m_plotRect.top && y < m_plotRect.Bottom() )
            DrawLine( m_plotRect.left, y, m_plotRect.Right() - 1, y, gridColour, 1 );
    }

    // the marks, inside the plot
    PushClip( m_plotRect );
    const bool logScale = CChartAxis::SCALE_LOG == m_valueAxis.GetScale();
    Int32 baselineY = m_plotRect.Bottom() - 1;
    if ( !logScale && m_valueAxis.GetMin() <= 0.0 && m_valueAxis.GetMax() >= 0.0 )
        baselineY = ValueToPixel( 0.0 );
    else
    if ( !logScale && m_valueAxis.GetMin() > 0.0 )
        baselineY = m_plotRect.Bottom() - 1;
    else
    if ( !logScale )
        baselineY = m_plotRect.top;
    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
    {
        const CChartSeries& series = m_series[ i ];
        if ( series.IsVisible() && CChartSeries::SERIESTYPE_AREA == series.GetType() )
        {
            CollectDrawnPoints( series );
            DrawSeriesArea( i, GetSeriesColour( i ), baselineY );
        }
    }
    DrawBars( barSlot );
    for ( UInt32 i=0; i<(UInt32)m_series.size(); ++i )
    {
        const CChartSeries& series = m_series[ i ];
        if ( series.IsVisible() && CChartSeries::SERIESTYPE_BAR != series.GetType() )
        {
            CollectDrawnPoints( series );
            DrawSeriesLine( i, GetSeriesColour( i ) );
        }
    }
    PopClip();

    // the axes: the bottom line, the line at 0 when it is inside, tick marks and labels
    const Int32 plotBottom = m_plotRect.Bottom() - 1;
    DrawLine( m_plotRect.left, plotBottom, m_plotRect.Right() - 1, plotBottom, axisColour, 1 );
    if ( baselineY != plotBottom && baselineY > m_plotRect.top && baselineY < plotBottom )
        DrawLine( m_plotRect.left, baselineY, m_plotRect.Right() - 1, baselineY, axisColour, 1 );
    for ( UInt32 i=0; i<(UInt32)valueTickList.size(); ++i )
    {
        Int32 y = ValueToPixel( valueTickList[ i ].value );
        if ( y < m_plotRect.top - 1 || y > plotBottom + 1 )
            continue;
        Int32 w = TextWidth( valueTickList[ i ].label );
        DrawString( font, valueTickList[ i ].label, m_plotRect.left - LabelGap - w, y - lineHeight / 2, axisTextColour );
    }
    const CChartAxis::TTickVector& xTickList = m_xAxis.GetTicks();
    for ( UInt32 i=0; i<(UInt32)xTickList.size(); ++i )
    {
        Int32 x = XToPixel( (Int64) xTickList[ i ].value );
        if ( x < m_plotRect.left || x >= m_plotRect.Right() )
            continue;
        DrawLine( x, plotBottom + 1, x, plotBottom + 4, axisColour, 1 );
        Int32 w = TextWidth( xTickList[ i ].label );
        Int32 labelLeft = x - w / 2;
        if ( labelLeft + w > width - 1 )
            labelLeft = width - 1 - w;
        if ( labelLeft < 0 )
            labelLeft = 0;
        DrawString( font, xTickList[ i ].label, labelLeft, plotBottom + LabelGap, axisTextColour );
    }

    if ( legendHeight > 0 )
        DrawLegend( legendTop, width );
    if ( !title.IsNULLOrEmpty() )
        DrawString( font, title, Padding, titleTop, textColour );

    // the crosshair follows the data: its points are looked up again for the current data
    if ( m_hasCrosshair )
        GetNearestPoints( m_crosshairX, m_crosshairPoints );
    DrawCrosshair();

    Commit();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
