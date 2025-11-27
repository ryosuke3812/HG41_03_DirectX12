struct VS_IN
{
    float3 pos : POSITION0;
    float2 uv : TEXCOORD0;
};
struct VS_OUT
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    float time : TEXCOORD1;
    float3 wPos : POSITION0;
};
cbuffer Param : register(b0) {
    float4x4 world;
    float4x4 view;
    float4x4 proj;
    float time;
    float3 dummy;
};
VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
    vout.pos = float4(vin.pos, 1.0f);
    vout.pos = mul(vout.pos, world);
    vout.wPos = vout.pos.xyz;
    vout.pos = mul(vout.pos, view);
    vout.pos = mul(vout.pos, proj);
    vout.uv = vin.uv;
    vout.time = time;
    return vout;
}
