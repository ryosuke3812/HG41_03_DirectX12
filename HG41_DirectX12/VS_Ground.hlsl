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
    
    // 高さ計算（Y座標）
    // ノイズ関数を用いてY座標を変位させる
    vout.pos.y = fBM_Turbulence(vin.uv * 3, 3) * 10 - 4.0f;
    
    vout.pos = mul(vout.pos, world);
    vout.pos = mul(vout.pos, view);
    vout.pos = mul(vout.pos, proj);

    // --- 法線計算の修正 ---
    
    // グリッドの間隔（World座標系）
    float3 margin = float3(0.04f, 0.0f, 0.04f); // 20.0f / 500.0f
    
    // UV空間での微小変化量 (Worldでの0.04f移動は、UV空間の全体20.0fに対して0.002fに相当)
    float2 uvStep = float2(0.002f, 0.002f);

    // 近傍点の高さを取得（中央差分法のようなアプローチ）
    // 注意: 位置計算と同じスケール(*3, *10)を適用する必要があります
    float startX = fBM_Turbulence((vin.uv - float2(uvStep.x, 0)) * 3, 3) * 10;
    float endX = fBM_Turbulence((vin.uv + float2(uvStep.x, 0)) * 3, 3) * 10;
    
    float startZ = fBM_Turbulence((vin.uv - float2(0, uvStep.y)) * 3, 3) * 10;
    float endZ = fBM_Turbulence((vin.uv + float2(0, uvStep.y)) * 3, 3) * 10;

    // 接線ベクトルの生成
    // (end - start) * 0.5f は中央差分の勾配（高さの半分）を表し、marginはその間の距離
    float3 axisX = normalize(float3(margin.x, (endX - startX) * 0.5f, 0.0f));
    float3 axisZ = normalize(float3(0.0f, (endZ - startZ) * 0.5f, margin.z));

    // 接線同士の外積で法線を求める
    vout.normal = normalize(cross(axisZ, axisX));
    
    // --------------------

    // UV
    vout.uv = vin.uv;
    return vout;
}