/*
 *  pluginglueOGRECOMMON: Header only adapters between GUCEF and the Ogre 1.x API, compiled into the plugins that use them
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

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_CVFSRESOURCETOOGREDATASTREAM_H
#define GUCEF_PLUGINGLUE_OGRECOMMON_CVFSRESOURCETOOGREDATASTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_VFS_CVFSHANDLE_H
#include "gucefVFS_CVFSHandle.h"
#define GUCEF_VFS_CVFSHANDLE_H
#endif /* GUCEF_VFS_CVFSHANDLE_H ? */

#ifndef GUCEF_VFS_ETYPES_H
#include "gucefVFS_ETypes.h"
#define GUCEF_VFS_ETYPES_H
#endif /* GUCEF_VFS_ETYPES_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H
#include "pluginglueOGRECOMMON_CIOAccessToOgreDataStream.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace OGRECOMMON {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An Ogre data stream over a resource of the VFS. The stream keeps the VFS resource (and with it the archive
 *  that handed it out) alive for as long as it exists, so the data stays valid while Ogre holds the stream.
 */
class CVfsResourceToOgreDataStream : public CIOAccessToOgreDataStream
{
    public:

    /**
     *  The name of the stream is the file name of the resource, unless another name is given. Resources of the VFS
     *  are mostly opened for reading and stay that way: the stream does not write unless asked to
     */
    CVfsResourceToOgreDataStream( VFS::TBasicVfsResourcePtr resource            ,
                                  const Ogre::String& name = Ogre::String()     ,
                                  bool readOnly = true                          )
        : CIOAccessToOgreDataStream( resource->GetAccess()                                      ,
                                     name.empty() ? Ogre::String( resource->GetFilename().C_String() ) : name ,
                                     readOnly                                                   )
        , m_resource( resource )
    {GUCEF_TRACE;
    }

    virtual ~CVfsResourceToOgreDataStream() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        // the access object goes first, then the resource that owns the data behind it
        close();
        m_resource.Unlink();
    }

    const VFS::TBasicVfsResourcePtr& GetResource( void ) const
    {
        return m_resource;
    }

    private:

    CVfsResourceToOgreDataStream( const CVfsResourceToOgreDataStream& src );              /**< not implemented */
    CVfsResourceToOgreDataStream& operator=( const CVfsResourceToOgreDataStream& src );   /**< not implemented */

    private:

    VFS::TBasicVfsResourcePtr m_resource;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OGRECOMMON */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_CVFSRESOURCETOOGREDATASTREAM_H ? */
