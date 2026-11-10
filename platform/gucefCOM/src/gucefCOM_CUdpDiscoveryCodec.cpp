/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_NETWORKBYTEORDER_H
#include "gucefCOM_NetworkByteOrder.h"
#define GUCEF_COM_NETWORKBYTEORDER_H
#endif /* GUCEF_COM_NETWORKBYTEORDER_H ? */

#include "gucefCOM_CUdpDiscoveryCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt8 CUdpDiscoveryCodec::ProtocolVersion = 1;
const UInt32 CUdpDiscoveryCodec::MaxBeaconSizeInBytes = 1400;

static const UInt8 BeaconMagic[ 4 ] = { 'G', 'D', 'S', 'C' };

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
WriteString( CNetworkByteWriter& writer, const CORE::CString& str )
{GUCEF_TRACE;

    UInt32 length = CNetworkByteWriter::GetStringByteLength( str );
    if ( length > 0xFFFF )
        return false;
    writer.WriteUInt16( (UInt16) length );
    writer.WriteStringBytes( str );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
ReadString( CNetworkByteReader& reader, CORE::CString& str )
{GUCEF_TRACE;

    UInt16 length = 0;
    return reader.ReadUInt16( length ) && reader.ReadString( length, str );
}

/*-------------------------------------------------------------------------*/

void
WriteHeader( CNetworkByteWriter& writer, CUdpDiscoveryCodec::TBeaconKind kind )
{GUCEF_TRACE;

    writer.WriteBytes( BeaconMagic, 4 );
    writer.WriteUInt8( CUdpDiscoveryCodec::ProtocolVersion );
    writer.WriteUInt8( (UInt8) kind );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CUdpDiscoveryCodec::IsBeacon( const void* data, UInt32 dataSize )
{GUCEF_TRACE;

    return GUCEF_NULL != data && dataSize >= 6 && 0 == memcmp( data, BeaconMagic, 4 );
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryCodec::EncodeQuery( const CORE::CString& serviceType ,
                                 CORE::CDynamicBuffer& beacon     )
{GUCEF_TRACE;

    beacon.SetDataSize( 0 );
    CNetworkByteWriter writer( beacon );
    WriteHeader( writer, BEACONKIND_QUERY );
    if ( !WriteString( writer, serviceType ) )
        return false;

    // The rest of the fields, empty, so every beacon has the same layout
    writer.WriteUInt16( 0 );    // instance id
    writer.WriteUInt16( 0 );    // instance name
    writer.WriteUInt16( 0 );    // ttl
    writer.WriteUInt16( 0 );    // endpoints
    writer.WriteUInt16( 0 );    // attributes
    return writer.GetSize() <= MaxBeaconSizeInBytes;
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryCodec::EncodeEntry( TBeaconKind kind                         ,
                                 const COMCORE::CIDiscoveryEntry& entry   ,
                                 CORE::CDynamicBuffer& beacon             )
{GUCEF_TRACE;

    beacon.SetDataSize( 0 );
    CNetworkByteWriter writer( beacon );
    WriteHeader( writer, kind );

    CORE::CString instanceId;
    entry.GetApplicationInstanceId( instanceId );

    if ( !WriteString( writer, entry.GetServiceType() )  ||
         !WriteString( writer, instanceId )              ||
         !WriteString( writer, entry.GetInstanceName() )  )
        return false;

    UInt32 ttl = BEACONKIND_GOODBYE == kind ? 0 : entry.GetTtlInSecs();
    writer.WriteUInt16( ttl > 0xFFFF ? (UInt16) 0xFFFF : (UInt16) ttl );

    const CORE::CString::StringVector& endpoints = entry.GetEndpoints();
    if ( endpoints.size() > 0xFFFF )
        return false;
    writer.WriteUInt16( (UInt16) endpoints.size() );
    for ( size_t i=0; i<endpoints.size(); ++i )
    {
        if ( !WriteString( writer, endpoints[ i ] ) )
            return false;
    }

    // Each value of a key is a pair of its own, which keeps keys with multiple values intact
    UInt32 countOffset = writer.GetSize();
    writer.WriteUInt16( 0 );
    UInt32 pairCount = 0;
    const CORE::CValueList& attributes = entry.GetAttributes();
    UInt32 keyCount = attributes.GetKeyCount();
    for ( UInt32 k=0; k<keyCount; ++k )
    {
        CORE::CString key = attributes.GetKey( k ).AsString();
        const CORE::CValueList::TVariantVector& values = attributes.GetValueVector( k );
        for ( size_t v=0; v<values.size(); ++v )
        {
            if ( !WriteString( writer, key ) || !WriteString( writer, values[ v ].AsString() ) )
                return false;
            ++pairCount;
        }
        if ( writer.GetSize() > MaxBeaconSizeInBytes )
            return false;
    }
    if ( pairCount > 0xFFFF )
        return false;
    writer.PatchUInt16( countOffset, (UInt16) pairCount );

    return writer.GetSize() <= MaxBeaconSizeInBytes;
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryCodec::Decode( const void* data                    ,
                            UInt32 dataSize                     ,
                            TBeaconKind& kind                   ,
                            COMCORE::CDiscoveryEntry& entry     )
{GUCEF_TRACE;

    if ( !IsBeacon( data, dataSize ) )
        return false;

    CNetworkByteReader reader( data, dataSize );
    reader.Skip( 4 );

    UInt8 version = 0;
    UInt8 kindValue = 0;
    if ( !reader.ReadUInt8( version ) || !reader.ReadUInt8( kindValue ) )
        return false;

    // Newer versions only add fields at the end, so what we know of them we can still read
    if ( version < 1 )
        return false;
    if ( kindValue < BEACONKIND_QUERY || kindValue > BEACONKIND_GOODBYE )
        return false;
    kind = (TBeaconKind) kindValue;

    CORE::CString str;
    if ( !ReadString( reader, str ) )
        return false;
    entry.SetServiceType( str );
    if ( !ReadString( reader, str ) )
        return false;
    entry.SetApplicationInstanceId( str );
    if ( !ReadString( reader, str ) )
        return false;
    entry.SetInstanceName( str );

    UInt16 ttl = 0;
    if ( !reader.ReadUInt16( ttl ) )
        return false;
    entry.SetTtlInSecs( ttl );

    UInt16 endpointCount = 0;
    if ( !reader.ReadUInt16( endpointCount ) )
        return false;
    CORE::CString::StringVector& endpoints = entry.GetEndpoints();
    endpoints.clear();
    endpoints.reserve( endpointCount );
    for ( UInt16 i=0; i<endpointCount; ++i )
    {
        if ( !ReadString( reader, str ) )
            return false;
        endpoints.push_back( str );
    }

    UInt16 pairCount = 0;
    if ( !reader.ReadUInt16( pairCount ) )
        return false;
    CORE::CValueList& attributes = entry.GetAttributes();
    attributes.DeleteAll();
    attributes.SetAllowMultipleValues( true );
    CORE::CString value;
    for ( UInt16 i=0; i<pairCount; ++i )
    {
        if ( !ReadString( reader, str ) || !ReadString( reader, value ) )
            return false;
        attributes.Set( str, value );
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
