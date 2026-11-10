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

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#define GUCEF_GUI_CWINDOWCONTEXT_H 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_GUI_CIGUICONTEXT_H
#include "gucefGUI_CIGUIContext.h"
#define GUCEF_GUI_CIGUICONTEXT_H
#endif /* GUCEF_GUI_CIGUICONTEXT_H ? */

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

class CGUIContext;

/**
 *  A window of a window driver.
 *
 *  Several GUI contexts can draw in one window, as layers: each has a layer number (CGUIContext::SetLayer()), and
 *  they are drawn from the lowest layer up, in the order they were made when the numbers are equal. The GUI
 *  contexts register themselves.
 */
class GUCEF_GUI_PUBLIC_CPP CWindowContext : public CORE::CObservingNotifier
{        
    public:
    
    static const GUCEF::CORE::CEvent WindowContextRedrawEvent;
    static const GUCEF::CORE::CEvent WindowContextSizeEvent;
    static const GUCEF::CORE::CEvent WindowContextActivateEvent;
    static const GUCEF::CORE::CEvent WindowContextCloseRequestEvent;    /**< the user asked to close the window (its close button); the application decides */

    typedef GUCEF::CORE::TCloneableBool WindowContextActivateEventData;
    
    static void RegisterEvents( void );
    
    public:
    
    CWindowContext( void );

    virtual ~CWindowContext();

    virtual UInt32 GetWidth( void ) const = 0;

    virtual UInt32 GetHeight( void ) const = 0;
    
    virtual UInt32 GetID( void ) const = 0;
    
    virtual bool IsActive( void ) const = 0;
    
    virtual CString GetName( void ) const = 0;

    virtual void SetGuiContext( TGuiContextPtr& context ) = 0;
    
    virtual TGuiContextPtr GetGuiContext( void ) = 0;

    virtual CString GetProperty( const CString& propertyName ) const = 0;

    /** Shows or hides the pointer of the system over the window, for GUIs that draw their own; false when the window cannot */
    virtual bool SetSystemCursorVisible( bool visible );

    /**
     *  Shows the pointer of the system unless the GUI layer that takes the input (the highest that shows something, else
     *  the lowest) draws a picture of its own (CGUIContext::SetCursor()). The window is asked only when that changes
     */
    void UpdateSystemCursor( void );

    typedef GUCEF::vector< CGUIContext* > TGuiContextLayers;

    /** The GUI contexts that draw in the window, from the lowest layer up */
    const TGuiContextLayers& GetGuiLayers( void ) const;

    /** For GUI contexts: they draw in this window, or no longer, or their layer number changed */
    void RegisterGuiLayer( CGUIContext* context );

    void UnregisterGuiLayer( CGUIContext* context );

    void SortGuiLayers( void );

    private:

    TGuiContextLayers m_guiLayers;
    Int32 m_systemCursorShown;      /**< -1 when not known yet */
    
    CWindowContext( const CWindowContext& src );            /**< private because: must be unique */ 
    CWindowContext& operator=( const CWindowContext& src ); /**< private because: must be unique */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CWindowContext, MT::CMutex > TWindowContextPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUI */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 08-10-2005 :
        - Initial version

---------------------------------------------------------------------------*/
