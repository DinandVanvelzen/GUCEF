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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETSTORE_ASSETSERVICEPAGES_H
#include "gucefASSETSTORE_AssetServicePages.h"
#define GUCEF_ASSETSTORE_ASSETSERVICEPAGES_H
#endif /* GUCEF_ASSETSTORE_ASSETSERVICEPAGES_H ? */

#include "AssetWorkbenchLib_CWorkbenchConsoleCommands.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The text without the white space around it and without a pair of quotes around the rest */
CORE::CString
TrimArgument( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString trimmed = text.Trim( true ).Trim( false );
    if ( trimmed.Length() >= 2 && ( ( '"' == trimmed[ 0 ] && '"' == trimmed[ trimmed.Length() - 1 ] ) || ( '\'' == trimmed[ 0 ] && '\'' == trimmed[ trimmed.Length() - 1 ] ) ) )
        trimmed = trimmed.SubstrFromRange( 1, trimmed.Length() - 1 );
    return trimmed;
}

}

/*-------------------------------------------------------------------------*/

const CWorkbenchConsoleCommands::SCommand*
CWorkbenchConsoleCommands::GetCommands( UInt32& count )
{GUCEF_TRACE;

    static const SCommand commands[] =
    {
        { "help", "help", "lists the commands", &CWorkbenchConsoleCommands::CommandHelp },
        { "open", "open <path>", "opens a folder, archive or file of the disk, or an asset of the VFS", &CWorkbenchConsoleCommands::CommandOpen },
        { "convert", "convert [pending | <pack id>]", "converts the packs that wait for it, or one pack of the primary store", &CWorkbenchConsoleCommands::CommandConvert },
        { "jobs", "jobs [open | all | active | running | queued | failed | done | cancelled | finished] [kind]", "lists the jobs of the asset store service and of the workbench, or opens the jobs page", &CWorkbenchConsoleCommands::CommandJobs },
        { "connect", "connect [host:port]", "connects to the remote asset store service of the address (the address of the settings by default); this computer has to be paired", &CWorkbenchConsoleCommands::CommandConnect },
        { "pair", "pair <code>", "pairs this computer with the remote asset store service with the one-time code of a pairing window of the server, then connects", &CWorkbenchConsoleCommands::CommandPair },
        { "disconnect", "disconnect", "ends the connection to the remote asset store service: the store of this computer is used again", &CWorkbenchConsoleCommands::CommandDisconnect },
        { "remote", "remote", "tells the state of the connection to the remote asset store service and what it is", &CWorkbenchConsoleCommands::CommandRemote },
        { "download", "download <pack id> <folder>", "copies the files of a pack of the remote asset store service to a folder of this computer (checked against their digests, continued when the link breaks), as a job", &CWorkbenchConsoleCommands::CommandDownload },
        { "offline", "offline <pack id> [release]", "keeps the files of a pack of the remote asset store service in the cache of this computer so they open without the service, or lets them go again, as a job", &CWorkbenchConsoleCommands::CommandOffline },
        { "layout", "layout <Browse | Review | Preview | lint [on | off | report]>", "puts the window in a layout, or runs the layout lint", &CWorkbenchConsoleCommands::CommandLayout },
        { "redo", "redo <reindex | reanalyse | reimport | reconvert | rethumbnail>", "does a step of the pack work again (the pack of the resource view, or the pack selected in the sources tab) as a job", &CWorkbenchConsoleCommands::CommandRedo },
        { "redownload", "redownload [<pack id>] [--no-convert] [--allow-large] [--no-repair]", "downloads the original of a pack of the store again from its store and verifies it: the same files change nothing (missing or damaged ones are put back), other files become a new pack, as a job", &CWorkbenchConsoleCommands::CommandRedownload },
        { "regen", "regen <intermediate | target> [<pack id> | .] [<asset path or glob> ...] [--failed] [--statuses a,b] [--kinds a,b] [--targets ogre] [--no-cook] [--accept-errors]", "makes the intermediate or the engine target resources of a pack (or of assets of it) again with the converter as it is now, as a job; the replaced outputs are kept for a rollback", &CWorkbenchConsoleCommands::CommandRegen },
        { "reprocess", "reprocess [plan | rollback] [<pack id> | .] [<asset path or glob> ...] [--steps redownload,intermediate,target] [the options of regen and redownload]", "the steps in a row as a job (by default intermediate and target); plan tells what would be done, rollback makes the previous generation of the outputs current again", &CWorkbenchConsoleCommands::CommandReprocess },
        { "bulk", "bulk [open | plan <pack id> | start <pack id>]", "opens the bulk conversion panel, or plans or starts the conversion of a pack of the store as a job", &CWorkbenchConsoleCommands::CommandBulk },
        { "reindex", "reindex", "builds the pack index of the pack in the resource view again (its resources by function)", &CWorkbenchConsoleCommands::CommandReIndex },
        { "resourceview", "resourceview [on | off | files | resources]", "tells or sets whether a pack that is opened starts in the resource view; files or resources switches the content browser now", &CWorkbenchConsoleCommands::CommandResourceView },
        { "presentation", "presentation [on | off | reset | <portrait | effect | scene | grid | animation | material | images | summary> | more]", "switches the presentation of packs that are opened, or shows another presentation of the pack that is shown", &CWorkbenchConsoleCommands::CommandPresentation },
        { "thumbnails", "thumbnails [on | off | clear]", "switches the automatic thumbnails, or deletes them", &CWorkbenchConsoleCommands::CommandThumbnails },
        { "source", "source [cooked | quicklook]", "tells which source the shown model, scene or pack comes from (the cooked Ogre output or the quick look) and what its cooked output is; cooked or quicklook switches the preference", &CWorkbenchConsoleCommands::CommandSource },
        { "ogre", "ogre", "Convert for Ogre: converts the shown model, scene or pack into cooked Ogre output as a job, and shows that when it is done", &CWorkbenchConsoleCommands::CommandOgre }
    };
    count = (UInt32) ( sizeof( commands ) / sizeof( commands[ 0 ] ) );
    return commands;
}

