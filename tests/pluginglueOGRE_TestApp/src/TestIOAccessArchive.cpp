/*
 *  pluginglueOGRE_TestApp: Test application for the Ogre glue modules (pluginglueOGRE, OgrePlugin_GucefVFS, OgreCodec_GucefIMAGE)
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <time.h>

#include "OgreArchiveManager.h"
#include "OgreResourceGroupManager.h"

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H
#include "pluginglueOGRECOMMON_COgreIOAccessArchive.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREIOACCESSARCHIVE_H ? */

#include "TestIOAccessArchive.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::OGRECOMMON;

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

static CORE::IOAccessPtr
MakeResource( const char* text )
{GUCEF_TRACE;

    // the access object owns the content
    CORE::CDynamicBuffer* buffer = GUCEF_NEW CORE::CDynamicBuffer( true );
    buffer->CopyFrom( (UInt32) strlen( text ), text );
    return CORE::IOAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( buffer, true ) );
}

/*-------------------------------------------------------------------------*/

static void
TestResources( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "IOAccess archive: named resources are opened, listed, found and have a modification time" )
        try
        {
            COgreIOAccessArchive archive( "memory", "GucefIOAccess" );
            archive.load();
            ASSERT_TRUE( archive.isReadOnly() );

            ASSERT_TRUE( archive.AddResource( "hello.txt", MakeResource( "hello" ), 1000 ) );
            ASSERT_TRUE( archive.AddResource( "gen/world.txt", MakeResource( "generated world" ), 2000 ) );
            ASSERT_TRUE( archive.AddResource( "gen\\deeper\\x.bin", MakeResource( "xyz" ) ) );
            ASSERT_FALSE( archive.AddResource( "", MakeResource( "nothing" ) ) );
            ASSERT_FALSE( archive.AddResource( "../out.txt", MakeResource( "out" ) ) );
            ASSERT_FALSE( archive.AddResource( "empty.txt", CORE::IOAccessPtr() ) );
            ASSERT_TRUE( 3 == archive.GetResourceCount() );

            Ogre::DataStreamPtr stream = archive.open( "gen/world.txt" );
            ASSERT_TRUE( (bool) stream );
            if ( stream )
            {
                ASSERT_TRUE( 15 == stream->size() );
                ASSERT_TRUE( "generated world" == stream->getAsString() );
            }
            // opened again it starts at the start
            stream = archive.open( "gen/world.txt" );
            ASSERT_TRUE( (bool) stream && "generated world" == stream->getAsString() );
            ASSERT_FALSE( (bool) archive.open( "nothing.txt" ) );
            ASSERT_FALSE( (bool) archive.open( "gen" ) );

            ASSERT_TRUE( archive.exists( "hello.txt" ) );
            ASSERT_TRUE( archive.exists( "gen/deeper/x.bin" ) );
            ASSERT_FALSE( archive.exists( "gen" ) );
            ASSERT_FALSE( archive.exists( "nothing" ) );

            ASSERT_TRUE( 1000 == archive.getModifiedTime( "hello.txt" ) );
            ASSERT_TRUE( 2000 == archive.getModifiedTime( "gen/world.txt" ) );
            ASSERT_TRUE( 0 == archive.getModifiedTime( "nothing" ) );
            // no time given: the time of the registration
            const time_t added = archive.getModifiedTime( "gen/deeper/x.bin" );
            const time_t now = time( GUCEF_NULL );
            ASSERT_TRUE( added <= now && added >= now - 60 );

            // the directories follow from the names
            Ogre::StringVectorPtr all = archive.list( true, false );
            ASSERT_TRUE( (bool) all && 3 == all->size() );
            Ogre::StringVectorPtr top = archive.list( false, false );
            ASSERT_TRUE( (bool) top && 1 == top->size() && "hello.txt" == (*top)[ 0 ] );
            Ogre::StringVectorPtr dirs = archive.list( true, true );
            ASSERT_TRUE( (bool) dirs && 2 == dirs->size() );
            Ogre::StringVectorPtr found = archive.find( "*.txt", true, false );
            ASSERT_TRUE( (bool) found && 2 == found->size() );
            found = archive.find( "gen/*", false, false );
            ASSERT_TRUE( (bool) found && 1 == found->size() && "gen/world.txt" == (*found)[ 0 ] );

            Ogre::FileInfoListPtr infos = archive.findFileInfo( "x.bin", true, false );
            ASSERT_TRUE( (bool) infos && 1 == infos->size() );
            if ( infos && 1 == infos->size() )
            {
                ASSERT_TRUE( "gen/deeper/x.bin" == (*infos)[ 0 ].filename );
                ASSERT_TRUE( 3 == (*infos)[ 0 ].uncompressedSize );
                ASSERT_TRUE( &archive == (*infos)[ 0 ].archive );
            }

            // replace and remove
            ASSERT_TRUE( archive.AddResource( "hello.txt", MakeResource( "hello again" ), 3000 ) );
            ASSERT_TRUE( 3 == archive.GetResourceCount() );
            stream = archive.open( "hello.txt" );
            ASSERT_TRUE( (bool) stream && "hello again" == stream->getAsString() && 3000 == archive.getModifiedTime( "hello.txt" ) );
            ASSERT_TRUE( archive.RemoveResource( "hello.txt" ) );
            ASSERT_FALSE( archive.RemoveResource( "hello.txt" ) );
            ASSERT_FALSE( archive.exists( "hello.txt" ) );
            ASSERT_TRUE( 2 == archive.list( true, false )->size() );

            // a stream that is open keeps its resource, whatever the archive does
            stream = archive.open( "gen/world.txt" );
            archive.ClearResources();
            ASSERT_TRUE( 0 == archive.GetResourceCount() );
            ASSERT_TRUE( (bool) stream && "generated world" == stream->getAsString() );

            bool createRefused = false;
            try { archive.create( "new.txt" ); } catch ( const Ogre::Exception& ) { createRefused = true; }
            ASSERT_TRUE( createRefused );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestThroughOgre( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "IOAccess archive: the archive type GucefIOAccess of the plugin serves the resources that the application registered, to the resource groups of Ogre" )
        try
        {
            Ogre::ArchiveManager& archives = Ogre::ArchiveManager::getSingleton();
            ASSERT_TRUE( archives.hasArchiveFactory( "GucefIOAccess" ) );

            // the application takes the archive that Ogre makes for the location, and gives it its resources
            Ogre::Archive* ogreArchive = archives.load( "generated", "GucefIOAccess", true );
            COgreIOAccessArchive* archive = COgreIOAccessArchive::FromArchive( ogreArchive, "GucefIOAccess" );
            ASSERT_TRUE( GUCEF_NULL != archive );
            ASSERT_TRUE( GUCEF_NULL == COgreIOAccessArchive::FromArchive( ogreArchive, "SomethingElse" ) );
            if ( GUCEF_NULL == archive )
                return;

            ASSERT_TRUE( archive->AddResource( "shader/test.glsl", MakeResource( "void main() {}" ) ) );
            ASSERT_TRUE( archive->AddResource( "data.cfg", MakeResource( "key=value" ) ) );

            Ogre::ResourceGroupManager& groups = Ogre::ResourceGroupManager::getSingleton();
            groups.createResourceGroup( "OgreTestMemoryGroup" );
            groups.addResourceLocation( "generated", "GucefIOAccess", "OgreTestMemoryGroup", true );

            ASSERT_TRUE( groups.resourceExists( "OgreTestMemoryGroup", "shader/test.glsl" ) );
            ASSERT_TRUE( groups.resourceExists( "OgreTestMemoryGroup", "data.cfg" ) );
            ASSERT_FALSE( groups.resourceExists( "OgreTestMemoryGroup", "nothing.cfg" ) );
            Ogre::DataStreamPtr stream = groups.openResource( "data.cfg", "OgreTestMemoryGroup" );
            ASSERT_TRUE( (bool) stream && "key=value" == stream->getAsString() );
            ASSERT_TRUE( 2 == groups.listResourceNames( "OgreTestMemoryGroup" )->size() );
            stream.reset();

            groups.destroyResourceGroup( "OgreTestMemoryGroup" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformIOAccessArchiveTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "OgrePlugin_GucefVFS.IOAccessArchive" );

    TestResources();
    TestThroughOgre();
}

/*-------------------------------------------------------------------------*/
