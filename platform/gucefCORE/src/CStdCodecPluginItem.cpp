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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/ 

#include <assert.h>
#include <string.h>

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H
#include "gucefCORE_CTVariantMapAdapter_Cpp2C.h"
#define GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H
#endif /* GUCEF_CORE_CTVARIANTMAPADAPTER_CPP2C_H ? */

#ifndef GUCEF_CORE_CDEFAULTCODECINSTANCE_H
#include "gucefCORE_CDefaultCodecInstance.h"
#define GUCEF_CORE_CDEFAULTCODECINSTANCE_H
#endif /* GUCEF_CORE_CDEFAULTCODECINSTANCE_H ? */

#ifndef GUCEF_CORE_CCODECINSTANCEBASE_H
#include "gucefCORE_CCodecInstanceBase.h"
#define GUCEF_CORE_CCODECINSTANCEBASE_H
#endif /* GUCEF_CORE_CCODECINSTANCEBASE_H ? */

#ifndef GUCEF_CORE_CSTDCODECPLUGIN_H
#include "CStdCodecPlugin.h"
#define GUCEF_CORE_CSTDCODECPLUGIN_H
#endif /* GUCEF_CORE_CSTDCODECPLUGIN_H ? */

#include "CStdCodecPluginItem.h"

#ifndef GUCEF_CORE_GUCEF_ESSENTIALS_H
#include "gucef_essentials.h"
#define GUCEF_CORE_GUCEF_ESSENTIALS_H
#endif /* GUCEF_CORE_GUCEF_ESSENTIALS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef CTVariantMapAdapterCpp2C< CString, CVariant > TParamMapAdapter;
typedef TParamMapAdapter::TLinkedMapType TParamMap;

/*-------------------------------------------------------------------------*/

/** The parameters as the plugin reads them: the values refer to those of the list, nothing is copied */
void
LinkParams( const CValueList& params, TParamMap& map )
{GUCEF_TRACE;

    UInt32 keyCount = params.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
        map[ params.GetKey( i ).AsString() ].LinkTo( params.GetValue( i ) );
}

/*-------------------------------------------------------------------------*/

void
InitErrorInfo( TCodecErrorInfo& errorInfo )
{GUCEF_TRACE;

    errorInfo.structSize = (UInt32) sizeof( errorInfo );
    errorInfo.message[ 0 ] = '\0';
}

/*-------------------------------------------------------------------------*/

/** The most passes a call of an instance makes for more output room before it gives up */
static const UInt32 MaxOutputGrowthPasses = 32;

/*-------------------------------------------------------------------------*/

/**
 *  An instance of a plugin codec. Holds the codec, and so the plugin, as long as it lives.
 *  A call goes straight to the plugin with the output buffer of the caller: no allocation once that buffer has room.
 */
class CStdCodecPluginInstance : public CCodecInstanceBase
{
    public:

    CStdCodecPluginInstance( const CICodecPlugin::CCodecPtr& codec, const TCodecPluginLink& link, void* pluginData, void* instanceData )
        : CCodecInstanceBase()
        , m_codec( codec )
        , m_link( link )
        , m_pluginData( pluginData )
        , m_instanceData( instanceData )
    {GUCEF_TRACE;

        InitErrorInfo( m_errorInfo );
    }

