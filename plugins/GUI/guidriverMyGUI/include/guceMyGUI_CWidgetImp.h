/*
 *  guidriverMyGUI: glue module for the MyGUI GUI backend
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

#ifndef GUCEF_MYGUI_CWIDGETIMP_H
#define GUCEF_MYGUI_CWIDGETIMP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "MyGUI.h"

//#ifndef __Exception_H_
//#include "OgreException.h"
//#define __Exception_H_
//#endif /* __Exception_H_ ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CSTDOSTREAMADAPTER_H
#include "CSTDOStreamAdapter.h"
#define GUCEF_CORE_CSTDOSTREAMADAPTER_H
#endif /* GUCEF_CORE_CSTDOSTREAMADAPTER_H ? */

#ifndef GUCEF_GUI_CWIDGET_H
#include "gucefGUI_CWidget.h"
#define GUCEF_GUI_CWIDGET_H
#endif /* GUCEF_GUI_CWIDGET_H ? */

#ifndef GUCEF_MYGUI_MACROS_H
#include "guceMyGUI_macros.h"     /* often used guceMyGUIOGRE macros */
#define GUCEF_MYGUI_MACROS_H
#endif /* GUCEF_MYGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MYGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEMPLATES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Generic widget API glue for the MyGUI/Ogre backend
 *  Implements the glue to MyGUI at the CWidget level
 */
template < class BaseClass >
class CWidgetImp : public BaseClass
{
    public:

    typedef GUCEF::GUI::CWidget::TWidgetSet TWidgetSet;

    CWidgetImp( void );

    virtual ~CWidgetImp();

    virtual bool ShowModal( void );

    virtual bool SetVisibility( const bool isVisible );

    virtual bool IsVisible( void ) const;

    virtual bool SetEnabled( const bool isEnabled );

    virtual bool IsEnabled( void ) const;

    virtual bool SetPosition( const GUCEF::GUI::CUVector& newPos );

    virtual bool GetPosition( GUCEF::GUI::CUVector& pos );

    virtual bool SetSize( const GUCEF::GUI::CUVector& newSize );

    virtual bool GetSize( GUCEF::GUI::CUVector& size ) const;

    virtual bool SetWidth( const GUCEF::GUI::CUDim& newWidth );

    virtual bool GetWidth( GUCEF::GUI::CUDim& width );

    virtual bool SetHeight( const GUCEF::GUI::CUDim& newHeight );

    virtual bool GetHeight( GUCEF::GUI::CUDim& height );

    virtual bool SetParentWidget( GUCEF::GUI::CWidget* parentWidget );

    virtual GUCEF::GUI::CWidget* GetParentWidget( void );

    virtual GUCEF::GUI::CWidget* GetChildWidget( const CString& widgetName );

    virtual bool GetChildWidgetSet( TWidgetSet& childSet );

    virtual CString GetName( void ) const;

    virtual bool SetPropertyValue( const CString& widgetProperty ,
                                   const CString& propertyValue  );

    virtual bool GetPropertyValue( const CString& widgetProperty ,
                                   CString& propertyValue        ) const;

    virtual bool SetProperties( const GUCEF::CORE::CValueList& list );

    virtual bool GetProperties( GUCEF::CORE::CValueList& list ) const;

    virtual bool HasProperty( const CString& widgetProperty ) const;

    virtual bool LoadProperties( GUCEF::CORE::CIOAccess& propertyStorage );

    virtual bool SaveProperties( GUCEF::CORE::CIOAccess& propertyStorage ) const;

    virtual void Hook( MyGUI::Widget* widget );

    virtual const CString& GetClassTypeName( void ) const;

    MyGUI::Widget* GetMyGUIWidget( void );

    MyGUI::IntPoint ConvertVectorToPoint( const GUCEF::GUI::CUVector& vector ) const;

    MyGUI::IntSize ConvertVectorToSize( const GUCEF::GUI::CUVector& vector ) const;

    GUCEF::GUI::CUVector ConvertVector( const MyGUI::IntPoint& vector ) const;

    GUCEF::GUI::CUVector ConvertVector( const MyGUI::IntSize& vector ) const;

    int ConvertXDim( const GUCEF::GUI::CUDim& x ) const;

    int ConvertYDim( const GUCEF::GUI::CUDim& x ) const;

    protected:

    virtual void OnMouseLostFocus( MyGUI::Widget* sender           ,
                                   MyGUI::Widget* newFocusedWidget );

    virtual void OnMouseReceivedFocus( MyGUI::Widget* sender           ,
                                       MyGUI::Widget* newFocusedWidget );

