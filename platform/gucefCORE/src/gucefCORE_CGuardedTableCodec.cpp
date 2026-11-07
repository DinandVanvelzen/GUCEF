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


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_TABLECODECPLUGINAPI_H
#include "gucefCORE_TableCodecPluginAPI.h"
#define GUCEF_CORE_TABLECODECPLUGINAPI_H
#endif /* GUCEF_CORE_TABLECODECPLUGINAPI_H ? */

#include "gucefCORE_CGuardedTableCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GUARDS OF THE OBJECTS A CODEC MAKES                                //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/**
 *  The destructors below all end the same way: the object of the module is released first, in the destructor body, and
 *  the lifetime is told after that. Members are destroyed after the body, so the lifetime pointer is still there.
 */

class CGuardedTableFileMetaData : public CITableFileMetaData
{
    public:

    CGuardedTableFileMetaData( const CITableFileMetaDataPtr& metaData, const TTableCodecModuleLifetimePtr& lifetime )
        : CITableFileMetaData()
        , m_metaData( metaData )
        , m_lifetime( lifetime )
    {GUCEF_TRACE;

        m_lifetime->ObjectCreated();
        m_lifetime->RegisterMetaData( this );
    }

    virtual ~CGuardedTableFileMetaData() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_lifetime->UnregisterMetaData( this );
        m_metaData.Unlink();
        m_lifetime->ObjectDestroyed();
    }

    const CITableFileMetaDataPtr& GetWrapped( void ) const { return m_metaData; }

    virtual const CDataTableSchemaPtr& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_metaData->GetSchema(); }
    virtual UInt64 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_metaData->GetRowCount(); }
    virtual UInt32 GetRowGroupCount( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_metaData->GetRowGroupCount(); }
    virtual bool GetRowGroupInfo( UInt32 rowGroup, CDataTableRowGroupInfo& info ) const GUCEF_VIRTUAL_OVERRIDE { return m_metaData->GetRowGroupInfo( rowGroup, info ); }
    virtual UInt64 GetMemoryFootprint( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_metaData->GetMemoryFootprint(); }
    virtual void GetKeyValueMetaData( CValueList& keyValues ) const GUCEF_VIRTUAL_OVERRIDE { m_metaData->GetKeyValueMetaData( keyValues ); }

    // added in API version 1.1: a module built for 1.0 has no such members in the vtable of its metadata
    virtual bool CanVerifySource( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_lifetime->ModuleHasApiMinorVersion( 1 ) && m_metaData->CanVerifySource(); }
    virtual bool VerifySource( CIOAccess& source ) const GUCEF_VIRTUAL_OVERRIDE { return m_lifetime->ModuleHasApiMinorVersion( 1 ) && m_metaData->VerifySource( source ); }

    private:

    CITableFileMetaDataPtr m_metaData;
    TTableCodecModuleLifetimePtr m_lifetime;
};

/*-------------------------------------------------------------------------*/

/** Metadata of the module, held by a guard so that it keeps the module loaded */
CITableFileMetaDataPtr
GuardMetaData( const CITableFileMetaDataPtr& metaData, const TTableCodecModuleLifetimePtr& lifetime )
{GUCEF_TRACE;

    if ( metaData.IsNULL() )
        return metaData;
    CTSharedPtr< CGuardedTableFileMetaData, MT::CMutex > guard( GUCEF_NEW CGuardedTableFileMetaData( metaData, lifetime ) );
    return CITableFileMetaDataPtr( guard );
}

/** The metadata object of the module behind a guard that the lifetime knows; anything else is returned as it is */
CITableFileMetaDataPtr
UnguardMetaData( const CITableFileMetaDataPtr& metaData, const TTableCodecModuleLifetimePtr& lifetime )
{GUCEF_TRACE;

    if ( metaData.IsNULL() || !lifetime->IsRegisteredMetaData( metaData.GetPointer() ) )
        return metaData;
    return static_cast< const CGuardedTableFileMetaData* >( metaData.GetPointer() )->GetWrapped();
}

/*-------------------------------------------------------------------------*/

class CGuardedTableReader : public CITableReader
{
    public:

    CGuardedTableReader( const CITableReaderPtr& reader, const TTableCodecModuleLifetimePtr& lifetime )
        : CITableReader()
        , m_reader( reader )
        , m_metaData()
        , m_lifetime( lifetime )
    {GUCEF_TRACE;

        m_lifetime->ObjectCreated();
        m_metaData = GuardMetaData( m_reader->GetMetaData(), m_lifetime );
    }

    virtual ~CGuardedTableReader() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_metaData.Unlink();
        m_reader.Unlink();
        m_lifetime->ObjectDestroyed();
    }

    virtual const CITableFileMetaDataPtr& GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_metaData; }
    virtual TDataTableStatus OpenRowGroup( UInt32 rowGroup, const CDataTableReadRequest& request ) GUCEF_VIRTUAL_OVERRIDE { return m_reader->OpenRowGroup( rowGroup, request ); }
    virtual TDataTableStatus ReadNextBatch( CDataTableBatchPtr& batch ) GUCEF_VIRTUAL_OVERRIDE { return m_reader->ReadNextBatch( batch ); }
    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_reader->GetLastErrorMessage(); }

    private:

    CITableReaderPtr m_reader;
    CITableFileMetaDataPtr m_metaData;
    TTableCodecModuleLifetimePtr m_lifetime;
};

/*-------------------------------------------------------------------------*/

class CGuardedTableWriter : public CITableWriter
{
    public:

    CGuardedTableWriter( const CITableWriterPtr& writer, const TTableCodecModuleLifetimePtr& lifetime )
        : CITableWriter()
        , m_writer( writer )
        , m_lifetime( lifetime )
    {GUCEF_TRACE;

        m_lifetime->ObjectCreated();
    }

    virtual ~CGuardedTableWriter() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        m_writer.Unlink();
        m_lifetime->ObjectDestroyed();
    }

    virtual TDataTableStatus WriteBatch( const CDataTableBatch& batch ) GUCEF_VIRTUAL_OVERRIDE { return m_writer->WriteBatch( batch ); }
    virtual TDataTableStatus FlushRowGroup( void ) GUCEF_VIRTUAL_OVERRIDE { return m_writer->FlushRowGroup(); }
    virtual TDataTableStatus Finish( CDataTableFileSummary& summary ) GUCEF_VIRTUAL_OVERRIDE { return m_writer->Finish( summary ); }
    virtual void Abort( void ) GUCEF_VIRTUAL_OVERRIDE { m_writer->Abort(); }
    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_writer->GetLastErrorMessage(); }

    private:

    CITableWriterPtr m_writer;
    TTableCodecModuleLifetimePtr m_lifetime;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CTableCodecModuleLifetime                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CTableCodecModuleLifetime::CTableCodecModuleLifetime( void )
    : m_liveObjects( 0 )
    , m_moduleApiVersion( 0 )
    , m_lock()
    , m_metaDataGuards()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTableCodecModuleLifetime::~CTableCodecModuleLifetime()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTableCodecModuleLifetime::ObjectCreated( void )
{GUCEF_TRACE;

    m_liveObjects.FetchAdd( 1 );
}

/*-------------------------------------------------------------------------*/

