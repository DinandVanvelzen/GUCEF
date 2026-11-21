/*
 *  assetpluginUNREAL: An opened Unreal package: summary, tables, soft object paths and trailer
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H
#include "assetpluginUNREAL_CUnrealPackageSource.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESOURCE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H
#include "assetpluginUNREAL_CUnrealPayloadReference.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPAYLOADREFERENCE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESUMMARY_H
#include "assetpluginUNREAL_CUnrealPackageSummary.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESUMMARY_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGESUMMARY_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H
#include "assetpluginUNREAL_CUnrealNameTable.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALNAMETABLE_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTIMPORT_H
#include "assetpluginUNREAL_CUnrealObjectImport.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTIMPORT_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTIMPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTEXPORT_H
#include "assetpluginUNREAL_CUnrealObjectExport.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTEXPORT_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALOBJECTEXPORT_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H
#include "assetpluginUNREAL_CUnrealPackageTrailer.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGETRAILER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H
#include "assetpluginUNREAL_CUnrealSoftObjectPath.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALSOFTOBJECTPATH_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#include "assetpluginUNREAL_CUnrealReaderLimits.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALREADERLIMITS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Opens a package by range: the header part (TotalHeaderSize bytes) is read once, the exports and payloads are read by
 *  their offsets through readers that this class hands out. It holds no export data. After Open() the package is
 *  immutable and may be shared between tasks; each task uses its own readers.
 *
 *  A damaged package never crashes the reader: Open() fails with a message, and problems that leave the rest usable (an
 *  export that lies outside the package, a damaged trailer) are listed in GetProblems().
 */
class GUCEF_ASSETPLUGIN_UNREAL_EXPORT_CPP CUnrealPackage
{
    public:

    typedef GUCEF::vector< CUnrealObjectImport > TImportVector;
    typedef GUCEF::vector< CUnrealObjectExport > TExportVector;
    typedef GUCEF::vector< bool > TBoolVector;

    CUnrealPackage( void );

    ~CUnrealPackage();

    /** Opens the package at a VFS path; false with GetError() set when it cannot be read as a package */
    bool Open( const CORE::CString& vfsPath, const CUnrealReaderLimits& limits = CUnrealReaderLimits() );

    /** Opens a package that is the byte range [offset,offset+size) of a file in the VFS, as a stored entry of an archive is */
    bool OpenEmbedded( const CORE::CString& vfsPath, UInt64 offset, UInt64 size, const CUnrealReaderLimits& limits = CUnrealReaderLimits() );

    /** Opens a package held in memory (a copy is taken) */
    bool OpenMemory( const void* bytes, UInt32 size, const CORE::CString& name, const CUnrealReaderLimits& limits = CUnrealReaderLimits() );

    void Close( void );

    bool IsOpen( void ) const;

    const CORE::CString& GetError( void ) const;

    const CORE::CString::StringVector& GetProblems( void ) const;

    const CUnrealPackageSource& GetSource( void ) const;

    const CUnrealReaderLimits& GetLimits( void ) const;

    const CUnrealPackageSummary& GetSummary( void ) const;

    const CUnrealVersionContext& GetVersions( void ) const;

    const CUnrealNameTable& GetNames( void ) const;

    const TImportVector& GetImports( void ) const;

    const TExportVector& GetExports( void ) const;

    const CUnrealSoftObjectPath::TSoftObjectPathVector& GetSoftObjectPaths( void ) const;

    bool HasTrailer( void ) const;

    const CUnrealPackageTrailer& GetTrailer( void ) const;

    /** Entries of the data resource table (UE5 version 1009 and later), -1 when the package has none */
    Int32 GetDataResourceCount( void ) const;

    /** Bytes taken from the source by everything this package read so far */
    UInt64 GetBytesFetched( void ) const;

    /** The name of the class of an export: the object name of its import or export; "Class" for a null index */
    CORE::CString GetClassName( const CUnrealObjectExport& object ) const;

    /** The name an index refers to: of an import, of an export, empty for null or an index outside the tables */
    CORE::CString GetObjectName( const CUnrealPackageIndex& index ) const;

    /** "Package.Object:Child" for an export, following the outer chain (cycles and depth are bounded) */
    CORE::CString GetExportPath( UInt32 exportIndex ) const;

    /** The path an import stands for, "/Script/Engine.StaticMesh" or "/Game/Dir/Package.Object" */
    CORE::CString GetImportPath( UInt32 importIndex ) const;

    /** The index of the first export of the given object name, -1 when absent */
    Int32 FindExport( const CORE::CString& objectName ) const;

    /** Whether the bytes of an export lie inside the package */
    bool IsExportRangeValid( UInt32 exportIndex ) const;

    /** A reader on all the serialized bytes of an export */
    bool OpenExportRange( UInt32 exportIndex, CUnrealArchiveReader& reader );

    /** A reader on the script part of an export (the tagged properties), the whole export before UE5 version 1010 */
    bool OpenPropertyRange( UInt32 exportIndex, CUnrealArchiveReader& reader );

    /** A reader on the stored bytes of a payload reference */
    bool OpenPayload( const CUnrealPayloadReference& reference, CUnrealArchiveReader& reader );

    /** Adds the bytes a reader took from the source to the statistics of the package */
    void AccountFetched( const CUnrealArchiveReader& reader );

    private:

    CUnrealPackage( const CUnrealPackage& src );            /**< not implemented */
    CUnrealPackage& operator=( const CUnrealPackage& src ); /**< not implemented */

    bool Load( const CUnrealReaderLimits& limits );

    bool CheckTable( CUnrealArchiveReader& reader, Int32 count, Int32 offset, UInt32 minEntryBytes, const char* what );

    void AddProblem( const CORE::CString& text );

    private:

    CUnrealPackageSource m_source;
    CUnrealReaderLimits m_limits;
    CUnrealPackageSummary m_summary;
    CUnrealNameTable m_names;
    TImportVector m_imports;
    TExportVector m_exports;
    TBoolVector m_exportRangeValid;
    CUnrealSoftObjectPath::TSoftObjectPathVector m_softPaths;
    CUnrealPackageTrailer m_trailer;
    bool m_hasTrailer;
    Int32 m_dataResourceCount;
    CORE::CString m_error;
    CORE::CString::StringVector m_problems;
    UInt64 m_bytesFetched;
    bool m_open;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPACKAGE_H ? */
