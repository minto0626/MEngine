struct BasicInput
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct BasicOutput
{
    float4 sv_pos : SV_POSITION;
    float4 world_pos : POSITION0;
    float4 light_view_pos : POSITION1;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSOutput
{
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 world_pos : SV_TARGET2;
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

Texture2D<float4> main_tex : register(t0);
SamplerState smp : register(s0);

BasicOutput vs(BasicInput input)
{
    BasicOutput output;
    output.sv_pos = mul(world, input.pos);
    output.world_pos = output.sv_pos;
    output.sv_pos = mul(mul(projection, view), output.world_pos);
    output.light_view_pos = mul(lightViewProj, output.world_pos);
    output.uv = input.uv;
    input.normal.w = 0;
    output.normal = mul(world, input.normal);
	return output;
}

PSOutput ps(BasicOutput input)
{
    PSOutput output;
    
    output.albedo = main_tex.Sample(smp, input.uv);
    output.normal = float4(normalize(input.normal.xyz) * 0.5f + 0.5f, 1.0f);
    output.world_pos = input.world_pos;
    output.world_pos.w = 1.0f;

    return output;
}
