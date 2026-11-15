/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CDRNUTILSDIGEST_H
#define GUCEF_DRNUTILS_CDRNUTILSDIGEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Canonical forms and SHA-256 digests of what remote commands carry, so that the audit trail can
 *  name what was sent without keeping it in clear (also for audit adapters of services that verify a record).
 *
 *  Canonical arguments: the arguments sorted by name, each "<name>=<value>" followed by a line feed (UTF-8).
 *  Canonical data node: depth first, per node "<type>:<name>=<value>" followed by its attributes sorted by name as
 *  "@<name>=<value>", then "{", the children in order, "}" (each item followed by a line feed).
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDrnUtilsDigest
{
    public:

    /** SHA-256 as 64 lower case hex characters */
    static CORE::CString Sha256Hex( const void* data, UInt32 size );

    /** Empty when there are no arguments */
    static CORE::CString ArgumentsDigest( const CORE::CString::StringMap& arguments );

    static void CanonicalDataNode( const CORE::CDataNode& node, CORE::CString& canonical );

    /** Digest of the canonical form; canonicalBytes is its size, the bound submitted payloads are measured with */
    static CORE::CString DataNodeDigest( const CORE::CDataNode& node, UInt32& canonicalBytes );

    private:
    CDrnUtilsDigest( void );    /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDRNUTILSDIGEST_H ? */
