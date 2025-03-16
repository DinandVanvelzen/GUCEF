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

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#include "gucefCORE_CDataNodeBinarySerializer.h"
#define GUCEF_CORE_CDATANODEBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CDATANODEBINARYSERIALIZER_H ? */

#include "gucefCORE_CDStoreBinaryCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataStoreBinaryCodec::CDataStoreBinaryCodec( void )
    : CDStoreCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataStoreBinaryCodec::~CDataStoreBinaryCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDataStoreBinaryCodec::IsValid( void ) const
{GUCEF_TRACE;
    
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataStoreBinaryCodec::StoreDataTree( const CDataNode* tree   ,
                                      const CString& filename )
{GUCEF_TRACE;

    UInt32 bytesWritten = 0;
    CDynamicBuffer buffer;
    CDataNodeBinarySerializerOptions options;
    if ( CDataNodeBinaryContainerSerializer::Serialize( options, *tree, 0, buffer, bytesWritten ) && bytesWritten > 0 )
    {
        return buffer.WriteContentToFile( filename, 0, true );
    } 
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataStoreBinaryCodec::StoreDataTree( const CDataNode* tree   ,
                                      CIOAccess* file         )
{GUCEF_TRACE;

    UInt32 bytesWritten = 0;
    CDynamicBuffer buffer;
    CDataNodeBinarySerializerOptions options;
    if ( CDataNodeBinaryContainerSerializer::Serialize( options, *tree, 0, buffer, bytesWritten ) && bytesWritten > 0 )
    {
        if ( file->Write( buffer ) == bytesWritten )
        {
            return true;
        }
    } 
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataStoreBinaryCodec::BuildDataTree( CDataNode* treeroot     ,
                                      const CString& filename )
{GUCEF_TRACE;

    if ( filename.IsNULLOrEmpty() || GUCEF_NULL == treeroot )
        return false;
        
    CDynamicBuffer buffer;
    if ( buffer.LoadContentFromFile( filename ) )
    {
        UInt32 bytesRead = 0;
        CDataNodeBinarySerializerOptions options;
        if ( CDataNodeBinaryContainerSerializer::Deserialize( options, false, *treeroot, 0, buffer, bytesRead ) )
        {
            return true;
        }   
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
CDataStoreBinaryCodec::BuildDataTree( CDataNode* treeroot ,
                                      CIOAccess* file     )
{GUCEF_TRACE;

    if ( GUCEF_NULL == file || GUCEF_NULL == treeroot )
        return false;
    
    CDynamicBuffer buffer;
    if ( buffer.Append( *file ) > 0 )
    {
        UInt32 bytesRead = 0;
        CDataNodeBinarySerializerOptions options;
        if ( CDataNodeBinaryContainerSerializer::Deserialize( options, false, *treeroot, 0, buffer, bytesRead ) )
        {
            return true;
        } 
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CString
CDataStoreBinaryCodec::GetName( void ) const
{GUCEF_TRACE;

    return "Build-in GUCEF data store codec for the \'binarydatanodes\' v" + ToString( CDataNodeBinaryContainerSerializer::CurrentFormatVersion ) + " format";
}

/*-------------------------------------------------------------------------*/

CString
CDataStoreBinaryCodec::GetTypeName( void ) const
{GUCEF_TRACE;
    
    return "binarydatanodes";
}

/*-------------------------------------------------------------------------*/

bool 
CDataStoreBinaryCodec::IsCodecTypeDataDriven( void ) const
{GUCEF_TRACE;

    return false; 
}

/*-------------------------------------------------------------------------*/

CString
CDataStoreBinaryCodec::GetCopyright( void ) const
{GUCEF_TRACE;

    return "Prevailing GUCEF license at " __TIMESTAMP__;
}

/*-------------------------------------------------------------------------*/

TVersion
CDataStoreBinaryCodec::GetVersion( void ) const
{GUCEF_TRACE;

    TVersion version;
    version.major = (Int16) CDataNodeBinaryContainerSerializer::CurrentFormatVersion;
    version.minor = 0;
    version.patch = 0;
    version.release = 0;
    return version;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataStoreBinaryCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataStoreBinaryCodec();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
