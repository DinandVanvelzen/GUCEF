/*
 *  ImageConvertor: converts images between the formats of the loaded image codecs
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
/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <list>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_IMAGE_CIMAGECONVERTER_H
#include "gucefIMAGE_CImageConverter.h"
#define GUCEF_IMAGE_CIMAGECONVERTER_H
#endif /* GUCEF_IMAGE_CIMAGECONVERTER_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H
#include "gucefIMAGE_CImageCodecCapabilities.h"
#define GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H
#endif /* GUCEF_IMAGE_CIMAGECODECCAPABILITIES_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#include "ImageConvertor_CImageConvertorApp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGECONVERTOR {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CImageConvertorApp::AppName = "ImageConvertor";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const PluginGroupName = "imageconvertor";
const char* const ThreadPoolName = "ImageConvertor::Convert";
const UInt32 JobsQueuedPerThread = 2;

/** One file to convert: where from, where to and, once done, how that went */
class CConversionJob : public CORE::CICloneable
{
    public:

    enum EOutcome { OUTCOME_NONE, OUTCOME_CONVERTED, OUTCOME_SKIPPED, OUTCOME_FAILED };

    CORE::CString inputPath;
    CORE::CString fromType;
    CORE::CString toType;
    CORE::CString outputPath;               /**< the file, or the file without its extension and frame number when the frames go to separate files */
    bool overwrite;
    IMAGE::CImageConversionSettings settings;

    EOutcome outcome;
    CORE::CString message;
    CORE::CString::StringVector notes;
    UInt32 filesWritten;
    CORE::CString writtenAs;                /**< what was written: the file, or the pattern of the files of the frames */

    CConversionJob( void ) : inputPath(), fromType(), toType(), outputPath(), overwrite( false ), settings(),
                             outcome( OUTCOME_NONE ), message(), notes(), filesWritten( 0 ), writtenAs() {}

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return GUCEF_NEW CConversionJob( *this );
    }
};

/*-------------------------------------------------------------------------*/

CORE::CString
LowerExtension( const CORE::CString& path )
{GUCEF_TRACE;

    return CORE::ExtractFileExtention( path ).Lowercase();
}

/*-------------------------------------------------------------------------*/

CORE::CString
FrameFilePath( const CORE::CString& stem, UInt32 index, const CORE::CString& type )
{GUCEF_TRACE;

    CORE::CString number = CORE::UInt32ToString( index );
    while ( number.Length() < 3 )
        number = "0" + number;
    return stem + "_" + number + "." + type;
}

/*-------------------------------------------------------------------------*/

bool
ReadWholeFile( const CORE::CString& path, CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return false;
    const UInt64 size = file.GetSize();
    if ( size > 0x7FFFFFFF )
        return false;
    content.SetDataSize( (UInt32) size );
    return 0 == size || 1 == file.Read( content.GetBufferPtr(), (UInt32) size, 1 );
}

/*-------------------------------------------------------------------------*/

bool
WriteWholeFile( const CORE::CString& path, const CORE::CDynamicBuffer& content )
{GUCEF_TRACE;

    if ( !CORE::CreatePathDirectories( path ) )
        return false;
    bool success = false;
    {
        CORE::CFileAccess file( path, "wb" );
        success = file.Opened() && ( 0 == content.GetDataSize() || 1 == file.Write( content.GetConstBufferPtr(), content.GetDataSize(), 1 ) );
    }
    if ( !success )
        CORE::DeleteFile( path );
    return success;
}

/*-------------------------------------------------------------------------*/

