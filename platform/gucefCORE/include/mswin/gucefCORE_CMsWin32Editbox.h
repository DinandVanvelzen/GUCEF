/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEF_CORE_CMSWIN32EDITBOX_H
#define GUCEF_CORE_CMSWIN32EDITBOX_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CMSWIN32WINDOW_H
#include "gucefCORE_CMsWin32Window.h"
#define GUCEF_CORE_CMSWIN32WINDOW_H
#endif /* GUCEF_CORE_CMSWIN32WINDOW_H ? */

#ifdef GUCEF_MSWIN_BUILD

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class with basic utilities for creating a Win32 editbox
 */
class GUCEF_CORE_PUBLIC_CPP CMsWin32Editbox : public CMsWin32Window
{
    public:
    
    static const CEvent EnterPressedEvent;
    static const CEvent TextChangedEvent;
    
    static void RegisterEvents( void );
    
    public:
    
    typedef GUCEF::vector< CString > TStringVector;
    
    CMsWin32Editbox();

    virtual ~CMsWin32Editbox();
       
    virtual const CString& GetClassTypeName( void ) const;
    
    virtual void SetText( const CString& text );
    
    virtual CString GetText( void ) const;
    
    void AppendLine( const CString& line );
    
    void AppendLines( TStringVector& lines );
    
    void Clear( void );

    virtual bool EditboxCreate( CMsWin32Window& parent           ,
                                const CString& windowTitle       ,
                                const bool createAsMultilineEdit ,
                                const Int32 xPosition            ,
                                const Int32 yPosition            ,
                                const UInt32 width               ,
                                const UInt32 height              ,
                                const bool readOnly              );

    protected:
    
    virtual LRESULT WindowProc( const HWND hWnd     ,
                                const UINT nMsg     ,
                                const WPARAM wParam ,
                                const LPARAM lParam );

    private:

    CMsWin32Editbox( const CMsWin32Editbox& src );       /**< cannot be copied */
    CMsWin32Editbox& operator=( const CMsWin32Editbox ); /**< cannot be copied */
    
    private:
    
    HWND m_hwnd;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MSWIN_BUILD ? */

#endif /* GUCEF_CORE_CMSWIN32WINDOW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 22-04-2003 :
        - Initial implementation
          
---------------------------------------------------------------------------*/
