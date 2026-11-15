/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#include "gucefCORE_CJobWorkItem.h"

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

SJobWorkItem::SJobWorkItem( void )
    : key()
    , publicLabel()
    , laneName()
    , priority( 1 )
    , estimatedCost( 1 )
    , taskTypeName()
    , payload()
    , taskData( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

SJobWorkItem::SJobWorkItem( const SJobWorkItem& src )
    : key( src.key )
    , publicLabel( src.publicLabel )
    , laneName( src.laneName )
    , priority( src.priority )
    , estimatedCost( src.estimatedCost )
    , taskTypeName( src.taskTypeName )
    , payload( src.payload )
    , taskData( GUCEF_NULL != src.taskData ? src.taskData->Clone() : GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

SJobWorkItem::~SJobWorkItem()
{GUCEF_TRACE;

    GUCEF_DELETE taskData;
    taskData = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

SJobWorkItem&
SJobWorkItem::operator=( const SJobWorkItem& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        key = src.key;
        publicLabel = src.publicLabel;
        laneName = src.laneName;
        priority = src.priority;
        estimatedCost = src.estimatedCost;
        taskTypeName = src.taskTypeName;
        payload = src.payload;
        CICloneable* copy = GUCEF_NULL != src.taskData ? src.taskData->Clone() : GUCEF_NULL;
        GUCEF_DELETE taskData;
        taskData = copy;
    }
    return *this;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
