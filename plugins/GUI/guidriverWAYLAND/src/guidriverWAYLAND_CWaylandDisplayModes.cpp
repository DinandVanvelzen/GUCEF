/*
 *  guidriverWAYLAND: GUCEF window driver for Wayland: native windows with OpenGL through EGL
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

#include "guidriverWAYLAND_CWaylandDisplayModes.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWAYLAND {

using namespace PLUGINGLUE::WAYLAND;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

GUI::CVideoOptions::TDisplayMode
MakeMode( UInt32 width, UInt32 height, UInt32 frequency )
{
    GUI::CVideoOptions::TDisplayMode mode;
    mode.widthInPixels = width;
    mode.heightInPixels = height;
    mode.depthInBits = 24;
    mode.frequency = frequency;
    return mode;
}

/** The size of a mode of a monitor as a window sees it: the logical size when the monitor told it and its mode is the current one */
void
SizeOfMode( const SWaylandOutput& output, const SWaylandOutputMode& mode, UInt32& width, UInt32& height )
{
    width = (UInt32) mode.width;
    height = (UInt32) mode.height;
    if ( output.hasLogicalSize && output.logicalWidth > 0 && output.logicalHeight > 0 && 0 != ( mode.flags & GUCEF_WL_WL_OUTPUT_MODE_CURRENT ) )
    {
        width = (UInt32) output.logicalWidth;
        height = (UInt32) output.logicalHeight;
    }
}

}

/*-------------------------------------------------------------------------*/

UInt32
CWaylandDisplayModes::FrequencyOf( Int32 refreshInMilliHertz )
{GUCEF_TRACE;

    if ( refreshInMilliHertz <= 0 )
        return 60;
    return (UInt32) ( ( refreshInMilliHertz + 500 ) / 1000 );
}

/*-------------------------------------------------------------------------*/

const SWaylandOutput*
CWaylandDisplayModes::SelectPrimaryOutput( const CWaylandConnection& connection )
{GUCEF_TRACE;

    const SWaylandOutput* first = GUCEF_NULL;
    for ( UInt32 i=0; i<connection.GetOutputCount(); ++i )
    {
        const SWaylandOutput* output = connection.GetOutput( i );
        if ( GUCEF_NULL == output->GetCurrentMode() )
            continue;
        if ( GUCEF_NULL == first )
            first = output;
        const Int32 x = output->hasLogicalSize ? output->logicalX : output->x;
        const Int32 y = output->hasLogicalSize ? output->logicalY : output->y;
        if ( 0 == x && 0 == y )
            return output;
    }
    return first;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandDisplayModes::GetCurrentMode( const CWaylandConnection& connection, GUI::CVideoOptions::TDisplayMode& mode )
{GUCEF_TRACE;

    const SWaylandOutput* output = SelectPrimaryOutput( connection );
    if ( GUCEF_NULL == output )
        return false;
    const SWaylandOutputMode* current = output->GetCurrentMode();
    UInt32 width = 0, height = 0;
    SizeOfMode( *output, *current, width, height );
    if ( 0 == width || 0 == height )
        return false;
    mode = MakeMode( width, height, FrequencyOf( current->refreshInMilliHertz ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWaylandDisplayModes::GetOptions( const CWaylandConnection& connection, GUI::CVideoOptions& options )
{GUCEF_TRACE;

    const SWaylandOutput* output = SelectPrimaryOutput( connection );
    if ( GUCEF_NULL == output )
        return false;
    options.Clear();
    for ( UInt32 i=0; i<output->modes.size(); ++i )
    {
        UInt32 width = 0, height = 0;
        SizeOfMode( *output, output->modes[ i ], width, height );
        if ( 0 == width || 0 == height )
            continue;
        options.AddDisplayMode( MakeMode( width, height, FrequencyOf( output->modes[ i ].refreshInMilliHertz ) ) );
    }
    return !options.GetDisplayModes().empty();
}

/*-------------------------------------------------------------------------*/

namespace {

/** Asks the compositor for the duration of the question: the connection is the shared one, so it stays when a window has it */
bool
RetrieveOptions( GUI::CVideoOptions& options )
{
    CWaylandConnection* connection = CWaylandConnection::Acquire();
    if ( GUCEF_NULL == connection )
        return false;
    const bool result = CWaylandDisplayModes::GetOptions( *connection, options );
    CWaylandConnection::Release( connection );
    return result;
}

bool
RetrieveCurrentMode( GUI::CVideoOptions::TDisplayMode& mode )
{
    CWaylandConnection* connection = CWaylandConnection::Acquire();
    if ( GUCEF_NULL == connection )
        return false;
    const bool result = CWaylandDisplayModes::GetCurrentMode( *connection, mode );
    CWaylandConnection::Release( connection );
    return result;
}

}

/*-------------------------------------------------------------------------*/

const GUI::CVideoOptions::SDisplayModeSource&
CWaylandDisplayModes::GetSource( void )
{GUCEF_TRACE;

    static GUI::CVideoOptions::SDisplayModeSource source;
    source.retrieveOptions = RetrieveOptions;
    source.retrieveCurrentMode = RetrieveCurrentMode;
    return source;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
