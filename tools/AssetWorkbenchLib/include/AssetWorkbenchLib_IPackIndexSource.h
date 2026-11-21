/*
 *  AssetWorkbenchLib: the library of the asset workbench
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


#ifndef ASSETWORKBENCHLIB_IPACKINDEXSOURCE_H
#define ASSETWORKBENCHLIB_IPACKINDEXSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#include "AssetConverterLib_CPackIndexDocument.h"
#define ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXDOCUMENT_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Where the resource view gets the pack index of a pack from (section 6.3 of the pack index design): one interface for the
 *  document of a pack on the disk (CLocalPackIndexSource) and for the pack index operations of the asset store service
 *  (CServicePackIndexSource, which serves the service of this process and a remote one alike). The view does not know which
 *  it has.
 *
 *  A pack is named by the id the source knows it under: the store pack id for the service, what the caller registered for a
 *  local source. The calls are the synchronous forms and run on the calling thread: a source that is not cheap (IsCheap()
 *  false: it waits for a network) is not called on the thread of a GUI.
 *
 *  - GetManifest(): the manifest of the index (identity, producer, profile, licence flags, counts, groups, shards) without the
 *    resource records; 'stale' says the index is known to be out of date and is being refreshed.
 *  - GetGroups(): the function groups with their sub-groups and counts: the first two levels of the resource tree.
 *  - GetResources(): a page of the records of a group or sub-group (all of the pack for an empty group), in the canonical order
 *    (group in the fixed order, sub-group, path); 'pageToken' is empty for the first page and what the previous page gave
 *    after that; 'nextPageToken' is empty on the last page. A source that serves the records from the catalogue gives the
 *    members the catalogue holds (path, name, kind, group, sub-group, format, size, digest, status); one that reads the
 *    document gives the whole record.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP IPackIndexSource
{
    public:

    enum EResult
    {
        RESULT_OK = 0       ,
        RESULT_NOT_FOUND    ,   /**< the pack has no index, or the source does not know the pack */
        RESULT_UNAVAILABLE  ,   /**< the source cannot be reached and has nothing cached */
        RESULT_FAILED
    };

    static const UInt32 DefaultPageSize = 100;

    IPackIndexSource( void );
    virtual ~IPackIndexSource();

    /** What the source is, for people */
    virtual CORE::CString GetName( void ) const = 0;

    /** Whether a call returns without waiting for a network: the source of this process, or a document on the disk */
    virtual bool IsCheap( void ) const = 0;

    virtual EResult GetManifest( const CORE::CString& packId, ASSETCONVERTER::CPackIndexDocument& manifest, bool& stale ) = 0;

    virtual EResult GetGroups( const CORE::CString& packId, ASSETCONVERTER::CPackIndexGroup::TGroupVector& groups ) = 0;

    virtual EResult GetResources( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup,
                                  const CORE::CString& pageToken, UInt32 maxEntries,
                                  ASSETCONVERTER::CPackIndexResource::TResourceVector& page, CORE::CString& nextPageToken, UInt64& total ) = 0;

    /** What the last call that did not give RESULT_OK said, in words */
    virtual CORE::CString GetLastMessage( void ) const = 0;

    private:

    IPackIndexSource( const IPackIndexSource& src );                /**< not implemented */
    IPackIndexSource& operator=( const IPackIndexSource& src );     /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_IPACKINDEXSOURCE_H ? */
