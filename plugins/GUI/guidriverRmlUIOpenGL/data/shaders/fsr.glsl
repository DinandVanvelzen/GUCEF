// AMD FidelityFX Super Resolution 1.0: EASU (edge adaptive spatial upsampling) and RCAS (robust contrast adaptive
// sharpening) on RGB pictures.
//
// Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved. MIT license, see LICENSE-FidelityFX-FSR.txt.
// Ported to GLSL in https://www.shadertoy.com/view/stXSWB (MIT license), adapted for libretro glsl-shaders.
//
// In the mpv user shader format: EASU upscales the picture to the size of the output when that is larger,
// RCAS sharpens the result. The parameter 'sharpness' goes from 0 (RCAS sharpens 2 stops less) to 1 (the
// sharpest RCAS does).

//!PARAM sharpness
//!DESC How sharp the result is, 0 .. 1
//!TYPE float
//!MINIMUM 0.0
//!MAXIMUM 1.0
0.5

//!DESC FidelityFX FSR 1 EASU
//!HOOK MAIN
//!BIND HOOKED
//!WIDTH OUTPUT.w
//!HEIGHT OUTPUT.h
//!WHEN OUTPUT.w HOOKED.w > OUTPUT.h HOOKED.h > +

vec3 FsrEasuCF( vec2 p )
{
    return HOOKED_tex( p ).rgb;
}

void FsrEasuCon( out vec4 con0, out vec4 con1, out vec4 con2, out vec4 con3,
                 vec2 inputViewportInPixels, vec2 inputSizeInPixels, vec2 outputSizeInPixels )
{
    // output integer position to a pixel position in the viewport
    con0 = vec4( inputViewportInPixels.x / outputSizeInPixels.x,
                 inputViewportInPixels.y / outputSizeInPixels.y,
                 0.5 * inputViewportInPixels.x / outputSizeInPixels.x - 0.5,
                 0.5 * inputViewportInPixels.y / outputSizeInPixels.y - 0.5 );
    // viewport pixel position to normalized image space, used to get the upper left of the 'F' tap
    con1 = vec4( 1.0, 1.0, 1.0, -1.0 ) / inputSizeInPixels.xyxy;
    // centres of gather4, first offset from the upper left of 'F'
    con2 = vec4( -1.0, 2.0, 1.0, 2.0 ) / inputSizeInPixels.xyxy;
    con3 = vec4( 0.0, 4.0, 0.0, 0.0 ) / inputSizeInPixels.xyxy;
}

// filtering for a given tap for the scalar
void FsrEasuTapF( inout vec3 aC, inout float aW, vec2 off, vec2 dir, vec2 len, float lob, float clp, vec3 c )
{
    // rotate the offset by the direction, anisotropy
    vec2 v = vec2( dot( off, dir ), dot( off, vec2( -dir.y, dir.x ) ) );
    v *= len;
    // distance^2, limited to the window
    float d2 = min( dot( v, v ), clp );
    // approximation of lanczos2 without sin() or rcp(), or sqrt() to get x
    float wB = 0.4 * d2 - 1.0;
    float wA = lob * d2 - 1.0;
    wB *= wB;
    wA *= wA;
    wB = 1.5625 * wB - 0.5625;
    float w = wB * wA;
    aC += c * w;
    aW += w;
}

// accumulate direction and length
void FsrEasuSetF( inout vec2 dir, inout float len, float w, float lA, float lB, float lC, float lD, float lE )
{
    float lenX = max( abs( lD - lC ), abs( lC - lB ) );
    float dirX = lD - lB;
    dir.x += dirX * w;
    lenX = clamp( abs( dirX ) / max( lenX, 1.0 / 65536.0 ), 0.0, 1.0 );    // flat: lenX and dirX are 0
    lenX *= lenX;
    len += lenX * w;

    float lenY = max( abs( lE - lC ), abs( lC - lA ) );
    float dirY = lE - lA;
    dir.y += dirY * w;
    lenY = clamp( abs( dirY ) / max( lenY, 1.0 / 65536.0 ), 0.0, 1.0 );
    lenY *= lenY;
    len += lenY * w;
}

