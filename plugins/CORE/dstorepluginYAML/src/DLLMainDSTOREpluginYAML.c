/*
 *  dstorepluginYAML: Plugin adding the ability to use libyaml for the YAML format
 *
 *  Copyright (C) 1998 - 2023.  Dinand Vanvelzen
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

#include <yaml.h>

#include "DLLMainDSTOREpluginYAML.h"    /* gucefCORE DSTORE codec plugin API */

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"  /* gucefCORE macros, used here for the export and callspec macros */
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define VERSION_MAJOR_FIELD     0
#define VERSION_MINOR_FIELD     2
#define VERSION_PATCH_FIELD     5
#define VERSION_RELEASE_FIELD   0

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SDestFileData
{
    TIOAccess* fptr;
    yaml_parser_t parser;
    yaml_emitter_t emitter;
    char activeNodeIsValueNode;
    char* base64EncodeBuffer;
    UInt32 base64EncodeBufferSize;
};
typedef struct SDestFileData TDestFileData;

/*---------------------------------------------------------------------------*/

struct SSrcFileData
{
    TIOAccess* access;
    yaml_parser_t parser;
    yaml_event_t event;
    void* privdata;
    TReadHandlers handlers;
    UInt32 stringEncodingType;
    UInt8* nestingType;
    UInt16 nestingTypeBufferSize;
    UInt16 nestingIndex;
};
typedef struct SSrcFileData TSrcFileData;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( !defined stricmp )
  #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #define stricmp   _stricmp
  #elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    #define stricmp   strcasecmp
  #endif
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static TGucefCoreCApi g_libApi;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt8
YamlEncodingToGucefDataType( yaml_encoding_t yamlEncoding )
{
    switch ( yamlEncoding )
    {
        case YAML_UTF8_ENCODING:
            return GUCEF_DATATYPE_UTF8_STRING;
        case YAML_UTF16LE_ENCODING:
            return GUCEF_DATATYPE_UTF16_LE_STRING;
        case YAML_UTF16BE_ENCODING:
            return GUCEF_DATATYPE_UTF16_BE_STRING;

        default:
            return GUCEF_DATATYPE_UNKNOWN;
    }
}

/*---------------------------------------------------------------------------*/

static UInt32
PushNesting( TSrcFileData* sd, UInt8 nestType )
{
    if ( GUCEF_NULL == sd )
        return 0;

    if ( sd->nestingIndex >= sd->nestingTypeBufferSize )
    {
        UInt16 currentBufferSize = sd->nestingTypeBufferSize;
        UInt16 newBufferSize = 64;
        if ( 0 != currentBufferSize )
            newBufferSize = 2 * currentBufferSize;
        void* newBuffer = realloc( sd->nestingType, newBufferSize );
        if ( GUCEF_NULL != newBuffer )
        {
            sd->nestingType = (UInt8*) newBuffer;
            sd->nestingTypeBufferSize = newBufferSize;
            memset( sd->nestingType, 0, sd->nestingTypeBufferSize );
        }
        else
        {
            return 0;
        }
    }

    sd->nestingType[ sd->nestingIndex ] = nestType;
    ++sd->nestingIndex;
    return 1;
}

/*---------------------------------------------------------------------------*/

