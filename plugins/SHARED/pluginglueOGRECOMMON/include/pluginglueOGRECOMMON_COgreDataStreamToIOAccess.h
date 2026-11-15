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

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_COGREDATASTREAMTOIOACCESS_H
#define GUCEF_PLUGINGLUE_OGRECOMMON_COGREDATASTREAMTOIOACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>

#include "OgreDataStream.h"

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H
#include "pluginglueOGRECOMMON_macros.h"
#define GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H
#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_MACROS_H ? */

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
 *  A GUCEF IO access object over an Ogre data stream, for handing Ogre data to code that works on CIOAccess.
 *
 *  The position is set the way CIOAccess does: Seek() and Setpos() return 0 on success and non zero on failure,
 *  as fseek() and fsetpos() do. Positions before the start and, for streams of known size, beyond the end fail.
 *  The access object shares the Ogre stream.
 */
class COgreDataStreamToIOAccess : public CORE::CIOAccess
{
    public:

    explicit COgreDataStreamToIOAccess( const Ogre::DataStreamPtr& stream )
        : CORE::CIOAccess()
        , m_stream( stream )
    {GUCEF_TRACE;
    }

    COgreDataStreamToIOAccess( const COgreDataStreamToIOAccess& src )
        : CORE::CIOAccess( src )
        , m_stream( src.m_stream )
    {GUCEF_TRACE;
    }

    virtual ~COgreDataStreamToIOAccess() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        // an Ogre stream is open from the moment it exists
    }

    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( m_stream )
            m_stream->close();
        m_stream.reset();
    }

    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return (bool) m_stream;
    }

    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !m_stream || !m_stream->isReadable() || 0 == esize || 0 == elements )
            return 0;

        const size_t bytes = m_stream->read( dest, (size_t) esize * (size_t) elements );
        return (UInt32) ( bytes / esize );
    }

    virtual UInt32 Write( const void* srcdata, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !m_stream || !m_stream->isWriteable() || 0 == esize || 0 == elements )
            return 0;

        const size_t bytes = m_stream->write( srcdata, (size_t) esize * (size_t) elements );
        return (UInt32) ( bytes / esize );
    }

    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !m_stream )
            return 0;
        return (UInt64) m_stream->tell();
    }

    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !m_stream )
            return -1;

        Int64 base = 0;
        switch ( origin )
        {
            case SEEK_SET : { base = 0; break; }
            case SEEK_CUR : { base = (Int64) m_stream->tell(); break; }
            case SEEK_END : { base = (Int64) m_stream->size(); break; }
            default : { return -1; }
        }

        const Int64 target = base + offset;
        if ( target < 0 )
            return -1;

        // Ogre streams do not go beyond their end, a size of 0 means the size is not known
        const size_t size = m_stream->size();
        if ( 0 != size && (UInt64) target > (UInt64) size )
            return -1;

        m_stream->seek( (size_t) target );
        return (Int64) m_stream->tell() == target ? 0 : -1;
    }

    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        char c = 0;
        if ( m_stream && m_stream->isReadable() )
            m_stream->read( &c, 1 );
        return c;
    }

    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return !m_stream || m_stream->eof();
    }

    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return m_stream && m_stream->isReadable();
    }

    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return m_stream && m_stream->isWriteable();
    }

    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return (bool) m_stream;
    }

    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( !m_stream )
            return 0;
        return (UInt64) m_stream->size();
    }

    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        // Ogre streams have no buffer of their own to flush
    }

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return GUCEF_NEW COgreDataStreamToIOAccess( *this );
    }

    private:

    COgreDataStreamToIOAccess& operator=( const COgreDataStreamToIOAccess& src );   /**< not implemented */

    private:

    Ogre::DataStreamPtr m_stream;
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

#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_COGREDATASTREAMTOIOACCESS_H ? */
