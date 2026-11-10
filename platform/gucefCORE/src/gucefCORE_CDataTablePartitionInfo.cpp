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


#include "gucefCORE_CDataTablePartitionInfo.h"

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

CDataTablePartitionInfo::CDataTablePartitionInfo( void )
    : CICloneable()
    , path()
    , keyValues()
    , hasFileStats( false )
    , fileSize( 0 )
    , modificationTime()
    , rowCount( GUCEF_UINT64MAX )
    , rowGroupCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePartitionInfo::CDataTablePartitionInfo( const CDataTablePartitionInfo& src )
    : CICloneable( src )
    , path( src.path )
    , keyValues( src.keyValues )
    , hasFileStats( src.hasFileStats )
    , fileSize( src.fileSize )
    , modificationTime( src.modificationTime )
    , rowCount( src.rowCount )
    , rowGroupCount( src.rowGroupCount )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePartitionInfo::~CDataTablePartitionInfo()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePartitionInfo&
CDataTablePartitionInfo::operator=( const CDataTablePartitionInfo& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        path = src.path;
        keyValues = src.keyValues;
        hasFileStats = src.hasFileStats;
        fileSize = src.fileSize;
        modificationTime = src.modificationTime;
        rowCount = src.rowCount;
        rowGroupCount = src.rowGroupCount;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTablePartitionInfo::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTablePartitionInfo( *this );
}

/*-------------------------------------------------------------------------*/

CDataTablePartitionInfoList::CDataTablePartitionInfoList( void )
    : CICloneable()
    , partitions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePartitionInfoList::CDataTablePartitionInfoList( const CDataTablePartitionInfoList& src )
    : CICloneable( src )
    , partitions( src.partitions )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePartitionInfoList::~CDataTablePartitionInfoList()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTablePartitionInfoList&
CDataTablePartitionInfoList::operator=( const CDataTablePartitionInfoList& src )
{GUCEF_TRACE;

    if ( &src != this )
        partitions = src.partitions;
    return *this;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTablePartitionInfoList::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTablePartitionInfoList( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
