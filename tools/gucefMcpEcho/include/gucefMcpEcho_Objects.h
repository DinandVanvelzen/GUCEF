/*
 *  gucefMcpEcho: a small reference MCP server built on gucefCOM
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

#ifndef GUCEF_MCPECHO_OBJECTS_H
#define GUCEF_MCPECHO_OBJECTS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The demo tools, resources and prompts of the reference server. They exist to be called by conformance runs and by
 *  the tests of the MCP transports:
 *
 *    echo        read-only, quick: returns its text argument
 *    slow_count  read-only, runs on the thread pool, reports progress and stops when it is cancelled
 *    ask_name    read-only, asks the client for a name by form elicitation (multi round-trip request)
 *    add_note    additive: denied and hidden until the access policy names it; stores a note
 *
 *    echo://about            a fixed resource
 *    echo://notes/{id}       a template: the notes that add_note stored
 *    greet                   a prompt with a required and an optional argument
 */
namespace GUCEFMcpEcho {

/** The notes that add_note stores, shared with the resource template that reads them */
class CNoteStore
{
    public:

    CNoteStore( void );

    /** @return the id of the note, from 1 */
    GUCEF::UInt32 Add( const GUCEF::CORE::CString& title, const GUCEF::CORE::CString& text );

    bool TryGet( GUCEF::UInt32 id, GUCEF::CORE::CString& title, GUCEF::CORE::CString& text ) const;

    private:

    mutable GUCEF::MT::CMutex m_lock;
    GUCEF::vector< GUCEF::CORE::CString > m_titles;
    GUCEF::vector< GUCEF::CORE::CString > m_texts;
};

/*-------------------------------------------------------------------------*/

/** Registers everything in the registries of the server. @return false when one of them was refused */
bool RegisterDemoObjects( GUCEF::COM::CMcpServer& server, CNoteStore& notes );

}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPECHO_OBJECTS_H ? */
