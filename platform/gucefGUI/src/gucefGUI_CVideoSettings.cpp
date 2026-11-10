/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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

#ifndef GUCEF_CORE_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_ESSENTIALS_H
#endif /* GUCEF_CORE_ESSENTIALS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefGUI_CVideoSettings.h"

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

const CString CVideoSettings::ConfigSectionName = "VideoSettings";
const CString CVideoSettings::MovieUpscalingFilter = "filter";
const CString CVideoSettings::MovieUpscalingConverted = "converted";

/*-------------------------------------------------------------------------*/

CVideoSettings::CVideoSettings( void )
    : CIConfigurable()                     ,
      m_isFullscreen( false )              ,
      m_hasVSync( false )                  ,
      m_antiAliasingFactor( 0 )            ,
      m_displayMode()                      ,
      m_presentMode( PRESENTMODE_NEAREST ) ,
      m_presentFilter()                    ,
      m_presentFilterParameters()          ,
      m_imageUpscaler()                    ,
      m_movieUpscaling()                   ,
      m_renderScale( 1 )
{GUCEF_TRACE;

    m_displayMode.depthInBits = 32;
    m_displayMode.frequency = 60;
    m_displayMode.widthInPixels = 800;
    m_displayMode.heightInPixels = 600;
}

/*-------------------------------------------------------------------------*/
    
CVideoSettings::CVideoSettings( const CVideoSettings& src )
    : CIConfigurable()                                 ,
      m_isFullscreen( src.m_isFullscreen )             ,
      m_hasVSync( src.m_hasVSync )                     ,
      m_antiAliasingFactor( src.m_antiAliasingFactor ) ,
      m_displayMode( src.m_displayMode )               ,
      m_presentMode( src.m_presentMode )               ,
      m_presentFilter( src.m_presentFilter )           ,
      m_presentFilterParameters( src.m_presentFilterParameters ) ,
      m_imageUpscaler( src.m_imageUpscaler )           ,
      m_movieUpscaling( src.m_movieUpscaling )         ,
      m_renderScale( src.m_renderScale )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CVideoSettings::~CVideoSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
    
CVideoSettings&
CVideoSettings::operator=( const CVideoSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_isFullscreen = src.m_isFullscreen;
        m_hasVSync = src.m_hasVSync;
        m_antiAliasingFactor = src.m_antiAliasingFactor;
        m_displayMode = src.m_displayMode;
        m_presentMode = src.m_presentMode;
        m_presentFilter = src.m_presentFilter;
        m_presentFilterParameters = src.m_presentFilterParameters;
        m_imageUpscaler = src.m_imageUpscaler;
        m_movieUpscaling = src.m_movieUpscaling;
        m_renderScale = src.m_renderScale;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/
    
void
CVideoSettings::SetFullscreenState( const bool isFullscreen )
{GUCEF_TRACE;

    m_isFullscreen = isFullscreen;
}

/*-------------------------------------------------------------------------*/
    
bool
CVideoSettings::GetFullscreenState( void ) const
{GUCEF_TRACE;

    return m_isFullscreen;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetVSyncState( const bool hasVSync )
{GUCEF_TRACE;

    m_hasVSync = hasVSync;
}

/*-------------------------------------------------------------------------*/
    
bool
CVideoSettings::GetVSyncState( void ) const
{GUCEF_TRACE;

    return m_hasVSync;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetResolutionWidthInPixels( const UInt32 widthInPixels )
{GUCEF_TRACE;

    m_displayMode.widthInPixels = widthInPixels;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoSettings::GetResolutionWidthInPixels( void ) const
{GUCEF_TRACE;

    return m_displayMode.widthInPixels;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetResolutionHeightInPixels( const UInt32 heightInPixels )
{GUCEF_TRACE;

    m_displayMode.heightInPixels = heightInPixels;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoSettings::GetResolutionHeightInPixels( void ) const
{GUCEF_TRACE;

    return m_displayMode.heightInPixels;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetResolutionDepthInBits( const UInt32 depthInBits )
{GUCEF_TRACE;

    m_displayMode.depthInBits = depthInBits;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoSettings::GetResolutionDepthInBits( void ) const
{GUCEF_TRACE;

    return m_displayMode.depthInBits;
}

/*-------------------------------------------------------------------------*/
    
void
CVideoSettings::SetResolution( const UInt32 widthInPixels  ,
                               const UInt32 heightInPixels ,
                               const UInt32 depthInPixels  )
{GUCEF_TRACE;

    m_displayMode.widthInPixels = widthInPixels;
    m_displayMode.heightInPixels = heightInPixels;
    m_displayMode.depthInBits = depthInPixels;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::GetResolution( UInt32& widthInPixels  ,
                               UInt32& heightInPixels ,
                               UInt32& depthInPixels  ) const
{GUCEF_TRACE;

    widthInPixels = m_displayMode.widthInPixels;
    heightInPixels = m_displayMode.heightInPixels;
    depthInPixels = m_displayMode.depthInBits;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetAntiAliasingFactor( const UInt32 newFactor )
{GUCEF_TRACE;

    m_antiAliasingFactor = newFactor;
}

/*-------------------------------------------------------------------------*/
    
UInt32
CVideoSettings::GetAntiAliasingFactor( void ) const
{GUCEF_TRACE;

    return m_antiAliasingFactor;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetFrequency( const UInt32 frequency )
{GUCEF_TRACE;

    m_displayMode.frequency = frequency;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoSettings::GetFrequency( void ) const
{GUCEF_TRACE;

    return m_displayMode.frequency;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetPresentMode( TPresentMode presentMode )
{GUCEF_TRACE;

    m_presentMode = presentMode;
}

/*-------------------------------------------------------------------------*/

TPresentMode
CVideoSettings::GetPresentMode( void ) const
{GUCEF_TRACE;

    return m_presentMode;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetPresentFilter( const CString& filterName )
{GUCEF_TRACE;

    m_presentFilter = filterName;
}

/*-------------------------------------------------------------------------*/

const CString&
CVideoSettings::GetPresentFilter( void ) const
{GUCEF_TRACE;

    return m_presentFilter;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetPresentFilterParameters( const TParameterMap& parameters )
{GUCEF_TRACE;

    m_presentFilterParameters = parameters;
}

/*-------------------------------------------------------------------------*/

const CVideoSettings::TParameterMap&
CVideoSettings::GetPresentFilterParameters( void ) const
{GUCEF_TRACE;

    return m_presentFilterParameters;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetImageUpscaler( const CString& upscalerName )
{GUCEF_TRACE;

    m_imageUpscaler = upscalerName;
}

/*-------------------------------------------------------------------------*/

const CString&
CVideoSettings::GetImageUpscaler( void ) const
{GUCEF_TRACE;

    return m_imageUpscaler;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetMovieUpscaling( const CString& movieUpscaling )
{GUCEF_TRACE;

    m_movieUpscaling = movieUpscaling;
}

/*-------------------------------------------------------------------------*/

const CString&
CVideoSettings::GetMovieUpscaling( void ) const
{GUCEF_TRACE;

    return m_movieUpscaling;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetRenderScale( UInt32 renderScale )
{GUCEF_TRACE;

    m_renderScale = renderScale;
}

/*-------------------------------------------------------------------------*/

UInt32
CVideoSettings::GetRenderScale( void ) const
{GUCEF_TRACE;

    return m_renderScale;
}

/*-------------------------------------------------------------------------*/

CString
CVideoSettings::PresentModeToString( TPresentMode presentMode )
{GUCEF_TRACE;

    switch ( presentMode )
    {
        case PRESENTMODE_LINEAR: return "linear";
        case PRESENTMODE_INTEGER: return "integer";
        case PRESENTMODE_STRETCH: return "stretch";
        case PRESENTMODE_NEAREST:
        default: return "nearest";
    }
}

/*-------------------------------------------------------------------------*/

TPresentMode
CVideoSettings::StringToPresentMode( const CString& presentModeStr, TPresentMode defaultMode )
{GUCEF_TRACE;

    CString mode = presentModeStr.Lowercase();
    if ( "nearest" == mode )
        return PRESENTMODE_NEAREST;
    if ( "linear" == mode )
        return PRESENTMODE_LINEAR;
    if ( "integer" == mode )
        return PRESENTMODE_INTEGER;
    if ( "stretch" == mode )
        return PRESENTMODE_STRETCH;
    return defaultMode;
}

/*-------------------------------------------------------------------------*/

bool
CVideoSettings::SaveConfig( GUCEF::CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* section = tree.Structure( ConfigSectionName, '/' );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "fullscreen", m_isFullscreen );
    section->SetAttribute( "vsync", m_hasVSync );
    section->SetAttribute( "width", m_displayMode.widthInPixels );
    section->SetAttribute( "height", m_displayMode.heightInPixels );
    section->SetAttribute( "depth", m_displayMode.depthInBits );
    section->SetAttribute( "frequency", m_displayMode.frequency );
    section->SetAttribute( "antiAliasing", m_antiAliasingFactor );
    section->SetAttribute( "presentMode", PresentModeToString( m_presentMode ) );
    section->SetAttribute( "presentFilter", m_presentFilter );
    section->SetAttribute( "imageUpscaler", m_imageUpscaler );
    section->SetAttribute( "movieUpscaling", m_movieUpscaling );
    section->SetAttribute( "renderScale", m_renderScale );

    section->DelChild( "PresentFilterParameters" );
    if ( !m_presentFilterParameters.empty() )
    {
        CORE::CDataNode* parameters = section->AddChild( "PresentFilterParameters" );
        TParameterMap::const_iterator i = m_presentFilterParameters.begin();
        while ( i != m_presentFilterParameters.end() )
        {
            parameters->SetAttribute( (*i).first, (*i).second );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CVideoSettings::LoadConfig( const GUCEF::CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    m_isFullscreen = section->GetAttributeValueOrChildValueByName( "fullscreen" ).AsBool( m_isFullscreen, true );
    m_hasVSync = section->GetAttributeValueOrChildValueByName( "vsync" ).AsBool( m_hasVSync, true );
    m_displayMode.widthInPixels = section->GetAttributeValueOrChildValueByName( "width" ).AsUInt32( m_displayMode.widthInPixels, true );
    m_displayMode.heightInPixels = section->GetAttributeValueOrChildValueByName( "height" ).AsUInt32( m_displayMode.heightInPixels, true );
    m_displayMode.depthInBits = section->GetAttributeValueOrChildValueByName( "depth" ).AsUInt32( m_displayMode.depthInBits, true );
    m_displayMode.frequency = section->GetAttributeValueOrChildValueByName( "frequency" ).AsUInt32( m_displayMode.frequency, true );
    m_antiAliasingFactor = section->GetAttributeValueOrChildValueByName( "antiAliasing" ).AsUInt32( m_antiAliasingFactor, true );
    m_presentMode = StringToPresentMode( section->GetAttributeValueOrChildValueByName( "presentMode" ).AsString( CString::Empty, true ), m_presentMode );
    m_presentFilter = section->GetAttributeValueOrChildValueByName( "presentFilter" ).AsString( m_presentFilter, true );
    m_imageUpscaler = section->GetAttributeValueOrChildValueByName( "imageUpscaler" ).AsString( m_imageUpscaler, true );
    m_movieUpscaling = section->GetAttributeValueOrChildValueByName( "movieUpscaling" ).AsString( m_movieUpscaling, true );
    m_renderScale = section->GetAttributeValueOrChildValueByName( "renderScale" ).AsUInt32( m_renderScale, true );

    const CORE::CDataNode* parameters = section->FindChild( "PresentFilterParameters" );
    if ( GUCEF_NULL != parameters )
    {
        m_presentFilterParameters.clear();
        for ( UInt32 i=0; i<parameters->GetAttCount(); ++i )
        {
            const CORE::CDataNode::TKeyValuePair* parameter = parameters->GetAttribute( i );
            if ( GUCEF_NULL != parameter )
                m_presentFilterParameters[ parameter->first ] = parameter->second.AsString();
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVideoSettings::SetDisplayMode( const TDisplayMode& displayMode )
{GUCEF_TRACE;

    m_displayMode = displayMode;
}

/*-------------------------------------------------------------------------*/
    
void
CVideoSettings::GetDisplayMode( TDisplayMode& displayMode ) const
{GUCEF_TRACE;

    displayMode = m_displayMode;
}

/*-------------------------------------------------------------------------*/

bool
CVideoSettings::RetrieveSettingsFromOS( CVideoSettings& settings )
{GUCEF_TRACE;

    TDisplayMode currentMode;
    if ( !CVideoOptions::RetrieveCurrentDisplayModeFromOS( currentMode ) )
        return false;
    settings.SetDisplayMode( currentMode );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CVideoSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUI::CVideoSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
