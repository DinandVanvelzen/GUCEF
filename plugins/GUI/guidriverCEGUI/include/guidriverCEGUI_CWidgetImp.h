/*
 *  guidriverCEGUI: glue module for the CEGUI GUI backend
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

#ifndef GUCEF_GUIDRIVERCEGUI_CWIDGETIMP_H
#define GUCEF_GUIDRIVERCEGUI_CWIDGETIMP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef _CEGUIWindow_h_
#include "CEGUI/Window.h"
#define _CEGUIWindow_h_
#endif /* _CEGUIWindow_h_ ? */

#ifndef _CEGUIExceptions_h_
#include "CEGUI/Exceptions.h"
#define _CEGUIExceptions_h_
#endif /* _CEGUIExceptions_h_ ? */

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

#ifndef GUCEF_GUIDRIVERCEGUI_MACROS_H
#include "guidriverCEGUI_macros.h"     /* often used guceMyGUIOGRE macros */
#define GUCEF_GUIDRIVERCEGUI_MACROS_H
#endif /* GUCEF_GUIDRIVERCEGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERCEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TEMPLATES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Generic widget API glue for the CEGUI backend
 *  Implements the glue to CEGUI at the CWidget level
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
    
    virtual void Hook( CEGUI::Window* window );
    
    CEGUI::UDim ConvertDim( const GUCEF::GUI::CUDim& src ) const;
    
    GUCEF::GUI::CUDim ConvertDim( const CEGUI::UDim& src ) const;
    
    CEGUI::UVector2 ConvertVector( const GUCEF::GUI::CUVector& src ) const;

    CEGUI::USize ConvertVectorToSize( const GUCEF::GUI::CUVector& src ) const;
    
    GUCEF::GUI::CUVector ConvertVector( const CEGUI::UVector2& src ) const;

    GUCEF::GUI::CUVector ConvertVector( const CEGUI::USize& src ) const;

    virtual const CString& GetClassTypeName( void ) const;

    protected:
    
    virtual bool OnMouseDown( const CEGUI::EventArgs& e );
    
    virtual bool OnMouseUp( const CEGUI::EventArgs& e );
    
    CEGUI::Window* GetCEGUIWindow( void );
    
    private:
    
    CWidgetImp( const CWidgetImp& src );
    CWidgetImp& operator=( const CWidgetImp& src );
    
    private:
    
    CEGUI::Window* m_window;
};

/*-------------------------------------------------------------------------*/

