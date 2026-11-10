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

#ifndef ANCESTRYBACKUP_CGRAMPSDATE_H
#define ANCESTRYBACKUP_CGRAMPSDATE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include <string>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What kind of Gramps date a GEDCOM date became */
enum EGrampsDateKind
{
    GRAMPSDATE_NONE = 0 ,       /**< there was no date */
    GRAMPSDATE_VALUE    ,       /**< a day, month or year, possibly with about, before or after: Gramps can sort and compare it */
    GRAMPSDATE_RANGE    ,       /**< between two dates */
    GRAMPSDATE_SPAN     ,       /**< from one date to another */
    GRAMPSDATE_TEXT             /**< not understood: kept as the text it was, Gramps shows it but can not compute with it */
};

/**
 *  Converts the dates of a GEDCOM file, as Ancestry writes them, into the date elements of Gramps XML.
 *  Understands 20 Sep 1980, Sep 1980, 1980, 20 SEPTEMBER 1980, 44 BC, Abt./ABT/About, Bef./Before, Aft./After,
 *  Est., Cal., Bet. x and y, Bet. 1921-1940, FROM x TO y, and the numeric 1980-09-20 and 20-9-1980 (but only
 *  if the order of day and month can not be mixed up). Anything else is kept as text.
 */
class CGrampsDate
{
    public:

    /**
     *  @param gedcomDate   the value of a DATE line
     *  @param xml          the date element, for example <dateval val="1980-09-20"/>. Empty for an empty date.
     *  @return what kind of date it became
     */
    static EGrampsDateKind ToXml( const std::string& gedcomDate, std::string& xml );

    /** A plain date: day month year, month year or just a year. The year can be negative for BC. */
    static bool ParseSimple( const std::string& text, Int32& year, UInt32& month, UInt32& day );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CGRAMPSDATE_H ? */
