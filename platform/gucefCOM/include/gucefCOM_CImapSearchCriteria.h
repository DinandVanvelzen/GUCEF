/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CIMAPSEARCHCRITERIA_H
#define GUCEF_COM_CIMAPSEARCHCRITERIA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COM_EMAILTYPES_H
#include "gucefCOM_EMailTypes.h"
#define GUCEF_COM_EMAILTYPES_H
#endif /* GUCEF_COM_EMAILTYPES_H ? */

#ifndef GUCEF_COM_CIMAPCOMMANDPARTS_H
#include "gucefCOM_CImapCommandParts.h"
#define GUCEF_COM_CIMAPCOMMANDPARTS_H
#endif /* GUCEF_COM_CIMAPCOMMANDPARTS_H ? */

#ifndef GUCEF_COM_CIMAPFETCHSPEC_H
#include "gucefCOM_CImapFetchSpec.h"
#define GUCEF_COM_CIMAPFETCHSPEC_H
#endif /* GUCEF_COM_CIMAPFETCHSPEC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a SEARCH is for. Without criteria it is "ALL". All criteria have to match */
class GUCEF_COM_PUBLIC_CPP CImapSearchCriteria
{
    public:

    CImapSearchCriteria( void );

    void AddUnseen( void );
    void AddSeen( void );
    void AddFlagged( void );
    void AddDeleted( void );

    /** A range of UIDs (RFC 3501 'UID' criterion); 'last' 0 is "*" */
    void AddUidRange( UInt32 first, UInt32 last );

    /** A header that holds the text, for example ("X-Redux-Game", "g-1234"). The text may be empty: the header only has to exist */
    void AddHeader( const CORE::CString& name, const CORE::CString& text );

    void AddSubject( const CORE::CString& text );
    void AddFrom( const CORE::CString& text );

    /** Delivered on or after the date */
    void AddSince( Int32 year, Int32 month, Int32 day );

    /** The criteria as the parts of a command. @return false with a reason when one of them cannot be sent */
    bool Build( CImapCommandParts& parts, CORE::CString& error ) const;

    bool IsEmpty( void ) const;

    private:

    enum EKind { KIND_UNSEEN, KIND_SEEN, KIND_FLAGGED, KIND_DELETED, KIND_UID, KIND_HEADER, KIND_SUBJECT, KIND_FROM, KIND_SINCE };

    struct SCriterion
    {
        EKind kind;
        CORE::CString name;
        CORE::CString text;
        UInt32 first;
        UInt32 last;
        Int32 year;
        Int32 month;
        Int32 day;
    };

    GUCEF::vector< SCriterion > m_criteria;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CIMAPSEARCHCRITERIA_H ? */