/** Converts the file of the job: the work of a task, or of the calling thread. The files are read and written here, the pixels never leave the thread */
void
RunJob( CConversionJob& job )
{GUCEF_TRACE;

    job.outcome = CConversionJob::OUTCOME_FAILED;
    job.notes.clear();
    job.filesWritten = 0;
    job.writtenAs.Clear();

    IMAGE::CImageCodecCapabilities::SImageCodecInfo target;
    if ( !IMAGE::CImageCodecCapabilities::TryGetCodec( job.toType, target ) )
    {
        job.message = "there is no image codec for the format \"" + job.toType + "\"";
        return;
    }

    // frames go to separate files when the target holds one picture and no frame was chosen
    const bool toFiles = !target.multipleFrames && job.settings.IsAllFrames();
    if ( !job.overwrite )
    {
        const CORE::CString plain = toFiles ? job.outputPath + "." + job.toType : job.outputPath;
        const CORE::CString first = toFiles ? FrameFilePath( job.outputPath, 0, job.toType ) : job.outputPath;
        if ( CORE::FileExists( plain ) || CORE::FileExists( first ) )
        {
            job.outcome = CConversionJob::OUTCOME_SKIPPED;
            job.message = "the output exists, --overwrite replaces it";
            return;
        }
    }

    CORE::CDynamicBuffer input;
    if ( !ReadWholeFile( job.inputPath, input ) )
    {
        job.message = "the file could not be read";
        return;
    }
    CORE::CDynamicBufferAccess inputAccess( &input, false );
    inputAccess.Setpos( 0 );

    IMAGE::CImageConverter converter;
    IMAGE::CImageConversionResult result;
    if ( toFiles )
    {
        IMAGE::CImageConverter::TEncodedFrameVector frames;
        if ( !converter.ConvertToFrames( inputAccess, job.fromType, job.toType, job.settings, result, frames ) )
        {
            job.message = result.GetProblem();
            return;
        }
        for ( size_t i=0; i<frames.size(); ++i )
        {
            const CORE::CString path = 1 == frames.size() ? job.outputPath + "." + job.toType : FrameFilePath( job.outputPath, frames[ i ].frameIndex, job.toType );
            if ( !WriteWholeFile( path, frames[ i ].data ) )
            {
                job.message = "the file " + path + " could not be written";
                return;
            }
            ++job.filesWritten;
        }
        job.writtenAs = 1 == frames.size() ? job.outputPath + "." + job.toType : job.outputPath + "_NNN." + job.toType;
    }
    else
    {
        CORE::CDynamicBuffer output;
        CORE::CDynamicBufferAccess outputAccess( &output, false );
        if ( !converter.Convert( inputAccess, job.fromType, outputAccess, job.toType, job.settings, result ) )
        {
            job.message = result.GetProblem();
            return;
        }
        if ( !WriteWholeFile( job.outputPath, output ) )
        {
            job.message = "the file " + job.outputPath + " could not be written";
            return;
        }
        job.filesWritten = 1;
        job.writtenAs = job.outputPath;
    }
    job.notes = result.GetNotes();
    job.outcome = CConversionJob::OUTCOME_CONVERTED;
    job.message.Clear();
}

/*-------------------------------------------------------------------------*/

/** The task that runs a job on a thread of the pool */
class CConvertTask : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE
    {
        CConversionJob* job = static_cast< CConversionJob* >( task->GetTaskData() );
        if ( GUCEF_NULL != job )
            RunJob( *job );
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) GUCEF_VIRTUAL_OVERRIDE
    {
    }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return TaskType;
    }
};

const CORE::CString CConvertTask::TaskType = "ImageConvertor::Convert";

typedef CORE::CTFactory< CORE::CTaskConsumer, CConvertTask, MT::CMutex > TConvertTaskFactory;

/*-------------------------------------------------------------------------*/

/** What the run of the jobs found, for the summary */
struct SRunTotals
{
    UInt32 converted;
    UInt32 skipped;
    UInt32 failed;
    UInt32 files;
    UInt32 lossy;

    SRunTotals( void ) : converted( 0 ), skipped( 0 ), failed( 0 ), files( 0 ), lossy( 0 ) {}
};

/*-------------------------------------------------------------------------*/

