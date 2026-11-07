/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#define GUCEF_MULTIPLAY_CMATCHID_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The identity of a match: a random 128 bit value as the lower case text of a GUID, for example
 *  3f2504e0-4f89-41d3-9a0c-0305e82c3301. It is the same for all participants and all providers, it cannot be
 *  guessed, and it is the only part of a match that is ever put in a file name, a link, a subject line or the
 *  context of a signature. Everything else is taken from the match record.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CMatchId : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt32 TextLength = 36;
    static const UInt32 ShortLength = 8;

    CORE::CString id;

    CMatchId( void );

    explicit CMatchId( const CORE::CString& text );

    CMatchId( const CMatchId& src );

    virtual ~CMatchId();

    CMatchId& operator=( const CMatchId& src );

    bool operator==( const CMatchId& other ) const;

    bool IsValid( void ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const char* CloneableTypeName( void );

    /** A new random match id */
    static CORE::CString Generate( void );

    /** Whether the text is a match id: 36 characters, lower case hex digits, dashes at 8, 13, 18 and 23 */
    static bool IsValid( const CORE::CString& text );

    /** The id in the lower case form, empty when the text is no match id in either case */
    static CORE::CString Normalize( const CORE::CString& text );

    /** The first characters of an id, as used in subject lines and file names that people see */
    static CORE::CString Short( const CORE::CString& matchId );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */
