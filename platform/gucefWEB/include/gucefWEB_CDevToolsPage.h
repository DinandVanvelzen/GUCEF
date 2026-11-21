/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CDEVTOOLSPAGE_H
#define GUCEF_WEB_CDEVTOOLSPAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#ifndef GUCEF_WEB_CCONTROLINFO_H
#include "gucefWEB_CControlInfo.h"
#define GUCEF_WEB_CCONTROLINFO_H
#endif /* GUCEF_WEB_CCONTROLINFO_H ? */

#ifndef GUCEF_WEB_CNAVIGATIONGUARD_H
#include "gucefWEB_CNavigationGuard.h"
#define GUCEF_WEB_CNAVIGATIONGUARD_H
#endif /* GUCEF_WEB_CNAVIGATIONGUARD_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Drives one page (tab) of a browser over a session of a client that is connected to the browser itself: navigate,
 *  click, type into a field, choose an option, read an attribute, wait for an element, list the controls. Generic: no
 *  site, no flow logic, no deny-list of control texts (the flow runner has those). See the site discovery design
 *  section 12.1.
 *
 *  Selectors are CSS selectors of the main frame's document, or "aria/<accessible name>" for the first control whose
 *  accessible name, as the browser computes it, is exactly that name. Elements are found and clicked through the DOM,
 *  Accessibility and Input domains of the protocol; a script runs in the page only where the protocol has no command
 *  (choosing an option, clearing a field, waiting for a change of the DOM), always a fixed function with the caller's
 *  text as an argument, never spliced into script text.
 *
 *  Every member waits for the browser's answers by pumping the client's pulse generator, so it is refused on any thread
 *  but the one that drives it. The session id of the client is put back after every command. Not thread-safe.
 */
class GUCEF_WEB_PUBLIC_CPP CDevToolsPage
{
    public:

    static const CORE::CString AriaSelectorPrefix;      /**< "aria/" */
    static const UInt32 MaxControls;                    /**< 1000: ListControls() reports no more */

    /** @param sessionId a session of the client on a tab, for example of AttachToTarget(); empty: set it later */
    explicit CDevToolsPage( CChromeDevToolsClient& client, const CORE::CString& sessionId = CORE::CString::Empty );

    virtual ~CDevToolsPage();

    /** Attaches to a tab (Target.attachToTarget, flatten) and uses that session from then on */
    bool AttachToTarget( const CORE::CString& targetId );

    /** Lets the page believe it has the focus of the window (Emulation.setFocusEmulationEnabled), as a page the person is looking at has */
    bool SetFocusEmulation( bool enabled );

    void SetSessionId( const CORE::CString& sessionId );

    const CORE::CString& GetSessionId( void ) const;

    /** A guard that Navigate() asks first; GUCEF_NULL for none. Not owned */
    void SetNavigationGuard( CNavigationGuard* guard );

    CNavigationGuard* GetNavigationGuard( void ) const;

    /** How long each command waits for its answer, 30 s by default */
    void SetCommandTimeout( UInt32 timeoutInMs );

    /**
     *  Whether ListControls() reports the accessible name (and the option labels of a select) of a control inside a table
     *  cell, true by default. The plain text of a cell is never reported; false withholds the names as well.
     */
    void SetReportTableCellControlNames( bool report );

    bool GetReportTableCellControlNames( void ) const;

    /**
     *  How long Navigate() waits for the new document, 30 s by default: the browser has to commit it (Page.frameNavigated
     *  of the main frame) and load it within that time. 0: Navigate() returns as soon as the browser answers
     *  Page.navigate, which is before the commit
     */
    void SetNavigationTimeout( UInt32 timeoutInMs );

    UInt32 GetNavigationTimeout( void ) const;

    /**
     *  Page.navigate, then waits (see SetNavigationTimeout()) until the browser has committed the new document of the main
     *  frame and loaded it. Refused without sending anything when the guard does not allow the address.
     *
     *  True: the new document is committed, so GetUrl() reports the new address. A document that did not load within the
     *  time still gives true, with GetLastNavigationLoaded() false. A navigation within the document (a fragment) is
     *  committed when the browser reports it (Page.navigatedWithinDocument), with nothing to load.
     *  False: refused by the guard, the browser reports an error (errorText, for example net::ERR_ABORTED of a response
     *  without content or a download), no commit within the time, or the connection or session ended. The reason is in
     *  GetLastNavigationError()
     */
    bool Navigate( const CORE::CString& url );

    /**
     *  Whether the latest Navigate() saw the new document load: its load event, or when the time ran out its
     *  DOMContentLoaded (the DOM is complete, only sub resources are pending). False after a navigation timeout of 0
     */
    bool GetLastNavigationLoaded( void ) const;

    /** Why the latest Navigate() gave false, empty when it gave true. Never holds the address */
    const CORE::CString& GetLastNavigationError( void ) const;

    /**
     *  The address the tab shows (Page.getNavigationHistory), query included: the caller's to keep out of logs. After a
     *  Navigate() that gave true this is the committed address of the new document
     */
    bool GetUrl( CORE::CString& url );

    /** A trusted mouse click at the centre of the element, after scrolling it into view */
    bool Click( const CORE::CString& selector );

    /** Click() on the match of the selector at the index (0: the first), in document order: for a control that every row of a list has */
    bool ClickNth( const CORE::CString& selector, UInt32 index );

    /** The number of elements the selector matches: for an aria selector the controls with the name (not the text inside them) */
    bool CountMatches( const CORE::CString& selector, UInt32& count );

    /**
     *  A trusted click on the menu item (role menuitem, menuitemradio or menuitemcheckbox, also inside shadow roots) whose name holds the text,
     *  case ignored, that a person can see: laid out inside the window, not hidden and not covered. A hidden copy of a menu, or the menu of another
     *  row that is not open, is never clicked. Nothing is scrolled, since a scroll closes an open menu. False when no such item is there
     */
    bool ClickMenuItem( const CORE::CString& nameContains );

    /**
     *  A GET of the address from inside the page, as the page's own scripts make their requests by default: cookies only to the page's own origin, and the headers given
     *  (for example those of a request the page made itself), from a world of the driver's own. The text of the answer is given when it is at most
     *  maxBytes long, else only the status. The headers and the answer are never logged. False when the page could not make the request at all
     */
    bool FetchInPage( const CORE::CString& url, const CORE::CString::StringMap& headers, UInt32 maxBytes, UInt32 timeoutInMs, Int32& status, CORE::CString& body );

    /** Replaces the content of a field as typing would. Refused for password fields; the text is never logged */
    bool TypeInto( const CORE::CString& selector, const CORE::CString& text );

    /** On a select element: the option whose label equals 'option' (whitespace collapsed), else whose value does */
    bool SelectOption( const CORE::CString& selector, const CORE::CString& option );

    /**
     *  The value of an attribute, for example the target of a link. False when the element or the attribute is missing,
     *  or the value is longer than maxBytes. Refuses the value attribute of input and textarea elements
     */
    bool GetElementAttribute( const CORE::CString& selector, const CORE::CString& name, CORE::CString& value, UInt32 maxBytes = 2048 );

    /** Is there such an element right now */
    bool ElementExists( const CORE::CString& selector );

    /**
     *  Waits until the element exists, without polling: after each check it waits for the next change of the page's
     *  DOM (at most one wake-up per 100 ms) or the time that is left
     */
    bool WaitForElement( const CORE::CString& selector, UInt32 timeoutInMs );

    /**
     *  The controls in the scope (a CSS selector; empty: the whole document) in document order, at most MaxControls:
     *  role, accessible label, visible, enabled, and for a select element the labels of its options. Never input
     *  values, never password fields, and nothing of the text of table cells, see CControlInfo.
     *
     *  A page that is still rendering can remove a control while it is listed: a control whose element is gone, or whose
     *  description, options or boxes the browser no longer gives, is left out and counted, see
     *  GetLastListSkippedCount(). When the scope itself is gone (the document changed, so its node ids are stale) the
     *  listing starts once more on a fresh document. False: the scope is not there (also not on the fresh document), or
     *  the connection or session ended
     */
    bool ListControls( const CORE::CString& scopeSelector, TControlInfoVector& result );

    /** How many controls the latest ListControls() left out because they went away while it listed them */
    UInt32 GetLastListSkippedCount( void ) const;

    private:

    CDevToolsPage( const CDevToolsPage& src );                  /**< not implemented */
    CDevToolsPage& operator=( const CDevToolsPage& src );       /**< not implemented */

    enum EListOutcome
    {
        LIST_DONE   ,
        LIST_STALE  ,   /**< the scope or the document went away: worth one more try on a fresh document */
        LIST_FAILED
    };

    bool Send( const CORE::CString& method, const CORE::CDataNode* params, CORE::CDataNode& result, bool logFailure = true, COM::SJsonRpcError* error = GUCEF_NULL );
    bool IsSessionLost( const COM::SJsonRpcError& error ) const;
    EListOutcome ListControlsOnce( const CORE::CString& scopeSelector, TControlInfoVector& result, UInt32& skipped );
    bool NavigateWithoutWait( const CORE::CString& url );
    void SetNavigationError( const CORE::CString& url, const CORE::CString& reason );
    bool GetDocumentRoot( Int64& rootNodeId, COM::SJsonRpcError* error = GUCEF_NULL );
    bool Resolve( const CORE::CString& selector, Int64& nodeId, bool logMissing = true, COM::SJsonRpcError* error = GUCEF_NULL, UInt32 index = 0 );
    /** The centre of the first part of the element with an area, after scrolling it into view; false when it has none (hidden, not laid out yet) */
    bool GetClickPoint( Int64 nodeId, Float64& x, Float64& y );
    /** A trusted left click (move, press, release) at the point in the coordinates of the window */
    bool ClickAt( Float64 x, Float64 y );
    /** The backend ids of the matches of a selector, in document order: for an aria selector the controls, and the first text node with the name apart */
    bool FindMatches( const CORE::CString& selector, GUCEF::vector< Int64 >& ids, Int64& textBackendNodeId, bool& backendIds, COM::SJsonRpcError* error );
    bool DescribeNode( Int64 nodeId, CORE::CString& nodeName, CORE::CString::StringMap& attributes, COM::SJsonRpcError* error = GUCEF_NULL );
    bool CallOnNode( Int64 nodeId, const CORE::CString& functionDeclaration, const CORE::CString* argument, CORE::CString& returned );
    bool WaitForDomChange( UInt32 timeoutInMs );
    bool GetIsolatedContext( Int64& contextId );

    CChromeDevToolsClient& m_client;
    CORE::CString m_sessionId;
    CNavigationGuard* m_guard;
    UInt32 m_commandTimeoutInMs;
    bool m_reportTableCellControlNames;
    Int64 m_isolatedContextId;
    UInt32 m_navigationTimeoutInMs;
    bool m_pageEnabled;
    bool m_lastNavigationLoaded;
    CORE::CString m_lastNavigationError;
    UInt32 m_lastListSkippedCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CDEVTOOLSPAGE_H ? */
