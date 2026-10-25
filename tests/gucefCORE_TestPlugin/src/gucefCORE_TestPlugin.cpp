/*
 *  gucefCORE_TestPlugin: generic plugin used by the gucefCORE tests of the plugin system
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
#include <stdlib.h>
#include <string.h>
#include <stdexcept>

#ifndef GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#include "gucefCORE_CGloballyConfigurable.h"
#define GUCEF_CORE_CGLOBALLYCONFIGURABLE_H
#endif /* GUCEF_CORE_CGLOBALLYCONFIGURABLE_H ? */

#include "gucefCORE_TestPlugin.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A configurable of the process, as the globals that plugins bring along are, whose config always fails */
class CFailingConfigurable : public GUCEF::CORE::CGloballyConfigurable
{
    public:

    CFailingConfigurable( void ) : GUCEF::CORE::CGloballyConfigurable( false ) {}

    virtual bool SaveConfig( GUCEF::CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool LoadConfig( const GUCEF::CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "GUCEF::CORE::TestPlugin::CFailingConfigurable";
        return classTypeName;
    }
};

static CFailingConfigurable* g_failingConfigurable = GUCEF_NULL;

/*-------------------------------------------------------------------------*/

static bool
IsFlagSet( const char* variable )
{
    const char* value = getenv( variable );
    return GUCEF_NULL != value && 0 == strcmp( value, "1" );
}

/*-------------------------------------------------------------------------*/

/** Counts calls in an environment variable, which the tests read; the module itself may be gone by then */
static void
CountCall( const char* variable )
{
    const char* current = getenv( variable );
    char text[ 32 ];
    snprintf( text, sizeof( text ), "%d", ( GUCEF_NULL != current ? atoi( current ) : 0 ) + 1 );
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    _putenv_s( variable, text );
    #else
    setenv( variable, text, 1 );
    #endif
}

/*-------------------------------------------------------------------------*/

GUCEF::CORE::Int32 GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Load( GUCEF::CORE::UInt32 argc, const char** argv ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CountCall( "GUCEF_TESTPLUGIN_LOADS" );
    if ( IsFlagSet( "GUCEF_TESTPLUGIN_LOAD_THROWS" ) )
        throw std::runtime_error( "gucefCORE_TestPlugin: Load fails on purpose" );
    if ( IsFlagSet( "GUCEF_TESTPLUGIN_LOAD_FAILS" ) )
        return 0;
    if ( IsFlagSet( "GUCEF_TESTPLUGIN_LOAD_ADDS_FAILING_CONFIGURABLE" ) && GUCEF_NULL == g_failingConfigurable )
        g_failingConfigurable = new CFailingConfigurable();
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_Unload( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    CountCall( "GUCEF_TESTPLUGIN_UNLOADS" );
    delete g_failingConfigurable;
    g_failingConfigurable = GUCEF_NULL;
    const char* throws = getenv( "GUCEF_TESTPLUGIN_UNLOAD_THROWS" );
    if ( GUCEF_NULL != throws && 0 == strcmp( throws, "1" ) )
        throw std::runtime_error( "gucefCORE_TestPlugin: Unload fails on purpose" );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetVersion( GUCEF::CORE::TVersion* versionInfo ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    versionInfo->major = 1;
    versionInfo->minor = 0;
    versionInfo->patch = 0;
    versionInfo->release = 0;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetCopyright( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen, LGPL";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
GUCEFPlugin_GetDescription( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Plugin used by the gucefCORE tests of the plugin system";
}

/*-------------------------------------------------------------------------*/
