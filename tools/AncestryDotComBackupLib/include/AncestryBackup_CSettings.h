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

#ifndef ANCESTRYBACKUP_CSETTINGS_H
#define ANCESTRYBACKUP_CSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ANCESTRYBACKUP_CDATALAYOUT_H
#include "AncestryBackup_CDataLayout.h"
#define ANCESTRYBACKUP_CDATALAYOUT_H
#endif /* ANCESTRYBACKUP_CDATALAYOUT_H ? */

#include <string>
#include <vector>
#include <map>

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

/**
 *  The settings of the tool. They come from the ini file next to the tool and from the command line, where
 *  the command line wins. Setting names are not case sensitive.
 *
 *    dataDir              where the data of all trees goes (default: data)
 *    siteTreeId           the id Ancestry uses for the tree, a number. Default: the treeId if that is a number, else the one in the baseline GEDCOM
 *    treeId               names the directory of this tree below the data dir (default: default)
 *    chromePath           the browser to use, by default it is looked for
 *    debugPort            the remote debugging port of the browser (default: 9222)
 *    startUrl             the page the browser opens (default: https://www.ancestry.com/)
 *    cookieDomain         the domain the login cookies belong to (default: ancestry.com)
 *    hostFilter           terms, separated by ; or , of which a host has to contain one to be recorded (default: ancestry)
 *    discoverSeconds      stop discovery after this long, 0 waits for the user (default: 0)
 *    maxBodyMegabytes     responses larger than this are not saved during discovery (default: 32)
 *    headless             run the browser without a window, for tests (default: false)
 *    noWait               do not wait for the user: login and discover only act on what already happened (default: false)
 *    chromeArgs           extra arguments for the browser, separated by ;
 *    baselineGedcom       the GEDCOM export of the tree to start from, optional. Also accepted as 'gedcom'. See the baseline command.
 *    exportDirs           where to look for a downloaded GEDCOM export, separated by ; (default: the downloads directory)
 *    exportMaxAgeDays     how old a downloaded export may be to count as recent (default: 7)
 *    requestDelayMs       the pause between two requests to the site (default: 2500), varied by requestJitterPercent (default: 40)
 *    maxRequestsPerRun    the most requests in one run, 0 for no limit (default: 500)
 *    maxRequestsPerDay    the most requests in one UTC day, 0 for no limit (default: 3000)
 *    changesSince         for the changes command: look for changes since this time (2026-10-16T21:27:48Z). Default: since the last run, or since the baseline export
 *    changesMaxPages      the most pages of the change feed to read in one run (default: 50, 20 changes per page)
 *    recordDna            let discover record the DNA pages as well, to learn how the DNA facility has to talk to the site. Passwords and sign in pages are never recorded. (default: false)
 *    dnaSampleId          for the dna command: the id of your DNA test at Ancestry. Default: found in the earlier discovery of the DNA pages
 *    retryMissing         for the media command: ask again for files the site could not find earlier (default: false)
 *    grampsFile           for the gramps command: the file to write (default: export/<treeId>.gramps in the data of the tree)
 *    grampsMediaPath      for the gramps command: the base media path of the Gramps tree. Default: the absolute path of the media directory of the tree
 *    grampsHomePerson     for the gramps command: the id of the person (I40537067793) who is the home person in Gramps. Default: the first person of the GEDCOM file
 *    researcherName       for the gramps command: your name as the researcher of the Gramps tree, optional
 *    researcherEmail      for the gramps command: your e-mail address as the researcher, optional
 *    attachToRunning      use a browser that is already open when it allows remote debugging, see chrome://inspect/#remote-debugging (default: true)
 *    chromeUserDataDir    where to look for that browser's DevToolsActivePort file, separated by ; (default: the usual profile locations)
 */
class CSettings
{
    public:

    CORE::CString dataDir;
    CORE::CString treeId;
    CORE::CString dnaSampleId;
    CORE::CString siteTreeId;               /**< empty: worked out, see CCommands */
    CORE::CString chromePath;
    CORE::CString startUrl;
    CORE::CString cookieDomain;
    CORE::CString grampsFile;               /**< empty: export/<treeId>.gramps */
    CORE::CString grampsMediaPath;          /**< empty: the media directory of the tree */
    CORE::CString grampsHomePerson;
    CORE::CString researcherName;
    CORE::CString researcherEmail;
    CORE::CString baselineGedcom;           /**< empty: none given, the one imported earlier is used if there is one */
    std::vector< std::string > exportDirs;  /**< empty: the downloads directory */
    UInt32 exportMaxAgeDays;
    CORE::CString changesSince;
    UInt32 changesMaxPages;
    UInt32 requestDelayInMs;
    UInt32 requestJitterPercent;
    UInt32 maxRequestsPerRun;
    UInt32 maxRequestsPerDay;
    UInt16 debugPort;
    UInt32 discoverSeconds;
    UInt64 maxBodyBytes;
    bool headless;
    bool noWait;
    bool attachToRunning;
    bool retryMissing;
    bool recordDna;
    std::vector< std::string > chromeUserDataDirs;
    std::vector< std::string > hostFilter;
    std::vector< std::string > chromeArgs;

    CSettings( void );

    /**
     *  Takes the settings from name value pairs, names in lower case. Settings that are not there keep their value.
     *  @param problem  describes what is wrong when this fails
     */
    bool Load( const std::map< std::string, std::string >& options, CORE::CString& problem );

    CDataLayout GetLayout( void ) const;

    /**
     *  Splits a command line into the command and the settings:
     *    command setting=value --setting=value --flag
     *  @param args     the arguments, without the name of the program
     */
    static bool ParseCommandLine( const std::vector< std::string >& args                  ,
                                  std::string& command                                    ,
                                  std::map< std::string, std::string >& options           ,
                                  CORE::CString& problem                                  );

    static std::vector< std::string > SplitList( const std::string& text );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CSETTINGS_H ? */
