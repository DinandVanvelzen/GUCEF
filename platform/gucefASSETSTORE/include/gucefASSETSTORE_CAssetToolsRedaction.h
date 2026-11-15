/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLSREDACTION_H
#define GUCEF_ASSETSTORE_CASSETTOOLSREDACTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CREDACTOR_H
#include "gucefCORE_CRedactor.h"
#define GUCEF_CORE_CREDACTOR_H
#endif /* GUCEF_CORE_CREDACTOR_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_ASSETSTORE_MACROS_H
#include "gucefASSETSTORE_macros.h"
#define GUCEF_ASSETSTORE_MACROS_H
#endif /* GUCEF_ASSETSTORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The output filter of the asset tools (section 11.5 of the asset tools design): every text that leaves through
 *  MCP (results, error texts, untrusted text, resource text, progress messages) goes through ForResult():
 *
 *  - the exact secrets the process knows (AddSecret(): the database password, store tokens and cookies, a bearer
 *    token, keys of storage locations), their percent-encoded form as well, longest first;
 *  - CORE::CRedactor with the user info of URLs, the values of sensitive query items and fragments, name=value
 *    pairs of sensitive names (password, pwd, token, secret, key, cookie, authorization, ...), the credentials of
 *    Bearer and Basic; connection strings are URLs (user info) or key=value pairs (password=...);
 *  - control characters (other than line ends and tabs) and bidirectional overrides are taken out.
 *
 *  Digests and job handles are not secrets and stay readable. Thread safe.
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolsRedaction
{
    public:

    CAssetToolsRedaction( void );

    /** A secret the process knows; false for an empty one */
    bool AddSecret( const CORE::CString& secret );

    bool RemoveSecret( const CORE::CString& secret );

    UInt32 GetSecretCount( void ) const;

    /** The text as it may leave */
    CORE::CString ForResult( const CORE::CString& text ) const;

    /** ForResult() of every text value in the tree, in place */
    void RedactTree( CORE::CDataNode& node ) const;

    /** Takes control characters (but line ends and tabs) and bidirectional overrides out */
    static CORE::CString Sanitize( const CORE::CString& text );

    private:

    CAssetToolsRedaction( const CAssetToolsRedaction& src );              /**< not implemented */
    CAssetToolsRedaction& operator=( const CAssetToolsRedaction& src );   /**< not implemented */

    void RedactTreeLocked( CORE::CDataNode& node ) const;

    mutable MT::CMutex m_lock;
    CORE::CRedactor m_redactor;
    UInt32 m_secretCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLSREDACTION_H ? */
