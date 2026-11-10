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
 
#ifndef GUCEF_GUI_CVIDEOSETTINGS_H
#define GUCEF_GUI_CVIDEOSETTINGS_H 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/ 

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_GUI_CVIDEOOPTIONS_H
#include "gucefGUI_CVideoOptions.h"
#define GUCEF_GUI_CVIDEOOPTIONS_H
#endif /* GUCEF_GUI_CVIDEOOPTIONS_H ? */

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

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

class GUCEF_GUI_PUBLIC_CPP CVideoSettings : public CORE::CIConfigurable
{
    public:
    
    typedef CVideoOptions::TDisplayMode TDisplayMode;
    typedef GUCEF::map< CString, CString > TParameterMap;

    /** The name of the config section: SaveConfig() writes it in the node it gets, LoadConfig() reads it from there */
    static const CString ConfigSectionName;

    /** Values of the movie upscaling: an upscaling filter on the picture while a movie plays, or upscaled copies made once */
    static const CString MovieUpscalingFilter;
    static const CString MovieUpscalingConverted;
    
    CVideoSettings( void );
    
    CVideoSettings( const CVideoSettings& src );
    
    virtual ~CVideoSettings();
    
    CVideoSettings& operator=( const CVideoSettings& src );
    
    void SetDisplayMode( const TDisplayMode& displayMode );
    
    void GetDisplayMode( TDisplayMode& displayMode ) const;
    
    void SetFullscreenState( const bool isFullscreen );
    
    bool GetFullscreenState( void ) const;

    void SetVSyncState( const bool hasVSync );
    
    bool GetVSyncState( void ) const;
    
    void SetResolutionWidthInPixels( const UInt32 widthInPixels );
    
    UInt32 GetResolutionWidthInPixels( void ) const;
    
    void SetResolutionHeightInPixels( const UInt32 heightInPixels );
    
    UInt32 GetResolutionHeightInPixels( void ) const;
    
    void SetResolutionDepthInBits( const UInt32 depthInBits );
    
    UInt32 GetResolutionDepthInBits( void ) const;
    
    void SetResolution( const UInt32 widthInPixels  ,
                        const UInt32 heightInPixels ,
                        const UInt32 depthInBits    );

    void GetResolution( UInt32& widthInPixels  ,
                        UInt32& heightInPixels ,
                        UInt32& depthInBits    ) const;
                        
    void SetAntiAliasingFactor( const UInt32 newFactor );
    
    UInt32 GetAntiAliasingFactor( void ) const;
    
    void SetFrequency( const UInt32 frequency );
    
    UInt32 GetFrequency( void ) const;

    /** How the logical picture of a GUI context is scaled to the window */
    void SetPresentMode( TPresentMode presentMode );

    TPresentMode GetPresentMode( void ) const;

    /** A present filter of the GUI driver that upscales the picture when it is presented, empty for none */
    void SetPresentFilter( const CString& filterName );

    const CString& GetPresentFilter( void ) const;

    /** Parameters of the present filter by name, like "sharpness" */
    void SetPresentFilterParameters( const TParameterMap& parameters );

    const TParameterMap& GetPresentFilterParameters( void ) const;

    /** An image upscaler for the pictures of the application, empty for none */
    void SetImageUpscaler( const CString& upscalerName );

    const CString& GetImageUpscaler( void ) const;

    /**
     *  How movies are upscaled: empty for not at all (they are shown as they are, scaled with the rest),
     *  MovieUpscalingFilter or MovieUpscalingConverted. What the filter is and how the copies are made is up to
     *  the application, for example a present filter for movies and the image upscaler for the copies
     */
    void SetMovieUpscaling( const CString& movieUpscaling );

    const CString& GetMovieUpscaling( void ) const;

    /**
     *  How many times the logical resolution a GUI context draws at: 1 is the logical resolution itself,
     *  0 lets the application pick one for the window and the image upscaler
     */
    void SetRenderScale( UInt32 renderScale );

    UInt32 GetRenderScale( void ) const;

    static CString PresentModeToString( TPresentMode presentMode );

    /** "nearest", "linear", "integer" or "stretch"; anything else gives 'defaultMode' */
    static TPresentMode StringToPresentMode( const CString& presentModeStr, TPresentMode defaultMode );

    /** Writes the settings as the section ConfigSectionName in 'tree' */
    virtual bool SaveConfig( GUCEF::CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the section ConfigSectionName of 'treeroot', or 'treeroot' itself when it is that section; what it lacks stays as it is */
    virtual bool LoadConfig( const GUCEF::CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    /** The current display mode of the main display; false when there is no display to ask */
    static bool RetrieveSettingsFromOS( CVideoSettings& settings );
        
    private:
    
    bool m_isFullscreen;
    bool m_hasVSync;
    UInt32 m_antiAliasingFactor;
    TDisplayMode m_displayMode;
    TPresentMode m_presentMode;
    CString m_presentFilter;
    TParameterMap m_presentFilterParameters;
    CString m_imageUpscaler;
    CString m_movieUpscaling;
    UInt32 m_renderScale;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CVIDEOSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 02-04-2005 :
       - Initial version of this file.

---------------------------------------------------------------------------*/
