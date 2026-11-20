/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_WEB_CREACTFLIGHTPAYLOAD_H
#include "gucefWEB_CReactFlightPayload.h"
#define GUCEF_WEB_CREACTFLIGHTPAYLOAD_H
#endif /* GUCEF_WEB_CREACTFLIGHTPAYLOAD_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestReactFlightPayload.h"

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

/** Loads a plugin from the directory of the test application, the debug or the release build of it */
bool
LoadFlightTestPlugin( const CORE::CString& pluginName )
{
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
}

/** Whether the "json" codec is there, loading its plugin when it is not */
bool
FlightJsonCodecAvailable( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        LoadFlightTestPlugin( "dstorepluginJSONPARSER" );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/** The text as a JavaScript string literal the way Next.js writes it into a page: quotes, backslashes, line ends and '<' escaped */
CORE::CString
ToJsLiteral( const CORE::CString& text )
{
    CORE::CString literal = "\"";
    const char* data = text.C_String();
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = data[ i ];
        if ( c == '\\' )
            literal += "\\\\";
        else
        if ( c == '"' )
            literal += "\\\"";
        else
        if ( c == '\n' )
            literal += "\\n";
        else
        if ( c == '<' )
            literal += "\\u003c";
        else
        {
            char one[ 2 ] = { c, 0 };
            literal += one;
        }
    }
    literal += "\"";
    return literal;
}

/** The text of a T row of the fixture: 12 bytes with a two byte character and a line end */
const char* const TextRowValue = "H\xC3\xA9llo\nworld";

/** A payload as a store page of a Next.js site could have it: every row kind the reader must cope with */
CORE::CString
FixturePayloadPart( UInt32 part )
{
    if ( 0 == part )
        return CORE::CString( "1:\"$Sreact.fragment\"\n"
                              "2:I[\"123\",[\"static/chunks/a.js\"],\"Link\"]\n"
                              "3:HL[\"/_next/static/css/x.css\",\"style\"]\n"
                              "4:Tc," ) + TextRowValue;
    if ( 1 == part )
        return "0:[\"$\",\"div\",null,{\"className\":\"lib\",\"children\":["
                   "[\"$\",\"$L2\",null,{\"href\":\"/home/library/c7a551c0-0d76-4f56-8d26-596225ced5a4\",\"children\":\"Sounds to die for\"}],"
                   "[\"$\",\"a\",null,{\"href\":\"/home/vault\",\"children\":[\"The\",\" \",[\"$\",\"span\",null,{\"children\":\"Vault\"}]]}],"
                   "[\"$\",\"p\",null,{\"children\":\"$4\"}],"
                   "[\"$\",\"a\",null,{\"href\":\"/x?a=1&b=<2>\",\"children\":\"$$5 off\"}],"
                   "\"$L5\"]}]\n";
    return "5:{\"order\":{\"gamekey\":\"abc\",\"amount_spent\":10.5,\"subproducts\":[{\"machine_name\":\"sound_a\"}]}}\n"
           "this line is no row\n"
           "6:E{\"digest\":\"123\"}\n";
}

CORE::CString
FixturePayload( void )
{
    return FixturePayloadPart( 0 ) + FixturePayloadPart( 1 ) + FixturePayloadPart( 2 );
}

CORE::CString
FixturePage( void )
{
    return "<!DOCTYPE html><html><head><script>(self.__next_f=self.__next_f||[]).push([0])</script></head><body>"
           "<script>self.__next_f.push([1," + ToJsLiteral( FixturePayloadPart( 0 ) ) + "])</script>"
           "<script>self.__next_f.push([2,\"form state\"])</script>"
           "<script>self.__next_f.push([1, " + ToJsLiteral( FixturePayloadPart( 1 ) ) + "])</script>"
           "<script>self.__next_f.push([3,\"QUJD\"])</script>"
           "<script>self.__next_f.push([1," + ToJsLiteral( FixturePayloadPart( 2 ) ) + "])</script>"
           "</body></html>";
}

