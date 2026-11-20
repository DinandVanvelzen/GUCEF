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

#ifndef GUCEF_MCPCI_CCIDESCRIPTION_H
#define GUCEF_MCPCI_CCIDESCRIPTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include "gucefMcpCi_CCiSubmitOptions.h"

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

/** What a CI id can build: the values the filters of ci_submit take, and what "everything" is */
class CCiDescription
{
    public:

    class CToolchain
    {
        public:

        GUCEF::CORE::CString id;
        GUCEF::CORE::CString platform;
        GUCEF::CORE::CString compiler;
        GUCEF::CORE::CString arch;
        bool enabled;               /**< a runner exists for it: part of "everything" */

        CToolchain( void );
    };

    typedef GUCEF::vector< CToolchain > TToolchainVector;

    GUCEF::CORE::CString ciId;
    GUCEF::CORE::CString source;                        /**< where the options come from, for the reader */
    TToolchainVector toolchains;
    GUCEF::CORE::CString::StringVector cppStandards;
    GUCEF::CORE::CString::StringVector buildTypes;
    GUCEF::CORE::CString::StringVector products;
    GUCEF::CORE::CString::StringVector modes;
    GUCEF::UInt32 defaultCellCount;                    /**< cells of a run without filters, 0 when not known */
    bool hasOptions;                                    /**< false: the CI does not say, the filters are passed on unchecked */

    CCiDescription( void );

    bool HasToolchain( const GUCEF::CORE::CString& id ) const;
    bool HasCppStandard( const GUCEF::CORE::CString& standard ) const;
    bool HasProduct( const GUCEF::CORE::CString& product ) const;

    /** The enabled toolchains, comma separated */
    GUCEF::CORE::CString ListEnabledToolchains( void ) const;

    /**
     *  Reads the options from the CI files of a working tree: matrix.json and products.json in 'matrixDir'
     *  (projects/CI/scripts/ci_plan.py reads the same files). Sets hasOptions; leaves ciId and source as they are.
     *  @param found    false when there is no matrix.json, which is not an error
     *  @return false when matrix.json is there but cannot be read as JSON
     */
    bool LoadFromMatrixFiles( const GUCEF::CORE::CString& matrixDir, bool& found, GUCEF::CORE::CString& error );

    /**
     *  Checks the filters of a run against the options, when there are options; the mode is not checked here.
     *  @return false with the values that are not known in 'error'
     */
    bool CheckFilters( const CCiSubmitOptions& options, const GUCEF::CORE::CString& ciId, GUCEF::CORE::CString& error ) const;
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIDESCRIPTION_H ? */
