/*
 *  dstorepluginPARSIFALXML: Plugin adding the ability to use Parsifal XML
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include <stdlib.h>     /* memory management */
#include <stdio.h>      /* standard I/O utils */
#include <string.h>     /* standard string utils */

#ifndef PARSIFAL__H
#include "parsifal.h"   /* Parsifal XML Parser API */
#define PARSIFAL__H
#endif /* PARSIFAL__H ? */

/* Parsifal defines BYTE in the global namesapce which can cause problems :( */
#undef BYTE

#include "DLLMainDSTOREpluginPARSIFALXML.h"    /* gucefCORE DSTORE codec plugin API */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* gucefCORE macros, used here for the export and callspec macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define NODE_STORE_INDENT       1
#define INDENT_CHAR             9  /* horizontal tab */

#define VERSION_MAJOR_FIELD     1
#define VERSION_MINOR_FIELD     1
#define VERSION_PATCH_FIELD     0
#define VERSION_RELEASE_FIELD   0

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #define strdup _strdup
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SDestFileData
{
    TIOAccess* fptr;
    Int32 indent;
    char* line;
    UInt32 linelen;

    char* buffer;
    UInt32 bufferSize;
};

typedef struct SDestFileData TDestFileData;

/*---------------------------------------------------------------------------*/

struct SSrcFileData
{
    TIOAccess* access;
    LPXMLPARSER parser;
    TReadHandlers handlers;
    void* privdata;
    XMLSTRINGBUF textBuffer;
};

typedef struct SSrcFileData TSrcFileData;

/*---------------------------------------------------------------------------*/

struct SEscapeEntry
{
    char escapeChar;
    const char* replacementStr;
    char replacementStrLen;
};

typedef struct SEscapeEntry TEscapeEntry;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static int
ParsifalReader( unsigned char* buf ,
                int cbytes         ,
                int *cbytesactual  ,
                void* inputdata    )
{
        *cbytesactual = (int) ((TIOAccess*)inputdata)->read( ((TIOAccess*)inputdata), buf, 1, cbytes );
        return (*cbytesactual < cbytes);
}

/*---------------------------------------------------------------------------*/

static void
ParsifalError( struct tagXMLPARSER *parser )
{
        TSrcFileData* sd = (TSrcFileData*) parser->UserData;
        (*sd->handlers.OnError)( sd->privdata, parser->ErrorCode, parser->ErrorString );
}

/*---------------------------------------------------------------------------*/

static int
ParsifalDocumentStart( void* userdata )
{
        TSrcFileData* sd = (TSrcFileData*) userdata;
        (*sd->handlers.OnTreeBegin)( sd->privdata );
        return XML_OK;
}

/*---------------------------------------------------------------------------*/

static int
ParsifalDocumentEnd( void* userdata )
{
        TSrcFileData* sd = (TSrcFileData*) userdata;
        (*sd->handlers.OnTreeEnd)( sd->privdata );
        return XML_OK;
}

/*---------------------------------------------------------------------------*/

static int
ParsifalCharacters( void *userdata     ,
                    const XMLCH *chars ,
                    int cbChars        )
{
    TSrcFileData* sd = (TSrcFileData*) userdata;
	XMLStringbuf_Append( &sd->textBuffer, (XMLCH*)chars, cbChars );
	return XML_OK;
}
/*---------------------------------------------------------------------------*/

static int
ParsifalElementStart( void* userdata         ,
                      const XMLCH *uri       ,
                      const XMLCH *localname ,
                      const XMLCH *qname     ,
                      LPXMLVECTOR atts       )
{
    Int32 i;
    LPXMLRUNTIMEATT att;

    TSrcFileData* sd = (TSrcFileData*) userdata;

    (*sd->handlers.OnNodeBegin)( sd->privdata, qname, GUCEF_DATATYPE_OBJECT );
    if ( atts->length )
    {
        for ( i=0; i<atts->length; ++i )
        {
            att = (LPXMLRUNTIMEATT) XMLVector_Get( atts, i );
            (*sd->handlers.OnNodeAtt)( sd->privdata           ,
                                        qname                 ,
                                        att->qname            ,
                                        att->value            ,
                                        GUCEF_DATATYPE_STRING );
        }
    }
    return XML_OK;
}

/*---------------------------------------------------------------------------*/

void
GuaranteeMinBufferSize( char** destBuffer        ,
                        UInt32* destBufferLength ,
                        UInt32 desiredSize       )
{
    if ( NULL != destBuffer && NULL != destBufferLength )
    {
        if ( NULL != *destBuffer )
        {
            if ( desiredSize > *destBufferLength )
            {
                *destBuffer = realloc( *destBuffer, desiredSize );
                *destBufferLength = desiredSize;
            }
        }
        else
        {
            *destBuffer = malloc( desiredSize );
            *destBufferLength = desiredSize;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
HandleEscapeCharacterSet( const char* srcStr         ,
                          UInt32 srcStrLen           ,
                          char** destBuffer          ,
                          UInt32* destBufferLength   ,
                          TEscapeEntry* escapeEntry  ,
                          UInt32 escapeEntryCount    ,
                          UInt32* strLen             )
{
    UInt32 n = 0;
    UInt32 i = 0;
    UInt32 replCount = 0;
    UInt32 replSize = 0;
    char* bufferPtr = NULL;

    for ( i=0; i<srcStrLen; ++i )
    {
        for ( n=0; n<escapeEntryCount; ++n )
        {
            if ( srcStr[ i ] == escapeEntry[ n ].escapeChar )
            {
                replCount++;
                replSize += escapeEntry[ n ].replacementStrLen;
                break;
            }
        }
    }

    *strLen = srcStrLen-replCount+replSize;
    GuaranteeMinBufferSize( destBuffer, destBufferLength, (*strLen)+1 );
    bufferPtr = *destBuffer;
    memset( bufferPtr, 0, (*strLen)+1 );

    for ( i=0; i<srcStrLen; ++i )
    {
        char foundEscChar = 0;
        for ( n=0; n<escapeEntryCount; ++n )
        {
            if ( srcStr[ i ] == escapeEntry[ n ].escapeChar )
            {
                foundEscChar = 1;
                memcpy( bufferPtr, escapeEntry[ n ].replacementStr, escapeEntry[ n ].replacementStrLen );
                bufferPtr += escapeEntry[ n ].replacementStrLen;
                break;
            }
        }

        if ( 0 == foundEscChar )
        {
            *bufferPtr = srcStr[ i ];
            ++bufferPtr;
        }
    }
}

/*---------------------------------------------------------------------------*/

void
PrintNumericVariant( const TVariantData* src  ,
                     char* destBuffer         ,
                     UInt32 destBufferLength  ,
                     UInt32* strLen           ,
                     Int32* wasNumeric        )
{
    if ( GUCEF_NULL != src )
    {
        switch ( src->containedType )
        {
            /* at init we already make sue the line buffer is large enough for numeric values
             * as such no need to worry about that here
             */
            case GUCEF_DATATYPE_INT8:    { snprintf( destBuffer, destBufferLength, "%d", (Int32) src->union_data.int8_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_UINT8:   { snprintf( destBuffer, destBufferLength, "%u", (UInt32) src->union_data.uint8_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_INT16:   { snprintf( destBuffer, destBufferLength, "%d", (Int32) src->union_data.int16_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_UINT16:  { snprintf( destBuffer, destBufferLength, "%u", (UInt32) src->union_data.uint16_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_INT32:   { snprintf( destBuffer, destBufferLength, "%d", src->union_data.int32_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_UINT32:  { snprintf( destBuffer, destBufferLength, "%u", src->union_data.uint32_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            #ifdef GUCEF_MSWIN_BUILD
            case GUCEF_DATATYPE_INT64:   { snprintf( destBuffer, destBufferLength, "%I64d", src->union_data.int64_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_UINT64:  { snprintf( destBuffer, destBufferLength, "%I64u", src->union_data.uint64_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            #else
            case GUCEF_DATATYPE_INT64:   { snprintf( destBuffer, destBufferLength, "%lld", src->union_data.int64_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_UINT64:  { snprintf( destBuffer, destBufferLength, "%llu", src->union_data.uint64_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            #endif
            case GUCEF_DATATYPE_FLOAT32: { snprintf( destBuffer, destBufferLength, "%f", src->union_data.float32_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
            case GUCEF_DATATYPE_FLOAT64: { snprintf( destBuffer, destBufferLength, "%lf", src->union_data.float64_data ); *strLen = (UInt32) strlen( destBuffer ); *wasNumeric = 1; break; }
        
            default:
            {
                *destBuffer = '\0';
                *strLen = 0;
                *wasNumeric = 0;
                break;
            }
        }
    }
    else
    {
        *destBuffer = '\0';
        *strLen = 0;
        *wasNumeric = 0;
    }
}

/*---------------------------------------------------------------------------*/

void
HandleEscapeCharacters( const char* srcStr       ,
                        char** destBuffer        ,
                        UInt32* destBufferLength ,
                        UInt32* strLen           )
{
    if ( GUCEF_NULL != srcStr )
    {
        TEscapeEntry entries[ 6 ];
        UInt32 srcStrLen = (UInt32) strlen( srcStr );

        entries[ 0 ].escapeChar = '<';
        entries[ 0 ].replacementStr = "&lt;";
        entries[ 0 ].replacementStrLen = 4;
        entries[ 1 ].escapeChar = '>';
        entries[ 1 ].replacementStr = "&gt;";
        entries[ 1 ].replacementStrLen = 4;
        entries[ 2 ].escapeChar = '&';
        entries[ 2 ].replacementStr = "&amp;";
        entries[ 2 ].replacementStrLen = 5;
        entries[ 3 ].escapeChar = '\'';
        entries[ 3 ].replacementStr = "&apos;";
        entries[ 3 ].replacementStrLen = 6;
        entries[ 4 ].escapeChar = '\"';
        entries[ 4 ].replacementStr = "&quot;";
        entries[ 4 ].replacementStrLen = 6;
        entries[ 5 ].escapeChar = '�';
        entries[ 5 ].replacementStr = "&#187;";
        entries[ 5 ].replacementStrLen = 6;

        HandleEscapeCharacterSet( srcStr           ,
                                  srcStrLen        ,
                                  destBuffer       ,
                                  destBufferLength ,
                                  entries          ,
                                  6                ,
                                  strLen           );
        return;
    }

    /* default: replace with empty string */
    *strLen = 0;
    if ( *destBufferLength > 0 )
    {
        memset( *destBuffer, 0, *destBufferLength );
    }
}

/*---------------------------------------------------------------------------*/

void
HandleEscapeCharactersInVariant( const TVariantData* src  ,
                                 char** destBuffer        ,
                                 UInt32* destBufferLength ,
                                 UInt32* strLen           )
{
    if ( GUCEF_NULL != src )
    {
        Int32 isNumeric = 0;
        PrintNumericVariant( src, *destBuffer, *destBufferLength, strLen, &isNumeric );

        if ( 0 == isNumeric )
        {
            *strLen = src->union_data.heap_data.heap_data_size;
            const char* srcStr = src->union_data.heap_data.union_data.char_heap_data;
            HandleEscapeCharacters( srcStr, destBuffer, destBufferLength, strLen );
            return;
        }
    }

    /* default: replace with empty string */
    *strLen = 0;
    if ( *destBufferLength > 0 )
    {
        memset( *destBuffer, 0, *destBufferLength );
    }
}

/*---------------------------------------------------------------------------*/

static int
ParsifalElementEnd( void* userdata         ,
                    const XMLCH *uri       ,
                    const XMLCH *localname ,
                    const XMLCH *qname     )
{
    TSrcFileData* sd = (TSrcFileData*) userdata;

    char* text = strdup( (const char*) XMLStringbuf_ToString( &sd->textBuffer ) );
    if ( 0 != text )
    {
        /* normalize buffer, note that XMLNormalizeBuf doesn't nul terminate the buffer: */
		int len = XMLNormalizeBuf( text, sd->textBuffer.len );
		if ( len < sd->textBuffer.len ) text[ len ] = '\0';

        if ( len > 0 )
        {
            (*sd->handlers.OnNodeAtt)( sd->privdata          ,
                                       qname                 ,
                                       0                     ,
                                       text                  ,
                                       GUCEF_DATATYPE_STRING );
        }

        /* we'll reuse Stringbuf just setting its length to 0: */
		XMLStringbuf_SetLength( &sd->textBuffer, 0 );
    }

    (*sd->handlers.OnNodeEnd)( sd->privdata, qname );
    return XML_OK;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Init( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        /* parsifal does not require global initialization */
        *plugdata = NULL;
        return 1;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Shutdown( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        /* parsifal does not require global cleanup */
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Dest_File_Open( void** plugdata    ,
                           void** filedata    ,
                           TIOAccess* outFile ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        *filedata = NULL;
        if ( outFile != NULL )
        {
                char outBuffer[ 275 ];
                TDestFileData* fd = (TDestFileData*) malloc( sizeof( TDestFileData ) );
                fd->fptr = outFile;
                fd->indent = 0;
                fd->line = NULL;
                fd->linelen = 0;
                fd->buffer = NULL;
                fd->bufferSize = 0;
                *filedata = fd;

                /* start with a buffer large enough to hold 64 bit numbers when printed */
                GuaranteeMinBufferSize( &fd->line, &fd->linelen, 21 );
                
                sprintf( outBuffer, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\r\n" );
                outFile->write( outFile, outBuffer, (UInt32)strlen( outBuffer ), 1 );
                sprintf( outBuffer, "<!-- Generated using dstorepluginXMLPARSIFAL (v%d.%d.%d.%d) for the GUCEF platform - Copyright (C) Dinand Vanvelzen. LPGLv3 -->\r\n", VERSION_MAJOR_FIELD, VERSION_MINOR_FIELD, VERSION_PATCH_FIELD, VERSION_RELEASE_FIELD );
                outFile->write( outFile, outBuffer, (UInt32)strlen( outBuffer ), 1 );
                return 1;
        }
        return 0;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Dest_File_Close( void** plugdata ,
                            void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( filedata )
    {
        if ( *filedata )
        {
            TDestFileData* fd = (TDestFileData*)*filedata;
            fd->fptr->close( fd->fptr );
            if ( fd->line && fd->linelen )
            {
                free( fd->line );
                fd->line = NULL;
            }
            if ( fd->buffer && fd->bufferSize )
            {
                free( fd->buffer );
                fd->buffer = NULL;
            }
            free( fd );
        }
    }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Begin_Node_Store( void** plugdata      ,
                             void** filedata      ,
                             const char* nodename ,
                             Int32 nodeType       ,
                             UInt32 attscount     ,
                             UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TDestFileData* fd = (TDestFileData*)*filedata;
    UInt32 max, len, strLen=0;
    char* escNodeName = NULL;

    if ( !fd ) return;

    if ( !nodename )
    {
        nodename = "noname";
    }

    HandleEscapeCharacters( nodename, &fd->buffer, &fd->bufferSize, &strLen );
    escNodeName = (char*) malloc( strLen+1 );
    memcpy( escNodeName, fd->buffer, strLen+1 );

    len = (UInt32)strlen( escNodeName );
    max = len + 5*attscount + 6 + fd->indent;
    if ( fd->linelen < max )
    {
        char* newLine = realloc( fd->line, max );
        if ( GUCEF_NULL != newLine )
        {
            fd->line = newLine;
            *(fd->line) = 0;
            fd->linelen = max;
        }
    }
    memset( fd->line, INDENT_CHAR, fd->indent );
    if ( attscount > 0 )
    {
        sprintf( fd->line+fd->indent, "<%s ", escNodeName );
    }
    else
    {
        haschildren ? sprintf( fd->line+fd->indent, "<%s>\r\n", escNodeName ) : sprintf( fd->line+fd->indent, "<%s/>\r\n", escNodeName );
        fd->fptr->write( fd->fptr, fd->line, (UInt32)strlen( fd->line ), 1 );
    }

    free( escNodeName );
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Store( void** plugdata      ,
                           void** filedata      ,
                           const char* nodename ,
                           UInt32 attscount     ,
                           UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( !nodename )
    {
        nodename = "noname";
    }
    if ( haschildren )
    {
        TDestFileData* fd = (TDestFileData*)*filedata;
        UInt32 strLen = 0;
        char* escNodeName = NULL;

        HandleEscapeCharacters( nodename, &fd->buffer, &fd->bufferSize, &strLen );
        escNodeName = (char*) malloc( strLen+1 );
        memcpy( escNodeName, fd->buffer, strLen+1 );

        memset( fd->line, INDENT_CHAR, fd->indent );
        sprintf( fd->line+fd->indent, "</%s>\r\n", escNodeName );
        fd->fptr->write( fd->fptr, fd->line, (UInt32)strlen( fd->line ), 1 );

        free( escNodeName );
    }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Store_Node_Att( void** plugdata              ,
                           void** filedata              ,
                           const char* nodename         ,
                           UInt32 attscount             ,
                           UInt32 attindex              ,
                           const char* attname          ,
                           const TVariantData* attvalue ,
                           UInt32 haschildren           ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TDestFileData* fd = (TDestFileData*)*filedata;
    UInt32 linelen, len;
    UInt32 strLen = 0;
    char* escAttName = NULL;
    char* escAttValue = NULL;

    if ( GUCEF_NULL == attname || 0 == attscount ) 
        return;

    if ( GUCEF_NULL == attvalue )
    {
        static TVariantData dummy;
        memset( &dummy, 0, sizeof dummy );
        attvalue = &dummy;
    }
    if ( GUCEF_NULL == nodename )
    {
        nodename = "noname";
    }

    HandleEscapeCharacters( attname, &fd->buffer, &fd->bufferSize, &strLen );
    escAttName = (char*) malloc( strLen+1 );
    memcpy( escAttName, fd->buffer, strLen+1 );
    HandleEscapeCharactersInVariant( attvalue, &fd->buffer, &fd->bufferSize, &strLen );
    escAttValue = (char*) malloc( strLen+1 );
    memcpy( escAttValue, fd->buffer, strLen+1 );

    linelen = (UInt32)strlen( fd->line );
    len = (UInt32)strlen( escAttName )+(UInt32)strlen( escAttValue )+2;
    if ( fd->linelen < linelen+10+len )
    {
        fd->line = realloc( fd->line, linelen+11+len );
    }
    if ( attindex+1 < attscount )
    {
        sprintf( fd->line+linelen, "%s=\"%s\" ", escAttName, escAttValue );
        return;
    }
    if ( haschildren )
    {
        sprintf( fd->line+linelen, "%s=\"%s\" >\r\n", escAttName, escAttValue );
    }
    else
    {
        sprintf( fd->line+linelen, "%s=\"%s\" />\r\n", escAttName, escAttValue );
    }
    fd->fptr->write( fd->fptr, fd->line, (UInt32)strlen( fd->line ), 1 );

    free( escAttName );
    free( escAttValue );
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Begin_Node_Children( void** plugdata      ,
                                void** filedata      ,
                                const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        TDestFileData* fd = (TDestFileData*)*filedata;
        fd->indent += NODE_STORE_INDENT;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Children( void** plugdata      ,
                              void** filedata      ,
                              const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        TDestFileData* fd = (TDestFileData*)*filedata;
        fd->indent -= NODE_STORE_INDENT;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Src_File_Open( void** plugdata      ,
                          void** filedata      ,
                          TIOAccess* file      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TSrcFileData* sd;
    *plugdata = NULL;

    sd = (TSrcFileData*) malloc( sizeof(TSrcFileData) );
    sd->access = file;

    if ( XMLParser_Create( &sd->parser ) )
    {
        sd->privdata = NULL;
        memset( &sd->handlers, 0, sizeof(TReadHandlers) );

        /* init Stringbuf: blockSize 256, no pre-allocation: */
        XMLStringbuf_Init( &sd->textBuffer, 256, 0 );

        sd->parser->startDocumentHandler = &ParsifalDocumentStart;
        sd->parser->endDocumentHandler = &ParsifalDocumentEnd;
        sd->parser->startElementHandler = &ParsifalElementStart;
        sd->parser->endElementHandler = &ParsifalElementEnd;
        sd->parser->charactersHandler = &ParsifalCharacters;
        sd->parser->errorHandler = &ParsifalError;
        sd->parser->UserData = sd;
        *filedata = sd;
        return 1;
    }
    free( sd );
    return 0;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Src_File_Close( void** plugdata ,
                           void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        if ( *filedata )
        {
                TSrcFileData* sd = (TSrcFileData*) *filedata;
                XMLParser_Free( sd->parser );
                sd->access->close( sd->access );
        }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Set_Read_Handlers( void** plugdata                ,
                              void** filedata                ,
                              const TReadHandlers* rhandlers ,
                              void* privdata                 ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        if ( *filedata )
        {
                TSrcFileData* sd = (TSrcFileData*) *filedata;
                sd->handlers = *rhandlers;
                sd->privdata = privdata;
        }
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Start_Reading( void** plugdata ,
                          void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        if ( *filedata )
        {
                TSrcFileData* sd = (TSrcFileData*) *filedata;
                if ( sd )
                {
                        XMLParser_Parse( sd->parser      ,
                                         &ParsifalReader ,
                                         sd->access      ,
                                         NULL            );
                }
        }
        return 0;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Type( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        return "xml";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Name( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        static char buffer[ 128 ];
        sprintf( buffer, "gucefCORE DSTORE codec plugin utilizing the Parsifal XML parser %s", XMLParser_GetVersionString() );
        return buffer;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Copyright( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        return "Copyright (C) Dinand Vanvelzen. 2002 - 2005. LGPLv3.";
}

/*---------------------------------------------------------------------------*/

const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Version( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
        static TVersion version;
        version.major = VERSION_MAJOR_FIELD;
        version.minor = VERSION_MINOR_FIELD;
        version.patch = VERSION_PATCH_FIELD;
        version.release = VERSION_RELEASE_FIELD;

        return &version;
}

/*---------------------------------------------------------------------------*/
