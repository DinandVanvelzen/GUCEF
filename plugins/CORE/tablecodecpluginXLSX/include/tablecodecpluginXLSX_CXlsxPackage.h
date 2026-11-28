/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H
#define TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H
#include "tablecodecpluginXLSX_CXlsxReadOptions.h"
#define TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXXMLCALLBACKS_H
#include "tablecodecpluginXLSX_CXlsxXmlCallbacks.h"
#define TABLECODECPLUGIN_XLSX_CXLSXXMLCALLBACKS_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXXMLCALLBACKS_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXLIMITEDACCESS_H
#include "tablecodecpluginXLSX_CXlsxLimitedAccess.h"
#define TABLECODECPLUGIN_XLSX_CXLSXLIMITEDACCESS_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXLIMITEDACCESS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace XLSX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An .xlsx file as a package of parts. The zip archive is read through the zip archive type of the VFS (the plugin
 *  vfspluginZIP) and the XML parts are parsed with the XML codec of the data store codec registry (the plugin
 *  dstorepluginPARSIFALXML).
 *
 *  The zip archive type of the VFS opens an archive from a file in the file system: it does not load an archive from a
 *  resource (its resource based LoadArchive() refuses), while the source of a table codec is any CIOAccess. The package
 *  therefore writes the source to a file in the temporary directory (owner access only), mounts it in the VFS under a mount
 *  point of its own and removes both again when it is destroyed. The size of the file, the number of the entries, the sizes
 *  the entries state and the bytes that are really read are all held against the limits of the options, so that a zip bomb
 *  is refused before and while its parts are read.
 *
 *  One thread at a time.
 */
class TABLECODECPLUGIN_XLSX_PUBLIC_CPP CXlsxPackage
{
    public:

    explicit CXlsxPackage( const SXlsxReadOptions& options );

    /** Unmounts the archive and removes the file */
    ~CXlsxPackage();

    /** Reads the source from its start; the zip archive type of the VFS and the XML codec must be there */
    CORE::TDataTableStatus Open( CORE::CIOAccess& source, CORE::CString& error );

    /** The name of a part is without a leading slash: "xl/workbook.xml" */
    bool HasPart( const CORE::CString& partName ) const;

    /** The part is parsed with the XML codec; false (with the status and the error) when it cannot be read or is no XML */
    CORE::TDataTableStatus ParseXmlPart( const CORE::CString& partName, CXlsxXmlCallbacks& callbacks, CORE::CString& error );

    /** Cuts the rest of the part that is being parsed off, for a parser that has found its answer or a failure */
    void AbortParse( void );

    /** The bytes of all parts that were read so far */
    UInt64 GetBytesRead( void ) const;

    /** The size of the .xlsx file */
    UInt64 GetFileBytes( void ) const;

    /** The file in the temporary directory and the name of the mount of the VFS, empty when the package is not open */
    const CORE::CString& GetFilePath( void ) const;
    const CORE::CString& GetMountName( void ) const;

    /** The packages that hold a temporary file and a mount right now: 0 when every package was destroyed */
    static UInt32 GetOpenPackageCount( void );

    /** The part name that a relationship target means, relative to the folder of the part that holds it; false for a target that leaves the package */
    static bool ResolveTarget( const CORE::CString& basePart, const CORE::CString& target, CORE::CString& partName );

    private:

    CXlsxPackage( const CXlsxPackage& src );                /**< not implemented */
    CXlsxPackage& operator=( const CXlsxPackage& src );     /**< not implemented */

    CORE::TDataTableStatus Spill( CORE::CIOAccess& source, CORE::CString& error );
    CORE::TDataTableStatus CheckEntries( CORE::CString& error );
    void Release( void );

    SXlsxReadOptions m_options;
    CORE::CString m_mountName;
    CORE::CString m_filePath;
    bool m_mounted;
    UInt64 m_fileBytes;
    UInt64 m_budget;                    /**< bytes the parts may still be read */
    UInt64 m_bytesRead;
    CXlsxLimitedAccess* m_activeInput;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSX */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_XLSX_CXLSXPACKAGE_H ? */
