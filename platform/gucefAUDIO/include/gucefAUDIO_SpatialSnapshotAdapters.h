/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_SPATIALSNAPSHOTADAPTERS_H
#define GUCEF_AUDIO_SPATIALSNAPSHOTADAPTERS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

#ifndef GUCEF_AUDIO_CAUDIOLISTENER_H
#include "gucefAUDIO_CAudioListener.h"
#define GUCEF_AUDIO_CAUDIOLISTENER_H
#endif /* GUCEF_AUDIO_CAUDIOLISTENER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOEMITTER_H
#include "gucefAUDIO_CAudioEmitter.h"
#define GUCEF_AUDIO_CAUDIOEMITTER_H
#endif /* GUCEF_AUDIO_CAUDIOEMITTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Adapters that drive the listener and the emitters from the placement of a node of a transform tree, without gucefAUDIO
 *  depending on the module that owns the tree (section 7.3.6 and 7.11 of the gucefMATH / gucefGEOMETRY design). The
 *  game captures a plain snapshot of the node on the thread that owns the tree (gucefGEOMETRY CTGeoSpace::CaptureSnapshot with
 *  CCoordinateSystem::GucefDefault(), whose axes are the ones of the audio design 5.1: +Y up, -Z forward) and hands
 *  that one value to these functions on the same thread; the listener or the emitter publishes it to the audio thread
 *  through its own latest-only exchange, so the audio thread neither locks nor allocates. A snapshot SET is not for
 *  this route: it owns a vector and allocates on the first copies through an exchange, which is why the listener and
 *  every emitter take one single snapshot.
 *
 *  The functions are templates over any type that has the members of a gucefGEOMETRY CTGeoSpaceSnapshot ( worldPosition
 *  with x, y and z; forward and up that convert with AsVector(); velocity with x, y and z ), so Float32 and Float64
 *  trees work and the module does not include a geometry header. Narrowing to Float32 happens here, once per call. A
 *  Float64 tree that is large should capture camera-relative snapshots (CaptureSnapshotRelativeTo) so that the
 *  narrowing does not lose precision near the listener.
 */

namespace SpatialSnapshotAdapterDetail {

template < typename T >
inline Float32
Narrow( const T& value )
{
    return MATH::CTNumericTraits< T >::ToFloat32( value );
}

template < typename TVectorLike >
inline CAudioSpatialMath::TVector3
ToVector3( const TVectorLike& v )
{
    return CAudioSpatialMath::TVector3( Narrow( v.x ), Narrow( v.y ), Narrow( v.z ) );
}

template < typename TPointLike >
inline CAudioSpatialMath::TPoint3
ToPoint3( const TPointLike& p )
{
    return CAudioSpatialMath::TPoint3( Narrow( p.x ), Narrow( p.y ), Narrow( p.z ) );
}

}

/*-------------------------------------------------------------------------*/

/**
 *  Fills position, forward, up and velocity of 'state' from a snapshot; the 2D pan width is left alone. Nothing here
 *  validates: CAudioListener::SetState refuses a state that is not finite or has a degenerate orientation.
 */
template < typename TSnapshot >
inline void
ListenerStateFromSnapshot( const TSnapshot& snapshot, SListenerState& state )
{
    state.position = SpatialSnapshotAdapterDetail::ToPoint3( snapshot.worldPosition );
    state.forward = SpatialSnapshotAdapterDetail::ToVector3( snapshot.forward.AsVector() );
    state.up = SpatialSnapshotAdapterDetail::ToVector3( snapshot.up.AsVector() );
    state.velocity = SpatialSnapshotAdapterDetail::ToVector3( snapshot.velocity );
}

/*-------------------------------------------------------------------------*/

/**
 *  Moves the listener to the placement of a node, for example a camera: one update, so position, orientation and velocity
 *  are always heard from the same capture, and the 2D pan width stays as it was. Call on the thread that captured the
 *  snapshot (the owner of the tree); false when the listener refuses the state.
 */
template < typename TSnapshot >
inline bool
SetListenerFromSnapshot( CAudioListener& listener, const TSnapshot& snapshot )
{
    SListenerState state;
    listener.GetState( state );
    ListenerStateFromSnapshot( snapshot, state );
    return listener.SetState( state );
}

/*-------------------------------------------------------------------------*/

/**
 *  Moves an emitter to the placement of a node in one update: position and velocity and, when 'useForwardAsDirection' is
 *  set, the cone axis along the forward axis of the node (an emitter that points where its object points). The velocity is
 *  the one of the snapshot (or the derived one when the emitter derives it, see CAudioEmitter::SetDeriveVelocity).
 *  False when the emitter refuses a value.
 */
template < typename TSnapshot >
inline bool
SetEmitterFromSnapshot( CAudioEmitter& emitter, const TSnapshot& snapshot, bool useForwardAsDirection = false )
{
    if ( useForwardAsDirection )
        return emitter.SetPlacement( SpatialSnapshotAdapterDetail::ToPoint3( snapshot.worldPosition ), SpatialSnapshotAdapterDetail::ToVector3( snapshot.velocity ),
                                     SpatialSnapshotAdapterDetail::ToVector3( snapshot.forward.AsVector() ) );
    return emitter.SetPositionAndVelocity( SpatialSnapshotAdapterDetail::ToPoint3( snapshot.worldPosition ), SpatialSnapshotAdapterDetail::ToVector3( snapshot.velocity ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Gives an emitter the occlusion and the obstruction of an estimate made on the owner thread (gucefGEOMETRY
 *  SAudioOcclusionEstimate or any type with 'occlusion' and 'obstruction' members): the emitter publishes them to the audio
 *  thread. Values outside 0 .. 1 are limited by the emitter; smoothing over time is the caller's choice. False when the
 *  emitter refuses a value.
 */
template < typename TEstimate >
inline bool
SetEmitterOcclusionFromEstimate( CAudioEmitter& emitter, const TEstimate& estimate )
{
    return emitter.SetOcclusionAndObstruction( SpatialSnapshotAdapterDetail::Narrow( estimate.occlusion ), SpatialSnapshotAdapterDetail::Narrow( estimate.obstruction ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_SPATIALSNAPSHOTADAPTERS_H ? */
