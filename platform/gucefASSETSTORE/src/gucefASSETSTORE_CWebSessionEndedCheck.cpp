/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_ASSETSTORE_CWEBSESSIONENDEDCHECK_H
#include "gucefASSETSTORE_CWebSessionEndedCheck.h"
#define GUCEF_ASSETSTORE_CWEBSESSIONENDEDCHECK_H
#endif /* GUCEF_ASSETSTORE_CWEBSESSIONENDEDCHECK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebSessionEndedCheck::CWebSessionEndedCheck( void )
    : CIWebSessionEndedCheck()
    , m_statusCodes()
    , m_redirectPattern()
{GUCEF_TRACE;

    m_statusCodes.push_back( 401 );
}

/*-------------------------------------------------------------------------*/

CWebSessionEndedCheck::~CWebSessionEndedCheck()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWebSessionEndedCheck::Configure( const CWebLoginScript& script )
{GUCEF_TRACE;

    m_statusCodes = script.sessionEndedStatusCodes;
    if ( m_statusCodes.empty() )
        m_statusCodes.push_back( 401 );
    m_redirectPattern = script.sessionEndedRedirectPattern;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionEndedCheck::SetStatusCodes( const CWebLoginScript::TUInt32Vector& statusCodes )
{GUCEF_TRACE;

    m_statusCodes = statusCodes;
}

/*-------------------------------------------------------------------------*/

const CWebLoginScript::TUInt32Vector&
CWebSessionEndedCheck::GetStatusCodes( void ) const
{GUCEF_TRACE;

    return m_statusCodes;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionEndedCheck::SetRedirectPattern( const CORE::CString& pattern )
{GUCEF_TRACE;

    m_redirectPattern = pattern;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWebSessionEndedCheck::GetRedirectPattern( void ) const
{GUCEF_TRACE;

    return m_redirectPattern;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionEndedCheck::IsSessionEnded( const WEB::CHttpResponseData& response ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<m_statusCodes.size(); ++i )
        if ( (UInt32) response.statusCode == m_statusCodes[ i ] )
            return true;

    if ( !m_redirectPattern.IsNULLOrEmpty() && response.statusCode >= 300 && response.statusCode < 400 && !response.location.IsNULLOrEmpty() )
        return CWebLoginScriptCondition::MatchesPattern( response.location, m_redirectPattern );
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