/** The object of the payload that has an attribute of the name */
const CORE::CDataNode*
FindObjectWith( const WEB::CReactFlightPayload& payload, const char* attribute )
{
    WEB::CReactFlightPayload::TConstNodeVector objects;
    payload.CollectObjects( objects );
    for ( size_t i=0; i<objects.size(); ++i )
        if ( !objects[ i ]->GetAttributeValue( attribute ).IsNULLOrEmpty() )
            return objects[ i ];
    return GUCEF_NULL;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformReactFlightPayloadTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "ReactFlightPayload" );

    GUCEF_TESTFW_TESTCASE( "Test: string literals of a page are decoded: escapes, \\u with surrogate pairs and braces, \\x, single quotes" )
        try
        {
            CORE::CString value;
            UInt32 end = 0;
            const CORE::CString text = "x=\"a\\\"b\\\\c\\/d\\u003cx\\ud83d\\ude00\\x41'\\n\";";
            ASSERT_TRUE( WEB::CReactFlightPayload::DecodeStringLiteral( text, 2, value, end ) );
            ASSERT_TRUE( value == "a\"b\\c/d<x\xF0\x9F\x98\x80" "A'\n" );
            ASSERT_TRUE( text.C_String()[ end ] == ';' );

            ASSERT_TRUE( WEB::CReactFlightPayload::DecodeStringLiteral( "'it\\'s \\u{1F600}'", 0, value, end ) );
            ASSERT_TRUE( value == "it's \xF0\x9F\x98\x80" );
            ASSERT_TRUE( WEB::CReactFlightPayload::DecodeStringLiteral( "\"\"", 0, value, end ) && value.IsNULLOrEmpty() && 2 == end );
            ASSERT_FALSE( WEB::CReactFlightPayload::DecodeStringLiteral( "\"not closed", 0, value, end ) );
            ASSERT_FALSE( WEB::CReactFlightPayload::DecodeStringLiteral( "no quote", 0, value, end ) );
            ASSERT_FALSE( WEB::CReactFlightPayload::DecodeStringLiteral( "\"bad \\u12\"", 0, value, end ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Test: the payload of a page is the type 1 strings joined; the start, form state and binary chunks are skipped" )
        try
        {
            CORE::CString flight;
            UInt32 chunks = 0;
            UInt32 skipped = 0;
            ASSERT_TRUE( WEB::CReactFlightPayload::ExtractFromHtml( FixturePage(), flight, &chunks, &skipped ) );
            ASSERT_TRUE( 3 == chunks );
            ASSERT_TRUE( 2 == skipped );
            ASSERT_TRUE( flight == FixturePayload() );
            ASSERT_FALSE( WEB::CReactFlightPayload::ExtractFromHtml( "<html><body><p>no data</p></body></html>", flight, &chunks, &skipped ) );
            ASSERT_TRUE( 0 == chunks && flight.IsNULLOrEmpty() );
            ASSERT_TRUE( WEB::CReactFlightPayload::LooksLikePayload( "0:[\"$\"]" ) );
            ASSERT_TRUE( WEB::CReactFlightPayload::LooksLikePayload( "1a:I[]" ) );
            ASSERT_FALSE( WEB::CReactFlightPayload::LooksLikePayload( "<!DOCTYPE html>" ) );
            ASSERT_FALSE( WEB::CReactFlightPayload::LooksLikePayload( ":x" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Test: the rows: models, module, hint and error rows, a length prefixed text row with a two byte character and a line end, a line that is no row" )
        try
        {
            ASSERT_TRUE( FlightJsonCodecAvailable() );
            WEB::CReactFlightPayload payload;
            ASSERT_TRUE( payload.Parse( FixturePayload() ) );
            const WEB::TReactFlightRowVector& rows = payload.GetRows();
            ASSERT_TRUE( 7 == rows.size() );
            ASSERT_TRUE( 1 == payload.GetSkippedCount() );
            ASSERT_TRUE( rows[ 0 ].id == "1" && rows[ 0 ].IsModel() );
            ASSERT_TRUE( rows[ 1 ].id == "2" && rows[ 1 ].tag == "I" && rows[ 1 ].hasJson );
            ASSERT_TRUE( rows[ 2 ].id == "3" && rows[ 2 ].tag == "HL" && rows[ 2 ].hasJson );
            ASSERT_TRUE( rows[ 3 ].id == "4" && rows[ 3 ].IsText() && rows[ 3 ].text == TextRowValue && !rows[ 3 ].hasJson );
            ASSERT_TRUE( rows[ 4 ].id == "0" && rows[ 4 ].IsModel() && rows[ 4 ].hasJson );
            ASSERT_TRUE( rows[ 5 ].id == "5" && rows[ 5 ].hasJson );
            ASSERT_TRUE( rows[ 6 ].id == "6" && rows[ 6 ].tag == "E" && rows[ 6 ].hasJson );
            ASSERT_TRUE( payload.GetJsonRowCount() >= 5 );
            ASSERT_TRUE( GUCEF_NULL != payload.FindRow( "5" ) && GUCEF_NULL == payload.FindRow( "7" ) );

            // the page gives the same rows
            WEB::CReactFlightPayload fromPage;
            ASSERT_TRUE( fromPage.ParseHtml( FixturePage() ) );
            ASSERT_TRUE( 7 == fromPage.GetRows().size() );
            ASSERT_TRUE( fromPage.GetRows()[ 3 ].text == TextRowValue );
            WEB::CReactFlightPayload any;
            ASSERT_TRUE( any.ParseAny( "  \n" + FixturePage() ) && 7 == any.GetRows().size() );
            ASSERT_TRUE( any.ParseAny( FixturePayload() ) && 7 == any.GetRows().size() );
            ASSERT_FALSE( any.ParseAny( "<html></html>" ) );
            ASSERT_TRUE( any.GetRows().empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Test: references between rows, escaped dollar strings and special values" )
        try
        {
            ASSERT_TRUE( FlightJsonCodecAvailable() );
            WEB::CReactFlightPayload payload;
            ASSERT_TRUE( payload.Parse( FixturePayload() ) );
            CORE::CString id;
            ASSERT_TRUE( WEB::CReactFlightPayload::IsRowReference( "$L2", id ) && id == "2" );
            ASSERT_TRUE( WEB::CReactFlightPayload::IsRowReference( "$@1A", id ) && id == "1a" );
            ASSERT_TRUE( WEB::CReactFlightPayload::IsRowReference( "$3:props:children", id ) && id == "3" );
            ASSERT_TRUE( WEB::CReactFlightPayload::IsRowReference( "$4", id ) && id == "4" );
            ASSERT_FALSE( WEB::CReactFlightPayload::IsRowReference( "$undefined", id ) );
            ASSERT_FALSE( WEB::CReactFlightPayload::IsRowReference( "$$5", id ) );
            ASSERT_FALSE( WEB::CReactFlightPayload::IsRowReference( "$Sreact.suspense", id ) );
            ASSERT_FALSE( WEB::CReactFlightPayload::IsRowReference( "$D2024-01-01T00:00:00.000Z", id ) );
            ASSERT_FALSE( WEB::CReactFlightPayload::IsRowReference( "plain", id ) );
            const WEB::CReactFlightRow* row = payload.ResolveReference( "$L2" );
            ASSERT_TRUE( GUCEF_NULL != row && row->tag == "I" );
            ASSERT_TRUE( GUCEF_NULL == payload.ResolveReference( "$L9" ) );
            ASSERT_TRUE( payload.ValueAsText( "$$5 off" ) == "$5 off" );
            ASSERT_TRUE( payload.ValueAsText( "$4" ) == TextRowValue );
            ASSERT_TRUE( payload.ValueAsText( "$undefined" ).IsNULLOrEmpty() );
            ASSERT_TRUE( payload.ValueAsText( "plain text" ) == "plain text" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Test: the objects of the models and the links of the element trees with their visible text" )
        try
        {
            ASSERT_TRUE( FlightJsonCodecAvailable() );
            WEB::CReactFlightPayload payload;
            ASSERT_TRUE( payload.ParseHtml( FixturePage() ) );

            // an object of a data row, found wherever it is
            const CORE::CDataNode* order = FindObjectWith( payload, "gamekey" );
            ASSERT_TRUE( GUCEF_NULL != order );
            ASSERT_TRUE( order->GetAttributeValue( "gamekey" ).AsString() == "abc" );
            ASSERT_FALSE( order->GetAttributeValue( "amount_spent" ).IsNULLOrEmpty() );
            ASSERT_TRUE( GUCEF_NULL != order->FindChild( "subproducts" ) );
            ASSERT_TRUE( GUCEF_NULL != FindObjectWith( payload, "machine_name" ) );

            WEB::CReactFlightPayload::TStringPairVector links;
            payload.CollectLinks( links );
            ASSERT_TRUE( 3 == links.size() );
            ASSERT_TRUE( links[ 0 ].first == "/home/library/c7a551c0-0d76-4f56-8d26-596225ced5a4" && links[ 0 ].second == "Sounds to die for" );
            ASSERT_TRUE( links[ 1 ].first == "/home/vault" && links[ 1 ].second == "The Vault" );
            ASSERT_TRUE( links[ 2 ].first == "/x?a=1&b=<2>" && links[ 2 ].second == "$5 off" );

            // the text of an element whose child is a text row
            const CORE::CDataNode* lib = FindObjectWith( payload, "className" );
            ASSERT_TRUE( GUCEF_NULL != lib );
            const CORE::CString all = payload.GetElementText( *lib );
            ASSERT_TRUE( all.HasSubstr( "Sounds to die for", true ) >= 0 );
            ASSERT_TRUE( all.HasSubstr( "world", true ) >= 0 );
            ASSERT_TRUE( all.HasSubstr( "$5 off", true ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
