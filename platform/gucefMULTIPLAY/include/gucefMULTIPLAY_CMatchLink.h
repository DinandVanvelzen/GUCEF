/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CMATCHLINK_H
#define GUCEF_MULTIPLAY_CMATCHLINK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Links that open a match: made for a match alert or a mail, read from the command line, from the handler of a URI
 *  scheme or from a link that was clicked. Two forms are known, both carry the match id and nothing else:
 *
 *      <scheme>://<custom host>/<match id>        for example  game://match/3f2504e0-4f89-41d3-9a0c-0305e82c3301
 *      https://<game domain>/match/<match id>      the verified link form of the web and of Android App Links
 *
 *  The scheme, the custom host (default "match") and the game domains are set by the game. The reader is for text from
 *  outside: it is bounded, takes no percent encoding, no user info, no port and no characters outside the link
 *  alphabet, ignores a query and a fragment, and only gives back a well formed match id (in lower case).
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchLink
{
    public:

    static const UInt32 MaxLinkLength = 512;

    CMatchLink( void );

    CMatchLink( const CORE::CString& customScheme, const CORE::CString& httpsHost );

    CMatchLink( const CMatchLink& src );

    ~CMatchLink();

    CMatchLink& operator=( const CMatchLink& src );

    /** The scheme of the custom form, for example "game"; empty for none */
    void SetCustomScheme( const CORE::CString& scheme );

    /** The host part of the custom form, "match" by default */
    void SetCustomHost( const CORE::CString& host );

    /** A game domain of the https form; any number can be added */
    void AddHttpsHost( const CORE::CString& host );

    const CORE::CString& GetCustomScheme( void ) const;

    const CORE::CString& GetCustomHost( void ) const;

    /** The custom form for the match, empty when there is no scheme or the id is not a match id */
    CORE::CString MakeCustomLink( const CORE::CString& matchId ) const;

    /** The https form on the first game domain, empty when there is none or the id is not a match id */
    CORE::CString MakeHttpsLink( const CORE::CString& matchId ) const;

    /** Gets the match id out of a link of either form; false for anything else */
    bool Parse( const CORE::CString& link, CORE::CString& matchId ) const;

    private:

    CORE::CString m_customScheme;
    CORE::CString m_customHost;
    CORE::CString::StringVector m_httpsHosts;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHLINK_H ? */
