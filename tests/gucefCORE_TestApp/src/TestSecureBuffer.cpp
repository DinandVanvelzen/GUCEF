/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_SECUREMEMORY_H
#include "gucefCORE_SecureMemory.h"
#define GUCEF_CORE_SECUREMEMORY_H
#endif /* GUCEF_CORE_SECUREMEMORY_H ? */

#ifndef GUCEF_CORE_CSECUREBUFFER_H
#include "gucefCORE_CSecureBuffer.h"
#define GUCEF_CORE_CSECUREBUFFER_H
#endif /* GUCEF_CORE_CSECUREBUFFER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestSecureBuffer.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsAllZero( const void* memory, UInt32 size )
{
    const UInt8* bytes = static_cast< const UInt8* >( memory );
    for ( UInt32 i=0; i<size; ++i )
        if ( 0 != bytes[ i ] )
            return false;
    return true;
}

/** What the release auditor saw: the storage of a buffer at the moment it was given back */
struct SRelease
{
    const void* storage;
    UInt32 size;
    bool wasZero;
    UInt32 count;
};

SRelease g_release = { GUCEF_NULL, 0, false, 0 };

void
Audit( const void* storage, UInt32 size )
{
    g_release.storage = storage;
    g_release.size = size;
    g_release.wasZero = IsAllZero( storage, size );
    ++g_release.count;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSecureBufferTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING SecureBuffer TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "SecureBuffer" );

    GUCEF_TESTFW_TESTCASE( "SecureWipe: overwrites the memory with zeros, accepts NULL and zero size" )
    try
    {
        UInt8 memory[ 64 ];
        memset( memory, 0xAA, sizeof( memory ) );
        CORE::SecureWipe( memory, sizeof( memory ) );
        ASSERT_TRUE( IsAllZero( memory, sizeof( memory ) ) );

        memset( memory, 0xAA, sizeof( memory ) );
        CORE::SecureWipe( memory + 8, 16 );
        ASSERT_TRUE( 0xAA == memory[ 7 ] && 0 == memory[ 8 ] && 0 == memory[ 23 ] && 0xAA == memory[ 24 ] );
        CORE::SecureWipe( memory, 0 );
        CORE::SecureWipe( GUCEF_NULL, 10 );
        ASSERT_TRUE( 0xAA == memory[ 0 ] && 0xAA == memory[ 63 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SecureWipeString: the string is empty afterwards and the memory of a string that borrows a buffer is left alone" )
    try
    {
        CORE::CString text( "correct horse battery staple" );
        CORE::SecureWipeString( text );
        ASSERT_TRUE( text.IsNULLOrEmpty() && 0 == text.Length() );

        CORE::SecureWipeString( text );      // an empty string is fine
        ASSERT_TRUE( text.IsNULLOrEmpty() );

        char borrowed[] = "owned by someone else";
        CORE::CString linked;
        linked.LinkTo( borrowed );
        CORE::SecureWipeString( linked );
        ASSERT_TRUE( linked.IsNULLOrEmpty() && 0 == strcmp( borrowed, "owned by someone else" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CSecureBuffer: the capacity is a hard bound, content is appended and replaced, a terminator always follows the content" )
    try
    {
        CORE::CSecureBuffer buffer( 8 );
        ASSERT_TRUE( 8 == buffer.GetCapacity() && 0 == buffer.GetSize() && buffer.IsEmpty() && buffer.IsOwningStorage() );
        ASSERT_TRUE( 0 == buffer.GetData()[ 0 ] );

        ASSERT_TRUE( buffer.Append( "abc", 3 ) && buffer.Append( "de", 2 ) );
        ASSERT_TRUE( 5 == buffer.GetSize() && 0 == memcmp( buffer.GetData(), "abcde", 5 ) && 0 == buffer.GetData()[ 5 ] );

        // 4 more bytes do not fit: nothing is appended
        ASSERT_FALSE( buffer.Append( "wxyz", 4 ) );
        ASSERT_TRUE( 5 == buffer.GetSize() && 0 == memcmp( buffer.GetData(), "abcde", 5 ) && 0 == buffer.GetData()[ 5 ] );
        ASSERT_TRUE( buffer.Append( "fgh", 3 ) && 8 == buffer.GetSize() && 0 == buffer.GetData()[ 8 ] );
        ASSERT_FALSE( buffer.Append( "i", 1 ) );

        // too large to assign: the old content is gone, not half replaced
        ASSERT_FALSE( buffer.Assign( "123456789", 9 ) );
        ASSERT_TRUE( buffer.IsEmpty() && IsAllZero( buffer.GetRawStorage(), buffer.GetRawStorageSize() ) );

        ASSERT_TRUE( buffer.Assign( "xyz", 3 ) && 3 == buffer.GetSize() && 0 == memcmp( buffer.GetData(), "xyz", 3 ) );
        ASSERT_TRUE( buffer.Append( GUCEF_NULL, 0 ) && 3 == buffer.GetSize() );
        ASSERT_FALSE( buffer.Append( GUCEF_NULL, 1 ) );

        CORE::CSecureBuffer none;
        ASSERT_TRUE( 0 == none.GetCapacity() && none.IsEmpty() && none.Assign( GUCEF_NULL, 0 ) && !none.Append( "a", 1 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CSecureBuffer: Truncate, Clear and a shorter Assign overwrite the bytes that are no longer content" )
    try
    {
        CORE::CSecureBuffer buffer( 32 );
        ASSERT_TRUE( buffer.Assign( "hunter2-hunter2-hunter2", 23 ) );
        const UInt8* raw = buffer.GetRawStorage();

        buffer.Truncate( 7 );
        ASSERT_TRUE( 7 == buffer.GetSize() && 0 == memcmp( raw, "hunter2", 7 ) && IsAllZero( raw + 7, buffer.GetRawStorageSize() - 7 ) );
        buffer.Truncate( 20 );          // never grows
        ASSERT_TRUE( 7 == buffer.GetSize() );

        ASSERT_TRUE( buffer.Assign( "ab", 2 ) );
        ASSERT_TRUE( 0 == memcmp( raw, "ab", 2 ) && IsAllZero( raw + 2, buffer.GetRawStorageSize() - 2 ) );

        buffer.Clear();
        ASSERT_TRUE( buffer.IsEmpty() && IsAllZero( raw, buffer.GetRawStorageSize() ) );
        ASSERT_TRUE( 32 == buffer.GetCapacity() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CSecureBuffer: storage the caller supplies is wiped when the buffer is destroyed and is never freed by it" )
    try
    {
        UInt8 storage[ 17 ];
        memset( storage, 0x5A, sizeof( storage ) );
        {
            CORE::CSecureBuffer buffer( storage, 16 );
            ASSERT_TRUE( 16 == buffer.GetCapacity() && !buffer.IsOwningStorage() && buffer.IsEmpty() );
            ASSERT_TRUE( IsAllZero( storage, sizeof( storage ) ) );      // taking the storage clears it
            ASSERT_TRUE( buffer.Assign( "topsecretvalue", 14 ) && 0 == memcmp( storage, "topsecretvalue", 14 ) );
            ASSERT_FALSE( buffer.SetCapacity( 64 ) );                     // storage that is not ours does not move
        }
        ASSERT_TRUE( IsAllZero( storage, sizeof( storage ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CSecureBuffer: owned storage is zero when it is given back, on destruction, on SetCapacity and on a failed copy" )
    try
    {
        CORE::CSecureBuffer::SetReleaseAuditor( &Audit );
        g_release.count = 0;
        const void* storage = GUCEF_NULL;
        {
            CORE::CSecureBuffer buffer( 24 );
            ASSERT_TRUE( buffer.Assign( "a-password-of-sorts", 19 ) );
            storage = buffer.GetRawStorage();
            ASSERT_TRUE( 0 == memcmp( storage, "a-password-of-sorts", 19 ) );
        }
        ASSERT_TRUE( 1 == g_release.count && storage == g_release.storage && 25 == g_release.size && g_release.wasZero );

        // a bigger capacity: the content moves, the old storage is wiped before it is freed
        g_release.count = 0;
        {
            CORE::CSecureBuffer buffer( 8 );
            ASSERT_TRUE( buffer.Assign( "secret", 6 ) );
            const void* oldStorage = buffer.GetRawStorage();
            ASSERT_TRUE( buffer.SetCapacity( 40 ) && 40 == buffer.GetCapacity() && 6 == buffer.GetSize() && 0 == memcmp( buffer.GetData(), "secret", 6 ) );
            ASSERT_TRUE( 1 == g_release.count && oldStorage == g_release.storage && g_release.wasZero );

            // a smaller one keeps what fits and wipes what does not
            g_release.count = 0;
            ASSERT_TRUE( buffer.SetCapacity( 4 ) && 4 == buffer.GetCapacity() && 4 == buffer.GetSize() && 0 == memcmp( buffer.GetData(), "secr", 4 ) && 0 == buffer.GetData()[ 4 ] );
            ASSERT_TRUE( 1 == g_release.count && g_release.wasZero );
            g_release.count = 0;
        }
        ASSERT_TRUE( 1 == g_release.count && g_release.wasZero );

        CORE::CSecureBuffer::SetReleaseAuditor( GUCEF_NULL );
    }
    catch ( ... )
    {
        CORE::CSecureBuffer::SetReleaseAuditor( GUCEF_NULL );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CSecureBuffer: a copy is deep, has storage of its own, and is wiped when it goes out of scope" )
    try
    {
        CORE::CSecureBuffer::SetReleaseAuditor( &Audit );
        g_release.count = 0;
        CORE::CSecureBuffer original( 16 );
        ASSERT_TRUE( original.Assign( "copied-secret", 13 ) );
        const void* copyStorage = GUCEF_NULL;
        {
            CORE::CSecureBuffer copy( original );
            ASSERT_TRUE( 13 == copy.GetSize() && 0 == memcmp( copy.GetData(), "copied-secret", 13 ) && copy.GetRawStorage() != original.GetRawStorage() );
            copyStorage = copy.GetRawStorage();
            original.Clear();
            ASSERT_TRUE( 13 == copy.GetSize() && 0 == memcmp( copy.GetData(), "copied-secret", 13 ) );
        }
        ASSERT_TRUE( 1 == g_release.count && copyStorage == g_release.storage && g_release.wasZero );

        // CopyFrom copies into the storage that is there, and says no when the content does not fit
        CORE::CSecureBuffer small( 4 );
        CORE::CSecureBuffer big( 16 );
        ASSERT_TRUE( big.Assign( "0123456789", 10 ) );
        ASSERT_FALSE( small.CopyFrom( big ) );
        ASSERT_TRUE( small.IsEmpty() );
        ASSERT_TRUE( big.Assign( "0123", 4 ) && small.CopyFrom( big ) && 4 == small.GetSize() && 0 == memcmp( small.GetData(), "0123", 4 ) );
        CORE::CSecureBuffer::SetReleaseAuditor( GUCEF_NULL );
    }
    catch ( ... )
    {
        CORE::CSecureBuffer::SetReleaseAuditor( GUCEF_NULL );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CSecureBuffer: Equals compares the whole content, GetWritableData and SetSize take bytes written in place, CopyToString ends the protection explicitly" )
    try
    {
        CORE::CSecureBuffer buffer( 16 );
        ASSERT_TRUE( buffer.Assign( "abcdef", 6 ) );
        ASSERT_TRUE( buffer.Equals( "abcdef", 6 ) );
        ASSERT_FALSE( buffer.Equals( "abcdeg", 6 ) );
        ASSERT_FALSE( buffer.Equals( "abcde", 5 ) );
        ASSERT_FALSE( buffer.Equals( "abcdefg", 7 ) );
        ASSERT_FALSE( buffer.Equals( "bbcdef", 6 ) );
        CORE::CSecureBuffer empty( 4 );
        ASSERT_TRUE( empty.Equals( GUCEF_NULL, 0 ) && !buffer.Equals( GUCEF_NULL, 0 ) );

        CORE::CSecureBuffer filled( 8 );
        memcpy( filled.GetWritableData(), "12345", 5 );
        ASSERT_TRUE( filled.SetSize( 5 ) && 5 == filled.GetSize() && 0 == filled.GetData()[ 5 ] && filled.Equals( "12345", 5 ) );
        ASSERT_FALSE( filled.SetSize( 9 ) );
        ASSERT_TRUE( 5 == filled.GetSize() );

        CORE::CString text;
        buffer.CopyToString( text );
        ASSERT_TRUE( "abcdef" == text );
        CORE::SecureWipeString( text );
        ASSERT_TRUE( text.IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL SecureBuffer TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