/*-------------------------------------------------------------------------*/

CWorkbenchConsoleCommands::CWorkbenchConsoleCommands( CWorkbenchShell& shell, GUI::CDeveloperConsoleForm& console )
    : CORE::CObserver()
    , m_shell( shell )
    , m_console( console )
{GUCEF_TRACE;

    TEventCallback callback( this, &CWorkbenchConsoleCommands::OnCommandEntered );
    SubscribeTo( &m_console, GUI::CDeveloperConsoleForm::CommandEnteredEvent, callback );
}

/*-------------------------------------------------------------------------*/

CWorkbenchConsoleCommands::~CWorkbenchConsoleCommands()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchConsoleCommands::GetCommandCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    GetCommands( count );
    return count;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchConsoleCommands::GetCommandName( UInt32 index ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    const SCommand* commands = GetCommands( count );
    return index < count ? CORE::CString( commands[ index ].name ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchConsoleCommands::GetCommandUsage( UInt32 index ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    const SCommand* commands = GetCommands( count );
    return index < count ? CORE::CString( commands[ index ].usage ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::Run( const CORE::CString& commandLine )
{GUCEF_TRACE;

    const CORE::CString trimmed = commandLine.Trim( true ).Trim( false );
    if ( trimmed.IsNULLOrEmpty() )
        return false;
    // the command is the first word, the argument is the rest of the line
    const Int32 space = trimmed.HasChar( ' ' );
    const CORE::CString name = space < 0 ? trimmed : trimmed.SubstrToIndex( (UInt32) space, true );
    const CORE::CString argument = space < 0 ? CORE::CString::Empty : TrimArgument( trimmed.SubstrToIndex( (UInt32) space + 1, false ) );
    UInt32 count = 0;
    const SCommand* commands = GetCommands( count );
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( name.Equals( commands[ i ].name, false ) )
        {
            ( this->*commands[ i ].function )( argument );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchConsoleCommands::OnCommandEntered( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const GUI::CDeveloperConsoleForm::TCommandEnteredEventData* command = static_cast< const GUI::CDeveloperConsoleForm::TCommandEnteredEventData* >( eventData );
    if ( GUCEF_NULL != command )
        Run( command->GetData() );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandHelp( const CORE::CString& argument )
{GUCEF_TRACE;

    UInt32 count = 0;
    const SCommand* commands = GetCommands( count );
    for ( UInt32 i=0; i<count; ++i )
        m_console.AppendLine( CORE::CString( commands[ i ].usage ) + ": " + commands[ i ].description );
    m_console.AppendLine( "gui.lint [on | off | overlay | log | report]: the layout lint of the GUI library" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandOpen( const CORE::CString& argument )
{GUCEF_TRACE;

    if ( argument.IsNULLOrEmpty() )
    {
        m_console.AppendLine( "Usage: open <path>" );
        return false;
    }
    // a path of the disk is mounted and shown, a path that is not of the disk is an asset of the VFS
    const CORE::CString diskPath = CORE::ResolveVars( argument );
    const bool opened = CORE::FileExists( diskPath ) || CORE::DirExists( diskPath ) ? m_shell.OpenLocation( diskPath ) : m_shell.OpenAsset( argument );
    m_console.AppendLine( opened ? "Opened " + argument : "Cannot open " + argument + " (the report of the workbench says why)" );
    return opened;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandConvert( const CORE::CString& argument )
{GUCEF_TRACE;

    CAssetSourcesPane* sources = m_shell.GetSources();
    if ( GUCEF_NULL == sources || GUCEF_NULL == sources->GetConverterHook() )
    {
        m_console.AppendLine( "There is no converter: the asset sources are not ready or conversion is switched off in the settings" );
        return false;
    }
    const bool started = argument.IsNULLOrEmpty() || argument.Equals( "pending", false ) ? sources->ConvertPendingPacks() : sources->ConvertPack( argument );
    m_console.AppendLine( started ? CORE::CString( "The conversion is queued, see the transfers" ) : CORE::CString( "Nothing to convert, or the pack is not there" ) );
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandDownload( const CORE::CString& argument )
{GUCEF_TRACE;

    CAssetSourcesPane* sources = m_shell.GetSources();
    if ( GUCEF_NULL == sources || !sources->IsUsingRemoteService() )
    {
        m_console.AppendLine( "There is no remote asset store service: connect to one first" );
        return false;
    }
    const CORE::CString text = argument.Trim( true ).Trim( false );
    const Int32 space = text.HasChar( ' ', true );
    if ( space <= 0 )
    {
        m_console.AppendLine( "Usage: download <pack id> <folder>" );
        return false;
    }
    const CORE::CString packId = text.SubstrFromRange( 0, (UInt32) space );
    const CORE::CString folder = TrimArgument( text.CutChars( (UInt32) space + 1, true ) );
    CORE::CString message;
    const bool started = sources->DownloadPack( packId, folder, message );
    m_console.AppendLine( started ? "The download of " + packId + " to " + folder + " is queued, see the transfers" : "The download cannot start: " + message );
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandOffline( const CORE::CString& argument )
{GUCEF_TRACE;

    CAssetSourcesPane* sources = m_shell.GetSources();
    if ( GUCEF_NULL == sources || !sources->IsUsingRemoteService() )
    {
        m_console.AppendLine( "There is no remote asset store service: connect to one first" );
        return false;
    }
    const CORE::CString text = argument.Trim( true ).Trim( false );
    const Int32 space = text.HasChar( ' ', true );
    const CORE::CString packId = space > 0 ? text.SubstrFromRange( 0, (UInt32) space ) : text;
    const bool release = space > 0 && text.CutChars( (UInt32) space + 1, true ).Trim( true ).Trim( false ).Equals( "release", false );
    if ( packId.IsNULLOrEmpty() )
    {
        m_console.AppendLine( "Usage: offline <pack id> [release]" );
        return false;
    }
    CORE::CString message;
    const bool started = sources->KeepPackOffline( packId, !release, message );
    m_console.AppendLine( started ? ( release ? "The pins of " + packId + " are released, see the transfers" : "Keeping " + packId + " offline, see the transfers" ) : "That cannot start: " + message );
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandConnect( const CORE::CString& argument )
{GUCEF_TRACE;

    CWorkbenchStoreConnection* connection = m_shell.GetStoreConnection();
    if ( GUCEF_NULL == connection )
    {
        m_console.AppendLine( "The connection to a remote store is not ready: the asset sources are not made" );
        return false;
    }
    if ( connection->IsBusy() )
    {
        m_console.AppendLine( "Something is going on already: " + connection->GetStateText() );
        return false;
    }
    const bool started = connection->Connect( argument );
    if ( started )
        m_console.AppendLine( connection->IsConnected() ? "Connected to " + connection->GetStoreLabel() : "Connecting to " + ( connection->GetEndpoint().IsNULLOrEmpty() ? CORE::CString( "the store service" ) : connection->GetEndpoint() ) + ": type remote to see how it goes" );
    else
        m_console.AppendLine( connection->GetErrorText().IsNULLOrEmpty() ? CORE::CString( "The connection could not be started" ) : connection->GetErrorText() );
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandPair( const CORE::CString& argument )
{GUCEF_TRACE;

    CWorkbenchStoreConnection* connection = m_shell.GetStoreConnection();
    if ( GUCEF_NULL == connection )
    {
        m_console.AppendLine( "The connection to a remote store is not ready: the asset sources are not made" );
        return false;
    }
    if ( argument.IsNULLOrEmpty() )
    {
        m_console.AppendLine( "Usage: pair <code>   (the one-time code that the server shows when pairing is opened; the Pair button of Settings, Remote store does not leave it in the console)" );
        return false;
    }
    AUTH::CSecretString code( argument );
    const bool started = connection->Pair( code );
    code.Clear();
    m_console.AppendLine( started ? CORE::CString( "Pairing with " ) + ( connection->GetPairingEndpoint().IsNULLOrEmpty() ? CORE::CString( "the service on the local network" ) : connection->GetPairingEndpoint() ) + ": type remote to see how it goes" :
                                    ( connection->GetErrorText().IsNULLOrEmpty() ? CORE::CString( "The pairing could not be started" ) : connection->GetErrorText() ) );
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandDisconnect( const CORE::CString& argument )
{GUCEF_TRACE;

    CWorkbenchStoreConnection* connection = m_shell.GetStoreConnection();
    if ( GUCEF_NULL == connection )
    {
        m_console.AppendLine( "The connection to a remote store is not ready: the asset sources are not made" );
        return false;
    }
    connection->Disconnect();
    m_console.AppendLine( "Disconnected: the workbench uses the store of this computer" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandRemote( const CORE::CString& argument )
{GUCEF_TRACE;

    CWorkbenchStoreConnection* connection = m_shell.GetStoreConnection();
    if ( GUCEF_NULL == connection )
    {
        m_console.AppendLine( "The connection to a remote store is not ready: the asset sources are not made" );
        return false;
    }
    m_console.AppendLine( CORE::CString( "State: " ) + CWorkbenchStoreConnection::GetStateName( connection->GetState() ) + ". " + connection->GetStateText() );
    m_console.AppendLine( "Address: " + ( connection->GetEndpoint().IsNULLOrEmpty() ? CORE::CString( "none (the endpoints of the pairing, else the local network)" ) : connection->GetEndpoint() ) +
                          ( connection->IsPaired() ? CORE::CString( ", this computer is paired" ) : CORE::CString( ", this computer is not paired" ) ) );
    if ( !connection->GetErrorText().IsNULLOrEmpty() )
        m_console.AppendLine( "Problem: " + connection->GetErrorText() );
    const CORE::CString summary = connection->GetServerSummary();
    if ( !summary.IsNULLOrEmpty() )
    {
        const CORE::CString::StringVector lines = summary.ParseElements( '\n', false );
        for ( size_t i=0; i<lines.size(); ++i )
            m_console.AppendLine( lines[ i ] );
    }
    const CWorkbenchStoreConnection::TDiscoveredVector& found = connection->GetDiscoveredServices();
    for ( size_t i=0; i<found.size(); ++i )
        m_console.AppendLine( "Found on the network: " + CWorkbenchStoreConnection::DescribeService( found[ i ] ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandJobs( const CORE::CString& argument )
{GUCEF_TRACE;

    if ( argument.Equals( "open", false ) )
        return m_shell.ShowJobs();
    CAssetSourcesPane* sources = m_shell.GetSources();
    CAssetStoreTransferModel* model = GUCEF_NULL != sources ? sources->GetTransferModel() : GUCEF_NULL;
    if ( GUCEF_NULL == model )
    {
        m_console.AppendLine( "The asset store service is not there" );
        return false;
    }
    CWorkbenchJobFilter filter;
    const CORE::CString::StringVector words = argument.ParseElements( ' ', false );
    for ( size_t w=0; w<words.size(); ++w )
    {
        CWorkbenchJobFilter::EStateGroup group = CWorkbenchJobFilter::STATES_ALL;
        if ( CWorkbenchJobFilter::ParseStateGroup( words[ w ], group ) )
            filter.states = group;
        else
            filter.kind = words[ w ];
    }
    ASSETSTORE::TAssetJobEntryVector jobs;
    model->GetJobs( jobs );
    UInt32 shown = 0;
    for ( size_t i=0; i<jobs.size(); ++i )
    {
        if ( !filter.Matches( jobs[ i ] ) )
            continue;
        const ASSETSTORE::CAssetJobEntry& job = jobs[ i ];
        m_console.AppendLine( job.jobId + "  " + CWorkbenchJobFilter::DisplayKindOf( job ) + "  " + ( job.scope.IsNULLOrEmpty() ? job.name : job.scope ) + ": " +
                              CWorkbenchJobFilter::StateTextOf( job ) + ( job.resultText.IsNULLOrEmpty() ? CORE::CString::Empty : " (" + job.resultText + ")" ) );
        ++shown;
    }
    if ( 0 == shown )
        m_console.AppendLine( "No jobs" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandRedo( const CORE::CString& argument )
{GUCEF_TRACE;

    CWorkbenchRedoActions::ERedoAction action = CWorkbenchRedoActions::REDO_REINDEX;
    if ( !CWorkbenchRedoActions::Parse( TrimArgument( argument ), action ) )
    {
        m_console.AppendLine( "redo <reindex | reanalyse | reimport | reconvert | rethumbnail>" );
        return false;
    }
    const bool started = m_shell.RedoPack( action );
    m_console.AppendLine( started ? CORE::CString( CWorkbenchRedoActions::GetInfo( action ).menuText ) + " is a job now, see the jobs" : CORE::CString( "There is no pack to do that for: open a pack, or select one in the sources tab" ) );
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::RunReprocess( CWorkbenchReprocessActions::EReprocessAction action, const CORE::CString::StringVector& words, const char* usage )
{GUCEF_TRACE;

    // the pack id first ("." or nothing: the selected pack), then the options and the assets
    CORE::CString packId;
    CORE::CString::StringVector rest = words;
    if ( !rest.empty() && 0 != rest[ 0 ].HasSubstr( "--", true ) )
    {
        packId = "." == rest[ 0 ] ? CORE::CString::Empty : rest[ 0 ];
        rest.erase( rest.begin() );
    }
    ASSETSTORE::CAssetReprocessRequest options;
    CORE::CString problem;
    if ( !CWorkbenchReprocessActions::ParseOptions( rest, options, problem ) )
    {
        m_console.AppendLine( problem );
        m_console.AppendLine( CORE::CString( "Usage: " ) + usage );
        return false;
    }
    CORE::CString message;
    const bool started = m_shell.ReprocessPack( action, packId, options, message );
    m_console.AppendLine( message );
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandRedownload( const CORE::CString& argument )
{GUCEF_TRACE;

    RunReprocess( CWorkbenchReprocessActions::REPROCESS_REDOWNLOAD, TrimArgument( argument ).ParseElements( ' ', false ), "redownload [<pack id>] [--no-convert] [--allow-large] [--no-repair]" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandRegen( const CORE::CString& argument )
{GUCEF_TRACE;

    static const char* usage = "regen <intermediate | target> [<pack id> | .] [<asset path or glob> ...] [--failed] [--statuses a,b] [--kinds a,b] [--targets ogre] [--no-cook] [--accept-errors]";
    CORE::CString::StringVector words = TrimArgument( argument ).ParseElements( ' ', false );
    CWorkbenchReprocessActions::EReprocessAction action = CWorkbenchReprocessActions::REPROCESS_INTERMEDIATE;
    if ( words.empty() || !CWorkbenchReprocessActions::Parse( words[ 0 ], action ) ||
         ( CWorkbenchReprocessActions::REPROCESS_INTERMEDIATE != action && CWorkbenchReprocessActions::REPROCESS_TARGET != action ) )
    {
        m_console.AppendLine( CORE::CString( "Usage: " ) + usage );
        return true;
    }
    words.erase( words.begin() );
    RunReprocess( action, words, usage );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandReprocess( const CORE::CString& argument )
{GUCEF_TRACE;

    static const char* usage = "reprocess [plan | rollback] [<pack id> | .] [<asset path or glob> ...] [--steps redownload,intermediate,target] [options]";
    CORE::CString::StringVector words = TrimArgument( argument ).ParseElements( ' ', false );
    CWorkbenchReprocessActions::EReprocessAction action = CWorkbenchReprocessActions::REPROCESS_ALL;
    if ( !words.empty() && ( words[ 0 ].Equals( "plan", false ) || words[ 0 ].Equals( "rollback", false ) ) )
    {
        CWorkbenchReprocessActions::Parse( words[ 0 ], action );
        words.erase( words.begin() );
    }
    RunReprocess( action, words, usage );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandBulk( const CORE::CString& argument )
{GUCEF_TRACE;

    const CORE::CString::StringVector words = TrimArgument( argument ).ParseElements( ' ', false );
    if ( words.empty() || words[ 0 ].Equals( "open", false ) )
        return m_shell.ShowBulkConversion();
    CBulkConversionController* controller = m_shell.GetBulkConversionController();
    if ( GUCEF_NULL == controller || words.size() < 2 )
    {
        m_console.AppendLine( "bulk [open | plan <pack id> | start <pack id>]" );
        return false;
    }
    CBulkConversionSettings settings;
    settings.packId = words[ 1 ];
    if ( !controller->RequestPlan( settings ) )
    {
        m_console.AppendLine( controller->GetProblem() );
        return false;
    }
    m_console.AppendLine( controller->IsPlanning() ? CORE::CString( "Planning..." ) : controller->GetPlan().ToSummaryText() );
    if ( words[ 0 ].Equals( "start", false ) )
    {
        const UInt32 started = controller->Start();
        m_console.AppendLine( started > 0 ? CORE::CString( "The conversion is a job now, see the jobs" ) : controller->GetProblem() );
        return started > 0;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandLayout( const CORE::CString& argument )
{GUCEF_TRACE;

    if ( 0 == argument.HasSubstr( "lint" ) )
    {
        // the lint is the console's own command
        const CORE::CString lintArgument = argument.SubstrToIndex( 4, false ).Trim( true ).Trim( false );
        m_console.ExecuteCommand( lintArgument.IsNULLOrEmpty() ? CORE::CString( "gui.lint report" ) : "gui.lint " + lintArgument );
        return true;
    }
    if ( !m_shell.ApplyLayoutPreset( argument ) )
    {
        m_console.AppendLine( "There is no layout " + argument + ": Browse, Review or Preview" );
        return false;
    }
    m_console.AppendLine( "The layout is " + argument );
    return true;
}

/*-------------------------------------------------------------------------*/
bool
CWorkbenchConsoleCommands::CommandReIndex( const CORE::CString& argument )
{GUCEF_TRACE;

    const bool started = m_shell.ReIndexPack();
    m_console.AppendLine( started ? CORE::CString( "The pack index is built again" ) : CORE::CString( "There is no pack in the resource view" ) );
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandResourceView( const CORE::CString& argument )
{GUCEF_TRACE;

    if ( argument.Equals( "files", false ) || argument.Equals( "resources", false ) )
    {
        m_shell.ShowResourceView( argument.Equals( "resources", false ) );
        m_console.AppendLine( CORE::CString( "The content browser shows the " ) + ( m_shell.IsResourceViewShown() ? "resources" : "files" ) );
        return true;
    }
    if ( argument.Equals( "on", false ) || argument.Equals( "off", false ) )
        m_shell.SetOpenPacksInResourceView( argument.Equals( "on", false ) );
    m_console.AppendLine( CORE::CString( "A pack that is opened starts in the " ) + ( m_shell.IsOpenPacksInResourceView() ? "resource view" : "files view" ) );
    return true;
}

/*-------------------------------------------------------------------------*/


bool
CWorkbenchConsoleCommands::CommandPresentation( const CORE::CString& argument )
{GUCEF_TRACE;

    if ( argument.IsNULLOrEmpty() )
    {
        m_console.AppendLine( CORE::CString( "The presentation of a pack that is opened is " ) + ( m_shell.IsPackPresentationEnabled() ? "on" : "off" ) );
        return true;
    }
    if ( argument.Equals( "on", false ) || argument.Equals( "off", false ) )
    {
        m_shell.SetPackPresentationEnabled( argument.Equals( "on", false ) );
        m_console.AppendLine( CORE::CString( "The presentation of a pack that is opened is " ) + ( m_shell.IsPackPresentationEnabled() ? "on" : "off" ) );
        return true;
    }
    bool done = false;
    if ( argument.Equals( "reset", false ) )
        done = m_shell.ResetPackPresentation();
    else
    if ( argument.Equals( "more", false ) )
        done = m_shell.LoadMoreOfPresentation();
    else
        done = m_shell.ShowPackPresentation( m_shell.GetPackRoot(), argument.Lowercase(), true );
    m_console.AppendLine( done ? CORE::CString( "The presentation is shown" ) : CORE::CString( "There is no pack shown, or it has no such presentation" ) );
    return done;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandThumbnails( const CORE::CString& argument )
{GUCEF_TRACE;

    if ( argument.Equals( "clear", false ) )
    {
        m_console.AppendLine( CORE::UInt32ToString( m_shell.ClearAutoThumbnails() ) + " automatic thumbnails were deleted" );
        return true;
    }
    if ( argument.Equals( "on", false ) || argument.Equals( "off", false ) )
        m_shell.SetAutomaticThumbnailsEnabled( argument.Equals( "on", false ) );
    m_console.AppendLine( CORE::CString( "Automatic thumbnails are " ) + ( m_shell.GetAutoThumbnails().IsEnabled() ? "on" : "off" ) + ", " + CORE::UInt32ToString( m_shell.GetAutoThumbnails().GetCount() ) + " are kept" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandSource( const CORE::CString& argument )
{GUCEF_TRACE;

    if ( argument.Equals( "cooked", false ) || argument.Equals( "quicklook", false ) )
        m_shell.SetCookedOutputPreferred( argument.Equals( "cooked", false ) );
    m_console.AppendLine( m_shell.GetSourceReport() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchConsoleCommands::CommandOgre( const CORE::CString& argument )
{GUCEF_TRACE;

    const bool started = m_shell.ConvertForOgre();
    m_console.AppendLine( started ? CORE::CString( "Convert for Ogre is started; the view switches to the cooked output when it is done" ) : CORE::CString( "Convert for Ogre was not started: see the report" ) );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchConsoleCommands::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CWorkbenchConsoleCommands";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
