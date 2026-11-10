/*
 *  gucefAUDIO_TestCodecPlugin: gucefAUDIO codec plugin for tests, with the optional plugin interface additions
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

/*
 *  The .au codec of audiocodecpluginAU, under the type names "autest" and "sndtest" so both plugins can be
 *  loaded at the same time, with the optional entry points of the codec plugin interface added:
 *
 *    AUDIOCODECPLUG_Threading_Model  streams can be decoded at the same time, unless the environment variable
 *                                    GUCEF_AUDIOCODECPLUGINAUTEST_THREADING is "lock" when the plugin is linked
 *                                    ("unknown" makes it answer a value that is not defined, to see how the host copes)
 *    AUDIOCODECPLUG_Src_Meta_Data    the annotation of the file as tags: lines of NAME=value become the lower case
 *                                    key "name", the whole text is "comment", plus "encoding" and "position"
 *    AUDIOCODECPLUG_Probe            90 when the data starts with the .au magic, 0 otherwise
 *
 *  The additions live here, after the include of the .au source, which they extend with its own stream state.
 *  Per stream they use nothing but that state and the input of the stream, which is what the per source
 *  threading model asks for.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>

#define AU_CODEC_TYPE_PRIMARY       "autest"
#define AU_CODEC_TYPE_SECONDARY     "sndtest"

#include "../../../plugins/AUDIO/audiocodecpluginAU/src/audiocodecpluginAU.c"

/*-------------------------------------------------------------------------//
//                                                                         //
//      PLUGIN API                                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

AUDIOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Threading_Model )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Meta_Data )( void** plugdata, void** codecdata, void** sourcedata, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

AUDIOCODECPLUG_EXPORT_C UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Probe )( void** plugdata, void** codecdata, TIOAccess* input, UInt32* confidence ) GUCEF_PLUGIN_CALLSPEC_SUFFIX;

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Threading_Model )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const char* mode = getenv( "GUCEF_AUDIOCODECPLUGINAUTEST_THREADING" );
    if ( NULL != mode && 0 == strcmp( mode, "lock" ) )
        return GUCEF_AUDIOCODEC_THREADING_PLUGIN_LOCK;
    if ( NULL != mode && 0 == strcmp( mode, "unknown" ) )
        return 77;
    return GUCEF_AUDIOCODEC_THREADING_PER_SOURCE;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Meta_Data )( void** plugdata, void** codecdata, void** sourcedata, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TAuSource* source = (TAuSource*) *sourcedata;
    UInt32 dataOffset, dataSize, encoding, sampleRate, channels, length;
    UInt64 resumeAt;
    char* annotation;
    char number[ 32 ];
    char key[ 64 ];
    char* line;

    if ( NULL == source )
        return 0;

    /* the files of the tests start at the start of the input; the stream is put back where it was */
    resumeAt = source->input->tell( source->input );
    if ( 0 != source->input->setpos( source->input, 0 ) )
        return 0;
    if ( !ReadHeader( source->input, &dataOffset, &dataSize, &encoding, &sampleRate, &channels ) )
    {
        source->input->setpos( source->input, resumeAt );
        return 0;
    }

    SetTextValue( metaData, "encoding", AU_ENCODING_MULAW == encoding ? "mu-law" : ( AU_ENCODING_ALAW == encoding ? "A-law" : "linear" ) );
    sprintf( number, "%lu", (unsigned long) source->position );
    SetTextValue( metaData, "position", number );

    length = dataOffset - AU_HEADER_SIZE;
    if ( 0 != length && length <= AU_MAX_ANNOTATION )
    {
        annotation = (char*) calloc( 1, length + 1 );
        if ( NULL != annotation )
        {
            if ( 1 == source->input->read( source->input, annotation, length, 1 ) && '\0' != annotation[ 0 ] )
            {
                SetTextValue( metaData, "comment", annotation );

                /* NAME=value lines, as in the comments of Ogg files */
                line = strtok( annotation, "\n" );
                while ( NULL != line )
                {
                    char* equals = strchr( line, '=' );
                    if ( NULL != equals && equals != line && (size_t) ( equals - line ) < sizeof( key ) )
                    {
                        size_t i, keyLength = (size_t) ( equals - line );
                        for ( i=0; i<keyLength; ++i )
                            key[ i ] = (char) ( line[ i ] >= 'A' && line[ i ] <= 'Z' ? line[ i ] + ( 'a' - 'A' ) : line[ i ] );
                        key[ keyLength ] = '\0';
                        SetTextValue( metaData, key, equals + 1 );
                    }
                    line = strtok( NULL, "\n" );
                }
            }
            free( annotation );
        }
    }

    source->input->setpos( source->input, resumeAt );
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Probe )( void** plugdata, void** codecdata, TIOAccess* input, UInt32* confidence ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    UInt8 magic[ 4 ];

    *confidence = 0;
    if ( NULL == input || NULL == input->read )
        return 0;
    if ( 1 == input->read( input, magic, 4, 1 ) && AU_MAGIC == ReadBE32( magic ) )
        *confidence = 90;
    return 1;
}

/*-------------------------------------------------------------------------*/
