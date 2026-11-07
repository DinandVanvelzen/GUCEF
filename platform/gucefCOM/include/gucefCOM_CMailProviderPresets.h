/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMAILPROVIDERPRESETS_H
#define GUCEF_COM_CMAILPROVIDERPRESETS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_COM_CMAILPROVIDERPRESET_H
#include "gucefCOM_CMailProviderPreset.h"
#define GUCEF_COM_CMAILPROVIDERPRESET_H
#endif /* GUCEF_COM_CMAILPROVIDERPRESET_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The table of mail providers. A new table holds the defaults that are compiled in (Gmail and Google Workspace, Outlook / Hotmail / Live /
 *  Microsoft 365, Yahoo, iCloud, GMX, Web.de, Fastmail, a local bridge and the generic "custom" entry); LoadConfig() overrides entries of the
 *  same id and adds new ones, so a configuration changes the table without a new build. The configuration is a node with one child per
 *  entry (CMailProviderPreset::LoadConfig()); the attribute "replaceDefaults" on the node starts from an empty table, and a child with the
 *  attribute "remove" set removes the entry of its id. Entries that are not usable are skipped and counted, the rest is taken.
 *
 *  An address is recognized from its domain only: the domain of a hosted domain (a company on Google Workspace) says nothing about the
 *  provider, so such an address is "unknown" and the person picks the provider by hand.
 */
class GUCEF_COM_PUBLIC_CPP CMailProviderPresets : public CORE::CIConfigurable
{
    public:

    /** What recognizing an address found */
    enum ERecognition
    {
        RECOGNIZE_INVALID_ADDRESS = 0 ,     /**< not a usable address: nothing is recognized, the custom entry is given */
        RECOGNIZE_KNOWN ,                   /**< the domain belongs to a provider of the table */
        RECOGNIZE_UNKNOWN                   /**< a good address of a domain that is in no entry: the custom entry is given */
    };

    static const char* const CustomId;      /**< "custom": the entry for every provider that is not in the table */
    static const UInt32 MaxEntries = 256;
    static const UInt32 MaxDomainsPerEntry = 256;

    CMailProviderPresets( void );

    virtual ~CMailProviderPresets() GUCEF_VIRTUAL_OVERRIDE;

    /** Starts over with the compiled in defaults */
    void LoadDefaults( void );

    void Clear( void );

    /** Adds the entry or replaces the one of the same id. False (with the reason) when the entry is not usable or the table is full */
    bool Add( const CMailProviderPreset& preset, CORE::CString* reason = GUCEF_NULL );

    bool Remove( const CORE::CString& id );

    UInt32 GetCount( void ) const;

    /** The entry at the index (in the order of the table); NULL beyond the end. Pointers stay valid until the table changes */
    const CMailProviderPreset* GetPreset( UInt32 index ) const;

    const CMailProviderPreset* FindById( const CORE::CString& id ) const;

    /** The entry for the address' domain (case does not matter, nor a plus tag; a subdomain counts). 'preset' is never NULL: unknown and invalid addresses give the custom entry */
    ERecognition Recognize( const CORE::CString& address, const CMailProviderPreset*& preset ) const;

    /** The custom entry of the table, or the compiled in one when the table lost it */
    const CMailProviderPreset& GetCustomPreset( void ) const;

    /** How many entries the last LoadConfig() skipped because they were not usable */
    UInt32 GetSkippedCount( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  What the person reads about signing in to this provider, in plain words: the steps for the password (an app password where the provider
     *  wants one), and when the provider has an OAuth sign-in, whether it is available in this build ('oauthAvailable': a client id for the
     *  OAuth provider is configured). When it is not, the text says so and what to do instead. Never mentions the address.
     */
    static CORE::CString DescribeSignIn( const CMailProviderPreset& preset, bool oauthAvailable );

    /** The name of an OAuth provider as a person reads it: "Google", "Microsoft", else the name with a capital */
    static CORE::CString GetOAuthDisplayName( const CORE::CString& oauthProvider );

    private:

    CMailProviderPresets( const CMailProviderPresets& src );
    CMailProviderPresets& operator=( const CMailProviderPresets& src );

    GUCEF::vector< CMailProviderPreset > m_presets;
    CMailProviderPreset m_fallbackCustom;
    UInt32 m_skipped;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMAILPROVIDERPRESETS_H ? */
