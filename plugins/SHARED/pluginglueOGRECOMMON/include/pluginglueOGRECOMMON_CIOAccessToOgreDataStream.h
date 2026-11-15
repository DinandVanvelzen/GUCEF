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

#ifndef GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H
#define GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H

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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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
 *  An Ogre data stream that reads and writes through a GUCEF IO access object.
 *
 *  Positions and sizes are 64 bit on the GUCEF side. Ogre's stream interface is size_t, so on 32 bit builds a
 *  size that does not fit is reported as 0 ("not known"), which Ogre allows. The position of the IO access is
 *  set with Setpos(), whose result follows fsetpos(): 0 is success.
 *
 *  Either the stream shares the IO access object (the owning constructor) or it only refers to one that the
 *  caller keeps alive for as long as the stream is used (the non owning constructor).
 */
class CIOAccessToOgreDataStream : public Ogre::DataStream
{
    public:

    /**
     *  Shares the IO access object; the access mode follows what the object can do, or is reading only when that
     *  is asked for: some objects (a file that is loaded in memory) can be written while the data is not to be changed
     */
    CIOAccessToOgreDataStream( const CORE::IOAccessPtr& access ,
                               const Ogre::String& name        ,
                               bool readOnly = false           )
        : Ogre::DataStream( name, ModeOf( const_cast< CORE::CIOAccess* >( access.GetPointerAlways() ), readOnly ) )
        , m_keepAlive( access )
        , m_access( const_cast< CORE::CIOAccess* >( access.GetPointerAlways() ) )
        , m_sizeKnown( false )
    {GUCEF_TRACE;
        DetermineSize();
    }

    /** Refers to the IO access object, which must outlive the stream and is not closed by it */
    CIOAccessToOgreDataStream( CORE::CIOAccess& access  ,
                               const Ogre::String& name ,
                               bool readOnly = false    )
        : Ogre::DataStream( name, ModeOf( &access, readOnly ) )
        , m_keepAlive()
        , m_access( &access )
        , m_sizeKnown( false )
    {GUCEF_TRACE;
        DetermineSize();
    }

    virtual ~CIOAccessToOgreDataStream() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        close();
    }

    virtual size_t read( void* buf, size_t count ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_access || !isReadable() || 0 == count )
            return 0;

        // the IO access reads in blocks of at most 32 bits
        UInt8* dest = static_cast< UInt8* >( buf );
        size_t total = 0;
        while ( total < count )
        {
            const UInt32 chunk = (UInt32) ( ( count - total ) > (size_t) MaxChunk ? (size_t) MaxChunk : ( count - total ) );
            const UInt32 got = m_access->Read( dest + total, 1, chunk );
            total += got;
            if ( got < chunk )
                break;
        }
        return total;
    }

    virtual size_t write( const void* buf, size_t count ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_access || !isWriteable() || 0 == count )
            return 0;

        const UInt8* src = static_cast< const UInt8* >( buf );
        size_t total = 0;
        while ( total < count )
        {
            const UInt32 chunk = (UInt32) ( ( count - total ) > (size_t) MaxChunk ? (size_t) MaxChunk : ( count - total ) );
            const UInt32 put = m_access->Write( src + total, 1, chunk );
            total += put;
            if ( put < chunk )
                break;
        }

        // a stream that grows is only known by what the access says now
        DetermineSize();
        return total;
    }

    virtual void skip( long count ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_access )
            return;

        // as the file based Ogre streams: the position stays within the stream, it never goes before the start
        const Int64 current = (Int64) m_access->Tell();
        Int64 target = current + (Int64) count;
        if ( target < 0 )
            target = 0;
        if ( m_sizeKnown && target > (Int64) mSize )
            target = (Int64) mSize;
        SetPosition( (UInt64) target );
    }

    virtual void seek( size_t pos ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_access )
            return;

        UInt64 target = (UInt64) pos;
        if ( m_sizeKnown && target > (UInt64) mSize )
            target = (UInt64) mSize;
        SetPosition( target );
    }

    virtual size_t tell( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_access )
            return 0;
        return (size_t) m_access->Tell();
    }

    virtual bool eof( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( GUCEF_NULL == m_access )
            return true;

        // some IO access objects only know the end after a read went past it
        if ( m_sizeKnown )
            return m_access->Tell() >= (UInt64) mSize;
        return m_access->Eof();
    }

    virtual void close( void ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        // the access object is closed by whoever owns it, which is us only when nobody else holds it
        m_access = GUCEF_NULL;
        m_keepAlive.Unlink();
    }

    /** The IO access object the stream works on, NULL after close() */
    CORE::CIOAccess* GetIOAccess( void ) const
    {
        return m_access;
    }

    protected:

    enum { MaxChunk = 0x40000000 };

    static Ogre::uint16 ModeOf( CORE::CIOAccess* access, bool readOnly )
    {GUCEF_TRACE;

        Ogre::uint16 mode = 0;
        if ( GUCEF_NULL != access )
        {
            if ( access->IsReadable() )
                mode |= Ogre::DataStream::READ;
            if ( !readOnly && access->IsWriteable() )
                mode |= Ogre::DataStream::WRITE;
        }
        return mode;
    }

    /** Fsetpos semantics: 0 is success */
    void SetPosition( UInt64 position )
    {GUCEF_TRACE;

        if ( 0 != m_access->Setpos( position ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CIOAccessToOgreDataStream: could not move to position "
                << position << " of stream " << mName );
        }
    }

    void DetermineSize( void )
    {GUCEF_TRACE;

        mSize = 0;
        m_sizeKnown = false;
        if ( GUCEF_NULL == m_access )
            return;

        const UInt64 size = m_access->GetSize();
        if ( size <= (UInt64) ( (size_t) -1 ) )
        {
            mSize = (size_t) size;
            // the size of an unbuffered source can be 0 for "not known", an empty stream is also all that is there
            m_sizeKnown = true;
        }
    }

    private:

    CIOAccessToOgreDataStream( const CIOAccessToOgreDataStream& src );              /**< not implemented */
    CIOAccessToOgreDataStream& operator=( const CIOAccessToOgreDataStream& src );   /**< not implemented */

    private:

    CORE::IOAccessPtr m_keepAlive;
    CORE::CIOAccess* m_access;
    bool m_sizeKnown;
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

#endif /* GUCEF_PLUGINGLUE_OGRECOMMON_CIOACCESSTOOGREDATASTREAM_H ? */