    virtual void OnMouseButtonPressed( MyGUI::Widget* sender     ,
                                       int left                  ,
                                       int top                   ,
                                       MyGUI::MouseButton button );

    virtual void OnMouseButtonReleased( MyGUI::Widget* sender     ,
                                        int left                  ,
                                        int top                   ,
                                        MyGUI::MouseButton button );

    virtual void OnMouseMove( MyGUI::Widget* sender ,
                              int left              ,
                              int top               );

    virtual void OnMouseClick( MyGUI::Widget* sender );

    virtual void OnMouseDoubleClick( MyGUI::Widget* sender );

    virtual void OnKeyboardFocusLost( MyGUI::Widget* sender           ,
                                      MyGUI::Widget* newFocusedWidget );

    virtual void OnKeyboardFocusReceived( MyGUI::Widget* sender           ,
                                          MyGUI::Widget* newFocusedWidget );

    virtual void OnKeyPressed( MyGUI::Widget* sender  ,
                               MyGUI::KeyCode keyCode ,
                               MyGUI::Char keyChar    );

    virtual void OnKeyReleased( MyGUI::Widget* sender  ,
                                MyGUI::KeyCode keyCode );

    private:

    CWidgetImp( const CWidgetImp& src );
    CWidgetImp& operator=( const CWidgetImp& src );

    private:

