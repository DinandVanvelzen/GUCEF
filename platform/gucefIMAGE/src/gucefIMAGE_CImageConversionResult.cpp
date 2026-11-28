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
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefIMAGE_CImageConversionResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageConversionResult::CImageConversionResult( void )
    : m_success( false )          ,
      m_problem()                 ,
      m_sourceFrameCount( 0 )     ,
      m_writtenFrameCount( 0 )    ,
      m_notes()                   ,
      m_hasLoss( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageConversionResult::CImageConversionResult( const CImageConversionResult& src )
    : m_success( src.m_success )                    ,
      m_problem( src.m_problem )                    ,
      m_sourceFrameCount( src.m_sourceFrameCount )  ,
      m_writtenFrameCount( src.m_writtenFrameCount ),
      m_notes( src.m_notes )                        ,
      m_hasLoss( src.m_hasLoss )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageConversionResult::~CImageConversionResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageConversionResult&
CImageConversionResult::operator=( const CImageConversionResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_success = src.m_success;
        m_problem = src.m_problem;
        m_sourceFrameCount = src.m_sourceFrameCount;
        m_writtenFrameCount = src.m_writtenFrameCount;
        m_notes = src.m_notes;
        m_hasLoss = src.m_hasLoss;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CImageConversionResult::Clear( void )
{GUCEF_TRACE;

    m_success = false;
    m_problem.Clear();
    m_sourceFrameCount = 0;
    m_writtenFrameCount = 0;
    m_notes.clear();
    m_hasLoss = false;
}

/*-------------------------------------------------------------------------*/

bool
CImageConversionResult::IsSuccess( void ) const
{GUCEF_TRACE;

    return m_success;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImageConversionResult::GetProblem( void ) const
{GUCEF_TRACE;

    return m_problem;
}

/*-------------------------------------------------------------------------*/

UInt32
CImageConversionResult::GetSourceFrameCount( void ) const
{GUCEF_TRACE;

    return m_sourceFrameCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CImageConversionResult::GetWrittenFrameCount( void ) const
{GUCEF_TRACE;

    return m_writtenFrameCount;
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CImageConversionResult::GetNotes( void ) const
{GUCEF_TRACE;

    return m_notes;
}

/*-------------------------------------------------------------------------*/

bool
CImageConversionResult::HasLoss( void ) const
{GUCEF_TRACE;

    return m_hasLoss;
}

/*-------------------------------------------------------------------------*/

void
CImageConversionResult::SetFailed( const CORE::CString& problem )
{GUCEF_TRACE;

    m_success = false;
    m_problem = problem;
}

/*-------------------------------------------------------------------------*/

void
CImageConversionResult::SetSuccess( void )
{GUCEF_TRACE;

    m_success = true;
    m_problem.Clear();
}

/*-------------------------------------------------------------------------*/

void
CImageConversionResult::SetSourceFrameCount( UInt32 frameCount )
{GUCEF_TRACE;

    m_sourceFrameCount = frameCount;
}

/*-------------------------------------------------------------------------*/

void
CImageConversionResult::SetWrittenFrameCount( UInt32 frameCount )
{GUCEF_TRACE;

    m_writtenFrameCount = frameCount;
}

/*-------------------------------------------------------------------------*/

void
CImageConversionResult::AddNote( const CORE::CString& note, bool lossy )
{GUCEF_TRACE;

    m_notes.push_back( note );
    m_hasLoss = m_hasLoss || lossy;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
