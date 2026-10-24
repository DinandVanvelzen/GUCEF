/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MULTIPLAY_DATANODEUTILS_H
#define GUCEF_MULTIPLAY_DATANODEUTILS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Writes a value list as the attributes of a child node with the given name, one value per key: the form that
 *  survives every data tree codec, JSON included
 */
inline void
WriteValueListAsChild( CORE::CDataNode& parent, const CORE::CString& childName, const CORE::CValueList& values )
{GUCEF_TRACE;

    CORE::CDataNode* child = parent.AddChild( childName, GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL == child )
        return;
    UInt32 keyCount = values.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
    {
        const CORE::CValueList::TVariantVector& keyValues = values.GetValueVector( i );
        child->SetAttribute( values.GetKey( i ).AsString(), keyValues.empty() ? CORE::CString::Empty : keyValues[ 0 ].AsString() );
    }
}

/*-------------------------------------------------------------------------*/

/** Reads what WriteValueListAsChild() wrote */
inline void
ReadValueListFromChild( const CORE::CDataNode& parent, const CORE::CString& childName, CORE::CValueList& values )
{GUCEF_TRACE;

    values.DeleteAll();
    const CORE::CDataNode* child = parent.FindChild( childName );
    if ( GUCEF_NULL == child )
        return;
    CORE::CDataNode::TAttributeMap::const_iterator i = child->AttributeBegin();
    while ( i != child->AttributeEnd() )
    {
        values.Set( (*i).first, (*i).second.AsString() );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

/** Writes a list of strings as an array child */
inline void
WriteStringsAsChild( CORE::CDataNode& parent, const CORE::CString& childName, const CORE::CString::StringVector& values )
{GUCEF_TRACE;

    CORE::CDataNode* child = parent.AddChild( childName, GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == child )
        return;
    for ( size_t i=0; i<values.size(); ++i )
        child->AddValueAsChild( values[ i ] );
}

/*-------------------------------------------------------------------------*/

/** Reads what WriteStringsAsChild() wrote */
inline void
ReadStringsFromChild( const CORE::CDataNode& parent, const CORE::CString& childName, CORE::CString::StringVector& values )
{GUCEF_TRACE;

    values.clear();
    const CORE::CDataNode* child = parent.FindChild( childName );
    if ( GUCEF_NULL == child )
        return;
    CORE::CDataNode::TVariantVector childValues = child->GetChildrenValues();
    values.reserve( childValues.size() );
    for ( size_t i=0; i<childValues.size(); ++i )
        values.push_back( childValues[ i ].AsString() );
}

/*-------------------------------------------------------------------------*/

/** A monotonic clock in milliseconds */
inline UInt64
GetTimeInMs( void )
{GUCEF_TRACE;

    UInt64 resolution = MT::PrecisionTimerResolution();
    UInt64 ticks = MT::PrecisionTickCount();
    if ( resolution >= 1000 )
        return ticks / ( resolution / 1000 );
    if ( 0 == resolution )
        return ticks;
    return ticks * ( 1000 / resolution );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_DATANODEUTILS_H ? */
