/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_WEB_TESTAPP_TESTHTMLTEXTEXTRACTOR_H
#define GUCEF_WEB_TESTAPP_TESTHTMLTEXTEXTRACTOR_H

/**
 *  Tests the plain text of HTML fragments (CHtmlTextExtractor): the paragraphs, line breaks, lists and tables, the
 *  character references, the links as text with their address and as a list, what is left out with its content
 *  (scripts, styles, comments), addresses that are not kept, malformed markup, and the recognition of text that is
 *  HTML. Text only, nothing is fetched.
 */
void
PerformHtmlTextExtractorTests( void );

#endif /* GUCEF_WEB_TESTAPP_TESTHTMLTEXTEXTRACTOR_H ? */
