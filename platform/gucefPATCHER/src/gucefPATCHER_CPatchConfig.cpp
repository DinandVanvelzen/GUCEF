 /*
 *  gucefPATCHER: GUCEF RAD module providing a patch delivery system
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefPATCHER_CPatchConfig.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PATCHER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CPatchConfig::CPatchConfig( void )
    : CIConfigurable()                    ,
      m_patchListURL()                    ,
      m_patchListCodec()                  ,
      m_localRoot()                       ,
      m_tempStorageRoot()                 ,
      m_startTriggers()                   ,
      m_stopTriggers()                    ,
      m_stopOnReplacementFailure( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPatchConfig::CPatchConfig( const CPatchConfig& src )
    : CIConfigurable( src )                      ,
      m_patchListURL( src.m_patchListURL )       ,
      m_patchListCodec( src.m_patchListCodec )   ,
      m_localRoot( src.m_localRoot )             ,
      m_tempStorageRoot( src.m_tempStorageRoot ) ,
      m_startTriggers( src.m_startTriggers )     ,
      m_stopTriggers( src.m_stopTriggers )       ,
      m_stopOnReplacementFailure( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPatchConfig::~CPatchConfig()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPatchConfig&
CPatchConfig::operator=( const CPatchConfig& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_patchListURL = src.m_patchListURL;
        m_patchListCodec = src.m_patchListCodec;
        m_localRoot = src.m_localRoot;
        m_tempStorageRoot = src.m_tempStorageRoot;
        m_startTriggers = src.m_startTriggers;
        m_stopTriggers = src.m_stopTriggers;
        m_stopOnReplacementFailure = src.m_stopOnReplacementFailure;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CPatchConfig::SetStopOnFileReplacementFailure( const bool stopOnFailure )
{GUCEF_TRACE;

    m_stopOnReplacementFailure = stopOnFailure;
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::GetStopOnFileReplacementFailure( void ) const
{GUCEF_TRACE;

    return m_stopOnReplacementFailure;
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::HasEngineStartTriggerEvent( const CORE::CEvent& triggerEvent )
{GUCEF_TRACE;

    return m_startTriggers.end() != m_startTriggers.find( triggerEvent );
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::HasEngineStopTriggerEvent( const CORE::CEvent& triggerEvent )
{GUCEF_TRACE;

    return m_stopTriggers.end() != m_stopTriggers.find( triggerEvent );
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    //@TODO
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "CPatchConfig: Loading configuration" );

    const CORE::CDataNode* infoNode = treeroot.Find( "CPatchConfig" );
    if ( infoNode != GUCEF_NULL )
    {
        // First we obtain the mandatory atributes
        const CORE::CDataNode::TKeyValuePair* att = infoNode->GetAttribute( "LocalRootDir" );
        if ( att == GUCEF_NULL ) return false;
        SetLocalRootDir( att->second );

        att = infoNode->GetAttribute( "TempStorageRoot" );
        if ( att == GUCEF_NULL ) return false;
        SetLocalTempStorageRootDir( att->second );

        att = infoNode->GetAttribute( "PatchListURL" );
        if ( att == GUCEF_NULL ) return false;
        SetPatchListURL( att->second );

        att = infoNode->GetAttribute( "PatchListCodec" );
        if ( att == GUCEF_NULL ) return false;
        SetPatchListCodec( att->second );

        // Load optional attributes
        att = infoNode->GetAttribute( "StopOnFileReplacementFailure" );
        if ( att == GUCEF_NULL )
        {
            m_stopOnReplacementFailure = true;
        }
        else
        {
            SetStopOnFileReplacementFailure( att->second.AsBool() );
        }

        // Try and find some optional engine trigger events
        const CORE::CDataNode* childNode = GUCEF_NULL;
        CORE::CDataNode::const_iterator i = infoNode->ConstBegin();
        while ( i != infoNode->ConstEnd() )
        {
            childNode = (*i);
            if ( childNode->GetName() == "EngineStartTrigger" )
            {
                att = infoNode->GetAttribute( "Event" );
                if ( att != GUCEF_NULL )
                {
                    AddEngineStartTriggerEvent( att->second.AsString() );
                }
            }
            else
            if ( childNode->GetName() == "EngineStopTrigger" )
            {
                att = infoNode->GetAttribute( "Event" );
                if ( att != GUCEF_NULL )
                {
                    AddEngineStopTriggerEvent( att->second.AsString() );
                }
            }
            ++i;
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CPatchConfig::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::PATCHER::CPatchConfig";
    return classTypeName; 
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::AddEngineStartTriggerEvent( const CORE::CEvent& triggerEvent )
{GUCEF_TRACE;

    m_startTriggers.insert( triggerEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::RemoveEngineStartTriggerEvent( const CORE::CEvent& triggerEvent )
{GUCEF_TRACE;

    m_startTriggers.erase( triggerEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::AddEngineStopTriggerEvent( const CORE::CEvent& triggerEvent )
{GUCEF_TRACE;

    m_stopTriggers.insert( triggerEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::RemoveEngineStopTriggerEvent( const CORE::CEvent& triggerEvent )
{GUCEF_TRACE;

    m_stopTriggers.erase( triggerEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::SetLocalRootDir( const CORE::CString& localRoot )
{GUCEF_TRACE;

    m_localRoot = localRoot;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPatchConfig::GetLocalRootDir( void ) const
{GUCEF_TRACE;

    return m_localRoot;
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::SetLocalTempStorageRootDir( const CORE::CString& tempStorageRoot )
{GUCEF_TRACE;

    m_tempStorageRoot = tempStorageRoot;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPatchConfig::GetLocalTempStorageRootDir( void ) const
{GUCEF_TRACE;

    return m_tempStorageRoot;
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::SetPatchListURL( const CORE::CString& patchListURL )
{GUCEF_TRACE;

    m_patchListURL = patchListURL;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPatchConfig::GetPatchListURL( void ) const
{GUCEF_TRACE;

    return m_patchListURL;
}

/*-------------------------------------------------------------------------*/

bool
CPatchConfig::SetPatchListCodec( const CORE::CString& patchListCodec )
{GUCEF_TRACE;

    m_patchListCodec = patchListCodec;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPatchConfig::GetPatchListCodec( void ) const
{GUCEF_TRACE;

    return m_patchListCodec;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PATCHER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
