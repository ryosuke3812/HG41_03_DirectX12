struct VS_IN {
    float3 pos : POSITION0;
    float4 color : COLOR0;
};
struct VS_OUT
{
    float4 pos : SV_POSITION0;
    float4 color : COLOR0;
};
cbuffer Matrix : register(b0) {
    float4x4 WVP;
};

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
    vout.pos = float4(vin.pos, 1.0f);
    vout.pos = mul(vout.pos, WVP);
    vout.color = vin.color;
    return vout;
}
