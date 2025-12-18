#include "Noise.hlsli"
 
struct VS_IN
{
    float3 pos : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};
struct VS_OUT
{
    float4 pos : SV_POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};
 
cbuffer WVP : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
};
 
VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
    // 座標
    vout.pos = float4(vin.pos, 1.0f);
    vout.pos.y = fBM_Turbulence(vin.uv * 3, 3) * 10 - 8.0f;
    vout.pos = mul(vout.pos, world);
    vout.pos = mul(vout.pos, view);
    vout.pos = mul(vout.pos, proj);
    // 法線
    // サイズ/格子の数 = 20 / 500;
    //float3 margin = float3(0.04f, 0.0f, 0.04f);
    //float startX = fBM_Turbulence(vin.uv ? float2(margin.x, 0));
    //float endX = ノイズ関数(vin.uv + float2(margin.x, 0));
    //float startZ = ノイズ関数(vin.uv ? float2(0, margin.z));
    //float endZ = ノイズ関数(vin.uv + float2(0, margin.z));
    //float3 axisX = normalize(float3(margin.x, (endX - startX) * 0.5f, 0.0f));
    //float3 axisZ = normalize(float3(0.0f, (endZ - startZ) * 0.5f, margin.z));
    //vout.normal = normalize(cross(axisZ, axisX));

    vout.normal = vin.normal;
    // UV
    vout.uv = vin.uv;
    return vout;
}