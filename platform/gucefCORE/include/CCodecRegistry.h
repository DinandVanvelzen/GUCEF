/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#define GUCEF_CORE_CCODECREGISTRY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTONREGISTRY_H
#include "CTONRegistry.h"
#define GUCEF_CORE_CTONREGISTRY_H
#endif /* GUCEF_CORE_CTONREGISTRY_H ? */

#ifndef GUCEF_CORE_CICODEC_H
#include "CICodec.h"
#define GUCEF_CORE_CICODEC_H
#endif /* GUCEF_CORE_CICODEC_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

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

class CCoreGlobal;

/*-------------------------------------------------------------------------*/

/**
 *  The codecs of the process: a registry per family with a codec per type.
 *
 *  Several providers can offer the same family and type (the OS, a plugin, built in software). Each registration
 *  through RegisterCodec() has a priority; the family registry holds the provider with the highest priority, so code
 *  that looks codecs up in the family registries gets that one. GetCodecs() lists all of them.
 */
class GUCEF_CORE_PUBLIC_CPP CCodecRegistry : public CTONRegistry< CTONRegistry< CICodec, MT::CMutex >, MT::CMutex >
{
    public:

    typedef CTONRegistry< CICodec, MT::CMutex >                  TCodecFamilyRegistry;
    typedef CTBasicSharedPtr< TCodecFamilyRegistry, MT::CMutex > TCodecFamilyRegistryPtr;
    typedef CTSharedPtr< TCodecFamilyRegistry, MT::CMutex >      TCodecFamilyRegistryTypedPtr;
    typedef TCodecFamilyRegistry::TRegisteredObjPtr              TICodecPtr;
    typedef TCodecFamilyRegistry::TRegisteredObjTypedPtr         TICodecTypedPtr;
    typedef TCodecFamilyRegistry::TRegisteredObjPtrVector        TICodecPtrVector;

    static const Int32 SoftwareCodecPriority;   /**< built in portable code: 25, the fallback that is always there */
    static const Int32 PluginCodecPriority;     /**< plugins: 50 unless the plugin or its configuration says otherwise */
    static const Int32 OsCodecPriority;         /**< the operating system: 100 */

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The provider with the highest priority of the family and type (case insensitive), NULL when there is none */
    TICodecPtr TryGetCodec( const CString& codecFamily ,
                            const CString& codecName   ) const;

    /**
     *  Adds a provider of the codec's family and type (GetFamilyName(), GetType()).
     *  Equal priorities keep the order of registration. The family registry is made when needed.
     *  Returns false when this codec object is already registered under that family and type
     */
    bool RegisterCodec( const TICodecPtr& codec                     ,
                        Int32 priority = PluginCodecPriority        );

    /** Adds a provider under the given family and type, for a codec that is offered under more than one name */
    bool RegisterCodec( const CString& codecFamily     ,
                        const CString& codecType       ,
                        const TICodecPtr& codec        ,
                        Int32 priority                 );

    /**
     *  Takes out every registration of this codec object. Other providers of the same types stay,
     *  the one with the highest priority of them takes its place in the family registry
     */
    bool UnregisterCodec( const TICodecPtr& codec );

    /** All providers of the family and type, highest priority first. False when there is none */
    bool GetCodecs( const CString& codecFamily ,
                    const CString& codecType   ,
                    TICodecPtrVector& codecs   ) const;

    /** The priority the codec object is registered with under the family and type, false when it is not */
    bool TryGetCodecPriority( const CString& codecFamily ,
                              const CString& codecType   ,
                              const CICodec* codec       ,
                              Int32& priority            ) const;

    /** How many references to the codec object the registry holds, so an owner can tell whether others use it */
    UInt32 GetHeldReferenceCount( const CICodec* codec ) const;

    /** Takes out all families and all registrations */
    virtual void UnregisterAll( void ) GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CCoreGlobal;

    CCodecRegistry( void );

    virtual ~CCodecRegistry();

    private:

    CCodecRegistry( const CCodecRegistry& src );              /**< not implemented, don't use */
    CCodecRegistry& operator=( const CCodecRegistry& src );   /**< not implemented, don't use */

    struct SCodecRegistration
    {
        TICodecPtr codec;
        Int32 priority;
    };
    typedef GUCEF::vector< SCodecRegistration > TCodecRegistrationVector;

    /** The registrations of one family and type, highest priority first */
    struct SCodecTypeRegistrations
    {
        CString codecFamily;
        CString codecType;
        TCodecRegistrationVector registrations;
    };
    typedef GUCEF::map< CString, SCodecTypeRegistrations > TCodecTypeRegistrationsMap;

    static CString GetRegistrationKey( const CString& codecFamily, const CString& codecType );

    TCodecFamilyRegistryPtr GetOrCreateFamilyRegistry( const CString& codecFamily );

    void UpdateFamilyRegistry( const SCodecTypeRegistrations& typeRegistrations );

    private:

    MT::CMutex m_dataLock;
    TCodecTypeRegistrationsMap m_registrations;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */
