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

#ifndef GUCEF_GUI_CFORMEX_H
#define GUCEF_GUI_CFORMEX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CFORM_H
#include "gucefGUI_CForm.h"
#define GUCEF_GUI_CFORM_H
#endif /* GUCEF_GUI_CFORM_H ? */

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

class GUCEF_GUI_PUBLIC_CPP CFormEx : public CForm
{
    public:
    
    CFormEx( void );

    CFormEx( CIGUIContext* context );
    
    virtual ~CFormEx();

    // the overloads of CForm this class does not override stay visible: layouts from trees and with a codec
    using CForm::LoadLayout;
    using CForm::SaveLayout;

    virtual bool LoadLayout( const CString& layoutStoragePath );

    virtual bool SaveLayout( const CString& layoutStoragePath );
    
    virtual bool LoadLayout( CORE::CIOAccess& layoutStorage );
    
    virtual bool SaveLayout( CORE::CIOAccess& layoutStorage );

    virtual bool LoadLayoutUsingVfs( const CString& filename );
    
    virtual bool SaveLayoutUsingVfs( const CString& filename );
    
    virtual bool Reload( void );
    
    virtual bool IsReloadable( void ) const;
    
    virtual const CString& GetClassTypeName( void ) const;
    
    private:
    
    CFormEx( const CFormEx& src );
    CFormEx& operator=( const CFormEx& src );
    
    private:
    
    bool m_isReloadable;
    CString m_layoutFile;
    bool m_useVfsDirect;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_GUI_CFORMEX_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

-----------------------------------------------------------------------------*/
