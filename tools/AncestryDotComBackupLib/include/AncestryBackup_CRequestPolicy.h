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

#ifndef ANCESTRYBACKUP_CREQUESTPOLICY_H
#define ANCESTRYBACKUP_CREQUESTPOLICY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include <string>
#include <vector>
#include <set>

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

/** The verdict on a request */
struct SRequestVerdict
{
    bool allowed;
    std::string reason;         /**< why not, when it is not allowed */

    SRequestVerdict( void ) : allowed( false ), reason() {}
};

/**
 *  The one place that decides whether the tool may send a request to the site. Every request the tool makes
 *  goes through it, and a request is allowed only when ALL of these hold:
 *
 *    - it is a GET. The tool never changes anything on the site. The one exception are a few POST requests of the DNA
 *      facility that only read, see AddDnaAllowed(), which have to be asked for and carry nothing but ids
 *    - it goes to an Ancestry host over https
 *    - it does not carry credentials: sign in, password, verification and account pages are never touched
 *    - it is not about DNA, unless the DNA facility asked for DNA access
 *    - it is about the tree the tool was set up for and no other tree
 *    - it is one of the endpoints on the allowlist. Some GET requests of the site change things (saving a person
 *      to the tree is a GET), so being a GET is not enough: only requests known to only read are allowed
 *
 *  The allowlist is made of path templates in the form of the endpoint report: host + path in which ids are written
 *  as {n} and {guid}, and a * stands for any one segment. {tree} stands for the id of the tree and only that id.
 */
class CRequestPolicy
{
    public:

    CRequestPolicy( void );

    /**
     *  The tree the tool works with, a request about another tree is refused. It is the id the site uses, a number.
     *  Anything else (such as the name of a directory) is not accepted and leaves no tree set, so nothing about a tree is allowed.
     *  @return false if it is not a number
     */
    bool SetTreeId( const std::string& treeId );

    /** Allow the DNA endpoints that are on the allowlist. Credentials stay forbidden regardless */
    void SetDnaAllowed( bool allowed );

    /** Allow requests to start a new GEDCOM export (the export url with start=true). Off unless a command needs it */
    void SetExportStartAllowed( bool allowed );

    /** For tests that talk to a site of their own. The tool itself only ever works with Ancestry's hosts over https. */
    void SetHostSuffixesForTesting( const std::vector< std::string >& suffixes, bool requireHttps );

    /** Adds a path template, for example: www.ancestry.com/api/treesui-list/trees/{n}/persons */
    void AddAllowed( const std::string& pathTemplate );

    /** The endpoints which are known to only read: tree, people, media, comments, change feed, export */
    void AddDefaultAllowed( void );

    /**
     *  The endpoints of the DNA facility, and with them the only requests that are not a GET. The DNA pages of the site ask
     *  for many matches at once with a POST that only reads, a list of the ids of the matches in, details out. Those, and
     *  only those, are let through, and only when the body is a list of ids: letters, digits and dashes and nothing else.
     *  Nothing here writes: tagging a match, ignoring it, or any other change is not on the list.
     */
    void AddDnaAllowed( void );

    /**
     *  Adds templates from a text file, one per line. Lines that start with # are comments.
     *  @return the number added
     */
    UInt32 LoadAllowedFile( const CORE::CString& path );

    /**
     *  @param body     what would be sent with the request. Only looked at for the few POST requests that read, see AddDnaAllowed()
     */
    SRequestVerdict Check( const std::string& method, const std::string& url, const std::string& body = std::string() ) const;

    size_t GetAllowedCount( void ) const;

    private:

    bool MatchesAllowed( const std::string& hostAndPathTemplate, const std::string& hostAndRawPath, bool forPost ) const;

    private:

    std::string m_treeId;
    bool m_dnaAllowed;
    bool m_exportStartAllowed;
    bool m_dnaPostsEnabled;
    bool m_requireHttps;
    std::vector< std::string > m_hostSuffixes;
    std::vector< std::string > m_allowed;
    std::vector< std::string > m_allowedPosts;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CREQUESTPOLICY_H ? */