void
Report( const CConversionJob& job, SRunTotals& totals, bool quiet )
{GUCEF_TRACE;

    switch ( job.outcome )
    {
        case CConversionJob::OUTCOME_CONVERTED :
        {
            ++totals.converted;
            totals.files += job.filesWritten;
            if ( !quiet )
                GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, job.inputPath << " -> " << job.writtenAs );
            for ( size_t i=0; i<job.notes.size(); ++i )
                GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, "  note: " << job.notes[ i ] );
            if ( !job.notes.empty() )
                ++totals.lossy;
            break;
        }
        case CConversionJob::OUTCOME_SKIPPED :
        {
            ++totals.skipped;
            if ( !quiet )
                GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, job.inputPath << ": skipped, " << job.message );
            break;
        }
        default :
        {
            ++totals.failed;
            GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_CRITICAL, job.inputPath << ": failed, " << job.message );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

/** Runs the jobs on the thread of the caller */
void
RunJobsHere( std::list< CConversionJob >& jobs, SRunTotals& totals, bool quiet )
{GUCEF_TRACE;

    for ( std::list< CConversionJob >::iterator i=jobs.begin(); i!=jobs.end(); ++i )
    {
        RunJob( *i );
        Report( *i, totals, quiet );
    }
}

/*-------------------------------------------------------------------------*/

/** Runs the jobs on the threads of a pool, a few more than the threads are queued at a time */
void
RunJobsOnPool( std::list< CConversionJob >& jobs, UInt32 threads, SRunTotals& totals, bool quiet )
{GUCEF_TRACE;

    TConvertTaskFactory factory;
    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.RegisterTaskConsumerFactory( CConvertTask::TaskType, &factory );
    CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( ThreadPoolName );
    if ( pool.IsNULL() )
    {
        taskManager.UnregisterTaskConsumerFactory( CConvertTask::TaskType );
        RunJobsHere( jobs, totals, quiet );
        return;
    }
    pool->SetDesiredMinNrOfWorkerThreads( threads );

    typedef std::list< CORE::CFutureResult > TFutureList;
    TFutureList inProgress;
    std::list< CConversionJob >::iterator next = jobs.begin();
    while ( next != jobs.end() || !inProgress.empty() )
    {
        // collect what is done
        bool any = false;
        for ( TFutureList::iterator i=inProgress.begin(); i!=inProgress.end(); )
        {
            if ( i->HasAFuture() && !i->IsReady() )
            {
                ++i;
                continue;
            }
            CORE::CTaskPtr task = i->HasAFuture() ? i->GetResult() : CORE::CTaskPtr();
            const CConversionJob* done = task.IsNULL() ? GUCEF_NULL : static_cast< const CConversionJob* >( task->GetTaskData() );
            if ( GUCEF_NULL != done )
                Report( *done, totals, quiet );
            else
                ++totals.failed;
            i = inProgress.erase( i );
            any = true;
        }

        // queue more
        while ( next != jobs.end() && inProgress.size() < (size_t) threads * JobsQueuedPerThread )
        {
            CORE::CFutureResult future = pool->QueueTask( CConvertTask::TaskType, &(*next) );
            if ( future.HasAFuture() )
            {
                inProgress.push_back( future );
            }
            else
            {
                RunJob( *next );
                Report( *next, totals, quiet );
            }
            ++next;
            any = true;
        }
        if ( !any )
            MT::ThreadDelay( 2 );
    }
    taskManager.UnregisterThreadPool( ThreadPoolName );
    taskManager.UnregisterTaskConsumerFactory( CConvertTask::TaskType );
}

/*-------------------------------------------------------------------------*/

bool
LoadImageCodecPlugin( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CPluginMetaData pluginMetaData;
    pluginMetaData.SetPluginType( "GucefImageCodecPlugin" );
    pluginMetaData.SetFullModulePath( CORE::StripFilename( CORE::RelativePath( path ) ) );
    pluginMetaData.SetModuleFilename( CORE::StripFileExtention( CORE::ExtractFilename( path ) ) );
    pluginMetaData.SetAllLinkBackModules( "gucefCORE,gucefIMAGE" );
    return CORE::CCoreGlobal::Instance()->GetPluginControl().AddPluginMetaData( pluginMetaData, PluginGroupName, true, true );
}

