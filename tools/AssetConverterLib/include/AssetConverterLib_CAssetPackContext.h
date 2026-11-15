/*
 *  AssetConverterLib: a pack while it is converted: its source, its files and its index
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

#ifndef ASSETCONVERTERLIB_CASSETPACKCONTEXT_H
#define ASSETCONVERTERLIB_CASSETPACKCONTEXT_H

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

#ifndef ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#include "AssetConverterLib_CAssetPackManifest.h"
#define ASSETCONVERTERLIB_CASSETPACKMANIFEST_H
#endif /* ASSETCONVERTERLIB_CASSETPACKMANIFEST_H ? */


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A pack while it is converted. Open() makes the source of the manifest readable: a VFS folder is used as it is, an
 *  archive (zip, .unitypackage, ...) is mounted read-only for as long as the context is open, and Close() or the
 *  destructor unmounts it. It lists the files of the source that pass the include and exclude globs, as paths
 *  relative to the root of the source: the paths in the pack.
 *
 *  The index resolves cross-references of the pack: named indexes ("guid", "uid", "objectPath", ...) of keys to paths
 *  in the pack. The importers fill it in IndexPack(), once per pack, before the imports, which only read it. Index
 *  access is guarded, so an importer that fills it later does no harm.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CAssetPackContext
{
    public:

    CAssetPackContext( void );

    ~CAssetPackContext();

    bool Open( const CAssetPackManifest& manifest );

    void Close( void );

    bool IsOpen( void ) const;

    const CAssetPackManifest& GetManifest( void ) const;

    /** The VFS folder that is the root of the source */
    const CORE::CString& GetSourceRoot( void ) const;

    /** Whether Open() mounted an archive */
    bool IsArchiveMounted( void ) const;

    /** The paths in the pack of its files, sorted */
    const CORE::CString::StringVector& GetFiles( void ) const;

    bool HasFile( const CORE::CString& assetPath ) const;

    /** The VFS path of a path in the pack */
    CORE::CString ToVfsPath( const CORE::CString& assetPath ) const;

    /** The path in the pack of a VFS path; false when it is not in the source */
    bool ToAssetPath( const CORE::CString& vfsPath, CORE::CString& assetPath ) const;

    void SetIndexEntry( const CORE::CString& indexName, const CORE::CString& key, const CORE::CString& assetPath );

    bool TryResolve( const CORE::CString& indexName, const CORE::CString& key, CORE::CString& assetPath ) const;

    UInt32 GetIndexSize( const CORE::CString& indexName ) const;

    private:

    CAssetPackContext( const CAssetPackContext& src );              /**< not implemented: it owns a mount */
    CAssetPackContext& operator=( const CAssetPackContext& src );   /**< not implemented: it owns a mount */

    typedef GUCEF::map< CORE::CString, CORE::CString > TKeyMap;
    typedef GUCEF::map< CORE::CString, TKeyMap > TIndexMap;

    CAssetPackManifest m_manifest;
    CORE::CString m_sourceRoot;
    CORE::CString m_mountName;
    CORE::CString::StringVector m_files;
    CORE::CString::StringSet m_fileSet;
    TIndexMap m_index;
    MT::CMutex m_indexLock;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CASSETPACKCONTEXT_H ? */
