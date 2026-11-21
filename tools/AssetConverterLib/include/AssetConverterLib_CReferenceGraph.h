/*
 *  AssetConverterLib: the reference graph of the cooked files of a profile
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

#ifndef ASSETCONVERTERLIB_CREFERENCEGRAPH_H
#define ASSETCONVERTERLIB_CREFERENCEGRAPH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */


#ifndef ASSETCONVERTERLIB_CASSETREFERENCE_H
#include "AssetConverterLib_CAssetReference.h"
#define ASSETCONVERTERLIB_CASSETREFERENCE_H
#endif /* ASSETCONVERTERLIB_CASSETREFERENCE_H ? */

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
 *  references.json of a cook profile: every cooked file with the asset it was cooked for and the files it references
 *  (a scene its meshes, a mesh its materials and skeleton, a material its textures). Paths are relative to the folder
 *  of the profile, sorted. Built from the cooked files of a report (CConversionReportFile with the profile). The member
 *  "assets" holds the normalized relations of the converted assets translated to cooked files (converter design 5.8.5).
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CReferenceGraph
{
    public:

    struct SFile
    {
        CORE::CString asset;
        CORE::CString::StringVector references;
    };
    typedef GUCEF::map< CORE::CString, SFile > TFileMap;

    /** A normalized relation of an asset (converter design 5.8) with the cooked files of its target in the profile */
    struct SRelation
    {
        CAssetReference relation;                   /**< target as a path in the pack */
        CORE::CString::StringVector files;          /**< the cooked files of the target when it is a converted asset, sorted */
    };
    typedef GUCEF::vector< SRelation > TRelationVector;
    typedef GUCEF::map< CORE::CString, TRelationVector > TAssetRelationMap;

    CReferenceGraph( void );

    /** The cooked files of the profile in the report; 'profileDir' is the VFS folder the paths are made relative to */
    void Build( const CConversionReport& report, const CORE::CString& profile, const CORE::CString& profileDir );

    /**
     *  The normalized relations of an asset of the graph, after Build(): each is kept with the cooked files of its target (the
     *  files of the target's entry in the profile). They are written in the member "assets", only when an asset has any.
     */
    void SetRelations( const CORE::CString& assetPath, const CAssetReference::TReferenceVector& relations );

    const CORE::CString& GetProfile( void ) const;
    const TFileMap& GetFiles( void ) const;
    const TAssetRelationMap& GetAssetRelations( void ) const;

    /** The cooked files of an asset of the graph, sorted */
    void GetFilesOfAsset( const CORE::CString& assetPath, CORE::CString::StringVector& files ) const;

    /** The file and everything it references, directly or not */
    void GetClosure( const CORE::CString& file, CORE::CString::StringSet& closure ) const;

    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );

    bool Load( const CORE::CString& vfsPath );
    bool Save( const CORE::CString& vfsPath ) const;

    private:

    typedef GUCEF::map< CORE::CString, CORE::CString::StringSet > TAssetFileMap;

    CORE::CString m_profile;
    TFileMap m_files;
    TAssetFileMap m_assetFiles;
    TAssetRelationMap m_relations;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CREFERENCEGRAPH_H ? */
