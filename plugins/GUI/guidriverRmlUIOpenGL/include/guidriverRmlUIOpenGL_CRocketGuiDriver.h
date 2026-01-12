/*
 *  guidriverRocketOpenGL: module providing a GUI backend using Rocket and OpenGL
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

#ifndef GUCEF_GUIDRIVERROCKETGL_CROCKETGUIDRIVER_H
#define GUCEF_GUIDRIVERROCKETGL_CROCKETGUIDRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_GUI_CGUIDRIVER_H
#include "gucefGUI_CGUIDriver.h"
#define GUCEF_GUI_CGUIDRIVER_H
#endif /* GUCEF_GUI_CGUIDRIVER_H ? */

#ifndef GUCEF_GUIDRIVERROCKET_CROCKETSYSTEMINTERFACE_H
#include "guidriverRocket_CRocketSystemInterface.h"
#define GUCEF_GUIDRIVERROCKET_CROCKETSYSTEMINTERFACE_H
#endif /* GUCEF_GUIDRIVERROCKET_CROCKETSYSTEMINTERFACE_H ? */

#ifndef GUCEF_GUIDRIVERROCKET_CROCKETVFSINTERFACE_H
#include "guidriverRocket_CRocketVFSInterface.h"
#define GUCEF_GUIDRIVERROCKET_CROCKETVFSINTERFACE_H
#endif /* GUCEF_GUIDRIVERROCKET_CROCKETVFSINTERFACE_H ? */

#ifndef GUCEF_GUIDRIVERROCKETGL_CROCKETRENDERINTERFACEOPENGL_H
#include "guidriverRocketOpenGL_CRocketRenderInterfaceOpenGL.h"
#define GUCEF_GUIDRIVERROCKETGL_CROCKETRENDERINTERFACEOPENGL_H
#endif /* GUCEF_GUIDRIVERROCKETGL_CROCKETRENDERINTERFACEOPENGL_H ? */

#ifndef GUCEF_GUIDRIVERROCKETGL_MACROS_H
#include "guidriverRocketOpenGL_macros.h"
#define GUCEF_GUIDRIVERROCKETGL_MACROS_H
#endif /* GUCEF_GUIDRIVERROCKETGL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERROCKETGL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUIDRIVERROCKETGL_PUBLIC_CPP CRocketGuiDriver : public GUI::CGUIDriver
{    
    public:

    CRocketGuiDriver( void );

    virtual ~CRocketGuiDriver();
    
    virtual GUI::TGuiContextPtr CreateGUIContext( GUI::TWindowContextPtr windowContext );
    
    virtual TGUIContextSet GetContextList( void );
    
    virtual GUI::UInt32 GetContextCount( void );
    
    virtual GUI::CString GetDriverName( void );
    
    virtual TStringSet GetAvailableFormTypes( void );
    
    virtual TStringSet GetAvailableWidgetTypes( void );

    virtual TStringSet GetAvailableFonts( void );

    virtual bool LoadFontFromAsset( const GUI::CString& assetPath );
    
    virtual const GUI::CString& GetClassTypeName( void ) const;
    
    virtual GUI::CString GetDriverProperty( const GUI::CString& propertyName ) const;

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec metadata
     *
     *      @param tree the data tree you wish to store
     *      @return wheter storing the tree was successfull
     */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const;

    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @return whether building the tree from the given file was successfull.
     */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot );    
    private:
    
    CRocketGuiDriver( const CRocketGuiDriver& src );

    private:

    CRocketRenderInterfaceOpenGL m_rocketRenderer;
    GUIDRIVERROCKET::CRocketSystemInterface m_systemInterface;
    GUIDRIVERROCKET::CRocketVFSInterface m_fileInterface;
    bool m_isRocketInitialized;
    GUI::CGUIDriver::TGUIContextSet m_contextSet;
    TStringSet m_fontTypes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERROCKETGL */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_GUIDRIVERROCKETGL_CROCKETGUIDRIVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2010 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/