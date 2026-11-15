/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#include "gucefASSETSTORE_CAssetContentAnalyzerRegistry.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetContentAnalyzerRegistry::CAssetContentAnalyzerRegistry( void )
    : m_analyzers()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetContentAnalyzerRegistry::~CAssetContentAnalyzerRegistry()
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_analyzers.clear();
}

/*-------------------------------------------------------------------------*/

bool
CAssetContentAnalyzerRegistry::Register( const TAssetContentAnalyzerPtr& analyzer )
{GUCEF_TRACE;

    if ( analyzer.IsNULL() || analyzer->GetName().IsNULLOrEmpty() )
        return false;
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_analyzers.size(); ++i )
        if ( m_analyzers[ i ]->GetName() == analyzer->GetName() )
            return false;
    m_analyzers.push_back( analyzer );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetContentAnalyzerRegistry::Unregister( const CORE::CString& name )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_analyzers.size(); ++i )
    {
        if ( m_analyzers[ i ]->GetName() == name )
        {
            m_analyzers.erase( m_analyzers.begin() + i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAssetContentAnalyzerRegistry::IsRegistered( const CORE::CString& name ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_analyzers.size(); ++i )
        if ( m_analyzers[ i ]->GetName() == name )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAssetContentAnalyzerRegistry::GetAnalyzersFor( const CAssetRecord& asset, TAssetContentAnalyzerPtrVector& analyzers ) const
{GUCEF_TRACE;

    analyzers.clear();
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_analyzers.size(); ++i )
        if ( m_analyzers[ i ]->CanAnalyze( asset ) )
            analyzers.push_back( m_analyzers[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CAssetContentAnalyzerRegistry::GetNames( CORE::CString::StringVector& names ) const
{GUCEF_TRACE;

    names.clear();
    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_analyzers.size(); ++i )
        names.push_back( m_analyzers[ i ]->GetName() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
