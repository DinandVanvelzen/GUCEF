/*
 *  AssetConverterLib: conversion of asset packs into an engine-neutral intermediate and its cooking
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

#ifndef ASSETCONVERTERLIB_ETYPES_H
#define ASSETCONVERTERLIB_ETYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What a texture holds: it selects the colour space default and the format of a cook profile */
enum ETextureUsage
{
    TEXTUREUSAGE_COLOUR = 0,
    TEXTUREUSAGE_NORMAL    ,
    TEXTUREUSAGE_MASK      ,    /**< linear data: metal-roughness, occlusion, masks */
    TEXTUREUSAGE_HDR       ,

    TEXTUREUSAGE_COUNT
};
typedef enum ETextureUsage TTextureUsage;

/** The steps of the operation chain of a texture (CTextureOperation) */
enum ETextureOperationType
{
    TEXTUREOPERATION_EXTRACTCHANNEL = 0,    /**< one channel becomes a single channel picture */
    TEXTUREOPERATION_SWIZZLE           ,    /**< channels reordered, with constants: "GGG1" */
    TEXTUREOPERATION_INVERT            ,    /**< 1 - value for the channels of a mask: smoothness to roughness */
    TEXTUREOPERATION_SCALEBIAS         ,    /**< value * scale + bias for the channels of a mask */
    TEXTUREOPERATION_PACK              ,    /**< a picture of one channel of each of up to four sources of the texture */
    TEXTUREOPERATION_GREENFLIP         ,    /**< the green flip of DirectX normal maps */
    TEXTUREOPERATION_PREMULTIPLYALPHA  ,    /**< colour times alpha */
    TEXTUREOPERATION_UNPREMULTIPLYALPHA,    /**< colour divided by alpha */
    TEXTUREOPERATION_COLOURSPACETAG    ,    /**< names the colour space of the result, changes no texel */

    TEXTUREOPERATION_COUNT
};
typedef enum ETextureOperationType TTextureOperationType;

/** The alpha modes of glTF */
enum EAlphaMode
{
    ALPHAMODE_OPAQUE = 0,
    ALPHAMODE_MASK      ,
    ALPHAMODE_BLEND     ,

    ALPHAMODE_COUNT
};
typedef enum EAlphaMode TAlphaMode;

/** What an animation channel animates, as in glTF */
enum EAnimationPath
{
    ANIMATIONPATH_TRANSLATION = 0,
    ANIMATIONPATH_ROTATION       ,
    ANIMATIONPATH_SCALE          ,
    ANIMATIONPATH_WEIGHTS        ,

    ANIMATIONPATH_COUNT
};
typedef enum EAnimationPath TAnimationPath;

/** The interpolation of an animation channel, as in glTF */
enum EAnimationInterpolation
{
    ANIMATIONINTERPOLATION_LINEAR = 0,
    ANIMATIONINTERPOLATION_STEP      ,
    ANIMATIONINTERPOLATION_CUBICSPLINE,

    ANIMATIONINTERPOLATION_COUNT
};
typedef enum EAnimationInterpolation TAnimationInterpolation;

/** The collider shapes of OMI_physics_shape */
enum EColliderShape
{
    COLLIDERSHAPE_BOX = 0,
    COLLIDERSHAPE_SPHERE ,
    COLLIDERSHAPE_CAPSULE,
    COLLIDERSHAPE_CYLINDER,
    COLLIDERSHAPE_CONVEX ,
    COLLIDERSHAPE_TRIMESH,

    COLLIDERSHAPE_COUNT
};
typedef enum EColliderShape TColliderShape;

/** The light types of KHR_lights_punctual */
enum ELightType
{
    LIGHTTYPE_DIRECTIONAL = 0,
    LIGHTTYPE_POINT          ,
    LIGHTTYPE_SPOT           ,

    LIGHTTYPE_COUNT
};
typedef enum ELightType TLightType;

/** The green channel convention of normal maps: OpenGL (green up, the glTF convention) or DirectX (green down) */
enum ENormalMapConvention
{
    NORMALMAPCONVENTION_OPENGL = 0,
    NORMALMAPCONVENTION_DIRECTX   ,

    NORMALMAPCONVENTION_COUNT
};
typedef enum ENormalMapConvention TNormalMapConvention;

