/*
 *  guidriverRmlUI: GUI backend using RmlUI
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

#ifndef GUCEF_guidriverRmlUI_CRmlUISTREAMADAPTER_H
#define GUCEF_guidriverRmlUI_CRmlUISTREAMADAPTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef RmlUICORESTREAM_H
#include "RmlUI/Core/Stream.h"
#define RmlUICORESTREAM_H
#endif /* RmlUICORESTREAM_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef GUCEF_guidriverRmlUI_MACROS_H
#include "guidriverRmlUI_macros.h"
#define GUCEF_guidriverRmlUI_MACROS_H
#endif /* GUCEF_guidriverRmlUI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace guidriverRmlUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class GUCEF_guidriverRmlUI_PUBLIC_CPP CRmlUIStreamAdapter : public RmlUI::Core::Stream
{
	public:

	CRmlUIStreamAdapter( CORE::CIOAccess& access );

	virtual ~CRmlUIStreamAdapter();

	/// Closes the stream.
	virtual void Close();

	/// Returns the mode the stream was opened in.
	int GetStreamMode() const;

	/// Obtain the source url of this stream (if available)
	const RmlUI::Core::URL& GetSourceURL() const;

	/// Are we at the end of the stream
	virtual bool IsEOS() const;

	/// Returns the size of this stream (in bytes).
	virtual size_t Length() const;

	/// Returns the position of the stream pointer (in bytes).
	virtual size_t Tell() const;
	
    /// Sets the stream position (in bytes).
	virtual bool Seek( long offset, int origin ) const;

	/// Read from the stream.
	virtual size_t Read( void* buffer, size_t bytes ) const;
	
    /// Read from the stream into another stream.
	virtual size_t Read (RmlUI::Core::Stream* stream, size_t bytes ) const;
	
    /// Read from the stream and append to the string buffer
	virtual size_t Read( RmlUI::Core::String& buffer, size_t bytes ) const;

	/// Read from the stream, without increasing the stream offset.
	virtual size_t Peek( void* buffer, size_t bytes ) const;

	/// Write to the stream at the current position.
	virtual size_t Write( const void* buffer, size_t bytes );

	/// Write to this stream from another stream.
	virtual size_t Write( const Stream* stream, size_t bytes );

	/// Write a character array to the stream.
	virtual size_t Write( const char* string );

	/// Write a string to the stream
	virtual size_t Write( const RmlUI::Core::String& string );

	/// Truncate the stream to the specified length.
	virtual size_t Truncate( size_t bytes );

	/// Push onto the front of the stream.
	virtual size_t PushFront( const void* buffer, size_t bytes );

	/// Push onto the back of the stream.
	virtual size_t PushBack( const void* buffer, size_t bytes );

	/// Pop from the front of the stream.
	virtual size_t PopFront( size_t bytes );

	/// Pop from the back of the stream.
	virtual size_t PopBack( size_t bytes );

	/// Returns true if the stream is ready for reading, false otherwise.
	/// This is usually only implemented on streams supporting asynchronous
	/// operations.
	virtual bool IsReadReady();

	/// Returns true if the stream is ready for writing, false otherwise.
	/// This is usually only implemented on streams supporting asynchronous
	/// operations.
	virtual bool IsWriteReady();

    protected:

	/// Sets the mode on the stream; should be called by a stream when it is opened.
	void SetStreamDetails( const RmlUI::Core::URL& url, int stream_mode );		

	/// Deletes the stream.
	virtual void OnReferenceDeactivate();

    private:

    CRmlUIStreamAdapter( void );
    CRmlUIStreamAdapter( const CRmlUIStreamAdapter& src );

    private:

    CORE::CIOAccess* m_access;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace guidriverRmlUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
          
#endif /* GUCEF_guidriverRmlUI_CRmlUISTREAMADAPTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 18-08-2007 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/

