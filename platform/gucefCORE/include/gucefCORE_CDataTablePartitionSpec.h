/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CDATATABLEPARTITIONSPEC_H
#define GUCEF_CORE_CDATATABLEPARTITIONSPEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDataTableResolvedQuery;

/**
 *  How the rows of a table are split into partitions, and the relative path of each partition.
 *
 *  A partition field takes its value from a source column through a transform. The path template is a relative path
 *  ('/' separated) whose segments hold literal text and {fieldName} tokens; every field appears exactly once and two
 *  tokens never touch, so a segment can be parsed back.
 *
 *  Key values: a CValueList keyed by field name, one value per key (BuildPath uses the first value of a key). For an
 *  IDENTITY field the value is the column value (text, integer or a date variant); for YEAR, MONTH and DAY fields any
 *  date or timestamp variant within the period. ParseSegment gives IDENTITY values as UTF8 text and YEAR, MONTH and DAY
 *  values as the date variant of the first day of the period, replacing values the keys had.
 *
 *  Values in paths are escaped: every byte outside [A-Z0-9._-] becomes %XX (upper case hex), lower case letters
 *  included, so names cannot collide on case insensitive file systems. A leading or trailing '.' is escaped, and the
 *  first character of a value equal to a Windows device name (CON, PRN, AUX, NUL, COM1..COM9, LPT1..LPT9, ignoring
 *  case), alone or followed by '.', is escaped: "AUX" becomes "%41UX".
 *
 *  Serialized as "fields" (an array of { "name", "source", "transform" }) and "pathTemplate".
 *  Immutable once shared; copies are independent.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTablePartitionSpec : public CIDataNodeSerializable
{
    public:

    struct GUCEF_CORE_PUBLIC_CPP SField
    {
        CString name;                               /**< token name in the template */
        CString sourceColumn;
        TDataTablePartitionTransform transform;     /**< IDENTITY, YEAR, MONTH, DAY */

        SField( void );
        bool operator==( const SField& other ) const;
    };
    typedef GUCEF::vector< SField > TFieldVector;

    CDataTablePartitionSpec( void );

    CDataTablePartitionSpec( const CDataTablePartitionSpec& src );

    virtual ~CDataTablePartitionSpec() GUCEF_VIRTUAL_OVERRIDE;

    CDataTablePartitionSpec& operator=( const CDataTablePartitionSpec& src );

    bool operator==( const CDataTablePartitionSpec& other ) const;
    bool operator!=( const CDataTablePartitionSpec& other ) const;

    /** False for a name that is empty, not made of [A-Za-z0-9_], or already used */
    bool AddField( const CString& name, const CString& sourceColumn, TDataTablePartitionTransform transform );

    /** "{mic}/{symbol}/bars_1d/{year}.parquet"; false for a template that is absolute, has empty, "." or ".." segments, unbalanced braces or touching tokens */
    bool SetPathTemplate( const CString& pathTemplate );

    const CString& GetPathTemplate( void ) const;

    const TFieldVector& GetFields( void ) const;

    /** The field with the name, GUCEF_NULL when absent */
    const SField* FindField( const CString& name ) const;

    /** Segments of the path template */
    UInt32 GetSegmentCount( void ) const;

    bool IsPartitioned( void ) const;

    /** Removes the fields and the template */
    void Clear( void );

    /**
     *  Checks the spec against the schema: a template exactly when there are fields; every field token exactly once
     *  in the template and no unknown tokens; source columns that exist and are not nullable; IDENTITY on text, integer
     *  or date columns, YEAR, MONTH and DAY on date or timestamp columns; a primary key, when set, that contains every
     *  source column; a separator between two fields of a segment that holds a byte no escaped value holds (outside
     *  A-Z, 0-9, '.', '_', '-' and '%'), so "{sym}~{cls}" and "day_{date}_s{n}" are taken and "{sym}-{cls}" is not.
     */
    bool Validate( const CDataTableSchema& schema, CString& error ) const;

    /** The relative path of the partition holding the given key values (escaped). False when a field has no usable value */
    bool BuildPath( const CValueList& keyValues, CString& relativePath ) const;

    /**
     *  One segment of the path (escaped), for a backend that walks the template level by level: the segment's literal
     *  text with the values of its fields. False for a segment index out of range or a field of the segment without
     *  a usable value; keys of fields in other segments are not needed.
     */
    bool BuildSegment( UInt32 segmentIndex, const CValueList& keyValues, CString& segmentName ) const;

    /** The names of the fields whose tokens are in the segment, in template order; none for a literal segment */
    void GetSegmentFields( UInt32 segmentIndex, CString::StringVector& fieldNames ) const;

    /** Parses one path segment level against the template; false for names that do not match (temp files, foreign files) */
    bool ParseSegment( UInt32 segmentIndex, const CString& name, CValueList& keyValues ) const;

    /** False when the partition with these key values cannot hold a row matching the query (5.4) */
    bool MayMatch( const CValueList& keyValues, const CDataTableResolvedQuery& query ) const;

    /** The escaped form of a value's text as it appears in a path segment */
    static CString EscapeSegmentValue( const CString& valueText );

    /** Reverses EscapeSegmentValue; false for malformed escapes and for text that is not in the escaped form */
    static bool UnescapeSegmentValue( const CString& escapedText, CString& valueText );

    virtual bool Serialize( CDataNode& domRootNode                        ,
                            const CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CDataNode& domRootNode                  ,
                              const CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SSegmentPart
    {
        CString literal;        /**< literal text before the token */
        CString token;          /**< field name of the token after the literal, empty for the literal at the end of a segment */
    };
    typedef GUCEF::vector< SSegmentPart > TSegmentPartVector;
    typedef GUCEF::vector< TSegmentPartVector > TSegmentVector;

    static bool FormatFieldValue( const SField& field, const CVariant& value, CString& text );
    static bool ParseFieldValue( const SField& field, const CString& text, CVariant& value );
    static bool ParseTemplate( const CString& pathTemplate, TSegmentVector& segments );

    TFieldVector m_fields;
    CString m_pathTemplate;
    TSegmentVector m_segments;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEPARTITIONSPEC_H ? */
