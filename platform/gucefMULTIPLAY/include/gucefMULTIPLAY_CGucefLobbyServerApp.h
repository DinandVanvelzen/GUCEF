/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYSERVERAPP_H
#define GUCEF_MULTIPLAY_CGUCEFLOBBYSERVERAPP_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H
#include "gucefMULTIPLAY_CGucefLobbyServer.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYSERVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The standalone lobby server (apps/gucefLOBBYSERVER) without its main: reads the config file, runs a
 *  CGucefLobbyServer, stops it. A test can run the same in its own process.
 *
 *  The config file is read with the data store codec of its extension (json, xml, ...). The settings are the
 *  "GucefLobbyServer" node of the file (or its root), see CGucefLobbyServerSettings, for instance
 *  { "GucefLobbyServer": { "listen": "tcp://0.0.0.0:47300;udp://0.0.0.0:47300", "relayEnabled": true,
 *                          "key": "...", "password": "", "maxLobbies": 100 } }
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGucefLobbyServerApp
{
    public:

    static const CORE::CString SettingsNodeName;    /**< "GucefLobbyServer" */

    CGucefLobbyServerApp( void );

    ~CGucefLobbyServerApp();

    bool LoadConfigFile( const CORE::CString& path );

    /** Takes the settings from the "GucefLobbyServer" node of the tree, or from the tree itself */
    bool LoadConfig( const CORE::CDataNode& tree );

    bool Start( void );

    void Stop( void );

    /**
     *  Starts the server and runs the application's pulse until the application stops (CGUCEFApplication::Stop(),
     *  for instance from a signal handler), then stops the server
     *
     *  @return the exit code: 0, or 1 when the server could not start
     */
    int Run( int argc, char** argv );

    CGucefLobbyServer& GetServer( void );

    private:

    CGucefLobbyServerApp( const CGucefLobbyServerApp& src );
    CGucefLobbyServerApp& operator=( const CGucefLobbyServerApp& src );

    CGucefLobbyServer m_server;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYSERVERAPP_H ? */
