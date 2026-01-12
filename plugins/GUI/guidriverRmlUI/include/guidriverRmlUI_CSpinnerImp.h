///*
// *  guceMyGUIOgre: glue module for the MyGUI+Ogre GUI backend
// *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
// *
// *  This library is free software; you can redistribute it and/or
// *  modify it under the terms of the GNU Lesser General Public
// *  License as published by the Free Software Foundation; either
// *  version 2.1 of the License, or (at your option) any later version.
// *
// *  This library is distributed in the hope that it will be useful,
// *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// *  Lesser General Public License for more details.
// *
// *  You should have received a copy of the GNU Lesser General Public
// *  License along with this library; if not, write to the Free Software
// *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
// */
//
//#ifndef GUCE_MYGUIOGRE_CSPINNERIMP_H
//#define GUCE_MYGUIOGRE_CSPINNERIMP_H
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef _CEGUISpinner_h_
//#include "elements/CEGUISpinner.h"
//#define _CEGUISpinner_h_
//#endif /* _CEGUISpinner_h_ ? */
//
//#ifndef GUCE_MYGUIOGRE_CWIDGETIMP_H
//#include "guceMyGUIOgre_CWidgetImp.h"
//#define GUCE_MYGUIOGRE_CWIDGETIMP_H
//#endif /* GUCE_MYGUIOGRE_CWIDGETIMP_H ? */
//
//#ifndef GUCEF_GUI_CSPINNER_H
//#include "gucefGUI_CSpinner.h"
//#define GUCEF_GUI_CSPINNER_H
//#endif /* GUCEF_GUI_CSPINNER_H ? */
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//namespace GUCE {
//namespace MYGUIOGRE {
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      CLASSES                                                            //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//class GUCE_MYGUIOGRE_EXPORT_CPP CSpinnerImp : public CWidgetImp< GUCEF::GUI::CSpinner >
//{    
//    public:
//    
//    CSpinnerImp( void );
//    
//    virtual ~CSpinnerImp();
//    
//    virtual bool SetValueUInt32( const UInt32 value );
//    
//    virtual bool SetValueInt32( const Int32 value );
//    
//    virtual bool SetValueFloat32( const Float32 value );
//    
//    virtual UInt32 GetValueUInt32( void ) const;
//    
//    virtual Int32 GetValueInt32( void ) const;
//    
//    virtual Float32 GetValueFloat32( void ) const;
//    
//    virtual const CString& GetClassTypeName( void ) const;
//    
//    virtual void Hook( CEGUI::Spinner* spinner );
//    
//    private:
//    
//    CSpinnerImp( const CSpinnerImp& src );
//    CSpinnerImp& operator=( const CSpinnerImp& src );
//    
//    bool OnValueChanged( const CEGUI::EventArgs& e );
//    
//    private:
//    
//    CEGUI::Spinner* m_spinner;
//};
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      NAMESPACE                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//}; /* namespace MYGUIOGRE */
//}; /* namespace GUCE */
//
///*-------------------------------------------------------------------------*/
//          
//#endif /* GUCE_MYGUIOGRE_CSPINNERIMP_H ? */
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      Info & Changes                                                     //
////                                                                         //
////-------------------------------------------------------------------------//
//
//- 18-08-2007 :
//        - Dinand: Initial implementation
//
//-----------------------------------------------------------------------------*/

