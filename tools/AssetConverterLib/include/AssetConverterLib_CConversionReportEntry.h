/*
 *  AssetConverterLib: the report of one asset
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

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORTENTRY_H
#define ASSETCONVERTERLIB_CCONVERSIONREPORTENTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORTITEM_H
#include "AssetConverterLib_CConversionReportItem.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORTITEM_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORTITEM_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORTFILE_H
#include "AssetConverterLib_CConversionReportFile.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORTFILE_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORTFILE_H ? */


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
 *  The report of one asset: its path in the pack, the importer, its findings and the files written for it. The
 *  status follows from them (GetStatus()) unless it was set: cached and refused assets are marked so.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CConversionReportEntry
{
    public:

    typedef GUCEF::vector< CConversionReportItem > TItemVector;
    typedef GUCEF::vector< CConversionReportFile > TFileVector;

    CORE::CString assetPath;
    CORE::CString importer;
    TItemVector items;
    TFileVector files;
    bool cached;
    bool refused;
    bool skipped;
    bool cookBlocked;           /**< validation errors keep its cooked files from being made (unless forced) */
    UInt32 durationInMs;

    CConversionReportEntry( void );

    /** Refused, cached, failed (any error), skipped, approximated (any approximation) or converted, in that order */
    TAssetStatus GetStatus( void ) const;

    UInt32 GetItemCount( TReportItemKind kind ) const;

    bool HasErrors( void ) const;

    /** Adds the file, or adds the references to the file of the same path and profile that is there */
    void AddFile( const CConversionReportFile& file );

    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORTENTRY_H ? */
