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

    void SetSessionId( const CORE::CString& sessionId );

    const CORE::CString& GetSessionId( void ) const;

    /** A guard that Navigate() asks first; GUCEF_NULL for none. Not owned */
    void SetNavigationGuard( CNavigationGuard* guard );

    CNavigationGuard* GetNavigationGuard( void ) const;

    /** How long each command waits for its answer, 30 s by default */
    void SetCommandTimeout( UInt32 timeoutInMs );

    /** Page.navigate; refused without sending anything when the guard does not allow the address */
    bool Navigate( const CORE::CString& url );

    /** The address the tab shows (Page.getNavigationHistory), query included: the caller's to keep out of logs */
    bool GetUrl( CORE::CString& url );

    /** A trusted mouse click at the centre of the element, after scrolling it into view */
    bool Click( const CORE::CString& selector );

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
     *  values, never password fields, and nothing of the text of table cells, see CControlInfo
     */
    bool ListControls( const CORE::CString& scopeSelector, TControlInfoVector& result );

    private:

    CDevToolsPage( const CDevToolsPage& src );                  /**< not implemented */
    CDevToolsPage& operator=( const CDevToolsPage& src );       /**< not implemented */

    bool Send( const CORE::CString& method, const CORE::CDataNode* params, CORE::CDataNode& result, bool logFailure = true );
    bool GetDocumentRoot( Int64& rootNodeId );
    bool Resolve( const CORE::CString& selector, Int64& nodeId, bool logMissing = true );
    bool DescribeNode( Int64 nodeId, CORE::CString& nodeName, CORE::CString::StringMap& attributes );
    bool CallOnNode( Int64 nodeId, const CORE::CString& functionDeclaration, const CORE::CString* argument, CORE::CString& returned );
    bool WaitForDomChange( UInt32 timeoutInMs );
    bool GetIsolatedContext( Int64& contextId );

    CChromeDevToolsClient& m_client;
    CORE::CString m_sessionId;
    CNavigationGuard* m_guard;
    UInt32 m_commandTimeoutInMs;
    Int64 m_isolatedContextId;
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
