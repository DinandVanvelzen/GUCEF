/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNVARIANTCODEC_H
#define GUCEF_DRN_CDRNVARIANTCODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How the values of a key are reduced before they go on the wire */
class GUCEF_DRN_EXPORT_CPP CDRNQuantisation
{
    public:

    Int32 floatPrecision;   /**< decimal places a float keeps, -1 = full precision */
    UInt32 intBits;         /**< bits an integer keeps (masked / clamped), 0 = all */

    CDRNQuantisation( void );
    CDRNQuantisation( Int32 precision, UInt32 bits );

    bool IsFull( void ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  Writes variants compactly: a tag byte, then integers as varints (signed ones zig-zag), floats as 4 or 8 bytes or,
 *  quantised, as a zig-zag varint of the value times 10^precision, strings and blobs length-prefixed.
 *  The tag is the GUCEF type id of the variant, so the receiver gets the same type back; a few tags are DRN's own.
 *  Numbers are written and read without allocating.
 */
class GUCEF_DRN_EXPORT_CPP CDRNVariantCodec
{
    public:

    static const UInt8 TagQuantisedFloat32  = 228;  /**< u8 precision, zig-zag varint */
    static const UInt8 TagQuantisedFloat64  = 229;  /**< u8 precision, zig-zag varint */
    static const UInt8 TagOther             = 230;  /**< u8 type id, string form */
    static const UInt8 TagRemoved           = 231;  /**< a key that was removed */

    static void Write( CDRNWireWriter& writer, const CORE::CVariant& value, const CDRNQuantisation* quantisation = GUCEF_NULL );

    /** Writes the removal marker of a key */
    static void WriteRemoved( CDRNWireWriter& writer );

    /**
     *  @param isRemoved set when the value is the removal marker, value is then NIL
     */
    static bool Read( CDRNWireReader& reader, CORE::CVariant& value, bool* isRemoved = GUCEF_NULL );

    /** Applies the quantisation to a value the way the receiver will see it */
    static void Quantise( CORE::CVariant& value, const CDRNQuantisation& quantisation );

    /** True when both have the same type and value */
    static bool IsSame( const CORE::CVariant& a, const CORE::CVariant& b );

    private:

    CDRNVariantCodec( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNVARIANTCODEC_H ? */
