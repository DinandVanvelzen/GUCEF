/*
 *  AssetConverterLib: the state of one step for one resource of the pack index
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#include "AssetConverterLib_CPackIndexSerial.h"
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

#include "AssetConverterLib_CPackIndexStepStatus.h"

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

const CORE::CString CPackIndexStepStatus::Fresh = "fresh";
const CORE::CString CPackIndexStepStatus::Stale = "stale";
const CORE::CString CPackIndexStepStatus::Missing = "missing";
const CORE::CString CPackIndexStepStatus::Failed = "failed";
const CORE::CString CPackIndexStepStatus::Refused = "refused";
const CORE::CString CPackIndexStepStatus::Skipped = "skipped";

/*-------------------------------------------------------------------------*/

CPackIndexStepStatus::CPackIndexStepStatus( void )
    : state( Missing )
    , key()
    , reasons()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPackIndexStepStatus::CPackIndexStepStatus( const CORE::CString& stepState )
    : state( stepState )
    , key()
    , reasons()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CPackIndexStepStatus::operator==( const CPackIndexStepStatus& other ) const
{GUCEF_TRACE;

    return state == other.state && key == other.key && reasons == other.reasons;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexStepStatus::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "s", state );
    CPackIndexSerial::SetText( node, "key", key );
    CPackIndexSerial::SetList( node, "reasons", reasons );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexStepStatus::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    state = CPackIndexSerial::GetText( node, "s" );
    key = CPackIndexSerial::GetText( node, "key" );
    CPackIndexSerial::GetList( node, "reasons", reasons );
    return !state.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