/** The kinds of the items of a conversion report; info, warning and error are also the severities of validators */
enum EReportItemKind
{
    REPORTITEM_INFO = 0     ,
    REPORTITEM_APPROXIMATION,   /**< converted, but not exactly: what and why */
    REPORTITEM_SKIPPED      ,   /**< not converted: what and why */
    REPORTITEM_WARNING      ,
    REPORTITEM_ERROR        ,

    REPORTITEM_COUNT
};
typedef enum EReportItemKind TReportItemKind;

/** The outcome of an asset in a run */
enum EAssetStatus
{
    ASSETSTATUS_CONVERTED = 0,
    ASSETSTATUS_APPROXIMATED ,  /**< converted with at least one approximation */
    ASSETSTATUS_SKIPPED      ,  /**< left out on purpose (an importer that declines it, a hint) */
    ASSETSTATUS_FAILED       ,  /**< an error stopped it */
    ASSETSTATUS_CACHED       ,  /**< unchanged since the last run: the files of that run stand */
    ASSETSTATUS_REFUSED      ,  /**< the licence record of the pack does not allow the conversion */

    ASSETSTATUS_COUNT
};
typedef enum EAssetStatus TAssetStatus;

/** The action of an entry of the import activity log */
enum EActivityAction
{
    ACTIVITYACTION_IMPORTED = 0,
    ACTIVITYACTION_CACHED      ,
    ACTIVITYACTION_FAILED      ,
    ACTIVITYACTION_REFUSED     ,
    ACTIVITYACTION_SKIPPED     ,  /**< not converted in this run (a run that was cancelled before it got to the asset) */

    ACTIVITYACTION_COUNT
};
typedef enum EActivityAction TActivityAction;

/** Why an asset was imported again or not: the first part of the cache key that differs */
enum EActivityReason
{
    ACTIVITYREASON_NEW = 0           ,  /**< no cache entry */
    ACTIVITYREASON_UNCHANGED         ,  /**< the cache key is the same */
    ACTIVITYREASON_SOURCE_CHANGED    ,
    ACTIVITYREASON_SETTINGS_CHANGED  ,
    ACTIVITYREASON_VERSION_CHANGED   ,  /**< a plugin of the stack changed version, or the stack itself changed (the detail names it) */
    ACTIVITYREASON_DEPENDENCY_CHANGED,  /**< the detail names the dependency */
    ACTIVITYREASON_OUTPUT_MISSING    ,  /**< a file of the last run is gone (the detail names it) */
    ACTIVITYREASON_FORCED            ,  /**< the cache was not used on request */
    ACTIVITYREASON_LICENCE           ,  /**< refused by the licence record (the detail says why) */
    ACTIVITYREASON_CANCELLED         ,  /**< the run was asked to stop before the asset was converted */
    ACTIVITYREASON_COOK_PENDING      ,  /**< the intermediate is up to date, the cooked files are not made yet (an interrupted run) */

    ACTIVITYREASON_COUNT
};
typedef enum EActivityReason TActivityReason;

/** What a run of the pipeline does (CAssetPipelineOptions::mode) */
enum EPipelineMode
{
    PIPELINEMODE_FULL = 0       ,   /**< convert, cook, write the cache, the activity log and the report */
    PIPELINEMODE_VALIDATE_ONLY  ,   /**< import, normalise, post-process, validate; nothing is written but the report when a report path is given */
    PIPELINEMODE_COOK_ONLY      ,   /**< cook what the intermediate holds (from the cache entries); refused without an intermediate */

    PIPELINEMODE_COUNT
};
typedef enum EPipelineMode TPipelineMode;

/** The phases of a run, for an observer (CIAssetPipelineObserver::OnPhase()) */
enum EPipelinePhase
{
    PIPELINEPHASE_LICENCE = 0   ,   /**< the licence record is checked */
    PIPELINEPHASE_INDEX         ,   /**< the importers index the pack */
    PIPELINEPHASE_CONVERT       ,   /**< the assets are converted (or validated) */
    PIPELINEPHASE_COOK          ,   /**< the cookers run */
    PIPELINEPHASE_REPORT        ,   /**< the cache entries, the activity log and the report are written */

    PIPELINEPHASE_COUNT
};
typedef enum EPipelinePhase TPipelinePhase;

