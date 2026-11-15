/*
 *  guidriverMyGUI: GUI backend using MyGUI
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

#ifndef GUCEF_guidriverMyGUI_CMYGUIVFSDATASTREAM_H
#define GUCEF_guidriverMyGUI_CMYGUIVFSDATASTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "MyGUI_IDataStream.h"

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_guidriverMyGUI_MACROS_H
#include "guidriverMyGUI_macros.h"
#define GUCEF_guidriverMyGUI_MACROS_H
#endif /* GUCEF_guidriverMyGUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverMyGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A MyGUI data stream on a file of the virtual file system: the whole file is loaded when the
 *  stream is made, MyGUI reads from the memory copy, so a read never touches a file system.
 */
class GUCEF_guidriverMyGUI_PUBLIC_CPP CMyGUIVfsDataStream : public MyGUI::IDataStream
{
    public:

    /** Loads the file; IsValid() tells whether that worked */
    explicit CMyGUIVfsDataStream( const CORE::CString& vfsPath );

    /** A stream on a copy of the given bytes */
    CMyGUIVfsDataStream( const void* data, size_t size );

    virtual ~CMyGUIVfsDataStream();

    bool IsValid( void ) const;

    virtual bool eof() override;

    virtual size_t size() override;

    virtual void readline( std::string& source, MyGUI::Char delimiter = '\n' ) override;

    virtual size_t read( void* buffer, size_t count ) override;

    private:

    CMyGUIVfsDataStream( const CMyGUIVfsDataStream& src );
    CMyGUIVfsDataStream& operator=( const CMyGUIVfsDataStream& src );

    private:

    CORE::CDynamicBuffer m_buffer;
    size_t m_position;
    bool m_isValid;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverMyGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_guidriverMyGUI_CMYGUIVFSDATASTREAM_H ? */
