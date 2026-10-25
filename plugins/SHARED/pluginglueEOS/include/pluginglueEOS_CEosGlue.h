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

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H
#define GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_PLUGINGLUE_EOS_CEOSPLATFORM_H
#include "pluginglueEOS_CEosPlatform.h"
#define GUCEF_PLUGINGLUE_EOS_CEOSPLATFORM_H
#endif /* GUCEF_PLUGINGLUE_EOS_CEOSPLATFORM_H ? */

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
 *  The one owner of the EOS platform of the process. Plugins that need EOS (the auth plugin, the lobby service, the
 *  P2P channel) acquire the platform here instead of making one: the first acquire makes it (EOS_Initialize,
 *  EOS_Platform_Create), the last release lets it go (EOS_Platform_Release), in between it is ticked from the pulse
 *  generator of the application and everybody sees the same login state.
 *
 *  Platforms are told apart by the function table that made them: all holders of the real SDK pass no table and get
 *  the platform of the SDK that is compiled in, a test passes its fake. The settings are the keys of the "epic"
 *  provider of the auth section (see CEosPlatformSettings).
 *
 *  The SDK is compiled in with the compiler flag GUCEF_AUTH_HAS_EOS (the flag of the auth plugin: one flag for one SDK),
 *  the include directory of the SDK, and the SDK library. Without it the default tables are empty and nothing can be
 *  acquired by default.
 */
class GUCEF_PLUGINGLUE_EOS_EXPORT_CPP CEosGlue
{
    public:

    static CEosGlue* Instance( void );

    /** Whether the build has the SDK compiled in */
    static bool IsSdkCompiledIn( void );

    /** The settings that platforms are made with when the one who acquires gives none */
    void SetSettings( const CEosPlatformSettings& settings );

    CEosPlatformSettings GetSettings( void ) const;

    /** Reads the settings from the section "Auth" of an application config (Providers/epic); false when there are none */
    bool LoadAuthSection( const CORE::CDataNode& authSection );

    /**
     *  Gives the tables that are used when somebody acquires without one: the real ones when the SDK is compiled in,
     *  or the ones an application or a test sets here
     */
    void SetDefaultShims( const SEosPlatformShim& platform, const SEosP2PShim& p2p );

    /** Back to the real tables */
    void ClearDefaultShims( void );

    bool GetDefaultPlatformShim( SEosPlatformShim& shim ) const;

    bool GetDefaultP2PShim( SEosP2PShim& shim ) const;

    /**
     *  Gives the platform for the table (the default platform for NULL), made when there is none yet, and counts the
     *  caller as a holder. NULL with the reason when it cannot be made: no SDK, no settings, the SDK said no. The
     *  platform that already exists is shared when the settings are for the same product; settings of another product
     *  are refused.
     *
     *  @param settings the settings to make it with, NULL for those of SetSettings()
     */
    CEosPlatform* AcquirePlatform( const SEosPlatformShim* shim, const CEosPlatformSettings* settings, CORE::CString& reason );

    /** Gives the platform of the user that is logged in (empty: the only platform that has a user logged in) and counts the caller as a holder. NULL if there is none or more than one */
    CEosPlatform* AcquirePlatformOfUser( const CORE::CString& productUserId, CORE::CString& reason );

    /** Counts one more holder of a platform that the caller holds already */
    void AddRef( CEosPlatform* platform );

    /** The caller lets go; the last one releases the platform */
    void ReleasePlatform( CEosPlatform* platform );

    UInt32 GetPlatformCount( void ) const;

    private:

    friend class CModule;

    static void Deinstance( void );

    CEosGlue( void );
    ~CEosGlue();
    CEosGlue( const CEosGlue& src );                /**< not implemented */
    CEosGlue& operator=( const CEosGlue& src );     /**< not implemented */

    CEosPlatform* FindPlatform( const SEosPlatformShim& shim ) const;

    private:

    typedef GUCEF::vector< CEosPlatform* > TPlatformVector;

    mutable MT::CMutex m_lock;
    CEosPlatformSettings m_settings;
    TPlatformVector m_platforms;
    bool m_haveOverride;
    SEosPlatformShim m_overridePlatform;
    SEosP2PShim m_overrideP2P;

    static MT::CMutex g_dataLock;
    static CEosGlue* g_instance;
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

#endif /* GUCEF_PLUGINGLUE_EOS_CEOSGLUE_H ? */
