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

#ifndef GUCEF_GUI_CUDIM_H
#define GUCEF_GUI_CUDIM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

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

class GUCEF_GUI_PUBLIC_CPP CUDim
{
    public:
    
    CUDim( void );
    
    CUDim( const Float32 relativeValue ,
           const Int32 pixelValue      );
    
    CUDim( const CUDim& src );
    
    ~CUDim();
    
    CUDim& operator=( const CUDim& src );
    
    bool operator==( const CUDim& other ) const;
    
    bool operator!=( const CUDim& other ) const;
    
    void SetRelativeValue( const Float32 relativeValue );
    
    Float32 GetRelativeValue( void ) const;
    
    void SetPixelValue( const Int32 pixelValue );
    
    Int32 GetPixelValue( void ) const;

    /** The size in pixels for the given size of the parent: relative * parentSize + pixels, rounded */
    Int32 Resolve( const Int32 parentSize ) const;

    /** "relative,pixels", for example "0.5,-20" */
    CString ToString( void ) const;

    /** Accepts "relative,pixels" or a single number, which is taken as pixels */
    static bool FromString( const CString& str, CUDim& dim );
    
    private:
    
    Float32 m_relativeValue;
    Int32 m_pixelValue;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_GUI_CUDIM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

-----------------------------------------------------------------------------*/
