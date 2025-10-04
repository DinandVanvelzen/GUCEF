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

#ifndef GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H
#define GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string>
#include <map>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CILOCKABLE_H
#include "gucefMT_CILockable.h"
#define GUCEF_MT_CILOCKABLE_H
#endif /* GUCEF_MT_CILOCKABLE_H ? */

#ifndef GUCEF_CORE_CEVENT_H
#include "CEvent.h"
#define GUCEF_CORE_CEVENT_H
#endif /* GUCEF_CORE_CEVENT_H ? */

#ifndef GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#include "ExceptionClassMacros.h"
#define GUCEF_CORE_EXCEPTIONCLASSMACROS_H
#endif /* GUCEF_CORE_EXCEPTIONCLASSMACROS_H ? */

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
 *  The CNotificationIDRegistry is used to obtain unique event id's for
 *  events name strings.
 *
 *  Note that it is strongly recommended to include the various namespaces
 *  in the event strings. For example:
 *      "GUCEF::CORE::CNotificationIDRegistry::FooBarEvent"
 *  If you follow this rule you should never have to worry about naming
 *  conflicts, while keeping the ease of use that comes from integer id's
 */
class GUCEF_CORE_PUBLIC_CPP CNotificationIDRegistry : public MT::CILockable
{
    public:

    /**
     *  Registers string identifiers and returns a unique event ID which can be used
     *  in it's place.
     *
     *  @param keyvalue the key string you wish to register and obtain a unique notification event ID for.
     *  @param okIfAlreadyRegistered indicates whether a register action using an already registered key should be handled silently
     *  @throw EKeyAlreadyRegistered thrown when the given key is already registered and okIfAlreadyRegistered is false or an empty key string is given
     *  @return unique notification id for the given key identifier
     */
    CEvent Register( const CString& keyvalue                  ,
                     const bool okIfAlreadyRegistered = false );

    /**
     *  Unregisters the string identifier. The ascociated event id should not longer be used.
     *
     *  @param keyvalue the key string identifier you wish to unregister
     *  @param okIfUnknownKeyGiven indicates whether an unregister action using an unknown key string identifier should be handled silently
     *  @throw EUnknownKey thrown when the given key string is unknown and okIfUnknownKeyGiven is false or an empty key string is given
     */
    void Unregister( const CString& keyvalue                ,
                     const bool okIfUnknownKeyGiven = false );

    /**
     *  Performs a lookup operation for the given key and if possible returns the
     *  associated unique notification event ID.
     *
     *  @param keyvalue the key you wish to use for the lookup operation.
     *  @param registerUnknown Whether or not to register the key if it does not yet exist.
     *  @throw EUnknownKey thrown when the requested item is not found and registerUnknown == false.
     */
    CEvent Lookup( const CString& keyvalue            ,
                   const bool registerUnknown = false );

    /**
     *  Performs a lookup operation for the given notification id and attempts to
     *  locate a matching identifier string. Verry usefull for translation into human readable
     *  output (when debugging).
     *
     *  Note that this operation is secondary and is not optimized for speed
     *  in any way.
     *
     *  @param eventID the notification ID you wish to resolve into an identifier
     *  @throw EUnknownEventID thrown when the given event id is unknown
     *  @return the notification event identifier for the given id.
     */
    CString Lookup( const CEvent& eventID ) const;

    bool IsRegistered( const CString& keyvalue ) const;

    GUCEF_DEFINE_MSGEXCEPTION( GUCEF_CORE_PUBLIC_CPP, EKeyAlreadyRegistered );
    GUCEF_DEFINE_MSGEXCEPTION( GUCEF_CORE_PUBLIC_CPP, EEmptyKeyString );
    GUCEF_DEFINE_MSGEXCEPTION( GUCEF_CORE_PUBLIC_CPP, EUnknownKey );
    GUCEF_DEFINE_MSGEXCEPTION( GUCEF_CORE_PUBLIC_CPP, EInvalidKey );
    GUCEF_DEFINE_MSGEXCEPTION( GUCEF_CORE_PUBLIC_CPP, EInvalidEventID );

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:
    
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;
    
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CNotificationIDRegistry( void );

    virtual ~CNotificationIDRegistry();

    private:

    CNotificationIDRegistry( const CNotificationIDRegistry& src );

    CNotificationIDRegistry& operator=( const CNotificationIDRegistry& src );

    private:
    typedef std::pair< const CString, UInt32 > TStringAndUInt32Pair;
    typedef GUCEF::map< CString, UInt32, std::less< CString >, gucef_allocator< TStringAndUInt32Pair > > TRegistryList;

    TRegistryList m_list;
    UInt32 m_lastid;
    MT::CMutex m_dataLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CNOTIFICATIONIDREGISTRY_H ? */
