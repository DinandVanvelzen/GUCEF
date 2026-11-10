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

#ifndef ANCESTRYBACKUP_CGEDCOMFILE_H
#define ANCESTRYBACKUP_CGEDCOMFILE_H

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
#include <vector>

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

/**
 *  One line of a GEDCOM record with the lines below it. CONT and CONC lines are not nodes: they are
 *  already joined into the value of the line they continue (CONT with a line break, CONC without),
 *  and a '@@' in a value is an '@'.
 */
struct SGedcomNode
{
    std::string tag;
    std::string xref;                       /**< the id of the record, only set on the root of a record */
    std::string value;
    std::vector< SGedcomNode > children;

    /** The first child with the tag, or NULL */
    const SGedcomNode* Child( const char* childTag ) const;

    /** The value of the first child with the tag, empty if there is none */
    const std::string& ChildValue( const char* childTag ) const;

    /** Adds the children with the tag to the list */
    void Children( const char* childTag, std::vector< const SGedcomNode* >& list ) const;
};

/** Where a level 0 record is in the text of the file */
struct SGedcomRecordRef
{
    std::string type;                       /**< HEAD, INDI, FAM, SOUR... */
    std::string xref;                       /**< without the @ signs, empty if the record has none */
    size_t begin;
    size_t end;

    SGedcomRecordRef( void ) : type(), xref(), begin( 0 ), end( 0 ) {}
};

/**
 *  A GEDCOM file that is held as text with an index of its records. The tree of lines of a record is only built
 *  when asked for: a file of 160,000 people does not fit in memory as a tree of strings, one record does.
 */
class CGedcomFile
{
    public:

    CGedcomFile( void );

    bool Load( const CORE::CString& path );

    /** Takes the text of a file, for tests */
    void LoadFromText( const std::string& text );

    const std::vector< SGedcomRecordRef >& GetRecords( void ) const;

    /** Builds the tree of lines of a record */
    bool ParseRecord( const SGedcomRecordRef& record, SGedcomNode& root ) const;

    /** Looks a record up by type and id, NULL if there is none */
    const SGedcomRecordRef* FindRecord( const std::string& type, const std::string& xref ) const;

    private:

    void IndexRecords( void );

    std::vector< char > m_text;
    std::vector< SGedcomRecordRef > m_records;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CGEDCOMFILE_H ? */