void
CTableCodecModuleLifetime::ObjectDestroyed( void )
{GUCEF_TRACE;

    // release: everything the destroyed object did is visible to the thread that sees the count drop
    m_liveObjects.FetchSub( 1, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------*/

UInt32
CTableCodecModuleLifetime::GetLiveObjectCount( void ) const
{GUCEF_TRACE;

    return m_liveObjects.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

void
CTableCodecModuleLifetime::SetModuleApiVersion( UInt32 apiVersion )
{GUCEF_TRACE;

    m_moduleApiVersion.Store( apiVersion );
}

/*-------------------------------------------------------------------------*/

UInt32
CTableCodecModuleLifetime::GetModuleApiVersion( void ) const
{GUCEF_TRACE;

    return m_moduleApiVersion.Load();
}

/*-------------------------------------------------------------------------*/

bool
CTableCodecModuleLifetime::ModuleHasApiMinorVersion( UInt32 minorVersion ) const
{GUCEF_TRACE;

    const UInt32 apiVersion = m_moduleApiVersion.Load();
    return ( apiVersion >> 16 ) == GUCEF_TABLECODECPLUGIN_API_VERSION_MAJOR && ( apiVersion & 0xFFFF ) >= minorVersion;
}

/*-------------------------------------------------------------------------*/

void
CTableCodecModuleLifetime::RegisterMetaData( const CITableFileMetaData* guard )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_metaDataGuards.insert( guard );
}

/*-------------------------------------------------------------------------*/

void
CTableCodecModuleLifetime::UnregisterMetaData( const CITableFileMetaData* guard )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_metaDataGuards.erase( guard );
}

/*-------------------------------------------------------------------------*/

bool
CTableCodecModuleLifetime::IsRegisteredMetaData( const CITableFileMetaData* guard ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_metaDataGuards.end() != m_metaDataGuards.find( guard );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      CGuardedTableCodec                                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

CGuardedTableCodec::CGuardedTableCodec( CITableCodec* codec, const TTableCodecModuleLifetimePtr& lifetime, bool counted )
    : CITableCodec()
    , m_codec( codec )
    , m_lifetime( lifetime )
    , m_counted( counted )
{GUCEF_TRACE;

    if ( m_counted )
        m_lifetime->ObjectCreated();
}

/*-------------------------------------------------------------------------*/

CGuardedTableCodec::~CGuardedTableCodec()
{GUCEF_TRACE;

    // the codec of the module goes first, its destructor is code of the module
    GUCEF_DELETE m_codec;
    m_codec = GUCEF_NULL;
    if ( m_counted )
        m_lifetime->ObjectDestroyed();
}

/*-------------------------------------------------------------------------*/

CString
CGuardedTableCodec::GetTypeName( void ) const
{GUCEF_TRACE;

    return m_codec->GetTypeName();
}

/*-------------------------------------------------------------------------*/

CString
CGuardedTableCodec::GetName( void ) const
{GUCEF_TRACE;

    return m_codec->GetName();
}

/*-------------------------------------------------------------------------*/

CString
CGuardedTableCodec::GetCopyright( void ) const
{GUCEF_TRACE;

    return m_codec->GetCopyright();
}

/*-------------------------------------------------------------------------*/

TVersion
CGuardedTableCodec::GetVersion( void ) const
{GUCEF_TRACE;

    return m_codec->GetVersion();
}

/*-------------------------------------------------------------------------*/

UInt32
CGuardedTableCodec::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_codec->GetCapabilities();
}

/*-------------------------------------------------------------------------*/

void
CGuardedTableCodec::GetFileExtensions( CString::StringSet& extensions ) const
{GUCEF_TRACE;

    m_codec->GetFileExtensions( extensions );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CGuardedTableCodec::ReadMetaData( CIOAccess& source, const CValueList& options, CITableFileMetaDataPtr& metaData, CString& error ) const
{GUCEF_TRACE;

    CITableFileMetaDataPtr made;
    TDataTableStatus status = m_codec->ReadMetaData( source, options, made, error );
    metaData = GuardMetaData( made, m_lifetime );
    return status;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CGuardedTableCodec::CreateReader( const IOAccessPtr& source, const CITableFileMetaDataPtr& metaData, const CValueList& options,
                                  CITableReaderPtr& reader, CString& error ) const
{GUCEF_TRACE;

    CITableReaderPtr made;
    TDataTableStatus status = m_codec->CreateReader( source, UnguardMetaData( metaData, m_lifetime ), options, made, error );
    if ( made.IsNULL() )
    {
        reader.Unlink();
        return status;
    }
    CTSharedPtr< CGuardedTableReader, MT::CMutex > guard( GUCEF_NEW CGuardedTableReader( made, m_lifetime ) );
    reader = CITableReaderPtr( guard );
    return status;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CGuardedTableCodec::CreateWriter( const IOAccessPtr& destination, const CDataTableSchemaPtr& schema, const CDataTableWriterOptions& options,
                                  CITableWriterPtr& writer, CString& error ) const
{GUCEF_TRACE;

    CITableWriterPtr made;
    TDataTableStatus status = m_codec->CreateWriter( destination, schema, options, made, error );
    if ( made.IsNULL() )
    {
        writer.Unlink();
        return status;
    }
    CTSharedPtr< CGuardedTableWriter, MT::CMutex > guard( GUCEF_NEW CGuardedTableWriter( made, m_lifetime ) );
    writer = CITableWriterPtr( guard );
    return status;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CGuardedTableCodec::Clone( void ) const
{GUCEF_TRACE;

    CICloneable* clone = m_codec->Clone();
    if ( GUCEF_NULL == clone )
        return GUCEF_NULL;
    return GUCEF_NEW CGuardedTableCodec( static_cast< CITableCodec* >( clone ), m_lifetime, true );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
