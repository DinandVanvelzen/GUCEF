/*
 *  pluginglueEOS: Library that owns the Epic Online Services platform for the plugins that share it
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

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H
#include "pluginglueEOS_CEosGlue.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Holds the platform of the glue for the time of a scope: the acquire of CEosGlue is done by the constructor and the
 *  matching ReleasePlatform() by the destructor, the way CScopeMutex holds a lock. For the code that acquires a
 *  platform and then has more steps that can fail before it has what it needs:
 *
 *      CEosPlatformLease lease( user, reason );
 *      if ( !lease.IsHeld() )
 *          return false;
 *      <steps that return early when they fail: the platform is released with every one of them>
 *      m_platform = lease.Keep();      // all went well: the holder is the owner now, the lease releases nothing
 *
 *  The lease is for a hold that lasts a block. A holder that keeps the platform as long as it lives (a member) still
 *  releases it itself.
 */
class CEosPlatformLease
{
    public:

    /** Holds nothing */
    CEosPlatformLease( void )
        : m_platform( GUCEF_NULL )
    {
    }

    /** Acquires the platform for the shim (the default platform for NULL), see CEosGlue::AcquirePlatform(); IsHeld() tells whether it worked, 'reason' why not */
    CEosPlatformLease( const SEosPlatformShim* shim, const CEosPlatformSettings* settings, CORE::CString& reason )
        : m_platform( CEosGlue::Instance()->AcquirePlatform( shim, settings, reason ) )
    {
    }

    /** Acquires the platform of the user, see CEosGlue::AcquirePlatformOfUser(); IsHeld() tells whether it worked, 'reason' why not */
    CEosPlatformLease( const CORE::CString& productUserId, CORE::CString& reason )
        : m_platform( CEosGlue::Instance()->AcquirePlatformOfUser( productUserId, reason ) )
    {
    }

    /** Lets go of the platform when it is still held */
    ~CEosPlatformLease()
    {
        Release();
    }

    bool IsHeld( void ) const
    {
        return GUCEF_NULL != m_platform;
    }

    /** The platform, NULL when none is held; it stays held by the lease */
    CEosPlatform* Get( void ) const
    {
        return m_platform;
    }

    /** Hands the hold to the caller, who has to release the platform: the lease holds nothing after this */
    CEosPlatform* Keep( void )
    {
        CEosPlatform* platform = m_platform;
        m_platform = GUCEF_NULL;
        return platform;
    }

    /** Lets go of the platform now */
    void Release( void )
    {
        if ( GUCEF_NULL != m_platform )
        {
            CEosGlue::Instance()->ReleasePlatform( m_platform );
            m_platform = GUCEF_NULL;
        }
    }

    private:

    CEosPlatformLease( const CEosPlatformLease& src );              /* Copying doesnt make sense */
    CEosPlatformLease& operator=( const CEosPlatformLease& src );   /* Copying doesnt make sense */

    CEosPlatform* m_platform;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORMLEASE_H ? */