    virtual ~CStdCodecPluginInstance() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL != m_link.destroyInstance )
            m_link.destroyInstance( m_instanceData );
        m_instanceData = GUCEF_NULL;
    }

    virtual bool Process( UInt32 flags, const void* input, UInt32 inputSize, const SCodecCallArgs* args, CDynamicBuffer& output ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        // room for the call: the bound the plugin gives, or a guess that grows when the codec needs more
        UInt32 room = 0;
        if ( GUCEF_NULL == m_link.getOutputSizeBound ||
             GUCEF_CODEC_STATUS_OK != m_link.getOutputSizeBound( m_pluginData, m_link.codecData, m_instanceData, flags, inputSize, &room ) )
        {
            room = ( inputSize < GUCEF_UINT32MAX - 4096 ) ? inputSize + 4096 : GUCEF_UINT32MAX - output.GetDataSize();
        }

        UInt32 callFlags = flags;
        const void* callInput = input;
        UInt32 callInputSize = inputSize;
        for ( UInt32 pass=0; pass<MaxOutputGrowthPasses; ++pass )
        {
            UInt8* out = GUCEF_NULL;
            if ( !ReserveOutput( output, room, out ) )
                return false;

            UInt32 written = 0;
            m_errorInfo.message[ 0 ] = '\0';
            TCodecStatus status = m_link.instanceProcess( m_instanceData, callFlags, callInput, callInputSize, args, out, room, &written, &m_errorInfo );
            if ( written > room )
                return SetStatus( GUCEF_CODEC_STATUS_FAILED, "The plugin wrote more than the room it was given" );
            output.SetDataSize( output.GetDataSize() + written );

            if ( GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL != status )
                return SetStatus( status, GUCEF_CODEC_STATUS_OK == status ? GUCEF_NULL : m_errorInfo.message );

            // more room: a message only codec changed nothing and gets the same call again,
            // a streaming codec took all of the input and keeps the rest of its output for the next call
            if ( room > GUCEF_UINT32MAX / 2 )
                break;
            room *= 2;
            if ( 0 == ( m_link.capabilities & GUCEF_CODEC_CAPABILITY_MESSAGE_ONLY ) )
            {
                callFlags = flags & ~( (UInt32) GUCEF_CODEC_PROCESS_BEGIN );
                callInput = GUCEF_NULL;
                callInputSize = 0;
            }
        }
        return SetStatus( GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL, "The codec kept asking for more output room" );
    }

    virtual UInt32 GetOverhead( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        return m_link.overhead;
    }

    private:

    CICodecPlugin::CCodecPtr m_codec;
    TCodecPluginLink m_link;
    void* m_pluginData;
    void* m_instanceData;
    TCodecErrorInfo m_errorInfo;
};

/*-------------------------------------------------------------------------*/

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CStdCodecPluginItem::CStdCodecPluginItem( CStdCodecPlugin* plugin       ,
                                          void* pluginData              ,
                                          const TCodecPluginLink& link  )
    : CICodec()                  ,
      m_codecLink( link )        ,
      m_pluginData( pluginData ) ,
      m_plugin( plugin )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStdCodecPluginItem::CStdCodecPluginItem( const CStdCodecPluginItem& src )
    : CICodec( src )                   ,
      m_codecLink( src.m_codecLink )   ,
      m_pluginData( src.m_pluginData ) ,
      m_plugin( src.m_plugin )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CStdCodecPluginItem::~CStdCodecPluginItem()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const TCodecPluginLink&
CStdCodecPluginItem::GetCodecPluginLink( void ) const
{GUCEF_TRACE;

    return m_codecLink;
}

/*-------------------------------------------------------------------------*/

