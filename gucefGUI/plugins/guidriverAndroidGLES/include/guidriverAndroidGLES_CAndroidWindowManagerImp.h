/*
 *  guidriverAndroidGLES: module implementing GLES based window management for Android
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
 
#ifndef GUIDRIVERANDROIDGLES_CANDROIDWINDOWMANAGERIMP_H
#define GUIDRIVERANDROIDGLES_CANDROIDWINDOWMANAGERIMP_H 

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERANDROIDGLES {
         
/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_GUI_PUBLIC_CPP CAndroidWindowManagerImp : public GUI::CWindowManagerBackend
{
    public:
    
    CAndroidWindowManagerImp( void );
    
    virtual ~CAndroidWindowManagerImp();
    
    virtual TWindowContextPtr CreateWindowContext( const GUI::CString& title ,
                                                   const GUI::UInt32 width   ,
                                                   const GUI::UInt32 height  ,
                                                   const bool fullscreen     );

    virtual TWindowContextPtr CreateWindowContext( const GUI::CString& title             ,
                                                   const GUI::CVideoSettings& settings   ,
                                                   const CORE::CValueList* params = NULL );

    virtual TWindowContextPtr CreateWindowContext( const GUI::CString& title      ,
                                                   const GUI::UInt32 width        ,
                                                   const GUI::UInt32 height       ,
                                                   const bool fullscreen          ,
                                                   const CORE::CValueList& params );
    
    virtual void DestroyWindowContext( TWindowContextPtr& windowContext );
    
    virtual bool ApplyVideoSettings( TWindowContextPtr& windowContext ,
                                     const CVideoSettings& settings   );

    private:

    CAndroidWindowManagerImp( const CAndroidWindowManagerImp& src );
    CAndroidWindowManagerImp& operator=( const CAndroidWindowManagerImp& src );        
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERANDROIDGLES */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUIDRIVERANDROIDGLES_CANDROIDWINDOWMANAGERIMP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 02-04-2005 :
       - Initial version of this file.

---------------------------------------------------------------------------*/
