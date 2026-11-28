/*
 *  gucefIMAGE: GUCEF module providing image utilities
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
#ifndef GUCEF_IMAGE_CIMAGECONVERSIONRESULT_H
#define GUCEF_IMAGE_CIMAGECONVERSIONRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEFIMAGE_MACROS_H
#include "gucefIMAGE_macros.h"       /* module macro's */
#define GUCEFIMAGE_MACROS_H
#endif /* GUCEFIMAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What came of a conversion by CImageConverter: whether it worked, why not when it did not, and what the converter
 *  had to change to make the image fit the target format. The texts are for the user of the tool or the workbench.
 */
class GUCEF_IMAGE_EXPORT_CPP CImageConversionResult
{
    public:

    CImageConversionResult( void );

    CImageConversionResult( const CImageConversionResult& src );

    ~CImageConversionResult();

    CImageConversionResult& operator=( const CImageConversionResult& src );

    /** Forgets everything of an earlier conversion */
    void Clear( void );

    bool IsSuccess( void ) const;

    /** Why the conversion failed, empty when it did not */
    const CORE::CString& GetProblem( void ) const;

    /** The number of frames of the image that was read */
    UInt32 GetSourceFrameCount( void ) const;

    /** The number of frames that were written, in one output or as separate outputs */
    UInt32 GetWrittenFrameCount( void ) const;

    /** What was changed of the image to fit the target format, and what was lost by that: one text per change */
    const CORE::CString::StringVector& GetNotes( void ) const;

    /** Whether any change lost information */
    bool HasLoss( void ) const;

    void SetFailed( const CORE::CString& problem );

    void SetSuccess( void );

    void SetSourceFrameCount( UInt32 frameCount );

    void SetWrittenFrameCount( UInt32 frameCount );

    void AddNote( const CORE::CString& note, bool lossy );

    private:

    bool m_success;
    CORE::CString m_problem;
    UInt32 m_sourceFrameCount;
    UInt32 m_writtenFrameCount;
    CORE::CString::StringVector m_notes;
    bool m_hasLoss;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_IMAGE_CIMAGECONVERSIONRESULT_H ? */
