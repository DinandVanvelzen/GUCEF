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

#ifndef GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H
#include "gucefCORE_CTVariantMapAdapter_Cpp2C.h"
#define GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H
#endif /* GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H ? */

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVariantMapAdapter.h"

using namespace GUCEF;

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

namespace {

/** What a C plugin does: hands keys and values over as variant data */
UInt8
CInsert( CORE::TVariantMapApi* api, const char* key, const CORE::CVariant& value )
{
    CORE::CVariant keyVar( key );
    CORE::CVariant valueVar( value );
    return api->insert( api->privateData, const_cast< CORE::TVariantData* >( keyVar.CStyleAccess() ), const_cast< CORE::TVariantData* >( valueVar.CStyleAccess() ), 0 );
}

UInt8
CSet( CORE::TVariantMapApi* api, const char* key, const CORE::CVariant& value )
{
    CORE::CVariant keyVar( key );
    CORE::CVariant valueVar( value );
    return api->set( api->privateData, const_cast< CORE::TVariantData* >( keyVar.CStyleAccess() ), const_cast< CORE::TVariantData* >( valueVar.CStyleAccess() ), 0 );
}

/** Hands the value over as it is, the caller keeps it alive: a linked map entry refers to its memory */
UInt8
CInsertInPlace( CORE::TVariantMapApi* api, const char* key, const CORE::CVariant& value, bool linkIfPossible )
{
    CORE::CVariant keyVar( key );
    return api->insert( api->privateData, const_cast< CORE::TVariantData* >( keyVar.CStyleAccess() ), const_cast< CORE::TVariantData* >( value.CStyleAccess() ), linkIfPossible ? 1 : 0 );
}

UInt8
CSetInPlace( CORE::TVariantMapApi* api, const char* key, const CORE::CVariant& value, bool linkIfPossible )
{
    CORE::CVariant keyVar( key );
    return api->set( api->privateData, const_cast< CORE::TVariantData* >( keyVar.CStyleAccess() ), const_cast< CORE::TVariantData* >( value.CStyleAccess() ), linkIfPossible ? 1 : 0 );
}

CORE::CVariant
CAtKey( const CORE::TVariantMapApi* api, const char* key )
{
    CORE::CVariant keyVar( key );
    CORE::TVariantData out;
    memset( &out, 0, sizeof( out ) );
    api->at_key( api->privateData, const_cast< CORE::TVariantData* >( keyVar.CStyleAccess() ), &out );
    CORE::CVariant result;
    result.LinkTo( &out );
    return CORE::CVariant( result );    // a copy that owns its data
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVariantMapAdapterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefCORE.VariantMapAdapter" );

    GUCEF_TESTFW_TESTCASE( "VariantMapAdapter: Values are stored as the value type of the map, not the key type" )
    try
    {
        typedef CORE::CTVariantMapAdapterCpp2C< CORE::CString, Int32 > TAdapter;
        TAdapter::TLinkedMapType map;
        TAdapter adapter;
        adapter.SetMapAccessPtr( &map );
        CORE::TVariantMapApi* api = adapter.GetCStyleAccess();

        ASSERT_TRUE( 1 == CInsert( api, "volume", CORE::CVariant( (Int32) 42 ) ) );
        ASSERT_TRUE( 1 == map.size() );
        ASSERT_TRUE( 42 == map[ "volume" ] );
        ASSERT_TRUE( 42 == CAtKey( api, "volume" ).AsInt32() );

        // a value of another type is converted to the value type
        ASSERT_TRUE( 1 == CInsert( api, "loops", CORE::CVariant( "7" ) ) );
        ASSERT_TRUE( 7 == map[ "loops" ] );

        // insert does not overwrite
        ASSERT_TRUE( 0 == CInsert( api, "volume", CORE::CVariant( (Int32) 1 ) ) );
        ASSERT_TRUE( 42 == map[ "volume" ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VariantMapAdapter: Set overwrites an existing key and adds a missing one, as the C interface describes" )
    try
    {
        typedef CORE::CTVariantMapAdapterCpp2C< CORE::CString, CORE::CString > TAdapter;
        TAdapter::TLinkedMapType map;
        TAdapter adapter;
        adapter.SetMapAccessPtr( &map );
        CORE::TVariantMapApi* api = adapter.GetCStyleAccess();

        map[ "title" ] = "old";
        ASSERT_TRUE( 1 == CSet( api, "title", CORE::CVariant( "new" ) ) );
        ASSERT_TRUE( "new" == map[ "title" ] );
        ASSERT_TRUE( 1 == CSet( api, "artist", CORE::CVariant( "someone" ) ) );
        ASSERT_TRUE( 2 == map.size() );
        ASSERT_TRUE( "someone" == map[ "artist" ] );
        ASSERT_TRUE( 2 == api->size( api->privateData ) );

        // a map given as const can be read, not changed
        TAdapter readOnly;
        readOnly.SetMapAccessPtr( (const TAdapter::TLinkedMapType*) &map );
        CORE::TVariantMapApi* readApi = readOnly.GetCStyleAccess();
        ASSERT_TRUE( "someone" == CAtKey( readApi, "artist" ).AsString() );
        ASSERT_TRUE( 0 == CSet( readApi, "title", CORE::CVariant( "changed" ) ) );
        ASSERT_TRUE( 0 == CInsert( readApi, "other", CORE::CVariant( "x" ) ) );
        ASSERT_TRUE( "new" == map[ "title" ] && 2 == map.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VariantMapAdapter: A buffer entry links to the data of the caller when asked, else holds a copy" )
    try
    {
        typedef CORE::CTVariantMapAdapterCpp2C< CORE::CString, CORE::CDynamicBuffer > TAdapter;
        TAdapter::TLinkedMapType map;
        TAdapter adapter;
        adapter.SetMapAccessPtr( &map );
        CORE::TVariantMapApi* api = adapter.GetCStyleAccess();

        const char blob[] = { 1, 2, 3, 4, 5 };
        CORE::CVariant blobVar;
        blobVar.LinkTo( (const void*) blob, (UInt32) sizeof( blob ) );

        // linked: the entry is the memory of the caller, nothing is copied
        ASSERT_TRUE( 1 == CInsertInPlace( api, "linked", blobVar, true ) );
        ASSERT_TRUE( map[ "linked" ].IsLinked() );
        ASSERT_TRUE( blob == map[ "linked" ].GetConstBufferPtr() );
        ASSERT_TRUE( sizeof( blob ) == map[ "linked" ].GetDataSize() );

        // not linked: a copy of its own with the same content
        ASSERT_TRUE( 1 == CInsertInPlace( api, "copied", blobVar, false ) );
        ASSERT_FALSE( map[ "copied" ].IsLinked() );
        ASSERT_TRUE( blob != map[ "copied" ].GetConstBufferPtr() );
        ASSERT_TRUE( sizeof( blob ) == map[ "copied" ].GetDataSize() );
        ASSERT_TRUE( 0 == memcmp( blob, map[ "copied" ].GetConstBufferPtr(), sizeof( blob ) ) );

        // a set without linking over a linked entry: the entry lets go of the memory of the caller and copies
        const char other[] = { 9, 8, 7 };
        CORE::CVariant otherVar;
        otherVar.LinkTo( (const void*) other, (UInt32) sizeof( other ) );
        ASSERT_TRUE( 1 == CSetInPlace( api, "linked", otherVar, false ) );
        ASSERT_FALSE( map[ "linked" ].IsLinked() );
        ASSERT_TRUE( sizeof( other ) == map[ "linked" ].GetDataSize() );
        ASSERT_TRUE( 0 == memcmp( other, map[ "linked" ].GetConstBufferPtr(), sizeof( other ) ) );

        // a set with linking over a copy: the entry now refers to the memory of the caller
        ASSERT_TRUE( 1 == CSetInPlace( api, "copied", otherVar, true ) );
        ASSERT_TRUE( map[ "copied" ].IsLinked() );
        ASSERT_TRUE( other == map[ "copied" ].GetConstBufferPtr() );

        // a fixed size value lives in the variant handed over, so it is always copied
        CORE::CVariant number( (UInt32) 0x01020304 );
        ASSERT_TRUE( 1 == CInsertInPlace( api, "number", number, true ) );
        ASSERT_FALSE( map[ "number" ].IsLinked() );
        ASSERT_TRUE( sizeof( UInt32 ) == map[ "number" ].GetDataSize() );
        ASSERT_TRUE( 0x01020304 == map[ "number" ].AsConstType< UInt32 >() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VariantMapAdapter: A variant entry links to the heap data of the caller when asked, else holds a copy" )
    try
    {
        typedef CORE::CTVariantMapAdapterCpp2C< CORE::CString, CORE::CVariant > TAdapter;
        TAdapter::TLinkedMapType map;
        TAdapter adapter;
        adapter.SetMapAccessPtr( &map );
        CORE::TVariantMapApi* api = adapter.GetCStyleAccess();

        CORE::CVariant text( "some text that lives on the heap" );
        ASSERT_TRUE( 1 == CInsertInPlace( api, "linked", text, true ) );
        ASSERT_TRUE( map[ "linked" ].IsDynamicMemoryLinked() );
        ASSERT_TRUE( text.AsVoidPtr() == map[ "linked" ].AsVoidPtr() );
        ASSERT_TRUE( "some text that lives on the heap" == map[ "linked" ].AsString() );

        ASSERT_TRUE( 1 == CInsertInPlace( api, "copied", text, false ) );
        ASSERT_FALSE( map[ "copied" ].IsDynamicMemoryLinked() );
        ASSERT_TRUE( text.AsVoidPtr() != map[ "copied" ].AsVoidPtr() );
        ASSERT_TRUE( "some text that lives on the heap" == map[ "copied" ].AsString() );

        // a value without heap data is held in the entry itself
        ASSERT_TRUE( 1 == CSetInPlace( api, "linked", CORE::CVariant( (Int32) 5 ), true ) );
        ASSERT_FALSE( map[ "linked" ].IsDynamicMemoryLinked() );
        ASSERT_TRUE( 5 == map[ "linked" ].AsInt32() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
