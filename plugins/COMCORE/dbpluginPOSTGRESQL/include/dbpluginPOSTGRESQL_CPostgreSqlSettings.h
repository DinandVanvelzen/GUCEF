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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H
#include "gucefCOMCORE_CDatabaseClientSettings.h"
#define GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H
#endif /* GUCEF_COMCORE_CDATABASECLIENTSETTINGS_H ? */

#ifndef DBPLUGIN_POSTGRESQL_MACROS_H
#include "dbpluginPOSTGRESQL_macros.h"
#define DBPLUGIN_POSTGRESQL_MACROS_H
#endif /* DBPLUGIN_POSTGRESQL_MACROS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How the postgresql backend connects, read from CDatabaseClientSettings: settings.database is the name of the
 *  PostgreSQL database, settings.readOnly makes every transaction of the session read only, and the backend options
 *  (all optional) are:
 *
 *      connectionString     a libpq connection string or URI ("host=... user=...", "postgresql://..."), the base of the
 *                           connection; the other options and settings.database override what it says
 *      host, port           the server, default the libpq defaults (PGHOST, PGPORT, the local socket, 5432)
 *      user                 the role, default the libpq default (PGUSER, the name of the process user)
 *      password             a secret: never logged nor described; prefer passwordFile or the environment (PGPASSWORD)
 *                           for settings that are stored, since SaveConfig() writes the backend options as given
 *      passwordFile         a libpq password file (passfile)
 *      sslMode              disable, allow, prefer, require, verify-ca, verify-full (libpq default prefer)
 *      applicationName      shown in pg_stat_activity, default "gucef"
 *      connectTimeout       seconds, default the libpq default (no limit)
 *      schema               the PostgreSQL schema that holds the tables, default "public"
 *      createSchema         "true": the schema is made at open when it does not exist (read-write opens only)
 *      maxRowsPerStatement  rows of one INSERT statement of a write, 1 .. 10000, default 1000
 */
class DBPLUGIN_POSTGRESQL_PUBLIC_CPP CPostgreSqlSettings
{
    public:

    static const CORE::CString DefaultSchema;
    static const CORE::CString DefaultApplicationName;
    static const UInt32 DefaultMaxRowsPerStatement = 1000;
    static const UInt32 MaxRowsPerStatementLimit = 10000;

    CORE::CString connectionString;
    CORE::CString host;
    UInt16 port;                        /**< 0: the libpq default */
    CORE::CString database;
    CORE::CString user;
    CORE::CString password;
    CORE::CString passwordFile;
    CORE::CString sslMode;
    CORE::CString applicationName;
    UInt32 connectTimeoutInSecs;        /**< 0: the libpq default */
    CORE::CString schema;
    bool createSchema;
    bool readOnly;
    UInt32 maxRowsPerStatement;

    CPostgreSqlSettings( void );

    /** Reads the database settings; false with the reason for a value that cannot be used. The settings are left as they were then */
    bool LoadFrom( const COMCORE::CDatabaseClientSettings& settings, CORE::CString& error );

    /**
     *  The keywords and values of PQconnectdbParams() with expand_dbname set: the connection string first (as "dbname",
     *  which libpq expands), then every option that is set. The values hold the password: never log them.
     */
    void GetConnectionParameters( CORE::CString::StringVector& keywords, CORE::CString::StringVector& values ) const;

    /** The settings for the log: server, database, user, schema and modes; never the password or the connection string */
    CORE::CString Describe( void ) const;

    /** True for the ssl modes of libpq */
    static bool IsValidSslMode( const CORE::CString& mode );

    /**
     *  The keywords and values of a libpq connection string or URI, through PQconninfoParse(); false with the reason
     *  (without the text, which can hold a password) when libpq cannot read it. Values can hold a password.
     */
    static bool ParseConnectionString( const CORE::CString& text, CORE::CValueList& keywordValues, CORE::CString& error );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H ? */
