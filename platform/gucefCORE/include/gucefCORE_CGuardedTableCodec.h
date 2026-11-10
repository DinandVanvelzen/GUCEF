/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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


#ifndef GUCEF_CORE_CGUARDEDTABLECODEC_H
#define GUCEF_CORE_CGUARDEDTABLECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CITABLECODEC_H
#include "gucefCORE_CITableCodec.h"
#define GUCEF_CORE_CITABLECODEC_H
#endif /* GUCEF_CORE_CITABLECODEC_H ? */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

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

/**
 *  Counts the objects made by the code of one table codec plugin module that are alive: metadata, readers, writers and
 *  codec clones. The module may only be unloaded when the count is 0.
 *
 *  The count lives here, in gucefCORE, and not in the module: it is changed by the guard objects below, which are code
 *  of gucefCORE. A guard releases the object of the module it holds first and decrements the count as the last thing it
 *  does with the module, so when another thread sees the count at 0 no thread is running code of the module on behalf of
 *  a counted object any more: the destructor of the object, including the operator delete behind it, has returned.
 */
class GUCEF_CORE_PUBLIC_CPP CTableCodecModuleLifetime
{
    public:

    CTableCodecModuleLifetime( void );

    ~CTableCodecModuleLifetime();

    /** An object made by the code of the module came alive */
    void ObjectCreated( void );

    /** The object is destroyed, the code of the module has finished with it */
    void ObjectDestroyed( void );

    UInt32 GetLiveObjectCount( void ) const;

    /** The guards of metadata that are alive, so that the metadata given back to a codec can be told from its own */
    void RegisterMetaData( const CITableFileMetaData* guard );

    void UnregisterMetaData( const CITableFileMetaData* guard );

    bool IsRegisteredMetaData( const CITableFileMetaData* guard ) const;

    /**
     *  The table codec plugin API version the module was built for (GUCEF_TABLECODECPLUGIN_API_VERSION); 0, the default,
     *  when it is not known. Members of the interfaces added in a later minor version are only called on objects of a
     *  module built for that version or later.
     */
    void SetModuleApiVersion( UInt32 apiVersion );

    UInt32 GetModuleApiVersion( void ) const;

    /** Whether the module was built for at least the given minor version of the host's major API version */
    bool ModuleHasApiMinorVersion( UInt32 minorVersion ) const;

    private:

    CTableCodecModuleLifetime( const CTableCodecModuleLifetime& src );              /**< not implemented */
    CTableCodecModuleLifetime& operator=( const CTableCodecModuleLifetime& src );   /**< not implemented */

    typedef GUCEF::set< const void* > TPointerSet;

    MT::CTAtomic< UInt32 > m_liveObjects;
    MT::CTAtomic< UInt32 > m_moduleApiVersion;
    mutable MT::CMutex m_lock;
    TPointerSet m_metaDataGuards;
};

typedef CTSharedPtr< CTableCodecModuleLifetime, MT::CMutex >    TTableCodecModuleLifetimePtr;

/*-------------------------------------------------------------------------*/

/**
 *  Stands in for a codec made by the code of a plugin module. It forwards every call to that codec and wraps what the
 *  codec makes (file metadata, readers, writers and clones) in guards of the same kind that count themselves in the
 *  lifetime of the module, so that the module stays loaded while any of them is alive. See the table codec plugin (10.4
 *  of the design).
 *
 *  The metadata a caller gives to CreateReader() may be a guard from ReadMetaData(); the codec of the module is given the
 *  metadata object it made itself, as it expects.
 */
class GUCEF_CORE_PUBLIC_CPP CGuardedTableCodec : public CITableCodec
{
    public:

    /**
     *  Takes ownership of codec. counted is true for a clone: it is an object of the module that keeps it loaded, where
     *  the codec the plugin itself holds is accounted for by the references to that plugin object.
     */
    CGuardedTableCodec( CITableCodec* codec, const TTableCodecModuleLifetimePtr& lifetime, bool counted );

    virtual ~CGuardedTableCodec() GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetFileExtensions( CString::StringSet& extensions ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus ReadMetaData( CIOAccess& source, const CValueList& options,
                                           CITableFileMetaDataPtr& metaData, CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus CreateReader( const IOAccessPtr& source, const CITableFileMetaDataPtr& metaData,
                                           const CValueList& options, CITableReaderPtr& reader, CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual TDataTableStatus CreateWriter( const IOAccessPtr& destination, const CDataTableSchemaPtr& schema,
                                           const CDataTableWriterOptions& options, CITableWriterPtr& writer, CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CGuardedTableCodec( const CGuardedTableCodec& src );            /**< not implemented */

    CITableCodec* m_codec;
    TTableCodecModuleLifetimePtr m_lifetime;
    bool m_counted;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CGUARDEDTABLECODEC_H ? */