void FsrEasuF( out vec3 pix, vec2 ip, vec4 con0, vec4 con1, vec4 con2, vec4 con3 )
{
    // the position of 'f'
    vec2 pp = ip * con0.xy + con0.zw;
    vec2 fp = floor( pp );
    pp -= fp;

    // 12 tap kernel
    //    b c
    //  e f g h
    //  i j k l
    //    n o
    vec2 p0 = fp * con1.xy + con1.zw;
    vec2 p1 = p0 + con2.xy;
    vec2 p2 = p0 + con2.zw;
    vec2 p3 = p0 + con3.xy;
    vec4 off = vec4( -0.5, 0.5, -0.5, 0.5 ) * con1.xxyy;
    vec3 bC = FsrEasuCF( p0 + off.xw ); float bL = bC.g + 0.5 * ( bC.r + bC.b );
    vec3 cC = FsrEasuCF( p0 + off.yw ); float cL = cC.g + 0.5 * ( cC.r + cC.b );
    vec3 iC = FsrEasuCF( p1 + off.xw ); float iL = iC.g + 0.5 * ( iC.r + iC.b );
    vec3 jC = FsrEasuCF( p1 + off.yw ); float jL = jC.g + 0.5 * ( jC.r + jC.b );
    vec3 fC = FsrEasuCF( p1 + off.yz ); float fL = fC.g + 0.5 * ( fC.r + fC.b );
    vec3 eC = FsrEasuCF( p1 + off.xz ); float eL = eC.g + 0.5 * ( eC.r + eC.b );
    vec3 kC = FsrEasuCF( p2 + off.xw ); float kL = kC.g + 0.5 * ( kC.r + kC.b );
    vec3 lC = FsrEasuCF( p2 + off.yw ); float lL = lC.g + 0.5 * ( lC.r + lC.b );
    vec3 hC = FsrEasuCF( p2 + off.yz ); float hL = hC.g + 0.5 * ( hC.r + hC.b );
    vec3 gC = FsrEasuCF( p2 + off.xz ); float gL = gC.g + 0.5 * ( gC.r + gC.b );
    vec3 oC = FsrEasuCF( p3 + off.yz ); float oL = oC.g + 0.5 * ( oC.r + oC.b );
    vec3 nC = FsrEasuCF( p3 + off.xz ); float nL = nC.g + 0.5 * ( nC.r + nC.b );

    // accumulate for bilinear interpolation
    vec2 dir = vec2( 0.0 );
    float len = 0.0;
    FsrEasuSetF( dir, len, ( 1.0 - pp.x ) * ( 1.0 - pp.y ), bL, eL, fL, gL, jL );
    FsrEasuSetF( dir, len, pp.x * ( 1.0 - pp.y ), cL, fL, gL, hL, kL );
    FsrEasuSetF( dir, len, ( 1.0 - pp.x ) * pp.y, fL, iL, jL, kL, nL );
    FsrEasuSetF( dir, len, pp.x * pp.y, gL, jL, kL, lL, oL );

    // normalize with approximation, and cleanup close to zero
    vec2 dir2 = dir * dir;
    float dirR = dir2.x + dir2.y;
    bool zro = dirR < ( 1.0 / 32768.0 );
    dirR = inversesqrt( dirR );
    dirR = zro ? 1.0 : dirR;
    dir.x = zro ? 1.0 : dir.x;
    dir *= vec2( dirR );
    // transform from {0 to 2} to {0 to 1} range, and shape with square
    len = len * 0.5;
    len *= len;
    // stretch kernel {1.0 vert|horz, to sqrt(2.0) on diagonal}
    float stretch = dot( dir, dir ) / ( max( abs( dir.x ), abs( dir.y ) ) );
    vec2 len2 = vec2( 1.0 + ( stretch - 1.0 ) * len, 1.0 - 0.5 * len );
    float lob = 0.5 - 0.29 * len;
    float clp = 1.0 / lob;

    // accumulation mixed with min/max of the 4 nearest
    vec3 min4 = min( min( fC, gC ), min( jC, kC ) );
    vec3 max4 = max( max( fC, gC ), max( jC, kC ) );
    vec3 aC = vec3( 0.0 );
    float aW = 0.0;
    FsrEasuTapF( aC, aW, vec2( 0.0, -1.0 ) - pp, dir, len2, lob, clp, bC );
    FsrEasuTapF( aC, aW, vec2( 1.0, -1.0 ) - pp, dir, len2, lob, clp, cC );
    FsrEasuTapF( aC, aW, vec2( -1.0, 1.0 ) - pp, dir, len2, lob, clp, iC );
    FsrEasuTapF( aC, aW, vec2( 0.0, 1.0 ) - pp, dir, len2, lob, clp, jC );
    FsrEasuTapF( aC, aW, vec2( 0.0, 0.0 ) - pp, dir, len2, lob, clp, fC );
    FsrEasuTapF( aC, aW, vec2( -1.0, 0.0 ) - pp, dir, len2, lob, clp, eC );
    FsrEasuTapF( aC, aW, vec2( 1.0, 1.0 ) - pp, dir, len2, lob, clp, kC );
    FsrEasuTapF( aC, aW, vec2( 2.0, 1.0 ) - pp, dir, len2, lob, clp, lC );
    FsrEasuTapF( aC, aW, vec2( 2.0, 0.0 ) - pp, dir, len2, lob, clp, hC );
    FsrEasuTapF( aC, aW, vec2( 1.0, 0.0 ) - pp, dir, len2, lob, clp, gC );
    FsrEasuTapF( aC, aW, vec2( 1.0, 2.0 ) - pp, dir, len2, lob, clp, oC );
    FsrEasuTapF( aC, aW, vec2( 0.0, 2.0 ) - pp, dir, len2, lob, clp, nC );

    // normalize and dering
    pix = min( max4, max( min4, aC / aW ) );
}