void*
CStdCodecPluginItem::GetPluginData( void ) const
{GUCEF_TRACE;

    return m_pluginData;
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPluginItem::Run( bool encode, CIOAccess& source, CIOAccess& dest, const CValueList* params )
{GUCEF_TRACE;

    TCODECPLUGFPTR_Encode function = encode ? m_codecLink.encode : m_codecLink.decode;
    if ( GUCEF_NULL == function )
    {
        SetLastStatus( GUCEF_CODEC_STATUS_UNSUPPORTED, encode ? "The codec does not encode" : "The codec does not decode" );
        return false;
    }

    TParamMap paramMap;
    TParamMapAdapter paramAdapter;
    if ( GUCEF_NULL != params )
        LinkParams( *params, paramMap );
    paramAdapter.SetMapAccessPtr( (const TParamMap*) &paramMap );

    TCodecErrorInfo errorInfo;
    InitErrorInfo( errorInfo );
    TCodecStatus status = function( m_pluginData                     ,
                                    m_codecLink.codecData            ,
                                    paramAdapter.GetCStyleAccess()   ,
                                    source.CStyleAccess()            ,
                                    dest.CStyleAccess()              ,
                                    &errorInfo                       );
    errorInfo.message[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 ] = '\0';
    SetLastStatus( status, errorInfo.message );
    return GUCEF_CODEC_STATUS_OK == status;
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPluginItem::Encode( CIOAccess& source ,
                             CIOAccess& dest   )
{GUCEF_TRACE;

    return Run( true, source, dest, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPluginItem::Decode( CIOAccess& source ,
                             CIOAccess& dest   )
{GUCEF_TRACE;

    return Run( false, source, dest, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPluginItem::Encode( CIOAccess& source        ,
                             CIOAccess& dest          ,
                             const CValueList& params )
{GUCEF_TRACE;

    return Run( true, source, dest, &params );
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPluginItem::Decode( CIOAccess& source        ,
                             CIOAccess& dest          ,
                             const CValueList& params )
{GUCEF_TRACE;

    return Run( false, source, dest, &params );
}

/*-------------------------------------------------------------------------*/

void
CStdCodecPluginItem::GetParameterInfo( CCodecParameterInfoList& info ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_codecLink.getParameterInfo )
        return;

    // an index past the last parameter ends the list, the cap guards against a plugin that never says so
    for ( UInt32 i=0; i<256; ++i )
    {
        TCodecParameterInfo paramInfo;
        memset( &paramInfo, 0, sizeof( paramInfo ) );
        paramInfo.structSize = (UInt32) sizeof( paramInfo );
        if ( GUCEF_CODEC_STATUS_OK != m_codecLink.getParameterInfo( m_pluginData, m_codecLink.codecData, i, &paramInfo ) )
            break;
        if ( GUCEF_NULL == paramInfo.name )
            continue;

        info.push_back( CCodecParameterInfo( paramInfo.name                                                                ,
                                             paramInfo.typeId                                                              ,
                                             0 != paramInfo.required                                                       ,
                                             GUCEF_NULL != paramInfo.defaultValue ? CVariant( paramInfo.defaultValue ) : CVariant() ,
                                             GUCEF_NULL != paramInfo.description ? CString( paramInfo.description ) : CString() ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CStdCodecPluginItem::CreateInstance( const CValueList& params     ,
                                     CICodecInstancePtr& instance )
{GUCEF_TRACE;

    CICodecPlugin::CCodecPtr self;
    if ( GUCEF_NULL != m_plugin )
        self = m_plugin->GetCodecPtr( this );

    const bool pluginHasInstances = 0 != ( m_codecLink.capabilities & GUCEF_CODEC_CAPABILITY_INSTANCES ) &&
                                    GUCEF_NULL != m_codecLink.createInstance &&
                                    GUCEF_NULL != m_codecLink.instanceProcess &&
                                    GUCEF_NULL != m_codecLink.destroyInstance;
    if ( !pluginHasInstances )
    {
        if ( self.IsNULL() )
            return CICodec::CreateInstance( params, instance );

        // the default instance keeps this codec, and so the plugin, alive
        instance = CICodecInstancePtr( GUCEF_NEW CDefaultCodecInstance( self, params ) );
        SetLastStatus( GUCEF_CODEC_STATUS_OK );
        return true;
    }

    TParamMap paramMap;
    TParamMapAdapter paramAdapter;
    LinkParams( params, paramMap );
    paramAdapter.SetMapAccessPtr( (const TParamMap*) &paramMap );

    TCodecErrorInfo errorInfo;
    InitErrorInfo( errorInfo );
    void* instanceData = GUCEF_NULL;
    TCodecStatus status = m_codecLink.createInstance( m_pluginData, m_codecLink.codecData, paramAdapter.GetCStyleAccess(), &instanceData, &errorInfo );
    errorInfo.message[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 ] = '\0';
    SetLastStatus( status, errorInfo.message );
    if ( GUCEF_CODEC_STATUS_OK != status )
        return false;

    instance = CICodecInstancePtr( GUCEF_NEW CStdCodecPluginInstance( self, m_codecLink, m_pluginData, instanceData ) );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CStdCodecPluginItem::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_codecLink.capabilities;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CStdCodecPluginItem::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CStdCodecPluginItem( *this );
}

/*-------------------------------------------------------------------------*/

CString
CStdCodecPluginItem::GetType( void ) const
{GUCEF_TRACE;

    return m_codecLink.codecType;
}

/*-------------------------------------------------------------------------*/

CString
CStdCodecPluginItem::GetFamilyName( void ) const
{GUCEF_TRACE;

    return m_codecLink.codecFamily;
}

/*-------------------------------------------------------------------------*/

const CString&
CStdCodecPluginItem::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::CORE::CStdCodecPluginItem";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
