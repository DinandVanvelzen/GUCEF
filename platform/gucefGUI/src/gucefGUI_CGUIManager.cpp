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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefGUI_CGUIManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CGUIManager::DriverRegisteredEvent = "GUCEF::GUI::CGUIManager::DriverRegisteredEvent";
const CORE::CEvent CGUIManager::DriverUnregisteredEvent = "GUCEF::GUI::CGUIManager::DriverUnregisteredEvent";
const CORE::CEvent CGUIManager::FormFactoryRegisteredEvent = "GUCEF::GUI::CGUIManager::FormFactoryRegisteredEvent";
const CORE::CEvent CGUIManager::FormFactoryUnregisteredEvent = "GUCEF::GUI::CGUIManager::FormFactoryUnregisteredEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CGUIManager::RegisterEvents( void )
{GUCEF_TRACE;

    DriverRegisteredEvent.Initialize();
    DriverUnregisteredEvent.Initialize();
    FormFactoryRegisteredEvent.Initialize();
    FormFactoryUnregisteredEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CGUIManager::CGUIManager( void )
    : CObservingNotifier()   ,
      m_genericFormFactory() ,
      m_guiDrivers()
{GUCEF_TRACE;

    RegisterEvents();

    TEventCallback registeredCallback( this, &CGUIManager::OnFormFactoryRegistered );
    SubscribeTo( &m_genericFormFactory, CORE::CAbstractFactoryBase::ConcreteFactoryRegisteredEvent, registeredCallback );
    TEventCallback unregisteredCallback( this, &CGUIManager::OnFormFactoryUnregistered );
    SubscribeTo( &m_genericFormFactory, CORE::CAbstractFactoryBase::ConcreteFactoryUnregisteredEvent, unregisteredCallback );
}

/*-------------------------------------------------------------------------*/

CGUIManager::~CGUIManager()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TGuiContextPtr
CGUIManager::CreateGUIContext( const CString& guiDriverName    ,
                               TWindowContextPtr windowContext )
{GUCEF_TRACE;

    TGUIDriverMap::iterator i = m_guiDrivers.find( guiDriverName );
    if ( i != m_guiDrivers.end() )
    {
        CGUIDriver* guiDriver = (*i).second;
        return guiDriver->CreateGUIContext( windowContext );
    }
    return TGuiContextPtr();
}

/*-------------------------------------------------------------------------*/

void
CGUIManager::RegisterGUIDriver( const CString& guiDriverName ,
                                CGUIDriver* guiDriver        )
{GUCEF_TRACE;

    if ( GUCEF_NULL == guiDriver )
        return;

    // drivers that report no version (0) predate the version; any other mismatch cannot work
    UInt32 driverVersion = guiDriver->GetInterfaceVersion();
    if ( 0 != driverVersion && CGUIDriver::InterfaceVersion != driverVersion )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CGUIManager: Refused GUI driver \"" << guiDriverName << "\": it was built for driver interface version "
                         << driverVersion << " while this gucefGUI has version " << CGUIDriver::InterfaceVersion );
        return;
    }

    m_guiDrivers[ guiDriverName ] = guiDriver;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CGUIManager: Registered GUI driver (" + PointerToString( guiDriver ) + ") with name \"" + guiDriverName + "\"" );
    
    GUCEF::CORE::TCloneableString driverName( guiDriverName );
    NotifyObservers( DriverRegisteredEvent, &driverName );
}

/*-------------------------------------------------------------------------*/
    
void
CGUIManager::UnregisterGUIDriverByName( const CString& guiDriverName )
{GUCEF_TRACE;

    GUCEF::CORE::TCloneableString driverName( guiDriverName );
    NotifyObservers( DriverUnregisteredEvent, &driverName );

    m_guiDrivers.erase( guiDriverName );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CGUIManager: Unregistered GUI driver with name \"" + guiDriverName + "\"" );
}

/*-------------------------------------------------------------------------*/

CGUIDriver*
CGUIManager::GetGuiDriver( const CString& guiDriverName )
{GUCEF_TRACE;

    TGUIDriverMap::iterator i = m_guiDrivers.find( guiDriverName );    
    if ( i != m_guiDrivers.end() )
    {
        return (*i).second;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CGUIManager::GetGuiDriverCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_guiDrivers.size();
}

/*-------------------------------------------------------------------------*/

CGUIManager::TDriverNameSet
CGUIManager::GetGuiDriverList( void ) const
{GUCEF_TRACE;

    TDriverNameSet driverList;
    TGUIDriverMap::const_iterator i = m_guiDrivers.begin();    
    while ( i != m_guiDrivers.end() )
    {
        driverList.insert( (*i).first );
        ++i;
    }
    return driverList;
}

/*-------------------------------------------------------------------------*/

void
CGUIManager::RegisterGenericFormFactory( const CString& formTypeName   ,
                                         TConcreteFormFactory* factory )
{GUCEF_TRACE;

    m_genericFormFactory.RegisterConcreteFactory( formTypeName, factory );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CGUIManager: Registered Generic form factory (" + PointerToString( factory ) + ") with type name \"" + formTypeName + "\"" );
}    

/*-------------------------------------------------------------------------*/

void
CGUIManager::UnregisterGenericFormFactory( const CString& formTypeName )
{GUCEF_TRACE;

    m_genericFormFactory.UnregisterConcreteFactory( formTypeName );
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "CGUIManager: Unregistered Generic form factory with type name \"" + formTypeName + "\"" );
}

/*-------------------------------------------------------------------------*/
    
CGUIManager::TFormTypeSet
CGUIManager::GetGenericFormTypes( void ) const
{GUCEF_TRACE;

    TFormTypeSet formTypes;
    m_genericFormFactory.ObtainKeySet( formTypes );
    return formTypes;
}

/*-------------------------------------------------------------------------*/
    
bool
CGUIManager::HasGenericFormType( const CString& formTypeName )
{GUCEF_TRACE;

    return m_genericFormFactory.IsConstructible( formTypeName );
}

/*-------------------------------------------------------------------------*/
    
CFormPtr
CGUIManager::CreateGenericForm( const CString& formTypeName )
{GUCEF_TRACE;

    return m_genericFormFactory.Create( formTypeName );
}

/*-------------------------------------------------------------------------*/
    
/*-------------------------------------------------------------------------*/

void
CGUIManager::OnFormFactoryRegistered( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventid  ,
                                      CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    NotifyObservers( FormFactoryRegisteredEvent, eventdata );
}

/*-------------------------------------------------------------------------*/

void
CGUIManager::OnFormFactoryUnregistered( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventid  ,
                                        CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    NotifyObservers( FormFactoryUnregisteredEvent, eventdata );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
