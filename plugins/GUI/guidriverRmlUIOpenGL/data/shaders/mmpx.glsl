// MMPX
// by Morgan McGuire and Mara Gagiu
// https://casual-effects.com/research/McGuire2021PixelArt/
// License: MIT
// adapted for glsl by hunterk (libretro glsl-shaders, MIT)
//
// In the mpv user shader format: the pixels around the output pixel are read at the centres of the texels, so
// the filter works with linear sampling, and colours are compared with their alpha, so it works on pictures with
// transparency. The picture is magnified 2 times; the present pass scales the result to the window.

//!DESC MMPX 2x pixel art magnification
//!HOOK MAIN
//!BIND HOOKED
//!WIDTH HOOKED.w 2 *
//!HEIGHT HOOKED.h 2 *

float luma( vec4 col )
{
    return dot( col.rgb, vec3( 0.2126, 0.7152, 0.0722 ) );
}

bool same( vec4 B, vec4 A0 ) { return all( equal( B, A0 ) ); }
bool notsame( vec4 B, vec4 A0 ) { return any( notEqual( B, A0 ) ); }
bool all_eq2( vec4 B, vec4 A0, vec4 A1 ) { return same( B, A0 ) && same( B, A1 ); }
bool all_eq3( vec4 B, vec4 A0, vec4 A1, vec4 A2 ) { return same( B, A0 ) && same( B, A1 ) && same( B, A2 ); }
bool all_eq4( vec4 B, vec4 A0, vec4 A1, vec4 A2, vec4 A3 ) { return same( B, A0 ) && same( B, A1 ) && same( B, A2 ) && same( B, A3 ); }
bool any_eq3( vec4 B, vec4 A0, vec4 A1, vec4 A2 ) { return same( B, A0 ) || same( B, A1 ) || same( B, A2 ); }
bool none_eq2( vec4 B, vec4 A0, vec4 A1 ) { return notsame( B, A0 ) && notsame( B, A1 ); }
bool none_eq4( vec4 B, vec4 A0, vec4 A1, vec4 A2, vec4 A3 ) { return notsame( B, A0 ) && notsame( B, A1 ) && notsame( B, A2 ) && notsame( B, A3 ); }

#define src( c, d ) HOOKED_tex( centre + vec2( c, d ) * HOOKED_pt )

vec4 hook()
{
    vec2 centre = ( floor( HOOKED_pos * HOOKED_size ) + 0.5 ) * HOOKED_pt;

    // the current pixel and the 3x3 grid around it
    vec4 E = src( 0.0, 0.0 );
    vec4 A = src( -1.0, -1.0 );
    vec4 B = src( 0.0, -1.0 );
    vec4 C = src( 1.0, -1.0 );
    vec4 D = src( -1.0, 0.0 );
    vec4 F = src( 1.0, 0.0 );
    vec4 G = src( -1.0, 1.0 );
    vec4 H = src( 0.0, 1.0 );
    vec4 I = src( 1.0, 1.0 );

    // nearest magnification by default
    vec4 J = E;
    vec4 K = E;
    vec4 L = E;
    vec4 M = E;

    // constant 3x3 centres stay nearest
    if ( !( same( E, A ) && same( E, B ) && same( E, C ) && same( E, D ) && same( E, F ) && same( E, G ) && same( E, H ) && same( E, I ) ) )
    {
        // the tips of the diamond
        vec4 P = src( 0.0, -2.0 );
        vec4 Q = src( -2.0, 0.0 );
        vec4 R = src( 2.0, 0.0 );
        vec4 S = src( 0.0, 2.0 );

        float Bl = luma( B );
        float Dl = luma( D );
        float El = luma( E );
        float Fl = luma( F );
        float Hl = luma( H );

        // round some corners and fill in 1:1 slopes, but keep sharp right angles
        if ( ( same( D, B ) && notsame( D, H ) && notsame( D, F ) ) && ( ( El >= Dl ) || same( E, A ) ) && any_eq3( E, A, C, G ) && ( ( El < Dl ) || notsame( A, D ) || notsame( E, P ) || notsame( E, Q ) ) ) J = D;
        if ( ( same( B, F ) && notsame( B, D ) && notsame( B, H ) ) && ( ( El >= Bl ) || same( E, C ) ) && any_eq3( E, A, C, I ) && ( ( El < Bl ) || notsame( C, B ) || notsame( E, P ) || notsame( E, R ) ) ) K = B;
        if ( ( same( H, D ) && notsame( H, F ) && notsame( H, B ) ) && ( ( El >= Hl ) || same( E, G ) ) && any_eq3( E, A, G, I ) && ( ( El < Hl ) || notsame( G, H ) || notsame( E, S ) || notsame( E, Q ) ) ) L = H;
        if ( ( same( F, H ) && notsame( F, B ) && notsame( F, D ) ) && ( ( El >= Fl ) || same( E, I ) ) && any_eq3( E, C, G, I ) && ( ( El < Fl ) || notsame( I, H ) || notsame( E, R ) || notsame( E, S ) ) ) M = F;

        // clean up disconnected line intersections
        if ( ( notsame( E, F ) && all_eq4( E, C, I, D, Q ) && all_eq2( F, B, H ) ) && notsame( F, src( 3.0, 0.0 ) ) ) { K = F; M = F; }
        if ( ( notsame( E, D ) && all_eq4( E, A, G, F, R ) && all_eq2( D, B, H ) ) && notsame( D, src( -3.0, 0.0 ) ) ) { J = D; L = D; }
        if ( ( notsame( E, H ) && all_eq4( E, G, I, B, P ) && all_eq2( H, D, F ) ) && notsame( H, src( 0.0, 3.0 ) ) ) { L = H; M = H; }
        if ( ( notsame( E, B ) && all_eq4( E, A, C, H, S ) && all_eq2( B, D, F ) ) && notsame( B, src( 0.0, -3.0 ) ) ) { J = B; K = B; }

        // remove the tips of bright triangles on dark backgrounds
        if ( ( Bl < El ) && all_eq4( E, G, H, I, S ) && none_eq4( E, A, D, C, F ) ) { J = B; K = B; }
        if ( ( Hl < El ) && all_eq4( E, A, B, C, P ) && none_eq4( E, D, G, I, F ) ) { L = H; M = H; }
        if ( ( Fl < El ) && all_eq4( E, A, D, G, Q ) && none_eq4( E, B, C, I, H ) ) { K = F; M = F; }
        if ( ( Dl < El ) && all_eq4( E, C, F, I, R ) && none_eq4( E, B, A, G, H ) ) { J = D; L = D; }

        // 2:1 and 1:2 slopes of constant colour
        if ( notsame( H, B ) )
        {
            if ( notsame( H, A ) && notsame( H, E ) && notsame( H, C ) )
            {
                if ( all_eq3( H, G, F, R ) && none_eq2( H, D, src( 2.0, -1.0 ) ) ) L = M;
                if ( all_eq3( H, I, D, Q ) && none_eq2( H, F, src( -2.0, -1.0 ) ) ) M = L;
            }
            if ( notsame( B, I ) && notsame( B, G ) && notsame( B, E ) )
            {
                if ( all_eq3( B, A, F, R ) && none_eq2( B, D, src( 2.0, 1.0 ) ) ) J = K;
                if ( all_eq3( B, C, D, Q ) && none_eq2( B, F, src( -2.0, 1.0 ) ) ) K = J;
            }
        }
        if ( notsame( F, D ) )
        {
            if ( notsame( D, I ) && notsame( D, E ) && notsame( D, C ) )
            {
                if ( all_eq3( D, A, H, S ) && none_eq2( D, B, src( 1.0, 2.0 ) ) ) J = L;
                if ( all_eq3( D, G, B, P ) && none_eq2( D, H, src( 1.0, -2.0 ) ) ) L = J;
            }
            if ( notsame( F, E ) && notsame( F, A ) && notsame( F, G ) )
            {
                if ( all_eq3( F, C, H, S ) && none_eq2( F, B, src( -1.0, 2.0 ) ) ) K = M;
                if ( all_eq3( F, I, B, P ) && none_eq2( F, H, src( -1.0, -2.0 ) ) ) M = K;
            }
        }
    }

    // which of the 4 output pixels of the source pixel this one is
    vec2 a = fract( HOOKED_pos * HOOKED_size );
    return ( a.x < 0.5 ) ? ( a.y < 0.5 ? J : L ) : ( a.y < 0.5 ? K : M );
}
