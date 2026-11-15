/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CHELPCATALOGUE_H
#define GUCEF_GUI_CHELPCATALOGUE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CIHELPSOURCE_H
#include "gucefGUI_CIHelpSource.h"
#define GUCEF_GUI_CIHELPSOURCE_H
#endif /* GUCEF_GUI_CIHELPSOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A help source in memory: documents by topic id and brief texts by id, read from text in the format of CHelpMarkupParser
 *  or from a directory of the VFS: every "*.help" file is a document, every "*.briefs" file a brief table. Read once and
 *  then only asked, on the thread of the GUI.
 *
 *  Validate() checks the content as a whole: what a reader can reach must exist. An application runs it in its tests so
 *  that a broken link or a missing brief fails the build of the content, not the reader.
 */
class GUCEF_GUI_PUBLIC_CPP CHelpCatalogue : public CIHelpSource
{
    public:

    typedef GUCEF::vector< CString > TStringVector;

    static const char* DocumentExtension;       /**< "help" */
    static const char* BriefTableExtension;     /**< "briefs" */

    /** Files larger than this are not read: a damaged or foreign file */
    enum ELimits
    {
        MaxFileBytes = 1048576
    };

    CHelpCatalogue( void );

    virtual ~CHelpCatalogue();

    /** Adds the document, replacing one with the same id; false (and nothing changes) for a document without id or title */
    bool AddDocument( const CHelpDocument& document );

    /** Parses and adds a document; false with the reason, which names 'origin' (a file name) */
    bool AddDocumentText( const CString& text, const CString& origin, CString& error );

    /** Adds a brief entry, replacing one with the same id; false for an id that is not valid or an empty text */
    bool AddBrief( const CString& id, const CString& text, const CString& topicId );

    /** Parses a brief table and adds its entries; the lines it skips are described in 'problems' with 'origin'. The number added */
    UInt32 AddBriefTableText( const CString& text, const CString& origin, TStringVector& problems );

    /**
     *  Reads every document and brief table of the VFS directory (not its sub-directories), in the order of their names, through
     *  the VFS of CVfsGlobal. A file that cannot be read or parsed is described in 'problems' and the others are read. False when
     *  the directory does not exist or a file was refused
     */
    bool LoadFromVfs( const CString& vfsDirectory, TStringVector& problems );

    void Clear( void );

    UInt32 GetTopicCount( void ) const;

    UInt32 GetBriefCount( void ) const;

    /** The document of a topic, NULL when there is none; valid until the catalogue changes */
    const CHelpDocument* FindTopic( const CString& topicId ) const;

    /** The ids of the documents, in id order */
    void GetTopicIds( TStringVector& ids ) const;

    /** The ids of the brief entries, in id order */
    void GetBriefIds( TStringVector& ids ) const;

    /**
     *  Checks that a reader can reach only what exists and finds what a tooltip needs: every document has a title, a brief and
     *  content; its see also ids and inline links name documents; every brief entry has a text and its topic is a document; no id
     *  is both a document and a brief entry. Appends a line per problem; true when there is none
     */
    bool Validate( TStringVector& problems ) const;

    /*
     *  CIHelpSource
     */

    virtual bool GetTopic( const CString& topicId, CHelpDocument& document ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void ListTopics( TTopicEntryVector& topics ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetBrief( const CString& id, CString& text, CString& topicId ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool HasTopic( const CString& topicId ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetTopicTitle( const CString& topicId ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SBrief
    {
        CString text;
        CString topic;
    };

    typedef GUCEF::map< CString, CHelpDocument > TDocumentMap;
    typedef GUCEF::map< CString, SBrief > TBriefMap;

    CHelpCatalogue( const CHelpCatalogue& src );
    CHelpCatalogue& operator=( const CHelpCatalogue& src );

    TDocumentMap m_documents;
    TBriefMap m_briefs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CHELPCATALOGUE_H ? */
