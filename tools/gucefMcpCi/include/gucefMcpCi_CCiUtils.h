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

#ifndef GUCEF_MCPCI_CCIUTILS_H
#define GUCEF_MCPCI_CCIUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

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
 *  Small helpers shared by the classes of the server: git as a child process, JSON trees, text and time
 */
class CCiUtils
{
    public:

    /**
     *  Runs git with the arguments, in 'repoDir' (git -C) when it is given. The environment holds the overrides on top of
     *  the environment of this process; GIT_TERMINAL_PROMPT=0 and GIT_OPTIONAL_LOCKS=0 are always added, so that git never
     *  waits for input and a read never rewrites the index of a repository.
     *
     *  @param output   standard output and standard error as they arrived
     *  @return true when git ran and exited with 0
     */
    static bool RunGit( const GUCEF::CORE::CString& repoDir                     ,
                        const GUCEF::CORE::CProcess::TStringVector& args        ,
                        const GUCEF::CORE::CProcess::TStringMap& environment    ,
                        GUCEF::UInt32 timeoutInMs                               ,
                        GUCEF::CORE::CString& output                            ,
                        GUCEF::Int32* exitCode = GUCEF_NULL                     );

    /** The last line of the text that is a full hexadecimal object id (40 or 64 characters), empty when there is none */
    static GUCEF::CORE::CString FindObjectId( const GUCEF::CORE::CString& text );

    /** Whether the text consists of hexadecimal digits only and has the given minimum length */
    static bool IsHex( const GUCEF::CORE::CString& text, GUCEF::UInt32 minLength );

    /** The values of an array child of a node, as strings; empty when there is no such child */
    static GUCEF::CORE::CString::StringVector GetStringArray( const GUCEF::CORE::CDataNode& node, const char* name );

    /** The elements joined with the separator */
    static GUCEF::CORE::CString Join( const GUCEF::CORE::CString::StringVector& elements, const char* separator );

    /** The text without white space at both ends */
    static GUCEF::CORE::CString Trimmed( const GUCEF::CORE::CString& text );

    /** Percent encoding of a URL path segment or query value: everything but A-Z a-z 0-9 - . _ ~ is encoded */
    static GUCEF::CORE::CString PercentEncode( const GUCEF::CORE::CString& text );

    /** As PercentEncode() but '/' is kept, for a branch name in a path */
    static GUCEF::CORE::CString PercentEncodePath( const GUCEF::CORE::CString& text );

    /** Reads a whole file. @return false when it cannot be read */
    static bool ReadFile( const GUCEF::CORE::CString& path, GUCEF::CORE::CString& content );

    /** Reads JSON text into a tree, through the JSON codec */
    static bool ParseJson( const GUCEF::CORE::CString& text, GUCEF::CORE::CDataNode& tree );

    /** Reads JSON bytes into a tree, through the JSON codec */
    static bool ParseJson( const GUCEF::CORE::CDynamicBuffer& buffer, GUCEF::CORE::CDataNode& tree );

    /** Writes a tree as JSON text */
    static bool ToJson( const GUCEF::CORE::CDataNode& tree, GUCEF::CORE::CString& text );

    /** An ISO 8601 date and time as servers send it (2026-11-22T19:30:00Z, with a fraction or an offset) as Unix seconds */
    static bool ParseIsoToUnix( const GUCEF::CORE::CString& text, GUCEF::Int64& unixSeconds );

    /** The present moment as Unix seconds */
    static GUCEF::Int64 NowUnix( void );

    /** The present UTC moment as YYYYMMDDTHHMMSSZ, the form used in names */
    static GUCEF::CORE::CString FormatCompactUtcNow( void );

    /** YYYYMMDDTHHMMSSZ as Unix seconds, the inverse of FormatCompactUtcNow() */
    static bool ParseCompactUtcToUnix( const GUCEF::CORE::CString& text, GUCEF::Int64& unixSeconds );

    /** A duration for a person: 45s, 5m12s, 2h03m */
    static GUCEF::CORE::CString FormatDuration( GUCEF::UInt64 seconds );

    /** A size for a person: 512 B, 3.4 KiB, 12.0 MiB */
    static GUCEF::CORE::CString FormatBytes( GUCEF::UInt64 bytes );

    /** The text cut to at most maxChars bytes at a line end where possible, with a note when it was cut */
    static GUCEF::CORE::CString Bound( const GUCEF::CORE::CString& text, GUCEF::UInt32 maxChars );

    /** The last maxLines lines of the text, skipping 'skipFromEnd' lines at the very end first */
    static GUCEF::CORE::CString TailLines( const GUCEF::CORE::CString& text, GUCEF::UInt32 maxLines, GUCEF::UInt32 skipFromEnd, GUCEF::UInt32& totalLines );

    /** Lower case letters and digits, with '-' for anything else: a name that is safe in a branch or file name */
    static GUCEF::CORE::CString SafeName( const GUCEF::CORE::CString& text, GUCEF::UInt32 maxLength );

    /** Milliseconds since a MT::PrecisionTickCount() */
    static GUCEF::UInt64 MilliSecsSince( GUCEF::UInt64 startTicks );

    /** A monotonic clock in milliseconds, for timers and ages: not the time of day */
    static GUCEF::UInt64 MonotonicMs( void );

    /** Whether the path is absolute on this platform */
    static bool IsAbsolute( const GUCEF::CORE::CString& path );

    /** The path made absolute against the current working directory */
    static GUCEF::CORE::CString MakeAbsolute( const GUCEF::CORE::CString& path );

    private:

    CCiUtils( void );   /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIUTILS_H ? */
