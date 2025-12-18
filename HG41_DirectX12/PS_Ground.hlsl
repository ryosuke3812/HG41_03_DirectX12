struct PS_IN
{
    float4 pos : SV_Position;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
};

float4 main(PS_IN pin) : SV_TARGET
{
    // ライティング
    float3 L = normalize(float3(1, -1, 1));
    float3 N = normalize(pin.normal);
    float d = saturate(dot(-L, N));
    
    
    
    return float4(d, d, d, 1.0f);
    //return float4(pin.uv, d, 1.0f);
}