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
    
    int blockNum = 10;  // 格子の数
    float2 vec = pin.uv * blockNum;
    float2 i_uv = floor(vec);   // 拡大したUVの整数部分
    float2 f_uv = frac(vec);    // 拡大したUVの小数部分
    
    //i_uv += sin(pin.time); // 時間で動かす)
    
    // ランダムな点との距離を調べる
    float min_dist = 100.0f; // 最小距離
    // 自身を含む周辺9マスのランダムな点と距離を調べる
    float2 offset[] =
    {
        float2(-1.0f,  1.0f), float2(0.0f,  1.0f), float2(1.0f,  1.0f),
        float2(-1.0f,  0.0f), float2(0.0f,  0.0f), float2(1.0f,  0.0f),
        float2(-1.0f, -1.0f), float2(0.0f, -1.0f), float2(1.0f, -1.0f)
    };
    for (int i = 0; i < 9; ++i)
    {
        // マスのランダムの点を取得
        float2 pos = Random2(i_uv + offset[i]);
        // (時間経過に合わせて点の位置を変える)
        pos += sin(pin.time + pos * 10) * 0.5;
        // 点の位置をオフセット分補正
        pos += offset[i];
        // 補正後の点との距離を計算        
        min_dist = min(min_dist, length(pos - f_uv));
    }
    //min_dist = min(min_dist, length(Random2(i_uv) - f_uv));
    
    
    color.rgb = min_dist;
    return color;
}