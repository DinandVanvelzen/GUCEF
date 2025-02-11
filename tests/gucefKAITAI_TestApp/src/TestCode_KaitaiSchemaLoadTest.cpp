/*
 *  gucefKAITAI: Platform module supporting the Kaitai binary description format
 *
 *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
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

#include <vector>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"          /* GUCEF CORE library API */
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_KAITAI_H
#include "gucefKAITAI.h"
#define GUCEF_KAITAI_H
#endif /* GUCEF_KAITAI_H ? */

#ifdef GUCEF_MSWIN_BUILD
  #include <windows.h>
#else
  #include <assert.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MSWIN_BUILD
  #define DebugBreak() assert( 0 )
#endif

#define ERRORHERE { GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "Test failed @ " + CORE::CString( __FILE__ ) + CORE::UInt32ToString( __LINE__ ) ); DebugBreak(); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* kaitaiSchema_core_dynamic =
"meta:\n"
"  id: gucef_core_dynamic\n"
"  endian: le\n"
"seq:\n"
"  - id: len_data\n"
"    type: u4\n"
"  - id: data\n"
"    size: len_data\n";


const char* kaitaiSchema_pubsub_msgcontainer_header =
"meta:\n"
"  id: gucef_pubsub_msgcontainer_header\n"
"  endian: le\n"
"seq:\n"
"  - id: magic_text\n"
"    contents: PUBSUBMSGS\n"
"  - id: version\n"
"    type: u1\n"
"  - id: options\n"
"    type: u4\n";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformKaitaiSchemaLoadTest( void )
{  
    #ifdef GUCEF_VFS_DEBUG_MODE
    //MEMAN_LogStackToStdOut();
    //MEMAN_SetStackLogging( 1 );
    #endif /* GUCEF_VFS_DEBUG_MODE ? */
    
    try
    {
        KAITAI::CKaitaiGlobal* kaitai = KAITAI::CKaitaiGlobal::Instance();

        KAITAI::CKaitaiSchemaRegistry& schemaRegistry = kaitai->GetKaitaiSchemaRegistry();

        schemaRegistry.LoadSchemaFromString( kaitaiSchema_core_dynamic, "gucef" );
        schemaRegistry.LoadSchemaFromString( kaitaiSchema_pubsub_msgcontainer_header, "gucef" );
        
        GUCEF_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "Finished all load unload tests" );
        
    }
    catch ( ... )
    {
        GUCEF_ERROR_LOG( GUCEF::CORE::LOGLEVEL_NORMAL, "unhandled exception during test" );
        #ifdef GUCEF_VFS_DEBUG_MODE
        //GUCEF::MEMAN_PrintCallstack();
        #endif /* GUCEF_VFS_DEBUG_MODE ? */
        ERRORHERE;
    } 
}

/*-------------------------------------------------------------------------*/
