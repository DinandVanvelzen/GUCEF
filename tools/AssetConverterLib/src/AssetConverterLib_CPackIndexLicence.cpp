/*
 *  AssetConverterLib: the licence reference and restriction flags of a pack in the pack index
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

#include <algorithm>

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#include "AssetConverterLib_CPackIndexLicence.h"

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

CPackIndexLicence::CPackIndexLicence( void )
    : recordId()
    , licenceId()
    , licenceClass()
    , restrictions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexLicence::IsEmpty( void ) const
{GUCEF_TRACE;

    return recordId.IsNULLOrEmpty() && licenceId.IsNULLOrEmpty() && licenceClass.IsNULLOrEmpty() && restrictions.empty();
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexLicence::HasRestriction( const CORE::CString& flag ) const
{GUCEF_TRACE;

    return std::find( restrictions.begin(), restrictions.end(), flag ) != restrictions.end();
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexLicence::operator==( const CPackIndexLicence& other ) const
{GUCEF_TRACE;

    return recordId == other.recordId && licenceId == other.licenceId && licenceClass == other.licenceClass && restrictions == other.restrictions;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexLicence::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetText( node, "recordId", recordId );
    CPackIndexSerial::SetText( node, "licenceId", licenceId );
    CPackIndexSerial::SetText( node, "class", licenceClass );
    CORE::CString::StringVector sorted = restrictions;
    std::sort( sorted.begin(), sorted.end() );
    sorted.erase( std::unique( sorted.begin(), sorted.end() ), sorted.end() );
    node.SetValuesOfChildByName( "restrictions", sorted );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexLicence::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    recordId = CPackIndexSerial::GetText( node, "recordId" );
    licenceId = CPackIndexSerial::GetText( node, "licenceId" );
    licenceClass = CPackIndexSerial::GetText( node, "class" );
    CPackIndexSerial::GetList( node, "restrictions", restrictions );
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
