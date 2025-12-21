struct BasicInput
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

cbuffer SceneCB : register(b0)
{
    matrix view;
    matrix projection;
    float3 eyePos;
    float pad0;

    matrix lightViewProj;
    float3 lightVec;
    float pad1;
};

cbuffer Transform : register(b1)
{
    matrix world;
};

float4 vs(BasicInput input) : SV_Position
{
    float4 pos = mul(world, input.pos);
    pos = mul(lightViewProj, pos);
    return pos;
}