/** What a run would do with an asset (CAssetPipeline::PlanPack()) */
enum EPlanAction
{
    PLANACTION_IMPORT = 0       ,   /**< imported and converted; the reason says why the cache does not hold it */
    PLANACTION_CACHED           ,   /**< the cache entry stands: the files of the last run are kept */
    PLANACTION_COOK             ,   /**< the intermediate stands, the cooking is still to be done */
    PLANACTION_REFUSED          ,   /**< the licence record does not allow the conversion */
    PLANACTION_FAILED           ,   /**< the asset cannot be converted (the detail says why: an unreadable source, a name clash) */
    PLANACTION_VALIDATE         ,   /**< validate-only: imported and validated, nothing written */
    PLANACTION_NO_INTERMEDIATE  ,   /**< cook-only: the asset has no intermediate to cook from */

    PLANACTION_COUNT
};
typedef enum EPlanAction TPlanAction;

/** How a particle value (CAssetParticleCurve) varies: one constant, random between two constants, a curve over the
 *  normalised time, random between two curves */
enum EParticleValueMode
{
    PARTICLEVALUE_CONSTANT = 0           ,
    PARTICLEVALUE_RANDOMBETWEENCONSTANTS ,
    PARTICLEVALUE_CURVE                  ,
    PARTICLEVALUE_RANDOMBETWEENCURVES    ,

    PARTICLEVALUE_COUNT
};
typedef enum EParticleValueMode TParticleValueMode;

/** How a particle colour (CAssetParticleGradient) varies */
enum EParticleColourMode
{
    PARTICLECOLOUR_COLOUR = 0              ,
    PARTICLECOLOUR_RANDOMBETWEENCOLOURS    ,
    PARTICLECOLOUR_GRADIENT                ,
    PARTICLECOLOUR_RANDOMBETWEENGRADIENTS  ,
    PARTICLECOLOUR_RANDOMFROMGRADIENT      ,

    PARTICLECOLOUR_COUNT
};
typedef enum EParticleColourMode TParticleColourMode;

/** The space particles move in: that of the node of the system (they move along with it) or the world */
enum EParticleSpace
{
    PARTICLESPACE_LOCAL = 0,
    PARTICLESPACE_WORLD    ,

    PARTICLESPACE_COUNT
};
typedef enum EParticleSpace TParticleSpace;

/** The shapes particles are emitted from; the canonical shape has its axis along +Y */
enum EParticleShape
{
    PARTICLESHAPE_POINT = 0 ,
    PARTICLESHAPE_SPHERE    ,
    PARTICLESHAPE_HEMISPHERE,
    PARTICLESHAPE_CONE      ,
    PARTICLESHAPE_BOX       ,
    PARTICLESHAPE_CIRCLE    ,
    PARTICLESHAPE_RING      ,
    PARTICLESHAPE_EDGE      ,
    PARTICLESHAPE_MESH      ,
    PARTICLESHAPE_POINTS    ,

    PARTICLESHAPE_COUNT
};
typedef enum EParticleShape TParticleShape;

/** The initial direction of particles: a fixed direction with a spread, or away from the centre of the shape */
enum EParticleDirectionMode
{
    PARTICLEDIRECTION_FIXED = 0 ,
    PARTICLEDIRECTION_OUTWARD   ,

    PARTICLEDIRECTION_COUNT
};
typedef enum EParticleDirectionMode TParticleDirectionMode;

/** How particles are drawn */
enum EParticleRenderMode
{
    PARTICLERENDER_BILLBOARD = 0,
    PARTICLERENDER_STRETCHED    ,
    PARTICLERENDER_HORIZONTAL   ,
    PARTICLERENDER_VERTICAL     ,
    PARTICLERENDER_ALIGNED      ,
    PARTICLERENDER_MESH         ,
    PARTICLERENDER_NONE         ,

    PARTICLERENDER_COUNT
};
typedef enum EParticleRenderMode TParticleRenderMode;

/** What a billboard faces: the view plane, the camera position, or it is stretched along the velocity */
enum EParticleAlignment
{
    PARTICLEALIGNMENT_VIEW = 0  ,
    PARTICLEALIGNMENT_FACING    ,
    PARTICLEALIGNMENT_VELOCITY  ,

