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

#include "gucefCORE_CDataDrivenDStoreCodecMeta.h"       

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

const CString CDataDrivenDStoreCodecMeta::ClassTypeName = "GUCEF::CORE::CDataDrivenDStoreCodecMeta";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecMeta::CDataDrivenDStoreCodecMeta( void )
    : CIConfigurable()
    , CTSharedObjCreator< CDataDrivenDStoreCodecMeta, MT::CMutex >( this )
    , m_baseCodecTypeName()
    , m_dataDrivenCodecTypeName()
    , m_resources()
    , m_isShareable( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecMeta::CDataDrivenDStoreCodecMeta( const CDataDrivenDStoreCodecMeta& src )
    : CIConfigurable( src )
    , CTSharedObjCreator< CDataDrivenDStoreCodecMeta, MT::CMutex >( this )
    , m_baseCodecTypeName( src.m_baseCodecTypeName )
    , m_dataDrivenCodecTypeName( src.m_dataDrivenCodecTypeName )
    , m_resources( src.m_resources )
    , m_isShareable( src.m_isShareable )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecMeta::~CDataDrivenDStoreCodecMeta()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataDrivenDStoreCodecMeta& 
CDataDrivenDStoreCodecMeta::operator=( const CDataDrivenDStoreCodecMeta& src )
{GUCEF_TRACE;

    if( this != &src )
    {
        CIConfigurable::operator=( src );
        m_baseCodecTypeName = src.m_baseCodecTypeName;
        m_dataDrivenCodecTypeName = src.m_dataDrivenCodecTypeName;
        m_resources = src.m_resources;
        m_isShareable = src.m_isShareable;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void 
CDataDrivenDStoreCodecMeta::SetBaseCodecTypeName( const CString& codecTypeName )
{GUCEF_TRACE;

    m_baseCodecTypeName = codecTypeName;
}

/*-------------------------------------------------------------------------*/

const CString& 
CDataDrivenDStoreCodecMeta::GetBaseCodecTypeName( void ) const
{GUCEF_TRACE;

    return m_baseCodecTypeName;
}

/*-------------------------------------------------------------------------*/

void 
CDataDrivenDStoreCodecMeta::SetDataDrivenCodecTypeName( const CString& codecTypeName )
{GUCEF_TRACE;

    m_dataDrivenCodecTypeName = codecTypeName;
}

/*-------------------------------------------------------------------------*/

const CString& 
CDataDrivenDStoreCodecMeta::GetDataDrivenCodecTypeName( void ) const
{GUCEF_TRACE;

    return m_dataDrivenCodecTypeName;
}

/*-------------------------------------------------------------------------*/

void 
CDataDrivenDStoreCodecMeta::SetDataMap( const CStringMap& resources )
{GUCEF_TRACE;

    m_resources = resources;
}

/*-------------------------------------------------------------------------*/

const CStringMap& 
CDataDrivenDStoreCodecMeta::GetDataMap( void ) const
{GUCEF_TRACE;

    return m_resources;
}

/*-------------------------------------------------------------------------*/

CStringMap& 
CDataDrivenDStoreCodecMeta::GetDataMap( void )
{GUCEF_TRACE;

    return m_resources;
}

/*-------------------------------------------------------------------------*/

void 
CDataDrivenDStoreCodecMeta::SetParams( const CStringMap& params )
{GUCEF_TRACE;

    m_params = params;
}

/*-------------------------------------------------------------------------*/

const CStringMap& 
CDataDrivenDStoreCodecMeta::GetParams( void ) const
{GUCEF_TRACE;

    return m_params;
}

/*-------------------------------------------------------------------------*/

CStringMap& 
CDataDrivenDStoreCodecMeta::GetParams( void )
{GUCEF_TRACE;

    return m_params;
}

/*-------------------------------------------------------------------------*/

void 
CDataDrivenDStoreCodecMeta::SetIsShareable( bool isSharable )
{GUCEF_TRACE;

    m_isShareable = isSharable;
}

/*-------------------------------------------------------------------------*/

bool 
CDataDrivenDStoreCodecMeta::IsShareable( void ) const
{GUCEF_TRACE;

    return m_isShareable;
}

/*-------------------------------------------------------------------------*/

bool 
CDataDrivenDStoreCodecMeta::SaveConfig( CDataNode& config ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CDataDrivenDStoreCodecMeta::LoadConfig( const CDataNode& config )
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

const CString& 
CDataDrivenDStoreCodecMeta::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CDataDrivenDStoreCodecMeta::LinkCAdapter( TDataDrivenDStoreCodecMeta* cAdapter )
{GUCEF_TRACE;

    if ( GUCEF_NULL == cAdapter )
        return false;

    memset( cAdapter, 0, sizeof( TDataDrivenDStoreCodecMeta ) );
    cAdapter->base_codec_type_name = m_baseCodecTypeName.C_String();
    cAdapter->data_driven_codec_typename = m_dataDrivenCodecTypeName.C_String();
    //cAdapter->resources = m_resources;
    //cAdapter->params = m_params;           // TODO
    cAdapter->is_shareable = m_isShareable ? 1 : 0;

    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