vec4 hook()
{
    vec4 con0, con1, con2, con3;
    FsrEasuCon( con0, con1, con2, con3, HOOKED_size, HOOKED_size, target_size );
    vec3 c;
    FsrEasuF( c, floor( HOOKED_pos * target_size ), con0, con1, con2, con3 );
    return vec4( c, HOOKED_tex( HOOKED_pos ).a );
}

//!DESC FidelityFX FSR 1 RCAS
//!HOOK MAIN
//!BIND HOOKED

#define FSR_RCAS_LIMIT ( 0.25 - ( 1.0 / 16.0 ) )

vec4 FsrRcasLoadF( vec2 ip )
{
    return HOOKED_tex( ( ip + 0.5 ) * HOOKED_pt );
}

vec3 FsrRcasF( vec2 ip, float con )
{
    // minimal 3x3 neighbourhood
    //    b
    //  d e f
    //    h
    vec3 b = FsrRcasLoadF( ip + vec2( 0.0, -1.0 ) ).rgb;
    vec3 d = FsrRcasLoadF( ip + vec2( -1.0, 0.0 ) ).rgb;
    vec3 e = FsrRcasLoadF( ip ).rgb;
    vec3 f = FsrRcasLoadF( ip + vec2( 1.0, 0.0 ) ).rgb;
    vec3 h = FsrRcasLoadF( ip + vec2( 0.0, 1.0 ) ).rgb;

    // min and max of the ring
    vec3 mn4 = min( b, min( f, h ) );
    vec3 mx4 = max( b, max( f, h ) );
    // immediate constants for the peak range
    vec2 peakC = vec2( 1.0, -4.0 );
    // limiters; the denominators are kept from 0 (black and white rings) as the approximate reciprocals of the
    // original do, a division by 0 would give NaN here
    vec3 hitMin = mn4 / max( 4.0 * mx4, vec3( 1.0 / 65536.0 ) );
    vec3 hitMax = ( peakC.x - mx4 ) / min( 4.0 * mn4 + peakC.y, vec3( -1.0 / 65536.0 ) );
    vec3 lobeRGB = max( -hitMin, hitMax );
    float lobe = max( -FSR_RCAS_LIMIT, min( max( lobeRGB.r, max( lobeRGB.g, lobeRGB.b ) ), 0.0 ) ) * con;
    return ( lobe * ( b + d + h + f ) + e ) / ( 4.0 * lobe + 1.0 );
}

vec4 hook()
{
    // sharpness in stops: 0 is the sharpest
    float con = exp2( -( 1.0 - clamp( sharpness, 0.0, 1.0 ) ) * 2.0 );
    vec2 ip = floor( HOOKED_pos * HOOKED_size );
    return vec4( FsrRcasF( ip, con ), HOOKED_tex( HOOKED_pos ).a );
}
