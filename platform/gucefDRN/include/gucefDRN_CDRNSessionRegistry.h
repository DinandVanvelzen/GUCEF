/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


#ifndef GUCEF_DRN_CDRNSESSIONREGISTRY_H
#define GUCEF_DRN_CDRNSESSIONREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDRNSession;

/**
 *  The sessions an application makes known by name, so that code that works from configuration (a database client
 *  that opens "the session lobby1") can reach a session without holding it. Nothing registers by itself: an
 *  application that wants this registers its sessions. The registry does not own them, and forgets a session when
 *  it is destroyed.
 */
class GUCEF_DRN_EXPORT_CPP CDRNSessionRegistry : public CORE::CObserver
{
    public:

    CDRNSessionRegistry( void );

    virtual ~CDRNSessionRegistry() GUCEF_VIRTUAL_OVERRIDE;

    /** The session is found by CDRNSession::GetName(), which is set by Host() and Join(); false when it is registered already */
    bool Register( CDRNSession& session );

    void Unregister( CDRNSession& session );

    /** The registered session of the name, NULL if there is none */
    CDRNSession* Find( const CORE::CString& sessionName ) const;

    void GetSessionNames( CORE::CString::StringSet& names ) const;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CEntry
    {
        public:
        CDRNSession* session;
        CORE::CNotifier* notifier;      /**< the session as the notifier that tells about its destruction */
        CEntry( void ) : session( GUCEF_NULL ), notifier( GUCEF_NULL ) {}
    };
    typedef GUCEF::vector< CEntry >     TEntryVector;

    CDRNSessionRegistry( const CDRNSessionRegistry& src );                  /**< not implemented */
    CDRNSessionRegistry& operator=( const CDRNSessionRegistry& src );       /**< not implemented */

    mutable MT::CMutex m_lock;
    TEntryVector m_sessions;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNSESSIONREGISTRY_H ? */
