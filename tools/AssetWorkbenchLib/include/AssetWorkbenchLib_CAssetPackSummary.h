/*
 *  AssetWorkbenchLib: what the inspector shows of a folder or a recognised pack: engine, version, counts per kind, size and licence
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

#ifndef ASSETWORKBENCHLIB_CASSETPACKSUMMARY_H
#define ASSETWORKBENCHLIB_CASSETPACKSUMMARY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKLISTING_H
#include "AssetConverterLib_CAssetPackListing.h"
#define ASSETCONVERTERLIB_CASSETPACKLISTING_H
#endif /* ASSETCONVERTERLIB_CASSETPACKLISTING_H ? */

#ifndef ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#include "AssetConverterLib_CAssetPackProfile.h"
#define ASSETCONVERTERLIB_CASSETPACKPROFILE_H
#endif /* ASSETCONVERTERLIB_CASSETPACKPROFILE_H ? */

#ifndef ASSETCONVERTERLIB_ETYPES_H
#include "AssetConverterLib_ETypes.h"
#define ASSETCONVERTERLIB_ETYPES_H
#endif /* ASSETCONVERTERLIB_ETYPES_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLICENCE_H
#include "gucefASSETSTORE_CAssetLicence.h"
#define GUCEF_ASSETSTORE_CASSETLICENCE_H
#endif /* GUCEF_ASSETSTORE_CASSETLICENCE_H ? */

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
 *  The summary of a folder of the VFS for the inspector: the files per kind, their size, how many metadata files belong to the assets
 *  (they are not counted as files of their own), and, when the importer plugins recognise the folder as a pack (Unity project, Godot
 *  project, Unreal project or MetaHuman package, a model pack), the engine, its version, the type of the pack and the entry points
 *  the pack offers. The licence record is shown when the store has one (SetLicence()).
 *
 *  Read() lists the folder on the calling thread: it stops at maxFiles files and then says so.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CAssetPackSummary
{
    public:

    CAssetPackSummary( void );

    void Clear( void );

    /** Lists and counts the files below the folder, and asks the plugins whether it is a pack; false when the folder holds no files */
    bool Read( const CORE::CString& vfsFolder, UInt32 maxFiles = 20000 );

    /** Gives the licence record of the pack; Clear() takes it away */
    void SetLicence( const ASSETSTORE::CAssetLicence& licence );

    bool IsRecognisedPack( void ) const;

    const ASSETCONVERTER::CAssetPackProfile& GetProfile( void ) const;
    /** The files of the folder as the last Read() listed them (for the choice of the presentation) */
    const ASSETCONVERTER::CAssetPackListing& GetListing( void ) const;

    /** The files of a kind, without the metadata files (those are counted under ASSETKIND_METADATA) */
    UInt32 GetKindCount( ASSETCONVERTER::EAssetKind kind ) const;

    UInt32 GetFileCount( void ) const;

    /** The metadata files that belong to an asset of the folder, and are left out of the listing unless the person wants them */
    UInt32 GetMetadataCount( void ) const;

    UInt64 GetTotalSize( void ) const;

    bool IsTruncated( void ) const;

    const CORE::CString& GetFolder( void ) const;

    /** The tree for the inspector: a node "Pack" for a recognised pack, "Folder" for any other */
    void WriteInspectorData( CORE::CDataNode& node ) const;

    /** "12.5 MB": the size in the words of the inspector */
    static CORE::CString FormatSize( UInt64 bytes );

    private:

    CORE::CString m_folder;
    ASSETCONVERTER::CAssetPackProfile m_profile;
    ASSETCONVERTER::CAssetPackListing m_listing;
    UInt32 m_kindCounts[ ASSETCONVERTER::ASSETKIND_COUNT ];
    UInt32 m_fileCount;
    UInt32 m_metadataCount;
    UInt64 m_totalSize;
    bool m_truncated;
    bool m_hasLicence;
    ASSETSTORE::CAssetLicence m_licence;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CASSETPACKSUMMARY_H ? */
