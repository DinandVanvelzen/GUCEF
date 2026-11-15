/*
 *  AssetConverterLib: the interface of the checks of an asset before it is cooked
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

#include "AssetConverterLib_CIAssetValidator.h"

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

CIAssetValidator::CIAssetValidator( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIAssetValidator::~CIAssetValidator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TReportItemKind
CIAssetValidator::GetSeverity( const CORE::CDataNode& settings, TReportItemKind defaultSeverity )
{GUCEF_TRACE;

    TReportItemKind severity = defaultSeverity;
    if ( FromString( settings.GetAttributeValueOrChildValueByName( "severity" ).AsString(), severity ) )
    {
        if ( REPORTITEM_INFO == severity || REPORTITEM_WARNING == severity || REPORTITEM_ERROR == severity )
            return severity;
    }
    return defaultSeverity;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
