/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_GUI_CGUIDRIVER_H
#define GUCEF_GUI_CGUIDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#include "gucefGUI_CIGUIContext.h"
#define GUCEF_GUI_CIGUICONTEXT_H
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

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


class GUCEF_GUI_PUBLIC_CPP CGUIDriver : public CORE::CObservingNotifier    ,
                                        public CORE::CGloballyConfigurable
{    
    public:

    static const CORE::CEvent WidgetTypeRegisteredEvent;
    static const CORE::CEvent WidgetTypeUnregisteredEvent;    
    static const CORE::CEvent FormTypeRegisteredEvent;
    static const CORE::CEvent FormTypeUnregisteredEvent;
    static const CORE::CEvent FontTypeRegisteredEvent;
    static const CORE::CEvent FontTypeUnregisteredEvent;

    static void RegisterEvents( void );

    public:
    
    typedef GUCEF::set< CString > TStringSet;
    typedef GUCEF::set< TGuiContextPtr > TGUIContextSet;

    /** The version of the driver interface this gucefGUI was built with; a driver of another version is refused */
    static const UInt32 InterfaceVersion;

    virtual ~CGUIDriver();
    
    virtual TGuiContextPtr CreateGUIContext( TWindowContextPtr windowContext ) = 0;
    
    virtual TGUIContextSet GetContextList( void ) = 0;
    
    virtual UInt32 GetContextCount( void ) = 0;
    
    virtual CString GetDriverName( void ) = 0;
    
    virtual TStringSet GetAvailableFormTypes( void ) = 0;
    
    virtual TStringSet GetAvailableWidgetTypes( void ) = 0;
    
    virtual const CString& GetClassTypeName( void ) const;
    
    virtual CString GetDriverProperty( const CString& propertyName ) const = 0;

    virtual TStringSet GetAvailableFonts( void ) = 0;

    virtual bool LoadFontFromAsset( const CString& assetPath ) = 0;

    /** The handler for a widget type, NULL when the driver has none (the context then tries the base types) */
    virtual CWidgetHandler* GetWidgetHandler( const CString& widgetType );

    /**
     *  What the driver supports beyond widget handlers, for editors and tests, for example
     *  "Render", "CaptureFrame", "BitmapFonts", "VectorFonts", "NativeLayouts", "DrawSurface", "DynamicImages"
     */
    virtual TStringSet GetCapabilities( void );

    bool HasCapability( const CString& capability );

    /** The driver interface version the driver was built with; drivers return CGUIDriver::InterfaceVersion */
    virtual UInt32 GetInterfaceVersion( void ) const;
    
    protected:
    
    CGUIDriver( void );
    
    CGUIDriver( const CGUIDriver& src );
    
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_GUI_CGUIDRIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

-----------------------------------------------------------------------------*/