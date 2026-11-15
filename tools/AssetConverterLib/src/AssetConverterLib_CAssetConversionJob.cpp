/*
 *  AssetConverterLib: the conversion of one asset: its input and its results, the data of its task
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "AssetConverterLib_CAssetConversionJob.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetConversionJob::CAssetConversionJob( void )
    : CORE::CICloneable()
    , manifest( GUCEF_NULL )
    , context( GUCEF_NULL )
    , baker( GUCEF_NULL )
    , importer()
    , exporter()
    , cookerVersions()
    , processingHash()
    , maxBones( 0 )
    , force( false )
    , reimport( false )
    , validateOnly( false )
    , assetPath()
    , report()
    , activity()
    , cacheEntry()
    , cached( false )
    , converted( false )
    , cookAllowed( false )
    , cookPending( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetConversionJob::CAssetConversionJob( const CAssetConversionJob& src )
    : CORE::CICloneable( src )
    , manifest( src.manifest )
    , context( src.context )
    , baker( src.baker )
    , importer( src.importer )
    , exporter( src.exporter )
    , cookerVersions( src.cookerVersions )
    , processingHash( src.processingHash )
    , maxBones( src.maxBones )
    , force( src.force )
    , reimport( src.reimport )
    , validateOnly( src.validateOnly )
    , assetPath( src.assetPath )
    , report( src.report )
    , activity( src.activity )
    , cacheEntry( src.cacheEntry )
    , cached( src.cached )
    , converted( src.converted )
    , cookAllowed( src.cookAllowed )
    , cookPending( src.cookPending )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetConversionJob::~CAssetConversionJob()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetConversionJob&
CAssetConversionJob::operator=( const CAssetConversionJob& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        manifest = src.manifest;
        context = src.context;
        baker = src.baker;
        importer = src.importer;
        exporter = src.exporter;
        cookerVersions = src.cookerVersions;
        processingHash = src.processingHash;
        maxBones = src.maxBones;
        force = src.force;
        reimport = src.reimport;
        validateOnly = src.validateOnly;
        assetPath = src.assetPath;
        report = src.report;
        activity = src.activity;
        cacheEntry = src.cacheEntry;
        cached = src.cached;
        converted = src.converted;
        cookAllowed = src.cookAllowed;
        cookPending = src.cookPending;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAssetConversionJob::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAssetConversionJob( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CAssetConversionJob::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAssetConversionJob::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::ASSETCONVERTER::CAssetConversionJob";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
