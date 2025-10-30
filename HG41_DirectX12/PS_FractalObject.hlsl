struct PS_IN {
    float4 pos : SV_POSITION0;
    float4 color : COLOR0;
};

float4 main(PS_IN pin) : SV_TARGET
{
    return pin.color;
};
