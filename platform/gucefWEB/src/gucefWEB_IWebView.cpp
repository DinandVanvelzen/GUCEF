/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#include "gucefWEB_IWebView.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

const CORE::CEvent IWebView::LoadStartedEvent = "GUCEF::WEB::IWebView::LoadStartedEvent";
const CORE::CEvent IWebView::LoadCommittedEvent = "GUCEF::WEB::IWebView::LoadCommittedEvent";
const CORE::CEvent IWebView::LoadFinishedEvent = "GUCEF::WEB::IWebView::LoadFinishedEvent";
const CORE::CEvent IWebView::LoadFailedEvent = "GUCEF::WEB::IWebView::LoadFailedEvent";
const CORE::CEvent IWebView::TitleChangedEvent = "GUCEF::WEB::IWebView::TitleChangedEvent";
const CORE::CEvent IWebView::UrlChangedEvent = "GUCEF::WEB::IWebView::UrlChangedEvent";
const CORE::CEvent IWebView::ProgressChangedEvent = "GUCEF::WEB::IWebView::ProgressChangedEvent";
const CORE::CEvent IWebView::SecurityStateChangedEvent = "GUCEF::WEB::IWebView::SecurityStateChangedEvent";
const CORE::CEvent IWebView::CursorChangedEvent = "GUCEF::WEB::IWebView::CursorChangedEvent";
const CORE::CEvent IWebView::FrameAvailableEvent = "GUCEF::WEB::IWebView::FrameAvailableEvent";
const CORE::CEvent IWebView::NavigationRequestedEvent = "GUCEF::WEB::IWebView::NavigationRequestedEvent";
const CORE::CEvent IWebView::PopupRequestedEvent = "GUCEF::WEB::IWebView::PopupRequestedEvent";
const CORE::CEvent IWebView::DownloadRequestedEvent = "GUCEF::WEB::IWebView::DownloadRequestedEvent";
const CORE::CEvent IWebView::PermissionRequestedEvent = "GUCEF::WEB::IWebView::PermissionRequestedEvent";
const CORE::CEvent IWebView::CertificateErrorEvent = "GUCEF::WEB::IWebView::CertificateErrorEvent";
const CORE::CEvent IWebView::ScriptDialogEvent = "GUCEF::WEB::IWebView::ScriptDialogEvent";
const CORE::CEvent IWebView::ConsoleMessageEvent = "GUCEF::WEB::IWebView::ConsoleMessageEvent";
const CORE::CEvent IWebView::ProcessTerminatedEvent = "GUCEF::WEB::IWebView::ProcessTerminatedEvent";
const CORE::CEvent IWebView::FocusLeaveRequestedEvent = "GUCEF::WEB::IWebView::FocusLeaveRequestedEvent";
const CORE::CEvent IWebView::CloseRequestedEvent = "GUCEF::WEB::IWebView::CloseRequestedEvent";
const CORE::CEvent IWebView::EngineStalledEvent = "GUCEF::WEB::IWebView::EngineStalledEvent";
const CORE::CEvent IWebView::ClosedEvent = "GUCEF::WEB::IWebView::ClosedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
IWebView::RegisterEvents( void )
{GUCEF_TRACE;

    LoadStartedEvent.Initialize();
    LoadCommittedEvent.Initialize();
    LoadFinishedEvent.Initialize();
    LoadFailedEvent.Initialize();
    TitleChangedEvent.Initialize();
    UrlChangedEvent.Initialize();
    ProgressChangedEvent.Initialize();
    SecurityStateChangedEvent.Initialize();
    CursorChangedEvent.Initialize();
    FrameAvailableEvent.Initialize();
    NavigationRequestedEvent.Initialize();
    PopupRequestedEvent.Initialize();
    DownloadRequestedEvent.Initialize();
    PermissionRequestedEvent.Initialize();
    CertificateErrorEvent.Initialize();
    ScriptDialogEvent.Initialize();
    ConsoleMessageEvent.Initialize();
    ProcessTerminatedEvent.Initialize();
    FocusLeaveRequestedEvent.Initialize();
    CloseRequestedEvent.Initialize();
    EngineStalledEvent.Initialize();
    ClosedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

IWebView::IWebView( void )
    : CORE::CNotifier()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

IWebView::~IWebView()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
