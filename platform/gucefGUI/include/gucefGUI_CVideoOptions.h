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
 
#ifndef GUCEF_GUI_CVIDEOOPTIONS_H
#define GUCEF_GUI_CVIDEOOPTIONS_H 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/ 

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

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

class CVideoSettings;

/*-------------------------------------------------------------------------*/

/**
 *  The display modes a display offers. Where the platform tells: MS Windows, Linux with X11 (through XRandR when
 *  the system has it), the web (the screen the browser is on). On Android the window always covers the screen
 *  and the system picks its mode: there are no options to retrieve.
 */
class GUCEF_GUI_PUBLIC_CPP CVideoOptions
{
    public:
    
    struct SDisplayMode
    {
        UInt32 widthInPixels;
        UInt32 heightInPixels;
        UInt32 depthInBits;
        UInt32 frequency;     /**< display frequency in frames per second */
    };
    typedef struct SDisplayMode TDisplayMode;
    typedef GUCEF::vector< TDisplayMode > TDisplayModeVector;
    
    CVideoOptions( void );
    
    CVideoOptions( const CVideoOptions& src );
    
    virtual ~CVideoOptions();
    
    CVideoOptions& operator=( const CVideoOptions& src );
    
    /**
     *  Checks the given settings against the options to see if
     *  it is a possible option.
     */
    bool AreSettingsPossible( const CVideoSettings& settings ) const;
    
    const TDisplayModeVector& GetDisplayModes( void ) const;
    
    void GetDisplayModesForResolution( const UInt32 widthInPixels  , 
                                       const UInt32 heightInPixels ,
                                       TDisplayModeVector& modes   ) const;
    
    void Clear( void );

    /** Adds a mode, a mode that is there already is not added again */
    void AddDisplayMode( const TDisplayMode& displayMode );

    /**
     *  The usual game resolutions this display can show, smallest first. In a window: those that fit in the
     *  largest display mode (all of them when there are no modes). In full screen: those the display has as
     *  a mode, at the highest frequency it has for them. The largest mode of the display is always in the list, also when it is a size
     *  that is not a usual one, like that of a super wide display.
     */
    void GetCommonResolutions( bool fullscreen, TDisplayModeVector& resolutions ) const;

    /** The usual game resolutions, from 640 x 480 up to 5120 x 2160 and including the wide displays, smallest first */
    static const TDisplayModeVector& GetCommonGameResolutions( void );

    /**
     *  A source of display modes besides that of the platform, for a window system that sees the monitors in a way of its own
     *  (Wayland: the monitors the compositor tells of). A window driver registers one under its name; the application selects
     *  the one of the window system it uses. When the selected source has nothing, or none is selected, the platform is asked.
     */
    struct SDisplayModeSource
    {
        bool ( *retrieveOptions )( CVideoOptions& options );
        bool ( *retrieveCurrentMode )( TDisplayMode& displayMode );
    };

    static void RegisterDisplayModeSource( const CString& name, const SDisplayModeSource& source );

    static void UnregisterDisplayModeSource( const CString& name );

    /** The source RetrieveOptionsFromOS() and RetrieveCurrentDisplayModeFromOS() ask first, by name; empty for the platform only */
    static void SelectDisplayModeSource( const CString& name );

    static CString GetSelectedDisplayModeSource( void );

    /** The display modes of the main display; false when there is no display to ask */
    static bool RetrieveOptionsFromOS( CVideoOptions& options );

    /** The mode the main display is in; false when there is no display to ask */
    static bool RetrieveCurrentDisplayModeFromOS( TDisplayMode& displayMode );
        
    private:

    TDisplayModeVector m_displayModes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CVIDEOOPTIONS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 02-04-2005 :
       - Initial version of this file.

---------------------------------------------------------------------------*/
