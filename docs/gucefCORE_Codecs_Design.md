# gucefCORE codecs

Module `platform/gucefCORE`, namespace `GUCEF::CORE`. A codec turns bytes into other bytes: compression, checksums, hashes, message authentication, key derivation, encryption, key agreement and signatures. Codecs are found by **family** and **type** name in the `CCodecRegistry` (`CCoreGlobal::GetCodecRegistry()`). They come from three places: built into gucefCORE (portable software code), the operating system (registered by gucefCORE when the OS offers them), and **plugins** with a C interface (`plugins/*/codecsplugin*`, `plugins/IMAGE/imgplugin*`). `platform/gucefCORE` holds no third party code.

Tests: `tests/gucefCORE_TestApp` (`TestCodecPlugins.cpp`: the C interface through a plugin made of test functions, zlib and hash plugins; `TestCryptoCodecs.cpp`: registry, instances, known answers of every provider, allocations), `tests/gucefVFS_TestApp` (`TestVfsCodecs.cpp`), `tests/gucefIMAGE_TestApp` (`TestImageCodecPlugins.cpp`).

## 1. Scope

| supported | can be added without breaking the interface |
|---|---|
| one shot `Encode` / `Decode` from a `CIOAccess` to a `CIOAccess`, with or without parameters | asynchronous codecs (the browser's WebCrypto), see section 9 |
| parameters per call (`CValueList`): keys, nonces, levels, output forms | parameters that are not values (callbacks, streams) |
| a description of the parameters of every codec, for applications and user interfaces | ranges and allowed values in that description |
| prepared instances for hot paths and streams: no lookups and no heap allocation per call | instances that are shared between threads |
| several providers per type, picked by priority, all of them listed | per call choice of a provider by name |
| codec plugins in C, one breaking round of their interface with a version handshake | further functions at the end of the published structs |
| OS crypto: Linux AF_ALG, Windows CNG, macOS CommonCrypto | hardware tokens, key stores |

## 2. Layers

```
 application ──► CCodecRegistry ──► CICodec ──► CreateInstance() ──► CICodecInstance::Process()   (hot path: no lookups, no allocation)
                    │                  ├── built in: CSha256Codec, CHmacSha256Codec, CHkdfSha256Codec (CInstanceBasedCodec)
                    │                  ├── OS: AF_ALG (Linux), CNG (Windows), CommonCrypto (macOS)
                    │                  └── CStdCodecPluginItem ──► TCodecPluginLink ──► C plugin (codecsplugin*, imgplugin*)
                    └── family registry per family (CTONRegistry): the provider with the highest priority per type
```

### 2.1 CICodec

```cpp
virtual bool Encode( CIOAccess& source, CIOAccess& dest ) = 0;
virtual bool Decode( CIOAccess& source, CIOAccess& dest ) = 0;
virtual bool Encode( CIOAccess& source, CIOAccess& dest, const CValueList& params );     // default: the two argument version
virtual bool Decode( CIOAccess& source, CIOAccess& dest, const CValueList& params );     // default: the two argument version
virtual void GetParameterInfo( CCodecParameterInfoList& info ) const;                    // default: none
virtual bool CreateInstance( const CValueList& params, CICodecInstancePtr& instance );   // default: CDefaultCodecInstance
virtual UInt32 GetCapabilities( void ) const;                                            // GUCEF_CODEC_CAPABILITY_*, default encode | decode
TCodecStatus GetLastStatus( void ) const;
CString GetLastErrorMessage( void ) const;
```

* Parameters a codec does not know are ignored, so one parameter list can serve several providers.
* Codecs are shared: `Encode` and `Decode` may be called from several threads at once.
* `GetLastStatus()` / `GetLastErrorMessage()` give the outcome of the last call the **calling thread** made on that codec through the parameter overloads (`GUCEF_CODEC_STATUS_*`, see section 3.3), so a caller can tell a failed authentication from a missing key. A codec records it with the protected `SetLastStatus()`; codecs that never do report `GUCEF_CODEC_STATUS_OK` or `GUCEF_CODEC_STATUS_FAILED` as the overload returned.
* `GetParameterInfo()` fills `CCodecParameterInfo` entries: name, value type (`GUCEF_DATATYPE_*`), required, default value, description.

### 2.2 CICodecInstance

A prepared codec with its parameters taken in once: a cipher with its key, an HMAC with its key, compression at a level, a hash in progress.

```cpp
virtual bool Process( UInt32 flags, const void* input, UInt32 inputSize, const SCodecCallArgs* args, CDynamicBuffer& output ) = 0;
virtual UInt32 GetOverhead( void ) const = 0;          // bytes encoding adds to a message (an AEAD tag), 0 when none or not fixed
virtual TCodecStatus GetLastStatus( void ) const = 0;
virtual CString GetLastErrorMessage( void ) const = 0;
```

* `flags`: `GUCEF_CODEC_PROCESS_BEGIN` starts a message (an unfinished one is dropped), `GUCEF_CODEC_PROCESS_FINAL` ends it and produces what comes at the end (a digest, a tag), `GUCEF_CODEC_PROCESS_UPDATE` marks a call in between and has no effect of its own; plus one of `GUCEF_CODEC_PROCESS_ENCODE` / `GUCEF_CODEC_PROCESS_DECODE`. The input is always taken in. `GUCEF_CODEC_PROCESS_ENCODE_MESSAGE` and `GUCEF_CODEC_PROCESS_DECODE_MESSAGE` are the one call forms.
* `SCodecCallArgs` (the C struct `TCodecCallArgs`) is a plain struct of pointers: the nonce and the associated data (AAD) of this call. NULL when a codec needs neither.
* Output is **appended** to `output` after its current data. A caller that reuses one buffer and clears it logically (`Clear( true )`) makes no allocation once the buffer has grown to its working size.
* One instance is used by one thread at a time. Instances are independent of each other.
* The default instance (`CDefaultCodecInstance`) gathers the input until FINAL and then runs the codec's `Encode` / `Decode` with the parameters given at creation plus `nonce` / `aad` from the call arguments. It works for every codec, but allocates.

### 2.3 CCodecRegistry

```cpp
static const Int32 SoftwareCodecPriority = 25;
static const Int32 PluginCodecPriority = 50;
static const Int32 OsCodecPriority = 100;

bool RegisterCodec( const TICodecPtr& codec, Int32 priority = PluginCodecPriority );
bool RegisterCodec( const CString& family, const CString& type, const TICodecPtr& codec, Int32 priority );
bool UnregisterCodec( const TICodecPtr& codec );
TICodecPtr TryGetCodec( const CString& family, const CString& type ) const;
bool GetCodecs( const CString& family, const CString& type, TICodecPtrVector& codecs ) const;
```

* The registry stays a family → (type → codec) registry, so code that looks up `TryLookup( family )` and then `TryLookup( type )` keeps working: the family registry holds the provider with the highest priority.
* `RegisterCodec` adds a provider to the list of its family and type. Equal priorities keep the order of registration. A codec put into a family registry directly (the older way) counts as a registration at `PluginCodecPriority` once another provider of that type registers.
* `UnregisterCodec` takes out every registration of that codec object; the next provider of the type, if any, becomes the one in the family registry.
* `GetCodecs` lists all providers of a type, highest priority first, for fallback and cross checks.
* `UnregisterAll()` clears the lists too.

### 2.4 Priorities

| source | priority |
|---|---|
| OS (AF_ALG, CNG, CommonCrypto) | `OsCodecPriority` 100 |
| plugins | `PluginCodecPriority` 50, or the plugin's `priorityHint`, or the plugin metadata parameter `codecPriority` (the last one wins) |
| built in software | `SoftwareCodecPriority` 25 |

The OS goes first because it is maintained with the system and may use hardware. Built in software is the fallback that is always there.

## 3. The codec plugin C interface

Header `platform/gucefCORE/include/c_api/gucefCORE_c_codec_plugin_api.h` (`CodecPluginLink.h` includes it for older include lines). This is version **2.0** of the interface: everything below came in one breaking round, so that later additions only append.

### 3.1 Exports

```c
UInt32 CODECPLUGIN_GetApiVersion( void );                                   /* GUCEF_CODECPLUGIN_API_VERSION */
TCodecStatus CODECPLUGIN_Init( void** plugdata, TGucefCoreCApi* coreApi, const TVariantMapApi* pluginParams );
TCodecStatus CODECPLUGIN_Shutdown( void* plugdata );
const char* CODECPLUGIN_Description( void* plugdata );
const char* CODECPLUGIN_Copyright( void* plugdata );
TVersion CODECPLUGIN_Version( void* plugdata );
UInt32 CODECPLUGIN_GetCodecCount( void* plugdata );
TCodecStatus CODECPLUGIN_GetCodecLink( void* plugdata, UInt32 index, const TCodecPluginLink** codecLink );
```

* **Version handshake**: `GUCEF_CODECPLUGIN_API_VERSION` is `( major << 16 ) | minor`. The host refuses a plugin of another major version, and a plugin without `CODECPLUGIN_GetApiVersion` (built for the interface before 2.0), with a log line that names the versions. It never calls any other function of such a plugin. A higher minor version of the same major is accepted.
* **Init** gets the core C API (logging) and the parameters of the plugin's metadata (`CIPluginMetaData::GetParams()`) as a text map. Both stay valid until `Shutdown`.
* **Codec listing**: the plugin hands out links it owns (static data is fine); they stay valid until `Shutdown`.

### 3.2 TCodecPluginLink

```c
struct SCodecPluginLink
{
    UInt32 structSize;              /* sizeof( TCodecPluginLink ) as the plugin was built */
    const char* codecFamily;
    const char* codecType;
    void* codecData;                /* the plugin's own, identifies the codec in calls */
    UInt32 capabilities;            /* GUCEF_CODEC_CAPABILITY_* */
    Int32 priorityHint;             /* 0: the host default for plugins */
    UInt32 overhead;                /* bytes encoding adds to a message (AEAD tag), 0 when none or not fixed */
    TCODECPLUGFPTR_Encode encode;
    TCODECPLUGFPTR_Decode decode;
    TCODECPLUGFPTR_GetParameterInfo getParameterInfo;       /* optional */
    TCODECPLUGFPTR_CreateInstance createInstance;           /* optional, with the three below */
    TCODECPLUGFPTR_InstanceProcess instanceProcess;
    TCODECPLUGFPTR_DestroyInstance destroyInstance;
    TCODECPLUGFPTR_GetOutputSizeBound getOutputSizeBound;   /* optional */
};
```

* The host reads only what `structSize` covers and treats the rest as absent (zero / NULL), so fields only ever go at the end.
* `encode` / `decode`: `( pluginData, codecData, const TVariantMapApi* params, TIOAccess* input, TIOAccess* output, TCodecErrorInfo* error )`. The family and type are not passed: `codecData` identifies the codec, so there is no string compare per call. `encode` or `decode` may be NULL when the capability is absent.
* `getParameterInfo( pluginData, codecData, index, TCodecParameterInfo* )`: `GUCEF_CODEC_STATUS_OK` for every index below the parameter count, `GUCEF_CODEC_STATUS_UNSUPPORTED` after.
* `createInstance( pluginData, codecData, params, void** instanceData, error )`, `instanceProcess( instanceData, flags, input, inputSize, const TCodecCallArgs* args, void* output, UInt32 outputCapacity, UInt32* outputSize, error )`, `destroyInstance( instanceData )`: buffer based, no `TIOAccess`, so a call needs no allocation in the host or the plugin.
* `getOutputSizeBound( pluginData, codecData, instanceData, flags, inputSize, UInt32* bound )`: the most a call with that input can write. `instanceData` is NULL for one shot use. `GUCEF_CODEC_STATUS_UNSUPPORTED` when the output cannot be bounded beforehand (decompression).

### 3.3 Status and errors

`TCodecStatus` values never change; new ones are only added:

| value | meaning |
|---|---|
| `GUCEF_CODEC_STATUS_OK` 0 | done |
| `GUCEF_CODEC_STATUS_FAILED` 1 | failed for another reason (I/O, out of memory, the OS refused) |
| `GUCEF_CODEC_STATUS_INVALID_INPUT` 2 | the input is not what the codec decodes (corrupt, truncated) |
| `GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED` 3 | an AEAD tag or a signature does not match |
| `GUCEF_CODEC_STATUS_MISSING_PARAMETER` 4 | a required parameter is absent |
| `GUCEF_CODEC_STATUS_INVALID_PARAMETER` 5 | a parameter has a wrong size, type or value |
| `GUCEF_CODEC_STATUS_UNSUPPORTED` 6 | the codec does not do that (decoding a hash, a missing capability) |
| `GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL` 7 | see below |

`TCodecErrorInfo` is owned by the caller: `structSize` and a fixed `message` buffer (`GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE`). A plugin writes a zero terminated message on failure when the pointer is not NULL. The C++ side returns `bool` and keeps the status and message (`GetLastStatus()`, `GetLastErrorMessage()`).

**Output room for instances**: the host passes at least the bound of `getOutputSizeBound` for the call. A codec without a bound (decompression) that runs out of room returns `GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL` after taking in all of the input, with `*outputSize` what it wrote, and keeps the rest; the host gives more room and calls again without BEGIN and without input (FINAL again when it was set) until the status is OK. A codec with `GUCEF_CODEC_CAPABILITY_MESSAGE_ONLY` (AEAD) checks the room first and changes nothing when it is too small, so the host repeats the call with more room.

### 3.4 Rules

* Strings and links the plugin hands out are its own and stay valid until `Shutdown`.
* `encode`, `decode`, `getParameterInfo`, `createInstance` and `getOutputSizeBound` must be reentrant: the host calls them from several threads at once. One instance is used by one thread at a time.
* Published structs only grow at the end, and they start with `structSize`.
* Status values are never renumbered.
* Parameters come as a map with text keys; values are variants: text and numbers as `GUCEF_DATATYPE_*`, keys and nonces as `GUCEF_DATATYPE_BINARY_BLOB` (or the bytes of a string). Keys stay valid for the call only; an instance copies what it keeps.
* `GUCEF_CODEC_CAPABILITY_*`: `ENCODE`, `DECODE`, `INSTANCES` (the instance functions are there; streaming and incremental use), `DETERMINISTIC` (the same input and parameters give the same output), `MESSAGE_ONLY` (an instance call must have BEGIN and FINAL: AEAD).

## 4. Families, types and parameters

`CoreCodecTypes` names the families. Type names are lower case.

| family | types | parameters | Encode | Decode |
|---|---|---|---|---|
| `CompressionCodec` | `zlib`, `deflate` (`inflate`), `gzip` | `level` 0..9, default 9 | compresses | decompresses |
| `ChecksumCodec` | `crc-32`, `adler-32`, `crc32` | | 4 byte checksum | |
| `CryptographicHashCodec` | `sha256`, `sha512`, `md5`, `sha1`, `sha3`, `keccak`, `blake2b` | `output` = `hex` (default) or `raw`; `blake2b` also `length` (1..64, default 64) and `key` (keyed hash, at most 64 bytes) | digest | |
| `MacCodec` | `hmac-sha256` | `key` (required) | 32 byte MAC | |
| `KeyDerivationCodec` | `hkdf-sha256`, `hkdf-sha512`, `argon2i` | HKDF: `salt`, `info`, `length` (default the hash size); `argon2i`: `salt` (required, at least 8 bytes), `iterations` (3), `memory` in KiB (65536), `lanes` (1), `length` (32), `key`, `ad` | input = secret or password, output = derived key | |
| `EncryptionCodec` (AEAD) | `aes-256-gcm`, `chacha20-poly1305`, `xchacha20-poly1305` | `key` 32 bytes (required); one shot: `nonce` (12 bytes, 24 for xchacha), `aad` | ciphertext followed by the 16 byte tag | the plaintext, or AUTHENTICATION_FAILED |
| `KeyAgreementCodec` | `x25519` | Decode: `privateKey` | own private key → own public key | peer public key → shared secret |
| `SignatureCodec` | `ed25519` | Encode: `privateKey` (the 32 byte seed of RFC 8032), `output` = `signature` (default) or `publicKey`; Decode: `publicKey`, `signature` | message → signature, or (input ignored) the 32 byte public key | verifies the message, no output; AUTHENTICATION_FAILED when it does not match |

* For instances of an AEAD codec the key is a creation parameter and the nonce and AAD come per call in `SCodecCallArgs`. The caller makes sure a nonce is never used twice with one key.
* `GUCEF_CODEC_PROCESS_ENCODE_MESSAGE` on an AEAD instance writes ciphertext plus tag; `GetOverhead()` is the tag size, for MTU budgets.
* The `-raw` type names of `codecspluginSTBRUMMEHASH` (`sha1-raw` and so on) stay as aliases for `output=raw`.
* `x25519`: keys and shared secrets are 32 bytes. A shared secret of all zeros (the peer gave a point of low order) is refused with `GUCEF_CODEC_STATUS_INVALID_INPUT`. A raw shared secret goes through a KDF before use.
* `ed25519`: a private key is always the 32 byte seed of RFC 8032, never the 64 byte expanded form of some libraries.
* Families and types are open: anyone may register more.

## 5. Built in codecs

Registered by `CCoreGlobal` at start up through `CBuiltInCodecs` (`CCoreGlobal::GetBuiltInCodecs()`), and taken out at shut down.

### 5.1 Software (every platform), `SoftwareCodecPriority`

| family / type | class |
|---|---|
| `CryptographicHashCodec` / `sha256` | `CSha256Codec` |
| `MacCodec` / `hmac-sha256` | `CHmacSha256Codec` |
| `KeyDerivationCodec` / `hkdf-sha256` | `CHkdfSha256Codec` |

The algorithms are in `gucefCORE_CSha256.h`: `CSha256` (FIPS 180-4), `CHmacSha256` (RFC 2104), `CHkdfSha256` (RFC 5869) and `CSha256::ConstantTimeEquals`. They allocate nothing. The codecs derive from `CInstanceBasedCodec`, whose one shot `Encode` / `Decode` run through an instance.

### 5.2 OS providers, `OsCodecPriority`, switch `registerOsCryptoCodecs`

| platform | API | types |
|---|---|---|
| Linux | AF_ALG sockets (`src/linux`) | `aes-256-gcm` (`gcm(aes)`), `chacha20-poly1305` (`rfc7539(chacha20,poly1305)`), `hmac-sha256`, `sha256`, `sha512` |
| Windows | CNG / BCrypt (`src/mswin`) | `aes-256-gcm`, `x25519` (ECDH on curve25519, Windows 10 and later), `hmac-sha256`, `sha256`, `sha512` |
| macOS | CommonCrypto (`src/osx`) | `hmac-sha256`, `sha256`, `sha512` |

* Each algorithm is probed when the providers are created; one the kernel or OS lacks is simply not registered, and the software or plugin provider of that type stays in use.
* AF_ALG instances hold their sockets: a call is a `sendmsg` and a `recvmsg`, no allocation. An AEAD message is passed without waiting, so one larger than the socket buffer of the kernel (about 200 KB by default) is refused with `GUCEF_CODEC_STATUS_INVALID_INPUT` instead of blocking; packets are far below that, large one shot messages belong to a plugin.
* Config: `CBuiltInCodecs` is globally configurable: node `BuiltInCodecs`, attribute `registerOsCryptoCodecs` (default `true`). `SetRegisterOsCryptoCodecs( bool )` does the same at run time.

## 6. Plugins

| plugin | family / types | parameters |
|---|---|---|
| `codecspluginZLIB` | compression `zlib`, `deflate`, `gzip`; checksums `crc-32`, `adler-32` | `level`; instances for streaming compression and decompression |
| `codecspluginSTBRUMMEHASH` | hashes `md5`, `sha1`, `sha3`, `sha256`, `keccak` (+ `-raw`); checksum `crc32` | `output`; instances for incremental hashing |
| `imgpluginDEVIL`, `imgpluginFreeImage`, `imgpluginFLIC` | `ImageCodec` | |
| `codecspluginMONOCYPHER` (Monocypher 4.0.2, `dependencies/monocypher`, CC0 / BSD-2) | `xchacha20-poly1305`, `chacha20-poly1305`, `x25519`, `ed25519`, `blake2b`, `sha512`, `hkdf-sha512`, `argon2i` | instances for the AEADs (no allocation per call) and the hashes |
| `codecspluginOPENSSL` (Linux, the system libcrypto of OpenSSL 3) | `aes-256-gcm`, `chacha20-poly1305`, `x25519`, `ed25519`, `hkdf-sha256`, `hmac-sha256`, `sha256`, `sha512` | instances for the AEADs, the hashes and HMAC; an algorithm the library does not offer (a FIPS build) is left out; without the OpenSSL 3 development headers the plugin builds without codecs |

`CStdCodecPlugin` loads a plugin: the version handshake, `Init` with the metadata parameters, one `CStdCodecPluginItem` per link registered with its priority, `Shutdown` after the items are taken out of the registry. A plugin is not unloaded while one of its codecs or instances is still referenced elsewhere.

## 7. How to use

```cpp
CORE::CCodecRegistry& registry = CORE::CCoreGlobal::Instance()->GetCodecRegistry();
CORE::CCodecRegistry::TICodecPtr aead = registry.TryGetCodec( CORE::CoreCodecTypes::EncryptionCodec, "chacha20-poly1305" );

CORE::CValueList params;
params.Set( "key", CORE::CVariant( key, 32 ) );
CORE::CICodecInstancePtr cipher;
aead->CreateInstance( params, cipher );                     // once

CORE::SCodecCallArgs args;
memset( &args, 0, sizeof( args ) );
args.structSize = sizeof( args );
args.nonce = nonce; args.nonceSize = 12;
args.aad = header; args.aadSize = headerSize;
packet.Clear( true );
if ( !cipher->Process( GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, payload, payloadSize, &args, packet ) )   // per packet, no allocation
    ... cipher->GetLastStatus(), cipher->GetLastErrorMessage()
```

## 8. Decisions

* One breaking round of the C interface (2.0) with a version handshake, instead of added `Init2` / `Ex` exports: every implementer in the repository is updated.
* `pluginData` is passed to every link function, `getParameterInfo`, `createInstance` and `getOutputSizeBound` included, so a plugin can reach its own state (and the core API for logging) from all of them.
* Status values: OK is 0; a plugin of the interface before 2.0 (1 for success) never gets that far because the handshake refuses it.
* The link has a fixed `overhead` field, so `GetOverhead()` needs no extra call.
* `CICodec::GetLastStatus()` is kept per calling thread, because codecs are shared between threads.
* Instances append to their output buffer, so a caller can put a header in front without a copy.
* Hash codecs keep hex as their default output, as the existing `CryptographicHashCodec` types did; `output=raw` gives the digest bytes. MACs give raw bytes.
* The Windows provider leaves out `hkdf-sha256`: HKDF comes from the built in software codec on every platform.
* CNG `x25519` takes and gives keys in the little endian form of RFC 7748 (private scalar as `d` of a generic ECDH private blob, public key as `X`, the raw secret of `BCRYPT_KDF_RAW_SECRET`); the RFC 7748 tests of the CORE test app check that on Windows.
* Built in software codecs have the lowest priority, so a plugin can replace them without configuration.
* The BLAKE2b based KDF of the plan is `hkdf-sha512` instead: HKDF over BLAKE2b is not a standard construction, Monocypher brings RFC 5869 with SHA-512.
* Monocypher's AEAD stream interface changes its key after every message, so the plugin makes a fresh context per message from the key and the nonce of the call.
* AF_ALG runs an AEAD operation only for a read with room, so the decryption of a lone tag (no associated data, no plaintext) reads into a byte of room; without it the tag would not be checked at all.
* After a failed `sendmsg` / `recvmsg` an AF_ALG instance opens a new operation socket, so a refused message leaves nothing behind for the next one.
* A plugin codec without instances of its own gets the default instance, which holds the codec and so keeps the plugin linked.
* zlib gives no output bound: the host grows the room through the continuation rule (section 3.3). The hash plugin's instances allocate per digest, as its library returns text; the zero allocation promise is for the built in and OS codecs.
* The image codec plugin manager unlinks a plugin (Shutdown, codecs out of the registry) before its module is unloaded, otherwise the plugin object would call into the unloaded module later.

## 9. Not supported yet

* Emscripten: the browser's WebCrypto is asynchronous and cannot sit behind `Encode` / `Process`; the software codecs and plugins built into the program serve the web build.
* Instances shared between threads; choosing a provider per call other than by `GetCodecs`.
