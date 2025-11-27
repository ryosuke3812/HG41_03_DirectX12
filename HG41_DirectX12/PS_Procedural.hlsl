struct PS_IN
{
    float4 pos : SV_POSITION0;
    float2 uv : TEXCOORD0;
    float time : TEXCOORD1;
    float3 wPos : POSITION0;
};

float4 main(PS_IN pin) : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}