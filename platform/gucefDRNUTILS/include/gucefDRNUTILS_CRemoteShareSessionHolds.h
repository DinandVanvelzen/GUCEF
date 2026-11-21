/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CREMOTESHARESESSIONHOLDS_H
#define GUCEF_DRNUTILS_CREMOTESHARESESSIONHOLDS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {
class CDRNSession;
}
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How many mounts of remote shares (CRemoteShareMount) hold the registration of a DRN session in the DRN session registry: the mounts of one
 *  connection and channel share the one registration the first of them made, and the last one to let go takes it away. Only sessions that a mount
 *  registered are counted here; a session the application registered has no count and is never removed by a mount. The sessions are compared by
 *  address and never called. Thread safe.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteShareSessionHolds
{
    public:

    CRemoteShareSessionHolds( void );
    ~CRemoteShareSessionHolds();

    /** A mount registered the session: it holds it alone (count 1) */
    void Created( const DRN::CDRNSession* session );

    /** Another mount holds the session as well; false (nothing counted) when no mount registered it */
    bool Join( const DRN::CDRNSession* session );

    /** A mount lets go; true when it was the last one (the count is gone: the registration is to be taken away) */
    bool Release( const DRN::CDRNSession* session );

    /** The number of mounts that hold the session, 0 for one that no mount registered */
    UInt32 GetCount( const DRN::CDRNSession* session ) const;

    private:

    CRemoteShareSessionHolds( const CRemoteShareSessionHolds& src );                /**< not implemented */
    CRemoteShareSessionHolds& operator=( const CRemoteShareSessionHolds& src );     /**< not implemented */

    mutable MT::CMutex m_lock;
    GUCEF::map< const DRN::CDRNSession*, UInt32 > m_holds;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESHARESESSIONHOLDS_H ? */
