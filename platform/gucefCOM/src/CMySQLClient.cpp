/*
 *  gucefCOM: GUCEF module providing communication implementations 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include <stdlib.h>
#include <stdio.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef CMYSQLCLIENTSOCKETINTERFACE_H
#include "CMySQLClientInterface.h"      /* interface class for this client */
#define CMYSQLCLIENTSOCKETINTERFACE_H
#endif /* CMYSQLCLIENTSOCKETINTERFACE_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CLOGMANAGER_H
#include "CLogManager.h"
#define GUCEF_CORE_CLOGMANAGER_H
#endif /* GUCEF_CORE_CLOGMANAGER_H ? */

#include "CMySQLClient.h"               /* definition of this class */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define LONG_PASSWORD     1
#define FOUND_ROWS	  2
#define LONG_FLAG         4
#define CONNECT_WITH_DB   8
#define NO_SCHEMA	 16
#define COMPRESS	 32
#define ODBC             64
#define LOCAL_FILES     128
#define IGNORE_SPACE    256
#define CHANGE_USER     512
#define INTERACTIVE    1024
#define SSL            2048
#define IGNORE_SIGPIPE 4096
#define TRANSACTIONS   8192

/*--------------------------------------------------------------------------*/

/**
 *	Following are a number for charset defines which define the 'language'
 *	in which we talk to the server.
 */
#define big5        1
#define czech       2
#define dec8        3
#define dos         4
#define german1     5
#define hp8         6
#define koi8_ru     7
#define latin1      8
#define latin2      9
#define swe7       10
#define usa7       11
#define ujis       12
#define sjis       13
#define sp1251     14
#define danish     15
#define hebrew     16
#define win1251    17
#define tis620     18
#define euc_kr     19
#define estonia    20
#define hungarian  21
#define koi8_ukr   22
#define win1251ukr 23
#define gb2312     24
#define greek      25
#define win1250    26
#define croat      27
#define gbk        28
#define cp1257     29
#define latin5     30

/*--------------------------------------------------------------------------*/

#define IN_TRANS   1
#define AUTOCOMMIT 2

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *	Default constructor, we just init some vars
 */
CMySQLClient::CMySQLClient( bool blocking )
        : socket( blocking )
{GUCEF_TRACE;
        #ifdef DEBUG_MODE
        CORE::tsprintf( "CMySQLClient::Constructor()\n" );
        #endif /* DEBUG_MODE */

        socket.Subscribe( &AsObserver() );
        mysql_connection_state = MYSQL_DISCONNECTED;
        query_busy = false;
        iface = GUCEF_NULL;
}

/*--------------------------------------------------------------------------*/

/**
 *	Destructor, cleanup any allocated storage space
 */
CMySQLClient::~CMySQLClient( void )
{GUCEF_TRACE;
        #ifdef DEBUG_MODE
        CORE::tsprintf( "CMySQLClient::Destructor()\n" );
        #endif /* DEBUG_MODE */

        lock.Lock();

        Disconnect();
}

/*--------------------------------------------------------------------------*/

/**
 *	Attempt to connect to a MySQL server which is specified by ip_address
 *	and port using the username and password provided to connected to the
 *	given database on the given TCP client_socket.
 */
bool
CMySQLClient::ConnectTo( const GUCEF::CORE::CString& address  ,
                         UInt16 port                          ,
                         const GUCEF::CORE::CString& username ,
                         const GUCEF::CORE::CString& password ,
                         const GUCEF::CORE::CString& database )
{GUCEF_TRACE;
        #ifdef DEBUG_MODE
        CORE::tsprintf( "CMySQLClient::Connect_To()\n" );
        #endif /* DEBUG_MODE */

        if ( query_busy ) return false;

        lock.Lock();

        mysql_connection_state = MYSQL_CONNECTING;

        if( !socket.ConnectTo( address, port ) )
        {
                mysql_error = "Could not connect to ";
                mysql_error += address;
                mysql_error += ":";
                mysql_error += CORE::ToString( port );

                mysql_error_number = 0;

                mysql_connection_state = MYSQL_CONNECTION_FAILED;
                lock.Unlock();
                return false;
        }

        mysql_server_ip_address = address;
        mysql_server_port = port;
        mysql_username = username;
        mysql_password = password;
        mysql_database = database;

        lock.Unlock();
        return true;
}

/*--------------------------------------------------------------------------*/

/**
 *	Propperly disconnect from the MySQL server
 */
void
CMySQLClient::Disconnect( void )
{GUCEF_TRACE;
        #ifdef DEBUG_MODE
        CORE::tsprintf( "CMySQLClient::Disconnect()\n" );
        #endif /* DEBUG_MODE */

        lock.Lock();
        if ( socket.IsActive() )
        {
                char data[6];

                sprintf(data, "%c%c%c%c%c", 0x01, 0x00, 0x00, 0x00, 0x01);
                socket.Send( data, 5 );
                socket.Close();

                mysql_connection_state = MYSQL_DISCONNECTED;
        }
        lock.Unlock();
}

/*--------------------------------------------------------------------------*/

void
CMySQLClient::OnRead( COMCORE::CTCPClientSocket &socket ,
                      const char *data                  ,
                      UInt32 length                     ,
                      UInt32 &keepbytes                 )
{GUCEF_TRACE;
        #ifdef DEBUG_MODE
        CORE::tsprintf( "CMySQLClient::OnRead()\n" );
        #endif /* DEBUG_MODE */

        if(mysql_connection_state == MYSQL_CONNECTING) {
                if(data[4] == 0x0a) {
                        Login(data, length);
                }
                else if(data[4] == 0x00) {

                        #ifdef DEBUG_MODE
                        CORE::tsprintf( "CMySQLClient: mysql_connection_state==MYSQL_CONNECTED\n" );
                        #endif /* DEBUG_MODE */

                        mysql_connection_state = MYSQL_CONNECTED;
                }
                else if(data[4] == (char)0xff) {

                        #ifdef DEBUG_MODE
                        CORE::tsprintf( "CMySQLClient: mysql_connection_state==MYSQL_CONNECTION_FAILED\n" );
                        #endif /* DEBUG_MODE */

                        mysql_connection_state = MYSQL_CONNECTION_FAILED;

                        mysql_error_number = (UInt16)(data[5]) | (UInt16)(data[6] << 8);

                        mysql_error.Set( data+7, length-6 );
                }
        }
        else {
                if(data[4] == (char)0xff) {
                        UInt32 i;
                        char *temp_char;

                        #ifdef DEBUG_MODE
                        CORE::tsprintf( "CMySQLClient: state==CDB_ERROR\n" );
                        #endif /* DEBUG_MODE */

                        query->state = CDB_ERROR;

                        query->error = (UInt16)(data[5]) | (UInt16)(data[6] << 8);

                        query->description = GUCEF_NEW char[length - 6];
                        temp_char = query->description;

                        for(i = 7; i < length; i++) {
                                temp_char[i - 7] = data[i];
                        }

                        temp_char[length] = '\0';
                        if ( iface ) iface->OnQueryComplete( *this, *query );
                        query_busy = false;
                }

                else if(data[4] == (char)0x00) {
                        query->effected_rows = data[5];
                        query->insert_id = data[6];
                        query->status = data[7] | data[8] << 8;

                        #ifdef DEBUG_MODE
                        CORE::tsprintf( "CMySQLClient: state==CDB_CHANGED\n" );
                        #endif /* DEBUG_MODE */

                        query->state = CDB_CHANGED;
                        if ( iface ) iface->OnQueryComplete( *this, *query );
                        query_busy = false;
                }

                else {
                        UInt32 body_length, i = 0, i2, column, row;
                        UInt8 temp_len;
                        char *temp_char;

                        i += 4;

                        query->Columns(data[i]);

                        i++;

                        column = 0;
                        while(data[i + 4] != (char)0xfe) {
                                #ifdef DEBUG_MODE
                                CORE::tsprintf("Column: %d\n", column);
                                #endif /* DEBUG_MODE */

                                body_length = data[i] | data[i + 1] >> 8 | data[i + 2] >> 16;

                                i += 4;

                                temp_len = data[i]; // table name length
                                i++;

                                query->Column(column)->table = GUCEF_NEW char[temp_len + 1];
                                temp_char = query->Column(column)->table;

                                for(i2 = 0; i2 < temp_len; i2++) {
                                        temp_char[i2] = data[i2 + i];
                                }

                                i += temp_len;

                                temp_char[temp_len] = '\0';

                                temp_len = data[i]; // column name length
                                i++;

                                query->Column(column)->value = GUCEF_NEW char[temp_len + 1];
                                temp_char = query->Column(column)->value;

                                for(i2 = 0; i2 < temp_len; i2++) {
                                        temp_char[i2] = data[i2 + i];
                                }

                                i += temp_len;

                                temp_char[temp_len] = '\0';

                                i += 1 + data[i]; // length, we skip this for the moment
                                i += 1 + data[i]; // type, we skip this for the moment
                                i += 1 + data[i]; // flags + decimals, we skip this for the moment

                                column++;
                        }

                        i += 5;
                        i2 = i;

                        row = 0;
                        while(data[i2 + 4] != (char)0xfe) {
                                printf("Row: %d\n", row);

                                i2 += 4 + (data[i2] | data[i2 + 1] >> 8 | data[i2 + 2] >> 16);
                                row++;
                        }

                        query->Rows(row);

                        row = 0;
                        while(data[i + 4] != (char)0xfe) {
                                body_length = i + 4 + data[i] | data[i + 1] >> 8 | data[i + 2] >> 16;

                                i += 4;

                                column = 0;
                                while(i < body_length) {
                                        temp_len = data[i]; // field length
                                        if(temp_len == (UInt8)0xfb) {
                                                i++;
                                                query->Field(row, column)->value = GUCEF_NULL;
                                        }
                                        else {
                                                i++;

                                                query->Field(row, column)->value = GUCEF_NEW char[temp_len + 1];
                                                temp_char = query->Field(row, column)->value;

                                                for(i2 = 0; i2 < temp_len; i2++) {
                                                        temp_char[i2] = data[i2 + i];
                                                }
                                                temp_char[temp_len] = '\0';
                                                i += temp_len;
                                        }
                                        column++;
                                }
                                row++;
                        }
                        query->state = CDB_SELECT;
                        if ( iface ) iface->OnQueryComplete( *this, *query );
                        query_busy = false;
                }
        }
}

/*--------------------------------------------------------------------------*/

bool
CMySQLClient::Login( const char *data, UInt32 length )
{GUCEF_TRACE;
        #ifdef DEBUG_MODE
        CORE::tsprintf( "CMySQLClient::Login()\n" );
        #endif /* DEBUG_MODE */

        UInt32 i, body_length;
        UInt8 packet = data[3];

        mysql_protocol_version = data[ 4 ];
        mysql_server_version = data+5;
        i = 6+mysql_server_version.Length();
        mysql_thread_id = data[++i] | data[++i] << 8 | data[++i] << 16 | data[++i] << 24;
        mysql_ssl_salt = data+i;
        i+= mysql_ssl_salt.Length();

        mysql_server_capabilities = data[++i] | data[++i] << 8;
        mysql_client_capabilities = LONG_PASSWORD | LONG_FLAG | TRANSACTIONS | INTERACTIVE;

        mysql_charset = data[++i];
        mysql_status = data[++i] | data[++i] << 8;
/*
        #ifdef DEBUG_MODE
        CORE::tsprintf("Logging on to MySQL Server\n");
        CORE::tsprintf("Username: %s, Password: %s\n\n", mysql_username, mysql_password);

        CORE::tsprintf("Protocol: %d\n", mysql_protocol_version);
        CORE::tsprintf("Version: %s\n", mysql_server_version);
        CORE::tsprintf("Thread: %d\n", mysql_thread_id);
        CORE::tsprintf("SSL Salt: %s\n\n", mysql_ssl_salt);

        CORE::tsprintf("Server Capabilities:\n");
        if(mysql_server_capabilities & LONG_PASSWORD)   CORE::tsprintf("  Long Password\n");
        if(mysql_server_capabilities & FOUND_ROWS)      CORE::tsprintf("  Found Rows\n");
        if(mysql_server_capabilities & LONG_FLAG)       CORE::tsprintf("  Long Flag\n");
        if(mysql_server_capabilities & CONNECT_WITH_DB) CORE::tsprintf("  Connect with Database\n");
        if(mysql_server_capabilities & NO_SCHEMA)       CORE::tsprintf("  No Schema\n");
        if(mysql_server_capabilities & COMPRESS)        CORE::tsprintf("  Compression\n");
        if(mysql_server_capabilities & ODBC)            CORE::tsprintf("  ODBC\n");
        if(mysql_server_capabilities & LOCAL_FILES)     CORE::tsprintf("  Local Files\n");
        if(mysql_server_capabilities & IGNORE_SPACE)    CORE::tsprintf("  Ignore Space\n");
        if(mysql_server_capabilities & CHANGE_USER)     CORE::tsprintf("  Change User\n");
        if(mysql_server_capabilities & INTERACTIVE)     CORE::tsprintf("  Interactive\n");
        if(mysql_server_capabilities & SSL)             CORE::tsprintf("  SSL\n");
        if(mysql_server_capabilities & IGNORE_SIGPIPE)  CORE::tsprintf("  Ignore Sigpipe\n");
        if(mysql_server_capabilities & TRANSACTIONS)    CORE::tsprintf("  Transactions\n");
        CORE::tsprintf("\n");

        CORE::tsprintf("Charset: ");
        switch ( mysql_charset )
        {
                case big5 : { CORE::tsprintf("big5\n"); break; }
                case czech : {  CORE::tsprintf("czech\n"); break; }
                case dec8 : { CORE::tsprintf("dec8\n"); break; }
                case dos : { CORE::tsprintf("dos\n"); break; }
                case german1 : { CORE::tsprintf("german1\n"); break; }
                case hp8 : { CORE::tsprintf("hp8\n"); break; }
                case koi8_ru : { CORE::tsprintf("koi8_ru\n"); break; }
                case latin1 : { CORE::tsprintf("latin1\n"); break; }
                case latin2 : { CORE::tsprintf("latin2\n"); break; }
                case swe7 : { CORE::tsprintf("swe7\n"); break; }
                case usa7 : { CORE::tsprintf("usa7\n"); break; }
                case ujis : { CORE::tsprintf("ujis\n"); break; }
                case sjis : { CORE::tsprintf("sjis\n"); break; }
                case sp1251 : { CORE::tsprintf("sp1251\n"); break; }
                case danish : { CORE::tsprintf("danish\n"); break; }
                case hebrew : { CORE::tsprintf("hebrew\n"); break; }
                case win1251 : { CORE::tsprintf("win1251\n"); break; }
                case tis620 : { CORE::tsprintf("tis620\n"); break; }
                case euc_kr : { CORE::tsprintf("euc_kr\n"); break; }
                case estonia : { CORE::tsprintf("estonia\n"); break; }
                case hungarian : { CORE::tsprintf("hungarian\n"); break; }
                case koi8_ukr : { CORE::tsprintf("koi8_ukr\n"); break; }
                case win1251ukr : { CORE::tsprintf("win1251ukr\n"); break; }
                case gb2312 : { CORE::tsprintf("gb2312\n"); break; }
                case greek : { CORE::tsprintf("greek\n"); break; }
                case win1250 : { CORE::tsprintf("win1250\n"); break; }
                case croat : { CORE::tsprintf("croat\n"); break; }
                case gbk : { CORE::tsprintf("gbk\n"); break; }
                case cp1257 : { CORE::tsprintf("cp1257\n"); break; }
                case latin5 : { CORE::tsprintf("latin5\n"); break; }
                default : { CORE::tsprintf("UNKOWN\n"); break; }
        }
        CORE::tsprintf("\n");

        CORE::tsprintf("Status: ");
        if(mysql_status == IN_TRANS)   CORE::tsprintf("IN_TRANS\n"); else
        if(mysql_status == AUTOCOMMIT) CORE::tsprintf("AUTOCOMMIT\n"); else
                                       CORE::tsprintf("UNKNOWN\n");
        CORE::tsprintf("\n");
        #endif /* DEBUG_MODE */

        char *reply(GUCEF_NULL);
        if( !mysql_database.Length() )
        {
                length = 10 + mysql_username.Length() + mysql_password.Length()+1;
                reply = GUCEF_NEW char[length + 1];

                body_length = length - 4;

                sprintf(reply, "%c%c%c%c%c%c%c%c%c%s%c%s", body_length, body_length >> 8, body_length >> 16, packet + 1, mysql_client_capabilities, mysql_client_capabilities >> 8, 0x00, 0x00, 0x00, mysql_username.C_String(), 0x00, mysql_password.C_String());
        }
        else {
                length = 11 + mysql_username.Length() + mysql_password.Length() + mysql_database.Length()+1;
                reply = GUCEF_NEW char[length + 1];

                body_length = length - 4;

                sprintf(reply, "%c%c%c%c%c%c%c%c%c%s%c%s%c%s", body_length, body_length >> 8, body_length >> 16, packet + 1, mysql_client_capabilities, mysql_client_capabilities >> 8, 0x00, 0x00, 0x00, mysql_username.C_String(), 0x00, mysql_password.C_String(), 0x00, mysql_database.C_String());
        }

        socket.Send(reply, length);
        GUCEF_DELETE []reply;

        return true;
}

/*--------------------------------------------------------------------------*/

bool
CMySQLClient::Query( CDBQuery &cdbquery                   ,
                     const GUCEF::CORE::CString& sqlquery )
{GUCEF_TRACE;
        #ifdef DEBUG_MODE
        CORE::tsprintf( "CMySQLClient::Query()\n" );
        #endif /* DEBUG_MODE */

        lock.Lock();
        if ( query_busy )
        {
                lock.Unlock();
                return false;
        }
        query_busy = true;
        UInt32 body_length = sqlquery.Length()+1;
        UInt32 length = body_length + 4;

        char *data = GUCEF_NEW char[ 1 + length ];
        sprintf( data, "%c%c%c%c%c%s", body_length, body_length >> 8, body_length >> 16, 0x00, 0x03, sqlquery.C_String() );

        query = &cdbquery;
        query->state = CDB_GETTING_DATA;

        bool retval = socket.Send( data, length );
        lock.Unlock();
        return retval;
}

/*--------------------------------------------------------------------------*/

bool
CMySQLClient::QueryAndWait( CDBQuery &cdbquery   ,
                            const GUCEF::CORE::CString& sqlquery ,
                            Int32 timeout        )
{GUCEF_TRACE;
        #ifdef DEBUG_MODE
        CORE::tsprintf( "CMySQLClient::Query_And_Wait()\n" );
        #endif /* DEBUG_MODE */

        lock.Lock();

        /*
         *      First we must wait for the client to finish any existing query
         */
        UInt32 reqt = CORE::GUCEFGetTickCount();
        while ( query_busy && timeout >= 0 )
        {
                MT::ThreadDelay( 10 );
                timeout -= 10;
        }
        if ( timeout < 0 )
        {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CMySQLClient: Timeout waiting for an existing query to finish" );
                lock.Unlock();
                return false;
        }
        query_busy = true;

        /*
         *      Now we formulate and send the GUCEF_NEW query and wait for that to
         *      finish.
         */
        UInt32 body_length = sqlquery.Length()+1;
        UInt32 length = body_length + 4;

        char *data = GUCEF_NEW char[ 1 + length ];
        sprintf( data, "%c%c%c%c%c%s", body_length, body_length >> 8, body_length >> 16, 0x00, 0x03, sqlquery.C_String() );

        query = &cdbquery;
        query->state = CDB_GETTING_DATA;

        #ifndef GUCEF_COM_DEBUG_BUILD
        bool retval = socket.Send( data    ,
                                   length  ,
                                   timeout );
        lock.Unlock();
        return retval;
        #else
        if ( socket.Send_Data_And_Wait( data    ,
                                        length  ,
                                        timeout ) )
        {
                tsprintf( "CMySQLClient: Data recieved before timeout, query error state: %d\n", cdbquery.Error() );
                lock.Unlock_Mutex();
                return true;
        }
        tsprintf( "CMySQLClient: Timeout waiting for data after submitting query\n" );
        lock.Unlock();
        return false;
        #endif /* GUCEF_COM_DEBUG_BUILD ? */
}

/*--------------------------------------------------------------------------*/

/**
 *      Event handler called when we are about to connect to a server
 */
void
CMySQLClient::OnConnecting( COMCORE::CTCPClientSocket &socket )
{GUCEF_TRACE;
        if ( iface ) iface->OnConnecting( *this );
}

/*--------------------------------------------------------------------------*/

/**
 *      Event handler called when we have successfully connected to
 *      a server socket.
 */
void
CMySQLClient::OnConnected( COMCORE::CTCPClientSocket &socket )
{GUCEF_TRACE;
        if ( iface ) iface->OnConnected( *this );
}

/*--------------------------------------------------------------------------*/

/**
 *      Event handler called when have lost the connection.
 *      The socket is now closed.
 */
void
CMySQLClient::OnDisconnect( COMCORE::CTCPClientSocket &socket )
{GUCEF_TRACE;
        query_busy = false;
        if ( iface ) iface->OnDisconnect( *this );
}

/*--------------------------------------------------------------------------*/

/**
 *      Event handler called when a socket error occured.
 */
void
CMySQLClient::OnError( COMCORE::CTCPClientSocket &socket )
{GUCEF_TRACE;
        query_busy = false;
//        if ( iface ) iface->OnSocketError( *this, errorcode );
}

/*--------------------------------------------------------------------------*/

/**
 *      Sets the GUCEF_NEW event handling interface.
 *      Note that the interface can only be set if there is no active
 *      query.
 */
bool
CMySQLClient::SetInterface( CMySQLClientInterface *new_iface )
{GUCEF_TRACE;
        lock.Lock();
        iface = new_iface;
        lock.Unlock();
        return true;
}

/*--------------------------------------------------------------------------*/

CMySQLClientInterface*
CMySQLClient::GetInterface( void ) const
{GUCEF_TRACE;
        return iface;
}

/*--------------------------------------------------------------------------*/

bool
CMySQLClient::Query_Busy( void ) const
{GUCEF_TRACE;
        return query_busy;
}

/*--------------------------------------------------------------------------*/

CORE::CString
CMySQLClient::GetError( void ) const
{GUCEF_TRACE;
        return mysql_error;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