    MyGUI::Widget* m_widget;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CWidgetImp< GUCEF::GUI::CWidget > TBasicWidgetImp;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < class BaseClass >
CWidgetImp< BaseClass >::CWidgetImp( void )
    : BaseClass()      ,
      m_widget( GUCEF_NULL )
{GUCEF_TRACE;


}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
CWidgetImp< BaseClass >::~CWidgetImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::ShowModal( void )
{GUCEF_TRACE;

    //m_widget->setModalState( true );
    m_widget->setVisible( true );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::OnMouseLostFocus( MyGUI::Widget* sender           ,
                                           MyGUI::Widget* newFocusedWidget )
{GUCEF_TRACE;

    GUI::CWidget::NotifyObservers( GUI::CWidget::MouseFocusLostEvent );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::OnMouseReceivedFocus( MyGUI::Widget* sender           ,
                                               MyGUI::Widget* newFocusedWidget )
{GUCEF_TRACE;

    GUI::CWidget::NotifyObservers( GUI::CWidget::MouseFocusReceivedEvent );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::OnKeyboardFocusReceived( MyGUI::Widget* sender           ,
                                                  MyGUI::Widget* newFocusedWidget )
{GUCEF_TRACE;

    GUI::CWidget::NotifyObservers( GUI::CWidget::KeyboardFocusReceivedEvent );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::OnKeyboardFocusLost( MyGUI::Widget* sender           ,
                                              MyGUI::Widget* newFocusedWidget )
{GUCEF_TRACE;

    GUI::CWidget::NotifyObservers( GUI::CWidget::KeyboardFocusLostEvent );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::OnMouseButtonPressed( MyGUI::Widget* sender     ,
                                               int left                  ,
                                               int top                   ,
                                               MyGUI::MouseButton button )
{GUCEF_TRACE;

    GUI::CWidget::NotifyObservers( GUI::CWidget::MouseDownEvent );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::OnMouseButtonReleased( MyGUI::Widget* sender     ,
                                                int left                  ,
                                                int top                   ,
                                                MyGUI::MouseButton button )
{GUCEF_TRACE;

    GUI::CWidget::NotifyObservers( GUI::CWidget::MouseUpEvent );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::OnMouseMove( MyGUI::Widget* sender ,
                                      int left              ,
                                      int top               )
{GUCEF_TRACE;

    GUI::CWidget::NotifyObservers( GUI::CWidget::MouseMoveEvent );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::OnMouseClick( MyGUI::Widget* sender )
{GUCEF_TRACE;

    GUI::CWidget::NotifyObservers( GUI::CWidget::MouseClickEvent );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::OnMouseDoubleClick( MyGUI::Widget* sender )
{GUCEF_TRACE;

    GUI::CWidget::NotifyObservers( GUI::CWidget::MouseDoubleClickEvent );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::OnKeyPressed( MyGUI::Widget* sender  ,
                                       MyGUI::KeyCode keyCode ,
                                       MyGUI::Char keyChar    )
{GUCEF_TRACE;

    GUI::CWidget::NotifyObservers( GUI::CWidget::KeyDownEvent );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::OnKeyReleased( MyGUI::Widget* sender  ,
                                        MyGUI::KeyCode keyCode )
{GUCEF_TRACE;

    GUI::CWidget::NotifyObservers( GUI::CWidget::KeyUpEvent );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::Hook( MyGUI::Widget* widget )
{GUCEF_TRACE;

    m_widget = widget;
    GUI::CWidget::SetImplementationPtr( m_widget );

    if ( GUCEF_NULL != m_widget )
    {
        m_widget->setUserData( this );

        m_widget->eventMouseSetFocus += MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseReceivedFocus );
        m_widget->eventMouseLostFocus += MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseLostFocus );
        m_widget->eventMouseButtonPressed += MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseButtonPressed );
        m_widget->eventMouseButtonReleased += MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseButtonReleased );
        m_widget->eventMouseMove += MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseMove );
        m_widget->eventMouseButtonClick += MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseClick );
        m_widget->eventMouseButtonDoubleClick += MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnMouseDoubleClick );
        m_widget->eventKeySetFocus += MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnKeyboardFocusReceived );
        m_widget->eventKeyLostFocus += MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnKeyboardFocusLost );
        m_widget->eventKeyButtonPressed += MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnKeyPressed );
        m_widget->eventKeyButtonReleased += MyGUI::newDelegate( this, &CWidgetImp< BaseClass >::OnKeyReleased );
    }
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
int
CWidgetImp< BaseClass >::ConvertXDim( const GUCEF::GUI::CUDim& x ) const
{GUCEF_TRACE;

    MyGUI::Widget* parent = m_widget->getParent();
    if ( GUCEF_NULL != parent )
    {
        return (int) ( x.GetRelativeValue() * parent->getWidth() ) + x.GetPixelValue();
    }

    return x.GetPixelValue();
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
int
CWidgetImp< BaseClass >::ConvertYDim( const GUCEF::GUI::CUDim& y ) const
{GUCEF_TRACE;

    MyGUI::Widget* parent = m_widget->getParent();
    if ( GUCEF_NULL != parent )
    {
        return (int) ( y.GetRelativeValue() * parent->getHeight() ) + y.GetPixelValue();
    }

    return y.GetPixelValue();
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
GUCEF::GUI::CUVector
CWidgetImp< BaseClass >::ConvertVector( const MyGUI::IntSize& vector ) const
{GUCEF_TRACE;

    return GUCEF::GUI::CUVector( GUCEF::GUI::CUDim( 0.0, vector.width )  ,
                                 GUCEF::GUI::CUDim( 0.0, vector.height ) );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
GUCEF::GUI::CUVector
CWidgetImp< BaseClass >::ConvertVector( const MyGUI::IntPoint& vector ) const
{GUCEF_TRACE;

    return GUCEF::GUI::CUVector( GUCEF::GUI::CUDim( 0.0, vector.left ) ,
                                 GUCEF::GUI::CUDim( 0.0, vector.top )  );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
MyGUI::IntPoint
CWidgetImp< BaseClass >::ConvertVectorToPoint( const GUCEF::GUI::CUVector& vector ) const
{GUCEF_TRACE;

    MyGUI::IntPoint convertedValue;
    MyGUI::Widget* parent = m_widget->getParent();
    if ( GUCEF_NULL != parent )
    {
        convertedValue.left = (int) ( vector.GetX().GetRelativeValue() * parent->getWidth() ) + vector.GetX().GetPixelValue();
        convertedValue.top = (int) ( vector.GetY().GetRelativeValue() * parent->getHeight() ) + vector.GetY().GetPixelValue();
        return convertedValue;
    }

    convertedValue.left = vector.GetX().GetPixelValue();
    convertedValue.top = vector.GetY().GetPixelValue();
    return convertedValue;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
MyGUI::IntSize
CWidgetImp< BaseClass >::ConvertVectorToSize( const GUCEF::GUI::CUVector& vector ) const
{GUCEF_TRACE;

    MyGUI::IntSize convertedValue;
    MyGUI::Widget* parent = m_widget->getParent();
    if ( GUCEF_NULL != parent )
    {
        convertedValue.width = (int) ( vector.GetX().GetRelativeValue() * parent->getWidth() ) + vector.GetX().GetPixelValue();
        convertedValue.height = (int) ( vector.GetY().GetRelativeValue() * parent->getHeight() ) + vector.GetY().GetPixelValue();
        return convertedValue;
    }

    convertedValue.width = vector.GetX().GetPixelValue();
    convertedValue.height = vector.GetY().GetPixelValue();
    return convertedValue;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
const CString&
CWidgetImp< BaseClass >::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCE::MYGUIOGRE::CWidgetImp< BaseClass >";
    return typeName;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
MyGUI::Widget*
CWidgetImp< BaseClass >::GetMyGUIWidget( void )
{GUCEF_TRACE;

    return m_widget;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
GUCEF::GUI::CWidget*
CWidgetImp< BaseClass >::GetChildWidget( const CString& widgetName )
{GUCEF_TRACE;

    try
    {
        MyGUI::WidgetPtr childWidget = m_widget->findWidget( widgetName );
        if ( GUCEF_NULL != childWidget )
        {
            return *childWidget->getUserData< GUCEF::GUI::CWidget* >( false );
        }
    }
    catch ( std::exception& )
    {
        return GUCEF_NULL;
    }

    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
CString
CWidgetImp< BaseClass >::GetName( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_widget )
    {
        return m_widget->getName();
    }
    return CString();
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetVisibility( const bool isVisible )
{GUCEF_TRACE;

    m_widget->setVisible( isVisible );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::IsVisible( void ) const
{GUCEF_TRACE;

    return m_widget->getVisible();
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetEnabled( const bool isEnabled )
{GUCEF_TRACE;

    m_widget->setEnabled( isEnabled );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::IsEnabled( void ) const
{GUCEF_TRACE;

    return m_widget->getEnabled();
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetPosition( const GUCEF::GUI::CUVector& newPos )
{GUCEF_TRACE;

    m_widget->setPosition( ConvertVectorToPoint( newPos ) );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetPosition( GUCEF::GUI::CUVector& pos )
{GUCEF_TRACE;

    pos = ConvertVector( m_widget->getAbsolutePosition() );
    return false;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetSize( const GUCEF::GUI::CUVector& newSize )
{GUCEF_TRACE;

    m_widget->setSize( ConvertVectorToSize( newSize ) );
    return false;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetSize( GUCEF::GUI::CUVector& size ) const
{GUCEF_TRACE;

    size = ConvertVector( m_widget->getSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetWidth( const GUCEF::GUI::CUDim& newWidth )
{GUCEF_TRACE;

    m_widget->setSize( ConvertXDim( newWidth ), m_widget->getHeight() );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetWidth( GUCEF::GUI::CUDim& width )
{GUCEF_TRACE;

    width = GUCEF::GUI::CUDim( 0.0, m_widget->getWidth() );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetHeight( const GUCEF::GUI::CUDim& newHeight )
{GUCEF_TRACE;

    m_widget->setSize( m_widget->getWidth(), ConvertYDim( newHeight ) );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetHeight( GUCEF::GUI::CUDim& height )
{GUCEF_TRACE;

    height = GUCEF::GUI::CUDim( 0.0, m_widget->getHeight() );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetParentWidget( GUCEF::GUI::CWidget* parentWidget )
{GUCEF_TRACE;

    // Check if we have received a new parent
    if ( GUCEF_NULL != parentWidget )
    {
        // Try to get access to the implementation
        MyGUI::Widget* newParent = static_cast< MyGUI::Widget* >( parentWidget->GetImplementationPtr() );
        if ( GUCEF_NULL != newParent )
        {
            // Set the new parent
            m_widget->attachToWidget( newParent );
            return true;
        }
        return false;
    }
    else
    {
        m_widget->detachFromWidget();
        return true;
    }
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
GUCEF::GUI::CWidget*
CWidgetImp< BaseClass >::GetParentWidget( void )
{GUCEF_TRACE;

    MyGUI::Widget* parent = m_widget->getParent();
    if ( GUCEF_NULL != parent )
    {
        return *parent->getUserData< GUI::CWidget* >( false );
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetChildWidgetSet( TWidgetSet& childSet )
{GUCEF_TRACE;

    MyGUI::EnumeratorWidgetPtr childrenEnumerator = m_widget->getEnumerator();
    while ( childrenEnumerator.next() )
    {
        MyGUI::WidgetPtr child = childrenEnumerator.current();
        GUCEF::GUI::CWidget* wrapWindow = *child->getUserData< GUI::CWidget* >( false );
        if ( GUCEF_NULL != wrapWindow )
        {
            childSet.insert( wrapWindow );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetPropertyValue( const CString& widgetProperty ,
                                           const CString& propertyValue  )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetPropertyValue( const CString& widgetProperty ,
                                           CString& propertyValue        ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetProperties( const GUCEF::CORE::CValueList& list )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetProperties( GUCEF::CORE::CValueList& list ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::HasProperty( const CString& widgetProperty ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::LoadProperties( GUCEF::CORE::CIOAccess& propertyStorage )
{GUCEF_TRACE;

    // not supported
    return false;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SaveProperties( GUCEF::CORE::CIOAccess& propertyStorage ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MYGUIOGRE */
}; /* namespace GUCE */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MYGUI_CWIDGETIMP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/
