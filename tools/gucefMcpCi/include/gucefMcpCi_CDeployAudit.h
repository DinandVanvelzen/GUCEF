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

#ifndef GUCEF_MCPCI_CDEPLOYAUDIT_H
#define GUCEF_MCPCI_CDEPLOYAUDIT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#include "gucefMcpCi_CCiDeployResult.h"

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
 *  The audit of deployments: one JSON object per line for every deploy, rollback and refusal, appended to
 *  <directory>/deploy.jsonl:
 *
 *      { "time": "2026-11-23T10:00:00Z", "action": "deploy", "caller": "<machine id>", "ci": "...", "image": "...",
 *        "target": "...", "namespace": "...", "deployment": "...", "run": "...", "commit": "...", "digest": "sha256:...",
 *        "oldDigest": "sha256:...", "result": "deployed|refused|rollout_failed|...", "code": "...", "reason": "..." }
 *
 *  When the file passes its size limit it becomes deploy.jsonl.1 (the older ones move up to .3, the oldest goes), so
 *  the audit stays bounded. No credential is ever part of a line.
 *
 *  Thread-safe.
 */
class CDeployAudit
{
    public:

    static const char* FileName;            /**< deploy.jsonl */

    CDeployAudit( void );

    void SetDirectory( const GUCEF::CORE::CString& directory );

    /** The size after which the file is rotated; default 4 MiB */
    void SetMaxFileBytes( GUCEF::UInt64 maxBytes );

    GUCEF::CORE::CString GetFilePath( void ) const;

    /** A secret that never is part of a line; call before the audit is used */
    void AddSecret( const GUCEF::CORE::CString& secret );

    /** Appends the line of the result. @return false when it could not be written, which is logged */
    bool Record( const CCiDeployResult& result, const GUCEF::CORE::CString& callerMachineId );

    private:

    void RotateIfNeeded( const GUCEF::CORE::CString& path );

    GUCEF::CORE::CString m_directory;
    GUCEF::UInt64 m_maxFileBytes;
    GUCEF::CORE::CRedactor m_redactor;
    mutable GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CDEPLOYAUDIT_H ? */
