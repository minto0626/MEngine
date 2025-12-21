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

Texture2D<float> shadow_map : register(t0);
SamplerState shadow_smp : register(s0);
//SamplerComparisonState shadow_smp : register(s0);

Texture2D<float4> main_tex : register(t1);
SamplerState smp : register(s1);

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
    
    // シャドウマップによる影付け
    // 投影変換後の座標を正規化デバイス座標に変換
    float3 shadowViewProj = input.light_view_pos.xyz / input.light_view_pos.w;
    float2 shadowMapUV = (shadowViewProj.xy + float2(1, -1)) * float2(0.5f, -0.5f);
    float shadowWeight = shadow_map.Sample(shadow_smp, shadowMapUV) < shadowViewProj.z - 0.001f ? 0.5f : 1.0f;
    //float shadowWeight = lerp(0.5f, 1.0f, shadow_map.SampleCmp(shadow_smp, shadowMapUV, shadowViewProj.z - 0.005f));
    color.rgb *= shadowWeight;

    return color;
}
