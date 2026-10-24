/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CSOFTWARECRYPTOCODECS_H
#include "gucefCORE_CSoftwareCryptoCodecs.h"
#define GUCEF_CORE_CSOFTWARECRYPTOCODECS_H
#endif /* GUCEF_CORE_CSOFTWARECRYPTOCODECS_H ? */

#ifndef GUCEF_CORE_COSCRYPTOCODECS_H
#include "gucefCORE_COsCryptoCodecs.h"
#define GUCEF_CORE_COSCRYPTOCODECS_H
#endif /* GUCEF_CORE_COSCRYPTOCODECS_H ? */

#include "gucefCORE_CBuiltInCodecs.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CBuiltInCodecs::ClassTypeName = "GUCEF::CORE::CBuiltInCodecs";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CBuiltInCodecs::CBuiltInCodecs( CCodecRegistry& registry )
    : CGloballyConfigurable()
    , m_registry( &registry )
    , m_softwareCodecs()
    , m_osCodecs()
    , m_registerOsCryptoCodecs( true )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CBuiltInCodecs::~CBuiltInCodecs()
{GUCEF_TRACE;

    UnregisterCodecs();
}

/*-------------------------------------------------------------------------*/

void
CBuiltInCodecs::RegisterCodecs( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( m_softwareCodecs.empty() )
    {
        m_softwareCodecs.push_back( CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CSha256Codec() ) );
        m_softwareCodecs.push_back( CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CHmacSha256Codec() ) );
        m_softwareCodecs.push_back( CCodecRegistry::TICodecTypedPtr( GUCEF_NEW CHkdfSha256Codec() ) );

        TICodecPtrVector::iterator i = m_softwareCodecs.begin();
        while ( i != m_softwareCodecs.end() )
        {
            m_registry->RegisterCodec( (*i), CCodecRegistry::SoftwareCodecPriority );
            ++i;
        }
    }

    if ( m_registerOsCryptoCodecs )
        RegisterOsCryptoCodecs();
}

/*-------------------------------------------------------------------------*/

void
CBuiltInCodecs::UnregisterCodecs( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    UnregisterOsCryptoCodecs();

    TICodecPtrVector::iterator i = m_softwareCodecs.begin();
    while ( i != m_softwareCodecs.end() )
    {
        m_registry->UnregisterCodec( (*i) );
        ++i;
    }
    m_softwareCodecs.clear();
}

/*-------------------------------------------------------------------------*/

void
CBuiltInCodecs::RegisterOsCryptoCodecs( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( !m_osCodecs.empty() )
        return;

    COsCryptoCodecs::CreateCodecs( m_osCodecs );
    TICodecPtrVector::iterator i = m_osCodecs.begin();
    while ( i != m_osCodecs.end() )
    {
        m_registry->RegisterCodec( (*i), CCodecRegistry::OsCodecPriority );
        ++i;
    }

    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "BuiltInCodecs: Registered " << m_osCodecs.size() << " OS crypto codec(s) of \"" << COsCryptoCodecs::GetProviderName() << "\"" );
}

/*-------------------------------------------------------------------------*/

void
CBuiltInCodecs::UnregisterOsCryptoCodecs( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    TICodecPtrVector::iterator i = m_osCodecs.begin();
    while ( i != m_osCodecs.end() )
    {
        m_registry->UnregisterCodec( (*i) );
        ++i;
    }
    m_osCodecs.clear();
}

/*-------------------------------------------------------------------------*/

void
CBuiltInCodecs::SetRegisterOsCryptoCodecs( bool registerOsCryptoCodecs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    m_registerOsCryptoCodecs = registerOsCryptoCodecs;
    if ( registerOsCryptoCodecs )
        RegisterOsCryptoCodecs();
    else
        UnregisterOsCryptoCodecs();
}

/*-------------------------------------------------------------------------*/

bool
CBuiltInCodecs::GetRegisterOsCryptoCodecs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_registerOsCryptoCodecs;
}

/*-------------------------------------------------------------------------*/

void
CBuiltInCodecs::GetOsCryptoCodecs( TICodecPtrVector& codecs ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    codecs.insert( codecs.end(), m_osCodecs.begin(), m_osCodecs.end() );
}

/*-------------------------------------------------------------------------*/

void
CBuiltInCodecs::GetSoftwareCodecs( TICodecPtrVector& codecs ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    codecs.insert( codecs.end(), m_softwareCodecs.begin(), m_softwareCodecs.end() );
}

/*-------------------------------------------------------------------------*/

bool
CBuiltInCodecs::SaveConfig( CDataNode& cfg ) const
{GUCEF_TRACE;

    CDataNode* node = cfg.Structure( "BuiltInCodecs", '/' );
    if ( GUCEF_NULL == node )
        return false;
    node->SetAttribute( "registerOsCryptoCodecs", GetRegisterOsCryptoCodecs() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CBuiltInCodecs::LoadConfig( const CDataNode& cfg )
{GUCEF_TRACE;

    const CDataNode* node = cfg.Find( "BuiltInCodecs" );
    if ( GUCEF_NULL != node )
    {
        bool registerOsCryptoCodecs = node->GetAttributeValueOrChildValueByName( "registerOsCryptoCodecs" ).AsBool( GetRegisterOsCryptoCodecs(), true );
        SetRegisterOsCryptoCodecs( registerOsCryptoCodecs );
    }
    return true; // the settings are optional
}

/*-------------------------------------------------------------------------*/

const CString&
CBuiltInCodecs::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
