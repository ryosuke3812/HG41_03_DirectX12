struct PS_IN
{
    float4 wPos : SV_POSITION;
};

cbuffer Camera : register(b0)
{
    float3 camera;
    float tick;
}

static const float SCALE_DEPTH = 0.25f; // 大気密度
float ExpScale(float fcos)
{
    float x = 1.0 - fcos;
    return SCALE_DEPTH *
exp(-0.00287f + x * (0.459f + x * (3.83f + x * (-6.8f + x * 5.25f))));
}

float RayleightPhase(float d)
{
    // レイリー散乱
    return (3.0f / 4.0f) * (1.0f + d * d); // d = cosθ
    
}
float MiePhase(float d, float g)
{
    // ミー散乱
    return (3.0f / 8.0f) * ((1.0f - g * g) * (1.0f + d * d)) /
        pow(1.0f + g * g - 2.0f * g * d, 1.5f);
}

float4 main(PS_IN pin) : SV_TARGET
{
    //--- 光に関する定義
    const float Sun = 20.0f; // ？太陽の強さ
    const float Kr = 0.0025f; // レイリー散乱係数（？吸光度
    const float Km = 0.001f; // ミー散乱係数（？吸光度
    const float KrSun = Kr * Sun;
    const float KmSun = Km * Sun;
    const float Kr4PI = Kr * 4.0f * 3.141592f;
    const float Km4PI = Km * 4.0f * 3.141592f;
    // 光の周波数(https://www.otsukael.jp/weblearn/chapter/learnid/87/page/2
    const float3 WaveLength = float3(0.68f, 0.55f, 0.44f);
    // 散乱した光の強度は波長の4乗に反比例(https://global.canon/ja/technology/s_labo/light/001/01.html
    const float3 InvWaveLength = 1.0f / pow(WaveLength, 4);
    //--- 実際の地球の大きさ(km
    const float RealAtomosphere = 100.0f;
    const float RealInnerR = 6300.0f;
    const float RealOuterR = RealAtomosphere + RealInnerR;
    
    //--- ゲーム用にスケールダウンした地球
    const float InnerR = 100.0f;
    const float OuterR = RealOuterR * InnerR / RealInnerR;
    const float Atomosphere = OuterR - InnerR;
    
    const float AtomosphereScale = 1.0f / Atomosphere;
    //--- ゲーム空間内でのカメラの高さ
    const float GameSkySize = 100.0f;
    const float CameraH = max(0.0f, camera.y);
    const float CameraHRate = CameraH / GameSkySize;
    //--- カメラ
    float3 ViewRay = normalize(pin.wPos.xyz - camera);
    float3 ViewPos = float3(0.0f, InnerR + Atomosphere * CameraHRate, 0.0f);
    //--- 大気の位置
    float a = 1.0f;
    float b = 2.0f * dot(ViewPos, ViewRay);
    float c = dot(ViewPos, ViewPos) - OuterR * OuterR;
    // 解の公式
    float x = (-b + sqrt(b * b - 4.0f * a * c)) / (2.0f * a);
    float ViewLength = x;
    float3 AtomospherePos = ViewPos + ViewRay * ViewLength;
    //--- ライト
    float time = tick * 0.01f;
    float3 L = normalize(-float3(cos(time), sin(time), 0.0f));
    //--- サンプリング初期値
    const int Samples = 2;
    float scaleOverScaleDepth = AtomosphereScale / SCALE_DEPTH; // ？
    float3 startPos = ViewPos;
    float sampleLength = ViewLength / Samples;
    float scaledLength = sampleLength * AtomosphereScale;
    float3 sampleRay = ViewRay * sampleLength;
    float3 samplePos = startPos + sampleRay * 0.5f; // ベクトルの中点をサンプリング位置とする。
    //--- ？
    float height = length(startPos);
    float startAngle = dot(ViewRay, startPos) / height;
    float startDepth = exp(scaleOverScaleDepth * (InnerR - length(startPos)));
    float startOffset = startDepth * ExpScale(startAngle);
    //--- サンプリング
    float3 frontColor = float3(0.0f, 0.0f, 0.0f);
    for (
    int i = 0;i <
    Samples;++i) {
    float tempHeight = length(samplePos);
    float tempDepth = exp(scaleOverScaleDepth * (InnerR - tempHeight));
    float lightAngle = dot(-L, samplePos) / tempHeight;
    float cameraAngle = dot(ViewRay, samplePos) / tempHeight;
    // 分散(？多分サンプリング位置に入ってくる光の外積
    float scatter = startOffset + tempDepth * (ExpScale(lightAngle) - ExpScale(cameraAngle));
    // 減衰
    float3 attenuate = exp(-scatter * (InvWaveLength * Kr4PI + Km4PI));
    frontColor += attenuate * (tempDepth * scaledLength);
    samplePos +=
    sampleRay;
    }
    // サンプリング位置へ入射した光の色
    float3 rColor = frontColor * (InvWaveLength * KrSun);
    float3 mColor = frontColor * KmSun;
    // 位相関数
    float d = dot(L, ViewRay);
    float g = -0.999f;
    float4 color = float4(0, 0, 0, 1);
    color.rgb = rColor * RayleightPhase(d) + mColor * MiePhase(d, g);
    return color;
}