/*
 *  AssetConverterLib: the status of one resource of the pack index: its level and the state of its steps
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

#include "AssetConverterLib_CPackIndexStatus.h"

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

const CORE::CString CPackIndexStatus::Indexed = "indexed";
const CORE::CString CPackIndexStatus::Analysed = "analysed";
const CORE::CString CPackIndexStatus::Converted = "converted";
const CORE::CString CPackIndexStatus::Failed = "failed";
const CORE::CString CPackIndexStatus::Refused = "refused";
const CORE::CString CPackIndexStatus::StepAnalyse = "analyse";
const CORE::CString CPackIndexStatus::StepImport = "import";
const CORE::CString CPackIndexStatus::StepConvert = "convert";
const CORE::CString CPackIndexStatus::StepThumbnail = "thumbnail";
const CORE::CString CPackIndexStatus::StepCook = "cook";

/*-------------------------------------------------------------------------*/

CPackIndexStatus::CPackIndexStatus( void )
    : state( Indexed )
    , steps()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CPackIndexStepStatus*
CPackIndexStatus::FindStep( const CORE::CString& step ) const
{GUCEF_TRACE;

    TStepMap::const_iterator i = steps.find( step );
    return i != steps.end() ? &(*i).second : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CPackIndexStatus::SetStep( const CORE::CString& step, const CPackIndexStepStatus& status )
{GUCEF_TRACE;

    steps[ step ] = status;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexStatus::operator==( const CPackIndexStatus& other ) const
{GUCEF_TRACE;

    return state == other.state && steps == other.steps;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexStatus::Serialize( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CPackIndexSerial::SetTextAlways( node, "state", state );
    if ( !steps.empty() )
    {
        CORE::CDataNode* stepNode = node.AddChild( "steps", GUCEF_DATATYPE_OBJECT );
        TStepMap::const_iterator i = steps.begin();
        while ( i != steps.end() )
        {
            (*i).second.Serialize( *stepNode->AddChild( (*i).first, GUCEF_DATATYPE_OBJECT ) );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPackIndexStatus::Deserialize( const CORE::CDataNode& node )
{GUCEF_TRACE;

    state = CPackIndexSerial::GetText( node, "state" );
    steps.clear();
    const CORE::CDataNode* stepNode = node.FindChild( "steps" );
    if ( GUCEF_NULL != stepNode )
    {
        CORE::CDataNode::const_iterator i = stepNode->ConstBegin();
        while ( i != stepNode->ConstEnd() )
        {
            CPackIndexStepStatus step;
            if ( step.Deserialize( *(*i) ) )
                steps[ (*i)->GetName() ] = step;
            ++i;
        }
    }
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
