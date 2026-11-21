/*
 *  AssetConverterLib: the read and write helpers the pack index classes share (absent members are unknown, never zero)
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

#ifndef ASSETCONVERTERLIB_CPACKINDEXSERIAL_H
#define ASSETCONVERTERLIB_CPACKINDEXSERIAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Helpers of the pack index classes that turn members into data tree members and back. A write helper writes nothing for an
 *  empty text, an empty list or a zero count where the schema says "absent means unknown", so an unknown value is never
 *  written as zero; a read helper returns the default for a member that is not there.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackIndexSerial
{
    public:

    /** Writes the text when it is not empty */
    static void SetText( CORE::CDataNode& node, const char* name, const CORE::CString& value );

    /** Writes the text always (members that are a state) */
    static void SetTextAlways( CORE::CDataNode& node, const char* name, const CORE::CString& value );

    /** Writes the count when it is not zero */
    static void SetCount( CORE::CDataNode& node, const char* name, UInt64 value );

    /** Writes the boolean only when it is true */
    static void SetFlag( CORE::CDataNode& node, const char* name, bool value );

    /** Writes the strings as an array in the order given, when there are any */
    static void SetList( CORE::CDataNode& node, const char* name, const CORE::CString::StringVector& values );

    /** Writes the strings as an array, sorted (the canonical order of tags and restrictions) and without duplicates */
    static void SetSortedList( CORE::CDataNode& node, const char* name, const CORE::CString::StringVector& values );

    static CORE::CString GetText( const CORE::CDataNode& node, const char* name );
    static UInt64 GetCount( const CORE::CDataNode& node, const char* name, UInt64 defaultValue = 0 );
    static Int64 GetInt( const CORE::CDataNode& node, const char* name, Int64 defaultValue = 0 );
    static bool GetFlag( const CORE::CDataNode& node, const char* name );
    static bool HasMember( const CORE::CDataNode& node, const char* name );
    static void GetList( const CORE::CDataNode& node, const char* name, CORE::CString::StringVector& values );

    /** The lower case hexadecimal SHA-256 of the text, as every key of the index is made */
    static CORE::CString HashParts( const CORE::CString::StringVector& parts );

    private:

    CPackIndexSerial( void );
    ~CPackIndexSerial();
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKINDEXSERIAL_H ? */

