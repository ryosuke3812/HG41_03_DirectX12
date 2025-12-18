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
    // uvの分割数
    float blockNum = 10.0f;
    float2 uv = pin.uv * blockNum;
    
    // ランダム生成に必要なuvの値を取得
    float2 i_uv = floor(uv); // 拡大したUVの整数部分
    float2 f_uv = frac(uv); // 拡大したUVの小数部分
    
    f_uv = f_uv * f_uv * (3.0f - 2.0f * f_uv);
    
    
    // 4個のランダムな値を取得
    float2 offset[] =
    {
        { 0.0f, 0.0f }, // 左上
        { 1.0f, 0.0f }, // 右上
        { 0.0f, 1.0f }, // 左下
        { 1.0f, 1.0f }  // 右下
    };
    
    float lt, rt, lb, rb; // 左上(LeftTop)、右上、左下、右下
    lt = Random_(i_uv + offset[0]);
    rt = Random_(i_uv + offset[1]);
    lb = Random_(i_uv + offset[2]);
    rb = Random_(i_uv + offset[3]);
    
    // UVの値に基づいてランダムな値を補間
    float top       = lerp(lt, rt, f_uv.x);         // 上辺
    float bottom    = lerp(lb, rb, f_uv.x);         // 下辺
    float noise     = lerp(top, bottom, f_uv.y);    // 全体
    
    
    return float4(noise, noise, noise, 1.0f);
}