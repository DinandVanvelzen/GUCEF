/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
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

#ifndef GUCEF_COM_TESTMIME_H
#define GUCEF_COM_TESTMIME_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** base64, quoted-printable, encoded words, header folding, parameters, dates, character sets */
void PerformMimeCodecTests( void );

/** The parser with samples of real mail, and with hostile input */
void PerformMimeParserTests( void );

/** The composer, the round trip through the parser and the injection attempts */
void PerformMimeComposerTests( void );

/** Addresses, headers, attachments and the validation of a message */
void PerformMailModelTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_TESTMIME_H ? */
