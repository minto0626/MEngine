struct BasicInput
{
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct BasicOutput
{
    float4 sv_pos : SV_POSITION;
    float4 world_pos : POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCOORD;
};

cbuffer SceneCB : register(b0)
{
    matrix view;
    matrix projection;
    float3 eyePos;
    float pad0;

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
    output.sv_pos = mul(mul(projection, view), output.sv_pos);
    output.uv = input.uv;
    input.normal.w = 0;
    output.normal = mul(world, input.normal);
	return output;
}

float4 ps(BasicOutput input) : SV_TARGET
{
    // Phongの拡散反射モデル

    float3 light = normalize(lightVec);

    float3 ref = reflect(light, input.normal.xyz);
    float3 toEye = normalize(eyePos - input.world_pos.xyz);
    float specular = pow(saturate(dot(ref, toEye)), 50);
    float ambient = 0.3f;
    float diffuse = saturate(dot(-light, input.normal.xyz));
    float4 color = main_tex.Sample(smp, input.uv);
    color.rgb *= diffuse + specular + ambient;
    return color;
}
