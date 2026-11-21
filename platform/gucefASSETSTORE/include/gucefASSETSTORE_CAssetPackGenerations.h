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

#ifndef GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H
#define GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */
#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#include "gucefASSETSTORE_CAssetPackRecord.h"
#define GUCEF_ASSETSTORE_CASSETPACKRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETPACKRECORD_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETRECORD_H
#include "gucefASSETSTORE_CAssetRecord.h"
#define GUCEF_ASSETSTORE_CASSETRECORD_H
#endif /* GUCEF_ASSETSTORE_CASSETRECORD_H ? */
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
 *  The generations of the regenerated outputs of a stored pack (layout 2 of CPackStorageLayout): when the intermediate or the
 *  cooked files of a pack are made again (CAssetPackRegeneration), the new outputs are written to a staging folder and put in
 *  place in one swap; the outputs they replace are kept as the previous generation, so a regeneration that made things worse can
 *  be rolled back. One previous generation is kept per pack, until the next successful regeneration replaces it:
 *
 *    <location>/generations/<pack folder>/previous/intermediate/...     the intermediate area of the pack before the last regeneration
 *    <location>/generations/<pack folder>/previous/gucef-ogre3d/...     the cooked area, when that was regenerated
 *    <location>/generations/<pack folder>/previous/generation.json      the state of the catalogue that goes with those files (the
 *                                                                       conversion record, the converted paths and formats, the state
 *                                                                       of the pack) and what made the generation (job, caller, time)
 *
 *  A swap is a few renames inside the storage location (VFS MoveDir, no copies). A marker file (swap.json beside "previous")
 *  says which swap is under way: Undo() takes the files back when the catalogue did not take the change, and Recover() does the
 *  same for a swap that a crash interrupted (called before every regeneration and rollback of the pack). Commit() ends a swap
 *  after the catalogue was written. A rollback exchanges the previous generation and the current one, so a rollback of a
 *  rollback restores the regenerated outputs. Originals are never touched: only the areas "intermediate" and "gucef-ogre3d".
 *  Not thread safe: one job per pack at a time (the service refuses a second one).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetPackGenerations
{
    public:

    static const CORE::CString PreviousFolderName;      /**< "previous" */
    static const CORE::CString GenerationFileName;      /**< "generation.json" */
    static const CORE::CString MarkerFileName;          /**< "swap.json" */

    CAssetPackGenerations( const CPackStorageLayout& layout, const CORE::CString& locationRoot );

    /** Whether the pack has generations: packs of layout 2 */
    bool IsSupported( void ) const;

    /** "<location>/generations/<pack folder>" */
    const CORE::CString& GetRoot( void ) const;

    CORE::CString GetPreviousFolder( void ) const;

    /** The VFS folder of an area of the pack now ("intermediate", "gucef-ogre3d"); empty for another name */
    CORE::CString GetCurrentFolder( const CORE::CString& area ) const;

    /** Whether a previous generation is kept, with its generation.json */
    bool HasPrevious( void ) const;

    /** The generation.json of the previous generation */
    bool ReadPrevious( CORE::CDataNode& generation ) const;

    /** The areas the previous generation holds */
    void GetPreviousAreas( CORE::CString::StringVector& areas ) const;

    /**
     *  Puts the areas staged below stagingRoot ("<stagingRoot>/intermediate", "<stagingRoot>/gucef-ogre3d") in place: the current
     *  folders of those areas become the previous generation with 'replacedGeneration' as its generation.json, the previous
     *  generation before it is dropped at Commit(). False with the problem when a rename failed; the files are then as they were.
     */
    bool Replace( const CORE::CString& stagingRoot, const CORE::CString::StringVector& areas, const CORE::CDataNode& replacedGeneration, CORE::CString& problem );

    /**
     *  Exchanges the previous generation and the current outputs: the areas of the previous generation become current, the
     *  current ones become the previous generation with 'currentGeneration' as its generation.json. 'restored' gets the
     *  generation.json of what is current now (the one to apply to the catalogue), 'areas' the areas exchanged.
     */
    bool Rollback( const CORE::CDataNode& currentGeneration, CORE::CDataNode& restored, CORE::CString::StringVector& areas, CORE::CString& problem );

    /** The catalogue took the change: the marker and what the swap left (the generation before the previous one) go */
    void Commit( void );

    /** The catalogue did not take the change: the swap of Replace() or Rollback() is undone */
    bool Undo( CORE::CString& problem );

    /** Undoes a swap that did not reach Commit() (a crash); 'note' says what was done; false when nothing was to be done */
    bool Recover( CORE::CString& note );

    /** The generation.json of the conversion state of a pack: the conversion record, the converted paths and formats, the state; 'extra' (may be null) is copied in as the member "made" */
    static void MakeGeneration( const CAssetPackRecord& pack, const TAssetRecordVector& assets, const CORE::CDataNode* extra, CORE::CDataNode& generation );

    /** Sets the conversion state of a generation.json on the records (assets that it does not name lose their converted path) */
    static bool ApplyGeneration( const CORE::CDataNode& generation, CAssetPackRecord& pack, TAssetRecordVector& assets );

    /** Reads or writes a JSON file of the VFS */
    static bool LoadJson( const CORE::CString& vfsPath, CORE::CDataNode& root );
    static bool StoreJson( const CORE::CString& vfsPath, const CORE::CDataNode& root );

    private:

    bool WriteMarker( const CORE::CDataNode& marker );
    bool EnsureFolder( const CORE::CString& folder ) const;

    CPackStorageLayout m_layout;
    CORE::CString m_locationRoot;
    CORE::CString m_root;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETPACKGENERATIONS_H ? */
