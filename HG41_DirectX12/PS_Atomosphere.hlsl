struct VS_OUT
{
    float4 pos : SV_POSITION0;
    float3 wPos : TEXCOORD0;
};

// C++側の m_pWVPs[1] から渡されるパラメータ
cbuffer Param : register(b1)
{
    float3 camera; // カメラ座標
    float tick; // 時間
};

static const float SCALE_DEPTH = 0.25f;

// 密度関数の近似式
float ExpScale(float fcos)
{
    float x = 1.0f - fcos;
    return SCALE_DEPTH * exp(-0.00287f + x * (0.459f + x * (3.83f + x * (-6.8f + x * 5.25f))));
}

// レイリー散乱の位相関数
float RayleighPhase(float d)
{
    return 0.75f * (1.0f + d * d);
}

// ミー散乱の位相関数
float MiePhase(float d, float g)
{
    float g2 = g * g;
    float f = (3.0f * (1.0f - g2)) / (2.0f * (2.0f + g2));
    float num = 1.0f + d * d;
    float den = pow(1.0f + g2 - 2.0f * g * d, 1.5f);
    return f * (num / den);
}

// 光に関する定義
static const float Sun = 20.0f;
static const float Kr = 0.0025f;
static const float Km = 0.001f;
static const float KrSun = Kr * Sun;
static const float KmSun = Km * Sun;
static const float Kr4PI = Kr * 4.0f * 3.141592f;
static const float Km4PI = Km * 4.0f * 3.141592f;

static const float3 WaveLength = float3(0.68f, 0.55f, 0.44f);
static const float3 InvWaveLength = 1.0f / pow(WaveLength, 4);

// 地球のスケール設定
static const float RealInnerR = 6300.0f;
static const float RealAtomosphere = 100.0f;
static const float RealOuterR = RealAtomosphere + RealInnerR;

static const float InnerR = 100.0f;
static const float OuterR = RealOuterR * (InnerR / RealInnerR);
static const float Atomosphere = OuterR - InnerR;
static const float AtomosphereScale = 1.0f / Atomosphere;

static const float GameSkySize = 100.0f;

float4 main(VS_OUT pin) : SV_TARGET
{
    float CameraH = max(0.0f, camera.y);
    float CameraHRate = CameraH / GameSkySize;

    float3 ViewRay = normalize(pin.wPos - camera);
    float3 ViewPos = float3(0.0f, InnerR + Atomosphere * CameraHRate, 0.0f);

    // 大気圏外周との交点を求める2次方程式
    float a = 1.0f;
    float b = 2.0f * dot(ViewPos, ViewRay);
    float c = dot(ViewPos, ViewPos) - (OuterR * OuterR);
    float det = max(0.0f, b * b - 4.0f * a * c);
    float ViewLength = (-b + sqrt(det)) / (2.0f * a);

    // 時間経過による太陽光の向き（夕焼けのシミュレーション）
    float time = tick * 0.01f;
    float3 L = normalize(-float3(cos(time), sin(time), 0.0f));

    // サンプリング設定
    const int Samples = 2;
    float scaleOverScaleDepth = AtomosphereScale / SCALE_DEPTH;

    float3 startPos = ViewPos;
    float sampleLength = ViewLength / Samples;
    float scaledLength = sampleLength * AtomosphereScale;
    float3 sampleRay = ViewRay * sampleLength;
    float3 samplePos = startPos + sampleRay * 0.5f;

    float height = length(startPos);
    float startAngle = dot(ViewRay, startPos) / height;
    float startDepth = exp(scaleOverScaleDepth * (InnerR - height));
    float startOffset = startDepth * ExpScale(startAngle);

    float3 frontColor = float3(0.0f, 0.0f, 0.0f);

    // 大気中での光の減衰・散乱ループ
    for (int i = 0; i < Samples; ++i)
    {
        float tempHeight = length(samplePos);
        float tempDepth = exp(scaleOverScaleDepth * (InnerR - tempHeight));
        float lightAngle = dot(L, samplePos) / tempHeight;
        float cameraAngle = dot(ViewRay, samplePos) / tempHeight;

        float scatter = startOffset + tempDepth * (ExpScale(lightAngle) - ExpScale(cameraAngle));
        float3 attenuate = exp(-scatter * (InvWaveLength * Kr4PI + Km4PI));
        
        frontColor += attenuate * (tempDepth * scaledLength);
        samplePos += sampleRay;
    }

    float3 rColor = frontColor * (InvWaveLength * KrSun);
    float3 mColor = frontColor * KmSun;

    // 最終的な色の合成
    float d = dot(L, ViewRay);
    float g = -0.99f; // ミー散乱の非対称パラメータ (-1～1)

    float4 color = float4(0, 0, 0, 1);
    color.rgb = rColor * RayleighPhase(d) + mColor * MiePhase(d, g);

    return saturate(color);
}