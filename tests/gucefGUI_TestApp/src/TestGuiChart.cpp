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
#include <stdlib.h>
#include <limits>
#include <algorithm>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

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
const Int64 Hour = 3600000;

const Float64 NaN = std::numeric_limits< Float64 >::quiet_NaN();

bool
Close( Float64 a, Float64 b, Float64 tolerance = 1e-9 )
{
    return fabs( a - b ) <= tolerance * ( fabs( b ) > 1.0 ? fabs( b ) : 1.0 );
}

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

/** Rows of a pixel column (from 'top' to 'bottom' exclusive) in the colour */
Int32
CountInColumn( const IMAGE::TPixelMapPtr& pixels, Int32 x, Int32 top, Int32 bottom, const GUI::CColour& colour )
{
    Int32 count = 0;
    for ( Int32 y=top; y<bottom; ++y )
        if ( SameRgb( PixelAt( pixels, x, y ), colour ) )
            ++count;
    return count;
}

/** A repeatable sequence of numbers in [0, 1) */
class CSequence
{
    public:

    explicit CSequence( UInt32 seed ) : m_state( seed ) {}

    Float64 Next( void )
    {
        m_state = m_state * 1664525u + 1013904223u;
        return (Float64) ( m_state >> 8 ) / (Float64) ( 1u << 24 );
    }

    private:

    UInt32 m_state;
};

/** Counts CrosshairChangedEvent of one chart */
class CCrosshairCount : public CORE::CObserver
{
    public:

    UInt32 count;

    explicit CCrosshairCount( CORE::CNotifier* notifier )
        : CORE::CObserver(), count( 0 )
    {
        SubscribeTo( notifier, GUI::CChartView::CrosshairChangedEvent );
    }

    virtual ~CCrosshairCount() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& id, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUI::CChartView::CrosshairChangedEvent == id )
            ++count;
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

/*
 *  A PNG of a capture for a person to look at, written when GUCEF_TEST_SHOT_DIR names a directory:
 *  RGBA scan lines in stored deflate blocks
 */

UInt32
Crc32( const UInt8* data, size_t size, UInt32 crc )
{
    static UInt32 table[ 256 ];
    static bool tableMade = false;
    if ( !tableMade )
    {
        for ( UInt32 n=0; n<256; ++n )
        {
            UInt32 c = n;
            for ( UInt32 k=0; k<8; ++k )
                c = ( c & 1 ) ? 0xedb88320u ^ ( c >> 1 ) : c >> 1;
            table[ n ] = c;
        }
        tableMade = true;
    }
    crc = ~crc;
    for ( size_t i=0; i<size; ++i )
        crc = table[ ( crc ^ data[ i ] ) & 0xff ] ^ ( crc >> 8 );
    return ~crc;
}

void
Put32( GUCEF::vector< UInt8 >& out, UInt32 value )
{
    out.push_back( (UInt8) ( value >> 24 ) );
    out.push_back( (UInt8) ( value >> 16 ) );
    out.push_back( (UInt8) ( value >> 8 ) );
    out.push_back( (UInt8) value );
}

void
PutChunk( GUCEF::vector< UInt8 >& out, const char* type, const GUCEF::vector< UInt8 >& data )
{
    Put32( out, (UInt32) data.size() );
    GUCEF::vector< UInt8 > body( type, type + 4 );
    body.insert( body.end(), data.begin(), data.end() );
    out.insert( out.end(), body.begin(), body.end() );
    Put32( out, Crc32( &body[ 0 ], body.size(), 0 ) );
}

