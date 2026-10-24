/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVfsCodecs.h"

#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* TestCodecFamily = "VfsTestCodec";
static const char* TestCodecType = "marking";

/*-------------------------------------------------------------------------*/

/** A codec whose output tells which way it ran: "E:" before the input when encoding, "D:" when decoding */
class CMarkingTestCodec : public CORE::CICodec
{
    public:

    virtual bool Encode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return Mark( "E:", source, dest );
    }

    virtual bool Decode( CORE::CIOAccess& source, CORE::CIOAccess& dest ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return Mark( "D:", source, dest );
    }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return TestCodecType;
    }

    virtual CORE::CString GetFamilyName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return TestCodecFamily;
    }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return GUCEF_NEW CMarkingTestCodec();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CMarkingTestCodec";
        return classTypeName;
    }

    private:

    static bool Mark( const char* mark, CORE::CIOAccess& source, CORE::CIOAccess& dest )
    {GUCEF_TRACE;

        if ( 2 != dest.Write( mark, 1, 2 ) )
            return false;
        char buffer[ 256 ];
        UInt32 bytesRead = 0;
        while ( 0 < ( bytesRead = source.Read( buffer, 1, sizeof( buffer ) ) ) )
        {
            if ( bytesRead != dest.Write( buffer, 1, bytesRead ) )
                return false;
        }
        return true;
    }
};

/*-------------------------------------------------------------------------*/

static void
RegisterMarkingTestCodec( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CCodecRegistry::TCodecFamilyRegistryPtr family;
    if ( !codecRegistry.TryLookup( TestCodecFamily, family, true ) )
    {
        family = CORE::CCodecRegistry::TCodecFamilyRegistryTypedPtr( GUCEF_NEW CORE::CCodecRegistry::TCodecFamilyRegistry() );
        codecRegistry.TryRegister( TestCodecFamily, family );
    }
    family->TryRegister( TestCodecType, CORE::CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CMarkingTestCodec() ) );
}

/*-------------------------------------------------------------------------*/

static void
UnregisterMarkingTestCodec( void )
{GUCEF_TRACE;

    CORE::CCodecRegistry& codecRegistry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
    CORE::CCodecRegistry::TCodecFamilyRegistryPtr family;
    if ( codecRegistry.TryLookup( TestCodecFamily, family, true ) )
        family->TryUnregister( TestCodecType );
    codecRegistry.TryUnregister( TestCodecFamily );
}

/*-------------------------------------------------------------------------*/

void
PerformVfsCodecTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "VfsCodecs" );

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    // Regression: DecodeFile ran the codec the encoding way
    GUCEF_TESTFW_TESTCASE( "VfsCodecs: EncodeFile encodes and DecodeFile decodes" )
        try
        {
            const CORE::CString dir = CORE::CombinePath( CORE::TempDir(), "gucefVFS_TestApp_Codecs" );
            CORE::DeleteDir( dir, true );
            CORE::CreateDirs( dir );
            ASSERT_TRUE( CORE::WriteStringAsTextFile( CORE::CombinePath( dir, "plain.txt" ), "payload" ) );
            ASSERT_TRUE( vfs.AddRoot( dir, "VfsCodecTest", true, false, "vfscodectest" ) );
            RegisterMarkingTestCodec();

            ASSERT_TRUE( vfs.EncodeFile( "vfscodectest/plain.txt", "vfscodectest/encoded.txt", true, TestCodecFamily, TestCodecType ) );
            ASSERT_TRUE( vfs.DecodeFile( "vfscodectest/plain.txt", "vfscodectest/decoded.txt", true, TestCodecFamily, TestCodecType ) );

            CORE::CString encoded;
            CORE::CString decoded;
            ASSERT_TRUE( CORE::LoadTextFileAsString( CORE::CombinePath( dir, "encoded.txt" ), encoded ) );
            ASSERT_TRUE( CORE::LoadTextFileAsString( CORE::CombinePath( dir, "decoded.txt" ), decoded ) );
            ASSERT_TRUE( "E:payload" == encoded );
            ASSERT_TRUE( "D:payload" == decoded );

            CORE::CDynamicBuffer decodedBuffer;
            ASSERT_TRUE( vfs.DecodeAsFile( decodedBuffer, 0, "vfscodectest/plain.txt", TestCodecFamily, TestCodecType ) );
            ASSERT_TRUE( "D:payload" == CORE::CString( decodedBuffer.AsConstTypePtr< char >(), decodedBuffer.GetDataSize() ) );

            UnregisterMarkingTestCodec();
            ASSERT_TRUE( vfs.UnmountArchiveByName( "VfsCodecTest" ) );
            CORE::DeleteDir( dir, true );
        }
        catch ( ... )
        {
            GUCEF_TESTFW_ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
