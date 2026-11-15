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

#ifndef GUCEF_CORE_CTGLOBALONREGISTRY_H
#define GUCEF_CORE_CTGLOBALONREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CTONREGISTRY_H
#include "CTONRegistry.h"
#define GUCEF_CORE_CTONREGISTRY_H
#endif /* GUCEF_CORE_CTONREGISTRY_H ? */

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
 *  Template that expands the normal CTONRegistry functionality with
 *  handlers for application wide events.
 * 
 *  Global registries used for dynamically registering abstracted functionality suffer from
 *  the problem that they need to be cleaned up when the application is shutting down in a timely manner.
 *  To avoid having to reimplement such handling for various usecases this template is provided.
 *
 *  Two kinds of entries are told apart:
 *   - entries that come from plugins ( Register(), TryRegister() ): these are unregistered when the unregistration
 *     of all plugins starts and when the application has shut down
 *   - built-in entries ( RegisterBuiltIn(), TryRegisterBuiltIn() ): functionality of the module itself, for example the
 *     CPU compute backend or the null audio driver. They are not provided by a plugin and thus survive the unregistration
 *     of all plugins, so the functionality stays available when the plugins are unloaded and loaded again at run time.
 *     They are unregistered when the application has shut down, or individually through Unregister().
 */
template < class T, class LockType >
class CTGlobalONRegistry : public CTONRegistry< T, LockType >
{
    public:

    CTGlobalONRegistry( void );

    CTGlobalONRegistry( const CTGlobalONRegistry& src );

    virtual ~CTGlobalONRegistry();

    CTGlobalONRegistry& operator=( const CTGlobalONRegistry& src );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    typedef typename CTONRegistry< T, LockType >::TRegisteredObjPtr     TRegisteredObjPtr;

    /**
     *  Registers an entry of the module itself, see the class description. Same behaviour as Register() otherwise,
     *  including the exception when the name is already in use.
     */
    void RegisterBuiltIn( const CString& name                ,
                          const TRegisteredObjPtr& sharedPtr );

    /** Like RegisterBuiltIn() but returns false instead of throwing when the name is already in use */
    bool TryRegisterBuiltIn( const CString& name                ,
                             const TRegisteredObjPtr& sharedPtr );

    /** Whether the entry with the given name is a built-in entry */
    bool IsBuiltIn( const CString& name ) const;

    /** Unregisters everything that was not registered as a built-in entry */
    void UnregisterAllExceptBuiltIn( void );

