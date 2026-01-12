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
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#ifndef GUCEF_CORE_CTRACER_H
//#include "CTracer.h"
//#define GUCEF_CORE_CTRACER_H
//#endif /* GUCEF_CORE_CTRACER_H ? */
//
//#include "guceMyGUIOgre_CSpinnerImp.h"
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
////      UTILITIES                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//CSpinnerImp::CSpinnerImp( void )
//    : CWidgetImp< GUCEF::GUI::CSpinner >() ,
//      m_spinner( NULL )
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//
//CSpinnerImp::~CSpinnerImp()
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//
//const CString&
//CSpinnerImp::GetClassTypeName( void ) const
//{GUCE_TRACE;
//
//    static CString typeName = "GUCE::CEGUIOGRE::CSpinnerImp";
//    return typeName;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CSpinnerImp::SetValueUInt32( const UInt32 value )
//{GUCE_TRACE;
//
//    m_spinner->setCurrentValue( (float) value );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CSpinnerImp::SetValueInt32( const Int32 value )
//{GUCE_TRACE;
//
//    m_spinner->setCurrentValue( (float) value );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CSpinnerImp::SetValueFloat32( const Float32 value )
//{GUCE_TRACE;
//
//    m_spinner->setCurrentValue( (float) value );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//    
//UInt32
//CSpinnerImp::GetValueUInt32( void ) const
//{GUCE_TRACE;
//
//    return (UInt32) m_spinner->getCurrentValue();
//}
//
///*-------------------------------------------------------------------------*/
//    
//Int32
//CSpinnerImp::GetValueInt32( void ) const
//{GUCE_TRACE;
//
//    return (Int32) m_spinner->getCurrentValue();
//}
//
///*-------------------------------------------------------------------------*/
//    
//Float32
//CSpinnerImp::GetValueFloat32( void ) const
//{GUCE_TRACE;
//
//    return (Float32) m_spinner->getCurrentValue();
//}
//
///*-------------------------------------------------------------------------*/
//
//bool
//CSpinnerImp::OnValueChanged( const CEGUI::EventArgs& e )
//{GUCE_TRACE;
//
//    NotifyObservers( ValueChangedEvent );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//void
//CSpinnerImp::Hook( CEGUI::Spinner* spinner )
//{GUCE_TRACE;
//
//    m_spinner = spinner;
//    CWidgetImp< GUCEF::GUI::CSpinner >::Hook( spinner );
//    
//    m_spinner->subscribeEvent( CEGUI::Spinner::EventValueChanged                        ,
//                               CEGUI::Event::Subscriber( &CSpinnerImp::OnValueChanged , 
//                                                         this                         ) );    
//    
//}
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

