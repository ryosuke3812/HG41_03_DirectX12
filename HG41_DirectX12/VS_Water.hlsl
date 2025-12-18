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
    float3 wPos : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

cbuffer WVP : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Projection;
};

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
    
    // 座標
    vout.pos = float4(vin.pos, 1.0f);
    vout.pos = mul(vout.pos, World);
    vout.wPos = vout.pos.xyz; // ワールド座標を保存
    vout.pos = mul(vout.pos, View);
    vout.pos = mul(vout.pos, Projection);
    // 法線
    vout.normal = vin.normal;
    // UV
    vout.uv = vin.uv;

    
    
    
    return vout;
}