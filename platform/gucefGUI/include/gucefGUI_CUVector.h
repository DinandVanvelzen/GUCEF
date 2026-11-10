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

#ifndef GUCEF_GUI_CUVECTOR_H
#define GUCEF_GUI_CUVECTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CUDIM_H
#include "gucefGUI_CUDim.h"
#define GUCEF_GUI_CUDIM_H
#endif /* GUCEF_GUI_CUDIM_H ? */

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

class GUCEF_GUI_PUBLIC_CPP CUVector
{
    public:
    
    CUVector( void );
    
    CUVector( const CUDim& x ,
              const CUDim& y );
    
    CUVector( const CUVector& src );
    
    ~CUVector();
    
    CUVector& operator=( const CUVector& src );

    bool operator==( const CUVector& other ) const;
    
    bool operator!=( const CUVector& other ) const;
    
    void SetX( const CUDim& newWidth );
    
    const CUDim& GetX( void ) const;
    
    void SetY( const CUDim& newHeight );
    
    const CUDim& GetY( void ) const;

    /** "x y" with each a CUDim string, for example "0.5,-20 0,10" */
    CString ToString( void ) const;

    static bool FromString( const CString& str, CUVector& vec );
    
    private:
    
    CUDim m_x;
    CUDim m_y;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_GUI_CUVECTOR_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

-----------------------------------------------------------------------------*/