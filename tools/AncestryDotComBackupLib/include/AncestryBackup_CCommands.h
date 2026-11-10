/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CCOMMANDS_H
#define ANCESTRYBACKUP_CCOMMANDS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ANCESTRYBACKUP_CSETTINGS_H
#include "AncestryBackup_CSettings.h"
#define ANCESTRYBACKUP_CSETTINGS_H
#endif /* ANCESTRYBACKUP_CSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Exit codes of the commands */
enum EExitCode
{
    EXIT_CODE_OK              = 0 ,
    EXIT_CODE_USAGE           = 1 ,     /**< unknown command, bad settings */
    EXIT_CODE_BROWSER         = 2 ,     /**< no browser, no connection to it */
    EXIT_CODE_NOT_LOGGED_IN   = 3 ,     /**< the browser holds no session of the site */
    EXIT_CODE_FAILED          = 4 ,
    EXIT_CODE_NO_RECENT_EXPORT = 5 ,    /**< no recent GEDCOM export was found to take in */
    EXIT_CODE_HALTED          = 6       /**< the site showed signs of unhappiness, see state/HALTED.txt */
};

/**
 *  The commands of the tool. They are blocking: they pump the event loop of the application
 *  themselves while they wait for the browser or the user.
 */
class CCommands
{
    public:

    /**
     *  Starts the browser with the profile of the tree, takes the user to the login page and, once they say they
     *  are logged in, checks that the browser holds a session for the site. Nothing of the session is stored by us:
     *  it lives in the profile of the browser, that is where the next commands pick it up.
     */
    static int Login( const CSettings& settings );

    /**
     *  Watches the browser while the user clicks through the site and records every request the site makes to
     *  its servers. The result is the journal of requests, the saved JSON responses and the endpoint report.
     */
    static int Discover( const CSettings& settings );

    /**
     *  Takes in the GEDCOM export of the tree which is the starting point of a backup: checks it, copies it into
     *  the data of the tree as baseline.ged and writes what it holds to state/baseline-summary.txt.
     *  Optional: without it everything has to come from the site. Without a file given the one that was
     *  imported before is checked again.
     */
    static int Baseline( const CSettings& settings );

    /**
     *  Looks for a GEDCOM export of the tree that was downloaded recently (in the downloads directory unless
     *  exportDirs says otherwise) and takes in the newest one that was not taken in yet, as baseline does.
     *  Nothing is done on the site. Ends with EXIT_CODE_NO_RECENT_EXPORT if there is none.
     */
    static int Export( const CSettings& settings );

    /**
     *  Asks the site's change feed what changed in the tree since the last time (or since the baseline export) and writes
     *  down which people need to be looked at again. Reads from the newest change back to the last one that was seen, a day's
     *  work is a request or two. Writes state/changes.jsonl, state/changed-people.txt and keeps the pages it read in raw/changes.
     */
    static int Changes( const CSettings& settings );

    /**
     *  Lists the media of the tree (photos, documents, stories, recordings, videos) and downloads the files of the pictures
     *  and documents in full size, with a file of metadata next to each. What is done is remembered: the next run
     *  goes on where this one stopped, and a file that is there is never fetched again.
     *
     *  Writes state/media-inventory.jsonl, media/<id>.<ext> and media/<id>.<ext>.json, and keeps the pages of the list in raw/media-lists.
     */
    static int Media( const CSettings& settings );

    /**
     *  Downloads the DNA matches of the user: the list of matches with the shared DNA, and for each match the predicted
     *  relationship, the side of the family and the name that the site shows. Writes dna/matches.csv, a flat file with one row per
     *  match that opens in any spreadsheet and can be used to look matches up elsewhere, dna/matches.jsonl and the answers of the site
     *  as they were in dna/raw. Goes on where the last run stopped.
     *
     *  This is the only command that talks to the DNA pages, and it is only for the matches. DNA pages are never recorded by discover
     *  unless recordDna is on, the sign-in and security checks of the site are never touched, and the few POST requests it makes only
     *  read (see CRequestPolicy::AddDnaAllowed).
     */
    static int Dna( const CSettings& settings );

    /**
     *  Turns the baseline GEDCOM export and the downloaded media into a Gramps XML file (export/<treeId>.gramps) that Gramps takes in as a
     *  tree of its own: people, families, events, places as a hierarchy, sources, citations, notes, and the media linked to the people and
     *  events they belong to. Nothing is asked of the site. Run it again after a newer export was taken in (export, baseline) and after more
     *  media was downloaded, it always converts everything, and the same data gives the same ids and handles. Import it in an empty Gramps tree.
     *
     *  Writes the Gramps file and state/gramps-report.txt, which also says who was added to or left the tree since the last conversion.
     */
    static int Gramps( const CSettings& settings );

    /** Rebuilds the endpoint report from the journal of earlier discovery runs, no browser needed */
    static int Report( const CSettings& settings );

    /** Asks a running command to end, to be called from a signal handler or another thread */
    static void RequestStop( void );

    static bool IsStopRequested( void );

    static const char* GetUsage( void );

    private:

    static void Pump( UInt32 delayInMs );

    /** A zip file with the export in it, which is how the site delivers it */
    static int ImportZip( const CDataLayout& layout, const CORE::CString& zipPath, UInt64 zipModifiedInMs );

    static int ImportGedcom( const CDataLayout& layout, const CORE::CString& source, bool copyIn, UInt64 sourceModifiedInMs );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CCOMMANDS_H ? */
