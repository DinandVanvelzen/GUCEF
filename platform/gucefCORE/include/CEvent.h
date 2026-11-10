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

#ifndef GUCEF_CORE_CEVENT_H
#define GUCEF_CORE_CEVENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

class CNotificationIDRegistry;

/*-------------------------------------------------------------------------*/

/**
 *  Event object
 *  Used to identify an event within the GUCEF event system.
 *
 *  For performance reasons numeric ID's are used for event comparison
 *  operations. Initialize() gives the event its id; modules call it for their events when they load. This
 *  initialization procedure is required to give you the performance required plus the ability to define event
 *  objects as global variables. An event that has a name but no id yet (for instance a global that was constructed
 *  after its module registered it) finds its id on first use in a comparison or GetID().
 */
class GUCEF_CORE_PUBLIC_CPP CEvent
{
    public:

    CEvent( void ) GUCEF_NOEXCEPT;

    CEvent( const CString& eventName ) GUCEF_NOEXCEPT;

    CEvent( const char* eventName ) GUCEF_NOEXCEPT;

    CEvent( const CEvent& src ) GUCEF_NOEXCEPT;

    #ifdef GUCEF_RVALUE_REFERENCES_SUPPORTED
    CEvent( CEvent&& src ) GUCEF_NOEXCEPT;
    #endif

    ~CEvent();

    CEvent& operator=( const CEvent& src );

    CEvent& operator=( const CString& eventName );

    CEvent& operator=( const char* eventName );

    bool operator==( const CEvent& other ) const;

    bool operator==( const char* eventName ) const;

    bool operator==( const CORE::CString& eventName ) const;

    bool operator!=( const CEvent& other ) const;

    /**
     *  This operator is only implemented for sorting/list
     *  containers and has no real meaning since the actual
     *  value of an event ID has no meaning other then a
     *  means of identification.
     */
    bool operator<( const CEvent& other ) const;

    UInt32 GetID( void ) const;

    CString GetName( void ) const;

    /**
     *  Initializes the event object
     *  This should always be called before the event object is ever used.
     *  It registers the event if needed and initializes the numeric event ID.
     */
    void Initialize( void ) const;

    bool IsInitialized( void ) const;

    /** Initializes the event if it has a name but no id yet; comparisons and GetID() do this themselves */
    void EnsureInitialized( void ) const;

    private:
    friend class CNotificationIDRegistry;

    CEvent( const UInt32 eventID     ,
            const CString& eventName ) GUCEF_NOEXCEPT;

    private:

    MT::CTAtomic< UInt32 > m_eventID;   /**< found on first use by whichever thread uses the event, always the same value for a name */
    CString m_eventName;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CEVENT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 07-10-2006 :
        - Initial implementation

---------------------------------------------------------------------------*/
