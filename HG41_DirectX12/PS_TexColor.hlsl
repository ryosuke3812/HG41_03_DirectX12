struct PS_IN
{
    float4 pos : SV_POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 main(PS_IN pin) : SV_TARGET {
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    color = tex.Sample(samp, pin.uv);
    return float4(pin.uv, 0, 1);
    return color;
};
