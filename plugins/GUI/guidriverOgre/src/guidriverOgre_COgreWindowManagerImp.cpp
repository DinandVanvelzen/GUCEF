/*
 *  guidriverWin32GL: module implementing GL based window management for Win32
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

#ifndef _Ogre_H__
#include "Ogre.h"
#define _Ogre_H__
#endif /* _Ogre_H__ ? */ 

#ifndef GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#include "guidriverOgre_COgreWindowContext.h"
#define GUIDRIVEROGRE_COGREWINDOWCONTEXT_H
#endif /* GUIDRIVEROGRE_COGREWINDOWCONTEXT_H ? */

#include "guidriverOgre_COgreWindowManagerImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEROGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

COgreWindowManagerImp::COgreWindowManagerImp( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COgreWindowManagerImp::~COgreWindowManagerImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
COgreWindowManagerImp::CreateWindowContext( const GUI::CString& title ,
                                            const GUI::UInt32 width   ,
                                            const GUI::UInt32 height  ,
                                            const bool fullscreen     )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 24 );

    return CreateWindowContext( title         ,
                                videoSettings ,
                                GUCEF_NULL          );
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
COgreWindowManagerImp::CreateWindowContext( const GUI::CString& title           ,
                                            const GUI::CVideoSettings& settings ,
                                            const CORE::CValueList* params      )
{GUCEF_TRACE;

    TOgreWindowContextPtr windowContext = TOgreWindowContextPtr( new COgreWindowContext() );
    if ( windowContext->Initialize( title    ,
                                    settings ) )
    {
        return windowContext;
    }
    return GUI::TWindowContextPtr();
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
COgreWindowManagerImp::CreateWindowContext( const GUI::CString& title      ,
                                            const GUI::UInt32 width        ,
                                            const GUI::UInt32 height       ,
                                            const bool fullscreen          ,
                                            const CORE::CValueList& params )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 24 );

    return CreateWindowContext( title         ,
                                videoSettings ,
                                &params       );
}

/*-------------------------------------------------------------------------*/

void
COgreWindowManagerImp::DestroyWindowContext( GUI::TWindowContextPtr& windowContext )
{GUCEF_TRACE;

    COgreWindowContext* ogreContext = static_cast<COgreWindowContext*>( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL != ogreContext )
    {
        ogreContext->Shutdown();
    }
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowManagerImp::ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                           const GUI::CVideoSettings& settings   )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowManagerImp::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool
COgreWindowManagerImp::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;
      
    const CORE::CDataNode* n = config.Find( "OgreWindowManager" );
    if ( GUCEF_NULL != n )
    {
        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        if ( GUCEF_NULL == root )
        {
            // No plugins file or config file specified
            root = new Ogre::Root( "", "", "ogre.log" );
        }        
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVEROGRE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
