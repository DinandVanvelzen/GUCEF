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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLROOT_H
#define GUCEF_ASSETSTORE_CASSETTOOLROOT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

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
 *  A folder the asset tools may work in (section 11.1 of the asset tools design): an id, the folder as a path of the
 *  VFS, and its kind, which decides which tools accept paths in it and whether they may write there. Paths that tools
 *  take and give are rooted paths "<id>:<relative path>" (CAssetRootedPath) against these roots, never paths of the
 *  disk. The roots are configuration of the process that serves the tools: an MCP client cannot add or change them.
 *
 *  Kinds and what the tools may do in them:
 *  - source: purchased packs, folders and archives; read only, whatever 'writable' says
 *  - staging: downloads waiting for an ingest; read
 *  - licences: licence records; read only, whatever 'writable' says
 *  - manifests: pack manifests; written by the authoring tools when 'writable'
 *  - overrides: per asset import settings; written by the authoring tools when 'writable'
 *  - intermediate, output: written by the converter runs
 *  - work: scratch packs and previews; read and write
 *  - captures: written by the capture service only
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolRoot : public CORE::CIDataNodeSerializable
{
    public:

    static const CORE::CString KindSource;          /**< "source" */
    static const CORE::CString KindStaging;         /**< "staging" */
    static const CORE::CString KindLicences;        /**< "licences" */
    static const CORE::CString KindManifests;       /**< "manifests" */
    static const CORE::CString KindOverrides;       /**< "overrides" */
    static const CORE::CString KindIntermediate;    /**< "intermediate" */
    static const CORE::CString KindOutput;          /**< "output" */
    static const CORE::CString KindWork;            /**< "work" */
    static const CORE::CString KindCaptures;        /**< "captures" */

    typedef GUCEF::vector< CAssetToolRoot > TRootVector;

    CORE::CString id;               /**< [a-z][a-z0-9-]{0,31} */
    CORE::CString vfsPath;          /**< the folder in the VFS, mounted by the host before the tools start */
    CORE::CString diskPath;         /**< the folder of the file system that is mounted there, for the check of symbolic links; empty: not a folder of the disk. Never shown to a client */
    CORE::CString kind;             /**< one of the kinds above */
    bool writable;                  /**< may the tools write here; never for source and licences */

    CAssetToolRoot( void );

    virtual ~CAssetToolRoot() GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the kind is one of the kinds above */
    static bool IsKnownKind( const CORE::CString& kind );

    /** Whether tools may write below the root: 'writable' and a kind that can be written */
    bool AllowsWrites( void ) const;

    /** Whether the members are usable: a valid id, a VFS path and a known kind */
    bool IsValid( CORE::CString& problem ) const;

    /** The root with the id in the list; GUCEF_NULL when there is none */
    static const CAssetToolRoot* Find( const TRootVector& roots, const CORE::CString& id );

    /** The first root of the kind in the list; GUCEF_NULL when there is none */
    static const CAssetToolRoot* FindFirstOfKind( const TRootVector& roots, const CORE::CString& kind );

    /**
     *  The root of one of the kinds whose folder holds the VFS path (the root with the longest folder when they nest),
     *  with the path relative to it; GUCEF_NULL when no root of those kinds holds it. The kinds are a list separated by
     *  '|' ("source|staging"); empty: any kind
     */
    static const CAssetToolRoot* FindHolder( const TRootVector& roots, const CORE::CString& vfsPath, const CORE::CString& kinds, CORE::CString& relativePath );

    /** Reads the list of an array node of root objects; false when an entry is not valid ('problem' says which) */
    static bool LoadList( const CORE::CDataNode& arrayNode, TRootVector& roots, CORE::CString& problem );

    /** Writes the list as an array node */
    static void SaveList( const TRootVector& roots, CORE::CDataNode& arrayNode );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

typedef CAssetToolRoot::TRootVector     TAssetToolRootVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLROOT_H ? */
