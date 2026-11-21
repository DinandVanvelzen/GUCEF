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


#ifndef ASSETWORKBENCHLIB_CLOCALPACKINDEXSOURCE_H
#define ASSETWORKBENCHLIB_CLOCALPACKINDEXSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef ASSETWORKBENCHLIB_IPACKINDEXSOURCE_H
#include "AssetWorkbenchLib_IPackIndexSource.h"
#define ASSETWORKBENCHLIB_IPACKINDEXSOURCE_H
#endif /* ASSETWORKBENCHLIB_IPACKINDEXSOURCE_H ? */

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
 *  The pack index of packs that are documents on the disk (a standalone pack, the intermediate folder of a converter run, the
 *  folder of a pack in a store that the process reads itself): the reader of AssetConverterLib behind IPackIndexSource. A pack is
 *  registered with the VFS folder its pack-index.json is in; the manifest is read at every call (it is small), the records of a
 *  document are read once and kept until the hash of its manifest changes or the pack is invalidated. Never waits for a network.
 *  Thread safe.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CLocalPackIndexSource : public IPackIndexSource
{
    public:

    CLocalPackIndexSource( void );
    virtual ~CLocalPackIndexSource() GUCEF_VIRTUAL_OVERRIDE;

    /** Names the pack: the id the source knows it under, and the folder that holds its pack-index.json */
    void SetPackFolder( const CORE::CString& packId, const CORE::CString& indexFolder );

    /** Forgets the pack and what was read of it */
    void RemovePack( const CORE::CString& packId );

    /** Forgets what was read of the records of the pack; the next call reads the document again */
    void Invalidate( const CORE::CString& packId );

    /** How many times the records of a document were read from the disk (what the tests count) */
    UInt32 GetDocumentReadCount( void ) const;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsCheap( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual EResult GetManifest( const CORE::CString& packId, ASSETCONVERTER::CPackIndexDocument& manifest, bool& stale ) GUCEF_VIRTUAL_OVERRIDE;
    virtual EResult GetGroups( const CORE::CString& packId, ASSETCONVERTER::CPackIndexGroup::TGroupVector& groups ) GUCEF_VIRTUAL_OVERRIDE;
    virtual EResult GetResources( const CORE::CString& packId, const CORE::CString& groupId, const CORE::CString& subGroup,
                                  const CORE::CString& pageToken, UInt32 maxEntries,
                                  ASSETCONVERTER::CPackIndexResource::TResourceVector& page, CORE::CString& nextPageToken, UInt64& total ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetLastMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SPack
    {
        CORE::CString folder;
        bool hasRecords;
        CORE::CString recordsHash;
        ASSETCONVERTER::CPackIndexResource::TResourceVector records;     /**< in the canonical order */

        SPack( void );
    };

    typedef GUCEF::map< CORE::CString, SPack > TPackMap;

    EResult ReadManifestOf( const CORE::CString& packId, ASSETCONVERTER::CPackIndexDocument& manifest, CORE::CString& folder );
    EResult Fail( EResult result, const CORE::CString& message );

    private:

    TPackMap m_packs;
    UInt32 m_documentReads;
    CORE::CString m_lastMessage;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CLOCALPACKINDEXSOURCE_H ? */
