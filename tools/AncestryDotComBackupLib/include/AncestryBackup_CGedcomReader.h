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

#ifndef ANCESTRYBACKUP_CGEDCOMREADER_H
#define ANCESTRYBACKUP_CGEDCOMREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include <cstddef>

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

/** One line of a GEDCOM file, which points into the text that is being parsed and is only valid during the callback */
struct SGedcomLine
{
    UInt32 level;
    const char* xref;               /**< the id between the @ signs, without them. Not terminated, see xrefLength */
    UInt32 xrefLength;              /**< 0 when the line has no id */
    const char* tag;
    UInt32 tagLength;
    const char* value;
    UInt32 valueLength;             /**< 0 when the line has no value */
    UInt32 lineNumber;

    SGedcomLine( void ) : level( 0 ), xref( GUCEF_NULL ), xrefLength( 0 ), tag( GUCEF_NULL ), tagLength( 0 ), value( GUCEF_NULL ), valueLength( 0 ), lineNumber( 0 ) {}
};

/** Receives the lines of a GEDCOM file in the order they are in the file */
class CIGedcomHandler
{
    public:

    virtual ~CIGedcomHandler() {}

    virtual void OnLine( const SGedcomLine& line ) = 0;

    /** A line which is not GEDCOM: no level, or no tag. Parsing goes on with the next line. */
    virtual void OnMalformedLine( UInt32 lineNumber ) = 0;
};

/**
 *  Reads GEDCOM 5.5.1 (and 7) text line by line without building the file in memory: a tree of 160,000 people
 *  is a file of well over 100 MB. Understands any of CR, LF and CRLF as the line end and skips a byte order mark.
 *  The text has to be UTF-8 or ASCII, the old ANSEL character set is not decoded.
 *
 *  A line looks like:  level [@xref@] tag [value]
 *  Joining of CONT and CONC lines is left to the handler, which can then choose not to bother.
 */
class CGedcomReader
{
    public:

    /**
     *  @param data     the text of the file
     *  @param size     the size of the text in bytes
     *  @return the number of lines that were passed to the handler as GEDCOM lines
     */
    static UInt32 Parse( const char* data, size_t size, CIGedcomHandler& handler );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CGEDCOMREADER_H ? */
