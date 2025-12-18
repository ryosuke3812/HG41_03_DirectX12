#include "Noise.hlsli"

struct VS_IN
{
    float3 pos : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

cbuffer WVP : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

VS_OUT main (VS_IN vin)
{
    VS_OUT vout;
    
    // 座標
    vout.pos = float4(vin.pos, 1.0f);
    vout.pos.y = fBM(vin.uv * 3, 3) * 10 - 5;
    vout.pos = mul(vout.pos, World);
    vout.pos = mul(vout.pos, View);
    vout.pos = mul(vout.pos, Projection);
    
    // サイズ/格子の
    float3 margin = float3(0.04f, 0.0f, 0.04f);
    
    
    // 法線
    vout.normal = vin.normal;
    
    // UV
    vout.uv = vin.uv;
    
    
    return vout;
}