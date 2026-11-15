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

#ifndef GUCEF_GUI_CIHELPSOURCE_H
#define GUCEF_GUI_CIHELPSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_GUI_CHELPDOCUMENT_H
#include "gucefGUI_CHelpDocument.h"
#define GUCEF_GUI_CHELPDOCUMENT_H
#endif /* GUCEF_GUI_CHELPDOCUMENT_H ? */

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
 *  Where help content comes from: documents by topic id, the list of topics for an index, and brief texts by id for
 *  tooltips. The ids and the content are the application's; the tooltip service of a context (CTooltipService) and the
 *  help viewer (CHelpViewerForm) only ask. A brief id is either the id of a document (its brief) or the id of a short text
 *  of its own, which may name the document that explains it further.
 *
 *  Called on the thread of the GUI, synchronously: a source that is slow to read keeps what it serves in memory. Made with
 *  GUCEF_NEW and handed around as THelpSourcePtr ( ( GUCEF_NEW CHelpCatalogue() )->CreateSharedPtr() ).
 */
class GUCEF_GUI_PUBLIC_CPP CIHelpSource : public CORE::CTSharedPtrCreator< CIHelpSource, MT::CMutex >
{
    public:

    typedef CORE::CTSharedPtr< CIHelpSource, MT::CMutex > THelpSourcePtr;

    /** A topic as an index lists it */
    struct GUCEF_GUI_PUBLIC_CPP STopicEntry
    {
        CString id;
        CString title;
        CString group;

        STopicEntry( void );
        STopicEntry( const CString& topicId, const CString& topicTitle, const CString& topicGroup );
    };
    typedef struct STopicEntry TTopicEntry;
    typedef GUCEF::vector< TTopicEntry > TTopicEntryVector;

    CIHelpSource( void );

    virtual ~CIHelpSource();

    /** The document of the topic; false when the source has none */
    virtual bool GetTopic( const CString& topicId, CHelpDocument& document ) const = 0;

    /** Every topic, ordered by group and then by title */
    virtual void ListTopics( TTopicEntryVector& topics ) const = 0;

    /**
     *  The brief text of an id: a short text of its own, else the brief of the document with that id (topicId is then the id).
     *  'topicId' is the document a reader can open from the brief, empty when there is none. False when the id is unknown
     */
    virtual bool GetBrief( const CString& id, CString& text, CString& topicId ) const = 0;

    /** Whether the source has a document of the topic; by default GetTopic() */
    virtual bool HasTopic( const CString& topicId ) const;

    /** The title of the topic; the id itself when the source has no document of it */
    virtual CString GetTopicTitle( const CString& topicId ) const;

    private:

    CIHelpSource( const CIHelpSource& src );
    CIHelpSource& operator=( const CIHelpSource& src );
};

/*-------------------------------------------------------------------------*/

typedef CIHelpSource::THelpSourcePtr THelpSourcePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CIHELPSOURCE_H ? */
