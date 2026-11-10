/*
 *  authpluginSTEAM: gucefAUTH plugin for logging in with the Steam client
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

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMKEYVALUES_H
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMKEYVALUES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#include "authpluginSTEAM_macros.h"
#define GUCEF_AUTHPLUGIN_STEAM_MACROS_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace STEAM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A tolerant reader of the key values text format the Steam client keeps its files in (loginusers.vdf, registry.vdf):
 *  quoted or bare keys and values, blocks in braces, "//" comments, the escapes \n \t \\ \" in quoted text and a
 *  conditional in brackets after a value, which is skipped.
 *
 *  The text is untrusted: it is read in one pass without recursion, at most MaxBytes of it, MaxDepth blocks deep,
 *  with at most MaxEntries keys and MaxTokenBytes in a key or a value. Text that is cut short, has a closing brace
 *  too many or goes over a limit makes Parse() return false, and what was read before that stays available: a
 *  reader that asks for something that is there gets it.
 *
 *  The entries are kept in one list, an entry knows its parent: the root (index 0) is the file, a block has children,
 *  a value has not. Keys are compared without regard to case, as the client does.
 */
class GUCEF_AUTHPLUGIN_STEAM_EXPORT_CPP CSteamKeyValues
{
    public:

    typedef UInt32 TIndex;
    typedef GUCEF::vector< TIndex > TIndexVector;

    static const TIndex RootIndex = 0;
    static const TIndex NoIndex = 0xFFFFFFFF;

    static const UInt32 MaxBytes = 4 * 1024 * 1024;
    static const UInt32 MaxDepth = 16;
    static const UInt32 MaxEntries = 8192;
    static const UInt32 MaxTokenBytes = 4096;

    CSteamKeyValues( void );

    CSteamKeyValues( const CSteamKeyValues& src );

    ~CSteamKeyValues();

    CSteamKeyValues& operator=( const CSteamKeyValues& src );

    void Clear( void );

    /**
     *  Reads the text, replacing what was there. True when the whole text was well formed and inside the limits; false
     *  otherwise, with what could be read kept.
     */
    bool Parse( const CORE::CString& text );

    /** The number of entries read, the root not counted */
    UInt32 GetEntryCount( void ) const;

    /** The child of the parent with the key (not case sensitive), the first one when there are several; NoIndex when there is none */
    TIndex FindChild( TIndex parent, const CORE::CString& key ) const;

    /** The entries below the parent, in the order of the text */
    void GetChildren( TIndex parent, TIndexVector& children ) const;

    /** The text value of the child with the key; the default when there is no such child or it is a block */
    CORE::CString GetChildValue( TIndex parent, const CORE::CString& key, const CORE::CString& defaultValue = CORE::CString::Empty ) const;

    /** The block at a path of keys below the parent, for example "Registry", "HKCU", "Software"; NoIndex when a step is missing or is a value */
    TIndex FindBlock( TIndex parent, const CORE::CString::StringVector& path ) const;

    const CORE::CString& GetKey( TIndex index ) const;

    const CORE::CString& GetValue( TIndex index ) const;

    bool IsBlock( TIndex index ) const;

    private:

    struct SEntry
    {
        CORE::CString key;
        CORE::CString value;
        bool isBlock;
        TIndex parent;

        SEntry( void ) : key(), value(), isBlock( false ), parent( NoIndex ) {}
    };

    typedef GUCEF::vector< SEntry > TEntryVector;

    TEntryVector m_entries;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STEAM */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMKEYVALUES_H ? */