    PARTICLEALIGNMENT_COUNT
};
typedef enum EParticleAlignment TParticleAlignment;

/** How particles are blended with what is behind them */
enum EParticleBlendMode
{
    PARTICLEBLEND_ALPHA = 0     ,
    PARTICLEBLEND_ADDITIVE      ,
    PARTICLEBLEND_PREMULTIPLIED ,
    PARTICLEBLEND_MULTIPLY      ,
    PARTICLEBLEND_SUBTRACT      ,
    PARTICLEBLEND_OPAQUE        ,

    PARTICLEBLEND_COUNT
};
typedef enum EParticleBlendMode TParticleBlendMode;

/** The draw order of particles */
enum EParticleSortMode
{
    PARTICLESORT_NONE = 0       ,
    PARTICLESORT_DISTANCE       ,
    PARTICLESORT_OLDESTFIRST    ,
    PARTICLESORT_YOUNGESTFIRST  ,

    PARTICLESORT_COUNT
};
typedef enum EParticleSortMode TParticleSortMode;

/** The event on which a sub-emitter starts */
enum EParticleTrigger
{
    PARTICLETRIGGER_BIRTH = 0   ,
    PARTICLETRIGGER_COLLISION   ,
    PARTICLETRIGGER_DEATH       ,
    PARTICLETRIGGER_TRIGGER     ,
    PARTICLETRIGGER_MANUAL      ,

    PARTICLETRIGGER_COUNT
};
typedef enum EParticleTrigger TParticleTrigger;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The names of the values: in settings files, reports and the sidecar. FromString ignores letter case and leaves the value alone for an unknown name */

inline const char* const* GetTextureUsageNames( void ) { static const char* const names[] = { "colour", "normal", "mask", "hdr" }; return names; }
inline const char* const* GetTextureOperationNames( void ) { static const char* const names[] = { "extractChannel", "swizzle", "invert", "scaleBias", "pack", "greenFlip", "premultiplyAlpha", "unpremultiplyAlpha", "colourSpaceTag" }; return names; }
inline const char* const* GetAlphaModeNames( void ) { static const char* const names[] = { "OPAQUE", "MASK", "BLEND" }; return names; }
inline const char* const* GetAnimationPathNames( void ) { static const char* const names[] = { "translation", "rotation", "scale", "weights" }; return names; }
inline const char* const* GetAnimationInterpolationNames( void ) { static const char* const names[] = { "LINEAR", "STEP", "CUBICSPLINE" }; return names; }
inline const char* const* GetColliderShapeNames( void ) { static const char* const names[] = { "box", "sphere", "capsule", "cylinder", "convex", "trimesh" }; return names; }
inline const char* const* GetLightTypeNames( void ) { static const char* const names[] = { "directional", "point", "spot" }; return names; }
inline const char* const* GetNormalMapConventionNames( void ) { static const char* const names[] = { "opengl", "directx" }; return names; }
inline const char* const* GetReportItemKindNames( void ) { static const char* const names[] = { "info", "approximation", "skipped", "warning", "error" }; return names; }
inline const char* const* GetAssetStatusNames( void ) { static const char* const names[] = { "converted", "approximated", "skipped", "failed", "cached", "refused" }; return names; }
inline const char* const* GetActivityActionNames( void ) { static const char* const names[] = { "imported", "cached", "failed", "refused", "skipped" }; return names; }
inline const char* const* GetActivityReasonNames( void ) { static const char* const names[] = { "new", "unchanged", "source changed", "settings changed", "version changed", "dependency changed", "output missing", "forced", "licence", "cancelled", "cook pending" }; return names; }
inline const char* const* GetPipelineModeNames( void ) { static const char* const names[] = { "full", "validateOnly", "cookOnly" }; return names; }
inline const char* const* GetPipelinePhaseNames( void ) { static const char* const names[] = { "licence", "index", "convert", "cook", "report" }; return names; }
inline const char* const* GetPlanActionNames( void ) { static const char* const names[] = { "import", "cached", "cook", "refused", "failed", "validate", "no intermediate" }; return names; }
inline const char* const* GetParticleValueModeNames( void ) { static const char* const names[] = { "constant", "randomBetweenConstants", "curve", "randomBetweenCurves" }; return names; }
inline const char* const* GetParticleColourModeNames( void ) { static const char* const names[] = { "colour", "randomBetweenColours", "gradient", "randomBetweenGradients", "randomFromGradient" }; return names; }
inline const char* const* GetParticleSpaceNames( void ) { static const char* const names[] = { "local", "world" }; return names; }
inline const char* const* GetParticleShapeNames( void ) { static const char* const names[] = { "point", "sphere", "hemisphere", "cone", "box", "circle", "ring", "edge", "mesh", "points" }; return names; }
inline const char* const* GetParticleDirectionModeNames( void ) { static const char* const names[] = { "fixed", "outward" }; return names; }
inline const char* const* GetParticleRenderModeNames( void ) { static const char* const names[] = { "billboard", "stretched", "horizontal", "vertical", "aligned", "mesh", "none" }; return names; }
inline const char* const* GetParticleAlignmentNames( void ) { static const char* const names[] = { "view", "facing", "velocity" }; return names; }
inline const char* const* GetParticleBlendModeNames( void ) { static const char* const names[] = { "alpha", "additive", "premultiplied", "multiply", "subtract", "opaque" }; return names; }
inline const char* const* GetParticleSortModeNames( void ) { static const char* const names[] = { "none", "distance", "oldestFirst", "youngestFirst" }; return names; }
inline const char* const* GetParticleTriggerNames( void ) { static const char* const names[] = { "birth", "collision", "death", "trigger", "manual" }; return names; }

