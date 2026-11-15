// Vertex and fragment program of the GUI material of guidriverOgre (COgreGuiMaterial): a vertex colour times a
// texture, for GUI geometry in pixels of the GUI. Direct3D 11, shader model 4 (entry points GucefGuiVS, GucefGuiPS).

uniform float4x4 projection;
uniform float4x4 transform;
uniform float2 translation;

Texture2D guiTexture : register( t0 );
SamplerState guiSampler : register( s0 );

struct SGuiVertexIn
{
    float4 position : POSITION;
    float4 colour   : COLOR0;
    float2 uv       : TEXCOORD0;
};

struct SGuiVertexOut
{
    float4 position : SV_POSITION;
    float4 colour   : COLOR0;
    float2 uv       : TEXCOORD0;
};

SGuiVertexOut GucefGuiVS( SGuiVertexIn input )
{
    SGuiVertexOut output;
    output.position = mul( projection, mul( transform, float4( input.position.xy + translation, 0.0, 1.0 ) ) );
    output.colour = input.colour;
    output.uv = input.uv;
    return output;
}

float4 GucefGuiPS( SGuiVertexOut input ) : SV_Target
{
    return input.colour * guiTexture.Sample( guiSampler, input.uv );
}
