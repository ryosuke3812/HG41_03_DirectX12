#include "Noise.hlsli"

struct PS_IN
{
	float4 position : SV_POSITION;
	float3 wPos : POSITION0;
	float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

cbuffer Param : register(b1)
{
    float3 camPos;
    float time;
}



float4 main(PS_IN pin) : SV_TARGET
{
    float4 color = float4(0, 0, 0, 1.0f);
    
    float noise;
    noise = fBM(pin.uv * 5.0f + time, 3);
    noise = fBM(pin.uv + noise, 3) * 7.0f;
    noise = fBM(pin.uv + time + noise, 3);
    
    float3 color1 = float3(0.0f, 0.1f, 0.3f);
    float3 color2 = float3(0.6f, 0.8f, 1.0f);
    color.rgb = lerp(color1, color2, noise);
    
    // ƒtƒŒƒlƒ‹”½ŽË
    float3 eye = camPos - pin.wPos;
    float3 N = normalize(pin.normal);
    float3 L = normalize(eye);
    
    float dotNL = saturate(dot(N, L));
    
    float f0 = 0.7f;
    float fresnel = f0 + (1.0f - f0) * pow(1.0f - dotNL, 5.0f);
    
    color.a = fresnel;
    
    
    
    
	return color;
}