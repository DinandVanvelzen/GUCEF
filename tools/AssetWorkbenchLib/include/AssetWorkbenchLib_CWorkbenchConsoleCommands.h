/*
 *  AssetWorkbenchLib: the commands of the developer console of the workbench
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHCONSOLECOMMANDS_H
#define ASSETWORKBENCHLIB_CWORKBENCHCONSOLECOMMANDS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CDEVELOPERCONSOLEFORM_H
#include "gucefGUI_CDeveloperConsoleForm.h"
#define GUCEF_GUI_CDEVELOPERCONSOLEFORM_H
#endif /* GUCEF_GUI_CDEVELOPERCONSOLEFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSHELL_H
#include "AssetWorkbenchLib_CWorkbenchShell.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSHELL_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSHELL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The commands of the developer console of the workbench: what a person types in the console of gucefGUI (CDeveloperConsoleForm) is
 *  read here and given to the command of that name, whose answer goes to the output of the console.
 *
 *      help                                    the commands
 *      open <path>                             a folder, archive or file of the disk, or an asset of the VFS
 *      convert [pending | <pack id>]           converts the packs that wait for it (the default), or one pack of the primary store
 *      jobs                                    the jobs of the asset store service
 *      connect [host:port]                     connects to the remote asset store service (this computer has to be paired); the address of the settings by default
 *      pair <code>                             pairs this computer with the service with the one-time code of a pairing window of the server, then connects
 *      disconnect                              ends the connection to the remote store
 *      remote                                  the state of the connection to the remote store
 *      layout <name>                           puts the window in a layout: Browse, Review or Preview
 *      layout lint [on | off | report]         the layout lint of the GUI library (gui.lint, which the console has itself)
 *
 *  A command that is not here is not an error of the console: the line stays in the output and other observers of the console see it.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchConsoleCommands : public CORE::CObserver
{
    public:

    /** The shell and the console that the commands work on; neither is owned, both have to be there as long as the commands */
    CWorkbenchConsoleCommands( CWorkbenchShell& shell, GUI::CDeveloperConsoleForm& console );

    virtual ~CWorkbenchConsoleCommands() GUCEF_VIRTUAL_OVERRIDE;

    UInt32 GetCommandCount( void ) const;

    /** The name and the usage of the command at the index, for example "open" and "open <path>" */
    CORE::CString GetCommandName( UInt32 index ) const;
    CORE::CString GetCommandUsage( UInt32 index ) const;

    /** Runs a command line as the console gives it; false for a command that is not one of these. The answer is in the output of the console */
    bool Run( const CORE::CString& commandLine );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CWorkbenchConsoleCommands > TEventCallback;
    typedef bool ( CWorkbenchConsoleCommands::*TCommandFunction )( const CORE::CString& argument );

    struct SCommand
    {
        const char* name;
        const char* usage;
        const char* description;
        TCommandFunction function;
    };

    void OnCommandEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    bool CommandHelp( const CORE::CString& argument );
    bool CommandOpen( const CORE::CString& argument );
    bool CommandConvert( const CORE::CString& argument );
    bool CommandJobs( const CORE::CString& argument );
    bool CommandConnect( const CORE::CString& argument );
    bool CommandPair( const CORE::CString& argument );
    bool CommandDisconnect( const CORE::CString& argument );
    bool CommandRemote( const CORE::CString& argument );
    bool CommandDownload( const CORE::CString& argument );
    bool CommandOffline( const CORE::CString& argument );
    bool CommandLayout( const CORE::CString& argument );
    bool CommandPresentation( const CORE::CString& argument );
    bool CommandReIndex( const CORE::CString& argument );
    bool CommandRedo( const CORE::CString& argument );
    bool CommandRedownload( const CORE::CString& argument );
    bool CommandRegen( const CORE::CString& argument );
    bool CommandReprocess( const CORE::CString& argument );
    bool RunReprocess( CWorkbenchReprocessActions::EReprocessAction action, const CORE::CString::StringVector& words, const char* usage );
    bool CommandBulk( const CORE::CString& argument );
    bool CommandResourceView( const CORE::CString& argument );
    bool CommandThumbnails( const CORE::CString& argument );
    bool CommandSource( const CORE::CString& argument );
    bool CommandOgre( const CORE::CString& argument );

    static const SCommand* GetCommands( UInt32& count );

    CWorkbenchConsoleCommands( const CWorkbenchConsoleCommands& src );             /**< not implemented */
    CWorkbenchConsoleCommands& operator=( const CWorkbenchConsoleCommands& src );  /**< not implemented */

    private:

    CWorkbenchShell& m_shell;
    GUI::CDeveloperConsoleForm& m_console;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHCONSOLECOMMANDS_H ? */
