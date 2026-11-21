/*
 *  ooz_extract: command line wrapper around the Kraken family decompressor of ooz
 *
 *  Copyright (C) 2026 Dinand Vanvelzen
 *
 *  This is a derived work: it calls the decompressor (kraken.cpp, bitknit.cpp, lzna.cpp)
 *  of ooz, Copyright (C) 2016, Powzix, which is licensed under the GNU General Public
 *  License version 3 or (at your option) any later version. This file is licensed
 *  under the same terms.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  ---------------------------------------------------------------------------
 *
 *  This program is a stand alone tool. It is started as a child process by
 *  other programs and exchanges plain bytes through files and standard
 *  input and output. It is never linked into another program.
 *
 *  What the decoder needs: the compressed bytes of one Kraken stream (the data of
 *  one block of 256 KiB or of a whole stream of any number of blocks) and the
 *  exact size of the data they decompress to. The stream does not carry that size,
 *  the caller has to know it. The compressed size must be exact as well: a stream
 *  that is shorter or longer than the compressed size given is rejected.
 *
 *  Usage
 *
 *    ooz_extract --in FILE [--offset N] [--size N] --decompressed-size N --out FILE|-
 *                [--max-output N] [--timeout SECONDS]
 *    ooz_extract --batch [--max-output N] [--timeout SECONDS]
 *
 *    --in FILE              file that holds the compressed bytes
 *    --offset N             byte offset of the compressed bytes in the file (default 0)
 *    --size N               number of compressed bytes (default: to the end of the file)
 *    --decompressed-size N  exact number of bytes the stream decompresses to
 *    --out FILE|-           file to write the result to, or - for standard output
 *                           (single request only)
 *    --max-output N         largest decompressed size accepted (default 1073741824 which
 *                           is 1 GiB, at most 2147418112); the compressed size may be at most
 *                           N + N / 16 + 65536 bytes
 *    --timeout SECONDS      limit per request, 0 for none (default 120); POSIX only
 *
 *  Batch mode reads one request per line from standard input, with the same
 *  parameters (--in, --offset, --size, --decompressed-size, --out, --max-output,
 *  --timeout; the last two default to the values on the command line). Values with
 *  spaces are put in double quotes, a backslash escapes a quote or backslash inside
 *  them. For every request a line is written to standard output and flushed:
 *
 *    ok <bytes>
 *    error <code> <message>
 *
 *  The program stays alive until standard input is closed, empty lines are skipped,
 *  and the exit code is 0 unless the program itself fails (2 for bad command line
 *  parameters). A failed request does not stop the program.
 *
 *  Exit codes of a single request:
 *
 *    0  ok
 *    2  bad usage: missing, unknown or invalid parameter, sizes of zero or over the maximum
 *    3  input/output error: file not found, not readable, output not writable
 *    4  corrupt or truncated input: the range lies (partly) outside the file, the stream
 *       is not a valid Kraken stream, is shorter or longer than the sizes given, or the
 *       decoder crashed or timed out on it
 *    5  output size mismatch: the decoder reported another size than the size requested
 *
 *  The decoder of ooz is not written for hostile input: on POSIX systems every request
 *  is decoded in a child process (fork) that has its own output buffer, so that a
 *  decoder that crashes on corrupt data (or runs on without end) is reported as exit
 *  code 4 and does not take this program down. On other systems the decoder runs in
 *  this process, over buffers that are padded on both sides. The decoder uses SSE2
 *  and runs on x86 and x86-64 CPUs.
 */

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
  #define OOZX_FSEEK _fseeki64
  #define OOZX_FTELL _ftelli64
  #if defined(_MSC_VER) && _MSC_VER < 1900
    #define snprintf _snprintf
  #endif
#else
  #include <unistd.h>
  #include <signal.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/mman.h>
  #include <sys/wait.h>
  #define OOZX_FSEEK fseeko
  #define OOZX_FTELL ftello
  #define OOZX_FORK_ISOLATION 1
  #if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
    #define MAP_ANONYMOUS MAP_ANON
  #endif
#endif

// The decoder of ooz (kraken.cpp)
int Kraken_Decompress( const unsigned char* src, size_t src_len, unsigned char* dst, size_t dst_len );

typedef unsigned long long U64;

namespace {

const int kExitOk = 0;
const int kExitUsage = 2;
const int kExitIo = 3;
const int kExitCorrupt = 4;
const int kExitSizeMismatch = 5;

const U64 kDefaultMaxOutput = 1073741824ULL;           // 1 GiB
const U64 kHardMaxOutput = 2147418112ULL;              // the decoder works with 32 bit signed offsets
const unsigned int kDefaultTimeout = 120;
const size_t kPadBefore = 4096;                        // zeroed room before and after every buffer, the
const size_t kPadAfter = 65536;                        // decoder reads and writes a little past its ends
const size_t kMaxLine = 65536;
const size_t kMaxTokens = 64;

struct SRequest
{
    const char* in;
    const char* out;
    U64 offset;
    U64 size;
    U64 decompressedSize;
    U64 maxOutput;
    unsigned int timeout;
    bool haveSize;
    bool haveDecompressedSize;

    SRequest( void )
        : in( NULL ), out( NULL ), offset( 0 ), size( 0 ), decompressedSize( 0 )
        , maxOutput( kDefaultMaxOutput ), timeout( kDefaultTimeout )
        , haveSize( false ), haveDecompressedSize( false )
    {
    }
};

void SetMessage( char* message, size_t messageSize, const char* format, const char* arg = NULL, U64 value = 0, U64 value2 = 0 )
{
    snprintf( message, messageSize, format, arg != NULL ? arg : "", value, value2 );
}

bool ParseNumber( const char* text, U64& value )
{
    if ( text == NULL || *text < '0' || *text > '9' )
        return false;
    errno = 0;
    char* end = NULL;
    unsigned long long v = strtoull( text, &end, 10 );
    if ( errno != 0 || end == text || *end != '\0' )
        return false;
    value = v;
    return true;
}

// Reads the parameters of one request from tokens, returns 0 or kExitUsage
int ParseRequest( char** tokens, int count, SRequest& request, char* message, size_t messageSize )
{
    for ( int i = 0; i < count; ++i )
    {
        const char* name = tokens[ i ];
        if ( i + 1 >= count )
        {
            SetMessage( message, messageSize, "parameter %s needs a value", name );
            return kExitUsage;
        }
        const char* value = tokens[ ++i ];
        U64 number = 0;
        if ( strcmp( name, "--in" ) == 0 )
            request.in = value;
        else if ( strcmp( name, "--out" ) == 0 )
            request.out = value;
        else if ( strcmp( name, "--offset" ) == 0 || strcmp( name, "--size" ) == 0 ||
                  strcmp( name, "--decompressed-size" ) == 0 || strcmp( name, "--max-output" ) == 0 ||
                  strcmp( name, "--timeout" ) == 0 )
        {
            if ( !ParseNumber( value, number ) )
            {
                SetMessage( message, messageSize, "value of %s is not a number", name );
                return kExitUsage;
            }
            if ( strcmp( name, "--offset" ) == 0 )
                request.offset = number;
            else if ( strcmp( name, "--size" ) == 0 )
            {
                request.size = number;
                request.haveSize = true;
            }
            else if ( strcmp( name, "--decompressed-size" ) == 0 )
            {
                request.decompressedSize = number;
                request.haveDecompressedSize = true;
            }
            else if ( strcmp( name, "--max-output" ) == 0 )
                request.maxOutput = number;
            else
            {
                if ( number > 86400 )
                {
                    SetMessage( message, messageSize, "value of %s is over 86400", name );
                    return kExitUsage;
                }
                request.timeout = (unsigned int) number;
            }
        }
        else
        {
            SetMessage( message, messageSize, "unknown parameter %s", name );
            return kExitUsage;
        }
    }
    return 0;
}

// Checks what can be checked without the file
int ValidateRequest( const SRequest& request, char* message, size_t messageSize )
{
    if ( request.in == NULL || *request.in == '\0' )
    {
        SetMessage( message, messageSize, "--in is required" );
        return kExitUsage;
    }
    if ( request.out == NULL || *request.out == '\0' )
    {
        SetMessage( message, messageSize, "--out is required" );
        return kExitUsage;
    }
    if ( !request.haveDecompressedSize )
    {
        SetMessage( message, messageSize, "--decompressed-size is required" );
        return kExitUsage;
    }
    if ( request.maxOutput == 0 || request.maxOutput > kHardMaxOutput )
    {
        SetMessage( message, messageSize, "--max-output must be 1 to %s%llu", "", kHardMaxOutput );
        return kExitUsage;
    }
    if ( request.decompressedSize == 0 )
    {
        SetMessage( message, messageSize, "--decompressed-size must be over 0" );
        return kExitUsage;
    }
    if ( request.decompressedSize > request.maxOutput )
    {
        SetMessage( message, messageSize, "decompressed size %s%llu is over --max-output %llu", "", request.decompressedSize, request.maxOutput );
        return kExitUsage;
    }
    if ( request.haveSize )
    {
        if ( request.size == 0 )
        {
            SetMessage( message, messageSize, "--size must be over 0" );
            return kExitUsage;
        }
        if ( request.size > request.maxOutput + ( request.maxOutput >> 4 ) + 65536ULL )
        {
            SetMessage( message, messageSize, "compressed size %s%llu is over the limit derived from --max-output %llu", "", request.size, request.maxOutput );
            return kExitUsage;
        }
    }
    return 0;
}

struct SFile
{
    FILE* f;
    SFile( void ) : f( NULL ) {}
    ~SFile( void ) { if ( f != NULL ) fclose( f ); }
};

// Allocates size + padding zeroed bytes, returns the pointer to the usable part
unsigned char* AllocPadded( size_t size, unsigned char** base )
{
    unsigned char* p = (unsigned char*) calloc( 1, size + kPadBefore + kPadAfter );
    *base = p;
    return p != NULL ? p + kPadBefore : NULL;
}

int WriteOutput( const char* path, const unsigned char* data, size_t size, char* message, size_t messageSize )
{
    bool toStdout = strcmp( path, "-" ) == 0;
    FILE* f = stdout;
    if ( !toStdout )
    {
        f = fopen( path, "wb" );
        if ( f == NULL )
        {
            SetMessage( message, messageSize, "cannot open output file %s for writing", path );
            return kExitIo;
        }
    }
    else
    {
        #ifdef _WIN32
        _setmode( _fileno( stdout ), _O_BINARY );
        #endif
    }
    bool ok = fwrite( data, 1, size, f ) == size;
    if ( toStdout )
    {
        ok = ( fflush( f ) == 0 ) && ok;
    }
    else
    {
        ok = ( fclose( f ) == 0 ) && ok;
        if ( !ok )
            remove( path );
    }
    if ( !ok )
    {
        SetMessage( message, messageSize, "cannot write the output" );
        return kExitIo;
    }
    return 0;
}

// Decodes inputSize bytes at in (padded buffer) into out (padded buffer of decompressedSize bytes). Returns
// the result of Kraken_Decompress, or a negative value (and sets crashed) when the decoder did not return normally.
#ifdef OOZX_FORK_ISOLATION

int DecodeIsolated( const unsigned char* in, size_t inputSize, size_t decompressedSize, unsigned int timeout, unsigned char** outResult, long long& result, char* message, size_t messageSize )
{
    // shared mapping: a control page, then the padded output buffer
    const size_t control = 4096;
    size_t total = control + kPadBefore + decompressedSize + kPadAfter;
    void* mapping = mmap( NULL, total, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0 );
    if ( mapping == MAP_FAILED )
    {
        SetMessage( message, messageSize, "cannot allocate %s%llu bytes of output memory", "", (U64) total );
        return kExitIo;
    }
    unsigned char* base = (unsigned char*) mapping;
    long long* resultSlot = (long long*) base;
    *resultSlot = -2;
    unsigned char* out = base + control + kPadBefore;

    fflush( stdout );
    fflush( stderr );
    pid_t pid = fork();
    if ( pid < 0 )
    {
        munmap( mapping, total );
        SetMessage( message, messageSize, "cannot start the decoder process" );
        return kExitIo;
    }
    if ( pid == 0 )
    {
        if ( timeout > 0 )
            alarm( timeout );
        *resultSlot = Kraken_Decompress( in, inputSize, out, decompressedSize );
        _exit( 0 );
    }
    int status = 0;
    pid_t waited;
    do
    {
        waited = waitpid( pid, &status, 0 );
    }
    while ( waited < 0 && errno == EINTR );

    int code = 0;
    if ( waited < 0 )
    {
        SetMessage( message, messageSize, "waiting for the decoder process failed" );
        code = kExitIo;
    }
    else if ( WIFSIGNALED( status ) )
    {
        if ( WTERMSIG( status ) == SIGALRM )
            SetMessage( message, messageSize, "decoder timed out after %s%llu seconds", "", (U64) timeout );
        else
            SetMessage( message, messageSize, "decoder crashed on the input (signal %s%llu), the input is corrupt", "", (U64) WTERMSIG( status ) );
        code = kExitCorrupt;
    }
    else if ( !WIFEXITED( status ) || WEXITSTATUS( status ) != 0 || *resultSlot == -2 )
    {
        SetMessage( message, messageSize, "decoder process failed" );
        code = kExitCorrupt;
    }
    else
    {
        result = *resultSlot;
    }
    if ( code != 0 )
    {
        munmap( mapping, total );
        return code;
    }
    // hand the mapping over: the caller releases it with ReleaseIsolated
    *outResult = out;
    return 0;
}

#endif

// Runs one request. On success outBytes holds the size written.
int RunRequest( const SRequest& request, U64& outBytes, char* message, size_t messageSize )
{
    int code = ValidateRequest( request, message, messageSize );
    if ( code != 0 )
        return code;

    // Open the input and find the range
    SFile file;
    file.f = fopen( request.in, "rb" );
    if ( file.f == NULL )
    {
        SetMessage( message, messageSize, "cannot open input file %s", request.in );
        return kExitIo;
    }
    #ifdef OOZX_FORK_ISOLATION
    {
        struct stat st;
        if ( fstat( fileno( file.f ) , &st ) != 0 || !S_ISREG( st.st_mode ) )
        {
            SetMessage( message, messageSize, "input %s is not a regular file", request.in );
            return kExitIo;
        }
    }
    #endif
    if ( OOZX_FSEEK( file.f, 0, SEEK_END ) != 0 )
    {
        SetMessage( message, messageSize, "cannot determine the size of input file %s", request.in );
        return kExitIo;
    }
    long long end = (long long) OOZX_FTELL( file.f );
    if ( end < 0 )
    {
        SetMessage( message, messageSize, "cannot determine the size of input file %s", request.in );
        return kExitIo;
    }
    U64 fileSize = (U64) end;
    if ( request.offset > fileSize )
    {
        SetMessage( message, messageSize, "offset %s%llu is behind the end of the input file (%llu bytes), the input is truncated", "", request.offset, fileSize );
        return kExitCorrupt;
    }
    U64 size = request.haveSize ? request.size : fileSize - request.offset;
    if ( size == 0 )
    {
        SetMessage( message, messageSize, "the compressed range is empty" );
        return kExitCorrupt;
    }
    if ( size > fileSize - request.offset )
    {
        SetMessage( message, messageSize, "range %s%llu + %llu is behind the end of the input file, the input is truncated", "", request.offset, size );
        return kExitCorrupt;
    }
    if ( size > request.maxOutput + ( request.maxOutput >> 4 ) + 65536ULL )
    {
        SetMessage( message, messageSize, "compressed size %s%llu is over the limit derived from --max-output %llu", "", size, request.maxOutput );
        return kExitUsage;
    }

    // Read exactly the range, nothing outside it
    unsigned char* inBase = NULL;
    unsigned char* in = AllocPadded( (size_t) size, &inBase );
    if ( in == NULL )
    {
        SetMessage( message, messageSize, "out of memory for %s%llu input bytes", "", size );
        return kExitIo;
    }
    if ( OOZX_FSEEK( file.f, (long long) request.offset, SEEK_SET ) != 0 || fread( in, 1, (size_t) size, file.f ) != (size_t) size )
    {
        free( inBase );
        SetMessage( message, messageSize, "cannot read the range of input file %s", request.in );
        return kExitIo;
    }

    const size_t decompressedSize = (size_t) request.decompressedSize;
    long long result = -1;
    unsigned char* out = NULL;

    #ifdef OOZX_FORK_ISOLATION

    code = DecodeIsolated( in, (size_t) size, decompressedSize, request.timeout, &out, result, message, messageSize );
    free( inBase );
    if ( code != 0 )
        return code;
    // out lies in the mapping, which is unmapped below
    const size_t mappingSize = 4096 + kPadBefore + decompressedSize + kPadAfter;
    unsigned char* mappingBase = out - kPadBefore - 4096;

    #else

    unsigned char* outBase = NULL;
    out = AllocPadded( decompressedSize, &outBase );
    if ( out == NULL )
    {
        free( inBase );
        SetMessage( message, messageSize, "out of memory for %s%llu output bytes", "", request.decompressedSize );
        return kExitIo;
    }
    result = Kraken_Decompress( in, (size_t) size, out, decompressedSize );
    free( inBase );

    #endif

    if ( result < 0 )
    {
        code = kExitCorrupt;
        SetMessage( message, messageSize, "the input is not a valid stream of %s%llu bytes, it is corrupt or truncated", "", request.decompressedSize );
    }
    else if ( (U64) result != request.decompressedSize )
    {
        code = kExitSizeMismatch;
        SetMessage( message, messageSize, "decoder produced %s%llu bytes, %llu requested", "", (U64) result, request.decompressedSize );
    }
    else
    {
        code = WriteOutput( request.out, out, decompressedSize, message, messageSize );
        if ( code == 0 )
            outBytes = (U64) decompressedSize;
    }

    #ifdef OOZX_FORK_ISOLATION
    munmap( mappingBase, mappingSize );
    #else
    free( outBase );
    #endif
    return code;
}

// Splits a line in place into tokens; double quotes group, a backslash escapes inside quotes. Returns the count or -1.
int Tokenize( char* line, char** tokens, size_t maxTokens )
{
    int count = 0;
    char* p = line;
    for ( ;; )
    {
        while ( *p == ' ' || *p == '\t' )
            ++p;
        if ( *p == '\0' )
            break;
        if ( (size_t) count >= maxTokens )
            return -1;
        tokens[ count++ ] = p;
        char* w = p;
        bool quoted = false;
        while ( *p != '\0' && ( quoted || ( *p != ' ' && *p != '\t' ) ) )
        {
            if ( *p == '"' )
            {
                quoted = !quoted;
                ++p;
            }
            else if ( quoted && *p == '\\' && ( p[ 1 ] == '"' || p[ 1 ] == '\\' ) )
            {
                *w++ = p[ 1 ];
                p += 2;
            }
            else
                *w++ = *p++;
        }
        if ( quoted )
            return -1;
        bool atEnd = *p == '\0';
        *w = '\0';
        if ( !atEnd )
            ++p;
    }
    return count;
}

bool WriteStatus( int code, U64 bytes, const char* message )
{
    int rc;
    if ( code == 0 )
        rc = fprintf( stdout, "ok %llu\n", bytes );
    else
        rc = fprintf( stdout, "error %d %s\n", code, message );
    return rc >= 0 && fflush( stdout ) == 0;
}

int RunBatch( const SRequest& defaults )
{
    char* line = (char*) malloc( kMaxLine + 2 );
    if ( line == NULL )
        return kExitIo;
    char* tokens[ kMaxTokens ];
    bool discarding = false;
    int exitCode = 0;
    while ( fgets( line, (int) kMaxLine + 1, stdin ) != NULL )
    {
        size_t len = strlen( line );
        bool complete = len > 0 && line[ len - 1 ] == '\n';
        if ( discarding )
        {
            discarding = !complete;
            continue;
        }
        if ( !complete && len >= kMaxLine )
        {
            discarding = true;
            if ( !WriteStatus( kExitUsage, 0, "request line is too long" ) )
            {
                exitCode = kExitIo;
                break;
            }
            continue;
        }
        while ( len > 0 && ( line[ len - 1 ] == '\n' || line[ len - 1 ] == '\r' ) )
            line[ --len ] = '\0';

        char message[ 512 ];
        message[ 0 ] = '\0';
        U64 bytes = 0;
        int count = Tokenize( line, tokens, kMaxTokens );
        if ( count == 0 )
            continue;
        int code;
        if ( count < 0 )
        {
            code = kExitUsage;
            SetMessage( message, sizeof( message ), "request line has an open quote or too many parameters" );
        }
        else
        {
            SRequest request = defaults;
            code = ParseRequest( tokens, count, request, message, sizeof( message ) );
            if ( code == 0 && request.out != NULL && strcmp( request.out, "-" ) == 0 )
            {
                code = kExitUsage;
                SetMessage( message, sizeof( message ), "--out - is not possible in batch mode, standard output carries the status lines" );
            }
            if ( code == 0 )
                code = RunRequest( request, bytes, message, sizeof( message ) );
        }
        if ( !WriteStatus( code, bytes, message ) )
        {
            exitCode = kExitIo;
            break;
        }
    }
    free( line );
    return exitCode;
}

void PrintUsage( FILE* f )
{
    fprintf( f,
        "ooz_extract: decompress Kraken family streams (Kraken, Mermaid, Selkie, Leviathan)\n"
        "\n"
        "  ooz_extract --in FILE [--offset N] [--size N] --decompressed-size N --out FILE|-\n"
        "              [--max-output N] [--timeout SECONDS]\n"
        "  ooz_extract --batch [--max-output N] [--timeout SECONDS]\n"
        "\n"
        "  --in FILE              file with the compressed bytes\n"
        "  --offset N             offset of the compressed bytes in the file (default 0)\n"
        "  --size N               number of compressed bytes (default: to the end of the file)\n"
        "  --decompressed-size N  exact size of the decompressed data (required)\n"
        "  --out FILE|-           output file, or - for standard output (not in batch mode)\n"
        "  --max-output N         largest decompressed size accepted (default 1073741824)\n"
        "  --timeout SECONDS      limit per request, 0 for none (default 120)\n"
        "  --batch                read requests (same parameters) from standard input, one per line,\n"
        "                         answer 'ok <bytes>' or 'error <code> <message>' per line\n"
        "\n"
        "exit codes: 0 ok, 2 bad usage, 3 io error, 4 corrupt or truncated input, 5 output size mismatch\n" );
}

} // namespace

int main( int argc, char* argv[] )
{
    #ifdef SIGPIPE
    signal( SIGPIPE, SIG_IGN );
    #endif

    bool batch = false;
    bool help = false;
    char* params[ 64 ];
    int paramCount = 0;
    for ( int i = 1; i < argc; ++i )
    {
        if ( strcmp( argv[ i ], "--batch" ) == 0 )
            batch = true;
        else if ( strcmp( argv[ i ], "--help" ) == 0 || strcmp( argv[ i ], "-h" ) == 0 )
            help = true;
        else if ( paramCount < 64 )
            params[ paramCount++ ] = argv[ i ];
        else
        {
            fprintf( stderr, "error %d too many parameters\n", kExitUsage );
            return kExitUsage;
        }
    }
    if ( help )
    {
        PrintUsage( stdout );
        return 0;
    }
    if ( argc < 2 )
    {
        PrintUsage( stderr );
        fprintf( stderr, "error %d no parameters\n", kExitUsage );
        return kExitUsage;
    }

    char message[ 512 ];
    message[ 0 ] = '\0';
    SRequest request;
    int code = ParseRequest( params, paramCount, request, message, sizeof( message ) );
    if ( code == 0 && batch )
    {
        if ( request.in != NULL || request.out != NULL || request.offset != 0 || request.haveSize || request.haveDecompressedSize )
        {
            code = kExitUsage;
            SetMessage( message, sizeof( message ), "in batch mode only --max-output and --timeout are allowed on the command line" );
        }
        else if ( request.maxOutput == 0 || request.maxOutput > kHardMaxOutput )
        {
            code = kExitUsage;
            SetMessage( message, sizeof( message ), "--max-output must be 1 to %s%llu", "", kHardMaxOutput );
        }
    }
    if ( code != 0 )
    {
        fprintf( stderr, "error %d %s\n", code, message );
        return code;
    }
    if ( batch )
        return RunBatch( request );

    U64 bytes = 0;
    code = RunRequest( request, bytes, message, sizeof( message ) );
    if ( code != 0 )
    {
        fprintf( stderr, "error %d %s\n", code, message );
        return code;
    }
    if ( request.out != NULL && strcmp( request.out, "-" ) != 0 )
    {
        fprintf( stdout, "ok %llu\n", bytes );
        fflush( stdout );
    }
    return kExitOk;
}
