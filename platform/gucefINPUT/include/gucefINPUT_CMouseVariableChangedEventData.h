/*
 *  gucefINPUT: GUCEF module providing input device interaction
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

#ifndef GUCEF_INPUT_CMOUSEVARIABLECHANGEDEVENTDATA_H
#define GUCEF_INPUT_CMOUSEVARIABLECHANGEDEVENTDATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_INPUT_MACROS_H
#include "gucefINPUT_macros.h"
#define GUCEF_INPUT_MACROS_H
#endif /* GUCEF_INPUT_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A variable of a mouse changed: CMouse::MOUSEVARIABLE_WHEEL (notches, up is positive) or the horizontal wheel */
class GUCEF_INPUT_PUBLIC_CPP CMouseVariableChangedEventData : public CORE::CICloneable
{
    public:

    CMouseVariableChangedEventData( const Int32 contextId,
        const UInt32 deviceID,
        const UInt32 variableIndex,
        const Int32 value,
        const Int32 valueDelta );

    virtual ~CMouseVariableChangedEventData();

    Int32 GetContextId( void ) const;

    UInt32 GetDeviceID( void ) const;

    UInt32 GetVariableIndex( void ) const;

    Int32 GetValue( void ) const;

    Int32 GetValueDelta( void ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    Int32 m_contextId;
    UInt32 m_deviceID;
    UInt32 m_variableIndex;
    Int32 m_value;
    Int32 m_valueDelta;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_INPUT_CMOUSEVARIABLECHANGEDEVENTDATA_H ? */