/*-------------------------------------------------------------------------*/

void
CollectFiles( const CORE::CString& folder, bool recursive, CORE::CString::StringVector& files )
{GUCEF_TRACE;

    CORE::CFileSystemIterator iterator;
    if ( !iterator.FindFirst( folder ) )
        return;
    CORE::CString::StringVector subFolders;
    do
    {
        if ( iterator.IsADirectory() )
            subFolders.push_back( iterator.GetResourcePath() );
        else if ( iterator.IsAFile() )
            files.push_back( iterator.GetResourcePath() );
    }
    while ( iterator.FindNext() );
    iterator.FindClose();

    if ( recursive )
        for ( size_t i=0; i<subFolders.size(); ++i )
            CollectFiles( subFolders[ i ], true, files );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImageConvertorApp::CImageConvertorApp( int argc, char** argv )
    : m_argc( argc )
    , m_argv( argv )
    , m_bootstrapConfigFile()
    , m_minimalLogLevel( CORE::LOGLEVEL_IMPORTANT )
    , m_inputs()
    , m_plugins()
    , m_output()
    , m_toType()
    , m_fromType()
    , m_recursive( false )
    , m_mirror( false )
    , m_overwrite( false )
    , m_dryRun( false )
    , m_listCodecs( false )
    , m_quiet( false )
    , m_jobs( 1 )
    , m_settings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CImageConvertorApp::~CImageConvertorApp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CImageConvertorApp::LogUsage( void )
{GUCEF_TRACE;

    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, AppName << " converts images between the formats of the loaded image codecs\n"
        "  ImageConvertor [options] <input>...      files, or folders of files\n"
        "  -o, --output <file|folder>               the file (one input) or the folder of the converted files; default: next to the input\n"
        "  -t, --to <type>                          the target format by file extension (png, jpg, tga, gupimg, ...); default: that of --output\n"
        "  -f, --from <type>                        the format of the input when its extension does not say\n"
        "  -r, --recursive                          searches the folders below\n"
        "      --mirror                             keeps the folder structure of the input folders under --output\n"
        "      --frame <n>                          converts only that frame\n"
        "      --overwrite                          replaces outputs that exist; the default skips them\n"
        "      --no-lossy                           refuses conversions that lose information instead of reporting them\n"
        "      --compression <name>                 gupimg output: the compression codec, 'none' for none; default zlib\n"
        "  -j, --jobs <n>                           conversions at the same time; default 1, 0 is one per logical CPU\n"
        "      --plugin <module>                    loads an image codec plugin (a path without the extension)\n"
        "      --list-codecs                        lists the formats with what they can do, and stops\n"
        "      --dry-run                            shows what would be converted\n"
        "  -q, --quiet, -v, --verbose, --bootstrap <ini>, -h, --help\n"
        "Animations go to one file where the target holds frames, else to a file per frame: name_000.png, name_001.png, ...\n"
        "Exit code: 0 done, 1 wrong command line, 2 a file failed, 3 configuration" );
}

/*-------------------------------------------------------------------------*/

