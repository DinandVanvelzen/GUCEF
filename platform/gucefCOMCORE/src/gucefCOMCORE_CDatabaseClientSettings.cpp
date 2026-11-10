/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefCOMCORE_CDatabaseClientSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Stores a value list as an object child, one attribute per key with its first value, when it holds anything */
bool
SaveValueList( CORE::CDataNode& parent, const char* name, const CORE::CValueList& values )
{
    if ( 0 == values.GetKeyCount() )
        return true;
    CORE::CDataNode* node = parent.AddChild( name, GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL == node )
        return false;
    bool success = true;
    CORE::CValueList::TValueMap::const_iterator i = values.GetDataBeginIterator();
    while ( i != values.GetDataEndIterator() )
    {
        if ( !(*i).second.empty() )
            success = node->SetAttribute( (*i).first.AsString(), (*i).second.front() ) && success;
        ++i;
    }
    return success;
}

/** Loads a value list from the attributes and the valued children of an object child, when there is one */
void
LoadValueList( const CORE::CDataNode& parent, const char* name, CORE::CValueList& values )
{
    values.Clear();
    const CORE::CDataNode* node = parent.FindChild( name );
    if ( GUCEF_NULL == node )
        return;
    CORE::CDataNode::TAttributeMap::const_iterator a = node->AttributeBegin();
    while ( a != node->AttributeEnd() )
    {
        values.Delete( (*a).first );
        values.Set( (*a).first, (*a).second );
        ++a;
    }
    CORE::CDataNode::const_iterator n = node->ConstBegin();
    while ( n != node->ConstEnd() )
    {
        if ( (*n)->HasValue() )
        {
            values.Delete( (*n)->GetName() );
            values.Set( (*n)->GetName(), (*n)->GetValue() );
        }
        ++n;
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDatabaseClientSettings::CDatabaseClientSettings( void )
    : CORE::CIDataNodeSerializable()
    , backendName()
    , database()
    , readOnly( false )
    , threadPoolName( "DataTables" )
    , bufferPoolMaxBytes( DefaultBufferPoolMaxBytes )
    , backendOptions()
    , pulseGenerator()
    , threadPool()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseClientSettings::CDatabaseClientSettings( const CDatabaseClientSettings& src )
    : CORE::CIDataNodeSerializable( src )
    , backendName( src.backendName )
    , database( src.database )
    , readOnly( src.readOnly )
    , threadPoolName( src.threadPoolName )
    , bufferPoolMaxBytes( src.bufferPoolMaxBytes )
    , backendOptions( src.backendOptions )
    , pulseGenerator( src.pulseGenerator )
    , threadPool( src.threadPool )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseClientSettings::~CDatabaseClientSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDatabaseClientSettings&
CDatabaseClientSettings::operator=( const CDatabaseClientSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIDataNodeSerializable::operator=( src );
        backendName = src.backendName;
        database = src.database;
        readOnly = src.readOnly;
        threadPoolName = src.threadPoolName;
        bufferPoolMaxBytes = src.bufferPoolMaxBytes;
        backendOptions = src.backendOptions;
        pulseGenerator = src.pulseGenerator;
        threadPool = src.threadPool;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDatabaseClientSettings::operator==( const CDatabaseClientSettings& other ) const
{GUCEF_TRACE;

    if ( backendName != other.backendName || database != other.database || readOnly != other.readOnly ||
         threadPoolName != other.threadPoolName || bufferPoolMaxBytes != other.bufferPoolMaxBytes )
        return false;

    // the options are equal when they have the same keys with the same first values
    if ( backendOptions.GetKeyCount() != other.backendOptions.GetKeyCount() )
        return false;
    CORE::CValueList::TValueMap::const_iterator i = backendOptions.GetDataBeginIterator();
    while ( i != backendOptions.GetDataEndIterator() )
    {
        CORE::CVariant otherValue;
        if ( !other.backendOptions.TryGetValue( (*i).first, otherValue ) )
            return false;
        if ( (*i).second.empty() || (*i).second.front().AsString() != otherValue.AsString() )
            return false;
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDatabaseClientSettings::operator!=( const CDatabaseClientSettings& other ) const
{GUCEF_TRACE;

    return !( *this == other );
}

/*-------------------------------------------------------------------------*/

bool
CDatabaseClientSettings::Validate( CORE::CString& error ) const
{GUCEF_TRACE;

    if ( backendName.IsNULLOrEmpty() )
    {
        error = "The settings name no backend";
        return false;
    }
    if ( database.IsNULLOrEmpty() )
    {
        error = "The settings name no database";
        return false;
    }
    if ( threadPoolName.IsNULLOrEmpty() && threadPool.IsNULL() )
    {
        error = "The settings have no thread pool name";
        return false;
    }
    if ( 0 == bufferPoolMaxBytes )
    {
        error = "The buffer pool budget is zero";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDatabaseClientSettings::Serialize( CORE::CDataNode& domRootNode                        ,
                                    const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    bool success = true;
    success = domRootNode.SetAttribute( "backendName", backendName ) && success;
    success = domRootNode.SetAttribute( "database", database ) && success;
    success = domRootNode.SetAttribute( "readOnly", readOnly ) && success;
    success = domRootNode.SetAttribute( "threadPoolName", threadPoolName ) && success;
    success = domRootNode.SetAttribute( "bufferPoolMaxBytes", bufferPoolMaxBytes ) && success;
    success = SaveValueList( domRootNode, "backendOptions", backendOptions ) && success;
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CDatabaseClientSettings::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                      const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CDatabaseClientSettings loaded;
    loaded.backendName = domRootNode.GetAttributeValueOrChildValueByName( "backendName" ).AsString( loaded.backendName, true );
    loaded.database = domRootNode.GetAttributeValueOrChildValueByName( "database" ).AsString( loaded.database, true );
    loaded.readOnly = domRootNode.GetAttributeValueOrChildValueByName( "readOnly" ).AsBool( loaded.readOnly, true );
    loaded.threadPoolName = domRootNode.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( loaded.threadPoolName, true );
    loaded.bufferPoolMaxBytes = domRootNode.GetAttributeValueOrChildValueByName( "bufferPoolMaxBytes" ).AsUInt64( loaded.bufferPoolMaxBytes, true );
    LoadValueList( domRootNode, "backendOptions", loaded.backendOptions );

    // the code that opens the client gives what is not serialized
    loaded.pulseGenerator = pulseGenerator;
    loaded.threadPool = threadPool;

    CORE::CString error;
    if ( !loaded.Validate( error ) )
        return false;

    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CDatabaseClientSettings::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDatabaseClientSettings( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDatabaseClientSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CDatabaseClientSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