bool
WriteShot( const IMAGE::TPixelMapPtr& pixels, const CORE::CString& name )
{
    CORE::CString directory = CORE::GetEnv( "GUCEF_TEST_SHOT_DIR" );
    if ( directory.IsNULLOrEmpty() || pixels.IsNULL() )
        return false;
    UInt32 width = pixels->GetWidthInPixels();
    UInt32 height = pixels->GetHeightInPixels();
    const UInt8* rgba = static_cast< const UInt8* >( pixels->GetDataPtr() );

    GUCEF::vector< UInt8 > raw;
    raw.reserve( (size_t) height * ( width * 4 + 1 ) );
    for ( UInt32 y=0; y<height; ++y )
    {
        raw.push_back( 0 );
        raw.insert( raw.end(), rgba + (size_t) y * width * 4, rgba + (size_t) ( y + 1 ) * width * 4 );
    }
    GUCEF::vector< UInt8 > zlib;
    zlib.push_back( 0x78 );
    zlib.push_back( 0x01 );
    size_t position = 0;
    UInt32 a = 1, b = 0;
    for ( size_t i=0; i<raw.size(); ++i )
    {
        a = ( a + raw[ i ] ) % 65521;
        b = ( b + a ) % 65521;
    }
    do
    {
        size_t count = raw.size() - position;
        if ( count > 65535 )
            count = 65535;
        bool last = position + count >= raw.size();
        zlib.push_back( last ? 1 : 0 );
        zlib.push_back( (UInt8) ( count & 0xff ) );
        zlib.push_back( (UInt8) ( count >> 8 ) );
        zlib.push_back( (UInt8) ( ~count & 0xff ) );
        zlib.push_back( (UInt8) ( ( ~count >> 8 ) & 0xff ) );
        zlib.insert( zlib.end(), raw.begin() + position, raw.begin() + position + count );
        position += count;
    }
    while ( position < raw.size() );
    Put32( zlib, ( b << 16 ) | a );

    GUCEF::vector< UInt8 > header;
    Put32( header, width );
    Put32( header, height );
    header.push_back( 8 );
    header.push_back( 6 );
    header.push_back( 0 );
    header.push_back( 0 );
    header.push_back( 0 );

    GUCEF::vector< UInt8 > png;
    static const UInt8 signature[ 8 ] = { 0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a };
    png.insert( png.end(), signature, signature + 8 );
    PutChunk( png, "IHDR", header );
    PutChunk( png, "IDAT", zlib );
    PutChunk( png, "IEND", GUCEF::vector< UInt8 >() );

    CORE::CString path = directory;
    CORE::AppendToPath( path, name + ".png" );
    CORE::CFileAccess file( path, "wb" );
    return file.IsValid() && png.size() == file.Write( &png[ 0 ], 1, (UInt32) png.size() );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiChartTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.Chart" );

    GUCEF_TESTFW_TESTCASE( "Chart: a series keeps its points ascending by x, takes unsigned x values within the Int64 range and finds the nearest point" )
    try
    {
        GUI::CChartSeries series( "Price", GUI::CChartSeries::SERIESTYPE_LINE );
        const Int64 x[] = { 30, 10, 20, 10, 40 };
        const Float64 values[] = { 3.0, 1.0, 2.0, 1.5, NaN };
        ASSERT_TRUE( series.SetData( x, values, 5 ) );
        ASSERT_TRUE( 5 == series.GetCount() );
        // sorted, and equal x values keep their order
        ASSERT_TRUE( 10 == series.GetX( 0 ) && 1.0 == series.GetValue( 0 ) );
        ASSERT_TRUE( 10 == series.GetX( 1 ) && 1.5 == series.GetValue( 1 ) );
        ASSERT_TRUE( 20 == series.GetX( 2 ) && 30 == series.GetX( 3 ) && 40 == series.GetX( 4 ) );

        Int64 minX = 0, maxX = 0;
        Float64 minValue = 0.0, maxValue = 0.0;
        ASSERT_TRUE( series.GetXRange( minX, maxX ) && 10 == minX && 40 == maxX );
        ASSERT_TRUE( series.GetValueRange( minValue, maxValue ) && 1.0 == minValue && 3.0 == maxValue );

        // a tie goes to the lower index, the ends clamp
        ASSERT_TRUE( 0 == series.FindNearestIndex( 15 ) );
        ASSERT_TRUE( 2 == series.FindNearestIndex( 21 ) );
        ASSERT_TRUE( 0 == series.FindNearestIndex( -1000 ) );
        ASSERT_TRUE( 4 == series.FindNearestIndex( 1000 ) );
        // the point at 40 has no value: the nearest with one is at 30
        ASSERT_TRUE( 3 == series.FindNearestValidIndex( 39 ) );

        ASSERT_FALSE( series.SetData( (const Int64*) GUCEF_NULL, values, 3 ) );
        UInt32 revision = series.GetRevision();
        const UInt64 tooLarge[] = { 1, ( ~(UInt64) 0 ) };
        ASSERT_FALSE( series.SetData( tooLarge, values, 2 ) );
        ASSERT_TRUE( 5 == series.GetCount() && revision == series.GetRevision() );
        const UInt64 unsignedX[] = { 1700000000000ULL, 1700000086400ULL };
        ASSERT_TRUE( series.SetData( unsignedX, values, 2 ) && 2 == series.GetCount() && 1700000086400LL == series.GetX( 1 ) );
        ASSERT_TRUE( series.GetRevision() != revision );

        ASSERT_TRUE( series.Append( 1700000086400LL, 4.0 ) );
        ASSERT_FALSE( series.Append( 5, 4.0 ) );
        series.Clear();
        ASSERT_TRUE( 0 == series.GetCount() && GUCEF_NULL == series.GetXData() && 0 == series.FindNearestIndex( 5 ) );
        ASSERT_FALSE( series.GetXRange( minX, maxX ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: a linear value axis takes 1, 2, 2.5 or 5 times a power of 10 as its step, extends to whole ticks and labels values with separators, prefix and suffix" )
    try
    {
        ASSERT_TRUE( Close( 1.0, GUI::CChartAxis::NiceStep( 1.0 ) ) );
        ASSERT_TRUE( Close( 0.5, GUI::CChartAxis::NiceStep( 0.3 ) ) );
        ASSERT_TRUE( Close( 2.5, GUI::CChartAxis::NiceStep( 2.1 ) ) );
        ASSERT_TRUE( Close( 50.0, GUI::CChartAxis::NiceStep( 26.0 ) ) );
        ASSERT_TRUE( Close( 10.0, GUI::CChartAxis::NiceStep( 7.0 ) ) );
        ASSERT_TRUE( Close( 2000.0, GUI::CChartAxis::NiceStep( 2000.0 ) ) );

        GUI::CChartAxis axis;
        axis.Compute( 3.0, 97.0, 6 );
        ASSERT_TRUE( Close( 0.0, axis.GetMin() ) && Close( 100.0, axis.GetMax() ) && Close( 20.0, axis.GetTickStep() ) );
        ASSERT_TRUE( 6 == axis.GetTicks().size() && "0" == axis.GetTicks()[ 0 ].label && "100" == axis.GetTicks()[ 5 ].label );

        // without 0: the range extends to whole ticks around the data
        axis.SetIncludeZero( false );
        axis.Compute( 103.0, 147.0, 6 );
        ASSERT_TRUE( Close( 100.0, axis.GetMin() ) && Close( 150.0, axis.GetMax() ) && Close( 10.0, axis.GetTickStep() ) );

        // decimals follow the step
        axis.SetIncludeZero( true );
        axis.Compute( 0.0, 1.0, 5 );
        ASSERT_TRUE( Close( 0.25, axis.GetTickStep() ) && 2 == axis.GetLabelDecimals() && "0.25" == axis.GetTicks()[ 1 ].label );

        // money
        axis.SetValuePrefix( "$" );
        axis.Compute( 0.0, 1234567.0, 5 );
        ASSERT_TRUE( Close( 500000.0, axis.GetTickStep() ) && "$1,500,000" == axis.GetTicks().back().label );
        ASSERT_TRUE( "-$1,234.50" == axis.FormatValue( -1234.5, 2 ) );
        ASSERT_TRUE( "$0.00" == axis.FormatValue( -0.001, 2 ) );
        ASSERT_TRUE( "$999" == axis.FormatValue( 999.0, 0 ) );
        axis.SetValuePrefix( "" );
        axis.SetValueSuffix( "%" );
        ASSERT_TRUE( "12.5%" == axis.FormatValue( 12.5, 1 ) );

        // a fixed range is kept as it is
        axis.SetRange( 10.0, 13.0 );
        axis.Compute( 0.0, 1000.0, 4 );
        ASSERT_TRUE( Close( 10.0, axis.GetMin() ) && Close( 13.0, axis.GetMax() ) && Close( 1.0, axis.GetTickStep() ) && 4 == axis.GetTicks().size() );

        // one value: one unit either side
        axis.SetAutoRange();
        axis.SetIncludeZero( false );
        axis.Compute( 50.0, 50.0, 5 );
        ASSERT_TRUE( axis.GetMin() <= 49.0 && axis.GetMax() >= 51.0 );
        ASSERT_TRUE( Close( 0.0, axis.ToUnit( axis.GetMin() ) ) && Close( 1.0, axis.ToUnit( axis.GetMax() ) ) );
        ASSERT_TRUE( Close( 50.0, axis.FromUnit( axis.ToUnit( 50.0 ) ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: a log value axis has its ticks at powers of 10, with 2 and 5 times them over three decades or fewer, and cannot show values at or below 0" )
    try
    {
        GUI::CChartAxis axis;
        axis.SetScale( GUI::CChartAxis::SCALE_LOG );
        axis.Compute( 3.0, 870.0, 10 );
        ASSERT_TRUE( Close( 1.0, axis.GetMin() ) && Close( 1000.0, axis.GetMax() ) );
        ASSERT_TRUE( 10 == axis.GetTicks().size() );
        ASSERT_TRUE( Close( 2.0, axis.GetTicks()[ 1 ].value ) && Close( 5.0, axis.GetTicks()[ 2 ].value ) && "1,000" == axis.GetTicks()[ 9 ].label );
        ASSERT_TRUE( Close( 0.5, axis.ToUnit( pow( 10.0, 1.5 ) ) ) );
        ASSERT_TRUE( axis.ToUnit( 0.0 ) != axis.ToUnit( 0.0 ) );
        ASSERT_TRUE( axis.ToUnit( -5.0 ) != axis.ToUnit( -5.0 ) );

        // nine decades in four ticks: every third decade
        axis.Compute( 0.5, 2e6, 4 );
        ASSERT_TRUE( Close( 0.1, axis.GetMin() ) && Close( 1e7, axis.GetMax() ) );
        ASSERT_TRUE( 3 == axis.GetTicks().size() && "0.1" == axis.GetTicks()[ 0 ].label && Close( 100.0, axis.GetTicks()[ 1 ].value ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: a time axis picks hour, week, quarter and decade ticks by the span, aligned in UTC, with labels of the unit" )
    try
    {
        GUI::CChartAxis axis;
        axis.SetKind( GUI::CChartAxis::AXISKIND_TIME );
        const Int64 march5 = GUI::CChartAxis::MakeTime( 2024, 3, 5 );

        // eight hours in ten ticks: hourly
        axis.Compute( (Float64) ( march5 + 9 * Hour + 1234 ), (Float64) ( march5 + 17 * Hour ), 10 );
        ASSERT_TRUE( GUI::CChartAxis::TIMEUNIT_HOUR == axis.GetTimeUnit() && 1 == axis.GetTimeStep() );
        ASSERT_TRUE( 8 == axis.GetTicks().size() );
        ASSERT_TRUE( Close( (Float64) ( march5 + 10 * Hour ), axis.GetTicks()[ 0 ].value ) && "10:00" == axis.GetTicks()[ 0 ].label );

        // sixty days in ten ticks: weeks, starting on a Monday (2024-03-11)
        const Int64 march6 = GUI::CChartAxis::MakeTime( 2024, 3, 6 );
        axis.Compute( (Float64) march6, (Float64) ( march6 + 60 * Day ), 10 );
        ASSERT_TRUE( GUI::CChartAxis::TIMEUNIT_WEEK == axis.GetTimeUnit() );
        ASSERT_TRUE( Close( (Float64) GUI::CChartAxis::MakeTime( 2024, 3, 11 ), axis.GetTicks()[ 0 ].value ) && "Mar 11" == axis.GetTicks()[ 0 ].label );
        ASSERT_TRUE( Close( (Float64) ( 7 * Day ), axis.GetTicks()[ 1 ].value - axis.GetTicks()[ 0 ].value ) );

        // a year in six ticks: quarters
        axis.Compute( (Float64) GUI::CChartAxis::MakeTime( 2023, 11, 15 ), (Float64) GUI::CChartAxis::MakeTime( 2024, 11, 20 ), 6 );
        ASSERT_TRUE( GUI::CChartAxis::TIMEUNIT_MONTH == axis.GetTimeUnit() && 3 == axis.GetTimeStep() );
        ASSERT_TRUE( 4 == axis.GetTicks().size() && "Jan 2024" == axis.GetTicks()[ 0 ].label && "Oct 2024" == axis.GetTicks()[ 3 ].label );
        ASSERT_TRUE( Close( (Float64) GUI::CChartAxis::MakeTime( 2024, 4, 1 ), axis.GetTicks()[ 1 ].value ) );

        // 36 years in six ticks: decades
        axis.Compute( (Float64) GUI::CChartAxis::MakeTime( 1990, 6, 1 ), (Float64) GUI::CChartAxis::MakeTime( 2026, 1, 1 ), 6 );
        ASSERT_TRUE( GUI::CChartAxis::TIMEUNIT_YEAR == axis.GetTimeUnit() && 10 == axis.GetTimeStep() );
        ASSERT_TRUE( 3 == axis.GetTicks().size() && "2000" == axis.GetTicks()[ 0 ].label && "2020" == axis.GetTicks()[ 2 ].label );

        // the tooltip form of a time
        ASSERT_TRUE( "2024-03-05" == GUI::CChartAxis::FormatTime( march5 ) );
        ASSERT_TRUE( "2024-03-05 14:30" == GUI::CChartAxis::FormatTime( march5 + 14 * Hour + 30 * 60000 ) );
        ASSERT_TRUE( "2024-03-05 14:30:15" == GUI::CChartAxis::FormatTime( march5 + 14 * Hour + 30 * 60000 + 15000 ) );
        ASSERT_TRUE( "1969-12-31 23:59:59" == GUI::CChartAxis::FormatTime( -1000 ) );
        ASSERT_TRUE( "1969-12-31" == GUI::CChartAxis::FormatTime( GUI::CChartAxis::MakeTime( 1969, 12, 31 ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: decimation keeps the first, lowest, highest and last point of every pixel column in order, at most four, and keeps gaps" )
    try
    {
        const UInt32 count = 10000;
        const Int32 columns = 100;
        GUCEF::vector< Int64 > x( count );
        GUCEF::vector< Float64 > values( count );
        CSequence sequence( 7 );
        for ( UInt32 i=0; i<count; ++i )
        {
            x[ i ] = (Int64) i * 1000;
            values[ i ] = sequence.Next() * 100.0;
        }
        // in the middle of a column, so it splits that column in two runs
        values[ 5050 ] = NaN;
        const Float64 xMin = 0.0, xMax = (Float64) x[ count - 1 ];

        GUI::CChartDecimator::TIndexVector keep;
        GUI::CChartDecimator::Decimate( &x[ 0 ], &values[ 0 ], 0, count, xMin, xMax, columns, keep );
        ASSERT_TRUE( !keep.empty() && keep.size() <= (size_t) ( 4 * columns + 8 ) );
        for ( UInt32 k=1; k<(UInt32)keep.size(); ++k )
            ASSERT_TRUE( keep[ k-1 ] < keep[ k ] );
        ASSERT_TRUE( std::binary_search( keep.begin(), keep.end(), (UInt32) 5050 ) );
        ASSERT_TRUE( std::binary_search( keep.begin(), keep.end(), (UInt32) 5049 ) && std::binary_search( keep.begin(), keep.end(), (UInt32) 5051 ) );

        // against a brute force pass per column (the NaN splits its column in two runs)
        UInt32 i = 0;
        bool allFound = true;
        UInt32 groups = 0;
        while ( i < count )
        {
            if ( values[ i ] != values[ i ] )
            {
                ++i;
                continue;
            }
            Int32 column = GUI::CChartDecimator::ColumnOf( x[ i ], xMin, xMax, columns );
            UInt32 first = i, lowest = i, highest = i, last = i;
            while ( i < count && values[ i ] == values[ i ] && GUI::CChartDecimator::ColumnOf( x[ i ], xMin, xMax, columns ) == column )
            {
                if ( values[ i ] < values[ lowest ] )
                    lowest = i;
                if ( values[ i ] > values[ highest ] )
                    highest = i;
                last = i;
                ++i;
            }
            ++groups;
            allFound = allFound && std::binary_search( keep.begin(), keep.end(), first ) && std::binary_search( keep.begin(), keep.end(), lowest ) &&
                       std::binary_search( keep.begin(), keep.end(), highest ) && std::binary_search( keep.begin(), keep.end(), last );
        }
        ASSERT_TRUE( allFound );
        ASSERT_TRUE( groups == (UInt32) columns + 1 );

        // a steady caller does not allocate: the vector keeps its capacity
        size_t capacity = keep.capacity();
        GUI::CChartDecimator::Decimate( &x[ 0 ], &values[ 0 ], 0, count, xMin, xMax, columns, keep );
        ASSERT_TRUE( capacity == keep.capacity() );

        // fewer points than columns: every point stays
        GUI::CChartDecimator::Decimate( &x[ 0 ], &values[ 0 ], 0, 50, xMin, (Float64) x[ 49 ], 1000, keep );
        ASSERT_TRUE( 50 == keep.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: lookups find the nearest point of every series without a context, with the drawn value of stacked bars" )
    try
    {
        GUI::CChartView chart;
        UInt32 line = chart.AddSeries( "Value", GUI::CChartSeries::SERIESTYPE_LINE );
        UInt32 qualified = chart.AddSeries( "Qualified", GUI::CChartSeries::SERIESTYPE_BAR );
        UInt32 ordinary = chart.AddSeries( "Ordinary", GUI::CChartSeries::SERIESTYPE_BAR );
        chart.GetSeries( qualified )->SetStackGroup( "income" );
        chart.GetSeries( ordinary )->SetStackGroup( "income" );
        const Int64 days[] = { 0, Day, 2 * Day, 3 * Day };
        const Float64 values[] = { 100.0, NaN, 102.0, 103.0 };
        const Float64 qualifiedValues[] = { 10.0, 20.0, 30.0, 40.0 };
        const Float64 ordinaryValues[] = { 5.0, 6.0, -7.0, 8.0 };
        ASSERT_TRUE( chart.SetSeriesData( line, days, values, 4 ) );
        ASSERT_TRUE( chart.SetSeriesData( qualified, days, qualifiedValues, 4 ) );
        ASSERT_TRUE( chart.SetSeriesData( ordinary, days, ordinaryValues, 4 ) );
        ASSERT_FALSE( chart.SetSeriesData( 7, days, values, 4 ) );
        ASSERT_TRUE( 1 == chart.FindSeries( "Qualified" ) && -1 == chart.FindSeries( "Nothing" ) );

        GUI::CChartView::TSeriesPointVector points;
        ASSERT_TRUE( 3 == chart.GetNearestPoints( Day / 2, points ) );
        // the tie at half a day goes to the lower x
        ASSERT_TRUE( 0 == points[ 0 ].seriesIndex && 0 == points[ 0 ].pointIndex && 100.0 == points[ 0 ].value );
        ASSERT_TRUE( 1 == points[ 1 ].seriesIndex && 0 == points[ 1 ].x && 10.0 == points[ 1 ].plotValue );
        ASSERT_TRUE( 2 == points[ 2 ].seriesIndex && 5.0 == points[ 2 ].value && 15.0 == points[ 2 ].plotValue );

        // the line has no value at day 1: its nearest point with one is taken (day 2 is nearer than day 0)
        ASSERT_TRUE( 3 == chart.GetNearestPoints( Day + Day / 4, points ) );
        ASSERT_TRUE( 2 * Day == points[ 0 ].x && 1 == points[ 1 ].pointIndex && 26.0 == points[ 2 ].plotValue );

        // a negative value stacks downwards from 0 on its own
        ASSERT_TRUE( 3 == chart.GetNearestPoints( 2 * Day, points ) );
        ASSERT_TRUE( -7.0 == points[ 2 ].value && -7.0 == points[ 2 ].plotValue );

        // hidden series are left out
        chart.GetSeries( line )->SetVisible( false );
        ASSERT_TRUE( 2 == chart.GetNearestPoints( 3 * Day, points ) && 1 == points[ 0 ].seriesIndex && 48.0 == points[ 1 ].plotValue );

        ASSERT_TRUE( chart.RemoveSeries( line ) && 2 == chart.GetSeriesCount() && 0 == chart.FindSeries( "Qualified" ) );
        chart.RemoveAllSeries();
        ASSERT_TRUE( 0 == chart.GetNearestPoints( 0, points ) && points.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: after a rebuild pixels map to x values and back, and the crosshair snaps to the nearest point and reports its moves" )
    try
    {
        GUI::CChartView chart;
        UInt32 index = chart.AddSeries( "Value", GUI::CChartSeries::SERIESTYPE_LINE );
        GUCEF::vector< Int64 > x;
        GUCEF::vector< Float64 > values;
        const Int64 start = GUI::CChartAxis::MakeTime( 2024, 1, 1 );
        for ( UInt32 i=0; i<30; ++i )
        {
            x.push_back( start + (Int64) i * Day );
            values.push_back( 10.0 + (Float64) i );
        }
        ASSERT_TRUE( chart.SetSeriesData( index, &x[ 0 ], &values[ 0 ], 30 ) );
        ASSERT_TRUE( chart.IsDirty() );
        chart.Rebuild( 600, 300 );
        ASSERT_FALSE( chart.IsDirty() );
        const GUI::CRect& plot = chart.GetPlotRect();
        ASSERT_TRUE( plot.width > 400 && plot.height > 150 && plot.Right() <= 600 && plot.Bottom() <= 300 );
        ASSERT_TRUE( !chart.GetCommands().empty() );

        // the first and last point at the edges of the plot, values inside it
        ASSERT_TRUE( plot.left == chart.XToPixel( x[ 0 ] ) && plot.Right() - 1 == chart.XToPixel( x[ 29 ] ) );
        for ( UInt32 i=1; i<30; ++i )
            ASSERT_TRUE( chart.XToPixel( x[ i ] ) > chart.XToPixel( x[ i-1 ] ) );
        Int64 back = chart.PixelToX( chart.XToPixel( x[ 12 ] ) );
        ASSERT_TRUE( back > x[ 12 ] - Day / 4 && back < x[ 12 ] + Day / 4 );
        ASSERT_TRUE( chart.ValueToPixel( 10.0 ) > chart.ValueToPixel( 39.0 ) );
        ASSERT_TRUE( chart.ValueToPixel( chart.GetValueAxis().GetMin() ) == plot.Bottom() - 1 && chart.ValueToPixel( chart.GetValueAxis().GetMax() ) == plot.top );
        ASSERT_TRUE( Close( 25.0, chart.PixelToValue( chart.ValueToPixel( 25.0 ) ), 0.2 ) );

        GUI::CChartView::TSeriesPointVector points;
        ASSERT_TRUE( 1 == chart.GetNearestPointsAtPixel( chart.XToPixel( x[ 7 ] ) + 2, points ) && 7 == points[ 0 ].pointIndex );

        CCrosshairCount counter( &chart );
        ASSERT_TRUE( chart.SetCrosshairPixel( chart.XToPixel( x[ 5 ] ) + 3 ) );
        ASSERT_TRUE( chart.HasCrosshair() && x[ 5 ] == chart.GetCrosshairX() && 1 == counter.count );
        ASSERT_TRUE( 1 == chart.GetCrosshairPoints().size() && 15.0 == chart.GetCrosshairPoints()[ 0 ].value );
        ASSERT_TRUE( chart.IsDirty() );
        // the same point again is no move
        ASSERT_TRUE( chart.SetCrosshairX( x[ 5 ] + 1000 ) && 1 == counter.count );
        ASSERT_TRUE( chart.SetCrosshairX( x[ 6 ] ) && 2 == counter.count );
        ASSERT_FALSE( chart.SetCrosshairPixel( plot.Right() + 5 ) );
        ASSERT_FALSE( chart.HasCrosshair() );
        ASSERT_TRUE( 3 == counter.count );
        chart.ClearCrosshair();
        ASSERT_TRUE( 3 == counter.count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: the ChartView type is a DrawSurface with its properties, defaults, theme values and a lossless round trip of its property text" )
    try
    {
        ASSERT_TRUE( "XAxisType" == GUI::WidgetPropertyToString( GUI::EWidgetProperty_XAxisType ) );
        ASSERT_TRUE( GUI::EWidgetProperty_SeriesColours == GUI::StringToWidgetProperty( "SeriesColours" ) );
        ASSERT_TRUE( GUI::EWidgetProperty_CrosshairVisible == GUI::StringToWidgetProperty( "CrosshairVisible" ) );

        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CWidget* widget = context->CreateWidget( "ChartView", "Chart", GUCEF_NULL );
            GUI::CChartView* chart = GUI::CWidget::As< GUI::CChartView >( widget );
            ASSERT_TRUE( GUCEF_NULL != chart && widget->IsRegisteredAs( "DrawSurface" ) && "ChartView" == widget->GetType() );
            if ( GUCEF_NULL != chart )
            {
                ASSERT_TRUE( "Time" == chart->GetProperty( GUI::EWidgetProperty_XAxisType ) );
                ASSERT_TRUE( -1 == chart->GetIntProperty( GUI::EWidgetProperty_Decimals ) );
                ASSERT_TRUE( chart->GetBoolProperty( GUI::EWidgetProperty_LegendVisible ) && chart->GetBoolProperty( GUI::EWidgetProperty_CrosshairVisible ) );
                ASSERT_TRUE( chart->GetBoolProperty( GUI::EWidgetProperty_IncludeZero ) && !chart->GetBoolProperty( GUI::EWidgetProperty_LogScale ) );
                ASSERT_TRUE( chart->GetBoolProperty( GUI::EWidgetProperty_Clip ) );
                GUI::CWidget::TStringVector colours = chart->GetStringListProperty( GUI::EWidgetProperty_SeriesColours );
                ASSERT_TRUE( 8 == colours.size() && "57,135,229,255" == colours[ 0 ] );
                ASSERT_TRUE( GUI::CChartPalette::GetDarkPalette()[ 1 ] == chart->GetSeriesColour( 1 ) );
                ASSERT_TRUE( GUI::CChartPalette::GetDarkPalette()[ 0 ] == chart->GetSeriesColour( 8 ) );
                ASSERT_TRUE( GUI::CColour( 26, 26, 25, 255 ) == chart->GetColourProperty( GUI::EWidgetProperty_BackgroundColour ) );

                // enumerations are checked
                ASSERT_FALSE( chart->SetProperty( GUI::EWidgetProperty_LegendPosition, "Sideways" ) );
                ASSERT_TRUE( chart->SetProperty( "LegendPosition", "Bottom" ) && "Bottom" == chart->GetProperty( "LegendPosition" ) );

                // the light palette through the property; a series colour of its own wins
                ASSERT_TRUE( chart->SetStringListProperty( GUI::EWidgetProperty_SeriesColours, GUI::CChartPalette::ToStringList( GUI::CChartPalette::GetLightPalette() ) ) );
                ASSERT_TRUE( GUI::CChartPalette::GetLightPalette()[ 2 ] == chart->GetSeriesColour( 2 ) );
                UInt32 index = chart->AddSeries( "Own", GUI::CChartSeries::SERIESTYPE_LINE );
                chart->GetSeries( index )->SetColour( GUI::CColour( 1, 2, 3, 255 ) );
                ASSERT_TRUE( GUI::CColour( 1, 2, 3, 255 ) == chart->GetSeriesColour( index ) );

                chart->SetProperty( GUI::EWidgetProperty_ValuePrefix, "$" );
                chart->SetProperty( GUI::EWidgetProperty_XAxisType, "Numeric" );
                chart->SetProperty( GUI::EWidgetProperty_Title, "Income per month" );
                chart->SetIntProperty( GUI::EWidgetProperty_Decimals, 0 );
                chart->SetBoolProperty( GUI::EWidgetProperty_LogScale, true );

                // the property text of one chart loads into another one unchanged
                CORE::CDynamicBuffer buffer;
                {
                    CORE::CDynamicBufferAccess output( buffer );
                    ASSERT_TRUE( chart->SaveProperties( output ) );
                }
                GUI::CChartView* copy = GUI::CWidget::As< GUI::CChartView >( context->CreateWidget( "ChartView", "Copy", GUCEF_NULL ) );
                ASSERT_TRUE( GUCEF_NULL != copy );
                if ( GUCEF_NULL != copy )
                {
                    CORE::CDynamicBufferAccess input( buffer );
                    ASSERT_TRUE( copy->LoadProperties( input ) );
                    ASSERT_TRUE( chart->GetExplicitProperties() == copy->GetExplicitProperties() );
                    ASSERT_TRUE( "$" == copy->GetProperty( "ValuePrefix" ) && "Numeric" == copy->GetProperty( "XAxisType" ) && copy->GetBoolProperty( GUI::EWidgetProperty_LogScale ) );
                    ASSERT_TRUE( GUI::CChartPalette::GetLightPalette()[ 7 ] == copy->GetSeriesColour( 7 ) );

                    // the properties reach the axes at the rebuild
                    copy->Rebuild( 400, 200 );
                    ASSERT_TRUE( GUI::CChartAxis::AXISKIND_NUMERIC == copy->GetXAxis().GetKind() && GUI::CChartAxis::SCALE_LOG == copy->GetValueAxis().GetScale() );
                    ASSERT_TRUE( "$" == copy->GetValueAxis().GetValuePrefix() && 0 == copy->GetValueAxis().GetDecimals() );
                }

                // a theme sets the colours of every chart
                CORE::CDataNode theme( "GUITheme" );
                CORE::CDataNode* style = theme.AddChild( "Style" );
                style->SetAttribute( "type", "ChartView" );
                CORE::CDataNode* property = style->AddChild( "Property" );
                property->SetAttribute( "name", "GridColour" );
                property->SetAttribute( "value", "1,2,3,255" );
                ASSERT_TRUE( context->SetTheme( theme ) );
                ASSERT_TRUE( GUI::CColour( 1, 2, 3, 255 ) == chart->GetColourProperty( GUI::EWidgetProperty_GridColour ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: both default palettes keep neighbouring series apart under protanopia, deuteranopia and normal vision, and a palette of near colours fails" )
    try
    {
        GUI::CChartPalette::TValidation dark = GUI::CChartPalette::Validate( GUI::CChartPalette::GetDarkPalette(), true );
        GUI::CChartPalette::TValidation light = GUI::CChartPalette::Validate( GUI::CChartPalette::GetLightPalette(), false );
        ASSERT_TRUE( dark.passed && light.passed );
        ASSERT_TRUE( dark.worstAdjacentCvdDeltaE >= GUI::CChartPalette::CvdTarget && light.worstAdjacentCvdDeltaE >= GUI::CChartPalette::CvdTarget );
        ASSERT_TRUE( dark.worstAdjacentNormalDeltaE >= GUI::CChartPalette::NormalVisionFloor && light.worstAdjacentNormalDeltaE >= GUI::CChartPalette::NormalVisionFloor );
        // the values the palette was chosen with
        ASSERT_TRUE( fabs( dark.worstAdjacentCvdDeltaE - 8.4 ) < 0.1 && fabs( light.worstAdjacentCvdDeltaE - 9.1 ) < 0.1 );

        GUI::CChartPalette::TColourVector near;
        near.push_back( GUI::CColour( 57, 135, 229, 255 ) );
        near.push_back( GUI::CColour( 60, 138, 232, 255 ) );
        ASSERT_FALSE( GUI::CChartPalette::Validate( near, true ).passed );
        // red next to green is the classic failure under deuteranopia
        GUI::CChartPalette::TColourVector redGreen;
        redGreen.push_back( GUI::CColour( 200, 60, 50, 255 ) );
        redGreen.push_back( GUI::CColour( 90, 140, 40, 255 ) );
        ASSERT_FALSE( GUI::CChartPalette::Validate( redGreen, true ).passed );
        ASSERT_TRUE( 0.0 == GUI::CChartPalette::DeltaE( near[ 0 ], near[ 0 ], 2 ) );
        // grey has no chroma
        GUI::CChartPalette::TColourVector grey;
        grey.push_back( GUI::CColour( 128, 128, 128, 255 ) );
        ASSERT_FALSE( GUI::CChartPalette::Validate( grey, true ).passed );

        GUI::CChartPalette::TColourVector parsed;
        ASSERT_TRUE( GUI::CChartPalette::FromStringList( GUI::CChartPalette::ToStringList( GUI::CChartPalette::GetLightPalette() ), parsed ) );
        ASSERT_TRUE( parsed == GUI::CChartPalette::GetLightPalette() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: drawn by the software reference renderer the plot has its axis, grid and legend, the line is in its series colour, and nothing is rebuilt while nothing changes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CWidget* screen = MakeScreen( context );
            GUI::CChartView* chart = MakeChart( context, screen, "Chart", 20, 10, 600, 300 );
            ASSERT_TRUE( GUCEF_NULL != chart );
            if ( GUCEF_NULL != chart )
            {
                chart->SetBoolProperty( GUI::EWidgetProperty_CrosshairVisible, false );
                const Int64 start = GUI::CChartAxis::MakeTime( 2024, 1, 1 );
                GUCEF::vector< Int64 > x;
                GUCEF::vector< Float64 > flat, rising;
                for ( UInt32 i=0; i<=100; ++i )
                {
                    x.push_back( start + (Int64) i * Day );
                    flat.push_back( 50.0 );
                    rising.push_back( (Float64) i );
                }
                UInt32 flatIndex = chart->AddSeries( "Flat", GUI::CChartSeries::SERIESTYPE_LINE );
                UInt32 risingIndex = chart->AddSeries( "Rising", GUI::CChartSeries::SERIESTYPE_LINE );
                chart->SetSeriesData( flatIndex, &x[ 0 ], &flat[ 0 ], (UInt32) x.size() );
                chart->SetSeriesData( risingIndex, &x[ 0 ], &rising[ 0 ], (UInt32) x.size() );

                IMAGE::TPixelMapPtr pixels;
                ASSERT_TRUE( Capture( context, pixels ) );
                const GUI::CRect& plot = chart->GetPlotRect();
                ASSERT_TRUE( 600 == chart->GetRect().width && plot.width > 400 );
                const Int32 ox = chart->GetRect().left;
                const Int32 oy = chart->GetRect().top;
                const GUI::CColour axisColour = chart->GetColourProperty( GUI::EWidgetProperty_AxisColour );
                const GUI::CColour gridColour = chart->GetColourProperty( GUI::EWidgetProperty_GridColour );
                const GUI::CColour background = chart->GetColourProperty( GUI::EWidgetProperty_BackgroundColour );
                const GUI::CColour first = GUI::CChartPalette::GetDarkPalette()[ 0 ];
                const GUI::CColour second = GUI::CChartPalette::GetDarkPalette()[ 1 ];

                // the axis line along the bottom of the plot, a grid line at a value tick
                ASSERT_TRUE( SameRgb( axisColour, PixelAt( pixels, ox + plot.left + plot.width / 3, oy + plot.Bottom() - 1 ) ) );
                const GUI::CChartAxis::TTickVector& ticks = chart->GetValueAxis().GetTicks();
                ASSERT_TRUE( ticks.size() >= 3 );
                Int32 gridY = chart->ValueToPixel( ticks[ 1 ].value );
                ASSERT_TRUE( SameRgb( gridColour, PixelAt( pixels, ox + plot.left + 5, oy + gridY ) ) );
                // the background between the lines
                ASSERT_TRUE( SameRgb( background, PixelAt( pixels, ox + plot.left + 5, oy + gridY - 3 ) ) );

                // the flat line at its value, in the first palette colour; the rising one at its own place in the second
                const Int32 middle = chart->XToPixel( x[ 25 ] );
                ASSERT_TRUE( SameRgb( first, PixelAt( pixels, ox + middle, oy + chart->ValueToPixel( 50.0 ) ) ) );
                ASSERT_TRUE( SameRgb( second, PixelAt( pixels, ox + middle, oy + chart->ValueToPixel( 25.0 ) ) ) );
                // the legend swatches, top left
                ASSERT_TRUE( SameRgb( first, PixelAt( pixels, ox + 8 + 5, oy + 8 + 7 ) ) );

                // nothing changes: no rebuild, no new drawing
                UInt32 revision = chart->GetRevision();
                context->RenderFrame( 16 );
                context->RenderFrame( 16 );
                ASSERT_TRUE( revision == chart->GetRevision() );

                // a change of data or of a property rebuilds once
                chart->SetProperty( GUI::EWidgetProperty_Title, "Two lines" );
                context->RenderFrame( 16 );
                ASSERT_TRUE( revision + 1 == chart->GetRevision() );
                context->RenderFrame( 16 );
                ASSERT_TRUE( revision + 1 == chart->GetRevision() );

                // a new size is drawn in the same frame and the plot follows it
                chart->SetSize( GUI::CUVector( GUI::CUDim( 0.0f, 400 ), GUI::CUDim( 0.0f, 300 ) ) );
                ASSERT_TRUE( Capture( context, pixels ) );
                ASSERT_TRUE( 400 == chart->GetRect().width && chart->GetPlotRect().Right() <= 400 && chart->GetPlotRect().Right() > 350 );
                ASSERT_TRUE( SameRgb( first, PixelAt( pixels, ox + chart->XToPixel( x[ 25 ] ), oy + chart->ValueToPixel( 50.0 ) ) ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: stacked bars are drawn with segments as tall as their values in pixels, a line of the surface between them, side by side with another stack group" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CWidget* screen = MakeScreen( context );
            GUI::CChartView* chart = MakeChart( context, screen, "Income", 0, 0, 600, 400 );
            ASSERT_TRUE( GUCEF_NULL != chart );
            if ( GUCEF_NULL != chart )
            {
                chart->SetBoolProperty( GUI::EWidgetProperty_CrosshairVisible, false );
                chart->SetBoolProperty( GUI::EWidgetProperty_LegendVisible, false );
                Int64 months[ 3 ];
                months[ 0 ] = GUI::CChartAxis::MakeTime( 2024, 1, 15 );
                months[ 1 ] = GUI::CChartAxis::MakeTime( 2024, 2, 15 );
                months[ 2 ] = GUI::CChartAxis::MakeTime( 2024, 3, 15 );
                const Float64 qualified[] = { 10.0, 20.0, 30.0 };
                const Float64 ordinary[] = { 5.0, 5.0, 5.0 };
                UInt32 q = chart->AddSeries( "Qualified", GUI::CChartSeries::SERIESTYPE_BAR );
                UInt32 o = chart->AddSeries( "Ordinary", GUI::CChartSeries::SERIESTYPE_BAR );
                chart->GetSeries( q )->SetStackGroup( "income" );
                chart->GetSeries( o )->SetStackGroup( "income" );
                chart->SetSeriesData( q, months, qualified, 3 );
                chart->SetSeriesData( o, months, ordinary, 3 );

                IMAGE::TPixelMapPtr pixels;
                ASSERT_TRUE( Capture( context, pixels ) );
                const GUI::CRect& plot = chart->GetPlotRect();
                const GUI::CColour first = GUI::CChartPalette::GetDarkPalette()[ 0 ];
                const GUI::CColour second = GUI::CChartPalette::GetDarkPalette()[ 1 ];
                ASSERT_TRUE( chart->GetValueAxis().GetMin() == 0.0 && chart->GetValueAxis().GetMax() >= 35.0 );

                for ( UInt32 m=0; m<3; ++m )
                {
                    const Int32 x = chart->XToPixel( months[ m ] );
                    const Int32 zero = chart->ValueToPixel( 0.0 );
                    const Int32 lower = chart->ValueToPixel( qualified[ m ] );
                    const Int32 upper = chart->ValueToPixel( qualified[ m ] + ordinary[ m ] );
                    ASSERT_TRUE( zero - lower == CountInColumn( pixels, x, plot.top, plot.Bottom(), first ) );
                    ASSERT_TRUE( lower - upper - 1 == CountInColumn( pixels, x, plot.top, plot.Bottom(), second ) );
                    // the gap of one line of the surface between the segments
                    ASSERT_TRUE( !SameRgb( first, PixelAt( pixels, x, lower - 1 ) ) && !SameRgb( second, PixelAt( pixels, x, lower - 1 ) ) );
                    ASSERT_TRUE( SameRgb( second, PixelAt( pixels, x, upper ) ) && SameRgb( first, PixelAt( pixels, x, zero - 1 ) ) );
                }

                // the bars are wide, the slot is a month
                const Int32 centre = chart->XToPixel( months[ 1 ] );
                ASSERT_TRUE( SameRgb( first, PixelAt( pixels, centre - 20, chart->ValueToPixel( 1.0 ) ) ) );

                // a series of its own stack group stands next to the stack, both narrower
                const Float64 other[] = { 12.0, 12.0, 12.0 };
                UInt32 r = chart->AddSeries( "Return of capital", GUI::CChartSeries::SERIESTYPE_BAR );
                chart->SetSeriesData( r, months, other, 3 );
                ASSERT_TRUE( Capture( context, pixels ) );
                const GUI::CColour third = GUI::CChartPalette::GetDarkPalette()[ 2 ];
                const Int32 y = chart->ValueToPixel( 6.0 );
                Int32 firstRight = -1, thirdLeft = 100000;
                for ( Int32 px=centre - 60; px<=centre + 60; ++px )
                {
                    if ( SameRgb( first, PixelAt( pixels, px, y ) ) && px > firstRight )
                        firstRight = px;
                    if ( SameRgb( third, PixelAt( pixels, px, y ) ) && px < thirdLeft )
                        thirdLeft = px;
                }
                ASSERT_TRUE( firstRight >= 0 && thirdLeft < 100000 && firstRight < centre && thirdLeft > firstRight );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: a series of 20,000 points reduced to four per pixel column draws exactly the pixels of all its points" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            GUI::CWidget* screen = MakeScreen( context );
            GUI::CChartView* chart = MakeChart( context, screen, "Chart", 0, 0, 600, 400 );
            GUI::CWidget* referenceWidget = context->CreateWidget( "DrawSurface", "Reference", screen );
            GUI::CDrawSurface* reference = ( GUCEF_NULL != referenceWidget && referenceWidget->IsRegisteredAs( "DrawSurface" ) ) ? static_cast< GUI::CDrawSurface* >( referenceWidget ) : GUCEF_NULL;
            ASSERT_TRUE( GUCEF_NULL != chart && GUCEF_NULL != reference );
            if ( GUCEF_NULL != chart && GUCEF_NULL != reference )
            {
                // nothing but the line: no grid, axis, labels or crosshair
                chart->SetColourProperty( GUI::EWidgetProperty_GridColour, GUI::CColour( 0, 0, 0, 0 ) );
                chart->SetColourProperty( GUI::EWidgetProperty_AxisColour, GUI::CColour( 0, 0, 0, 0 ) );
                chart->SetBoolProperty( GUI::EWidgetProperty_CrosshairVisible, false );
                const UInt32 count = 20000;
                GUCEF::vector< Int64 > x( count );
                GUCEF::vector< Float64 > values( count );
                CSequence sequence( 11 );
                Float64 value = 1000.0;
                const Int64 start = GUI::CChartAxis::MakeTime( 2020, 1, 1 );
                for ( UInt32 i=0; i<count; ++i )
                {
                    x[ i ] = start + (Int64) i * 15 * 60000;
                    value += ( sequence.Next() - 0.5 ) * 20.0;
                    values[ i ] = value;
                }
                values[ 9000 ] = NaN;
                UInt32 index = chart->AddSeries( "Price", GUI::CChartSeries::SERIESTYPE_LINE );
                chart->SetSeriesData( index, &x[ 0 ], &values[ 0 ], count );

                reference->SetPosition( GUI::CUVector( GUI::CUDim( 0.0f, 0 ), GUI::CUDim( 0.0f, 0 ) ) );
                reference->SetSize( GUI::CUVector( GUI::CUDim( 0.0f, 600 ), GUI::CUDim( 0.0f, 400 ) ) );
                reference->SetColourProperty( GUI::EWidgetProperty_BackgroundColour, chart->GetColourProperty( GUI::EWidgetProperty_BackgroundColour ) );
                reference->SetVisibility( false );

                IMAGE::TPixelMapPtr decimated;
                ASSERT_TRUE( Capture( context, decimated ) );
                const GUI::CRect plot = chart->GetPlotRect();
                ASSERT_TRUE( plot.width < 600 && count > (UInt32) ( 4 * plot.width ) );
                // fewer line commands than points
                ASSERT_TRUE( chart->GetCommands().size() < (size_t) ( 4 * plot.width + 200 ) );

                // the same line through every point, with the mapping of the chart
                const GUI::CColour colour = chart->GetSeriesColour( index );
                reference->Clear();
                reference->PushClip( plot );
                for ( UInt32 i=1; i<count; ++i )
                {
                    if ( values[ i ] != values[ i ] || values[ i-1 ] != values[ i-1 ] )
                        continue;
                    reference->DrawLine( chart->XToPixel( x[ i-1 ] ), chart->ValueToPixel( values[ i-1 ] ), chart->XToPixel( x[ i ] ), chart->ValueToPixel( values[ i ] ), colour, 2 );
                }
                reference->PopClip();
                reference->Commit();
                chart->SetVisibility( false );
                reference->SetVisibility( true );
                IMAGE::TPixelMapPtr full;
                ASSERT_TRUE( Capture( context, full ) );

                UInt32 different = 0, lineCount = 0;
                for ( Int32 y=plot.top; y<plot.Bottom(); ++y )
                {
                    for ( Int32 px=plot.left; px<plot.Right(); ++px )
                    {
                        GUI::CColour a = PixelAt( decimated, px, y );
                        GUI::CColour b = PixelAt( full, px, y );
                        if ( !SameRgb( a, b ) )
                            ++different;
                        if ( SameRgb( a, colour ) )
                            ++lineCount;
                    }
                }
                ASSERT_TRUE( 0 == different );
                ASSERT_TRUE( lineCount > (UInt32) plot.width );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: the pointer over the plot puts a crosshair with a tooltip at the nearest point, leaving the chart takes it away" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            context->RegisterFont( "Test", MakeTestFont( 6, 10 ) );
            GUI::CWidget* screen = MakeScreen( context );
            GUI::CChartView* chart = MakeChart( context, screen, "Chart", 20, 20, 500, 300 );
            ASSERT_TRUE( GUCEF_NULL != chart );
            if ( GUCEF_NULL != chart )
            {
                chart->SetProperty( GUI::EWidgetProperty_Font, "Test" );
                // opaque, so the pixels of the box are its colour exactly
                chart->SetColourProperty( GUI::EWidgetProperty_TooltipColour, GUI::CColour( 38, 38, 36, 255 ) );
                const Int64 start = GUI::CChartAxis::MakeTime( 2024, 6, 1 );
                GUCEF::vector< Int64 > x;
                GUCEF::vector< Float64 > values;
                for ( UInt32 i=0; i<20; ++i )
                {
                    x.push_back( start + (Int64) i * Day );
                    values.push_back( 100.0 + (Float64) ( i % 5 ) * 10.0 );
                }
                UInt32 index = chart->AddSeries( "Price", GUI::CChartSeries::SERIESTYPE_AREA );
                chart->SetSeriesData( index, &x[ 0 ], &values[ 0 ], 20 );
                CCrosshairCount counter( chart );

                IMAGE::TPixelMapPtr pixels;
                ASSERT_TRUE( Capture( context, pixels ) );
                ASSERT_FALSE( chart->HasCrosshair() );
                const GUI::CRect plot = chart->GetPlotRect();
                const Int32 ox = chart->GetRect().left;
                const Int32 oy = chart->GetRect().top;
                // the area under the line is filled with a tint of the series colour
                const GUI::CColour colour = chart->GetSeriesColour( index );
                GUI::CColour under = PixelAt( pixels, ox + chart->XToPixel( x[ 2 ] ), oy + plot.Bottom() - 10 );
                ASSERT_TRUE( !SameRgb( under, chart->GetColourProperty( GUI::EWidgetProperty_BackgroundColour ) ) && !SameRgb( under, colour ) );

                const Int32 pointer = chart->XToPixel( x[ 8 ] ) + 2;
                context->InjectMouseMove( ox + pointer, oy + plot.top + plot.height / 2 );
                ASSERT_TRUE( Capture( context, pixels ) );
                ASSERT_TRUE( chart->HasCrosshair() && x[ 8 ] == chart->GetCrosshairX() && 1 == counter.count );
                ASSERT_TRUE( 1 == chart->GetCrosshairPoints().size() && 130.0 == chart->GetCrosshairPoints()[ 0 ].value );
                const Int32 column = ox + chart->XToPixel( x[ 8 ] );
                const GUI::CColour crosshair = chart->GetColourProperty( GUI::EWidgetProperty_CrosshairColour );
                ASSERT_TRUE( SameRgb( crosshair, PixelAt( pixels, column, oy + plot.top + 2 ) ) );
                // the marker on the line and the tooltip box to its right
                ASSERT_TRUE( SameRgb( colour, PixelAt( pixels, column + 2, oy + chart->ValueToPixel( 130.0 ) ) ) );
                ASSERT_TRUE( SameRgb( chart->GetColourProperty( GUI::EWidgetProperty_TooltipColour ), PixelAt( pixels, column + 14, oy + plot.top + 6 ) ) );

                // out of the chart: the crosshair goes
                context->InjectMouseMove( 630, 470 );
                ASSERT_TRUE( Capture( context, pixels ) );
                ASSERT_FALSE( chart->HasCrosshair() );
                ASSERT_TRUE( 2 == counter.count );
                ASSERT_FALSE( SameRgb( crosshair, PixelAt( pixels, column, oy + plot.top + 2 ) ) );
            }
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Chart: a sample of a value line over three years and monthly income stacked by tax character (a picture with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateNullContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL != context )
        {
            context->SetLogicalResolution( 1200, 800 );
            CORE::CString font = "Test";
            context->RegisterFont( "Test", MakeTestFont( 7, 12 ) );
            if ( LoadFreeTypeFontPlugin() && GUI::CGuiGlobal::Instance()->MountData() )
            {
                GUI::TBitmapFontPtr ui = GUI::CGuiGlobal::Instance()->GetFontRasterizerRegistry().RasterizeFont( GUI::CGuiGlobal::DefaultUiFontPath, 14, GUI::CFontRasterizer::GetLatin1Ranges() );
                if ( !ui.IsNULL() && context->RegisterFont( "UI", ui ) )
                    font = "UI";
            }
            GUI::CWidget* screen = MakeScreen( context );
            screen->SetColourProperty( GUI::EWidgetProperty_BackgroundColour, GUI::CColour( 13, 13, 13, 255 ) );
            GUI::CChartView* value = MakeChart( context, screen, "Value", 10, 10, 1180, 380 );
            GUI::CChartView* income = MakeChart( context, screen, "Income", 10, 400, 1180, 390 );
            ASSERT_TRUE( GUCEF_NULL != value && GUCEF_NULL != income );
            if ( GUCEF_NULL != value && GUCEF_NULL != income )
            {
                value->SetProperty( GUI::EWidgetProperty_Font, font );
                value->SetProperty( GUI::EWidgetProperty_Title, "Portfolio value" );
                value->SetProperty( GUI::EWidgetProperty_ValuePrefix, "$" );
                value->SetBoolProperty( GUI::EWidgetProperty_IncludeZero, false );
                income->SetProperty( GUI::EWidgetProperty_Font, font );
                income->SetProperty( GUI::EWidgetProperty_Title, "Income per month" );
                income->SetProperty( GUI::EWidgetProperty_ValuePrefix, "$" );

                // three years of daily values
                const Int64 start = GUI::CChartAxis::MakeTime( 2023, 1, 1 );
                GUCEF::vector< Int64 > days;
                GUCEF::vector< Float64 > portfolio, invested;
                CSequence sequence( 2026 );
                Float64 level = 250000.0;
                for ( UInt32 d=0; d<3*365; ++d )
                {
                    days.push_back( start + (Int64) d * Day );
                    level *= 1.0 + ( sequence.Next() - 0.48 ) * 0.02;
                    portfolio.push_back( level );
                    invested.push_back( 250000.0 + (Float64) ( d / 30 ) * 2000.0 );
                }
                UInt32 p = value->AddSeries( "Market value", GUI::CChartSeries::SERIESTYPE_AREA );
                UInt32 c = value->AddSeries( "Cost basis", GUI::CChartSeries::SERIESTYPE_LINE );
                value->SetSeriesData( p, &days[ 0 ], &portfolio[ 0 ], (UInt32) days.size() );
                value->SetSeriesData( c, &days[ 0 ], &invested[ 0 ], (UInt32) days.size() );

                // 36 months of income in three tax characters, centred in their month
                GUCEF::vector< Int64 > months;
                GUCEF::vector< Float64 > qualified, ordinary, capital;
                for ( Int32 m=0; m<36; ++m )
                {
                    months.push_back( GUI::CChartAxis::MakeTime( 2023 + m / 12, m % 12 + 1, 15 ) );
                    const bool quarterEnd = 2 == m % 3;
                    qualified.push_back( 600.0 + 15.0 * m + ( quarterEnd ? 900.0 : 0.0 ) );
                    ordinary.push_back( 250.0 + 40.0 * sequence.Next() );
                    capital.push_back( quarterEnd ? 120.0 : 60.0 );
                }
                const char* names[ 3 ] = { "Qualified dividends", "Ordinary dividends", "Return of capital" };
                const GUCEF::vector< Float64 >* data[ 3 ] = { &qualified, &ordinary, &capital };
                for ( UInt32 s=0; s<3; ++s )
                {
                    UInt32 index = income->AddSeries( names[ s ], GUI::CChartSeries::SERIESTYPE_BAR );
                    income->GetSeries( index )->SetStackGroup( "income" );
                    income->SetSeriesData( index, &months[ 0 ], &(*data[ s ])[ 0 ], (UInt32) months.size() );
                }

                IMAGE::TPixelMapPtr pixels;
                ASSERT_TRUE( Capture( context, pixels ) );
                // the pointer on a quarter end month of the income chart
                const Int32 pointerX = income->GetRect().left + income->XToPixel( months[ 20 ] );
                context->InjectMouseMove( pointerX, income->GetRect().top + income->GetPlotRect().top + 40 );
                ASSERT_TRUE( Capture( context, pixels ) );
                ASSERT_TRUE( income->HasCrosshair() && 3 == income->GetCrosshairPoints().size() );
                ASSERT_TRUE( Close( qualified[ 20 ] + ordinary[ 20 ] + capital[ 20 ], income->GetCrosshairPoints()[ 2 ].plotValue ) );
                ASSERT_TRUE( 1200 == pixels->GetWidthInPixels() && 800 == pixels->GetHeightInPixels() );
                WriteShot( pixels, "chart_sample" );
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
