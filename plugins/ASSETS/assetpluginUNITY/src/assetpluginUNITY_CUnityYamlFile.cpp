/*
 *  assetpluginUNITY: the documents of a text-serialised Unity file
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

#include "assetpluginUNITY_CUnityYamlFile.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNITY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUnityYamlFile::CUnityYamlFile( void )
    : m_documents()
    , m_index()
    , m_duplicates( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUnityYamlFile::~CUnityYamlFile()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CUnityYamlFile::Clear( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_documents.size(); ++i )
        GUCEF_DELETE m_documents[ i ];
    m_documents.clear();
    m_index.clear();
    m_duplicates = 0;
}

/*-------------------------------------------------------------------------*/

CUnityYamlDocument&
CUnityYamlFile::AddDocument( Int32 classId, Int64 fileId, bool stripped )
{GUCEF_TRACE;

    CUnityYamlDocument* document = GUCEF_NEW CUnityYamlDocument();
    document->classId = classId;
    document->fileId = fileId;
    document->stripped = stripped;
    m_documents.push_back( document );
    if ( classId >= 0 )
    {
        if ( m_index.find( fileId ) == m_index.end() )
            m_index[ fileId ] = (UInt32) m_documents.size() - 1;
        else
            ++m_duplicates;
    }
    return *document;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnityYamlFile::GetDocumentCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_documents.size();
}

/*-------------------------------------------------------------------------*/

const CUnityYamlDocument&
CUnityYamlFile::GetDocument( UInt32 index ) const
{GUCEF_TRACE;

    return *m_documents[ index ];
}

/*-------------------------------------------------------------------------*/

const CUnityYamlDocument*
CUnityYamlFile::FindDocument( Int64 fileId ) const
{GUCEF_TRACE;

    TIndexMap::const_iterator i = m_index.find( fileId );
    if ( i == m_index.end() )
        return GUCEF_NULL;
    return m_documents[ (*i).second ];
}

/*-------------------------------------------------------------------------*/

const CUnityYamlDocument*
CUnityYamlFile::FindFirstOfClass( Int32 classId ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_documents.size(); ++i )
        if ( m_documents[ i ]->classId == classId )
            return m_documents[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CUnityYamlFile::GetDocumentsOfClass( Int32 classId, TDocumentList& documents ) const
{GUCEF_TRACE;

    documents.clear();
    for ( size_t i=0; i<m_documents.size(); ++i )
        if ( m_documents[ i ]->classId == classId )
            documents.push_back( m_documents[ i ] );
}

/*-------------------------------------------------------------------------*/

UInt32
CUnityYamlFile::GetDuplicateIdCount( void ) const
{GUCEF_TRACE;

    return m_duplicates;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