    virtual void Unregister( const CString& name ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool TryUnregister( const CString& name ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void UnregisterAll( void ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    typedef CTEventHandlerFunctor< CTGlobalONRegistry< T, LockType > >  TGlobalONRegistryEventCallback;

    void
    OnApplicationShutdownComplete( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventid  ,
                                   CORE::CICloneable* eventdata );

    void
    OnUnregisterOfAllPluginsStarted( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventid  ,
                                     CORE::CICloneable* eventdata );

    void RegisterEventHandlers( void );

    private:

    CString::StringSet m_builtInNames;      /**< names of the entries registered as built-in, guarded by the lock of the registry */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< class T, class LockType >
CTGlobalONRegistry< T, LockType >::CTGlobalONRegistry( void )
    : CTONRegistry< T, LockType >()
    , m_builtInNames()
{GUCEF_TRACE;

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
CTGlobalONRegistry< T, LockType >::CTGlobalONRegistry( const CTGlobalONRegistry& src )
    : CTONRegistry< T, LockType >( src )
    , m_builtInNames( src.m_builtInNames )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
CTGlobalONRegistry< T, LockType >::~CTGlobalONRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
CTGlobalONRegistry< T, LockType >&
CTGlobalONRegistry< T, LockType >::operator=( const CTGlobalONRegistry& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CTONRegistry< T, LockType >::operator=( src );
        m_builtInNames = src.m_builtInNames;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
const CString&
CTGlobalONRegistry< T, LockType >::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::CORE::CTGlobalONRegistry< T, LockType >";
    return typeName;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTGlobalONRegistry< T, LockType >::RegisterBuiltIn( const CString& name                ,
                                                    const TRegisteredObjPtr& sharedPtr )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( CTONRegistry< T, LockType >::AsLockable() );

    // Throws when the name is in use: nothing is marked in that case
    CTONRegistry< T, LockType >::Register( name, sharedPtr );
    m_builtInNames.insert( name );
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
bool
CTGlobalONRegistry< T, LockType >::TryRegisterBuiltIn( const CString& name                ,
                                                       const TRegisteredObjPtr& sharedPtr )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( CTONRegistry< T, LockType >::AsLockable() );

    if ( !CTONRegistry< T, LockType >::TryRegister( name, sharedPtr ) )
        return false;
    m_builtInNames.insert( name );
    return true;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
bool
CTGlobalONRegistry< T, LockType >::IsBuiltIn( const CString& name ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( CTONRegistry< T, LockType >::AsLockable() );

    return m_builtInNames.find( name ) != m_builtInNames.end();
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTGlobalONRegistry< T, LockType >::Unregister( const CString& name )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( CTONRegistry< T, LockType >::AsLockable() );

    CTONRegistry< T, LockType >::Unregister( name );
    m_builtInNames.erase( name );
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
bool
CTGlobalONRegistry< T, LockType >::TryUnregister( const CString& name )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( CTONRegistry< T, LockType >::AsLockable() );

    if ( !CTONRegistry< T, LockType >::TryUnregister( name ) )
        return false;
    m_builtInNames.erase( name );
    return true;
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTGlobalONRegistry< T, LockType >::UnregisterAll( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( CTONRegistry< T, LockType >::AsLockable() );

    CTONRegistry< T, LockType >::UnregisterAll();
    m_builtInNames.clear();
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTGlobalONRegistry< T, LockType >::UnregisterAllExceptBuiltIn( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( CTONRegistry< T, LockType >::AsLockable() );

    CString::StringVector names;
    CTONRegistry< T, LockType >::GetRegisteredObjNames( names );
    for ( UInt32 i=0; i<names.size(); ++i )
    {
        if ( m_builtInNames.find( names[ i ] ) == m_builtInNames.end() )
            CTONRegistry< T, LockType >::Unregister( names[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTGlobalONRegistry< T, LockType >::OnApplicationShutdownComplete( CNotifier* notifier    ,
                                                                  const CEvent& eventid  ,
                                                                  CICloneable* eventdata )
{GUCEF_TRACE;

    UnregisterAll();
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTGlobalONRegistry< T, LockType >::OnUnregisterOfAllPluginsStarted( CNotifier* notifier    ,
                                                                    const CEvent& eventid  ,
                                                                    CICloneable* eventdata )
{GUCEF_TRACE;

    // Entries of the module itself do not depend on a plugin and stay
    UnregisterAllExceptBuiltIn();
}

/*-------------------------------------------------------------------------*/

template< class T, class LockType >
void
CTGlobalONRegistry< T, LockType >::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TGlobalONRegistryEventCallback callback( this, &CTGlobalONRegistry< T, LockType >::OnApplicationShutdownComplete );
    CTONRegistry< T, LockType >::AsObserver().SubscribeTo( &CORE::CCoreGlobal::Instance()->GetApplication()  ,
                                                           CORE::CGUCEFApplication::AppShutdownCompleteEvent ,
                                                           callback                                         );
    TGlobalONRegistryEventCallback callback2( this, &CTGlobalONRegistry< T, LockType >::OnUnregisterOfAllPluginsStarted );
    CTONRegistry< T, LockType >::AsObserver().SubscribeTo( &CORE::CCoreGlobal::Instance()->GetPluginControl()       ,
                                                           CORE::CPluginControl::UnregisterOfAllPluginsStartedEvent ,
                                                           callback2                                                );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTONREGISTRY_H  ? */
