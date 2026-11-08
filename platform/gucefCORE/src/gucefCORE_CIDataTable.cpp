/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "gucefCORE_CIDataTable.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CEvent CIDataTable::PartitionWrittenEvent = "GUCEF::CORE::CIDataTable::PartitionWrittenEvent";
const CEvent CIDataTable::PartitionDeletedEvent = "GUCEF::CORE::CIDataTable::PartitionDeletedEvent";
const CEvent CIDataTable::RowsChangedEvent = "GUCEF::CORE::CIDataTable::RowsChangedEvent";

/*-------------------------------------------------------------------------*/

void
CIDataTable::RegisterEvents( void )
{GUCEF_TRACE;

    PartitionWrittenEvent.Initialize();
    PartitionDeletedEvent.Initialize();
    RowsChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CIDataTable::CIDataTable( PulseGeneratorPtr pulseGenerator )
    : CTSGNotifier( pulseGenerator.IsNULL() ? CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CIDataTable::~CIDataTable()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CIDataTable::GetColumnHints( UInt32 /* columnIndex */, CValueList& /* hints */ ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

const CString&
CIDataTable::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CIDataTable";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
