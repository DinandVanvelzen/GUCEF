/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILADDRESSBOOK_H
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILADDRESSBOOK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#include "matchprovidermail_macros.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What the book knows of one mail address */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailAddressEntry
{
    public:

    CORE::CString address;              /**< lower case */
    CORE::CString publicKey;            /**< hex, lower case; empty when the address is only blocked */
    CORE::CString displayName;
    Int64 firstSeen;
    Int64 lastSeen;
    bool blocked;

    CMailAddressEntry( void );
};

/**
 *  The keys of the people a player has played with, by mail address, and the addresses the player does not want to hear from.
 *  Local to the player: nothing in it is sent. A key is remembered when it was introduced and, when the key for an address changes,
 *  only after the player confirmed it (the caller decides: Remember() replaces without asking). Bounded; the oldest unused entry goes
 *  first, blocked entries stay.
 */
class GUCEF_MULTIPLAYPLUGIN_MAIL_EXPORT_CPP CMailAddressBook : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt32 MaxEntries = 1000;
    static const UInt32 MaxFileBytes = 1024 * 1024;

    enum ECheck
    {
        CHECK_UNKNOWN = 0 ,             /**< the address has no key in the book */
        CHECK_SAME ,
        CHECK_DIFFERENT
    };

    CMailAddressBook( void );
    CMailAddressBook( const CMailAddressBook& src );
    virtual ~CMailAddressBook() GUCEF_VIRTUAL_OVERRIDE;
    CMailAddressBook& operator=( const CMailAddressBook& src );

    /** Whether the key is the one the book holds for the address (case of address and key do not matter) */
    ECheck Check( const CORE::CString& address, const CORE::CString& publicKeyHex ) const;

    bool Find( const CORE::CString& address, CMailAddressEntry& entry ) const;

    /** Remembers the key of an address, replacing another key of the address; false for an address or key that is not valid */
    bool Remember( const CORE::CString& address, const CORE::CString& publicKeyHex, const CORE::CString& displayName, Int64 now );

    void SetBlocked( const CORE::CString& address, bool blocked, Int64 now );

    bool IsBlocked( const CORE::CString& address ) const;

    /** Forgets the key of an address (the block stays) */
    void Forget( const CORE::CString& address );

    UInt32 GetCount( void ) const;

    bool SaveToFile( const CORE::CString& path ) const;

    /** Replaces the content by a file; false (and the book stays as it was) when the file is missing, too large or not a book */
    bool LoadFromFile( const CORE::CString& path );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    static const char* CloneableTypeName( void );

    static CORE::CString Normalize( const CORE::CString& address );

    private:

    CMailAddressEntry* FindEntry( const CORE::CString& address );
    const CMailAddressEntry* FindEntry( const CORE::CString& address ) const;
    void MakeRoom( void );

    GUCEF::vector< CMailAddressEntry > m_entries;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILADDRESSBOOK_H ? */
