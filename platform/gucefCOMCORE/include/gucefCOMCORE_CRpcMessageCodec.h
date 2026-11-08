/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCMESSAGECODEC_H
#define GUCEF_COMCORE_CRPCMESSAGECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CBINARYWIREWRITER_H
#include "gucefCORE_CBinaryWireWriter.h"
#define GUCEF_CORE_CBINARYWIREWRITER_H
#endif /* GUCEF_CORE_CBINARYWIREWRITER_H ? */

#ifndef GUCEF_CORE_CBINARYWIREREADER_H
#include "gucefCORE_CBinaryWireReader.h"
#define GUCEF_CORE_CBINARYWIREREADER_H
#endif /* GUCEF_CORE_CBINARYWIREREADER_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The RPC message encoding (gucefCOMCORE RPC design 5): protobuf key and wire types, GUCEF's value mapping
 *  (zig-zag for every signed integer, decimals, dates, timestamps), deterministic output (fields by ascending
 *  number, map entries sorted by key, packed repeated scalars). Stateless: every member is static and reentrant.
 *  Used by CRpcMessage and for the canonical contract schema.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcMessageCodec
{
    public:

    static bool Encode( const CRpcMessage& message, CORE::CBinaryWireWriter& writer, CRpcStatus* status = GUCEF_NULL );

    static bool Decode( CRpcMessage& message, const UInt8* data, UInt32 size, UInt32 depth, bool linkBinary, CRpcStatus& status );

    /**
     *  Converts a value to the stored form of the field type (see CRpcMessage). False when the conversion is not
     *  exact: out of range, an inexact rescale for a fixed-scale decimal, an unknown enum name.
     */
    static bool ConvertValue( const CRpcFieldDescriptor& field, TRpcFieldType type, const CORE::CVariant& in, CORE::CVariant& out );

    /** The value an unset field reads as */
    static CORE::CVariant ZeroValue( const CRpcFieldDescriptor& field, TRpcFieldType type );

    /**
     *  The stored form of a date time: an ISO 8601 string with the local fields and the explicit UTC offset
     *  ("2026-11-08T09:15:30.500+02:00"), so CVariant::AsDateTime() gives back the offset as well
     */
    static CORE::CVariant DateTimeToVariant( const CORE::CDateTime& value );

    /** Order of map keys on the wire: numeric for integers, false before true, byte order for strings */
    static Int32 CompareMapKeys( TRpcFieldType keyType, const CORE::CVariant& a, const CORE::CVariant& b );

    /** The indices of the keys in the order of the wire (CompareMapKeys), equal keys in their given order; n log n */
    static void SortMapKeyOrder( TRpcFieldType keyType, const GUCEF::vector< CORE::CVariant >& keys, GUCEF::vector< UInt32 >& order );

    /** Starts a LEN value: reserves room for the length, returns where the body starts */
    static UInt32 BeginLen( CORE::CBinaryWireWriter& writer );

    /** Ends a LEN value started with BeginLen(), writing the length in front of the body */
    static void EndLen( CORE::CBinaryWireWriter& writer, UInt32 bodyStart );

    static void WriteKey( CORE::CBinaryWireWriter& writer, UInt32 fieldNumber, UInt32 wireType );

    /** Skips one value of the wire type; false for invalid wire types or a value beyond the buffer */
    static bool SkipValue( CORE::CBinaryWireReader& reader, UInt32 wireType );

    private:

    static bool EncodeValue( CORE::CBinaryWireWriter& writer, const CRpcFieldDescriptor& field, TRpcFieldType type,
                             const CORE::CVariant& value, CRpcStatus* status );
    static bool DecodeValue( CORE::CBinaryWireReader& reader, const CRpcFieldDescriptor& field, TRpcFieldType type,
                             bool linkBinary, CORE::CVariant& value, CRpcStatus& status );
    static bool EncodeMessageField( CORE::CBinaryWireWriter& writer, const CRpcMessage& nested, CRpcStatus* status );
    static bool DecodeField( CRpcMessage& message, CORE::CBinaryWireReader& reader, const CRpcFieldDescriptor& field,
                             UInt32 wireType, UInt32 depth, bool linkBinary, CRpcStatus& status );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCMESSAGECODEC_H ? */
