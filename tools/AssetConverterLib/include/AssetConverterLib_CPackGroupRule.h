/*
 *  AssetConverterLib: an explicit group rule of a pack profile (stage P of the group assignment)
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

#ifndef ASSETCONVERTERLIB_CPACKGROUPRULE_H
#define ASSETCONVERTERLIB_CPACKGROUPRULE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef ASSETCONVERTERLIB_MACROS_H
#include "AssetConverterLib_macros.h"
#define ASSETCONVERTERLIB_MACROS_H
#endif /* ASSETCONVERTERLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  { match: { glob?, folder?, token?, kind? }, group?, subGroup?, priority }. A rule matches a file when every match member
 *  that is given matches: glob against the path in the pack (wildcards, case insensitive), folder as a folder of the path (the
 *  path is below it), token as a token of the file name (case insensitive), kind as the kind id. Rules are evaluated by
 *  priority, the highest first, and the first that matches decides; rules of the same priority are evaluated in the order
 *  they were given. The conventions of a pack are data, not code.
 */
class ASSETCONVERTERLIB_PUBLIC_CPP CPackGroupRule
{
    public:

    typedef GUCEF::vector< CPackGroupRule > TRuleVector;

    CORE::CString glob;
    CORE::CString folder;
    CORE::CString token;
    CORE::CString kind;
    CORE::CString group;
    CORE::CString subGroup;
    Int32 priority;

    CPackGroupRule( void );

    bool operator==( const CPackGroupRule& other ) const;
    bool Serialize( CORE::CDataNode& node ) const;
    bool Deserialize( const CORE::CDataNode& node );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_CPACKGROUPRULE_H ? */

