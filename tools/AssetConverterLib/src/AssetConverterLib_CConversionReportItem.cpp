/*
 *  AssetConverterLib: one finding of a conversion report
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

#include "AssetConverterLib_CConversionReportItem.h"

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

CConversionReportItem::CConversionReportItem( void )
    : kind( REPORTITEM_INFO )
    , source()
    , subject()
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CConversionReportItem::CConversionReportItem( TReportItemKind itemKind, const CORE::CString& itemSource, const CORE::CString& itemSubject, const CORE::CString& itemMessage )
    : kind( itemKind )
    , source( itemSource )
    , subject( itemSubject )
    , message( itemMessage )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CConversionReportItem::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "kind", ToString( kind ) );
    node.SetAttribute( "source", source );
    node.SetAttribute( "subject", subject );
    node.SetAttribute( "message", message );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CConversionReportItem::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    if ( !FromString( node.GetAttributeValueOrChildValueByName( "kind" ).AsString(), kind ) )
        return false;
    source = node.GetAttributeValueOrChildValueByName( "source" ).AsString();
    subject = node.GetAttributeValueOrChildValueByName( "subject" ).AsString();
    message = node.GetAttributeValueOrChildValueByName( "message" ).AsString();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
