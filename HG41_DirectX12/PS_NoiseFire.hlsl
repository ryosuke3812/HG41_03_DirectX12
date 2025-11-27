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
    float2 uv = pin.uv;
    
    // UVの分割数を上げて炎を細かくする
    uv.x *= 10.0f;
    uv.y *= 5.0f; 
    
    // 炎が燃え上がっている感を出すために上にスクロール
    uv.y = pin.time;
    
    // ノイズを取得
    float noise = PerlinNoise(uv);
    
    float4 color;
    
    // ノイズの値をもとに色を付ける
    float3 lowColor = float3(0.8f, 0.8f, 0.0f); 
    float3 highColor = float3(1.0f, 0.0f, 0.0f);
    color.rgb = lerp(lowColor, highColor, noise);
    
    // 高さ(uv.y)に応じて透明度を付ける
    color.a = pin.uv.y;
    
    color.a -= noise * 0.5f; // ノイズの値も透明度に影響させる
    
    
    return color;
}