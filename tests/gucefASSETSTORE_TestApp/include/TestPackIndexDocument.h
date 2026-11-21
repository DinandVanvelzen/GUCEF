/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTPACKINDEXDOCUMENT_H
#define TESTPACKINDEXDOCUMENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */
#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** One resource of a synthetic pack index document */
class CTestPackIndexResource
{
    public:

    GUCEF::CORE::CString path;
    GUCEF::CORE::CString kind;                  /**< "model", "texture", "image", "material", "audio", "animation", "scene", "document", ... */
    GUCEF::CORE::CString subGroup;
    GUCEF::UInt64 size;
    GUCEF::CORE::CString state;                 /**< "indexed" (default), "analysed", "converted" */
    GUCEF::CORE::CString::StringVector statNames;
    GUCEF::vector< GUCEF::UInt64 > statValues;
    GUCEF::CORE::CString::StringVector refPaths;    /**< usesTexture/usesMaterial targets, by path in the pack */
    GUCEF::CORE::CString::StringVector refKinds;
    GUCEF::vector< bool > refByPath;            /**< per reference: written with its "path" (and a "to" of the document id), as the converter writes them from the relations */
    bool hasThumbnail;

    CTestPackIndexResource( void );
    CTestPackIndexResource( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& kind, const GUCEF::CORE::CString& subGroup, GUCEF::UInt64 size );
    CTestPackIndexResource& Stat( const GUCEF::CORE::CString& name, GUCEF::UInt64 value );
    CTestPackIndexResource& Ref( const GUCEF::CORE::CString& refKind, const GUCEF::CORE::CString& toPath );

    /** A reference with the path of its target (pack index design 2.12); 'wrongId' writes a "to" that is no id of the document, so the path has to decide */
    CTestPackIndexResource& RefPath( const GUCEF::CORE::CString& refKind, const GUCEF::CORE::CString& toPath, bool wrongId = false );
};

/**
 *  A synthetic pack index document in the schema of the pack index design (section 2.4), written as the files the converter
 *  writes (the manifest "pack-index.json" and, above 'shardSize' records, shards in "pack-index.d"), so that the loader of the
 *  store is tested against a document that no code of the store made. The ids of the document are made with the pack name as
 *  the base, not the store pack id, so the loader has to map them. The hash of the document is a hash over what is in it, so a
 *  changed record changes it; the shards carry the sha256 of their bytes.
 */
class CTestPackIndexDocument
{
    public:

    GUCEF::CORE::CString packName;
    GUCEF::CORE::CString packKey;
    GUCEF::CORE::CString indexKey;
    GUCEF::CORE::CString state;                 /**< "complete" by default */
    GUCEF::UInt32 version;                      /**< 1 */
    GUCEF::CORE::CString format;                /**< "gucef.packIndex" */
    GUCEF::UInt64 fileCount;                    /**< of the fingerprint, 0: not written */
    GUCEF::UInt64 totalBytes;
    GUCEF::UInt32 noise;
    GUCEF::UInt32 shardSize;                    /**< 0: all inline */
    bool derived;
    bool omitDocumentHash;
    GUCEF::CORE::CString::StringVector restrictions;
    GUCEF::vector< CTestPackIndexResource > resources;

    CTestPackIndexDocument( void );

    /** The id the document gives the resource at the path */
    GUCEF::CORE::CString MakeDocumentId( const GUCEF::CORE::CString& path ) const;

    /** Writes the document into the folder (a VFS path of a mounted, writeable folder); the manifest path, empty on failure */
    GUCEF::CORE::CString WriteTo( const GUCEF::CORE::CString& indexFolder ) const;

    /** The function group the kind belongs to ("model" gives static-models, "texture" textures, ...) */
    static GUCEF::CORE::CString GroupOfKind( const GUCEF::CORE::CString& kind );
    static GUCEF::CORE::CString NameOfGroup( const GUCEF::CORE::CString& group );
};

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTPACKINDEXDOCUMENT_H ? */
