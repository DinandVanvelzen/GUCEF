/*
 *  assetcookerOGRE: GUCEF asset converter plugin: the cooker of the intermediate into Ogre 1.x runtime files
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

#ifndef GUCEF_ASSETCOOKER_OGRE_CBUFFERDATASTREAM_H
#define GUCEF_ASSETCOOKER_OGRE_CBUFFERDATASTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "OgreDataStream.h"

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_ASSETCOOKER_OGRE_MACROS_H
#include "assetcookerOGRE_macros.h"
#define GUCEF_ASSETCOOKER_OGRE_MACROS_H
#endif /* GUCEF_ASSETCOOKER_OGRE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCOOKER {
namespace OGRE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An Ogre data stream that writes into a GUCEF buffer, which grows as the serializers of Ogre write. The cooker
 *  serialises meshes and skeletons into it and stores the bytes through the VFS (StoreInVfs()).
 */
class GUCEF_ASSETCOOKER_OGRE_EXPORT_CPP CBufferDataStream : public Ogre::DataStream
{
    public:

    CBufferDataStream( void );

    virtual ~CBufferDataStream() GUCEF_VIRTUAL_OVERRIDE;

    virtual size_t read( void* buf, size_t count ) GUCEF_VIRTUAL_OVERRIDE;

    virtual size_t write( const void* buf, size_t count ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void skip( long count ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void seek( size_t pos ) GUCEF_VIRTUAL_OVERRIDE;

    virtual size_t tell( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool eof( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void close( void ) GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CDynamicBuffer& GetBuffer( void ) const;

    /** Writes the bytes to a file of the VFS, replacing it */
    bool StoreInVfs( const CORE::CString& vfsPath ) const;

    private:

    CORE::CDynamicBuffer m_buffer;
    size_t m_position;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRE */
}; /* namespace ASSETCOOKER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETCOOKER_OGRE_CBUFFERDATASTREAM_H ? */
