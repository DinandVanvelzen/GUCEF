/*
 *  gucefCOMPUTE: GUCEF module providing compute: devices, buffers and kernels over pluggable backends
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMPUTE_TYPEFUNCTIONS_H
#include "gucefCOMPUTE_TypeFunctions.h"
#define GUCEF_COMPUTE_TYPEFUNCTIONS_H
#endif /* GUCEF_COMPUTE_TYPEFUNCTIONS_H ? */

#include "gucefCOMPUTE_CComputeSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMPUTE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CComputeSettings::ConfigSectionName = "ComputeSettings";

/*-------------------------------------------------------------------------*/

namespace {

const char* const g_minCountNames[ PROVIDERTIER_COUNT ] = { "minCountScalar", "minCountSimd", "minCountMulticore", "minCountAccelerator" };

CString
JoinSet( const CString::StringSet& values )
{
    CString joined;
    for ( CString::StringSet::const_iterator i = values.begin(); i != values.end(); ++i )
    {
        if ( !joined.IsNULLOrEmpty() )
            joined += ",";
        joined += (*i);
    }
    return joined;
}

CString
JoinVector( const CString::StringVector& values )
{
    CString joined;
    for ( size_t i=0; i<values.size(); ++i )
    {
        if ( !joined.IsNULLOrEmpty() )
            joined += ",";
        joined += values[ i ];
    }
    return joined;
}

CString::StringVector
SplitList( const CString& text )
{
    CString::StringVector result;
    CString::StringVector parts = text.ParseElements( ',', false );
    for ( size_t i=0; i<parts.size(); ++i )
    {
        CString part = parts[ i ].Trim( true ).Trim( false ).Lowercase();
        if ( !part.IsNULLOrEmpty() )
            result.push_back( part );
    }
    return result;
}

CString::StringSet
SplitSet( const CString& text )
{
    CString::StringVector parts = SplitList( text );
    CString::StringSet result;
    for ( size_t i=0; i<parts.size(); ++i )
        result.insert( parts[ i ] );
    return result;
}

}

/*-------------------------------------------------------------------------*/

CComputeSettings::SKernelProviderOverride::SKernelProviderOverride( void )
    : forcedTier( PROVIDERTIER_NONE )
    , disabledTiers( 0 )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<PROVIDERTIER_COUNT; ++i )
        minCount[ i ] = 0;
}

/*-------------------------------------------------------------------------*/

bool
CComputeSettings::SKernelProviderOverride::IsTierDisabled( TComputeProviderTier tier ) const
{GUCEF_TRACE;

    return static_cast< UInt32 >( tier ) < PROVIDERTIER_COUNT && 0 != ( disabledTiers & ( 1u << static_cast< UInt32 >( tier ) ) );
}

/*-------------------------------------------------------------------------*/

