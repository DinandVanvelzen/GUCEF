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

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLFILE_H
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLDOCUMENT_H
#include "assetpluginUNITY_CUnityYamlDocument.h"
#define GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLDOCUMENT_H
#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLDOCUMENT_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The documents of a Unity YAML file in the order of the file, found by their file ID. The file owns them; it is
 *  not copyable. When two documents have the same file ID, the first is the one that is found.
 */
class GUCEF_ASSETPLUGIN_UNITY_EXPORT_CPP CUnityYamlFile
{
    public:

    typedef GUCEF::vector< const CUnityYamlDocument* > TDocumentList;

    CUnityYamlFile( void );

    ~CUnityYamlFile();

    void Clear( void );

    /** Appends a document with the facts of its header; the document to fill */
    CUnityYamlDocument& AddDocument( Int32 classId, Int64 fileId, bool stripped );

    UInt32 GetDocumentCount( void ) const;

    const CUnityYamlDocument& GetDocument( UInt32 index ) const;

    /** The document with the file ID, NULL when there is none */
    const CUnityYamlDocument* FindDocument( Int64 fileId ) const;

    /** The first document of the class, NULL when there is none */
    const CUnityYamlDocument* FindFirstOfClass( Int32 classId ) const;

    /** The documents of the class, in the order of the file */
    void GetDocumentsOfClass( Int32 classId, TDocumentList& documents ) const;

    /** The number of file IDs that more than one document has */
    UInt32 GetDuplicateIdCount( void ) const;

    private:

    CUnityYamlFile( const CUnityYamlFile& src );            /**< not implemented: it owns its documents */
    CUnityYamlFile& operator=( const CUnityYamlFile& src ); /**< not implemented: it owns its documents */

    typedef GUCEF::vector< CUnityYamlDocument* > TDocumentVector;
    typedef GUCEF::map< Int64, UInt32 > TIndexMap;

    TDocumentVector m_documents;
    TIndexMap m_index;
    UInt32 m_duplicates;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNITY */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNITY_CUNITYYAMLFILE_H ? */
