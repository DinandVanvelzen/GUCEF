/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_MCPCI_CCIREGISTRY_H
#define GUCEF_MCPCI_CCIREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMcpCi_CCiDefinition.h"
#include "gucefMcpCi_CCiGatewayEndpoint.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The repositories the CI knows: the entries of ci.json, an object { "ci": [ entry, ... ] } or a plain array of
 *  entries (see CCiDefinition). Only these are ever built: a repository whose remotes match none has no CI.
 *  The object form may also name the gateway of proxy mode: { "ci": [ ... ], "gateway": { ... } }, see
 *  CCiGatewayEndpoint.
 */
class CCiRegistry
{
    public:

    typedef GUCEF::vector< CCiDefinition > TDefinitionVector;

    CCiRegistry( void );

    /** Replaces the entries with those of the JSON text */
    bool LoadFromJson( const GUCEF::CORE::CString& jsonText, GUCEF::CORE::CString& problem );

    /**
     *  Replaces the entries with those of a file. With ownerOnly the file is read with CORE::ReadOwnerOnlyFile(), which
     *  refuses a file that others than its owner can write or read, as for files that lablink provides.
     */
    bool LoadFromFile( const GUCEF::CORE::CString& path, bool ownerOnly, GUCEF::CORE::CString& problem );

    const CCiDefinition* FindById( const GUCEF::CORE::CString& ciId ) const;

    /**
     *  The first entry with a remote that matches one of the given remotes after normalisation, GUCEF_NULL when none
     *  does. 'matchedRemote' receives the given remote that matched.
     */
    const CCiDefinition* FindByRemotes( const GUCEF::CORE::CString::StringVector& remotes, GUCEF::CORE::CString& matchedRemote ) const;

    const TDefinitionVector& GetDefinitions( void ) const;

    bool IsEmpty( void ) const;

    /** The gateway ci.json names, not set when it names none */
    const CCiGatewayEndpoint& GetGateway( void ) const;

    /** The CI ids, comma separated */
    GUCEF::CORE::CString ListIds( void ) const;

    /**
     *  The form in which remote URLs are compared: host (lower case, without user and port) and path (without a
     *  leading or trailing / and without .git), as "host/path". Understands https://, http://, ssh://, git://,
     *  scp style user@host:path and file:// URLs and local paths (as "file:" and the path). Empty for empty input.
     *
     *      https://github.com/Org/Repo.git   -> github.com/Org/Repo
     *      git@GitHub.com:Org/Repo.git       -> github.com/Org/Repo
     *      ssh://git@host:2222/org/repo      -> host/org/repo
     */
    static GUCEF::CORE::CString NormalizeRemoteUrl( const GUCEF::CORE::CString& url );

    private:

    TDefinitionVector m_definitions;
    CCiGatewayEndpoint m_gateway;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIREGISTRY_H ? */
