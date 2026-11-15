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
#ifndef GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H
#define GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_TYPES_H
#include "gucefGUI_Types.h"
#define GUCEF_GUI_TYPES_H
#endif /* GUCEF_GUI_TYPES_H ? */

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
 *  What a CAssetSearchForm builds its filter panel from: groups of filters, each a set of choices (check boxes, every one
 *  with the count of the results that have it when the model gives the counts) or a range of numbers (a minimum and a
 *  maximum with a unit). The model describes what can be filtered on with ids that the CSearchQuery uses, and titles for
 *  people; the form knows nothing of what the filters mean.
 */
class GUCEF_GUI_PUBLIC_CPP CSearchFilterDescription
{
    public:

    enum EFilterKind
    {
        FILTERKIND_CHOICES = 0  ,   /**< any of the chosen values */
        FILTERKIND_RANGE            /**< a number between a minimum and a maximum, both optional */
    };

    /** One choice of a group */
    struct GUCEF_GUI_PUBLIC_CPP SChoice
    {
        CString value;              /**< what a query holds */
        CString label;              /**< what the person reads; the value when empty */

        SChoice( void );
        SChoice( const CString& choiceValue, const CString& choiceLabel );
    };
    typedef struct SChoice TChoice;
    typedef GUCEF::vector< TChoice > TChoiceVector;

    /** One group of the panel */
    struct GUCEF_GUI_PUBLIC_CPP SGroup
    {
        CString id;                 /**< what a query and the facets of a result page call the group */
        CString title;
        EFilterKind kind;
        TChoiceVector choices;      /**< FILTERKIND_CHOICES */
        CString unit;               /**< FILTERKIND_RANGE: "MB", "triangles", "px"; shown beside the bounds */
        CString tooltip;

        SGroup( void );
    };
    typedef struct SGroup TGroup;
    typedef GUCEF::vector< TGroup > TGroupVector;

    CSearchFilterDescription( void );

    void Clear( void );

    /** Adds a group of choices; returns it to add the choices to */
    TGroup& AddChoiceGroup( const CString& id, const CString& title );

    /** Adds a group with a range of numbers */
    TGroup& AddRangeGroup( const CString& id, const CString& title, const CString& unit );

    /** Adds a choice to the group with the id; false when there is no such group of choices */
    bool AddChoice( const CString& groupId, const CString& value, const CString& label = CString::Empty );

    const TGroupVector& GetGroups( void ) const;

    /** The group with the id, GUCEF_NULL when there is none */
    const TGroup* FindGroup( const CString& id ) const;

    bool operator==( const CSearchFilterDescription& other ) const;

    private:

    TGroupVector m_groups;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CSEARCHFILTERDESCRIPTION_H ? */