bool
CImageConvertorApp::ParseCommandLine( void )
{GUCEF_TRACE;

    for ( int i=1; i<m_argc; ++i )
    {
        const CORE::CString arg( m_argv[ i ] );
        const bool hasValue = i+1 < m_argc;
        if ( "--help" == arg || "-h" == arg )
            return false;
        else if ( ( "--output" == arg || "-o" == arg ) && hasValue )
            m_output = m_argv[ ++i ];
        else if ( ( "--to" == arg || "-t" == arg ) && hasValue )
            m_toType = CORE::CString( m_argv[ ++i ] ).Lowercase();
        else if ( ( "--from" == arg || "-f" == arg ) && hasValue )
            m_fromType = CORE::CString( m_argv[ ++i ] ).Lowercase();
        else if ( "--recursive" == arg || "-r" == arg )
            m_recursive = true;
        else if ( "--mirror" == arg )
            m_mirror = true;
        else if ( "--frame" == arg && hasValue )
            m_settings.SetFrameIndex( CORE::StringToUInt32( m_argv[ ++i ] ) );
        else if ( "--overwrite" == arg )
            m_overwrite = true;
        else if ( "--no-lossy" == arg )
            m_settings.SetAllowLossyConversion( false );
        else if ( "--compression" == arg && hasValue )
            m_settings.GetEncoderParams().Set( "compression", CORE::CVariant( CORE::CString( m_argv[ ++i ] ) ) );
        else if ( ( "--jobs" == arg || "-j" == arg ) && hasValue )
            m_jobs = CORE::StringToInt32( m_argv[ ++i ] );
        else if ( "--plugin" == arg && hasValue )
            m_plugins.push_back( m_argv[ ++i ] );
        else if ( "--list-codecs" == arg )
            m_listCodecs = true;
        else if ( "--dry-run" == arg )
            m_dryRun = true;
        else if ( "--quiet" == arg || "-q" == arg )
        {
            m_quiet = true;
            m_minimalLogLevel = CORE::LOGLEVEL_CRITICAL;
        }
        else if ( "--verbose" == arg || "-v" == arg )
            m_minimalLogLevel = CORE::LOGLEVEL_NORMAL;
        else if ( "--bootstrap" == arg && hasValue )
            m_bootstrapConfigFile = m_argv[ ++i ];
        else if ( arg.Length() > 0 && '-' != arg[ 0 ] )
            m_inputs.push_back( arg );
        else
        {
            GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, "Unknown or incomplete option: " << arg );
            return false;
        }
    }
    if ( m_listCodecs )
        return true;
    if ( m_inputs.empty() )
    {
        GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, "There is nothing to convert: give files or folders" );
        return false;
    }
    if ( m_toType.IsNULLOrEmpty() && !m_output.IsNULLOrEmpty() )
        m_toType = LowerExtension( m_output );
    if ( m_toType.IsNULLOrEmpty() )
    {
        GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, "The target format is not known: give --to, or an --output that has an extension" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImageConvertorApp::GetBootstrapConfigFile( void ) const
{GUCEF_TRACE;

    return m_bootstrapConfigFile;
}

/*-------------------------------------------------------------------------*/

Int32
CImageConvertorApp::GetMinimalLogLevel( void ) const
{GUCEF_TRACE;

    return m_minimalLogLevel;
}

/*-------------------------------------------------------------------------*/

Int32
CImageConvertorApp::Run( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_plugins.size(); ++i )
    {
        if ( !LoadImageCodecPlugin( m_plugins[ i ] ) )
        {
            GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_CRITICAL, "The image codec plugin " << m_plugins[ i ] << " could not be loaded" );
            return ExitConfig;
        }
    }

    if ( m_listCodecs )
    {
        IMAGE::CImageCodecCapabilities::TImageCodecInfoVector codecs;
        IMAGE::CImageCodecCapabilities::GetCodecs( codecs );
        GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, "format      read  write  frames  mipmaps" );
        for ( size_t i=0; i<codecs.size(); ++i )
        {
            CORE::CString type = codecs[ i ].type;
            while ( type.Length() < 11 )
                type += " ";
            GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, type << " " << ( codecs[ i ].canDecode ? "yes " : "-   " ) << "  " << ( codecs[ i ].canEncode ? "yes  " : "-    " ) << "  " <<
                               ( codecs[ i ].multipleFrames ? "yes    " : "-      " ) << "  " << ( codecs[ i ].mipLevels ? "yes" : "-" ) );
        }
        return ExitOk;
    }

    IMAGE::CImageCodecCapabilities::SImageCodecInfo target;
    if ( !IMAGE::CImageCodecCapabilities::TryGetCodec( m_toType, target ) || !target.canEncode )
    {
        GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_CRITICAL, "The format \"" << m_toType << "\" cannot be written, --list-codecs shows what can" );
        return ExitUsage;
    }

    // the files: an input file is converted whatever its extension, the files of a folder when their format can be read
    const bool outputIsFolder = m_output.IsNULLOrEmpty() ? false : ( CORE::DirExists( m_output ) || m_inputs.size() > 1 || LowerExtension( m_output ).IsNULLOrEmpty() );
    std::list< CConversionJob > jobs;
    UInt32 notConverted = 0;
    for ( size_t i=0; i<m_inputs.size(); ++i )
    {
        const CORE::CString input = CORE::RelativePath( m_inputs[ i ] );
        const bool isFolder = CORE::DirExists( input );
        CORE::CString::StringVector files;
        if ( isFolder )
            CollectFiles( input, m_recursive, files );
        else if ( CORE::FileExists( input ) )
            files.push_back( input );
        else
        {
            GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_CRITICAL, m_inputs[ i ] << ": there is no such file or folder" );
            ++notConverted;
            continue;
        }

        for ( size_t n=0; n<files.size(); ++n )
        {
            CConversionJob job;
            job.inputPath = files[ n ];
            job.fromType = m_fromType.IsNULLOrEmpty() ? LowerExtension( files[ n ] ) : m_fromType;
            job.toType = m_toType;
            job.overwrite = m_overwrite;
            job.settings = m_settings;

            if ( isFolder )
            {
                IMAGE::CImageCodecCapabilities::SImageCodecInfo source;
                if ( !IMAGE::CImageCodecCapabilities::TryGetCodec( job.fromType, source ) || !source.canDecode )
                    continue;
            }

            // where it goes: the output folder (with the structure of the input folder), or next to the input
            CORE::CString stem = CORE::StripFileExtention( files[ n ] );
            if ( !m_output.IsNULLOrEmpty() )
            {
                if ( outputIsFolder )
                {
                    CORE::CString relative = CORE::ExtractFilename( stem );
                    if ( m_mirror && isFolder && stem.Length() > input.Length() )
                    {
                        relative = stem.CutChars( input.Length(), true );
                        while ( relative.Length() > 0 && ( '/' == relative[ 0 ] || '\\' == relative[ 0 ] ) )
                            relative = relative.CutChars( 1, true );
                    }
                    stem = CORE::CombinePath( CORE::RelativePath( m_output ), relative );
                }
                else
                {
                    stem = CORE::StripFileExtention( CORE::RelativePath( m_output ) );
                }
            }
            job.outputPath = target.multipleFrames || !job.settings.IsAllFrames() ? stem + "." + m_toType : stem;
            if ( job.inputPath == job.outputPath && job.fromType == job.toType )
            {
                GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_CRITICAL, job.inputPath << ": the output is the input" );
                ++notConverted;
                continue;
            }
            jobs.push_back( job );
        }
    }

    if ( m_dryRun )
    {
        for ( std::list< CConversionJob >::const_iterator i=jobs.begin(); i!=jobs.end(); ++i )
            GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, i->inputPath << " (" << i->fromType << ") -> " << i->outputPath << " (" << i->toType << ")" );
        GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, (UInt32) jobs.size() << " files would be converted" );
        return notConverted > 0 ? ExitFailed : ExitOk;
    }

    SRunTotals totals;
    UInt32 threads = m_jobs > 0 ? (UInt32) m_jobs : CORE::GetLogicalCPUCount();
    if ( threads > 1 && jobs.size() > 1 )
        RunJobsOnPool( jobs, threads, totals, m_quiet );
    else
        RunJobsHere( jobs, totals, m_quiet );

    totals.failed += notConverted;
    GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_IMPORTANT, totals.converted << " converted (" << totals.files << " files written, " << totals.lossy << " with changes to fit the format), " <<
                       totals.skipped << " skipped, " << totals.failed << " failed" );
    return totals.failed > 0 ? ExitFailed : ExitOk;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGECONVERTOR */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
