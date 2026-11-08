/*
 *  gucefMcpLocalLlm: MCP server that delegates coding tasks to local LLMs
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

#ifndef GUCEF_MCPLOCALLLM_BUILDLOGDIGEST_H
#define GUCEF_MCPLOCALLLM_BUILDLOGDIGEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <string>
#include <vector>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpLocalLlm {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reduces the output of builds and test runs to what a developer acts on: the distinct errors and warnings with
 *  their location, failed build targets and failed tests. Recognises:
 *
 *    GCC and Clang       file:line:col: error|warning|fatal error: message
 *    MSVC and MSBuild    file(line,col): error|warning C1234: message, LNK errors
 *    linkers             undefined reference to `symbol', ld and collect2 errors
 *    CMake               CMake Error|Warning at file:line (command): with its indented message
 *    ninja and make      FAILED: target, make: *** [target] Error N
 *    CTest               the list of failed tests and the "tests passed" summary
 *    others              lines starting with "error:" or "ERROR:"
 *
 *  Repeated diagnostics are counted once. Paths under the base directory, or under its parent or grandparent, are
 *  shown relative to it: src/a.cpp, ../src/a.cpp.
 */
class CBuildLogDigest
{
    public:

    struct SIssue
    {
        std::string location;           /**< file:line[:col], empty when the tool gave none */
        std::string message;
        GUCEF::UInt32 count;

        SIssue( void ) : location(), message(), count( 0 ) {}
    };

    typedef std::vector< SIssue > TIssueVector;
    typedef std::vector< std::string > TStringVector;

    explicit CBuildLogDigest( const GUCEF::CORE::CString& baseDir = GUCEF::CORE::CString::Empty );

    void Parse( const std::string& log );

    const TIssueVector& GetErrors( void ) const;
    const TIssueVector& GetWarnings( void ) const;
    const TStringVector& GetFailedTargets( void ) const;
    const TStringVector& GetFailedTests( void ) const;
    const std::string& GetTestSummary( void ) const;

    GUCEF::UInt32 GetErrorCount( void ) const;      /**< including repeats */
    GUCEF::UInt32 GetWarningCount( void ) const;    /**< including repeats */

    /** Errors, failed targets or failed tests were found */
    bool HasFailures( void ) const;

    /** The findings as text, with at most the given number of distinct errors and warnings */
    std::string Format( GUCEF::UInt32 maxErrors, GUCEF::UInt32 maxWarnings ) const;

    /** The last lines of a log, at most maxChars characters */
    static std::string Tail( const std::string& log, GUCEF::UInt32 maxLines, GUCEF::UInt32 maxChars );

    private:

    void ParseLine( const std::string& line, bool& consumed );
    void AddIssue( bool isError, const std::string& location, const std::string& message );
    std::string ShortenPath( const std::string& path ) const;
    void FlushCMakeMessage( void );

    typedef std::map< std::string, size_t > TIndexMap;

    std::vector< std::string > m_baseDirs;      /**< the base directory, its parent and grandparent, ending in / */
    std::vector< std::string > m_basePrefixes;  /**< what replaces each: "", "../", "../../" */
    TIssueVector m_errors;
    TIssueVector m_warnings;
    TIndexMap m_errorIndex;
    TIndexMap m_warningIndex;
    TStringVector m_failedTargets;
    TStringVector m_failedTests;
    std::string m_testSummary;
    GUCEF::UInt32 m_errorCount;
    GUCEF::UInt32 m_warningCount;

    // a CMake message being collected: its first line and the indented lines that follow
    bool m_inCMakeMessage;
    bool m_cmakeIsError;
    std::string m_cmakeLocation;
    std::string m_cmakeText;
    bool m_inFailedTests;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpLocalLlm */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPLOCALLLM_BUILDLOGDIGEST_H ? */
