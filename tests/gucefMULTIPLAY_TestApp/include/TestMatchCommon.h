/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTMATCHCOMMON_H
#define GUCEF_MULTIPLAY_TESTAPP_TESTMATCHCOMMON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "TestPump.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A directory for the files of a test, under the working directory of the test application, emptied first */
inline GUCEF::CORE::CString
MatchTestDirectory( const char* name )
{GUCEF_TRACE;

    GUCEF::CORE::CString dir = GUCEF::CORE::CombinePath( GUCEF::CORE::CombinePath( GUCEF::CORE::RelativePath( "$CURWORKDIR$" ), "multiplaytemp" ), "matches" );
    dir = GUCEF::CORE::CombinePath( dir, name );
    GUCEF::CORE::DeleteDir( dir, true );
    GUCEF::CORE::CreateDirs( dir );
    return dir;
}

/*-------------------------------------------------------------------------*/

/** Serializes the source to a data tree, stores it with the JSON codec, reads it back and deserializes it into the target */
inline bool
MatchJsonRoundTrip( const GUCEF::CORE::CIDataNodeSerializable& source, GUCEF::CORE::CIDataNodeSerializable& target, const char* testName )
{GUCEF_TRACE;

    GUCEF::CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !JsonCodecAvailable() || !GUCEF::CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) || codec.IsNULL() )
        return false;
    GUCEF::CORE::CString path = GUCEF::CORE::CombinePath( MatchTestDirectory( testName ), "roundtrip.json" );
    GUCEF::CORE::CDataNode root( "Root" );
    GUCEF::CORE::CDataNodeSerializableSettings settings;
    if ( !source.Serialize( root, settings ) || !codec->StoreDataTree( &root, path ) )
        return false;
    GUCEF::CORE::CDataNode read;
    return codec->BuildDataTree( &read, path ) && target.Deserialize( read, settings );
}

/*-------------------------------------------------------------------------*/

/** Whether the gzip codec is there, loading its plugin when it is not */
inline bool
GzipCodecAvailable( void )
{GUCEF_TRACE;

    if ( !GUCEF::MULTIPLAY::CMatchContainer::IsCompressionAvailable() )
        LoadTestPlugin( "codecspluginZLIB" );
    return GUCEF::MULTIPLAY::CMatchContainer::IsCompressionAvailable();
}

/*-------------------------------------------------------------------------*/

/** A small deterministic generator for the mutation tests: the same run every time */
class CTestRandom
{
    public:

    explicit CTestRandom( GUCEF::UInt32 seed ) : m_state( seed ) {}

    GUCEF::UInt32 Next( void )
    {
        m_state = m_state * 1664525u + 1013904223u;
        return m_state >> 8;
    }

    private:

    GUCEF::UInt32 m_state;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTMATCHCOMMON_H ? */
