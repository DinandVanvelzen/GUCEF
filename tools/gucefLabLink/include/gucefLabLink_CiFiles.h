/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_CIFILES_H
#define GUCEF_LABLINK_CIFILES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The CI configuration of a machine as files, for gucefMcpCi: ci.json (the registry with the object "gateway"),
 *  gateway.token (the gateway token of this machine, one line) and ca.crt (the authority of the gateway), in the
 *  directory ci of the MCP directory. A file is written only when its content differs, atomically (a new file next
 *  to it, moved over it).
 *
 *  gucefMcpCi reads them with CORE::ReadOwnerOnlyFile(), which takes only files of the user it runs as that nobody
 *  else may read:
 *  - Linux: the directory is mode 0700 and the files 0600. With an owner (the user of Claude Code) that exists, the
 *    directory and the files are given to that user; otherwise they stay with the user of the service. The directory
 *    is opened without following a link and the files are made, given away and moved relative to it, so a link put
 *    in their place (the MCP directory belongs to that user) is never followed.
 *  - Windows: the access of the state directory of gucefLabLink, SYSTEM and the Administrators only (owner the
 *    Administrators); a gucefMcpCi that runs as an ordinary user cannot read them. The owner is not used.
 *  - Elsewhere: owner-only access for the user of the service; the owner is not used.
 */
class CCiFiles
{
    public:

    static const char* RegistryFileName;        /**< ci.json */
    static const char* TokenFileName;           /**< gateway.token */
    static const char* CaFileName;              /**< ca.crt */

    /**
     *  @param directory    <mcp dir>/ci
     *  @param owner        the user the directory and files are given to (Linux); empty: the user of the service
     */
    CCiFiles( const GUCEF::CORE::CString& directory, const GUCEF::CORE::CString& owner );

    /**
     *  Writes the files whose content differs and gives every file and the directory the access above.
     *  @param changed  whether a file was written
     */
    bool Write( const GUCEF::CORE::CString& registry      ,
                const GUCEF::CORE::CString& gatewayToken  ,
                const GUCEF::CORE::CString& caCertificate ,
                bool& changed                             ,
                GUCEF::CORE::CString& problem             );

    /** Removes the three files; removed is whether one was there. The directory stays */
    bool Remove( bool& removed, GUCEF::CORE::CString& problem );

    const GUCEF::CORE::CString& GetDirectory( void ) const;

    /**
     *  Writes one file that only its owner may read, atomically and only when its content differs, in a directory that
     *  exists; protects it either way. Linux: mode 0600, given to the owner when that user exists (the new file is
     *  given away by its descriptor before it is moved over the old one); the file itself is never followed when it is
     *  a link. Elsewhere: owner-only access for the user of the process; the owner is not used.
     *  @param owner        the user the file is given to; empty: the user of the process
     *  @param ownerProblem set when an owner is given that cannot be used: the file then stays with the user of the process
     */
    static bool WriteOwnerOnlyFile( const GUCEF::CORE::CString& path      ,
                                    const GUCEF::CORE::CString& content   ,
                                    const GUCEF::CORE::CString& owner     ,
                                    bool& changed                         ,
                                    GUCEF::CORE::CString& ownerProblem    ,
                                    GUCEF::CORE::CString& problem         );

    private:

    GUCEF::CORE::CString m_directory;
    GUCEF::CORE::CString m_owner;
    bool m_ownerProblemReported;                /**< that the files cannot be given to the owner is said once */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_CIFILES_H ? */
