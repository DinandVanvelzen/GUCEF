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

#ifndef GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#include "gucefCORE_CVariantBinarySerializer.h"
#define GUCEF_CORE_CVARIANTBINARYSERIALIZER_H
#endif /* GUCEF_CORE_CVARIANTBINARYSERIALIZER_H ? */

#include "gucefCORE_CDataNodeBinarySerializer.h"

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

const CString CDataNodeBinarySerializerOptions::ClassTypeName = "GUCEF::CORE::CDataNodeBinarySerializerOptions";

const CString  CDataNodeBinaryContainerSerializer::MagicText = "DataNodeBinaryContainer";
const UInt8    CDataNodeBinaryContainerSerializer::CurrentFormatVersion = 1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataNodeBinarySerializerOptions::CDataNodeBinarySerializerOptions( void )
    : CIConfigurable()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataNodeBinarySerializerOptions::~CDataNodeBinarySerializerOptions()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDataNodeBinarySerializerOptions::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataNodeBinarySerializerOptions::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CString& 
CDataNodeBinarySerializerOptions::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

bool 
CDataNodeBinarySerializer::Serialize( const CDataNodeBinarySerializerOptions& options ,
                                      const CDataNode& node                           , 
                                      UInt32 currentTargetOffset                      , 
                                      CDynamicBuffer& target                          , 
                                      UInt32& bytesWritten                            )
{GUCEF_TRACE;
        
    try
    {
        // We scope this to limit local var memory during recursion
        UInt32 partBytesWritten = 0;
        {        
            // write node type
            Int32 nodeType = node.GetNodeType();
            partBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(Int32), &nodeType );
            currentTargetOffset += partBytesWritten;
            bytesWritten += partBytesWritten;
            if ( partBytesWritten != sizeof(Int32) )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Serialize: Failed to write nodeType field at offset " 
                    + ToString( currentTargetOffset - partBytesWritten )  );
                return false;
            }

            // write node name
            CVariant nodeNameVar;
            nodeNameVar.LinkTo( node.GetName() );
            partBytesWritten = 0;
            if ( !CORE::CVariantBinarySerializer::Serialize( nodeNameVar, currentTargetOffset, target, partBytesWritten ) )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Serialize: Failed to write node name field at offset " 
                    + ToString( currentTargetOffset )  );
                return false;
            }
            currentTargetOffset += partBytesWritten;
            bytesWritten += partBytesWritten;

            // write node value
            partBytesWritten = 0;
            if ( !CORE::CVariantBinarySerializer::Serialize( node.GetValue(), currentTargetOffset, target, partBytesWritten ) )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Serialize: Failed to write node value field at offset " 
                    + ToString( currentTargetOffset ) + " as " + ToString( node.GetValue().ByteSize() ) + " payload bytes, as variant type " + node.GetValue().GetTypeName() );
                return false;
            }
            currentTargetOffset += partBytesWritten;
            bytesWritten += partBytesWritten;

            // write attribute count, this helps deserialization
            UInt32 attCount = node.GetAttCount();
            partBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(UInt32), &attCount );
            currentTargetOffset += partBytesWritten;
            bytesWritten += partBytesWritten;
            if ( partBytesWritten != sizeof(UInt32) )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Serialize: Failed to write node attribute count field at offset " 
                    + ToString( currentTargetOffset - partBytesWritten ) );
                return false;
            }

            // Write all the key-value pair attributes
            CDataNode::TAttributeMap::const_iterator att = node.AttributeBegin();
            while ( att != node.AttributeEnd() )
            {
                CVariant attNameVar;
                attNameVar.LinkTo( (*att).first );
                partBytesWritten = 0;
                if ( !CORE::CVariantBinarySerializer::Serialize( attNameVar, currentTargetOffset, target, partBytesWritten ) )
                {
                    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Serialize: Failed to write node attribute field name at offset " 
                        + ToString( currentTargetOffset ) + " as " + ToString( node.GetValue().ByteSize() ) + " payload bytes, as variant type " + node.GetValue().GetTypeName() );
                    return false;
                }
                currentTargetOffset += partBytesWritten;
                bytesWritten += partBytesWritten;

                partBytesWritten = 0;
                if ( !CORE::CVariantBinarySerializer::Serialize( (*att).second, currentTargetOffset, target, partBytesWritten ) )
                {
                    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Serialize: Failed to write node attribute field value field at offset " 
                        + ToString( currentTargetOffset ) + " as " + ToString( node.GetValue().ByteSize() ) + " payload bytes,  as variant type " + node.GetValue().GetTypeName() );
                    return false;
                }
                currentTargetOffset += partBytesWritten;
                bytesWritten += partBytesWritten;

                ++att;
            }

            // write direct child count, this helps deserialization
            UInt32 childCount = node.GetNrOfDirectChildNodes();
            partBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(UInt32), &childCount );
            currentTargetOffset += partBytesWritten;
            bytesWritten += partBytesWritten;
            if ( partBytesWritten != sizeof(UInt32) )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Serialize: Failed to write node child count field at offset " 
                    + ToString( currentTargetOffset ) );
                return false;
            }
        }

        // Now go to the children, this is where things get recursive
        CDataNode::const_iterator ci = node.ConstBegin();
        while ( ci != node.ConstEnd() )
        {
            partBytesWritten = 0;
            if ( !Serialize( options, *(*ci), currentTargetOffset, target, partBytesWritten ) )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Serialize: Failed to serialize child node starting at offset " 
                    + ToString( currentTargetOffset ) );
                return false;
            }
            currentTargetOffset += partBytesWritten;
            bytesWritten += partBytesWritten;
            ++ci;
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, CString( "DataNodeBinarySerializer:Serialize: caught exception: " ) + e.what()  );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CDataNodeBinarySerializer::Deserialize( const CDataNodeBinarySerializerOptions& options ,
                                        bool linkWherePossible                          ,
                                        CDataNode& node                                 , 
                                        UInt32 currentSourceOffset                      , 
                                        const CDynamicBuffer& source                    ,
                                        UInt32& bytesRead                               )
{GUCEF_TRACE;

    try
    {
        // We scope this to limit local var memory during recursion
        UInt32 partBytesRead = 0;
        {
            // read the node type
            Int32 nodeType = source.ReadValue< Int32 >( currentSourceOffset );
            currentSourceOffset += sizeof(Int32);
            bytesRead += sizeof(Int32);
            node.SetNodeType( nodeType );
           
            // read the node name
            CVariant nodeNameVar;                
            UInt32 varByteSize = 0;
            if ( !CORE::CVariantBinarySerializer::Deserialize( nodeNameVar, currentSourceOffset, source, true, varByteSize ) )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Deserialize: Failed to read node name field at offset " 
                    + ToString( currentSourceOffset ) + " as " + ToString( node.GetValue().ByteSize() ) + " payload bytes, as variant type " + node.GetValue().GetTypeName() );
                return false;
            }
            currentSourceOffset += varByteSize;
            bytesRead += varByteSize;
            node.SetName( nodeNameVar.AsString() );

            // read the node value                
            varByteSize = 0;
            if ( !CORE::CVariantBinarySerializer::Deserialize( node.GetValue(), currentSourceOffset, source, linkWherePossible, varByteSize ) )
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Deserialize: Failed to read node value field at offset " 
                    + ToString( currentSourceOffset ) + " as " + ToString( node.GetValue().ByteSize() ) + " payload bytes, as variant type " + node.GetValue().GetTypeName() );
                return false;
            }
            currentSourceOffset += varByteSize;
            bytesRead += varByteSize;

            // read attribute count
            UInt32 attCount = source.ReadValue< Int32 >( currentSourceOffset );
            currentSourceOffset += sizeof(Int32);
            bytesRead += sizeof(Int32);

            // read all the key-value pair attributes
            for ( UInt32 i=0; i<attCount; ++i )
            {
                CVariant attKey;
                CVariant attValue;
            
                // read the attribute key
                varByteSize = 0;
                if ( !CORE::CVariantBinarySerializer::Deserialize( attKey, currentSourceOffset, source, linkWherePossible, varByteSize ) )
                {
                    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Deserialize: Failed to read node attribute name at offset " 
                        + ToString( currentSourceOffset ) + " as " + ToString( node.GetValue().ByteSize() ) + " payload bytes, as variant type " + node.GetValue().GetTypeName() );
                    return false;
                }
                currentSourceOffset += varByteSize;
                bytesRead += varByteSize;   
            
                // read the attribute value
                varByteSize = 0;
                if ( !CORE::CVariantBinarySerializer::Deserialize( attValue, currentSourceOffset, source, linkWherePossible, varByteSize ) )
                {
                    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Deserialize: Failed to read node attribute value at offset " 
                        + ToString( currentSourceOffset ) + " as " + ToString( node.GetValue().ByteSize() ) + " payload bytes, as variant type " + node.GetValue().GetTypeName() );
                    return false;
                }
                currentSourceOffset += varByteSize;
                bytesRead += varByteSize;  

                node.SetAttribute( attKey, attValue, linkWherePossible );
            }
        }

        // read direct child count
        UInt32 childCount = source.ReadValue< Int32 >( currentSourceOffset );
        currentSourceOffset += sizeof(Int32);
        bytesRead += sizeof(Int32);

        for ( UInt32 i=0; i<childCount; ++i )
        {
            CDataNode* childNode = node.AddChild();
            if ( GUCEF_NULL != childNode )
            {
                partBytesRead = 0;
                if ( !Deserialize( options, linkWherePossible, *childNode, currentSourceOffset, source, partBytesRead ) )
                {
                    GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Deserialize: Failed to deserialize child node starting at offset " 
                        + ToString( currentSourceOffset ) );
                    return false;
                }
                currentSourceOffset += partBytesRead;
                bytesRead += partBytesRead;
            }
            else
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "DataNodeBinarySerializer:Deserialize: Failed to add child node to prep for Deserialize" );
                return false;
            }
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, CString( "DataNodeBinarySerializer:Deserialize: caught exception: " ) + e.what()  );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDataNodeBinaryContainerSerializer::SerializeHeader( const CDataNodeBinarySerializerOptions& options ,
                                                     UInt32 currentTargetOffset                      ,
                                                     CORE::CDynamicBuffer& target                    ,
                                                     UInt32& bytesWritten                            )
{GUCEF_TRACE;

    // magic text bytes to help identify the blob as being a container representing a datanode DOM
    UInt32 newBytesWritten = target.CopyFrom( MagicText, currentTargetOffset, false );
    bytesWritten += newBytesWritten;
    currentTargetOffset += newBytesWritten;
    if ( newBytesWritten != MagicText.ByteSize()-1 )
        return false;

    // Reserved for the future version field
    UInt8 formatVersion = CurrentFormatVersion;
    newBytesWritten = target.CopyFrom( currentTargetOffset, sizeof(formatVersion), &formatVersion );
    bytesWritten += newBytesWritten;
    currentTargetOffset += newBytesWritten;
    if ( newBytesWritten != sizeof(formatVersion) )
        return false;

    // Since the serializer settings used have to match to ensure compatibility we will
    // store those in the header so that we know the format of each node
                // TODO - once relevant

    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CDataNodeBinaryContainerSerializer::Serialize( const CDataNodeBinarySerializerOptions& options ,
                                               const CDataNode& node                           , 
                                               UInt32 currentTargetOffset                      , 
                                               CDynamicBuffer& target                          , 
                                               UInt32& bytesWritten                            )
{GUCEF_TRACE;

    // the node follows the header
    UInt32 headerBytesWritten = 0;
    if ( SerializeHeader( options             ,
                          currentTargetOffset ,
                          target              ,
                          headerBytesWritten  ) )
    {
        bytesWritten += headerBytesWritten;
        return CDataNodeBinarySerializer::Serialize( options                                  ,
                                                     node                                     ,
                                                     currentTargetOffset + headerBytesWritten ,
                                                     target                                   ,
                                                     bytesWritten                             );
    }
    bytesWritten += headerBytesWritten;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDataNodeBinaryContainerSerializer::DeserializeHeader( CDataNodeBinarySerializerOptions& options ,
                                                       UInt32 currentSourceOffset                ,
                                                       const CORE::CDynamicBuffer& source        ,
                                                       UInt32& bytesRead                         )
{GUCEF_TRACE;

    try
    {
        // magic text bytes to help identify the blob as being a container of pub sub messages
        CORE::CString testStr;
        UInt32 newBytesRead = source.CopyTo( currentSourceOffset, MagicText.ByteSize()-1, testStr.Reserve( MagicText.ByteSize(), (Int32) MagicText.Length() ) );
        bytesRead += newBytesRead;
        currentSourceOffset += newBytesRead;
        if ( newBytesRead != MagicText.ByteSize()-1 || testStr != MagicText )
        {
            // We dont want to log errors or warnings for this because header reading magic is a typical way to brute force
            // check if the resource is of a certain type
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DataNodeBinaryContainerSerializer:DeserializeHeader: Magic text test failed" );
            return false;
        }

        UInt8 formatVersion = source.AsConstType< UInt8 >( currentSourceOffset );
        bytesRead += sizeof(formatVersion);
        currentSourceOffset += sizeof(formatVersion);
        if ( CurrentFormatVersion != formatVersion )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DataNodeBinaryContainerSerializer:DeserializeHeader: Incompatible format version" );
            return false;
        }

        // Since the serializer settings used have to match to ensure compatibility we will
        // read those from the header so that we know the format of each node
                    // TODO - once relevant

        return true;
    }
    catch ( const std::exception& )
    {
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CDataNodeBinaryContainerSerializer::Deserialize( CDataNodeBinarySerializerOptions& options ,
                                                 bool linkWherePossible                    ,
                                                 CDataNode& node                           , 
                                                 UInt32 currentSourceOffset                , 
                                                 const CDynamicBuffer& source              ,
                                                 UInt32& bytesRead                         )
{GUCEF_TRACE;

    // the node follows the header
    UInt32 headerBytesRead = 0;
    if ( DeserializeHeader( options             ,
                            currentSourceOffset ,
                            source              ,
                            headerBytesRead     ) )
    {
        bytesRead += headerBytesRead;
        return CDataNodeBinarySerializer::Deserialize( options                               ,
                                                       linkWherePossible                     ,
                                                       node                                  ,
                                                       currentSourceOffset + headerBytesRead ,
                                                       source                                ,
                                                       bytesRead                             );
    }
    bytesRead += headerBytesRead;
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
