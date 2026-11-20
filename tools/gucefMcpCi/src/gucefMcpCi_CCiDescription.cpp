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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "gucefMcpCi_CCiUtils.h"
#include "gucefMcpCi_CCiDescription.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString::StringVector& list, const CORE::CString& value )
{
    for ( size_t i=0; i<list.size(); ++i )
        if ( list[ i ] == value )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** The values of 'values' that are not in the allowed ones: 0 toolchains, 1 C++ standards, 2 products */
static CORE::CString::StringVector
Unknown( const CORE::CString::StringVector& values, const CCiDescription& description, int kind )
{
    CORE::CString::StringVector unknown;
    for ( size_t i=0; i<values.size(); ++i )
    {
        const bool known = 0 == kind ? description.HasToolchain( values[ i ] ) : 1 == kind ? description.HasCppStandard( values[ i ] ) : description.HasProduct( values[ i ] );
        if ( !known )
            unknown.push_back( values[ i ] );
    }
    return unknown;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCiDescription::CToolchain::CToolchain( void )
    : id()
    , platform()
    , compiler()
    , arch()
    , enabled( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCiDescription::CCiDescription( void )
    : ciId()
    , source()
    , toolchains()
    , cppStandards()
    , buildTypes()
    , products()
    , modes()
    , defaultCellCount( 0 )
    , hasOptions( false )
{GUCEF_TRACE;

    modes.push_back( "build" );
    modes.push_back( "test" );
}

/*-------------------------------------------------------------------------*/

bool
CCiDescription::HasToolchain( const CORE::CString& id ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<toolchains.size(); ++i )
        if ( toolchains[ i ].id == id )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CCiDescription::HasCppStandard( const CORE::CString& standard ) const
{GUCEF_TRACE;

    return Contains( cppStandards, standard );
}

/*-------------------------------------------------------------------------*/

bool
CCiDescription::HasProduct( const CORE::CString& product ) const
{GUCEF_TRACE;

    return Contains( products, product );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCiDescription::ListEnabledToolchains( void ) const
{GUCEF_TRACE;

    CORE::CString text;
    for ( size_t i=0; i<toolchains.size(); ++i )
    {
        if ( !toolchains[ i ].enabled )
            continue;
        if ( !text.IsNULLOrEmpty() )
            text += ", ";
        text += toolchains[ i ].id;
    }
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CCiDescription::LoadFromMatrixFiles( const CORE::CString& matrixDir, bool& found, CORE::CString& error )
{GUCEF_TRACE;

    found = false;
    const CORE::CString matrixPath = CORE::CombinePath( matrixDir, "matrix.json" );
    CORE::CString text;
    CORE::CDataNode matrix;
    if ( !CCiUtils::ReadFile( matrixPath, text ) )
        return true;
    found = true;
    if ( !CCiUtils::ParseJson( text, matrix ) )
    {
        error = "The matrix file of the repository is not JSON: " + matrixPath;
        return false;
    }
    cppStandards = CCiUtils::GetStringArray( matrix, "cppStandards" );
    buildTypes = CCiUtils::GetStringArray( matrix, "buildTypes" );
    toolchains.clear();
    const CORE::CDataNode* toolchainList = matrix.FindChild( "toolchains" );
    if ( GUCEF_NULL != toolchainList )
    {
        for ( CORE::CDataNode::const_iterator i = toolchainList->ConstBegin(); i != toolchainList->ConstEnd(); ++i )
        {
            CToolchain toolchain;
            toolchain.id = CCiUtils::Trimmed( (*i)->GetAttributeValue( "id" ).AsString() );
            toolchain.platform = (*i)->GetAttributeValue( "platform" ).AsString();
            toolchain.compiler = (*i)->GetAttributeValue( "compiler" ).AsString();
            toolchain.arch = (*i)->GetAttributeValue( "arch" ).AsString();
            toolchain.enabled = (*i)->GetAttributeValue( "enabled" ).AsBool( true );
            if ( !toolchain.id.IsNULLOrEmpty() )
                toolchains.push_back( toolchain );
        }
    }
    const CORE::CDataNode* cells = matrix.FindChild( "cells" );
    if ( GUCEF_NULL != cells )
    {
        defaultCellCount = cells->GetNrOfChildNodes();
    }
    else
    {
        UInt32 enabled = 0;
        for ( size_t i=0; i<toolchains.size(); ++i )
            if ( toolchains[ i ].enabled )
                ++enabled;
        defaultCellCount = enabled * (UInt32) cppStandards.size() * (UInt32) ( buildTypes.empty() ? 1 : buildTypes.size() );
    }

    products.clear();
    CORE::CDataNode productTree;
    if ( CCiUtils::ReadFile( CORE::CombinePath( matrixDir, "products.json" ), text ) && CCiUtils::ParseJson( text, productTree ) )
    {
        const CORE::CDataNode* list = productTree.FindChild( "products" );
        if ( GUCEF_NULL != list )
        {
            CORE::CString::StringSet seen;
            for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
            {
                const CORE::CString name = CCiUtils::Trimmed( (*i)->GetAttributeValue( "name" ).AsString() );
                if ( !name.IsNULLOrEmpty() && seen.find( name ) == seen.end() )
                {
                    seen.insert( name );
                    products.push_back( name );
                }
            }
        }
    }
    hasOptions = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCiDescription::CheckFilters( const CCiSubmitOptions& options, const CORE::CString& ciId, CORE::CString& error ) const
{GUCEF_TRACE;

    if ( !hasOptions || ( options.toolchains.empty() && options.cppStandards.empty() && options.products.empty() ) )
        return true;        // the CI does not say: passed on as they are

    const CORE::CString::StringVector unknownToolchains = toolchains.empty() ? CORE::CString::StringVector() : Unknown( options.toolchains, *this, 0 );
    const CORE::CString::StringVector unknownStandards = cppStandards.empty() ? CORE::CString::StringVector() : Unknown( options.cppStandards, *this, 1 );
    const CORE::CString::StringVector unknownProducts = products.empty() ? CORE::CString::StringVector() : Unknown( options.products, *this, 2 );
    if ( unknownToolchains.empty() && unknownStandards.empty() && unknownProducts.empty() )
        return true;
    error = "Unknown filter values for CI id " + ciId + ":";
    if ( !unknownToolchains.empty() )
    {
        CORE::CString::StringVector ids;
        for ( size_t i=0; i<toolchains.size(); ++i )
            ids.push_back( toolchains[ i ].id );
        error += " toolchains " + CCiUtils::Join( unknownToolchains, ", " ) + " (known: " + CCiUtils::Join( ids, ", " ) + ");";
    }
    if ( !unknownStandards.empty() )
        error += " cpp_standards " + CCiUtils::Join( unknownStandards, ", " ) + " (known: " + CCiUtils::Join( cppStandards, ", " ) + ");";
    if ( !unknownProducts.empty() )
        error += " products " + CCiUtils::Join( unknownProducts, ", " ) + " (known: " + CCiUtils::Join( products, ", " ) + ");";
    error += " Leave a filter out to build everything.";
    return false;
}

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/