/** The index of a name in a table of 'count' names, ignoring letter case; -1 when it is not there */
inline Int32
FindNameInTable( const CORE::CString& name, const char* const* names, Int32 count )
{
    const CORE::CString lower = name.Lowercase();
    for ( Int32 i=0; i<count; ++i )
        if ( lower == CORE::CString( names[ i ] ).Lowercase() )
            return i;
    return -1;
}

inline CORE::CString ToString( TTextureUsage value ) { return value < TEXTUREUSAGE_COUNT ? GetTextureUsageNames()[ value ] : ""; }
inline CORE::CString ToString( TTextureOperationType value ) { return value < TEXTUREOPERATION_COUNT ? GetTextureOperationNames()[ value ] : ""; }
inline CORE::CString ToString( TAlphaMode value ) { return value < ALPHAMODE_COUNT ? GetAlphaModeNames()[ value ] : ""; }
inline CORE::CString ToString( TAnimationPath value ) { return value < ANIMATIONPATH_COUNT ? GetAnimationPathNames()[ value ] : ""; }
inline CORE::CString ToString( TAnimationInterpolation value ) { return value < ANIMATIONINTERPOLATION_COUNT ? GetAnimationInterpolationNames()[ value ] : ""; }
inline CORE::CString ToString( TColliderShape value ) { return value < COLLIDERSHAPE_COUNT ? GetColliderShapeNames()[ value ] : ""; }
inline CORE::CString ToString( TLightType value ) { return value < LIGHTTYPE_COUNT ? GetLightTypeNames()[ value ] : ""; }
inline CORE::CString ToString( TNormalMapConvention value ) { return value < NORMALMAPCONVENTION_COUNT ? GetNormalMapConventionNames()[ value ] : ""; }
inline CORE::CString ToString( TReportItemKind value ) { return value < REPORTITEM_COUNT ? GetReportItemKindNames()[ value ] : ""; }
inline CORE::CString ToString( TAssetStatus value ) { return value < ASSETSTATUS_COUNT ? GetAssetStatusNames()[ value ] : ""; }
inline CORE::CString ToString( TActivityAction value ) { return value < ACTIVITYACTION_COUNT ? GetActivityActionNames()[ value ] : ""; }
inline CORE::CString ToString( TActivityReason value ) { return value < ACTIVITYREASON_COUNT ? GetActivityReasonNames()[ value ] : ""; }
inline CORE::CString ToString( TPipelineMode value ) { return value < PIPELINEMODE_COUNT ? GetPipelineModeNames()[ value ] : ""; }
inline CORE::CString ToString( TPipelinePhase value ) { return value < PIPELINEPHASE_COUNT ? GetPipelinePhaseNames()[ value ] : ""; }
inline CORE::CString ToString( TPlanAction value ) { return value < PLANACTION_COUNT ? GetPlanActionNames()[ value ] : ""; }
inline CORE::CString ToString( TParticleValueMode value ) { return value < PARTICLEVALUE_COUNT ? GetParticleValueModeNames()[ value ] : ""; }
inline CORE::CString ToString( TParticleColourMode value ) { return value < PARTICLECOLOUR_COUNT ? GetParticleColourModeNames()[ value ] : ""; }
inline CORE::CString ToString( TParticleSpace value ) { return value < PARTICLESPACE_COUNT ? GetParticleSpaceNames()[ value ] : ""; }
inline CORE::CString ToString( TParticleShape value ) { return value < PARTICLESHAPE_COUNT ? GetParticleShapeNames()[ value ] : ""; }
inline CORE::CString ToString( TParticleDirectionMode value ) { return value < PARTICLEDIRECTION_COUNT ? GetParticleDirectionModeNames()[ value ] : ""; }
inline CORE::CString ToString( TParticleRenderMode value ) { return value < PARTICLERENDER_COUNT ? GetParticleRenderModeNames()[ value ] : ""; }
inline CORE::CString ToString( TParticleAlignment value ) { return value < PARTICLEALIGNMENT_COUNT ? GetParticleAlignmentNames()[ value ] : ""; }
inline CORE::CString ToString( TParticleBlendMode value ) { return value < PARTICLEBLEND_COUNT ? GetParticleBlendModeNames()[ value ] : ""; }
inline CORE::CString ToString( TParticleSortMode value ) { return value < PARTICLESORT_COUNT ? GetParticleSortModeNames()[ value ] : ""; }
inline CORE::CString ToString( TParticleTrigger value ) { return value < PARTICLETRIGGER_COUNT ? GetParticleTriggerNames()[ value ] : ""; }

