///*
// *  guceMyGUIOgre: glue module for the MyGUI+Ogre GUI backend
// *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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
//#ifndef GUCE_MYGUIOGRE_CWIDGETIMP_H
//#define GUCE_MYGUIOGRE_CWIDGETIMP_H
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      INCLUDES                                                           //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//#include "MyGUI.h"
//
//#ifndef __Exception_H_
//#include "OgreException.h"
//#define __Exception_H_
//#endif /* __Exception_H_ ? */
//
//#ifndef GUCEF_CORE_CSTDOSTREAMADAPTER_H
//#include "CSTDOStreamAdapter.h"
//#define GUCEF_CORE_CSTDOSTREAMADAPTER_H
//#endif /* GUCEF_CORE_CSTDOSTREAMADAPTER_H ? */
//
//#ifndef GUCEF_GUI_CWIDGET_H
//#include "gucefGUI_CWidget.h"
//#define GUCEF_GUI_CWIDGET_H  
//#endif /* GUCEF_GUI_CWIDGET_H ? */
//
//#ifndef GUCE_MYGUIOGRE_MACROS_H
//#include "guceMyGUIOgre_macros.h"     /* often used guceMyGUIOGRE macros */
//#define GUCE_MYGUIOGRE_MACROS_H
//#endif /* GUCE_MYGUIOGRE_MACROS_H ? */
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
////      TEMPLATES                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
///**
// *  Generic widget API glue for the MyGUI/Ogre backend
// *  Implements the glue to MyGUI at the CWidget level
// */
//template < class BaseClass >
//class CWidgetImp : public BaseClass
//{
//    public:
//    
//    typedef GUCEF::GUI::CWidget::TWidgetSet TWidgetSet;
//    
//    CWidgetImp( void );
//    
//    virtual ~CWidgetImp();
//    
//    virtual bool ShowModal( void );
//    
//    virtual bool SetVisibility( const bool isVisible );
//    
//    virtual bool IsVisible( void ) const;
//    
//    virtual bool SetEnabled( const bool isEnabled );
//     
//    virtual bool IsEnabled( void ) const;
//    
//    virtual bool SetPosition( const GUCEF::GUI::CUVector& newPos );
//    
//    virtual bool GetPosition( GUCEF::GUI::CUVector& pos );
//    
//    virtual bool SetSize( const GUCEF::GUI::CUVector& newSize );
//    
//    virtual bool GetSize( GUCEF::GUI::CUVector& size ) const;
//    
//    virtual bool SetWidth( const GUCEF::GUI::CUDim& newWidth );
//    
//    virtual bool GetWidth( GUCEF::GUI::CUDim& width );
//    
//    virtual bool SetHeight( const GUCEF::GUI::CUDim& newHeight );
//    
//    virtual bool GetHeight( GUCEF::GUI::CUDim& height );    
//    
//    virtual bool SetParentWidget( GUCEF::GUI::CWidget* parentWidget );
//    
//    virtual GUCEF::GUI::CWidget* GetParentWidget( void );
//    
//    virtual GUCEF::GUI::CWidget* GetChildWidget( const CString& widgetName );
//    
//    virtual bool GetChildWidgetSet( TWidgetSet& childSet );
//    
//    virtual CString GetName( void ) const;
//    
//    virtual bool SetPropertyValue( const CString& widgetProperty ,
//                                   const CString& propertyValue  );
//
//    virtual bool GetPropertyValue( const CString& widgetProperty ,
//                                   CString& propertyValue        ) const;
//
//    virtual bool SetProperties( const GUCEF::CORE::CValueList& list );
//
//    virtual bool GetProperties( GUCEF::CORE::CValueList& list ) const;
//    
//    virtual bool HasProperty( const CString& widgetProperty ) const;
//    
//    virtual bool LoadProperties( GUCEF::CORE::CIOAccess& propertyStorage );
//    
//    virtual bool SaveProperties( GUCEF::CORE::CIOAccess& propertyStorage ) const;
//    
//    virtual void Hook( MyGUI::Widget* widget );   
//
//    virtual const CString& GetClassTypeName( void ) const;
//    
//    MyGUI::Widget* GetMyGUIWidget( void );
//    
//    MyGUI::IntPoint ConvertVectorToPoint( const GUCEF::GUI::CUVector& vector ) const;
//    
//    MyGUI::IntSize ConvertVectorToSize( const GUCEF::GUI::CUVector& vector ) const;
//   
//    GUCEF::GUI::CUVector ConvertVector( const MyGUI::IntPoint& vector ) const;
//    
//    GUCEF::GUI::CUVector ConvertVector( const MyGUI::IntSize& vector ) const;
//    
//    int ConvertXDim( const GUCEF::GUI::CUDim& x ) const;
//    
//    int ConvertYDim( const GUCEF::GUI::CUDim& x ) const;
//   
//    protected:
//    
//    virtual void OnMouseLostFocus( MyGUI::Widget* sender           , 
//                                   MyGUI::Widget* newFocusedWidget );
//
//    virtual void OnMouseReceivedFocus( MyGUI::Widget* sender           , 
//                                       MyGUI::Widget* newFocusedWidget );
//
//    virtual void OnMouseButtonPressed( MyGUI::Widget* sender     , 
//                                       int left                  ,
//                                       int top                   ,
//                                       MyGUI::MouseButton button );
//                                       
//    virtual void OnMouseButtonReleased( MyGUI::Widget* sender     , 
//                                        int left                  ,
//                                        int top                   ,
//                                        MyGUI::MouseButton button );
//
//    virtual void OnMouseMove( MyGUI::Widget* sender , 
//                              int left              ,
//                              int top               );
//    
//    virtual void OnMouseClick( MyGUI::Widget* sender );
//    
//    virtual void OnMouseDoubleClick( MyGUI::Widget* sender );
//
//    virtual void OnKeyboardFocusLost( MyGUI::Widget* sender           , 
//                                      MyGUI::Widget* newFocusedWidget );
//
//    virtual void OnKeyboardFocusReceived( MyGUI::Widget* sender           , 
//                                          MyGUI::Widget* newFocusedWidget );
//
//    virtual void OnKeyPressed( MyGUI::Widget* sender  , 
//                               MyGUI::KeyCode keyCode ,
//                               MyGUI::Char keyChar    );
//
//    virtual void OnKeyReleased( MyGUI::Widget* sender  , 
//                                MyGUI::KeyCode keyCode );
//                               
//    private:
//    
//    CWidgetImp( const CWidgetImp& src );
//    CWidgetImp& operator=( const CWidgetImp& src );
//    
//    private:
//    
//    MyGUI::Widget* m_widget;
//};
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      TYPES                                                              //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//typedef CWidgetImp< GUCEF::GUI::CWidget > TBasicWidgetImp;
//
///*-------------------------------------------------------------------------//
////                                                                         //
////      UTILITIES                                                          //
////                                                                         //
////-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//CWidgetImp< BaseClass >::CWidgetImp( void )
//    : BaseClass()      ,
//      m_widget( NULL )
//{GUCE_TRACE;
//
//
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//CWidgetImp< BaseClass >::~CWidgetImp()
//{GUCE_TRACE;
//
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::ShowModal( void )
//{GUCE_TRACE;
//
//    //m_widget->setModalState( true );
//    m_widget->setVisible( true );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::OnMouseLostFocus( MyGUI::Widget* sender           , 
//                                           MyGUI::Widget* newFocusedWidget )
//{GUCE_TRACE;
//
//    NotifyObservers( MouseFocusLostEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::OnMouseReceivedFocus( MyGUI::Widget* sender           , 
//                                               MyGUI::Widget* newFocusedWidget )
//{GUCE_TRACE;
//
//    NotifyObservers( MouseFocusReceivedEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::OnKeyboardFocusReceived( MyGUI::Widget* sender           , 
//                                                  MyGUI::Widget* newFocusedWidget )
//{GUCE_TRACE;
//
//    NotifyObservers( KeyboardFocusReceivedEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::OnKeyboardFocusLost( MyGUI::Widget* sender           , 
//                                              MyGUI::Widget* newFocusedWidget )
//{GUCE_TRACE;
//
//    NotifyObservers( KeyboardFocusLostEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::OnMouseButtonPressed( MyGUI::Widget* sender     , 
//                                               int left                  ,
//                                               int top                   ,
//                                               MyGUI::MouseButton button )
//{GUCE_TRACE;
//
//    NotifyObservers( MouseDownEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::OnMouseButtonReleased( MyGUI::Widget* sender     , 
//                                                int left                  ,
//                                                int top                   ,
//                                                MyGUI::MouseButton button )
//{GUCE_TRACE;
//
//    NotifyObservers( MouseUpEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::OnMouseMove( MyGUI::Widget* sender , 
//                                      int left              ,
//                                      int top               )
//{GUCE_TRACE;
//
//    NotifyObservers( MouseMoveEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::OnMouseClick( MyGUI::Widget* sender )
//{GUCE_TRACE;
//
//    NotifyObservers( MouseClickEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::OnMouseDoubleClick( MyGUI::Widget* sender )
//{GUCE_TRACE;
//
//    NotifyObservers( MouseDoubleClickEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::OnKeyPressed( MyGUI::Widget* sender  , 
//                                       MyGUI::KeyCode keyCode ,
//                                       MyGUI::Char keyChar    )
//{GUCE_TRACE;
//
//    NotifyObservers( KeyDownEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::OnKeyReleased( MyGUI::Widget* sender  , 
//                                        MyGUI::KeyCode keyCode )
//{GUCE_TRACE;
//
//    NotifyObservers( KeyUpEvent );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//void
//CWidgetImp< BaseClass >::Hook( MyGUI::Widget* widget )
//{GUCE_TRACE;
//
//    m_widget = widget;
//    SetImplementationPtr( m_widget );
//    
//    if ( NULL != m_widget )
//    {
//        m_widget->setUserData( this );
//                
//        m_widget->eventMouseSetFocus = MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseReceivedFocus );
//        m_widget->eventMouseLostFocus = MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseLostFocus );
//        m_widget->eventMouseButtonPressed = MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseButtonPressed );
//        m_widget->eventMouseButtonReleased = MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseButtonReleased );
//        m_widget->eventMouseMove = MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseMove );
//        m_widget->eventMouseButtonClick = MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseClick );
//        m_widget->eventMouseButtonDoubleClick = MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseDoubleClick );
//        m_widget->eventKeySetFocus = MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnKeyboardFocusReceived );
//        m_widget->eventKeyLostFocus = MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnKeyboardFocusLost );
//        m_widget->eventKeyButtonPressed = MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnKeyPressed );
//        m_widget->eventKeyButtonReleased = MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnKeyReleased );
//    }
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//int
//CWidgetImp< BaseClass >::ConvertXDim( const GUCEF::GUI::CUDim& x ) const
//{GUCE_TRACE;
//
//    MyGUI::Widget* parent = m_widget->getParent();
//    if ( NULL != parent )        
//    {
//        return ( x.GetRelativeValue() * parent->getWidth() ) + x.GetPixelValue();
//    }
//
//    return x.GetPixelValue();
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//int
//CWidgetImp< BaseClass >::ConvertYDim( const GUCEF::GUI::CUDim& y ) const
//{GUCE_TRACE;
//
//    MyGUI::Widget* parent = m_widget->getParent();
//    if ( NULL != parent )        
//    {
//        return ( y.GetRelativeValue() * parent->getHeight() ) + y.GetPixelValue();
//    }
//
//    return y.GetPixelValue();
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//GUCEF::GUI::CUVector
//CWidgetImp< BaseClass >::ConvertVector( const MyGUI::IntSize& vector ) const
//{GUCE_TRACE;
//
//    return GUCEF::GUI::CUVector( GUCEF::GUI::CUDim( 0.0, vector.width )  ,
//                                 GUCEF::GUI::CUDim( 0.0, vector.height ) );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//GUCEF::GUI::CUVector
//CWidgetImp< BaseClass >::ConvertVector( const MyGUI::IntPoint& vector ) const
//{GUCE_TRACE;
//
//    return GUCEF::GUI::CUVector( GUCEF::GUI::CUDim( 0.0, vector.left ) ,
//                                 GUCEF::GUI::CUDim( 0.0, vector.top )  );
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//MyGUI::IntPoint 
//CWidgetImp< BaseClass >::ConvertVectorToPoint( const GUCEF::GUI::CUVector& vector ) const
//{GUCE_TRACE;
//
//    MyGUI::IntPoint convertedValue;
//    MyGUI::Widget* parent = m_widget->getParent();
//    if ( NULL != parent )        
//    {
//        convertedValue.left = ( vector.GetX().GetRelativeValue() * parent->getWidth() ) + vector.GetX().GetPixelValue();
//        convertedValue.top = ( vector.GetY().GetRelativeValue() * parent->getHeight() ) + vector.GetY().GetPixelValue();
//        return convertedValue;
//    }
//    
//    convertedValue.left = vector.GetX().GetPixelValue();
//    convertedValue.top = vector.GetY().GetPixelValue();
//    return convertedValue;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//MyGUI::IntSize 
//CWidgetImp< BaseClass >::ConvertVectorToSize( const GUCEF::GUI::CUVector& vector ) const
//{GUCE_TRACE;
//
//    MyGUI::IntSize convertedValue;
//    MyGUI::Widget* parent = m_widget->getParent();
//    if ( NULL != parent )        
//    {
//        convertedValue.width = ( vector.GetX().GetRelativeValue() * parent->getWidth() ) + vector.GetX().GetPixelValue();
//        convertedValue.height = ( vector.GetY().GetRelativeValue() * parent->getHeight() ) + vector.GetY().GetPixelValue();
//        return convertedValue;
//    }
//    
//    convertedValue.width = vector.GetX().GetPixelValue();
//    convertedValue.height = vector.GetY().GetPixelValue();
//    return convertedValue;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//const CString&
//CWidgetImp< BaseClass >::GetClassTypeName( void ) const
//{GUCE_TRACE;
//
//    static CString typeName = "GUCE::MYGUIOGRE::CWidgetImp< BaseClass >";
//    return typeName;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//MyGUI::Widget*
//CWidgetImp< BaseClass >::GetMyGUIWidget( void )
//{GUCE_TRACE;
//
//    return m_widget;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//GUCEF::GUI::CWidget*
//CWidgetImp< BaseClass >::GetChildWidget( const CString& widgetName )
//{GUCE_TRACE;
//
//    try
//    {
//        MyGUI::WidgetPtr childWidget = m_widget->findWidget( widgetName );
//        if ( NULL != childWidget )
//        {
//            return *childWidget->getUserData< GUCEF::GUI::CWidget* >( false );
//        }    
//    }
//    catch ( Ogre::Exception& )
//    {
//        return NULL;
//    }
//    
//    return NULL;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//CString
//CWidgetImp< BaseClass >::GetName( void ) const
//{GUCE_TRACE;
//
//    if ( NULL != m_widget )
//    {
//        return m_widget->getName();
//    }
//    return CString();
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::SetVisibility( const bool isVisible )
//{GUCE_TRACE;
//
//    m_widget->setVisible( isVisible );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::IsVisible( void ) const
//{GUCE_TRACE;
//
//    return m_widget->isVisible();
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::SetEnabled( const bool isEnabled )
//{GUCE_TRACE;
//
//    m_widget->setEnabled( isEnabled );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//     
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::IsEnabled( void ) const
//{GUCE_TRACE;
//
//    return m_widget->isEnabled();
//}
//
///*-------------------------------------------------------------------------*/
//    
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::SetPosition( const GUCEF::GUI::CUVector& newPos )
//{GUCE_TRACE;
//
//    m_widget->setPosition( ConvertVectorToPoint( newPos ) );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//    
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::GetPosition( GUCEF::GUI::CUVector& pos )
//{GUCE_TRACE;
//
//    pos = ConvertVector( m_widget->getAbsolutePosition() );
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::SetSize( const GUCEF::GUI::CUVector& newSize )
//{GUCE_TRACE;
//
//    m_widget->setSize( ConvertVectorToSize( newSize ) );
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::GetSize( GUCEF::GUI::CUVector& size ) const
//{GUCE_TRACE;
//
//    size = ConvertVector( m_widget->getSize() );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//    
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::SetWidth( const GUCEF::GUI::CUDim& newWidth )
//{GUCE_TRACE;
//        
//    m_widget->setSize( ConvertXDim( newWidth ), m_widget->getHeight() );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//    
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::GetWidth( GUCEF::GUI::CUDim& width )
//{GUCE_TRACE;
//
//    width = GUCEF::GUI::CUDim( 0.0, m_widget->getWidth() );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >    
//bool
//CWidgetImp< BaseClass >::SetHeight( const GUCEF::GUI::CUDim& newHeight )
//{GUCE_TRACE;
//
//    m_widget->setSize( m_widget->getWidth(), ConvertYDim( newHeight ) );    
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::GetHeight( GUCEF::GUI::CUDim& height )
//{GUCE_TRACE;
//
//    height = GUCEF::GUI::CUDim( 0.0, m_widget->getHeight() );
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::SetParentWidget( GUCEF::GUI::CWidget* parentWidget )
//{GUCE_TRACE;
//
//    // Check if we have received a new parent
//    if ( NULL != parentWidget )
//    {
//        // Try to get access to the implementation
//        MyGUI::Widget* newParent = static_cast< MyGUI::Widget* >( parentWidget->GetImplementationPtr() );
//        if ( NULL != newParent )
//        {
//            // Set the new parent
//            m_widget->attachToWidget( newParent );
//            return true;
//        }
//        return false;
//    }
//    else
//    {
//        m_widget->detachFromWidget();
//        return true;
//    }
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//GUCEF::GUI::CWidget*
//CWidgetImp< BaseClass >::GetParentWidget( void )
//{GUCE_TRACE;
//
//    MyGUI::Widget* parent = m_widget->getParent();
//    if ( NULL != parent )
//    {
//        return *parent->getUserData< CWidget* >( false );
//    }
//    return NULL;
//}
//    
///*-------------------------------------------------------------------------*/
//    
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::GetChildWidgetSet( TWidgetSet& childSet )
//{GUCE_TRACE;
//
//    MyGUI::EnumeratorWidgetPtr childrenEnumerator = m_widget->getEnumerator();
//    while ( childrenEnumerator.next() )
//    {
//        MyGUI::WidgetPtr child = childrenEnumerator.current();
//        GUCEF::GUI::CWidget* wrapWindow = *child->getUserData< CWidget* >( false );
//        if ( NULL != wrapWindow )
//        {
//            childSet.insert( wrapWindow );
//        }
//    }
//    return true;
//}
//
///*-------------------------------------------------------------------------*/
//    
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::SetPropertyValue( const CString& widgetProperty ,
//                                           const CString& propertyValue  )
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::GetPropertyValue( const CString& widgetProperty ,
//                                           CString& propertyValue        ) const
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::SetProperties( const GUCEF::CORE::CValueList& list )
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::GetProperties( GUCEF::CORE::CValueList& list ) const
//{GUCE_TRACE;
//
//    return false;
//}
//    
///*-------------------------------------------------------------------------*/
//
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::HasProperty( const CString& widgetProperty ) const
//{GUCE_TRACE;
//
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::LoadProperties( GUCEF::CORE::CIOAccess& propertyStorage )
//{GUCE_TRACE;
//
//    // not supported
//    return false;
//}
//
///*-------------------------------------------------------------------------*/
//    
//template < class BaseClass >
//bool
//CWidgetImp< BaseClass >::SaveProperties( GUCEF::CORE::CIOAccess& propertyStorage ) const
//{GUCE_TRACE;
//
//    return false;
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
//          
//#endif /* GUCE_MYGUIOGRE_CWIDGETIMP_H ? */
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
//---------------------------------------------------------------------------*/

