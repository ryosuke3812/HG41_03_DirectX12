// --- 各種乱数
// float2からfloatを生成
float Random(float2 vec)
{
    return frac(sin(dot(
        vec,
        float2(12.9898f, 78.233f))
    ) * 43758.5453123f);
}

// float2からfloat2を生成
float2 Random2(float2 vec)
{
    return frac(sin(float2(
        dot(vec, float2(127.1f, 311.7f)),
        dot(vec, float2(269.5f, 183.3f))
    )) * 43758.5453f);
}

float3 Random3(float3 vec)
{
    return frac(sin(float3(
                    dot(vec, float3(127.1f, 311.7f, 245.4f)),
                    dot(vec, float3(269.5f, 183.3f, 131.2f)),
                    dot(vec, float3(522.3f, 243.1f, 532.4f)))) * 43758.5453);
}

// パーリンノイズ
float PerlinNoise(float2 vec)
{
    // 整数部分と小数部分を取り出す
    float2 i_uv = floor(vec);
    float2 f_uv = frac(vec);
    
    // 四隅までの移動量
    float2 offset[] =
    {
        { 0.0f, 0.0f }, // 左上
        { 1.0f, 0.0f }, // 右上
        { 0.0f, 1.0f }, // 左下
        { 1.0f, 1.0f } // 右下 }
    };
    
    // 四隅のランダムなベクトルを取得
    float2 corner[] =
    {
        Random2(i_uv + offset[0]) * 2.0f - 1.0f, // 左上
        Random2(i_uv + offset[1]) * 2.0f - 1.0f, // 右上
        Random2(i_uv + offset[2]) * 2.0f - 1.0f, // 左下
        Random2(i_uv + offset[3]) * 2.0f - 1.0f  // 右下
    };

    // 現在処理しているuvの値までのベクトルと
    // 四隅のランダムなベクトルで内積した結果を四隅の値とする
    float lt, rt, lb, rb; // 左上(LeftTop)、右上、左下、右下
    lt = dot(corner[0], f_uv - offset[0]);
    rt = dot(corner[1], f_uv - offset[1]);
    lb = dot(corner[2], f_uv - offset[2]);
    rb = dot(corner[3], f_uv - offset[3]);
    
    f_uv = smoothstep(0.05f, 0.95, f_uv);
    
    float top = lerp(lt, rt, f_uv.x); // 上辺
    float bottom = lerp(lb, rb, f_uv.x); // 下辺
    float noise = lerp(top, bottom, f_uv.y); // 全体
    
    
    return (noise + 1.0f) * 0.5f;
}


// fBM
float fBM(float2 vec, int octaves)
{
    const float lacunarity = 2.0f;  // 解像度の変化割合
    const float gain = 0.5f;    // 重ね合わせの変化割合
    
    float amplitude = 0.5f; // 重ね合わせの強さs
    float frequency = 1.0f; // 解像度
    
    float n = 0.0f; // ノイズ値の合計
    for (int i = 0;i < octaves; ++i)
    {
        n += PerlinNoise(vec * frequency) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }
    
    return n;
}

