/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_GUI_CHELPHISTORY_H
#define GUCEF_GUI_CHELPHISTORY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_MACROS_H
#include "gucefGUI_macros.h"
#define GUCEF_GUI_MACROS_H
#endif /* GUCEF_GUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The topics a reader visited, for back and forward as a web browser has them: a visit after going back drops the topics
 *  that were ahead; visiting the topic shown again adds nothing. At most 'maximumEntries' are kept, the oldest go first;
 *  0 keeps every one.
 */
class GUCEF_GUI_PUBLIC_CPP CHelpHistory
{
    public:

    typedef GUCEF::vector< CString > TStringVector;

    enum EDefaults
    {
        DefaultMaximumEntries = 256
    };

    explicit CHelpHistory( UInt32 maximumEntries = DefaultMaximumEntries );

    /** The topic becomes the current one; the topics ahead of the current one are dropped */
    void Visit( const CString& topicId );

    bool CanGoBack( void ) const;

    bool CanGoForward( void ) const;

    /** One topic back; false when there is none */
    bool Back( void );

    /** One topic forward; false when there is none */
    bool Forward( void );

    /** The topic shown, empty when nothing was visited */
    const CString& GetCurrent( void ) const;

    void Clear( void );

    UInt32 GetCount( void ) const;

    /** The place of the current topic in GetEntries(), -1 when there is none */
    Int32 GetIndex( void ) const;

    const TStringVector& GetEntries( void ) const;

    /** 0: no maximum. A smaller maximum drops the oldest entries at once */
    void SetMaximumEntries( UInt32 maximumEntries );

    UInt32 GetMaximumEntries( void ) const;

    private:

    void Trim( void );

    TStringVector m_entries;
    Int32 m_index;
    UInt32 m_maximumEntries;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CHELPHISTORY_H ? */
