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

#ifndef GUCEF_GUI_CHELPVIEWERFORM_H
#define GUCEF_GUI_CHELPVIEWERFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CIHELPSOURCE_H
#include "gucefGUI_CIHelpSource.h"
#define GUCEF_GUI_CIHELPSOURCE_H
#endif /* GUCEF_GUI_CIHELPSOURCE_H ? */

#ifndef GUCEF_GUI_CHELPHISTORY_H
#include "gucefGUI_CHelpHistory.h"
#define GUCEF_GUI_CHELPHISTORY_H
#endif /* GUCEF_GUI_CHELPHISTORY_H ? */

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
 *  A reader of help documents from a CIHelpSource: the title of the topic, its text, the topics it links to, an index of all
 *  topics, and back and forward over the topics read. The content and its ids are the application's.
 *
 *  Widgets: Back, Forward, TopicTitle; Topics (the index: a click opens a topic); Document (the text of the topic, read only,
 *  wrapped); LinksTitle and Links (see also and the links of the text: a click, a double click or Enter opens one); Reviewed
 *  (version and review date of the topic); Close. Escape closes, Alt+Left and Alt+Right go back and forward.
 *
 *  The document is shown as plain text (CHelpDocument::ToPlainText()) and its links as a list beside it, so every driver that
 *  draws a text box and a list shows it and every link is reachable by the keyboard. A topic the source does not have shows
 *  "Not found" with its id and is not added to the history.
 *
 *  The form builds its layout in code, as a window that opens modal (Build( title ), then Open()) or in the place of a widget of
 *  another form (Build( placeholder )). A viewer made with GUCEF_NEW is handed to the context with CGUIContext::AdoptForm(), as the
 *  column chooser of the grid is.
 */
class GUCEF_GUI_PUBLIC_CPP CHelpViewerForm : public CFormEx
{
    public:

    static const CORE::CEvent TopicShownEvent;      /**< TCloneableString: the id of the topic shown */
    static const CORE::CEvent ClosedEvent;

    static void RegisterEvents( void );

    typedef GUCEF::vector< CString > TStringVector;

    CHelpViewerForm( void );

    explicit CHelpViewerForm( CIGUIContext* context );

    virtual ~CHelpViewerForm();

    /** Makes the form as a window of the context, hidden until Open(); false when it has no context */
    bool Build( const CString& title = "Help" );

    /** Makes the form in the place of 'placeholder', which it fills */
    bool Build( CWidget* placeholder );

    void SetHelpSource( const THelpSourcePtr& source );

    const THelpSourcePtr& GetHelpSource( void ) const;

    /** Shows the topic; the window opens modal when the form is a window. True when the viewer shows (the topic may be not found) */
    bool Open( const CString& topicId );

    /** Shows the topic in the open viewer and adds it to the history; false when the source has no such topic */
    bool ShowTopic( const CString& topicId );

    bool GoBack( void );

    bool GoForward( void );

    /** Opens the link with the index in GetLinkTopics() */
    bool OpenLink( UInt32 index );

    /** Opens the entry with the index in GetIndex() */
    bool OpenIndexEntry( UInt32 index );

    void Close( void );

    bool IsOpen( void ) const;

    /** The id of the topic shown (also when it was not found) */
    const CString& GetCurrentTopic( void ) const;

    bool IsCurrentTopicFound( void ) const;

    const CHelpDocument& GetCurrentDocument( void ) const;

    /** The text the Document box shows */
    const CString& GetDocumentText( void ) const;

    /** The ids the Links list shows, in its order */
    const TStringVector& GetLinkTopics( void ) const;

    /** The topics the Topics list shows, in its order */
    const CIHelpSource::TTopicEntryVector& GetIndex( void ) const;

    CHelpHistory& GetHistory( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CHelpViewerForm > TEventCallback;

    void OnBackActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnForwardActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTopicsSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLinksClicked( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLinksSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLinksActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyDown( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout, bool asWindow, const CString& title );
    void LoadTopic( const CString& topicId );
    void ShowCurrent( void );
    void FillIndex( void );
    Int32 GetListSelection( const CString& listName );

    CHelpViewerForm( const CHelpViewerForm& src );
    CHelpViewerForm& operator=( const CHelpViewerForm& src );

    private:

    THelpSourcePtr m_source;
    CHelpHistory m_history;
    CHelpDocument m_document;
    CString m_currentTopic;
    bool m_found;
    CString m_text;
    TStringVector m_links;
    CIHelpSource::TTopicEntryVector m_index;
    bool m_open;
    bool m_asWindow;
    bool m_updatingWidgets;
    bool m_linkClickPending;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CHELPVIEWERFORM_H ? */