static UInt8
PopNesting( TSrcFileData* sd )
{
    if ( sd->nestingIndex > 0 )
    {
        --sd->nestingIndex;
        sd->nestingType[ sd->nestingIndex ] = 0;
        if ( sd->nestingIndex > 0 )
            return sd->nestingType[ sd->nestingIndex-1 ];
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

static UInt8
CurrentNesting( TSrcFileData* sd )
{
    if ( sd->nestingIndex > 0 )
        return sd->nestingType[ sd->nestingIndex-1 ];
    else
        return 0;
}

/*---------------------------------------------------------------------------*/

static Float64
ToFloat64( yaml_event_t* ymlEvent )
{
    Float64 val = 0.0;
    if ( 1 == sscanf( ymlEvent->data.scalar.value, "%lf", &val ) )
        return val;
    return 0.0;
}

/*---------------------------------------------------------------------------*/

static Int64
ToInt64( yaml_event_t* ymlEvent )
{
    Int64 val = 0;
    #ifdef GUCEF_MSWIN_BUILD
    if ( 1 == sscanf( ymlEvent->data.scalar.value, "%I64u", &val ) )
    #else
    if ( 1 == sscanf( ymlEvent->data.scalar.value, "%llu", &val ) )
    #endif
        return val;
    return 0;
}

/*---------------------------------------------------------------------------*/

UInt8
DetectScalarType( yaml_event_t* ymlEvent )
{
    if ( GUCEF_NULL == ymlEvent )
        return GUCEF_DATATYPE_UNKNOWN;

    if ( ymlEvent->data.scalar.style == YAML_PLAIN_SCALAR_STYLE )
    {
        /* with a plain scalar we have to consider that we could be dealing with an int or float
         * look for all digits and a dot
         */
        UInt8 allDigits = 1;
        UInt8 foundDotCount = 0;
        size_t scalarSize = ymlEvent->data.scalar.length;
        yaml_char_t* value = ymlEvent->data.scalar.value;
        for ( size_t i=0; i<scalarSize; ++i )
        {
            yaml_char_t c = value[ i ];
            if ( c == '.' )
            {
                ++foundDotCount;
                if ( foundDotCount > 1 )
                {
                    allDigits = 0;
                    break;
                }
            }
            if ( !( c >= '0' && c <= '9' || c == '+' || c == '-' || c == '.' ) )
            {
                allDigits = 0;
                break;
            }

        }

        if ( 0 == allDigits )
        {
            /* check if its a boolean value
             * yaml supports multiple specific keywords 
             */
            switch ( scalarSize )
            {
                case 5:
                {
                    if ( 0 == memcmp( "false", value, 5 ) )
                        return GUCEF_DATATYPE_BOOLEAN_ASCII_STRING;
                    else
                        return GUCEF_DATATYPE_UTF8_STRING;
                }
                case 4:
                {
                    if ( 0 == memcmp( "true", value, 4 ) )
                        return GUCEF_DATATYPE_BOOLEAN_ASCII_STRING;
                    else
                        return GUCEF_DATATYPE_UTF8_STRING;
                }
                case 3:
                {
                    if ( 0 == memcmp( "off", value, 3 ) ||
                         0 == memcmp( "yes", value, 3 ) )
                        return GUCEF_DATATYPE_BOOLEAN_ASCII_STRING;
                    else
                        return GUCEF_DATATYPE_UTF8_STRING;
                }
                case 2:
                {
                    if ( 0 == memcmp( "on", value, 2 ) ||
                         0 == memcmp( "no", value, 2 ) )
                        return GUCEF_DATATYPE_BOOLEAN_ASCII_STRING;
                    else
                        return GUCEF_DATATYPE_UTF8_STRING;
                }
                default:
                    return GUCEF_DATATYPE_UTF8_STRING;    
                
            }
        }
        else if ( 0 == foundDotCount )
            return GUCEF_DATATYPE_INT64;
        else
            return GUCEF_DATATYPE_FLOAT64;
    }

    return GUCEF_DATATYPE_UTF8_STRING;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Init( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* libyaml itself does not require global initialization */
    if ( GUCEF_NULL != plugdata )
        *plugdata = GUCEF_NULL;

    /* link the gucefCORE API */
    if ( GUCEF_NULL != libApi )
        g_libApi = *libApi;

    return 1;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Shutdown( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* libyaml itself does not require global cleanup */

    memset( &g_libApi, 0, sizeof g_libApi );
}

/*---------------------------------------------------------------------------*/

int 
yaml_write_handler( void *data, unsigned char *buffer, size_t size )
{
    TDestFileData* fd = (TDestFileData*) data;
    if ( GUCEF_NULL != fd )
    {
        if ( 1 == fd->fptr->write( fd->fptr, buffer, size, 1 ) )
        {
            return 1;
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Dest_File_Open( void** plugdata    ,
                           void** filedata    ,
                           TIOAccess* outFile ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    *filedata = GUCEF_NULL;
    if ( outFile != GUCEF_NULL )
    {
        TDestFileData* fd = (TDestFileData*) malloc( sizeof( TDestFileData ) );
        if ( GUCEF_NULL != fd )
        {
            memset( fd, 0, sizeof( TDestFileData ) );
            fd->fptr = outFile;
            fd->activeNodeIsValueNode = 0;
            fd->base64EncodeBuffer = GUCEF_NULL;
            fd->base64EncodeBufferSize = 0;

            if ( 0 != yaml_parser_initialize( &fd->parser ) )
            {
                if ( 0 != yaml_emitter_initialize( &fd->emitter ) )
                {
                    yaml_emitter_set_output( &fd->emitter, &yaml_write_handler, fd );
                    *filedata = fd;
                    return 1;
                }
                yaml_parser_delete( &fd->parser );
            }
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Dest_File_Close( void** plugdata ,
                            void** filedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( filedata == GUCEF_NULL )
        return;
    if ( *filedata == GUCEF_NULL )
        return;

    TDestFileData* fd = (TDestFileData*)*filedata;
    if ( fd != GUCEF_NULL )
    {
        yaml_parser_delete( &fd->parser );
        yaml_emitter_delete( &fd->emitter );
        fd->fptr->close( fd->fptr );

        if ( GUCEF_NULL != fd->base64EncodeBuffer )
        {
            free( fd->base64EncodeBuffer );
            fd->base64EncodeBuffer = GUCEF_NULL;
        }
        free( *filedata );
    }

    *filedata = GUCEF_NULL;
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
    //TDestFileData* fd = (TDestFileData*)*filedata;
    //fd->activeNodeIsValueNode = 0;

    //if ( GUCEF_NULL == fd->currentJsonNode )
    //{
    //    if ( nodeType == GUCEF_DATATYPE_ARRAY )
    //    {
    //        fd->jsonDoc = fd->currentJsonNode = json_array_new( attscount + haschildren );
    //    }
    //    else
    //    {
    //        fd->jsonDoc = fd->currentJsonNode = json_object_new( attscount + haschildren );
    //    }
    //}
    //else
    //{
    //    json_value* childNode = GUCEF_NULL;
    //    if ( nodeType == GUCEF_DATATYPE_ARRAY )
    //    {
    //        childNode = json_array_new( attscount + haschildren );
    //    }
    //    else
    //    if ( nodeType == GUCEF_DATATYPE_OBJECT )
    //    {
    //        if ( 1 != attscount || 0 != haschildren )
    //            childNode = json_object_new( attscount + haschildren );
    //        else
    //            fd->activeNodeIsValueNode = 1;
    //    }

    //    if ( GUCEF_NULL != childNode )
    //    {
    //        if ( GUCEF_NULL == nodename )
    //            nodename = "noname";
    //        if ( fd->currentJsonNode->type == json_object )
    //            json_object_push( fd->currentJsonNode, nodename, childNode );
    //        else
    //        if ( fd->currentJsonNode->type == json_array )
    //            json_array_push( fd->currentJsonNode, childNode );
    //        fd->currentJsonNode = childNode;
    //    }
    //    else
    //    {
    //        if ( 0 == attscount && 0 == haschildren )
    //        {
    //            TVariantData var;
    //            memset( &var, 0, sizeof var );
    //            var.containedType = nodeType;
    //            var.union_data.heap_data.union_data.char_heap_data = (char*) nodename;
    //            var.union_data.heap_data.heap_data_size = (UInt32) strlen( nodename );
    //            
    //            DSTOREPLUG_Store_Node_Att( plugdata, filedata, GUCEF_NULL, 1, 0, GUCEF_NULL, &var, haschildren );
    //        }
    //        if ( nodeType != GUCEF_DATATYPE_ARRAY && nodeType != GUCEF_DATATYPE_OBJECT )
    //        {
    //            fd->activeNodeIsValueNode = 1;
    //        }
    //    }
    //}
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Store( void** plugdata      ,
                           void** filedata      ,
                           const char* nodename ,
                           UInt32 attscount     ,
                           UInt32 haschildren   ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    //TDestFileData* fd = (TDestFileData*)*filedata;
    //if ( ( GUCEF_NULL != fd->currentJsonNode ) && ( 0 == fd->activeNodeIsValueNode ) )
    //    fd->currentJsonNode = fd->currentJsonNode->parent;
    //fd->activeNodeIsValueNode = 0;
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

    //if ( 0 == attscount )
    //    return;

    //if ( GUCEF_NULL == attvalue )
    //{
    //    static TVariantData dummy;
    //    memset( &dummy, 0, sizeof dummy );
    //    attvalue = &dummy;
    //}
    //if ( GUCEF_NULL == nodename )
    //{
    //    nodename = "noname";
    //}

    //switch ( attvalue->containedType )
    //{
    //    case GUCEF_DATATYPE_NULL:
    //    case GUCEF_DATATYPE_NIL:
    //    {
    //        json_value* att = json_null_new(); 
    //        if ( fd->currentJsonNode->type == json_array )
    //            json_array_push( fd->currentJsonNode, att );
    //        else
    //        if ( fd->currentJsonNode->type == json_object )
    //            if ( GUCEF_NULL != attname )
    //                json_object_push( fd->currentJsonNode, attname, att );
    //            else
    //                json_object_push( fd->currentJsonNode, nodename, att );
    //        break;
    //    }
    //    case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING:
    //    case GUCEF_DATATYPE_BOOLEAN_UTF8_STRING:
    //    {
    //        int isNotTrue = stricmp( (const char*) attvalue->union_data.heap_data.union_data.char_heap_data, "true" );
    //        int isNotFalse = stricmp( (const char*) attvalue->union_data.heap_data.union_data.char_heap_data, "false" );

    //        json_value* att = GUCEF_NULL;
    //        if ( isNotTrue == 0 )
    //            att = json_boolean_new( 1 );
    //        else
    //        if ( isNotFalse == 0 )
    //            att = json_boolean_new( 0 );

    //        if ( att != GUCEF_NULL )
    //        {
    //            if ( fd->currentJsonNode->type == json_array )
    //                json_array_push( fd->currentJsonNode, att );
    //            else
    //            if ( fd->currentJsonNode->type == json_object )
    //                if ( GUCEF_NULL != attname )
    //                    json_object_push( fd->currentJsonNode, attname, att );
    //                else
    //                    json_object_push( fd->currentJsonNode, nodename, att );
    //        }
    //        break;
    //    }
    //    case GUCEF_DATATYPE_BOOLEAN_INT32:
    //    {
    //        Int32 boolInt = attvalue->union_data.int32_data;
    //        json_value* att = json_boolean_new( boolInt );
    //        if ( fd->currentJsonNode->type == json_array )
    //            json_array_push( fd->currentJsonNode, att );
    //        else
    //        if ( fd->currentJsonNode->type == json_object )
    //            if ( GUCEF_NULL != attname )
    //                json_object_push( fd->currentJsonNode, attname, att );
    //            else
    //                json_object_push( fd->currentJsonNode, nodename, att );
    //        break;
    //    }
    //    case GUCEF_DATATYPE_FLOAT32:
    //    case GUCEF_DATATYPE_FLOAT64:
    //    case GUCEF_DATATYPE_NUMERIC:
    //    {
    //        double number = 0.0l;
    //        if ( GUCEF_DATATYPE_FLOAT32 == attvalue->containedType )
    //            number = attvalue->union_data.float32_data;
    //        else
    //            number = attvalue->union_data.float64_data;

    //        json_value* att = json_double_new( number );
    //        if ( fd->currentJsonNode->type == json_array )
    //            json_array_push( fd->currentJsonNode, att );
    //        else
    //        if ( fd->currentJsonNode->type == json_object )
    //            if ( GUCEF_NULL != attname )
    //                json_object_push( fd->currentJsonNode, attname, att );
    //            else
    //                json_object_push( fd->currentJsonNode, nodename, att );
    //        break;
    //    }
    //    case GUCEF_DATATYPE_INT8:
    //    case GUCEF_DATATYPE_UINT8:
    //    case GUCEF_DATATYPE_INT16:
    //    case GUCEF_DATATYPE_UINT16:
    //    case GUCEF_DATATYPE_INT32:
    //    case GUCEF_DATATYPE_UINT32:
    //    case GUCEF_DATATYPE_INT64:
    //    case GUCEF_DATATYPE_UINT64:
    //    {
    //        json_value* att = GUCEF_NULL;
    //        long long integer = 0;
    //        switch ( attvalue->containedType )
    //        {
    //            case GUCEF_DATATYPE_INT8:   { integer = (long long) attvalue->union_data.int8_data; break; }
    //            case GUCEF_DATATYPE_UINT8:  { integer = (long long) attvalue->union_data.uint8_data; break; }
    //            case GUCEF_DATATYPE_INT16:  { integer = (long long) attvalue->union_data.int16_data; break; }
    //            case GUCEF_DATATYPE_UINT16: { integer = (long long) attvalue->union_data.uint16_data; break; }
    //            case GUCEF_DATATYPE_INT32:  { integer = (long long) attvalue->union_data.int32_data; break; }
    //            case GUCEF_DATATYPE_UINT32: { integer = (long long) attvalue->union_data.uint32_data; break; }
    //            case GUCEF_DATATYPE_INT64:  { integer = (long long) attvalue->union_data.int64_data; break; }
    //            case GUCEF_DATATYPE_UINT64: { integer = (long long) attvalue->union_data.uint64_data; break; }
    //        }

    //        att = json_integer_new( integer );
    //        if ( fd->currentJsonNode->type == json_array )
    //            json_array_push( fd->currentJsonNode, att );
    //        else
    //        if ( fd->currentJsonNode->type == json_object )
    //            if ( GUCEF_NULL != attname )
    //                json_object_push( fd->currentJsonNode, attname, att );
    //            else
    //                json_object_push( fd->currentJsonNode, nodename, att );
    //        break;
    //    }
    //    case GUCEF_DATATYPE_BINARY_BLOB:
    //    {
    //        if ( 0 != Base64Encode( attvalue->union_data.heap_data.union_data.void_heap_data , 
    //                                attvalue->union_data.heap_data.heap_data_size            ,
    //                                fd                                                       ) )
    //        {
    //            if ( GUCEF_NULL != fd->base64EncodeBuffer )
    //            {
    //                json_value* att = json_string_new( fd->base64EncodeBuffer );
    //                if ( fd->currentJsonNode->type == json_array )
    //                    json_array_push( fd->currentJsonNode, att );
    //                else
    //                if ( fd->currentJsonNode->type == json_object )
    //                    if ( GUCEF_NULL != attname )
    //                        json_object_push( fd->currentJsonNode, attname, att );
    //                    else
    //                        json_object_push( fd->currentJsonNode, nodename, att );
    //                break;            
    //            }
    //        }
    //        break;
    //    }
    //    case GUCEF_DATATYPE_BINARY_BSOB:
    //    {
    //        if ( 0 != Base64Encode( &attvalue->union_data.bsob_data       , 
    //                                sizeof attvalue->union_data.bsob_data ,
    //                                fd                                    ) )
    //        {
    //            if ( GUCEF_NULL != fd->base64EncodeBuffer )
    //            {
    //                json_value* att = json_string_new( fd->base64EncodeBuffer );
    //                if ( fd->currentJsonNode->type == json_array )
    //                    json_array_push( fd->currentJsonNode, att );
    //                else
    //                if ( fd->currentJsonNode->type == json_object )
    //                    if ( GUCEF_NULL != attname )
    //                        json_object_push( fd->currentJsonNode, attname, att );
    //                    else
    //                        json_object_push( fd->currentJsonNode, nodename, att );
    //                break;            
    //            }
    //        }
    //        break;
    //    }
    //    case GUCEF_DATATYPE_UTF8_STRING:
    //    case GUCEF_DATATYPE_ASCII_STRING:
    //    default:
    //    {
    //        const char* attValue = attvalue->union_data.heap_data.union_data.char_heap_data;
    //        if ( GUCEF_NULL == attValue )
    //            attValue = "";
    //        json_value* att = json_string_new( attValue );
    //        if ( fd->currentJsonNode->type == json_array )
    //            json_array_push( fd->currentJsonNode, att );
    //        else
    //        if ( fd->currentJsonNode->type == json_object )
    //            if ( GUCEF_NULL != attname )
    //                json_object_push( fd->currentJsonNode, attname, att );
    //            else
    //                json_object_push( fd->currentJsonNode, nodename, att );
    //        break;
    //    }
    //}
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Begin_Node_Children( void** plugdata      ,
                                void** filedata      ,
                                const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{

}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_End_Node_Children( void** plugdata      ,
                              void** filedata      ,
                              const char* nodename ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{

}

/*---------------------------------------------------------------------------*/

int
yaml_read_handler( void *data, unsigned char *buffer, size_t size, size_t *size_read )
{
    TSrcFileData* sd = (TSrcFileData*) data;
    if ( GUCEF_NULL != sd )
    {
        UInt64 bytesRead = sd->access->read( sd->access, buffer, 1, size );
        *size_read = bytesRead;
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Src_File_Open( void** plugdata  ,
                          void** codecdata ,
                          void** filedata  ,
                          TIOAccess* file  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TSrcFileData* sd = GUCEF_NULL;
    *plugdata = GUCEF_NULL;

    sd = (TSrcFileData*) malloc( sizeof(TSrcFileData) );
    if ( GUCEF_NULL != sd )
    {
        memset( sd, 0, sizeof( TSrcFileData ) );
        sd->stringEncodingType = GUCEF_DATATYPE_UTF8_STRING;
        sd->access = file;
        if ( 0 != yaml_parser_initialize( &sd->parser ) )
        {
            yaml_parser_set_input( &sd->parser, yaml_read_handler, sd );            
            
            *filedata = sd;
            return 1;
        }

        free( sd );
    }    
    return 0;    
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Src_File_Close( void** plugdata  ,
                           void** codecdata ,
                           void** filedata  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( *filedata != GUCEF_NULL )
    {
        TSrcFileData* sd = (TSrcFileData*) *filedata;
        if ( GUCEF_NULL != sd )
        {
            yaml_parser_delete( &sd->parser );
            sd->access->close( sd->access );            
            free( sd );
        }        
    }
}

/*---------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Set_Read_Handlers( void** plugdata                ,
                              void** codecdata               ,
                              void** filedata                ,
                              const TReadHandlers* rhandlers ,
                              void* privdata                 ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( *filedata != GUCEF_NULL )
    {
        TSrcFileData* sd = (TSrcFileData*) *filedata;
        sd->handlers = *rhandlers;
        sd->privdata = privdata;
    }
}

/*---------------------------------------------------------------------------*/

//static void process_object( TSrcFileData* sd, const char* name, json_value* value )
//{
//    int length=0, x=0, childNodes=0;
//    if ( GUCEF_NULL == value )
//        return;
//
//    sd->handlers.OnNodeBegin( sd->privdata, name, GUCEF_DATATYPE_OBJECT );
//    length = value->u.object.length;
//    for ( x=0; x<length; x++ )
//    {
//        if ( value->type != json_object && value->type != json_array )
//            process_value( sd, name, value->u.object.values[x].name, value->u.object.values[x].value );
//        else
//            childNodes++;
//    }
//    if ( childNodes > 0 )
//    {
//        sd->handlers.OnNodeChildrenBegin( sd->privdata, name );
//        for ( x=0; x<length; x++ )
//        {
//            if ( value->type == json_object || value->type == json_array )
//                process_value( sd, name, value->u.object.values[x].name, value->u.object.values[x].value );
//        }
//        sd->handlers.OnNodeChildrenEnd( sd->privdata, name );
//    }
//    sd->handlers.OnNodeEnd( sd->privdata, name );
//}

/*---------------------------------------------------------------------------*/

//int
//json_type_to_gucef_type( json_type type )
//{
//    switch ( type )
//    {
//        case json_object:
//            return GUCEF_DATATYPE_OBJECT;
//        case json_array:
//            return GUCEF_DATATYPE_ARRAY;
//        case json_integer:
//            return GUCEF_DATATYPE_INT64;
//        case json_double:
//            return GUCEF_DATATYPE_FLOAT64;
//        case json_string:
//            return GUCEF_DATATYPE_STRING;
//        case json_boolean:
//            return GUCEF_DATATYPE_BOOLEAN_STRING;
//        case json_null:
//            return GUCEF_DATATYPE_NIL;
//        case json_none:
//        default: 
//            return GUCEF_DATATYPE_UNKNOWN;
//    }
//}

/*---------------------------------------------------------------------------*/

//static void
//process_array( TSrcFileData* sd, const char* name, json_value* value )
//{
//    int length=0, x=0;
//
//    if ( GUCEF_NULL == value )
//        return;
//
//    sd->handlers.OnNodeBegin( sd->privdata, name, GUCEF_DATATYPE_ARRAY );
//    sd->handlers.OnNodeChildrenBegin( sd->privdata, name );
//    length = value->u.array.length;
//    for ( x=0; x<length; x++ )
//    {
//        process_value( sd, name, GUCEF_NULL, value->u.array.values[x] );
//    }
//    sd->handlers.OnNodeChildrenEnd( sd->privdata, name );
//    sd->handlers.OnNodeEnd( sd->privdata, name );
//}

/*---------------------------------------------------------------------------*/
//
//static void
//process_value( TSrcFileData* sd    ,
//               const char* objName ,
//               const char* name    ,
//               json_value* value   )
//{
//    if ( GUCEF_NULL == value )
//        return;
//
//    switch ( value->type )
//    {
//        case json_none:
//        {
//            break;
//        }
//        case json_null:
//        {
//            sd->handlers.OnNodeAtt( sd->privdata, objName, name, NULL, GUCEF_DATATYPE_NIL );
//            break;
//        }
//        case json_object:
//        {
//            process_object( sd, name, value );
//            break;
//        }
//        case json_array:
//        {
//            process_array( sd, name, value );
//            break;
//        }
//        case json_integer:
//        {
//            char valueBuffer[64];
//            sprintf( valueBuffer, "%lli", value->u.integer );
//
//            sd->handlers.OnNodeAtt( sd->privdata, objName, name, valueBuffer, GUCEF_DATATYPE_INT64 );
//            break;
//        }
//        case json_double:
//        {
//            char valueBuffer[64];
//            sprintf( valueBuffer, "%f", value->u.dbl );
//
//            sd->handlers.OnNodeAtt( sd->privdata, objName, name, valueBuffer, GUCEF_DATATYPE_FLOAT64 );
//            break;
//        }
//        case json_string:
//        {
//            sd->handlers.OnNodeAtt( sd->privdata, objName, name, value->u.string.ptr, GUCEF_DATATYPE_STRING );
//            break;
//        }
//        case json_boolean:
//        {
//            char* boolValue = "true";
//            if ( 0 == value->u.boolean )
//                boolValue = "false";
//
//            sd->handlers.OnNodeAtt( sd->privdata, objName, name, boolValue, GUCEF_DATATYPE_BOOLEAN_STRING );
//            break;
//        }
//    }
//}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Start_Reading( void** plugdata  ,
                          void** codecdata ,
                          void** filedata  ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( GUCEF_NULL != filedata )
    {
        TSrcFileData* sd = (TSrcFileData*) *filedata;
        if ( GUCEF_NULL != sd )
        {
            UInt8 currentNesting = 0;
            UInt8 nextScalarIsAKey = 1;
            UInt8 dontDeleteYamlEvent = 0;
            yaml_event_t yamlEvent;
            yaml_event_t lastYamlEvent;
            do 
            {
                if (  0 == yaml_parser_parse( &sd->parser, &yamlEvent ) ) 
                {
                    size_t problemStrSize = strlen( sd->parser.problem );
                    char* problemStr = calloc( 1, 1024);
                    if ( NULL != problemStr )
                    {
                        sprintf( problemStr, "problem:\"%s\" offset=%zu", sd->parser.problem, sd->parser.problem_offset );   
                    }
                    (*sd->handlers.OnError)( sd->privdata, (Int32) sd->parser.error, problemStr );
                    free( problemStr );
                    return (UInt32) sd->parser.error;
                }

                switch ( yamlEvent.type )
                { 
                    case YAML_NO_EVENT: 
                        break; 
                    
                    /* Stream start/end */
                    case YAML_STREAM_START_EVENT:
                    {
                        sd->stringEncodingType = YamlEncodingToGucefDataType( yamlEvent.data.stream_start.encoding );
                        break;
                    }
                    case YAML_STREAM_END_EVENT: 
                        break;

                    /* Block delimeters */
                    case YAML_DOCUMENT_START_EVENT:
                    {
                        (*sd->handlers.OnTreeBegin)( sd->privdata );
                        break;
                    }
                    case YAML_DOCUMENT_END_EVENT:    
                    {
                        (*sd->handlers.OnTreeEnd)( sd->privdata );
                        break;
                    }
                    case YAML_SEQUENCE_START_EVENT:  
                    {
                        const char* name = GUCEF_NULL;
                        if ( 0 != dontDeleteYamlEvent )
                            name = lastYamlEvent.data.scalar.value;
                        sd->handlers.OnNodeBegin( sd->privdata, name, GUCEF_DATATYPE_ARRAY );
                        sd->handlers.OnNodeChildrenBegin( sd->privdata, name );
                        if ( 0 != dontDeleteYamlEvent )
                        {
                            dontDeleteYamlEvent = 0;
                            yaml_event_delete( &lastYamlEvent );
                        }
                        currentNesting = (UInt8) YAML_SEQUENCE_START_EVENT;
                        PushNesting( sd, (UInt8) YAML_SEQUENCE_START_EVENT );
                        break;
                    }
                    case YAML_SEQUENCE_END_EVENT:    
                    {
                        const char* name = yamlEvent.data.sequence_start.tag;
                        sd->handlers.OnNodeChildrenEnd( sd->privdata, name );
                        sd->handlers.OnNodeEnd( sd->privdata, name );
                        currentNesting = PopNesting( sd );
                        if ( YAML_MAPPING_START_EVENT == currentNesting )
                            nextScalarIsAKey = 1;
                        else
                            nextScalarIsAKey = 0;
                        break;
                    }
                    case YAML_MAPPING_START_EVENT:   
                    {
                        const char* name = GUCEF_NULL;
                        if ( 0 != dontDeleteYamlEvent )
                            name = lastYamlEvent.data.scalar.value;
                        currentNesting = (UInt8) YAML_MAPPING_START_EVENT;
                        nextScalarIsAKey = 1;
                        sd->handlers.OnNodeBegin( sd->privdata, name, GUCEF_DATATYPE_OBJECT );
                        sd->handlers.OnNodeChildrenBegin( sd->privdata, name );
                        if ( 0 != dontDeleteYamlEvent )
                        {
                            dontDeleteYamlEvent = 0;
                            yaml_event_delete( &lastYamlEvent );
                        }
                        PushNesting( sd, (UInt8) YAML_MAPPING_START_EVENT );
                        break;
                    }
                    case YAML_MAPPING_END_EVENT:   
                    {
                        const char* name = yamlEvent.data.mapping_start.tag;
                        sd->handlers.OnNodeChildrenEnd( sd->privdata, name );
                        sd->handlers.OnNodeEnd( sd->privdata, name );
                        currentNesting = PopNesting( sd );
                        if ( YAML_MAPPING_START_EVENT == currentNesting )
                            nextScalarIsAKey = 1;
                        else
                            nextScalarIsAKey = 0;
                        break;
                    }

                    /* Data */

                    case YAML_ALIAS_EVENT:   
                        break;
                        
                    case YAML_SCALAR_EVENT:  
                    {
                        if ( 0 != nextScalarIsAKey )
                        {
                            memcpy( &lastYamlEvent, &yamlEvent, sizeof( yamlEvent ) );
                            dontDeleteYamlEvent = 1;
                            nextScalarIsAKey = 0;                         
                        }
                        else
                        {
                            TVariantData var;
                            memset( &var, 0, sizeof( var ) );

                            UInt8 scalarType = DetectScalarType( &yamlEvent );
                            switch ( scalarType )
                            {
                                case GUCEF_DATATYPE_FLOAT64 :
                                {
                                    var.containedType = GUCEF_DATATYPE_FLOAT64;
                                    var.union_data.float64_data = ToFloat64( &yamlEvent );
                                    break;
                                }
                                case GUCEF_DATATYPE_INT64 :
                                {
                                    var.containedType = GUCEF_DATATYPE_INT64;
                                    var.union_data.int64_data = ToInt64( &yamlEvent );
                                    break;
                                }
                                case GUCEF_DATATYPE_BOOLEAN_ASCII_STRING :
                                case GUCEF_DATATYPE_UTF8_STRING:
                                case GUCEF_DATATYPE_UTF16_LE_STRING:
                                case GUCEF_DATATYPE_UTF16_BE_STRING:
                                default:
                                {
                                    var.containedType = scalarType;
                                    var.union_data.heap_data.heap_data_is_linked = 1;
                                    var.union_data.heap_data.heap_data_size = (UInt32) yamlEvent.data.scalar.length+1;
                                    var.union_data.heap_data.union_data.char_heap_data = yamlEvent.data.scalar.value;
                                    break;
                                }
                            }

                            yaml_char_t* key = GUCEF_NULL;
                            if ( YAML_MAPPING_START_EVENT == currentNesting )
                            {
                                key = lastYamlEvent.data.scalar.value;
                                nextScalarIsAKey = 1;
                            }

                            sd->handlers.OnNodeAtt( sd->privdata, "", key, &var );

                            if ( YAML_MAPPING_START_EVENT == currentNesting )
                            {
                                yaml_event_delete( &lastYamlEvent );
                                dontDeleteYamlEvent = 0;
                            }
                        }
                        break;
                    }
                }

                if( yamlEvent.type != YAML_STREAM_END_EVENT && 0 == dontDeleteYamlEvent )
                    yaml_event_delete( &yamlEvent );

            } 
            while( yamlEvent.type != YAML_STREAM_END_EVENT );
            yaml_event_delete( &yamlEvent );

            return 0; /* error code of 0 means no error */
        }
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Type( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "yaml";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Name( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "gucefCORE DSTORE codec plugin utilizing libyaml for reading/writing the YAML format";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
DSTOREPLUG_Copyright( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen. 1998 - 2023. Licensed under the Apache License, Version 2.0 (the \"License\")";
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
