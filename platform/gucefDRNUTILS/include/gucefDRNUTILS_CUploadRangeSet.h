/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CUPLOADRANGESET_H
#define GUCEF_DRNUTILS_CUPLOADRANGESET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The byte ranges of a file that arrived, merged and ordered: what an interrupted upload continues from */
class GUCEF_DRNUTILS_PUBLIC_CPP CUploadRangeSet
{
    public:

    struct SRange
    {
        UInt64 offset;
        UInt64 length;
    };
    typedef GUCEF::vector< SRange > TRangeVector;

    CUploadRangeSet( void );

    /** Adds [offset, offset + length), merging with what touches or overlaps it */
    void Add( UInt64 offset, UInt64 length );

    /** Whether every byte of [offset, offset + length) is held */
    bool Contains( UInt64 offset, UInt64 length ) const;

    /** Bytes held */
    UInt64 GetTotalBytes( void ) const;

    /** Whether [0, size) is held entirely (an empty file is complete) */
    bool IsComplete( UInt64 size ) const;

    /** The ranges held, ordered */
    const TRangeVector& GetRanges( void ) const;

    /** The ranges of [0, size) that are not held */
    void GetMissing( UInt64 size, TRangeVector& missing ) const;

    void Clear( void );

    /** "0-262143,524288-786431" (inclusive ends); empty for no range */
    CORE::CString ToString( void ) const;
    bool FromString( const CORE::CString& text );

    private:

    TRangeVector m_ranges;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CUPLOADRANGESET_H ? */
