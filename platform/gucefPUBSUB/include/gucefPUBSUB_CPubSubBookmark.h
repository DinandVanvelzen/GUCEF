/*
 *  gucefPUBSUB: GUCEF module providing pub-sub communication facilities
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#define GUCEF_PUBSUB_CPUBSUBBOOKMARK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CVARIANT_H
#include "gucefCORE_CVariant.h"
#define GUCEF_CORE_CVARIANT_H
#endif /* GUCEF_CORE_CVARIANT_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_PUBSUB_MACROS_H
#include "gucefPUBSUB_macros.h"
#define GUCEF_PUBSUB_MACROS_H
#endif /* GUCEF_PUBSUB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Class providing the a conceptual representation of a backend "bookmark"
 *  A bookmark is a way of keeping track where you are as you consume messages or for tracing
 *  published messages. The actual implementation will vary per backend but the concept is the same
 *  which is simply that, a way of demarkation of where you are.
 */
class GUCEF_PUBSUB_EXPORT_CPP CPubSubBookmark
{
    public:

    typedef GUCEF::vector< CPubSubBookmark >  TPubSubBookmarkVector;

    enum EBookmarkType : Int32
    {
        BOOKMARK_TYPE_NOT_INITIALIZED = 0 ,         /**< the backend has not initialized the bookmark information to anything */
        BOOKMARK_TYPE_NOT_AVAILABLE       ,         /**< bookmark information is not available at the moment but the concept is supported */
        BOOKMARK_TYPE_NOT_APPLICABLE      ,         /**< bookmark concept is NA, useless within the context with the given backends */
        
        BOOKMARK_TYPE_MSG_ID              ,
        BOOKMARK_TYPE_MSG_INDEX           ,
        BOOKMARK_TYPE_MSG_DATETIME        ,
        BOOKMARK_TYPE_TOPIC_INDEX         ,
        BOOKMARK_TYPE_INDEX_KEY_VALUE         /**< position identified by a named index field and its value; use GetBookmarkKeyField() for the field name and GetBookmarkData() for the value */
    };
    typedef enum EBookmarkType TBookmarkType;

    CPubSubBookmark( void );

    CPubSubBookmark( TBookmarkType bmType );

    CPubSubBookmark( TBookmarkType bmType         , 
                     const CORE::CVariant& bmData );

    CPubSubBookmark( TBookmarkType bmType         , 
                     const CORE::CVariant& bmData ,
                     const CORE::CDateTime& bmDt  );

    CPubSubBookmark( const CPubSubBookmark& src   , 
                     bool linkIfPossible          ,
                     const CORE::CDateTime& bmDt  );

    virtual ~CPubSubBookmark();

    CPubSubBookmark& operator=( const CPubSubBookmark& src );

    void SetBookmarkType( TBookmarkType bmType );
    
    TBookmarkType GetBookmarkType( void ) const;

    CORE::CString GetBookmarkTypeName( void ) const;

    void SetBookmarkData( const CORE::CVariant& bmData );

    const CORE::CVariant& GetBookmarkData( void ) const;

    CORE::CVariant& GetBookmarkData( void );

    /**
     *  For BOOKMARK_TYPE_INDEX_KEY_VALUE: sets the named index field (e.g. "mk:fix_seq_num").
     *  Has no meaning for other bookmark types.
     */
    void SetBookmarkKeyField( const CORE::CString& keyField );

    /**
     *  For BOOKMARK_TYPE_INDEX_KEY_VALUE: returns the named index field.
     *  Returns an empty string for other bookmark types.
     */
    const CORE::CString& GetBookmarkKeyField( void ) const;

    /**
     *  Factory: constructs a BOOKMARK_TYPE_INDEX_KEY_VALUE bookmark from a field name and value.
     *  The value is stored as-is in bmData; use any CVariant type (UInt64, CString, etc.).
     *  A NOT_INITIALIZED bookmark with an empty key field represents "no bound" (from beginning / to end).
     */
    static CPubSubBookmark MakeIndexKeyValueBookmark( const CORE::CString& keyField  ,
                                                      const CORE::CVariant& value    );

    /**
     *  Gets the bookmark's timestamp
     *  Note that a bookmark itself can also be a timestamp but this is not the same thing
     *  This is the time at which the bookmark was originated which can differ from the bookmark data itself
     *  for example when one is dealing with historical data
     */
    const CORE::CDateTime& GetBookmarkDateTime( void ) const;

    /**
     *  Gets the bookmark's timestamp
     *  Note that a bookmark itself can also be a timestamp but this is not the same thing
     *  This is the time at which the bookmark was originated which can differ from the bookmark data itself
     *  for example when one is dealing with historical data
     */
    CORE::CDateTime& GetBookmarkDateTime( void );

    CORE::CString ToString( void ) const;

    /**
     *  Same as the assignment operator except that the variant data is linked not copied
     *  This provides a small performance boost when said variant is reliant on dynamic memory
     *  Standard rules wrt LinkTo() behavior applies
     */
    CPubSubBookmark& LinkTo( const CPubSubBookmark& srcBookmark );

    private:

    TBookmarkType   m_bmType;
    CORE::CVariant  m_bmData;
    CORE::CDateTime m_bmDt;
    CORE::CString   m_bmKeyField;   /**< index field name; only meaningful for BOOKMARK_TYPE_INDEX_KEY_VALUE */
};

/*-------------------------------------------------------------------------*/

inline CORE::CString ToString( const CPubSubBookmark& bm ) { return bm.ToString(); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CPUBSUBBOOKMARK_H ? */

