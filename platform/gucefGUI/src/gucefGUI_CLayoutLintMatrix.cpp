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

#include <stdio.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#include "gucefGUI_CLayoutLintMatrix.h"

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

const UInt32 DefaultSizes[] = { 640, 480, 1024, 768, 1280, 720, 1600, 900, 1920, 1080, 3440, 1440 };

CString
ScaleText( Float32 value )
{
    char text[ 32 ];
    snprintf( text, sizeof( text ), "%g", (double) value );
    return text;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLayoutLintMatrix::SCombination::SCombination( void )
    : width( 0 )
    , height( 0 )
    , renderScale( 1 )
    , textScale( 1.0f )
    , pseudoLocalized( false )
    , lineSpacing( 1.0f )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLayoutLintMatrix::CLayoutLintMatrix( void )
    : m_sizes()
    , m_textScaleSupported( false )
    , m_combinations()
    , m_pseudoLocalizer()
    , m_saved( false )
    , m_savedWidth( 0 )
    , m_savedHeight( 0 )
    , m_savedRenderScale( 1 )
    , m_savedTransform( GUCEF_NULL )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32) ( sizeof( DefaultSizes ) / sizeof( DefaultSizes[ 0 ] ) ); ++i )
        m_sizes.push_back( DefaultSizes[ i ] );
    Build();
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintMatrix::ClearSizes( void )
{GUCEF_TRACE;

    m_sizes.clear();
    Build();
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintMatrix::AddSize( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    m_sizes.push_back( width );
    m_sizes.push_back( height );
    Build();
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintMatrix::SetMinimumSize( UInt32 width, UInt32 height )
{GUCEF_TRACE;

    if ( 0 == width && 0 == height )
        return;
    TUInt32Vector kept;
    kept.push_back( width );
    kept.push_back( height );
    for ( UInt32 i=0; i+1<(UInt32) m_sizes.size(); i+=2 )
    {
        if ( m_sizes[ i ] >= width && m_sizes[ i+1 ] >= height && !( m_sizes[ i ] == width && m_sizes[ i+1 ] == height ) )
        {
            kept.push_back( m_sizes[ i ] );
            kept.push_back( m_sizes[ i+1 ] );
        }
    }
    m_sizes = kept;
    Build();
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintMatrix::SetTextScaleSupported( bool supported )
{GUCEF_TRACE;

    m_textScaleSupported = supported;
    Build();
}

/*-------------------------------------------------------------------------*/

bool
CLayoutLintMatrix::IsTextScaleSupported( void ) const
{GUCEF_TRACE;

    return m_textScaleSupported;
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintMatrix::Build( void )
{GUCEF_TRACE;

    m_combinations.clear();
    const UInt32* sizes = m_sizes.empty() ? DefaultSizes : &m_sizes[ 0 ];
    const UInt32 count = m_sizes.empty() ? (UInt32) ( sizeof( DefaultSizes ) / sizeof( DefaultSizes[ 0 ] ) ) : (UInt32) m_sizes.size();

    // every size plain, then every size pseudo-localised
    for ( UInt32 pass=0; pass<2; ++pass )
    {
        for ( UInt32 i=0; i+1<count; i+=2 )
        {
            TCombination combination;
            combination.width = sizes[ i ];
            combination.height = sizes[ i+1 ];
            combination.pseudoLocalized = 1 == pass;
            m_combinations.push_back( combination );
        }
    }

    // the stresses at the common desktop size (or the largest size there is, when that is smaller)
    TCombination stress;
    stress.width = 1920;
    stress.height = 1080;
    if ( !m_sizes.empty() )
    {
        bool found = false;
        for ( UInt32 i=0; i+1<count; i+=2 )
            if ( 1920 == sizes[ i ] && 1080 == sizes[ i+1 ] )
                found = true;
        if ( !found )
        {
            stress.width = sizes[ count - 2 ];
            stress.height = sizes[ count - 1 ];
        }
    }
    TCombination scaled = stress;
    scaled.renderScale = 2;
    m_combinations.push_back( scaled );
    TCombination spaced = stress;
    spaced.pseudoLocalized = true;
    spaced.lineSpacing = 1.5f;
    m_combinations.push_back( spaced );
    if ( m_textScaleSupported )
    {
        TCombination larger = stress;
        larger.textScale = 2.0f;
        m_combinations.push_back( larger );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CLayoutLintMatrix::GetCombinationCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_combinations.size();
}

/*-------------------------------------------------------------------------*/

const CLayoutLintMatrix::TCombination&
CLayoutLintMatrix::GetCombination( UInt32 index ) const
{GUCEF_TRACE;

    if ( index >= (UInt32) m_combinations.size() )
        index = 0;
    return m_combinations[ index ];
}

/*-------------------------------------------------------------------------*/

CString
CLayoutLintMatrix::DescribeCombination( UInt32 index ) const
{GUCEF_TRACE;

    if ( index >= (UInt32) m_combinations.size() )
        return CString::Empty;
    const TCombination& combination = m_combinations[ index ];
    CString text = CORE::UInt32ToString( combination.width ) + "x" + CORE::UInt32ToString( combination.height );
    if ( combination.pseudoLocalized )
        text += " pseudo";
    if ( combination.lineSpacing > 1.0f )
        text += " line spacing " + ScaleText( combination.lineSpacing );
    if ( combination.renderScale > 1 )
        text += " render scale " + CORE::UInt32ToString( combination.renderScale );
    if ( combination.textScale != 1.0f )
        text += " text scale " + ScaleText( combination.textScale );
    return text;
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintMatrix::ApplyCombination( UInt32 index, CGUIContext& context )
{GUCEF_TRACE;

    if ( !m_saved )
    {
        m_saved = true;
        m_savedWidth = context.GetLogicalWidth();
        m_savedHeight = context.GetLogicalHeight();
        m_savedRenderScale = context.GetRenderScale();
        m_savedTransform = context.GetDisplayTextTransform();
    }
    const TCombination& combination = GetCombination( index );
    context.SetLogicalResolution( combination.width, combination.height );
    context.SetRenderScale( combination.renderScale );
    if ( combination.pseudoLocalized )
    {
        m_pseudoLocalizer.SetLineSpacing( combination.lineSpacing );
        context.SetDisplayTextTransform( GUCEF_NULL );       // the line spacing may have changed: the context lays out again
        context.SetDisplayTextTransform( &m_pseudoLocalizer );
    }
    else
    {
        context.SetDisplayTextTransform( m_savedTransform );
    }
    context.InvalidateLayout();
}

/*-------------------------------------------------------------------------*/

void
CLayoutLintMatrix::Restore( CGUIContext& context )
{GUCEF_TRACE;

    if ( !m_saved )
        return;
    context.SetLogicalResolution( m_savedWidth, m_savedHeight );
    context.SetRenderScale( m_savedRenderScale );
    context.SetDisplayTextTransform( m_savedTransform );
    context.InvalidateLayout();
    m_saved = false;
}

/*-------------------------------------------------------------------------*/

CPseudoLocalizer&
CLayoutLintMatrix::GetPseudoLocalizer( void )
{GUCEF_TRACE;

    return m_pseudoLocalizer;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
