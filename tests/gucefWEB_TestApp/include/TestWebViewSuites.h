/*
 *  gucefWEB_TestApp: tests of the gucefWEB module, its web views
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

#ifndef GUCEF_WEBVIEWTEST_TESTWEBVIEWSUITES_H
#define GUCEF_WEBVIEWTEST_TESTWEBVIEWSUITES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_WEB_WEBVIEW_H
#include "gucefWEB_WebView.h"
#define GUCEF_WEB_WEBVIEW_H
#endif /* GUCEF_WEB_WEBVIEW_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void PerformWebViewTypeTests( void );
void PerformWebViewFrameExchangeTests( void );
void PerformWebViewEngineHostTests( void );

/** The engine independent suite against every engine of the registry and against the fake engine */
void PerformWebViewConformanceTests( void );

/** The suite against one engine: what its capabilities allow, the rest is reported as skipped */
void RunWebViewConformanceSuite( const GUCEF::WEB::TWebViewEnginePtr& engine );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEBVIEWTEST_TESTWEBVIEWSUITES_H ? */
