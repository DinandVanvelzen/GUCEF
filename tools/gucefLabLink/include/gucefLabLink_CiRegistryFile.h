/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_CIREGISTRYFILE_H
#define GUCEF_LABLINK_CIREGISTRYFILE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the CI registry of the hub is */
enum ECiRegistryState
{
    CIREGISTRY_OK           = 0 ,   /**< valid; the merged text is there */
    CIREGISTRY_MISSING      = 1 ,   /**< there is no file: this hub hands out no CI configuration */
    CIREGISTRY_INVALID      = 2 ,   /**< the file is too big, not JSON, not an object, or has no "ci" list */
    CIREGISTRY_NO_PARSER    = 3     /**< dstorepluginJSONPARSER is not loaded: the file cannot be checked */
};
typedef enum ECiRegistryState TCiRegistryState;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The CI registry of the hub (ci.json, the format of gucefMcpCi: an object with the list "ci" of CI definitions),
 *  as the machines receive it: the text of the file with the top-level object "gateway" set by the hub,
 *  { "url": <the gateway URL>, "caFile": "ca.crt" }, which replaces one the file may have. The other members are
 *  passed on as they are in the file.
 *
 *  The file is read when it is first asked for and again only when its size or modification time changed. It may
 *  be at most MaxSizeInBytes; it is checked with the JSON codec of dstorepluginJSONPARSER, which is loaded when it
 *  is not there yet. Thread safe.
 */
class CCiRegistryFile
{
    public:

    static const GUCEF::UInt32 MaxSizeInBytes = 64 * 1024;

    CCiRegistryFile( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& gatewayUrl );

    /**
     *  The registry as the machines receive it.
     *  @param merged   the text, when CIREGISTRY_OK
     *  @param problem  what is wrong, for the log of the hub, when CIREGISTRY_INVALID or CIREGISTRY_NO_PARSER
     */
    TCiRegistryState Get( GUCEF::CORE::CString& merged, GUCEF::CORE::CString& problem );

    const GUCEF::CORE::CString& GetPath( void ) const;

    const GUCEF::CORE::CString& GetGatewayUrl( void ) const;

    /**
     *  Checks the text of a registry and sets its object "gateway", without a file.
     *  @return CIREGISTRY_OK, CIREGISTRY_INVALID or CIREGISTRY_NO_PARSER
     */
    static TCiRegistryState Merge( const GUCEF::CORE::CString& json       ,
                                   const GUCEF::CORE::CString& gatewayUrl ,
                                   GUCEF::CORE::CString& merged           ,
                                   GUCEF::CORE::CString& problem          );

    /** Whether the JSON codec is there; loads dstorepluginJSONPARSER once when it is not */
    static bool IsJsonCodecAvailable( void );

    private:

    CCiRegistryFile( const CCiRegistryFile& src );
    CCiRegistryFile& operator=( const CCiRegistryFile& src );

    GUCEF::CORE::CString m_path;
    GUCEF::CORE::CString m_gatewayUrl;
    bool m_read;                                /**< whether the file was looked at */
    bool m_existed;
    GUCEF::UInt64 m_size;
    GUCEF::UInt64 m_modifiedInMs;
    TCiRegistryState m_state;
    GUCEF::CORE::CString m_merged;
    GUCEF::CORE::CString m_problem;
    GUCEF::MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_CIREGISTRYFILE_H ? */