///**
// *  Dummy class used to gain access to CEGUI window properties
// */
//class CWindowDataAccessor : public CEGUI::Window
//{
//    public:
//    
//    typedef std::vector< CEGUI::Window* > CEGUIChildList;
//        
//    CEGUIChildList& GetChildList( void );
//    
//    private:
//    
//    CWindowDataAccessor( void );
//    CWindowDataAccessor( const CWindowDataAccessor& src );
//    ~CWindowDataAccessor();
//    CWindowDataAccessor& operator=( const CWindowDataAccessor& src );
//};

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
      m_window( NULL )
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

    m_window->setModalState( true );
    m_window->show();
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
void
CWidgetImp< BaseClass >::Hook( CEGUI::Window* window )
{GUCEF_TRACE;

    m_window = window;
    m_window->setUserData( this );
    SetImplementationPtr( m_window );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
const CString&
CWidgetImp< BaseClass >::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCE::CEGUIOGRE::CWidgetImp< BaseClass >";
    return typeName;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
CEGUI::Window*
CWidgetImp< BaseClass >::GetCEGUIWindow( void )
{GUCEF_TRACE;

    return m_window;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
GUCEF::GUI::CWidget*
CWidgetImp< BaseClass >::GetChildWidget( const CString& widgetName )
{GUCEF_TRACE;

    CEGUI::Window* childWidget = NULL;
    
    try
    {
        childWidget = m_window->getChild( widgetName.C_String() );
    }
    catch ( CEGUI::Exception& )
    {
        return NULL;
    }
    
    return static_cast< CWidget* >( childWidget->getUserData() );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
CString
CWidgetImp< BaseClass >::GetName( void ) const
{GUCEF_TRACE;

    if ( NULL != m_window )
    {
        return m_window->getName().c_str();
    }
    return CString();
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
CEGUI::UDim
CWidgetImp< BaseClass >::ConvertDim( const GUCEF::GUI::CUDim& src ) const
{GUCEF_TRACE;

    return CEGUI::UDim( src.GetRelativeValue(), (float) src.GetPixelValue() );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
CEGUI::USize
CWidgetImp< BaseClass >::ConvertVectorToSize( const GUCEF::GUI::CUVector& src ) const
{GUCEF_TRACE;

    return CEGUI::USize( ConvertDim( src.GetX() ), ConvertDim( src.GetY() ) );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
CEGUI::UVector2
CWidgetImp< BaseClass >::ConvertVector( const GUCEF::GUI::CUVector& src ) const
{GUCEF_TRACE;

    return CEGUI::UVector2( ConvertDim( src.GetX() ), ConvertDim( src.GetY() ) );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
GUCEF::GUI::CUDim
CWidgetImp< BaseClass >::ConvertDim( const CEGUI::UDim& src ) const
{GUCEF_TRACE;

    return GUCEF::GUI::CUDim( src.d_scale, (UInt32) src.d_offset );    
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
GUCEF::GUI::CUVector
CWidgetImp< BaseClass >::ConvertVector( const CEGUI::USize& src ) const
{GUCEF_TRACE;

    return GUCEF::GUI::CUVector( ConvertDim( src.d_width ), ConvertDim( src.d_height ) );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
GUCEF::GUI::CUVector
CWidgetImp< BaseClass >::ConvertVector( const CEGUI::UVector2& src ) const
{GUCEF_TRACE;

    return GUCEF::GUI::CUVector( ConvertDim( src.d_x ), ConvertDim( src.d_y ) );
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetVisibility( const bool isVisible )
{GUCEF_TRACE;

    m_window->setVisible( isVisible );
    if ( !isVisible )
    {
        m_window->setModalState( false );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::IsVisible( void ) const
{GUCEF_TRACE;

    return m_window->isVisible();
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetEnabled( const bool isEnabled )
{GUCEF_TRACE;

    m_window->setEnabled( isEnabled );
    return true;
}

/*-------------------------------------------------------------------------*/
     
template < class BaseClass >
bool
CWidgetImp< BaseClass >::IsEnabled( void ) const
{GUCEF_TRACE;

    return !m_window->isDisabled();
}

/*-------------------------------------------------------------------------*/
    
template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetPosition( const GUCEF::GUI::CUVector& newPos )
{GUCEF_TRACE;

    m_window->setPosition( ConvertVector( newPos ) );
    return true;
}

/*-------------------------------------------------------------------------*/
    
template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetPosition( GUCEF::GUI::CUVector& pos )
{GUCEF_TRACE;

    pos = ConvertVector( m_window->getPosition() );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetSize( const GUCEF::GUI::CUVector& newSize )
{GUCEF_TRACE;

    m_window->setSize( ConvertVectorToSize( newSize ) );
    return true;
}

/*-------------------------------------------------------------------------*/
    
template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetSize( GUCEF::GUI::CUVector& size ) const
{GUCEF_TRACE;

    size = ConvertVector( m_window->getSize() );
    return true;
}

/*-------------------------------------------------------------------------*/
    
template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetWidth( const GUCEF::GUI::CUDim& newWidth )
{GUCEF_TRACE;

    m_window->setWidth( ConvertDim( newWidth ) );    
    return true;
}

/*-------------------------------------------------------------------------*/
    
template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetWidth( GUCEF::GUI::CUDim& width )
{GUCEF_TRACE;

    width = ConvertDim( m_window->getWidth() );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >    
bool
CWidgetImp< BaseClass >::SetHeight( const GUCEF::GUI::CUDim& newHeight )
{GUCEF_TRACE;

    m_window->setHeight( ConvertDim( newHeight ) );    
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetHeight( GUCEF::GUI::CUDim& height )
{GUCEF_TRACE;

    height = ConvertDim( m_window->getHeight() );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetParentWidget( GUCEF::GUI::CWidget* parentWidget )
{GUCEF_TRACE;

    // First make sure we are no longer attached to another parent window
    if ( NULL != m_window )
    {
        if ( NULL != m_window->getParent() )
        {
            m_window->getParent()->removeChild( m_window );
        }
    }

    // Check if we have received a new parent
    if ( NULL != parentWidget )
    {
        // Try to get access to the implementation
        CEGUI::Window* newParent = static_cast< CEGUI::Window* >( parentWidget->GetImplementationPtr() );
        if ( NULL != newParent )
        {
            // Set the new parent
            newParent->addChild( m_window );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
GUCEF::GUI::CWidget*
CWidgetImp< BaseClass >::GetParentWidget( void )
{GUCEF_TRACE;

    CEGUI::Window* parent = m_window->getParent();
    if ( NULL != parent )
    {
        return static_cast< CWidget* >( parent->getUserData() );
    }
    return NULL;
}
    
/*-------------------------------------------------------------------------*/
    
template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetChildWidgetSet( TWidgetSet& childSet )
{GUCEF_TRACE;

    for ( UInt32 i=0; i < m_window->getChildCount(); ++i )
    {
        CEGUI::Window* childWindow = m_window->getChildAtIdx( i );
        if ( NULL != childWindow )
        {
            GUCEF::GUI::CWidget* wrapWindow = static_cast< CWidget* >( childWindow->getUserData() );
            if ( NULL != wrapWindow )
            {
                childSet.insert( wrapWindow );
            }
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

    try
    {
        m_window->setProperty( widgetProperty.C_String() ,
                               propertyValue.C_String()  );
        return true;
    }
    catch ( CEGUI::UnknownObjectException& )
    {
        return false;
    }
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetPropertyValue( const CString& widgetProperty ,
                                           CString& propertyValue        ) const
{GUCEF_TRACE;

    try
    {
        propertyValue = m_window->getProperty( widgetProperty.C_String() ).c_str();
        return true;
    }
    catch ( CEGUI::UnknownObjectException& )
    {
        return false;
    }
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::SetProperties( const GUCEF::CORE::CValueList& list )
{GUCEF_TRACE;

    bool success = true;
    for ( UInt32 i=0; i < list.GetKeyCount(); ++i )
    {
        try
        {
            m_window->setProperty( list.GetKey( i ).AsString().C_String()   ,
                                   list.GetValue( i ).AsString().C_String() );
        }
        catch ( CEGUI::UnknownObjectException& )
        {
            success = false;
        }
    }
    return success;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::GetProperties( GUCEF::CORE::CValueList& list ) const
{GUCEF_TRACE;

    CEGUI::PropertySet::PropertyIterator i = m_window->getPropertyIterator();
    while ( !i.isAtEnd() )
    {   
        CEGUI::Property* property = (*i);
        try
        { 
            list.Set( property->getName().c_str()                          ,
                      m_window->getProperty( property->getName() ).c_str() ); 
        }
        catch ( CEGUI::UnknownObjectException& )
        {
        }
        
        ++i;
    }
    return true;
}
    
/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::HasProperty( const CString& widgetProperty ) const
{GUCEF_TRACE;

    return m_window->isPropertyPresent( widgetProperty.C_String() );
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

    GUCEF::CORE::CSTDOStreamAdapter outputAccess( propertyStorage );
    CEGUI::XMLSerializer encoder( outputAccess );
    m_window->writeXMLToStream( encoder );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::OnMouseDown( const CEGUI::EventArgs& e )
{GUCEF_TRACE;

    NotifyObservers( MouseDownEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

template < class BaseClass >
bool
CWidgetImp< BaseClass >::OnMouseUp( const CEGUI::EventArgs& e )
{GUCEF_TRACE;

    NotifyObservers( MouseUpEvent );
    return true;
}
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERCEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUIDRIVERCEGUI_CWIDGETIMP_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/
