/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "gucef_platform.h"

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) )
  #include <pwd.h>
  #include <unistd.h>
  #include <sys/types.h>
#endif

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestUserNames.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/** Asks for the names over and over from a thread of its own */
class CUserNameAsker : public MT::CActiveObject
{
    public:

    CUserNameAsker( void )
        : MT::CActiveObject(), m_loginName(), m_displayName(), m_consistent( true ), m_done( false )
    {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        const CORE::CString loginName = CORE::GetCurrentUserName();
        const CORE::CString displayName = CORE::GetCurrentUserDisplayName();
        for ( UInt32 i=0; i<500; ++i )
        {
            if ( CORE::GetCurrentUserName() != loginName || CORE::GetCurrentUserDisplayName() != displayName )
                m_consistent.Store( false, MT::MEMORDER_RELEASE );
        }
        m_loginName = loginName;
        m_displayName = displayName;
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool WaitUntilDone( UInt32 timeoutInMs )
    {
        UInt32 waited = 0;
        while ( !m_done.Load( MT::MEMORDER_ACQUIRE ) && waited < timeoutInMs )
        {
            MT::PrecisionDelay( 1 );
            ++waited;
        }
        return m_done.Load( MT::MEMORDER_ACQUIRE );
    }

    bool IsConsistent( void ) const { return m_consistent.Load( MT::MEMORDER_ACQUIRE ); }

    CORE::CString m_loginName;
    CORE::CString m_displayName;

    private:

    MT::CTAtomic< bool > m_consistent;
    MT::CTAtomic< bool > m_done;
};

/*-------------------------------------------------------------------------*/

void
PerformUserNameTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING UserName TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "UserName" );

    GUCEF_TESTFW_TESTCASE( "Test: The GECOS field gives the full name before the first comma, trimmed" )
        try
        {
            ASSERT_TRUE( "Dinand van Velzen" == CORE::GetDisplayNameFromGecos( "Dinand van Velzen,Room 1,555-1234,,other" ) );
            ASSERT_TRUE( "Dinand van Velzen" == CORE::GetDisplayNameFromGecos( "Dinand van Velzen" ) );
            ASSERT_TRUE( "Dinand van Velzen" == CORE::GetDisplayNameFromGecos( "  Dinand van Velzen  ,x" ) );
            ASSERT_TRUE( CORE::GetDisplayNameFromGecos( ",,," ).IsNULLOrEmpty() );
            ASSERT_TRUE( CORE::GetDisplayNameFromGecos( "" ).IsNULLOrEmpty() );
            ASSERT_TRUE( CORE::GetDisplayNameFromGecos( "   " ).IsNULLOrEmpty() );
            ASSERT_TRUE( CORE::GetDisplayNameFromGecos( ",Room 1" ).IsNULLOrEmpty() );
            // UTF-8 stays as it is
            ASSERT_TRUE( "J\xC3\xBCrgen M\xC3\xBCller" == CORE::GetDisplayNameFromGecos( "J\xC3\xBCrgen M\xC3\xBCller,,," ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The user name from the environment is USER, then LOGNAME, and follows the overrides of GetEnv" )
        try
        {
            #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
            const CORE::CString originalUser = CORE::GetEnv( "USER" );
            const CORE::CString originalLogname = CORE::GetEnv( "LOGNAME" );

            CORE::SetEnvOverride( "USER", "envuser" );
            CORE::SetEnvOverride( "LOGNAME", "envlogname" );
            ASSERT_TRUE( "envuser" == CORE::GetUserNameFromEnvironment() );

            CORE::SetEnvOverride( "USER", "" );
            ASSERT_TRUE( "envlogname" == CORE::GetUserNameFromEnvironment() );

            CORE::SetEnvOverride( "LOGNAME", "" );
            ASSERT_TRUE( CORE::GetUserNameFromEnvironment().IsNULLOrEmpty() );

            // what the process had is back
            CORE::SetEnvOverride( "USER", originalUser );
            CORE::SetEnvOverride( "LOGNAME", originalLogname );
            ASSERT_TRUE( ( !originalUser.IsNULLOrEmpty() ? originalUser : originalLogname ) == CORE::GetUserNameFromEnvironment() );
            #endif
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_APPLE_MACOS ) )

    GUCEF_TESTFW_TESTCASE( "Test: The login name is the one of the user database for the effective user id, whatever the environment says" )
        try
        {
            struct passwd entry;
            struct passwd* found = GUCEF_NULL;
            char buffer[ 8192 ];
            ASSERT_TRUE( 0 == getpwuid_r( geteuid(), &entry, buffer, sizeof( buffer ), &found ) && GUCEF_NULL != found );

            const CORE::CString loginName = CORE::GetCurrentUserName();
            ASSERT_FALSE( loginName.IsNULLOrEmpty() );
            ASSERT_TRUE( CORE::CString( found->pw_name ) == loginName );

            // the environment is a fallback only: an override does not win over the database
            const CORE::CString originalUser = CORE::GetEnv( "USER" );
            CORE::SetEnvOverride( "USER", "somebody-else" );
            CORE::SetEnvOverride( "LOGNAME", "somebody-else" );
            ASSERT_TRUE( loginName == CORE::GetCurrentUserName() );
            CORE::SetEnvOverride( "USER", originalUser );
            CORE::SetEnvOverride( "LOGNAME", originalUser );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The login name is the output of id -un" )
        try
        {
            FILE* pipe = popen( "id -un", "r" );
            ASSERT_TRUE( GUCEF_NULL != pipe );
            char line[ 512 ];
            line[ 0 ] = '\0';
            const char* read = fgets( line, sizeof( line ), pipe );
            pclose( pipe );
            ASSERT_TRUE( GUCEF_NULL != read );
            // the line end and the blanks around the name are not part of it
            char* lineEnd = strpbrk( line, "\r\n" );
            if ( GUCEF_NULL != lineEnd )
                *lineEnd = '\0';
            const CORE::CString idName = CORE::CString( line ).Trim( true ).Trim( false );
            ASSERT_FALSE( idName.IsNULLOrEmpty() );
            ASSERT_TRUE( idName == CORE::GetCurrentUserName() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The display name is the first field of the GECOS field of the user database, empty when that is empty" )
        try
        {
            struct passwd entry;
            struct passwd* found = GUCEF_NULL;
            char buffer[ 8192 ];
            ASSERT_TRUE( 0 == getpwuid_r( geteuid(), &entry, buffer, sizeof( buffer ), &found ) && GUCEF_NULL != found );
            const CORE::CString expected = CORE::GetDisplayNameFromGecos( GUCEF_NULL != found->pw_gecos ? found->pw_gecos : "" );
            ASSERT_TRUE( expected == CORE::GetCurrentUserDisplayName() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #endif

    GUCEF_TESTFW_TESTCASE( "Test: Neither name has a control character or a line break" )
        try
        {
            const CORE::CString names[ 2 ] = { CORE::GetCurrentUserName(), CORE::GetCurrentUserDisplayName() };
            for ( UInt32 n=0; n<2; ++n )
            {
                const char* p = names[ n ].C_String();
                while ( GUCEF_NULL != p && '\0' != *p )
                {
                    ASSERT_TRUE( (unsigned char) *p >= 0x20 && (unsigned char) *p != 0x7F );
                    ++p;
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Several threads asking at once get the same names as this thread" )
        try
        {
            const CORE::CString loginName = CORE::GetCurrentUserName();
            const CORE::CString displayName = CORE::GetCurrentUserDisplayName();

            CUserNameAsker askers[ 4 ];
            for ( UInt32 i=0; i<4; ++i )
                ASSERT_TRUE( askers[ i ].Activate( GUCEF_NULL ) );
            for ( UInt32 i=0; i<4; ++i )
            {
                ASSERT_TRUE( askers[ i ].WaitUntilDone( 20000 ) );
                ASSERT_TRUE( askers[ i ].IsConsistent() );
                ASSERT_TRUE( loginName == askers[ i ].m_loginName );
                ASSERT_TRUE( displayName == askers[ i ].m_displayName );
            }
            for ( UInt32 i=0; i<4; ++i )
                askers[ i ].WaitForThreadToFinish( 10000 );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
