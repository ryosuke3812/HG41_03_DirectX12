#include "Noise.hlsli"
 
struct PS_IN
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    float time : TEXCOORD1;
    float3 wPos : POSITION0;
};
 
float4 main(PS_IN pin) : SV_TARGET
{
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    // floor - ¬”‚ğØ‚èÌ‚Ä‚éŠÖ”
    color.rgb = PerlinNoise(pin.uv * 10.0f);
    
    
    
    return color;
}