bool
CComputeSettings::SKernelProviderOverride::operator==( const SKernelProviderOverride& other ) const
{GUCEF_TRACE;

    if ( forcedTier != other.forcedTier || disabledTiers != other.disabledTiers )
        return false;
    for ( UInt32 i=0; i<PROVIDERTIER_COUNT; ++i )
        if ( minCount[ i ] != other.minCount[ i ] )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

CComputeSettings::CComputeSettings( void )
    : CORE::CIConfigurable()
    , m_enabledBackends()
    , m_disabledBackends()
    , m_backendOrder()
    , m_preferredDevice()
    , m_maxCpuWorkerCount( 0 )
    , m_kernelOverrideRoot()
    , m_validation( false )
    , m_allowFloat64OnGpu( false )
    , m_kernelOverrides()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeSettings::CComputeSettings( const CComputeSettings& src )
    : CORE::CIConfigurable( src )
    , m_enabledBackends( src.m_enabledBackends )
    , m_disabledBackends( src.m_disabledBackends )
    , m_backendOrder( src.m_backendOrder )
    , m_preferredDevice( src.m_preferredDevice )
    , m_maxCpuWorkerCount( src.m_maxCpuWorkerCount )
    , m_kernelOverrideRoot( src.m_kernelOverrideRoot )
    , m_validation( src.m_validation )
    , m_allowFloat64OnGpu( src.m_allowFloat64OnGpu )
    , m_kernelOverrides( src.m_kernelOverrides )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeSettings::~CComputeSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CComputeSettings&
CComputeSettings::operator=( const CComputeSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        m_enabledBackends = src.m_enabledBackends;
        m_disabledBackends = src.m_disabledBackends;
        m_backendOrder = src.m_backendOrder;
        m_preferredDevice = src.m_preferredDevice;
        m_maxCpuWorkerCount = src.m_maxCpuWorkerCount;
        m_kernelOverrideRoot = src.m_kernelOverrideRoot;
        m_validation = src.m_validation;
        m_allowFloat64OnGpu = src.m_allowFloat64OnGpu;
        m_kernelOverrides = src.m_kernelOverrides;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString typeName = "GUCEF::COMPUTE::CComputeSettings";
    return typeName;
}

/*-------------------------------------------------------------------------*/

const CString::StringSet&
CComputeSettings::GetEnabledBackends( void ) const
{GUCEF_TRACE;

    return m_enabledBackends;
}

/*-------------------------------------------------------------------------*/

void
CComputeSettings::SetEnabledBackends( const CString::StringSet& backendNames )
{GUCEF_TRACE;

    m_enabledBackends = SplitSet( JoinSet( backendNames ) );
}

/*-------------------------------------------------------------------------*/

const CString::StringSet&
CComputeSettings::GetDisabledBackends( void ) const
{GUCEF_TRACE;

    return m_disabledBackends;
}

/*-------------------------------------------------------------------------*/

void
CComputeSettings::SetDisabledBackends( const CString::StringSet& backendNames )
{GUCEF_TRACE;

    m_disabledBackends = SplitSet( JoinSet( backendNames ) );
}

/*-------------------------------------------------------------------------*/

const CString::StringVector&
CComputeSettings::GetBackendOrder( void ) const
{GUCEF_TRACE;

    return m_backendOrder;
}

/*-------------------------------------------------------------------------*/

void
CComputeSettings::SetBackendOrder( const CString::StringVector& backendNames )
{GUCEF_TRACE;

    m_backendOrder = SplitList( JoinVector( backendNames ) );
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeSettings::GetPreferredDevice( void ) const
{GUCEF_TRACE;

    return m_preferredDevice;
}

/*-------------------------------------------------------------------------*/

void
CComputeSettings::SetPreferredDevice( const CString& deviceIdentifier )
{GUCEF_TRACE;

    m_preferredDevice = deviceIdentifier;
}

/*-------------------------------------------------------------------------*/

UInt32
CComputeSettings::GetMaxCpuWorkerCount( void ) const
{GUCEF_TRACE;

    return m_maxCpuWorkerCount;
}

/*-------------------------------------------------------------------------*/

void
CComputeSettings::SetMaxCpuWorkerCount( UInt32 count )
{GUCEF_TRACE;

    m_maxCpuWorkerCount = count;
}

/*-------------------------------------------------------------------------*/

const CString&
CComputeSettings::GetKernelOverrideRoot( void ) const
{GUCEF_TRACE;

    return m_kernelOverrideRoot;
}

/*-------------------------------------------------------------------------*/

void
CComputeSettings::SetKernelOverrideRoot( const CString& vfsRoot )
{GUCEF_TRACE;

    m_kernelOverrideRoot = vfsRoot;
}

/*-------------------------------------------------------------------------*/

bool
CComputeSettings::GetValidation( void ) const
{GUCEF_TRACE;

    return m_validation;
}

/*-------------------------------------------------------------------------*/

void
CComputeSettings::SetValidation( bool validation )
{GUCEF_TRACE;

    m_validation = validation;
}

/*-------------------------------------------------------------------------*/

bool
CComputeSettings::GetAllowFloat64OnGpu( void ) const
{GUCEF_TRACE;

    return m_allowFloat64OnGpu;
}

/*-------------------------------------------------------------------------*/

void
CComputeSettings::SetAllowFloat64OnGpu( bool allow )
{GUCEF_TRACE;

    m_allowFloat64OnGpu = allow;
}

/*-------------------------------------------------------------------------*/

void
CComputeSettings::SetKernelProviderOverride( const CString& kernelId, const SKernelProviderOverride& providerOverride )
{GUCEF_TRACE;

    m_kernelOverrides[ kernelId ] = providerOverride;
}

/*-------------------------------------------------------------------------*/

bool
CComputeSettings::GetKernelProviderOverride( const CString& kernelId, SKernelProviderOverride& providerOverride ) const
{GUCEF_TRACE;

    TKernelProviderOverrideMap::const_iterator i = m_kernelOverrides.find( kernelId );
    if ( i == m_kernelOverrides.end() )
        return false;
    providerOverride = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComputeSettings::RemoveKernelProviderOverride( const CString& kernelId )
{GUCEF_TRACE;

    return 0 != m_kernelOverrides.erase( kernelId );
}

/*-------------------------------------------------------------------------*/

const CComputeSettings::TKernelProviderOverrideMap&
CComputeSettings::GetKernelProviderOverrides( void ) const
{GUCEF_TRACE;

    return m_kernelOverrides;
}

/*-------------------------------------------------------------------------*/

bool
CComputeSettings::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    tree.DelChild( ConfigSectionName );
    CORE::CDataNode* section = tree.AddChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    section->SetAttribute( "enabledBackends", JoinSet( m_enabledBackends ) );
    section->SetAttribute( "disabledBackends", JoinSet( m_disabledBackends ) );
    section->SetAttribute( "backendOrder", JoinVector( m_backendOrder ) );
    section->SetAttribute( "preferredDevice", m_preferredDevice );
    section->SetAttribute( "maxCpuWorkers", m_maxCpuWorkerCount );
    section->SetAttribute( "kernelOverrideRoot", m_kernelOverrideRoot );
    section->SetAttribute( "validation", m_validation );
    section->SetAttribute( "allowFloat64OnGpu", m_allowFloat64OnGpu );

    for ( TKernelProviderOverrideMap::const_iterator i = m_kernelOverrides.begin(); i != m_kernelOverrides.end(); ++i )
    {
        CORE::CDataNode* kernel = section->AddChild( "Kernel" );
        if ( GUCEF_NULL == kernel )
            continue;
        const SKernelProviderOverride& entry = (*i).second;
        kernel->SetAttribute( "id", (*i).first );
        kernel->SetAttribute( "forcedTier", CString( ComputeProviderTierToString( entry.forcedTier ) ) );
        CString disabled;
        for ( UInt32 t=0; t<PROVIDERTIER_COUNT; ++t )
        {
            if ( entry.IsTierDisabled( static_cast< TComputeProviderTier >( t ) ) )
            {
                if ( !disabled.IsNULLOrEmpty() )
                    disabled += ",";
                disabled += ComputeProviderTierToString( static_cast< TComputeProviderTier >( t ) );
            }
        }
        kernel->SetAttribute( "disabledTiers", disabled );
        for ( UInt32 t=0; t<PROVIDERTIER_COUNT; ++t )
            kernel->SetAttribute( CString( g_minCountNames[ t ] ), entry.minCount[ t ] );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CComputeSettings::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &treeroot;
    if ( treeroot.GetName() != ConfigSectionName )
        section = treeroot.FindChild( ConfigSectionName );
    if ( GUCEF_NULL == section )
        return false;

    if ( section->IsAttribute( "enabledBackends" ) )
        m_enabledBackends = SplitSet( section->GetAttributeValueOrChildValueByName( "enabledBackends" ).AsString( CString::Empty, true ) );
    if ( section->IsAttribute( "disabledBackends" ) )
        m_disabledBackends = SplitSet( section->GetAttributeValueOrChildValueByName( "disabledBackends" ).AsString( CString::Empty, true ) );
    if ( section->IsAttribute( "backendOrder" ) )
        m_backendOrder = SplitList( section->GetAttributeValueOrChildValueByName( "backendOrder" ).AsString( CString::Empty, true ) );
    m_preferredDevice = section->GetAttributeValueOrChildValueByName( "preferredDevice" ).AsString( m_preferredDevice, true );
    m_maxCpuWorkerCount = section->GetAttributeValueOrChildValueByName( "maxCpuWorkers" ).AsUInt32( m_maxCpuWorkerCount, true );
    m_kernelOverrideRoot = section->GetAttributeValueOrChildValueByName( "kernelOverrideRoot" ).AsString( m_kernelOverrideRoot, true );
    m_validation = section->GetAttributeValueOrChildValueByName( "validation" ).AsBool( m_validation, true );
    m_allowFloat64OnGpu = section->GetAttributeValueOrChildValueByName( "allowFloat64OnGpu" ).AsBool( m_allowFloat64OnGpu, true );

    CORE::CDataNode::TConstDataNodeSet kernels = section->FindChildrenOfType( "Kernel" );
    for ( CORE::CDataNode::TConstDataNodeSet::const_iterator i = kernels.begin(); i != kernels.end(); ++i )
    {
        const CORE::CDataNode* kernel = (*i);
        const CString id = kernel->GetAttributeValueOrChildValueByName( "id" ).AsString( CString::Empty, true );
        if ( id.IsNULLOrEmpty() )
            continue;
        SKernelProviderOverride entry;
        GetKernelProviderOverride( id, entry );

        const CString forced = kernel->GetAttributeValueOrChildValueByName( "forcedTier" ).AsString( CString::Empty, true );
        if ( !forced.IsNULLOrEmpty() && !StringToComputeProviderTier( forced, entry.forcedTier ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "ComputeSettings: Ignoring the unknown tier \"" << forced << "\" of kernel \"" << id << "\"" );
        }
        if ( kernel->IsAttribute( "disabledTiers" ) )
        {
            entry.disabledTiers = 0;
            CString::StringVector tiers = SplitList( kernel->GetAttributeValueOrChildValueByName( "disabledTiers" ).AsString( CString::Empty, true ) );
            for ( size_t t=0; t<tiers.size(); ++t )
            {
                TComputeProviderTier tier = PROVIDERTIER_NONE;
                if ( StringToComputeProviderTier( tiers[ t ], tier ) && PROVIDERTIER_NONE != tier )
                    entry.disabledTiers |= ( 1u << static_cast< UInt32 >( tier ) );
            }
        }
        for ( UInt32 t=0; t<PROVIDERTIER_COUNT; ++t )
            entry.minCount[ t ] = kernel->GetAttributeValueOrChildValueByName( g_minCountNames[ t ] ).AsUInt32( entry.minCount[ t ], true );
        m_kernelOverrides[ id ] = entry;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace COMPUTE */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
