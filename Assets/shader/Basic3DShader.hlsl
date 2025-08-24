struct BasicInput
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct BasicOutput
{
    float4 sv_pos : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

cbuffer SceneCB : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
};

Texture2D<float4> main_tex : register(t0);
SamplerState smp : register(s0);

BasicOutput vs(BasicInput input)
{
    BasicOutput output;
    output.sv_pos = mul(world, input.pos);
    output.sv_pos = mul(mul(projection, view), output.sv_pos);
    output.uv = input.uv;
    input.normal.w = 0;
    output.normal = mul(world, input.normal);
	return output;
}

float4 ps(BasicOutput input) : SV_TARGET
{
    float3 light = normalize(float3(1, -1, 1));
    float diffuse = saturate(dot(-light, input.normal));
    float4 color = main_tex.Sample(smp, input.uv);
    color.rgb *= diffuse;
    return color;
}