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
 
#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"   
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

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

class CVideoSettings;

/*-------------------------------------------------------------------------*/

class GUCEF_GUI_PUBLIC_CPP CWindowManagerBackend : public CORE::CObservingNotifier ,
                                                   public CORE::CIConfigurable
{
    public:
    
    static const CORE::CEvent PrimaryWindowContextCreatedEvent;
    static const CORE::CEvent PrimaryWindowContextDestroyedEvent;
    static const CORE::CEvent WindowContextCreatedEvent;
    static const CORE::CEvent WindowContextDestroyedEvent;
    
    typedef CORE::CTCloneableObj< TWindowContextPtr >    TWindowContextCreatedEventData;
    typedef CORE::CTCloneableObj< TWindowContextPtr >    TWindowContextDestroyedEventData;
    
    static void RegisterEvents( void );
    
    public:
    
    CWindowManagerBackend( void );
    
    virtual ~CWindowManagerBackend();
    
    virtual TWindowContextPtr CreateWindowContext( const CString& title  ,
                                                   const UInt32 width    ,
                                                   const UInt32 height   ,
                                                   const bool fullscreen ) = 0;

    virtual TWindowContextPtr CreateWindowContext( const CString& title                  ,
                                                   const CVideoSettings& settings        ,
                                                   const CORE::CValueList* params = GUCEF_NULL ) = 0;

    virtual TWindowContextPtr CreateWindowContext( const CString& title           ,
                                                   const UInt32 width             ,
                                                   const UInt32 height            ,
                                                   const bool fullscreen          ,
                                                   const CORE::CValueList& params ) = 0;

    virtual TWindowContextPtr CreateWindowContextViaConfig( const GUCEF::CORE::CDataNode& config ,
                                                            const CString& title = ""            );
    
    virtual void DestroyWindowContext( TWindowContextPtr& windowContext ) = 0;
    
    virtual bool ApplyVideoSettings( TWindowContextPtr& windowContext ,
                                     const CVideoSettings& settings   ) = 0;

    /**
     *  Whether windows can be made with this backend in this session. A backend that depends on a display server
     *  that may not be there (Wayland) says so here; the default is that it can.
     */
    virtual bool IsAvailable( void ) const;

    /** Why the backend is not available, in words for a log; empty while it is */
    virtual CString GetAvailabilityDescription( void ) const;

    /**
     *  The window creation parameter "GraphicsApi" (GraphicsApiParameterName) says what draws in the window:
     *
     *      "GL"    the window driver makes an OpenGL or OpenGL ES context for the window and offers it through the
     *              GL_* properties of the window context (GL_GETPROCADDRESS, GL_FRAMEBUFFER, GL_API, GL_VERSION)
     *      "None"  a native window without a graphics context or surface: something else (a render engine such as
     *              Ogre, or Vulkan, Direct3D) makes its surface from the native handles that the window context
     *              offers as properties (WINDOW, DISPLAY, ...). Events, sizing, full screen and the cursor work as
     *              for every window; the window driver neither clears nor presents
     *
     *  Without the parameter a backend makes the window it always made. A value the backend does not support is
     *  refused: CreateWindowContext() logs why and gives no window. Window contexts that know the parameter report
     *  the graphics API they were made for as the property "GRAPHICS_API".
     */
    static const CString GraphicsApiParameterName;
    static const CString GraphicsApiGL;
    static const CString GraphicsApiNone;

    /**
     *  The graphics API the parameters ask for: GraphicsApiGL or GraphicsApiNone for those values in any letter case,
     *  any other value as it is given, empty when the parameters do not name one
     */
    static CString GetRequestedGraphicsApi( const CORE::CValueList* params );

    /**
     *  Whether windows of this backend can be made for the graphics API (GraphicsApiGL, GraphicsApiNone). The
     *  default supports GraphicsApiGL only, what the OpenGL window drivers make; backends that support other values
     *  or that make no GL context override it
     */
    virtual bool IsGraphicsApiSupported( const CString& graphicsApi ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /**
     *  For the CreateWindowContext() implementations of backends: true when the parameters ask for no graphics API
     *  or for one the backend supports; otherwise the refusal is logged with the name of the backend and false is
     *  returned, and no window is to be made
     */
    bool IsRequestedGraphicsApiSupported( const CString& backendName      ,
                                          const CORE::CValueList* params ) const;

    private:

    CWindowManagerBackend( const CWindowManagerBackend& src );
    CWindowManagerBackend& operator=( const CWindowManagerBackend& src );        
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CWindowManagerBackend, MT::CMutex > TWindowManagerBackendPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUI */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 02-04-2005 :
       - Initial version of this file.

---------------------------------------------------------------------------*/
