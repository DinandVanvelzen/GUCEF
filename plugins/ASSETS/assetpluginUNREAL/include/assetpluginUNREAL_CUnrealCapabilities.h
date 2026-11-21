/*
 *  assetpluginUNREAL: What the reader can and cannot do on this machine (the Oodle tool)
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#include "assetpluginUNREAL_macros.h"
#define GUCEF_ASSETPLUGIN_UNREAL_MACROS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_MACROS_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The capability list of the Unreal reader, for the hosts to query (the workbench, the converter command line, the store
 *  service). Today it has one entry: whether the external Oodle tool "ooz_extract" can be found. Without it nothing fails: the
 *  assets that need it say so in their reports, and everything else loads as before.
 *
 *  The tool is looked for in one place: next to the executable of the host application, as "ooz_extract" (".exe" on MS
 *  Windows). The environment variable GUCEF_OOZ_EXTRACT_PATH overrides the place, for tests and for installs that keep the tool
 *  elsewhere. The system search path is never used.
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealCapabilities
{
    public:

    /** The name of the environment variable that names the tool (a path to the executable) */
    static const char* const OodleToolPathVariable;

    /** The text a report carries for an asset that needs the provider when the tool is missing, followed by the reason */
    static const char* const NeedsOodleProvider;

    /** The path the tool is expected at, whether it exists or not: the override of the environment, else next to the host executable */
    static CORE::CString GetExpectedOodleToolPath( void );

    /** Whether the tool is where it is expected; path receives the expected path */
    static bool FindOodleTool( CORE::CString& path );

    /** "oodle: available (path)" or "oodle: unavailable (tool not found at path): Unreal 5 textures and static meshes unreadable" */
    static CORE::CString GetOodleStatus( void );

    /** The capability lines of the reader, one per entry: the Oodle line first */
    static CORE::CString::StringVector GetCapabilityList( void );

    /** "needs the oodle provider: tool not found at path" for the report of an asset */
    static CORE::CString GetNeedsOodleText( void );

    /** Writes the Oodle status to the log, once per process (the host calls it at start up or the first load does) */
    static void NoteOodleStatusOnce( void );

    /** Lets NoteOodleStatusOnce() write again; for tests */
    static void ResetNotedState( void );

    private:
    CUnrealCapabilities( void );    /**< only static members */
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H ? */
