/*
 *  gucefIrcHub: IRC hub with LLM bots for a family IRC server
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

#ifndef GUCEF_IRCHUB_TEXTUTILS_H
#define GUCEF_IRCHUB_TEXTUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string>
#include <vector>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFIrcHub {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Whether a chat line is addressed to nick: it starts with "nick:", "nick," or "@nick", or it contains "@nick" further
 *  on as a whole word. Case does not matter.
 *  @param rest  receives the text without the address at the start; the whole text when the address is further on
 */
bool IsAddressedTo( const GUCEF::CORE::CString& text ,
                    const GUCEF::CORE::CString& nick ,
                    GUCEF::CORE::CString& rest       );

/**
 *  Whether a chat line starts with the address of one of the nicks ("nick:" or "nick,"), case ignored
 *  @param addressee  receives the nick it is addressed to
 */
bool StartsWithAddressOf( const GUCEF::CORE::CString& text                     ,
                          const GUCEF::CORE::CString::StringVector& nicks      ,
                          GUCEF::CORE::CString& addressee                      );

/**
 *  Splits a text into lines of at most maxBytes bytes of UTF-8 for IRC messages: at its line breaks, and long lines at
 *  the last space before the limit (at the limit when there is none, never inside a UTF-8 character). Empty lines,
 *  carriage returns and other control characters are left out; tabs become spaces.
 */
void SplitForIrc( const std::string& text           ,
                  size_t maxBytes                   ,
                  std::vector< std::string >& lines );

/** Trims spaces, tabs and line breaks from both ends */
std::string TrimWhitespace( const std::string& text );

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFIrcHub */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IRCHUB_TEXTUTILS_H ? */
