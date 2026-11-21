/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#include "AncestryBackup_CUrlTemplate.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CUrlTemplate::Parse( const CORE::CString& url, SUrlTemplate& result )
{GUCEF_TRACE;

    return WEB::CUrlPathTemplate::Parse( url, result );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlTemplate::TemplatePath( const CORE::CString& path )
{GUCEF_TRACE;

    return WEB::CUrlPathTemplate::TemplatePath( path );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlTemplate::TemplateSegment( const CORE::CString& segment )
{GUCEF_TRACE;

    return WEB::CUrlPathTemplate::TemplateSegment( segment );
}

/*-------------------------------------------------------------------------*/

bool
CUrlTemplate::LooksLikeId( const CORE::CString& segment )
{GUCEF_TRACE;

    return WEB::CUrlPathTemplate::LooksLikeId( segment );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlTemplate::FindIdAfterSegment( const CORE::CString& path, const CORE::CString& segmentName )
{GUCEF_TRACE;

    return WEB::CUrlPathTemplate::FindIdAfterSegment( path, segmentName );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlTemplate::PercentDecode( const CORE::CString& text )
{GUCEF_TRACE;

    return WEB::CUrlPathTemplate::PercentDecode( text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CUrlTemplate::EscapeJson( const CORE::CString& text )
{GUCEF_TRACE;

    return WEB::CUrlPathTemplate::EscapeJson( text );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
