/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#ifndef GUCEF_WEB_CBROWSERDOWNLOADCAPTURE_H
#define GUCEF_WEB_CBROWSERDOWNLOADCAPTURE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#ifndef GUCEF_WEB_CBROWSERDOWNLOADSETTINGS_H
#include "gucefWEB_CBrowserDownloadSettings.h"
#define GUCEF_WEB_CBROWSERDOWNLOADSETTINGS_H
#endif /* GUCEF_WEB_CBROWSERDOWNLOADSETTINGS_H ? */

#ifndef GUCEF_WEB_CBROWSERDOWNLOADRESULT_H
#include "gucefWEB_CBrowserDownloadResult.h"
#define GUCEF_WEB_CBROWSERDOWNLOADRESULT_H
#endif /* GUCEF_WEB_CBROWSERDOWNLOADRESULT_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Lets the browser save the downloads of its pages into a run folder of the caller, checks them and hands them over
 *  under a sanitised name (gap G1 of the investment design, see the site discovery design section 12.3).
 *
 *  Begin() makes the run folder owner-only (0700) with a sub folder "incoming" and tells the browser
 *  (Browser.setDownloadBehavior, allowAndName) to save every download there under its guid. The browser reports each
 *  download (Browser.downloadWillBegin, Browser.downloadProgress); one that passes maxBytes is canceled. A finished one
 *  is checked in Process(): size, extension, content type, not an HTML page; then made owner-only (0600), hashed
 *  (SHA-256) and moved in one step into the run folder. Whatever fails a check is deleted.
 *
 *  The content type is the server's when a Network.responseReceived of the download's address was seen on any session
 *  of the client (WatchSession() turns those events on for a page session); otherwise it is sniffed.
 *
 *  Non-blocking: ExpectDownload() arms a wait, Process() ends it, GetWaitState() and PopResult() read the outcome.
 *  Events: DownloadStartedEvent and DownloadFinishedEvent (TBrowserDownloadResultEventData), WaitTimedOutEvent.
 *  Process() runs on the thread that drives the client's pulse generator; the members that send commands and wait for
 *  the answer (Begin(), End(), WatchSession()) as well. Not thread-safe otherwise.
 */
class GUCEF_WEB_PUBLIC_CPP CBrowserDownloadCapture : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent DownloadStartedEvent;
    static const CORE::CEvent DownloadFinishedEvent;
    static const CORE::CEvent WaitTimedOutEvent;

    static void RegisterEvents( void );

    static const CORE::CString IncomingDirectoryName;      /**< "incoming", the folder the browser writes into */

    enum EWaitState
    {
        WAIT_IDLE       ,   /**< no wait armed */
        WAIT_PENDING    ,   /**< armed, no download finished yet */
        WAIT_DONE       ,   /**< a download finished, see PopResult() */
        WAIT_TIMED_OUT      /**< none finished in time */
    };

    /** @param browserClient a client connected to the browser itself, which must outlive this object */
    explicit CBrowserDownloadCapture( CChromeDevToolsClient& browserClient );

    virtual ~CBrowserDownloadCapture();

    void SetSettings( const CBrowserDownloadSettings& settings );

    const CBrowserDownloadSettings& GetSettings( void ) const;

    /** Creates the run folder (owner-only) and its incoming folder, and has the browser save downloads there */
    bool Begin( const CORE::CString& runDirectory );

    /** Cancels the downloads in progress; afterwards the browser refuses downloads (denyDownloads) or asks as usual */
    bool End( bool denyDownloads = true );

    bool IsCapturing( void ) const;

    const CORE::CString& GetRunDirectory( void ) const;

    /** Turns on the network events of a page session, so the server's content type of its downloads is known */
    bool WatchSession( const CORE::CString& sessionId );

    /** Arms a wait for the next download that finishes, completed or not */
    bool ExpectDownload( UInt32 timeoutInMs );

    EWaitState GetWaitState( void ) const;

    /** Checks and moves finished downloads, ends the wait. @return the downloads finished */
    UInt32 Process( void );

    /** Takes the oldest finished download */
    bool PopResult( CBrowserDownloadResult& result );

    UInt32 GetFinishedCount( void ) const;

    UInt32 GetInProgressCount( void ) const;

    /**
     *  A file name for the run folder: the name part only, letters, digits, '.', '-' and '_' (anything else becomes '_',
     *  runs of '_' collapse), no leading dot, no reserved device name, at most maxLength bytes with the extension kept.
     *  "download" when nothing is left
     */
    static CORE::CString SanitizeFileName( const CORE::CString& suggestedName, UInt32 maxLength );

    /** The content type the first bytes and the extension suggest: pdf, zip and office files, html, xml, json, csv, text */
    static CORE::CString SniffMimeType( const CORE::CString& fileName, const void* head, UInt32 headSize );

    /** Is the content type in the list (exact, or a glob such as text/x-*)? An empty list allows every type */
    static bool IsMimeTypeAllowed( const CORE::CString::StringVector& allowed, const CORE::CString& mimeType );

    /** The extension of a file name, lower case, without the dot */
    static CORE::CString GetExtension( const CORE::CString& fileName );

    private:

    CBrowserDownloadCapture( const CBrowserDownloadCapture& src );                  /**< not implemented */
    CBrowserDownloadCapture& operator=( const CBrowserDownloadCapture& src );       /**< not implemented */

    typedef CORE::CTEventHandlerFunctor< CBrowserDownloadCapture > TEventCallback;

    struct SDownload
    {
        CBrowserDownloadResult result;
        CORE::CString stagedPath;
        CORE::CString refuseReason;     /**< set when we canceled it */
        bool browserFinished;
        bool browserCompleted;

        SDownload( void ) : result(), stagedPath(), refuseReason(), browserFinished( false ), browserCompleted( false ) {}
    };

    typedef GUCEF::map< CORE::CString, SDownload >                      TDownloadMap;
    typedef std::pair< CORE::CString, CORE::CString >                   TUrlMimePair;
    typedef GUCEF::vector< TUrlMimePair >                               TUrlMimeVector;

    void OnProtocolEvent( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void HandleWillBegin( const SChromeProtocolEvent& event );
    void HandleProgress( const SChromeProtocolEvent& event );
    void HandleResponse( const SChromeProtocolEvent& event );
    void Finish( SDownload& download );
    void CancelDownload( const CORE::CString& guid );
    static bool HashFile( const CORE::CString& path, CORE::CString& hexDigest );
    static CORE::CString MakeUrlTemplate( const CORE::CString& url );
    UInt64 NowInMs( void ) const;

    CChromeDevToolsClient& m_client;
    CBrowserDownloadSettings m_settings;
    CORE::CString m_runDirectory;
    CORE::CString m_incomingDirectory;
    bool m_capturing;
    TDownloadMap m_downloads;
    TBrowserDownloadResultVector m_results;
    TUrlMimeVector m_seenResponses;             /**< the latest responses: URL and content type, bounded */
    UInt32 m_finishedCount;
    EWaitState m_waitState;
    UInt64 m_waitDeadline;
    UInt32 m_waitBaseline;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CBROWSERDOWNLOADCAPTURE_H ? */