inline bool FromString( const CORE::CString& name, TTextureUsage& value ) { Int32 i = FindNameInTable( name, GetTextureUsageNames(), TEXTUREUSAGE_COUNT ); if ( i < 0 ) return false; value = (TTextureUsage) i; return true; }
inline bool FromString( const CORE::CString& name, TTextureOperationType& value ) { Int32 i = FindNameInTable( name, GetTextureOperationNames(), TEXTUREOPERATION_COUNT ); if ( i < 0 ) return false; value = (TTextureOperationType) i; return true; }
inline bool FromString( const CORE::CString& name, TAlphaMode& value ) { Int32 i = FindNameInTable( name, GetAlphaModeNames(), ALPHAMODE_COUNT ); if ( i < 0 ) return false; value = (TAlphaMode) i; return true; }
inline bool FromString( const CORE::CString& name, TAnimationPath& value ) { Int32 i = FindNameInTable( name, GetAnimationPathNames(), ANIMATIONPATH_COUNT ); if ( i < 0 ) return false; value = (TAnimationPath) i; return true; }
inline bool FromString( const CORE::CString& name, TAnimationInterpolation& value ) { Int32 i = FindNameInTable( name, GetAnimationInterpolationNames(), ANIMATIONINTERPOLATION_COUNT ); if ( i < 0 ) return false; value = (TAnimationInterpolation) i; return true; }
inline bool FromString( const CORE::CString& name, TColliderShape& value ) { Int32 i = FindNameInTable( name, GetColliderShapeNames(), COLLIDERSHAPE_COUNT ); if ( i < 0 ) return false; value = (TColliderShape) i; return true; }
inline bool FromString( const CORE::CString& name, TLightType& value ) { Int32 i = FindNameInTable( name, GetLightTypeNames(), LIGHTTYPE_COUNT ); if ( i < 0 ) return false; value = (TLightType) i; return true; }
inline bool FromString( const CORE::CString& name, TNormalMapConvention& value ) { Int32 i = FindNameInTable( name, GetNormalMapConventionNames(), NORMALMAPCONVENTION_COUNT ); if ( i < 0 ) return false; value = (TNormalMapConvention) i; return true; }
inline bool FromString( const CORE::CString& name, TReportItemKind& value ) { Int32 i = FindNameInTable( name, GetReportItemKindNames(), REPORTITEM_COUNT ); if ( i < 0 ) return false; value = (TReportItemKind) i; return true; }
inline bool FromString( const CORE::CString& name, TAssetStatus& value ) { Int32 i = FindNameInTable( name, GetAssetStatusNames(), ASSETSTATUS_COUNT ); if ( i < 0 ) return false; value = (TAssetStatus) i; return true; }
inline bool FromString( const CORE::CString& name, TActivityAction& value ) { Int32 i = FindNameInTable( name, GetActivityActionNames(), ACTIVITYACTION_COUNT ); if ( i < 0 ) return false; value = (TActivityAction) i; return true; }
inline bool FromString( const CORE::CString& name, TActivityReason& value ) { Int32 i = FindNameInTable( name, GetActivityReasonNames(), ACTIVITYREASON_COUNT ); if ( i < 0 ) return false; value = (TActivityReason) i; return true; }
inline bool FromString( const CORE::CString& name, TPipelineMode& value ) { Int32 i = FindNameInTable( name, GetPipelineModeNames(), PIPELINEMODE_COUNT ); if ( i < 0 ) return false; value = (TPipelineMode) i; return true; }
inline bool FromString( const CORE::CString& name, TPipelinePhase& value ) { Int32 i = FindNameInTable( name, GetPipelinePhaseNames(), PIPELINEPHASE_COUNT ); if ( i < 0 ) return false; value = (TPipelinePhase) i; return true; }
inline bool FromString( const CORE::CString& name, TPlanAction& value ) { Int32 i = FindNameInTable( name, GetPlanActionNames(), PLANACTION_COUNT ); if ( i < 0 ) return false; value = (TPlanAction) i; return true; }
inline bool FromString( const CORE::CString& name, TParticleValueMode& value ) { Int32 i = FindNameInTable( name, GetParticleValueModeNames(), PARTICLEVALUE_COUNT ); if ( i < 0 ) return false; value = (TParticleValueMode) i; return true; }
inline bool FromString( const CORE::CString& name, TParticleColourMode& value ) { Int32 i = FindNameInTable( name, GetParticleColourModeNames(), PARTICLECOLOUR_COUNT ); if ( i < 0 ) return false; value = (TParticleColourMode) i; return true; }
inline bool FromString( const CORE::CString& name, TParticleSpace& value ) { Int32 i = FindNameInTable( name, GetParticleSpaceNames(), PARTICLESPACE_COUNT ); if ( i < 0 ) return false; value = (TParticleSpace) i; return true; }
inline bool FromString( const CORE::CString& name, TParticleShape& value ) { Int32 i = FindNameInTable( name, GetParticleShapeNames(), PARTICLESHAPE_COUNT ); if ( i < 0 ) return false; value = (TParticleShape) i; return true; }
inline bool FromString( const CORE::CString& name, TParticleDirectionMode& value ) { Int32 i = FindNameInTable( name, GetParticleDirectionModeNames(), PARTICLEDIRECTION_COUNT ); if ( i < 0 ) return false; value = (TParticleDirectionMode) i; return true; }
inline bool FromString( const CORE::CString& name, TParticleRenderMode& value ) { Int32 i = FindNameInTable( name, GetParticleRenderModeNames(), PARTICLERENDER_COUNT ); if ( i < 0 ) return false; value = (TParticleRenderMode) i; return true; }
inline bool FromString( const CORE::CString& name, TParticleAlignment& value ) { Int32 i = FindNameInTable( name, GetParticleAlignmentNames(), PARTICLEALIGNMENT_COUNT ); if ( i < 0 ) return false; value = (TParticleAlignment) i; return true; }
inline bool FromString( const CORE::CString& name, TParticleBlendMode& value ) { Int32 i = FindNameInTable( name, GetParticleBlendModeNames(), PARTICLEBLEND_COUNT ); if ( i < 0 ) return false; value = (TParticleBlendMode) i; return true; }
inline bool FromString( const CORE::CString& name, TParticleSortMode& value ) { Int32 i = FindNameInTable( name, GetParticleSortModeNames(), PARTICLESORT_COUNT ); if ( i < 0 ) return false; value = (TParticleSortMode) i; return true; }
inline bool FromString( const CORE::CString& name, TParticleTrigger& value ) { Int32 i = FindNameInTable( name, GetParticleTriggerNames(), PARTICLETRIGGER_COUNT ); if ( i < 0 ) return false; value = (TParticleTrigger) i; return true; }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETCONVERTERLIB_ETYPES_H ? */
