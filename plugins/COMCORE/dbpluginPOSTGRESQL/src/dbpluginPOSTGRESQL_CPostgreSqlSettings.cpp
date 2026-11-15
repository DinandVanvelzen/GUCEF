/*
 *  dbpluginPOSTGRESQL: gucefCOMCORE database backend for PostgreSQL
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

#include "libpq-fe.h"

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "dbpluginPOSTGRESQL_CPostgreSqlSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace POSTGRESQL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CPostgreSqlSettings::DefaultSchema = "public";
const CORE::CString CPostgreSqlSettings::DefaultApplicationName = "gucef";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The first value of the key, empty when it is not there */
CORE::CString
OptionText( const CORE::CValueList& options, const char* key )
{GUCEF_TRACE;

    return options.GetValueAlways( CORE::CString( key ) ).AsString( CORE::CString::Empty, true );
}

/*-------------------------------------------------------------------------*/

/** A whole number of the text within [ minimum, maximum ]; false for other text */
bool
ParseBoundedNumber( const CORE::CString& text, UInt32 minimum, UInt32 maximum, UInt32& value )
{GUCEF_TRACE;

    const CORE::CString trimmed = text.Trim( true ).Trim( false );
    if ( trimmed.IsNULLOrEmpty() || trimmed.Length() > 10 )
        return false;
    UInt64 number = 0;
    const char* c = trimmed.C_String();
    for ( UInt32 i=0; i<trimmed.Length(); ++i )
    {
        if ( c[ i ] < '0' || c[ i ] > '9' )
            return false;
        number = number * 10 + (UInt64) ( c[ i ] - '0' );
    }
    if ( number < minimum || number > maximum )
        return false;
    value = (UInt32) number;
    return true;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPostgreSqlSettings::CPostgreSqlSettings( void )
    : connectionString()
    , host()
    , port( 0 )
    , database()
    , user()
    , password()
    , passwordFile()
    , sslMode()
    , applicationName( DefaultApplicationName )
    , connectTimeoutInSecs( 0 )
    , schema( DefaultSchema )
    , createSchema( false )
    , readOnly( false )
    , maxRowsPerStatement( DefaultMaxRowsPerStatement )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlSettings::IsValidSslMode( const CORE::CString& mode )
{GUCEF_TRACE;

    return mode == "disable" || mode == "allow" || mode == "prefer" || mode == "require" || mode == "verify-ca" || mode == "verify-full";
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlSettings::LoadFrom( const COMCORE::CDatabaseClientSettings& settings, CORE::CString& error )
{GUCEF_TRACE;

    const CORE::CValueList& options = settings.backendOptions;
    CPostgreSqlSettings loaded;
    loaded.database = settings.database;
    loaded.readOnly = settings.readOnly;
    loaded.connectionString = OptionText( options, "connectionString" );
    loaded.host = OptionText( options, "host" );
    loaded.user = OptionText( options, "user" );
    loaded.password = OptionText( options, "password" );
    loaded.passwordFile = OptionText( options, "passwordFile" );
    loaded.sslMode = OptionText( options, "sslMode" );

    if ( loaded.database.IsNULLOrEmpty() )
    {
        error = "The settings name no database";
        return false;
    }
    if ( loaded.database.HasChar( '\0' ) >= 0 )
    {
        error = "The database name holds a NUL character";
        return false;
    }

    const CORE::CString portText = OptionText( options, "port" );
    if ( !portText.IsNULLOrEmpty() )
    {
        UInt32 portValue = 0;
        if ( !ParseBoundedNumber( portText, 1, 65535, portValue ) )
        {
            error = "The port \"" + portText + "\" is not a number from 1 to 65535";
            return false;
        }
        loaded.port = (UInt16) portValue;
    }

    if ( !loaded.sslMode.IsNULLOrEmpty() && !IsValidSslMode( loaded.sslMode ) )
    {
        error = "The ssl mode \"" + loaded.sslMode + "\" is not one of disable, allow, prefer, require, verify-ca, verify-full";
        return false;
    }

    const CORE::CString applicationNameText = OptionText( options, "applicationName" );
    if ( !applicationNameText.IsNULLOrEmpty() )
        loaded.applicationName = applicationNameText;

    const CORE::CString timeoutText = OptionText( options, "connectTimeout" );
    if ( !timeoutText.IsNULLOrEmpty() && !ParseBoundedNumber( timeoutText, 0, 86400, loaded.connectTimeoutInSecs ) )
    {
        error = "The connect timeout \"" + timeoutText + "\" is not a number of seconds from 0 to 86400";
        return false;
    }

    const CORE::CString schemaText = OptionText( options, "schema" );
    if ( !schemaText.IsNULLOrEmpty() )
        loaded.schema = schemaText;
    if ( CORE::ToUtf8String( loaded.schema ).ByteSize() - 1 > 63 )
    {
        error = "The schema name \"" + loaded.schema + "\" is longer than the 63 bytes of a PostgreSQL name";
        return false;
    }

    const CORE::CString createSchemaText = OptionText( options, "createSchema" );
    loaded.createSchema = CORE::StringToBool( createSchemaText, false );

    const CORE::CString rowsText = OptionText( options, "maxRowsPerStatement" );
    if ( !rowsText.IsNULLOrEmpty() && !ParseBoundedNumber( rowsText, 1, MaxRowsPerStatementLimit, loaded.maxRowsPerStatement ) )
    {
        error = "maxRowsPerStatement \"" + rowsText + "\" is not a number from 1 to " + CORE::ToString( MaxRowsPerStatementLimit );
        return false;
    }

    if ( !loaded.connectionString.IsNULLOrEmpty() )
    {
        CORE::CValueList parsed;
        if ( !ParseConnectionString( loaded.connectionString, parsed, error ) )
            return false;
    }

    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPostgreSqlSettings::GetConnectionParameters( CORE::CString::StringVector& keywords, CORE::CString::StringVector& values ) const
{GUCEF_TRACE;

    keywords.clear();
    values.clear();
    if ( !connectionString.IsNULLOrEmpty() )
    {
        // the first "dbname" is expanded by libpq; what follows overrides it
        keywords.push_back( "dbname" );
        values.push_back( connectionString );
    }
    if ( !host.IsNULLOrEmpty() )
    {
        keywords.push_back( "host" );
        values.push_back( host );
    }
    if ( 0 != port )
    {
        keywords.push_back( "port" );
        values.push_back( CORE::ToString( (UInt32) port ) );
    }
    keywords.push_back( "dbname" );
    values.push_back( database );
    if ( !user.IsNULLOrEmpty() )
    {
        keywords.push_back( "user" );
        values.push_back( user );
    }
    if ( !password.IsNULLOrEmpty() )
    {
        keywords.push_back( "password" );
        values.push_back( password );
    }
    if ( !passwordFile.IsNULLOrEmpty() )
    {
        keywords.push_back( "passfile" );
        values.push_back( passwordFile );
    }
    if ( !sslMode.IsNULLOrEmpty() )
    {
        keywords.push_back( "sslmode" );
        values.push_back( sslMode );
    }
    if ( !applicationName.IsNULLOrEmpty() )
    {
        keywords.push_back( "application_name" );
        values.push_back( applicationName );
    }
    if ( 0 != connectTimeoutInSecs )
    {
        keywords.push_back( "connect_timeout" );
        values.push_back( CORE::ToString( connectTimeoutInSecs ) );
    }
    keywords.push_back( "client_encoding" );
    values.push_back( "UTF8" );
    if ( readOnly )
    {
        keywords.push_back( "options" );
        values.push_back( "-c default_transaction_read_only=on" );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlSettings::Describe( void ) const
{GUCEF_TRACE;

    CORE::CString text = "database \"" + database + "\"";
    if ( !host.IsNULLOrEmpty() )
        text += " on " + host;
    if ( 0 != port )
        text += ":" + CORE::ToString( (UInt32) port );
    if ( !user.IsNULLOrEmpty() )
        text += " as " + user;
    if ( !connectionString.IsNULLOrEmpty() )
        text += ", with a connection string";
    text += ", schema \"" + schema + "\"";
    if ( !sslMode.IsNULLOrEmpty() )
        text += ", sslmode " + sslMode;
    if ( readOnly )
        text += ", read only";
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlSettings::ParseConnectionString( const CORE::CString& text, CORE::CValueList& keywordValues, CORE::CString& error )
{GUCEF_TRACE;

    keywordValues.Clear();
    char* parseError = GUCEF_NULL;
    const CORE::CUtf8String utf8 = CORE::ToUtf8String( text );
    PQconninfoOption* options = PQconninfoParse( utf8.C_String(), &parseError );
    if ( GUCEF_NULL == options )
    {
        // the message of libpq names the faulty part without the values
        error = "The connection string cannot be read: " + CORE::CString( GUCEF_NULL != parseError ? parseError : "out of memory" );
        if ( GUCEF_NULL != parseError )
            PQfreemem( parseError );
        return false;
    }
    for ( PQconninfoOption* option = options; GUCEF_NULL != option->keyword; ++option )
    {
        if ( GUCEF_NULL != option->val )
            keywordValues.Set( option->keyword, CORE::CVariant( option->val ) );
    }
    PQconninfoFree( options );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
